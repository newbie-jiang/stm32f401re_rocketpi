



MAX98357 io口介绍

以 **MAX98357A** 模块为例（典型 3W I2S D 类功放，单声道）：

- **BCLK**
   Bit Clock，比特时钟。
  - 由 **I2S 主机（MCU）输出**
  - 在 STM32 里一般叫 `I2S_SCK` 或 `CK`
  - 每一位数据来一个时钟
- **DIN**
   Data IN，串行音频数据输入。
  - 接 MCU 的 **I2S 发送数据脚**，通常叫 `I2S_SD` / `SDO` / `SD_TX`
  - 音频样本就是从这里进芯片
- **LRC**
   Left/Right Clock，也叫 **LRCK / WS / Word Select**。
  - 一般一半时间代表左声道，一半代表右声道
  - 由 MCU 的 `I2S_WS` / `LRCK` 输出
- **GAIN**
   增益 / 声道选择脚（上电时采样）。
  - 拉到 **不同电平/电阻组合** 可以选择不同输出增益（比如 3dB、6dB、9dB…）
  - 还可以指定芯片取左/右/左右混合哪个声道
  - 若是现成模块，通常板上已经接好默认增益，你可以先**悬空或接板上默认焊盘**使用；要精细调增益再按数据手册来接电阻
- **SD**
   Shutdown，使能脚。
  - **高电平：芯片工作**
  - 低电平：关断（静音+省电）
  - 很多简单用法直接用一个 10k 上拉到 3.3V/5V，让功放一直“开机”；
  - 也可以接 MCU GPIO，用来做“静音/省电控制”

另外模块上还有：

- **VIN/5V**：功放电源输入（一般 3V–5.5V 工作，具体看模块标注）
- **GND**：地
- **SPK+ / SPK-**：接喇叭（4–8Ω 单声道）



下面的这个模块只需要控制  BCLK  DIN LRC,GAIN和SD悬空

![image-20251122002611792](https://cloud.rocketpi.club/cloud/image-20251122002611792.png)

```
MCU I2S_SCK → BCLK

MCU I2S_WS/LRCK → LRC

MCU I2S_SD(发送) → DIN
```

**PB13 – I2S2_CK** → 接 MAX98357 的 **BCLK**

**PB12 – I2S2_WS** → 接 MAX98357 的 **LRC（LRCK/WS）**

**PB15 – I2S2_SD** → 接 MAX98357 的 **DIN**

## Edge TTS 工具（v1.0 / v2.0）

仓库根目录提供两个前端工具，均用于快速把 `edge-tts` 语音转成 I2S 需要的 PCM 数据：

| 目录 | 说明 | 默认参数 | 适用场景 |
| ---- | ---- | ---- | ---- |
| `tts_web` | v1.0，保持最初配置 | 16 kHz / 16-bit / 双声道 | 兼容历史脚本 |
| `tts_web_v2` | v2.0，新增采样率/声道/Voice 选择以及 `/api/config` | 默认与 v1 相同（16 kHz / 双声道 / `zh-CN-XiaoxiaoNeural`） | 推荐用于通用音频导入、不同功放/录音需求 |

### 使用

```bash
cd tts_web_v2         # 或 tts_web 获取旧版
npm install
npm start             # 启动后访问 http://localhost:5173
```

v2.0 支持以下优化：

- 采样率：8k/11.025k/16k/22.05k/32k/44.1k/48k/96k；
- 声道：单声道或双声道；
- Edge 语音：内置常用中文/英文 voice，可自定义；
- Output：C 头文件（限制 400 KB）或二进制 PCM；
- `/api/config` 可用于脚本化读取可选项。

前端生成 `audio.h` 时会自动插入 `audio_track[]`、采样率、声道等宏定义，直接拷贝到 STM32 工程即可。

<!-- BSP_DRIVERS_START -->
<!-- BSP_DRIVERS_HASH:1a7f0959d9b979c1ea35ec33bf47790674d360c7ed2bab60646082f00e70e8e0 -->
## 驱动以及测试代码

<details>
<summary>Core/Src/main.c</summary>

```c
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
#include "dma.h"
#include "i2s.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "audio.h"
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

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define AUDIO_DMA_MAX_TRANSFER_SAMPLES 65535U

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
static volatile audio_playback_ctrl_t audio_ctrl = {0};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
static HAL_StatusTypeDef Audio_StartNextChunk(void);
static void Audio_BeginPlayback(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static HAL_StatusTypeDef Audio_StartNextChunk(void)
{
  if (audio_ctrl.samples_remaining == 0U)
  {
    audio_ctrl.state = AUDIO_PLAYBACK_STATE_DONE;
    return HAL_OK;
  }

  uint32_t chunk = (audio_ctrl.samples_remaining > AUDIO_DMA_MAX_TRANSFER_SAMPLES) ?
                   AUDIO_DMA_MAX_TRANSFER_SAMPLES :
                   audio_ctrl.samples_remaining;

  const uint16_t *chunk_ptr = (const uint16_t *)&audio_track[audio_ctrl.next_index];
  HAL_StatusTypeDef status = HAL_I2S_Transmit_DMA(&hi2s2,
                                                  (uint16_t *)chunk_ptr,
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
  MX_I2S2_Init();
  /* USER CODE BEGIN 2 */
  Audio_BeginPlayback();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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
  RCC_OscInitStruct.PLL.PLLQ = 4;
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
void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s)
{
  if (hi2s->Instance != hi2s2.Instance)
  {
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

<details>
<summary>Core/Src/stm32f4xx_it.c</summary>

```c
/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f4xx_it.c
  * @brief   Interrupt Service Routines.
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
#include "stm32f4xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern DMA_HandleTypeDef hdma_spi2_tx;
extern I2S_HandleTypeDef hi2s2;
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M4 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
   while (1)
  {
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */

  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
  * @brief This function handles Pre-fetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */

  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */

  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
  /* USER CODE BEGIN SVCall_IRQn 0 */

  /* USER CODE END SVCall_IRQn 0 */
  /* USER CODE BEGIN SVCall_IRQn 1 */

  /* USER CODE END SVCall_IRQn 1 */
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
  /* USER CODE BEGIN PendSV_IRQn 0 */

  /* USER CODE END PendSV_IRQn 0 */
  /* USER CODE BEGIN PendSV_IRQn 1 */

  /* USER CODE END PendSV_IRQn 1 */
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  /* USER CODE BEGIN SysTick_IRQn 1 */

  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32F4xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f4xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles DMA1 stream4 global interrupt.
  */
void DMA1_Stream4_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Stream4_IRQn 0 */

  /* USER CODE END DMA1_Stream4_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_spi2_tx);
  /* USER CODE BEGIN DMA1_Stream4_IRQn 1 */

  /* USER CODE END DMA1_Stream4_IRQn 1 */
}

/**
  * @brief This function handles SPI2 global interrupt.
  */
void SPI2_IRQHandler(void)
{
  /* USER CODE BEGIN SPI2_IRQn 0 */

  /* USER CODE END SPI2_IRQn 0 */
  HAL_I2S_IRQHandler(&hi2s2);
  /* USER CODE BEGIN SPI2_IRQn 1 */

  /* USER CODE END SPI2_IRQn 1 */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
```

</details>

<!-- BSP_DRIVERS_END -->
