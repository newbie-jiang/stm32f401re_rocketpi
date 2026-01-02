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
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define UART_DMA_BUFFER_SIZE 128U  /* 单次回显缓存大小 */
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
static HAL_StatusTypeDef uart_restart_rx_dma(void);
static void uart_send_string(const char *msg);
static void uart_send_banner(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static uint8_t g_uart_rx_buffer[UART_DMA_BUFFER_SIZE];
static uint8_t g_uart_tx_buffer[UART_DMA_BUFFER_SIZE];

/* 重新启动串口空闲中断 DMA 接收，始终准备下一帧数据 */
static HAL_StatusTypeDef uart_restart_rx_dma(void)
{
  HAL_StatusTypeDef status = HAL_UARTEx_ReceiveToIdle_DMA(&huart2,
                                                          g_uart_rx_buffer,
                                                          UART_DMA_BUFFER_SIZE);
  if (status == HAL_OK)
  {
    if (huart2.hdmarx != NULL)
    {
      __HAL_DMA_DISABLE_IT(huart2.hdmarx, DMA_IT_HT);
    }
  }
  return status;
}

/* 阻塞方式发送字符串，主要用于提示和回显 */
static void uart_send_string(const char *msg)
{
  if ((msg == NULL) || (*msg == '\0'))
  {
    return;
  }
  const size_t length = strlen(msg);
  if (length == 0U)
  {
    return;
  }
  if (HAL_UART_Transmit(&huart2,
                        (uint8_t *)msg,
                        (uint16_t)length,
                        HAL_MAX_DELAY) != HAL_OK)
  {
    Error_Handler();
  }
}

/* 打印启动提示，告知用户当前示例用途 */
static void uart_send_banner(void)
{
  uart_send_string("\r\nRocketPi UART echo ready.\r\n");
  uart_send_string("Type any text and it will be echoed back.\r\n> ");
}

/* 串口接收到一帧数据后立即回显，并重新打开 DMA 接收 */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
  if ((huart != &huart2) || (Size == 0U))
  {
    return;
  }

  const uint16_t copy_size = (Size <= UART_DMA_BUFFER_SIZE) ? Size : UART_DMA_BUFFER_SIZE;
  memcpy(g_uart_tx_buffer, g_uart_rx_buffer, copy_size);

  if (uart_restart_rx_dma() != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_UART_Transmit(&huart2, g_uart_tx_buffer, copy_size, HAL_MAX_DELAY) != HAL_OK)
  {
    Error_Handler();
  }
}

/* 若发生串口错误则重新启动 DMA 接收，保证示例持续运行 */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
  if (huart != &huart2)
  {
    return;
  }

  if (uart_restart_rx_dma() != HAL_OK)
  {
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
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  /* 上电后先启动 DMA 接收，再输出提示信息 */
  if (uart_restart_rx_dma() != HAL_OK)
  {
    Error_Handler();
  }
  uart_send_banner();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    HAL_Delay(1);
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
