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
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdbool.h>

#define LFS_NOR_FLASH_TOTAL_SIZE (8UL * 1024UL * 1024UL) /* W25Q64 = 8 MiB */
#include "lfs_nor_flash_port.h"
#include "lfs_port.h"
#include "lfs_flash_port.h"
#include "debug_driver.h"
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
static void LittleFS_Test(bool use_nor_flash);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

typedef struct {
  int (*format)(lfs_t *lfs);
  int (*mount)(lfs_t *lfs);
  int (*unmount)(lfs_t *lfs);
  const char *label;
} lfs_storage_ops_t;

static void LittleFS_Test(bool use_nor_flash)
{
  struct lfs_file file;
  char read_buffer[64] = {0};
  const char *path = use_nor_flash ? "nor_flash.txt" : "int_flash.txt";
  const char *payload = use_nor_flash
                          ? "LittleFS NOR Flash Demo\r\n"
                          : "LittleFS Internal Flash Demo\r\n";

  const lfs_storage_ops_t ops = use_nor_flash
                                  ? (lfs_storage_ops_t){lfs_nor_flash_port_format, lfs_nor_flash_port_mount, lfs_nor_flash_port_unmount, "NOR"}
                                  : (lfs_storage_ops_t){lfs_flash_port_format, lfs_flash_port_mount, lfs_flash_port_unmount, "INT"};

  int err = ops.mount(&g_lfs);
  if (err != LFS_ERR_OK) {
    uart_printf("[%s] mount failed (%d), formatting...\r\n", ops.label, err);
    err = ops.format(&g_lfs);
    if (err == LFS_ERR_OK) {
      uart_printf("[%s] format done\r\n", ops.label);
      err = ops.mount(&g_lfs);
    } else {
      uart_printf("[%s] format failed (%d)\r\n", ops.label, err);
    }
  }

  if (err != LFS_ERR_OK) {
    uart_printf("[%s] mount failed (%d)\r\n", ops.label, err);
    return;
  }

  err = lfs_file_open(&g_lfs, &file, path,
                      LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC);
  if (err < 0) {
    uart_printf("[%s] open for write failed (%d)\r\n", ops.label, err);
    ops.unmount(&g_lfs);
    return;
  }

  lfs_ssize_t written = lfs_file_write(&g_lfs, &file, payload, strlen(payload));
  lfs_file_close(&g_lfs, &file);

  if (written < 0) {
    uart_printf("[%s] write failed (%ld)\r\n", ops.label, (long)written);
    ops.unmount(&g_lfs);
    return;
  }

  err = lfs_file_open(&g_lfs, &file, path, LFS_O_RDONLY);
  if (err < 0) {
    uart_printf("[%s] open for read failed (%d)\r\n", ops.label, err);
    ops.unmount(&g_lfs);
    return;
  }

  lfs_ssize_t read = lfs_file_read(&g_lfs, &file, read_buffer,
                                   sizeof(read_buffer) - 1U);
  lfs_file_close(&g_lfs, &file);

  if (read < 0) {
    uart_printf("[%s] read failed (%ld)\r\n", ops.label, (long)read);
    ops.unmount(&g_lfs);
    return;
  }

  read_buffer[read] = '\0';
  uart_printf("[%s] LittleFS OK (%ld bytes): %s\r\n", ops.label, (long)read, read_buffer);

  ops.unmount(&g_lfs);
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
  MX_SPI2_Init();
  /* USER CODE BEGIN 2 */
	HAL_Delay(500);
  LittleFS_Test(false); /* internal Flash */
  LittleFS_Test(true);  /* external NOR Flash*/
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
