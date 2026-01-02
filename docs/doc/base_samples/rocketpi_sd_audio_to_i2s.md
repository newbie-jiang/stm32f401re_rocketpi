# rocketpi_sd_audio_to_i2s

## 项目概述
- 从 SD 卡读取 16-bit 立体声 PCM，经 I2S2+DMA 输出，可播放 SD 流和内置音轨。   （拷贝整个audio目录至sd卡根目录）
- FATFS 速度测试工具，用于测量 SD 写入/读取性能。
- ST7789 SPI 屏幕初始化并显示实时频谱（Goertzel 算法，40 个频段）。

## 关键模块
- Core/Src/main.c：系统初始化、SD 双缓冲和状态机、音频播放与 I2S 回调、频谱计算与刷新。
- Core/Inc/audio.h：示例 PCM 音轨，16 kHz、立体声、16-bit。
- Core/Src/i2s.c、dma/sdio/fatfs 等：HAL 外设初始化（I2S、DMA、SDIO、FATFS）。
- st7789 驱动：SPI+DMA 方式驱动屏幕绘制频谱。

## 使用说明
1. 在 SD 根目录放置 `audio/audio.bin`，格式为 16 kHz、16-bit、立体声 PCM（与 AUDIO_SAMPLE_RATE_HZ/AUDIO_BITS_PER_SAMPLE/AUDIO_NUM_CHANNELS 一致）。
2. 上电后自动播放：双缓冲从 SD 读取并送入 I2S DMA；SD 出错时可回退到编译时内置音轨。
3. 频谱基于左声道数据，默认刷新周期 `SPECTRUM_DRAW_INTERVAL_MS = 40ms`。

## 可调参数（main.c）
- `SPECTRUM_BIN_COUNT`：频谱柱数量（默认 40）。
- `SPECTRUM_WINDOW_FRAMES`：Goertzel 窗长（默认 256）。
- `SPECTRUM_DRAW_INTERVAL_MS`：频谱刷新周期（默认 40 ms）。
- `AUDIO_STREAM_BUFFER_BYTES`：SD 流双缓冲大小（默认 5120 字节）。
- 其他：`AUDIO_DMA_MAX_TRANSFER_SAMPLES`、`FATFS_SPEED_TEST_BUFFER_SIZE` 等按需调整。

## 数字音量
- 宏 `AUDIO_VOLUME_PERCENT`（Core/Src/main.c），范围 0–100，默认 100。
- SD 流：每次从文件读入双缓冲后即按 Q15 乘法 + 饱和缩放；
  内置音轨：先拷贝到 RAM 缓冲，再缩放后送 DMA。
- 0 = 静音，100 = 直通；修改宏后重新编译下载即可。

## 注意事项
- 确保 SD 卡文件格式正确，帧大小需对齐到 16-bit 双声道。
- ST7789/I2S/SDIO 时钟与引脚若修改，请在 CubeMX 同步配置。
- 频谱计算和屏幕刷新占用 MCU 资源，可按需降低刷新频率或频段数。

## 扩展方向
- 增加播放列表与上一曲/下一曲控制。
- 支持单声道或其他采样率的自动转换。
- 增加 UI 控件显示播放状态、音量等信息。

<!-- BSP_DRIVERS_START -->
<!-- BSP_DRIVERS_HASH:97b74b1549ac461fed3795429d613cd31cbc99be725db9a3a0c6486365e19154 -->
## 驱动以及测试代码

<details>
<summary>Core/Src/main.c</summary>

```c
﻿/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "crc.h"
#include "dma.h"
#include "fatfs.h"
#include "i2s.h"
#include "sdio.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <ctype.h>
#include <stdbool.h>
#include <stdarg.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "audio.h"
#include "st7789.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum
{
  AUDIO_PLAYBACK_STATE_IDLE = 0,
  AUDIO_PLAYBACK_STATE_RUNNING,
  AUDIO_PLAYBACK_STATE_DONE,
  AUDIO_PLAYBACK_STATE_ERROR
} audio_playback_state_t;

typedef struct
{
  uint32_t next_index;
  uint32_t samples_remaining;
  audio_playback_state_t state;
} audio_playback_ctrl_t;

typedef enum
{
  AUDIO_SD_STATE_IDLE = 0,
  AUDIO_SD_STATE_PLAYING,
  AUDIO_SD_STATE_DONE,
  AUDIO_SD_STATE_ERROR
} audio_sd_state_t;

typedef struct
{
  FIL file;
  UINT half_bytes;
  UINT frame_bytes;
  UINT bytes_next;
  uint8_t *buf0;
  uint8_t *buf1;
  uint8_t *read_buf;
  uint8_t mounted;
  uint8_t file_opened;
  audio_sd_state_t state;
  FRESULT last_res;
  HAL_StatusTypeDef last_hal;
} audio_sd_playback_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#ifndef _USE_MKFS
#define _USE_MKFS 1
#endif

#if (_USE_MKFS == 1)
#define FATFS_MKFS_BUFFER_SIZE 4096U
#endif
#define FATFS_SPEED_TEST_BUFFER_SIZE 4096U

#define AUDIO_DMA_MAX_TRANSFER_SAMPLES 65535U
#define AUDIO_FILE_DIRECTORY        "audio"
#define AUDIO_FILE_NAME             "audio.bin"
#define AUDIO_FILE_PATH_MAX         64U
#define AUDIO_STREAM_BUFFER_BYTES   5120U

#define AUDIO_VOLUME_PERCENT        100U   /* 0-100%，用于数字音量缩放 */
#define AUDIO_VOLUME_BUFFER_SAMPLES 2048U  /* 内置音轨播放时的临时缓冲区大小（采样数） */

#define SPECTRUM_BIN_COUNT          40U
#define SPECTRUM_WINDOW_FRAMES      256U
#define SPECTRUM_DRAW_INTERVAL_MS   40U
#define SPECTRUM_MAX_HEIGHT         (ST7789_HIGHT - 10U)
#define SPECTRUM_BAR_GAP            2U
#define SPECTRUM_SMOOTH_ALPHA       0.30f
#define SPECTRUM_DECAY_ALPHA        0.90f
#define SPECTRUM_PEAK_INIT          2000.0f
#ifndef M_PI
#define M_PI                        3.1415926f
#endif
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
static volatile audio_playback_ctrl_t audio_ctrl = {0};
static uint16_t audio_sd_buffer[AUDIO_STREAM_BUFFER_BYTES / sizeof(uint16_t)];
static audio_sd_playback_t audio_sd_ctx = {0};
static volatile uint8_t audio_stream_from_sd = 0U;
static volatile uint8_t audio_sd_dma_done = 0U;
static uint16_t audio_volume_q15 = 0U;
static int16_t audio_volume_buffer[AUDIO_VOLUME_BUFFER_SAMPLES];
static float spectrum_coeff[SPECTRUM_BIN_COUNT];
static float spectrum_smooth[SPECTRUM_BIN_COUNT];
static uint16_t spectrum_levels[SPECTRUM_BIN_COUNT];
static uint16_t spectrum_draw_levels[SPECTRUM_BIN_COUNT];
static float spectrum_peak_avg = SPECTRUM_PEAK_INIT;
static uint32_t spectrum_last_draw_ms = 0U;
typedef struct
{
  float pos;
  uint16_t color;
} spectrum_stop_t;
static const spectrum_stop_t spectrum_gradient[] = {
  {0.0f, BLUE},
  {0.25f, CYAN},
  {0.50f, GREEN},
  {0.75f, YELLOW},
  {1.0f, RED}
};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
static void FATFS_SpeedTest(uint32_t kilobytes);
static HAL_StatusTypeDef Audio_StartNextChunk(void);
static void Audio_BeginPlayback(void);
static void Audio_SD_StopAndCleanup(void);
static FRESULT Audio_SD_StartPlayback(const char *directory,
                                      const char *file_name,
                                      uint8_t *work_buffer,
                                      UINT work_buffer_bytes);
static void Audio_SD_ProcessPlayback(void);
static void Spectrum_Init(void);
static void Spectrum_UpdateFromBuffer(const int16_t *samples, size_t sample_count);
static void Spectrum_DrawIfDue(void);
static uint16_t Spectrum_LerpColor(uint16_t c0, uint16_t c1, float t);
static uint16_t Spectrum_ColorForBin(uint32_t index);
static uint16_t Audio_ComputeVolumeQ15(void);
static int16_t Audio_ApplyVolumeToSample(int16_t sample, uint16_t gain_q15);
static void Audio_ApplyVolumeToBuffer(int16_t *buffer, size_t samples, uint16_t gain_q15);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static uint16_t Audio_ComputeVolumeQ15(void)
{
  uint32_t percent = AUDIO_VOLUME_PERCENT;
  if (percent > 100U)
  {
    percent = 100U;
  }

  return (uint16_t)(((percent * 32767U) + 50U) / 100U);
}

static int16_t Audio_ApplyVolumeToSample(int16_t sample, uint16_t gain_q15)
{
  int32_t scaled = (int32_t)sample * (int32_t)gain_q15;
  scaled += (1 << 14); /* 四舍五入 */
  scaled >>= 15;

  if (scaled > 32767)
  {
    scaled = 32767;
  }
  else if (scaled < -32768)
  {
    scaled = -32768;
  }

  return (int16_t)scaled;
}

static void Audio_ApplyVolumeToBuffer(int16_t *buffer, size_t samples, uint16_t gain_q15)
{
  if ((buffer == NULL) || (samples == 0U))
  {
    return;
  }

  if (gain_q15 == 0U)
  {
    memset(buffer, 0, samples * sizeof(int16_t));
    return;
  }

  if (gain_q15 >= 32767U)
  {
    return; /* 100% 音量，无需处理 */
  }

  for (size_t i = 0U; i < samples; ++i)
  {
    buffer[i] = Audio_ApplyVolumeToSample(buffer[i], gain_q15);
  }
}

static HAL_StatusTypeDef Audio_StartNextChunk(void)
{
  if (audio_ctrl.samples_remaining == 0U)
  {
    audio_ctrl.state = AUDIO_PLAYBACK_STATE_DONE;
    return HAL_OK;
  }

  uint32_t chunk_limit = (AUDIO_VOLUME_BUFFER_SAMPLES < AUDIO_DMA_MAX_TRANSFER_SAMPLES) ?
                         AUDIO_VOLUME_BUFFER_SAMPLES :
                         AUDIO_DMA_MAX_TRANSFER_SAMPLES;
  uint32_t chunk = (audio_ctrl.samples_remaining > chunk_limit) ?
                   chunk_limit :
                   audio_ctrl.samples_remaining;

  const int16_t *chunk_ptr = (const int16_t *)&audio_track[audio_ctrl.next_index];
  memcpy(audio_volume_buffer, chunk_ptr, chunk * sizeof(int16_t));
  Audio_ApplyVolumeToBuffer(audio_volume_buffer, chunk, audio_volume_q15);

  HAL_StatusTypeDef status = HAL_I2S_Transmit_DMA(&hi2s2,
                                                  (uint16_t *)audio_volume_buffer,
                                                  (uint16_t)chunk);

  if (status == HAL_OK)
  {
    audio_ctrl.next_index += chunk;
    audio_ctrl.samples_remaining -= chunk;
    audio_ctrl.state = AUDIO_PLAYBACK_STATE_RUNNING;
  }

  return status;
}

static void Audio_BeginPlayback(void)
{
  audio_volume_q15 = Audio_ComputeVolumeQ15();
  audio_ctrl.next_index = 0U;
  audio_ctrl.samples_remaining = (uint32_t)AUDIO_TRACK_SAMPLE_COUNT;
  audio_ctrl.state = AUDIO_PLAYBACK_STATE_IDLE;

  if (audio_ctrl.samples_remaining == 0U)
  {
    audio_ctrl.state = AUDIO_PLAYBACK_STATE_DONE;
    return;
  }

  if (Audio_StartNextChunk() != HAL_OK)
  {
    audio_ctrl.state = AUDIO_PLAYBACK_STATE_ERROR;
    Error_Handler();
  }
}

static void Audio_SD_StopAndCleanup(void)
{
  if (audio_sd_ctx.file_opened)
  {
    f_close(&audio_sd_ctx.file);
    audio_sd_ctx.file_opened = 0U;
  }

  if (audio_sd_ctx.mounted)
  {
    f_mount(NULL, (TCHAR const*)SDPath, 0);
    audio_sd_ctx.mounted = 0U;
  }

  audio_stream_from_sd = 0U;
}

static FRESULT Audio_SD_StartPlayback(const char *directory,
                                      const char *file_name,
                                      uint8_t *work_buffer,
                                      UINT work_buffer_bytes)
{
  audio_volume_q15 = Audio_ComputeVolumeQ15();
  const UINT frame_bytes = (UINT)(AUDIO_NUM_CHANNELS * sizeof(int16_t));
  if ((work_buffer == NULL) ||
      (work_buffer_bytes < (2U * frame_bytes)) ||
      ((work_buffer_bytes % frame_bytes) != 0U))
  {
    return FR_INVALID_PARAMETER;
  }

  if (audio_sd_ctx.state == AUDIO_SD_STATE_PLAYING)
  {
    return FR_LOCKED;
  }

  Audio_SD_StopAndCleanup();
  memset(&audio_sd_ctx, 0, sizeof(audio_sd_ctx));

  UINT half_bytes = work_buffer_bytes / 2U;
  const UINT max_bytes = AUDIO_DMA_MAX_TRANSFER_SAMPLES * sizeof(uint16_t);
  if (half_bytes > max_bytes)
  {
    half_bytes = max_bytes;
  }
  const UINT align = (512U > frame_bytes) ? 512U : frame_bytes;
  half_bytes -= (half_bytes % align);
  if (half_bytes == 0U)
  {
    return FR_INVALID_PARAMETER;
  }

  audio_sd_ctx.buf0 = work_buffer;
  audio_sd_ctx.buf1 = work_buffer + half_bytes;
  audio_sd_ctx.read_buf = audio_sd_ctx.buf1;
  audio_sd_ctx.half_bytes = half_bytes;
  audio_sd_ctx.frame_bytes = frame_bytes;
  audio_sd_ctx.state = AUDIO_SD_STATE_IDLE;

  char file_path[AUDIO_FILE_PATH_MAX];
  int written = ((directory != NULL) && (directory[0] != '\0')) ?
                snprintf(file_path, sizeof(file_path), "%s/%s", directory, file_name) :
                snprintf(file_path, sizeof(file_path), "%s", file_name);
  if ((written <= 0) || ((size_t)written >= sizeof(file_path)))
  {
    return FR_INVALID_NAME;
  }

  FRESULT res = f_mount(&SDFatFS, (TCHAR const*)SDPath, 1);
  if (res != FR_OK)
  {
    printf("audio(fs): mount failed (%d)\r\n", (int)res);
    audio_sd_ctx.last_res = res;
    audio_sd_ctx.state = AUDIO_SD_STATE_ERROR;
    return res;
  }
  audio_sd_ctx.mounted = 1U;

  res = f_open(&audio_sd_ctx.file, file_path, FA_READ);
  if (res != FR_OK)
  {
    printf("audio(fs): open %s failed (%d)\r\n", file_path, (int)res);
    audio_sd_ctx.last_res = res;
    audio_sd_ctx.state = AUDIO_SD_STATE_ERROR;
    Audio_SD_StopAndCleanup();
    return res;
  }
  audio_sd_ctx.file_opened = 1U;

  UINT bytes0 = 0U;
  UINT bytes1 = 0U;
  res = f_read(&audio_sd_ctx.file, audio_sd_ctx.buf0, half_bytes, &bytes0);
  if (res != FR_OK)
  {
    printf("audio(fs): first read failed (%d)\r\n", (int)res);
    audio_sd_ctx.last_res = res;
    audio_sd_ctx.state = AUDIO_SD_STATE_ERROR;
    Audio_SD_StopAndCleanup();
    return res;
  }
  bytes0 -= (bytes0 % frame_bytes);
  if (bytes0 > 0U)
  {
    Audio_ApplyVolumeToBuffer((int16_t *)audio_sd_ctx.buf0,
                              bytes0 / sizeof(int16_t),
                              audio_volume_q15);
  }
  if (bytes0 == 0U)
  {
    printf("audio(fs): file empty\r\n");
    audio_sd_ctx.state = AUDIO_SD_STATE_DONE;
    Audio_SD_StopAndCleanup();
    return FR_OK;
  }

  res = f_read(&audio_sd_ctx.file, audio_sd_ctx.buf1, half_bytes, &bytes1);
  if (res != FR_OK)
  {
    printf("audio(fs): second read failed (%d)\r\n", (int)res);
    audio_sd_ctx.last_res = res;
    audio_sd_ctx.state = AUDIO_SD_STATE_ERROR;
    Audio_SD_StopAndCleanup();
    return res;
  }
  bytes1 -= (bytes1 % frame_bytes);
  if (bytes1 > 0U)
  {
    Audio_ApplyVolumeToBuffer((int16_t *)audio_sd_ctx.buf1,
                              bytes1 / sizeof(int16_t),
                              audio_volume_q15);
  }
  audio_sd_ctx.bytes_next = bytes1;

  audio_stream_from_sd = 1U;
  audio_sd_dma_done = 0U;
  HAL_StatusTypeDef hal = HAL_I2S_Transmit_DMA(&hi2s2,
                                               (uint16_t *)audio_sd_ctx.buf0,
                                               (uint16_t)(bytes0 / sizeof(uint16_t)));
  if (hal != HAL_OK)
  {
    printf("audio(fs): start DMA failed (%ld)\r\n", (long)hal);
    audio_sd_ctx.last_res = FR_INT_ERR;
    audio_sd_ctx.last_hal = hal;
    audio_sd_ctx.state = AUDIO_SD_STATE_ERROR;
    Audio_SD_StopAndCleanup();
    return FR_INT_ERR;
  }

  Spectrum_UpdateFromBuffer((const int16_t *)audio_sd_ctx.buf0,
                            bytes0 / sizeof(uint16_t));

  audio_sd_ctx.state = AUDIO_SD_STATE_PLAYING;
  return FR_OK;
}

static void Audio_SD_ProcessPlayback(void)
{
  if (audio_sd_ctx.state != AUDIO_SD_STATE_PLAYING)
  {
    return;
  }

  if (audio_sd_dma_done == 0U)
  {
    return;
  }
  audio_sd_dma_done = 0U;

  if (audio_sd_ctx.bytes_next == 0U)
  {
    HAL_I2S_DMAStop(&hi2s2);
    audio_sd_ctx.state = AUDIO_SD_STATE_DONE;
    Audio_SD_StopAndCleanup();
    return;
  }

  uint8_t *play_buf = audio_sd_ctx.read_buf;
  UINT play_bytes = audio_sd_ctx.bytes_next;
  audio_sd_ctx.read_buf = (play_buf == audio_sd_ctx.buf0) ? audio_sd_ctx.buf1 : audio_sd_ctx.buf0;

  HAL_StatusTypeDef hal = HAL_I2S_Transmit_DMA(&hi2s2,
                                               (uint16_t *)play_buf,
                                               (uint16_t)(play_bytes / sizeof(uint16_t)));
  if (hal != HAL_OK)
  {
    printf("audio(fs): DMA start failed (%ld)\r\n", (long)hal);
    audio_sd_ctx.last_res = FR_INT_ERR;
    audio_sd_ctx.last_hal = hal;
    audio_sd_ctx.state = AUDIO_SD_STATE_ERROR;
    Audio_SD_StopAndCleanup();
    return;
  }

  Spectrum_UpdateFromBuffer((const int16_t *)play_buf, play_bytes / sizeof(uint16_t));

  UINT bytes_read = 0U;
  FRESULT res = f_read(&audio_sd_ctx.file, audio_sd_ctx.read_buf, audio_sd_ctx.half_bytes, &bytes_read);
  if (res != FR_OK)
  {
    printf("audio(fs): read failed (%d)\r\n", (int)res);
    audio_sd_ctx.last_res = res;
    audio_sd_ctx.state = AUDIO_SD_STATE_ERROR;
    Audio_SD_StopAndCleanup();
    HAL_I2S_DMAStop(&hi2s2);
    return;
  }
  bytes_read -= (bytes_read % audio_sd_ctx.frame_bytes);
  if (bytes_read > 0U)
  {
    Audio_ApplyVolumeToBuffer((int16_t *)audio_sd_ctx.read_buf,
                              bytes_read / sizeof(int16_t),
                              audio_volume_q15);
  }
  audio_sd_ctx.bytes_next = bytes_read;
}

static void Spectrum_Init(void)
{
  const float bin_width_hz = ((float)AUDIO_SAMPLE_RATE_HZ * 0.5f) / (float)SPECTRUM_BIN_COUNT;

  for (uint32_t i = 0U; i < SPECTRUM_BIN_COUNT; ++i)
  {
    float freq = bin_width_hz * (float)(i + 1U);
    float omega = 2.0f * (float)M_PI * freq / (float)AUDIO_SAMPLE_RATE_HZ;
    spectrum_coeff[i] = 2.0f * cosf(omega);
    spectrum_smooth[i] = 0.0f;
    spectrum_levels[i] = 0U;
    spectrum_draw_levels[i] = 0U;
  }

  spectrum_peak_avg = SPECTRUM_PEAK_INIT;
  spectrum_last_draw_ms = HAL_GetTick();
}

static void Spectrum_UpdateFromBuffer(const int16_t *samples, size_t sample_count)
{
  if ((samples == NULL) || (sample_count < AUDIO_NUM_CHANNELS))
  {
    return;
  }

  size_t frames = sample_count / AUDIO_NUM_CHANNELS;
  size_t window = (frames < (size_t)SPECTRUM_WINDOW_FRAMES) ? frames : (size_t)SPECTRUM_WINDOW_FRAMES;
  if (window < 8U)
  {
    return;
  }

  for (uint32_t bin = 0U; bin < SPECTRUM_BIN_COUNT; ++bin)
  {
    float coeff = spectrum_coeff[bin];
    float q0 = 0.0f;
    float q1 = 0.0f;
    float q2 = 0.0f;

    for (size_t n = 0U; n < window; ++n)
    {
      float x = (float)samples[n * AUDIO_NUM_CHANNELS];
      q0 = (coeff * q1) - q2 + x;
      q2 = q1;
      q1 = q0;
    }

    float power = (q1 * q1) + (q2 * q2) - (coeff * q1 * q2);
    if (power < 0.0f)
    {
      power = 0.0f;
    }
    float mag = sqrtf(power) / (float)window;

    spectrum_peak_avg = (SPECTRUM_DECAY_ALPHA * spectrum_peak_avg) +
                        ((1.0f - SPECTRUM_DECAY_ALPHA) * mag);
    float scaled = (mag * (float)SPECTRUM_MAX_HEIGHT) / (spectrum_peak_avg + 1.0f);
    if (scaled < 0.0f)
    {
      scaled = 0.0f;
    }
    if (scaled > (float)SPECTRUM_MAX_HEIGHT)
    {
      scaled = (float)SPECTRUM_MAX_HEIGHT;
    }

    float smooth = (1.0f - SPECTRUM_SMOOTH_ALPHA) * spectrum_smooth[bin] +
                   (SPECTRUM_SMOOTH_ALPHA * scaled);
    spectrum_smooth[bin] = smooth;
    spectrum_levels[bin] = (uint16_t)smooth;
  }
}

static void Spectrum_DrawIfDue(void)
{
  uint32_t now = HAL_GetTick();
  if ((now - spectrum_last_draw_ms) < SPECTRUM_DRAW_INTERVAL_MS)
  {
    return;
  }
  spectrum_last_draw_ms = now;

  const uint16_t bar_width = ST7789_WIDTH / SPECTRUM_BIN_COUNT;
  uint16_t usable_width = (bar_width > SPECTRUM_BAR_GAP) ? (bar_width - SPECTRUM_BAR_GAP) : bar_width;
  if (usable_width == 0U)
  {
    usable_width = 1U;
  }
  const uint16_t y_base = ST7789_HIGHT - 1U;

  for (uint32_t i = 0U; i < SPECTRUM_BIN_COUNT; ++i)
  {
    uint16_t h = spectrum_levels[i];
    uint16_t x0 = (uint16_t)(i * bar_width);
    x0 = (uint16_t)(x0 + ((bar_width - usable_width) / 2U));
    uint16_t x1 = x0 + usable_width - 1U;
    if (x1 >= ST7789_WIDTH)
    {
      x1 = ST7789_WIDTH - 1U;
    }

    /* Clear column then draw new bar */
    ST7789_FillRect(x0, 0U, x1, y_base, BLACK);

    if (h > 0U)
    {
      uint16_t y0 = (h >= ST7789_HIGHT) ? 0U : (uint16_t)(ST7789_HIGHT - h);
      uint16_t color = Spectrum_ColorForBin(i);
      ST7789_FillRect(x0, y0, x1, y_base, color);
    }

    spectrum_draw_levels[i] = h;
  }
}

static uint16_t Spectrum_LerpColor(uint16_t c0, uint16_t c1, float t)
{
  if (t < 0.0f) t = 0.0f;
  if (t > 1.0f) t = 1.0f;

  float r0 = (float)((c0 >> 11) & 0x1FU);
  float g0 = (float)((c0 >> 5) & 0x3FU);
  float b0 = (float)(c0 & 0x1FU);

  float r1 = (float)((c1 >> 11) & 0x1FU);
  float g1 = (float)((c1 >> 5) & 0x3FU);
  float b1 = (float)(c1 & 0x1FU);

  float r = r0 + (r1 - r0) * t;
  float g = g0 + (g1 - g0) * t;
  float b = b0 + (b1 - b0) * t;

  uint16_t ri = (uint16_t)(r + 0.5f);
  uint16_t gi = (uint16_t)(g + 0.5f);
  uint16_t bi = (uint16_t)(b + 0.5f);

  if (ri > 0x1F) ri = 0x1F;
  if (gi > 0x3F) gi = 0x3F;
  if (bi > 0x1F) bi = 0x1F;

  return (uint16_t)((ri << 11) | (gi << 5) | bi);
}

static uint16_t Spectrum_ColorForBin(uint32_t index)
{
  const size_t stop_count = sizeof(spectrum_gradient) / sizeof(spectrum_gradient[0]);
  if (stop_count == 0U)
  {
    return WHITE;
  }

  float t = (SPECTRUM_BIN_COUNT > 1U) ?
            ((float)index / (float)(SPECTRUM_BIN_COUNT - 1U)) : 0.0f;
  if (t <= spectrum_gradient[0].pos)
  {
    return spectrum_gradient[0].color;
  }
  if (t >= spectrum_gradient[stop_count - 1U].pos)
  {
    return spectrum_gradient[stop_count - 1U].color;
  }

  for (size_t i = 0U; i < (stop_count - 1U); ++i)
  {
    float p0 = spectrum_gradient[i].pos;
    float p1 = spectrum_gradient[i + 1U].pos;
    if ((t >= p0) && (t <= p1))
    {
      float local_t = (p1 > p0) ? ((t - p0) / (p1 - p0)) : 0.0f;
      return Spectrum_LerpColor(spectrum_gradient[i].color,
                                spectrum_gradient[i + 1U].color,
                                local_t);
    }
  }

  return spectrum_gradient[stop_count - 1U].color;
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_SPI1_Init();
  MX_SDIO_SD_Init();
  MX_FATFS_Init();
  MX_CRC_Init();
  MX_I2S2_Init();
  /* USER CODE BEGIN 2 */
  HAL_Delay(200);
  ST7789_Init();
  ST7789_Clear(BLACK);
  Spectrum_Init();

  const uint32_t speed_test_kbytes = 512U; /* Adjust size to profile different transfers */
  FATFS_SpeedTest(speed_test_kbytes);

  FRESULT audio_res = Audio_SD_StartPlayback(AUDIO_FILE_DIRECTORY,
                                             AUDIO_FILE_NAME,
                                             (uint8_t *)audio_sd_buffer,
                                             (UINT)sizeof(audio_sd_buffer));
  if (audio_res != FR_OK)
  {
    printf("audio(fs): playback failed (%d)\r\n", (int)audio_res);
  }
  uint8_t audio_playback_reported = 0U;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    Audio_SD_ProcessPlayback();
    Spectrum_DrawIfDue();

    if (audio_playback_reported == 0U)
    {
      if (audio_sd_ctx.state == AUDIO_SD_STATE_DONE)
      {
        printf("audio(fs): playback done\r\n");
        audio_playback_reported = 1U;
      }
      else if (audio_sd_ctx.state == AUDIO_SD_STATE_ERROR)
      {
        printf("audio(fs): playback error (%d, hal %ld)\r\n",
               (int)audio_sd_ctx.last_res, (long)audio_sd_ctx.last_hal);
        audio_playback_reported = 1U;
      }
    }
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 84;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */


static void FATFS_SpeedTest(uint32_t kilobytes)
{
  const char speed_file[] = "sd_speed.bin";
  FRESULT res;
  UINT bytes_io = 0U;
  uint32_t total_bytes = 0U;
  uint32_t remaining = 0U;
  uint32_t start_tick = 0U;
  uint32_t elapsed_ms = 0U;
  uint32_t speed_kbs = 0U;
  uint8_t is_mounted = 0U;
  uint8_t file_opened = 0U;
  uint8_t file_created = 0U;
  static uint8_t transfer_buffer[FATFS_SPEED_TEST_BUFFER_SIZE];

  if (kilobytes == 0U)
  {
    printf("fatfs speed test: size parameter must be > 0\r\n");
    return;
  }

  if (kilobytes > (UINT32_MAX / 1024U))
  {
    printf("fatfs speed test: size too large\r\n");
    return;
  }

  total_bytes = kilobytes * 1024U;

  for (uint32_t i = 0U; i < sizeof(transfer_buffer); ++i)
  {
    transfer_buffer[i] = (uint8_t)(i & 0xFFU);
  }

  printf("fatfs speed test: mounting %s\r\n", SDPath);
  res = f_mount(&SDFatFS, (TCHAR const*)SDPath, 1);
  if (res != FR_OK)
  {
    printf("fatfs speed test: mount failed (%d)\r\n", (int)res);
    return;
  }
  is_mounted = 1U;

  res = f_open(&SDFile, speed_file, FA_CREATE_ALWAYS | FA_WRITE);
  if (res != FR_OK)
  {
    printf("fatfs speed test: open for write failed (%d)\r\n", (int)res);
    goto cleanup;
  }
  file_opened = 1U;
  file_created = 1U;

  printf("fatfs speed test: writing %lu KB\r\n", (unsigned long)kilobytes);
  remaining = total_bytes;
  start_tick = HAL_GetTick();
  while (remaining > 0U)
  {
    uint32_t chunk = (remaining > sizeof(transfer_buffer)) ? (uint32_t)sizeof(transfer_buffer) : remaining;
    res = f_write(&SDFile, transfer_buffer, chunk, &bytes_io);
    if ((res != FR_OK) || (bytes_io != chunk))
    {
      printf("fatfs speed test: write error (%d)\r\n", (int)res);
      goto cleanup;
    }
    remaining -= chunk;
  }

  res = f_sync(&SDFile);
  if (res != FR_OK)
  {
    printf("fatfs speed test: sync failed (%d)\r\n", (int)res);
    goto cleanup;
  }
  elapsed_ms = HAL_GetTick() - start_tick;
  speed_kbs = (elapsed_ms > 0U) ? (uint32_t)(((uint64_t)total_bytes * 1000ULL) / ((uint64_t)elapsed_ms * 1024ULL)) : 0U;
  printf("fatfs speed test: write done in %lu ms (%lu KB/s)\r\n",
         (unsigned long)elapsed_ms, (unsigned long)speed_kbs);

  f_close(&SDFile);
  file_opened = 0U;

  res = f_open(&SDFile, speed_file, FA_READ);
  if (res != FR_OK)
  {
    printf("fatfs speed test: open for read failed (%d)\r\n", (int)res);
    goto cleanup;
  }
  file_opened = 1U;

  printf("fatfs speed test: reading %lu KB\r\n", (unsigned long)kilobytes);
  remaining = total_bytes;
  start_tick = HAL_GetTick();
  while (remaining > 0U)
  {
    uint32_t chunk = (remaining > sizeof(transfer_buffer)) ? (uint32_t)sizeof(transfer_buffer) : remaining;
    res = f_read(&SDFile, transfer_buffer, chunk, &bytes_io);
    if (res != FR_OK)
    {
      printf("fatfs speed test: read error (%d)\r\n", (int)res);
      goto cleanup;
    }
    if (bytes_io == 0U)
    {
      printf("fatfs speed test: unexpected end of file\r\n");
      goto cleanup;
    }
    remaining -= bytes_io;
  }
  elapsed_ms = HAL_GetTick() - start_tick;
  speed_kbs = (elapsed_ms > 0U) ? (uint32_t)(((uint64_t)total_bytes * 1000ULL) / ((uint64_t)elapsed_ms * 1024ULL)) : 0U;
  printf("fatfs speed test: read done in %lu ms (%lu KB/s)\r\n",
         (unsigned long)elapsed_ms, (unsigned long)speed_kbs);

  f_close(&SDFile);
  file_opened = 0U;

  printf("fatfs speed test: removing %s\r\n", speed_file);
  res = f_unlink(speed_file);
  if (res == FR_OK)
  {
    file_created = 0U;
  }
  else
  {
    printf("fatfs speed test: remove failed (%d)\r\n", (int)res);
  }

cleanup:
  if (file_opened)
  {
    f_close(&SDFile);
  }

  if (is_mounted && file_created)
  {
    FRESULT unlink_res = f_unlink(speed_file);
    if (unlink_res != FR_OK)
    {
      printf("fatfs speed test: cleanup remove %s failed (%d)\r\n", speed_file, (int)unlink_res);
    }
  }

  if (is_mounted)
  {
    f_mount(NULL, (TCHAR const*)SDPath, 0);
    printf("fatfs speed test: unmounted\r\n");
  }
}





void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s)
{
  if (hi2s->Instance != hi2s2.Instance)
  {
    return;
  }

  if (audio_stream_from_sd)
  {
    audio_sd_dma_done = 1U;
    return;
  }

  if (audio_ctrl.samples_remaining == 0U)
  {
    audio_ctrl.state = AUDIO_PLAYBACK_STATE_DONE;
    return;
  }

  if (Audio_StartNextChunk() != HAL_OK)
  {
    audio_ctrl.state = AUDIO_PLAYBACK_STATE_ERROR;
    Error_Handler();
  }
}

void HAL_I2S_ErrorCallback(I2S_HandleTypeDef *hi2s)
{
  if (hi2s->Instance != hi2s2.Instance)
  {
    return;
  }

  if (audio_stream_from_sd)
  {
    audio_sd_ctx.state = AUDIO_SD_STATE_ERROR;
    audio_sd_ctx.last_hal = HAL_ERROR;
    audio_sd_ctx.last_res = FR_INT_ERR;
    audio_sd_dma_done = 1U;
    Audio_SD_StopAndCleanup();
    return;
  }

  audio_ctrl.state = AUDIO_PLAYBACK_STATE_ERROR;
  Error_Handler();
}


/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
```

</details>

<!-- BSP_DRIVERS_END -->
