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
