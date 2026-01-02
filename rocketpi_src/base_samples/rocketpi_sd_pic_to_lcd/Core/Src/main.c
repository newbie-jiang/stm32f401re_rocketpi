/* USER CODE BEGIN Header */
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
#include "adc.h"
#include "dma.h"
#include "fatfs.h"
#include "sdio.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <ctype.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "st7789.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#ifndef LCD_FRAME_NAME_MAX
#define LCD_FRAME_NAME_MAX 13U
#endif

typedef struct
{
  char name[LCD_FRAME_NAME_MAX];
} LCD_FrameEntry;
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

#define LCD_FRAME_DIRECTORY        "PIC_BIN"
#define LCD_FRAME_EXTENSION        ".bin"
#define LCD_FRAME_DELAY_MS         25U
#define LCD_FRAME_PATH_MAX         64U
#define LCD_FRAME_LINE_BYTES       (ST7789_WIDTH * 2U)
#define LCD_FRAME_ROWS             ST7789_HIGHT
#define LCD_FRAME_MAX_COUNT        600U
#define LCD_FRAME_BATCH_ROWS       120U
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
static void fatfs_test(void);
static void fatfs_speed_test(uint32_t kilobytes);
static void fatfs_playback_frames_optimized(void);
static void lcd_log_error(const char *fmt, ...);
static FRESULT lcd_build_frame_path(char *frame_path, size_t frame_path_size, const char *directory, const char *file_name);
static FRESULT lcd_draw_frame_batched(const char *directory,
                                    const char *file_name,
                                    uint8_t *batch_buffer,
                                    UINT batch_rows);
static FRESULT lcd_populate_frame_list(const char *directory,
                                     LCD_FrameEntry *entries,
                                     UINT max_entries,
                                     UINT *out_count);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_ADC1_Init();
  MX_SDIO_SD_Init();
  MX_FATFS_Init();
  /* USER CODE BEGIN 2 */
  HAL_Delay(200);
  fatfs_test();
  const uint32_t speed_test_kbytes = 512U; /* Adjust size to profile different transfers */
  fatfs_speed_test(speed_test_kbytes);

  ST7789_Init();
  ST7789_Clear(BLACK);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    fatfs_playback_frames_optimized();
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
static bool lcd_is_bin_frame(const char *name)
{
  if ((name == NULL) || (name[0] == '\0') || (name[0] == '.'))
  {
    return false;
  }

  const size_t ext_len = strlen(LCD_FRAME_EXTENSION);
  const size_t name_len = strlen(name);
  if (name_len < ext_len)
  {
    return false;
  }

  const char *ext = name + name_len - ext_len;
  for (size_t i = 0U; i < ext_len; ++i)
  {
    const char lhs = (char)tolower((unsigned char)ext[i]);
    const char rhs = (char)tolower((unsigned char)LCD_FRAME_EXTENSION[i]);
    if (lhs != rhs)
    {
      return false;
    }
  }

  return true;
}

static void lcd_log_error(const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  vprintf(fmt, args);
  va_end(args);
  HAL_Delay(500U);
}

static FRESULT lcd_build_frame_path(char *frame_path, size_t frame_path_size, const char *directory, const char *file_name)
{
  int written = 0;
  if ((directory != NULL) && (directory[0] != '\0'))
  {
    written = snprintf(frame_path, frame_path_size, "%s/%s", directory, file_name);
  }
  else
  {
    written = snprintf(frame_path, frame_path_size, "%s", file_name);
  }

  if ((written <= 0) || ((size_t)written >= frame_path_size))
  {
    return FR_INVALID_NAME;
  }

  return FR_OK;
}

static FRESULT lcd_draw_frame_batched(const char *directory,
                                    const char *file_name,
                                    uint8_t *batch_buffer,
                                    UINT batch_rows)
{
  if ((batch_rows == 0U) || (batch_buffer == NULL))
  {
    return FR_INVALID_PARAMETER;
  }

  char frame_path[LCD_FRAME_PATH_MAX];
  FRESULT res = lcd_build_frame_path(frame_path, sizeof(frame_path), directory, file_name);
  if (res != FR_OK)
  {
    return res;
  }

  FIL frame_file;
  res = f_open(&frame_file, frame_path, FA_READ);
  if (res != FR_OK)
  {
    return res;
  }

  uint32_t row = 0U;
  while (row < LCD_FRAME_ROWS)
  {
    UINT rows_this = batch_rows;
    if (rows_this > (LCD_FRAME_ROWS - row))
    {
      rows_this = (UINT)(LCD_FRAME_ROWS - row);
    }

    UINT bytes_request = rows_this * LCD_FRAME_LINE_BYTES;
    UINT bytes_read = 0U;
    res = f_read(&frame_file, batch_buffer, bytes_request, &bytes_read);
    if ((res != FR_OK) || (bytes_read != bytes_request))
    {
      if (res == FR_OK)
      {
        res = FR_INT_ERR;
      }
      break;
    }

    ST7789_DrawBitmap(0U, (uint16_t)row, ST7789_WIDTH, rows_this, batch_buffer);
    row += rows_this;
  }

  f_close(&frame_file);
  return res;
}

static FRESULT lcd_populate_frame_list(const char *directory,
                                     LCD_FrameEntry *entries,
                                     UINT max_entries,
                                     UINT *out_count)
{
  if ((entries == NULL) || (out_count == NULL))
  {
    return FR_INVALID_PARAMETER;
  }

  DIR dir;
  FILINFO fno;
  FRESULT res = f_opendir(&dir, directory);
  if (res != FR_OK)
  {
    return res;
  }

  UINT count = 0U;
  while (1)
  {
    res = f_readdir(&dir, &fno);
    if (res != FR_OK)
    {
      break;
    }

    if (fno.fname[0] == '\0')
    {
      break;
    }

    if ((fno.fattrib & AM_DIR) != 0U)
    {
      continue;
    }

    if (!lcd_is_bin_frame(fno.fname))
    {
      continue;
    }

    if (count < max_entries)
    {
      strncpy(entries[count].name, fno.fname, LCD_FRAME_NAME_MAX - 1U);
      entries[count].name[LCD_FRAME_NAME_MAX - 1U] = '\0';
      count++;
    }
  }

  f_closedir(&dir);
  *out_count = count;
  return res;
}

static void fatfs_playback_frames_optimized(void)
{
  static LCD_FrameEntry s_frame_cache[LCD_FRAME_MAX_COUNT];
  static UINT s_frame_count = 0U;
  static uint8_t s_cache_valid = 0U;
  static uint8_t batch_buffer[LCD_FRAME_LINE_BYTES * LCD_FRAME_BATCH_ROWS];
  uint8_t is_mounted = 0U;
  FRESULT res;
  const char *dir_path = (LCD_FRAME_DIRECTORY[0] == '\0') ? "" : LCD_FRAME_DIRECTORY;

  res = f_mount(&SDFatFS, (TCHAR const*)SDPath, 1);
  if (res != FR_OK)
  {
    lcd_log_error("video(opt): mount failed (%d)\r\n", (int)res);
    return;
  }
  is_mounted = 1U;

  if (!s_cache_valid)
  {
    res = lcd_populate_frame_list(dir_path, s_frame_cache, LCD_FRAME_MAX_COUNT, &s_frame_count);
    if (res != FR_OK)
    {
      if (LCD_FRAME_DIRECTORY[0] == '\0')
      {
        lcd_log_error("video(opt): scan root failed (%d)\r\n", (int)res);
      }
      else
      {
        lcd_log_error("video(opt): scan %s failed (%d)\r\n", LCD_FRAME_DIRECTORY, (int)res);
      }
      goto cleanup;
    }

    if (s_frame_count == 0U)
    {
      if (LCD_FRAME_DIRECTORY[0] == '\0')
      {
        lcd_log_error("video(opt): no %s frames in root\r\n", LCD_FRAME_EXTENSION);
      }
      else
      {
        lcd_log_error("video(opt): no %s frames inside %s\r\n", LCD_FRAME_EXTENSION, LCD_FRAME_DIRECTORY);
      }
      goto cleanup;
    }

    if (LCD_FRAME_DIRECTORY[0] == '\0')
    {
      printf("video(opt): cached %lu frames from SD root\r\n", (unsigned long)s_frame_count);
    }
    else
    {
      printf("video(opt): cached %lu frames from /%s\r\n",
             (unsigned long)s_frame_count, LCD_FRAME_DIRECTORY);
    }
    s_cache_valid = 1U;
  }

  if (LCD_FRAME_DIRECTORY[0] == '\0')
  {
    printf("video(opt): streaming cached frames from SD root\r\n");
  }
  else
  {
    printf("video(opt): streaming cached frames from /%s\r\n", LCD_FRAME_DIRECTORY);
  }

  uint32_t cycle_start = HAL_GetTick();
  uint32_t frames_drawn = 0U;
  for (UINT idx = 0U; idx < s_frame_count; ++idx)
  {
    res = lcd_draw_frame_batched(dir_path, s_frame_cache[idx].name, batch_buffer, LCD_FRAME_BATCH_ROWS);
    if (res != FR_OK)
    {
      if (LCD_FRAME_DIRECTORY[0] == '\0')
      {
        lcd_log_error("video(opt): draw %s failed (%d)\r\n", s_frame_cache[idx].name, (int)res);
      }
      else
      {
        lcd_log_error("video(opt): draw %s/%s failed (%d)\r\n",
                     LCD_FRAME_DIRECTORY, s_frame_cache[idx].name, (int)res);
      }
      continue;
    }
    frames_drawn++;
    HAL_Delay(LCD_FRAME_DELAY_MS);
  }

  uint32_t elapsed_ms = HAL_GetTick() - cycle_start;
  if ((frames_drawn > 0U) && (elapsed_ms > 0U))
  {
    float fps = ((float)frames_drawn * 1000.0f) / (float)elapsed_ms;
    printf("video(opt): cycle %lu frames in %lums (%.2f FPS)\r\n",
           (unsigned long)frames_drawn,
           (unsigned long)elapsed_ms,
           fps);
  }

cleanup:
  if (is_mounted)
  {
    f_mount(NULL, (TCHAR const*)SDPath, 0);
  }
}

static void fatfs_speed_test(uint32_t kilobytes)
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

static void fatfs_test(void)
{
  const char test_file[] = "rocketpi.txt";
  const char test_payload[] = "RocketPi FATFS SDIO write/read demo.\r\n";
  char read_buffer[sizeof(test_payload)] = {0};
  const UINT payload_len = (UINT)strlen(test_payload);
  UINT bytes_written = 0;
  UINT bytes_read = 0;
  FRESULT res;
  uint8_t is_mounted = 0;
  uint8_t file_opened = 0;

  printf("fatfs: mounting %s\r\n", SDPath);
  res = f_mount(&SDFatFS, (TCHAR const*)SDPath, 1);
  if (res == FR_OK)
  {
    is_mounted = 1;
  }
  else if (res == FR_NO_FILESYSTEM)
  {
#if (_USE_MKFS == 1)
#if (FATFS_MKFS_BUFFER_SIZE < 1024U)
#error "FATFS_MKFS_BUFFER_SIZE must be >= 1024 bytes"
#endif
    static uint8_t mkfs_work[FATFS_MKFS_BUFFER_SIZE];

    printf("fatfs: no filesystem, formatting...\r\n");
    res = f_mkfs((TCHAR const*)SDPath, FM_ANY, 0, mkfs_work, sizeof(mkfs_work));
    if (res != FR_OK)
    {
      printf("fatfs: format failed (%d)\r\n", (int)res);
      return;
    }

    printf("fatfs: format complete, remounting\r\n");
    res = f_mount(&SDFatFS, (TCHAR const*)SDPath, 1);
    if (res != FR_OK)
    {
      printf("fatfs: mount after format failed (%d)\r\n", (int)res);
      return;
    }
    is_mounted = 1;
#else
    printf("fatfs: no filesystem and mkfs disabled\r\n");
    return;
#endif
  }
  else
  {
    printf("fatfs: mount failed (%d)\r\n", (int)res);
    return;
  }

  printf("fatfs: creating %s\r\n", test_file);
  res = f_open(&SDFile, test_file, FA_CREATE_ALWAYS | FA_WRITE);
  if (res != FR_OK)
  {
    printf("fatfs: open for write failed (%d)\r\n", (int)res);
    goto cleanup;
  }
  file_opened = 1;

  res = f_write(&SDFile, test_payload, payload_len, &bytes_written);
  if ((res != FR_OK) || (bytes_written != payload_len))
  {
    printf("fatfs: write failed (%d)\r\n", (int)res);
    goto cleanup;
  }
  printf("fatfs: wrote %lu bytes\r\n", (unsigned long)bytes_written);

  f_close(&SDFile);
  file_opened = 0;

  res = f_open(&SDFile, test_file, FA_READ);
  if (res != FR_OK)
  {
    printf("fatfs: open for read failed (%d)\r\n", (int)res);
    goto cleanup;
  }
  file_opened = 1;

  memset(read_buffer, 0, sizeof(read_buffer));
  res = f_read(&SDFile, read_buffer, sizeof(read_buffer) - 1, &bytes_read);
  if (res != FR_OK)
  {
    printf("fatfs: read failed (%d)\r\n", (int)res);
    goto cleanup;
  }
  printf("fatfs: read %lu bytes\r\n", (unsigned long)bytes_read);

  f_close(&SDFile);
  file_opened = 0;

  if ((bytes_read == bytes_written) && (strncmp(read_buffer, test_payload, payload_len) == 0))
  {
    printf("fatfs: verification OK\r\n");
    printf("fatfs: content: %s", read_buffer);
  }
  else
  {
    printf("fatfs: verification failed\r\n");
  }

cleanup:
  if (file_opened)
  {
    f_close(&SDFile);
  }

  if (is_mounted)
  {
    f_mount(NULL, (TCHAR const*)SDPath, 0);
    printf("fatfs: unmounted\r\n");
  }
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
