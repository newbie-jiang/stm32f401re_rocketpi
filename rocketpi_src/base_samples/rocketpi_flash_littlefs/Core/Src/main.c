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
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>

#include "lfs.h"
#include "lfs_port.h"
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
static lfs_t g_lfs;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
static void LittleFS_Test(void);
static void LittleFS_Log(const char *msg);
static void LittleFS_LogError(const char *msg, int err);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static void LittleFS_Log(const char *msg)
{
  if (msg == NULL) {
    return;
  }

  HAL_UART_Transmit(&huart2, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
}

static void LittleFS_LogError(const char *msg, int err)
{
  char buffer[96];
  int len = snprintf(buffer, sizeof(buffer),
                     "%s (err=%d)\r\n", msg, err);
  if ((len > 0) && (len < (int)sizeof(buffer))) {
    LittleFS_Log(buffer);
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
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  LittleFS_Test();

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
static void LittleFS_Test(void)
{
  struct lfs_file file;
  char read_buffer[64] = {0};
  const char *path = "flash.txt";
  const char *payload = "LittleFS STM32F401 Flash Demo\r\n";

  int err = lfs_port_mount(&g_lfs);
  if (err != LFS_ERR_OK) {
    LittleFS_LogError("LittleFS: initial mount failed", err);
    LittleFS_Log("LittleFS: formatting flash...\r\n");
    err = lfs_port_format(&g_lfs);
    if (err == LFS_ERR_OK) {
      LittleFS_Log("LittleFS: format done\r\n");
      err = lfs_port_mount(&g_lfs);
    } else {
      LittleFS_LogError("LittleFS: format failed", err);
    }
  }

  if (err != LFS_ERR_OK) {
    LittleFS_LogError("LittleFS: mount failed", err);
    return;
  }

  err = lfs_file_open(&g_lfs, &file, path,
                      LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC);
  if (err < 0) {
    LittleFS_LogError("LittleFS: open for write failed", err);
    lfs_port_unmount(&g_lfs);
    return;
  }

  lfs_ssize_t written = lfs_file_write(&g_lfs, &file, payload, strlen(payload));
  lfs_file_close(&g_lfs, &file);

  if (written < 0) {
    LittleFS_LogError("LittleFS: write failed", (int)written);
    lfs_port_unmount(&g_lfs);
    return;
  }

  err = lfs_file_open(&g_lfs, &file, path, LFS_O_RDONLY);
  if (err < 0) {
    LittleFS_LogError("LittleFS: open for read failed", err);
    lfs_port_unmount(&g_lfs);
    return;
  }

  lfs_ssize_t read = lfs_file_read(&g_lfs, &file, read_buffer,
                                   sizeof(read_buffer) - 1U);
  lfs_file_close(&g_lfs, &file);

  if (read < 0) {
    LittleFS_LogError("LittleFS: read failed", (int)read);
    lfs_port_unmount(&g_lfs);
    return;
  }

  read_buffer[read] = '\0';

  char log_buffer[128];
  int len = snprintf(log_buffer, sizeof(log_buffer),
                     "LittleFS OK (%ld bytes): %s\r\n",
                     (long)read, read_buffer);
  if ((len > 0) && (len < (int)sizeof(log_buffer))) {
    LittleFS_Log(log_buffer);
  }

  lfs_port_unmount(&g_lfs);
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
