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
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

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
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
static void EnterStandbyMode(void);
static void SetAllLeds(GPIO_PinState state);
static void EnableAllPeripheralClocks(void);
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
  /* USER CODE BEGIN 2 */
  /* 
	 * 打开所有可用外设时钟，让普通工作模式的功耗尽可能大 
	 * 这里的功耗大不了多少，只有当实际配置并让这些外设工作电流才会尽可能的大
	*/
  EnableAllPeripheralClocks();

  /* 判断当前是否是从 Standby 模式唤醒 */
  uint8_t woke_from_standby = (__HAL_PWR_GET_FLAG(PWR_FLAG_SB) != RESET);

  /* 处理唤醒后的流程 */
  if (woke_from_standby)  
  {
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB);
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
    SetAllLeds(GPIO_PIN_RESET); //打开LED
  }
  else
  {
    SetAllLeds(GPIO_PIN_SET); //关闭LED
  }

  /* 延迟 3 秒后再进入 Standby，方便观察与退出 */
  HAL_Delay(3000);
  EnterStandbyMode();

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
static void EnterStandbyMode(void)
{
  /* 进入 Standby 前先清除旧的唤醒标志并重新使能 PA0 为唤醒源 */
  HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN1);
  __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
  HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1);
  HAL_PWR_EnterSTANDBYMode();
}

static void SetAllLeds(GPIO_PinState state)
{
  /* 同步控制三色 LED，便于统一显示状态 */
  HAL_GPIO_WritePin(LED_B_GPIO_Port, LED_B_Pin, state);
  HAL_GPIO_WritePin(LED_G_GPIO_Port, LED_G_Pin, state);
  HAL_GPIO_WritePin(LED_R_GPIO_Port, LED_R_Pin, state);
}

static void EnableAllPeripheralClocks(void)
{
  /* AHB1 总线：GPIO、DMA 等 */
#if defined(__HAL_RCC_GPIOA_CLK_ENABLE)
  __HAL_RCC_GPIOA_CLK_ENABLE();
#endif
#if defined(__HAL_RCC_GPIOB_CLK_ENABLE)
  __HAL_RCC_GPIOB_CLK_ENABLE();
#endif
#if defined(__HAL_RCC_GPIOC_CLK_ENABLE)
  __HAL_RCC_GPIOC_CLK_ENABLE();
#endif
#if defined(__HAL_RCC_GPIOD_CLK_ENABLE)
  __HAL_RCC_GPIOD_CLK_ENABLE();
#endif
#if defined(__HAL_RCC_GPIOE_CLK_ENABLE)
  __HAL_RCC_GPIOE_CLK_ENABLE();
#endif
#if defined(__HAL_RCC_GPIOH_CLK_ENABLE)
  __HAL_RCC_GPIOH_CLK_ENABLE();
#endif
#if defined(__HAL_RCC_DMA1_CLK_ENABLE)
  __HAL_RCC_DMA1_CLK_ENABLE();
#endif
#if defined(__HAL_RCC_DMA2_CLK_ENABLE)
  __HAL_RCC_DMA2_CLK_ENABLE();
#endif
#if defined(__HAL_RCC_CRC_CLK_ENABLE)
  __HAL_RCC_CRC_CLK_ENABLE();
#endif
#if defined(__HAL_RCC_BKPSRAM_CLK_ENABLE)
  __HAL_RCC_BKPSRAM_CLK_ENABLE();
#endif

  /* AHB2 总线：USB OTG、RNG 等 */
#if defined(__HAL_RCC_USB_OTG_FS_CLK_ENABLE)
  __HAL_RCC_USB_OTG_FS_CLK_ENABLE();
#endif
#if defined(__HAL_RCC_RNG_CLK_ENABLE)
  __HAL_RCC_RNG_CLK_ENABLE();
#endif

  /* APB1 总线：定时器、串口、I2C 等 */
#if defined(__HAL_RCC_TIM2_CLK_ENABLE)
  __HAL_RCC_TIM2_CLK_ENABLE();
#endif
#if defined(__HAL_RCC_TIM3_CLK_ENABLE)
  __HAL_RCC_TIM3_CLK_ENABLE();
#endif
#if defined(__HAL_RCC_TIM4_CLK_ENABLE)
  __HAL_RCC_TIM4_CLK_ENABLE();
#endif
#if defined(__HAL_RCC_TIM5_CLK_ENABLE)
  __HAL_RCC_TIM5_CLK_ENABLE();
#endif
#if defined(__HAL_RCC_TIM6_CLK_ENABLE)
  __HAL_RCC_TIM6_CLK_ENABLE();
#endif
#if defined(__HAL_RCC_TIM7_CLK_ENABLE)
  __HAL_RCC_TIM7_CLK_ENABLE();
#endif
#if defined(__HAL_RCC_SPI2_CLK_ENABLE)
  __HAL_RCC_SPI2_CLK_ENABLE();
#endif
#if defined(__HAL_RCC_SPI3_CLK_ENABLE)
  __HAL_RCC_SPI3_CLK_ENABLE();
#endif
#if defined(__HAL_RCC_I2C1_CLK_ENABLE)
  __HAL_RCC_I2C1_CLK_ENABLE();
#endif
#if defined(__HAL_RCC_I2C2_CLK_ENABLE)
  __HAL_RCC_I2C2_CLK_ENABLE();
#endif
#if defined(__HAL_RCC_I2C3_CLK_ENABLE)
  __HAL_RCC_I2C3_CLK_ENABLE();
#endif
#if defined(__HAL_RCC_USART2_CLK_ENABLE)
  __HAL_RCC_USART2_CLK_ENABLE();
#endif
#if defined(__HAL_RCC_USART3_CLK_ENABLE)
  __HAL_RCC_USART3_CLK_ENABLE();
#endif
#if defined(__HAL_RCC_UART4_CLK_ENABLE)
  __HAL_RCC_UART4_CLK_ENABLE();
#endif
#if defined(__HAL_RCC_UART5_CLK_ENABLE)
  __HAL_RCC_UART5_CLK_ENABLE();
#endif
#if defined(__HAL_RCC_PWR_CLK_ENABLE)
  __HAL_RCC_PWR_CLK_ENABLE();
#endif
#if defined(__HAL_RCC_DAC_CLK_ENABLE)
  __HAL_RCC_DAC_CLK_ENABLE();
#endif

  /* APB2 总线：高速外设 */
#if defined(__HAL_RCC_TIM1_CLK_ENABLE)
  __HAL_RCC_TIM1_CLK_ENABLE();
#endif
#if defined(__HAL_RCC_TIM9_CLK_ENABLE)
  __HAL_RCC_TIM9_CLK_ENABLE();
#endif
#if defined(__HAL_RCC_TIM10_CLK_ENABLE)
  __HAL_RCC_TIM10_CLK_ENABLE();
#endif
#if defined(__HAL_RCC_TIM11_CLK_ENABLE)
  __HAL_RCC_TIM11_CLK_ENABLE();
#endif
#if defined(__HAL_RCC_ADC1_CLK_ENABLE)
  __HAL_RCC_ADC1_CLK_ENABLE();
#endif
#if defined(__HAL_RCC_SDIO_CLK_ENABLE)
  __HAL_RCC_SDIO_CLK_ENABLE();
#endif
#if defined(__HAL_RCC_SPI1_CLK_ENABLE)
  __HAL_RCC_SPI1_CLK_ENABLE();
#endif
#if defined(__HAL_RCC_SPI4_CLK_ENABLE)
  __HAL_RCC_SPI4_CLK_ENABLE();
#endif
#if defined(__HAL_RCC_SYSCFG_CLK_ENABLE)
  __HAL_RCC_SYSCFG_CLK_ENABLE();
#endif
#if defined(__HAL_RCC_USART1_CLK_ENABLE)
  __HAL_RCC_USART1_CLK_ENABLE();
#endif
#if defined(__HAL_RCC_USART6_CLK_ENABLE)
  __HAL_RCC_USART6_CLK_ENABLE();
#endif
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
