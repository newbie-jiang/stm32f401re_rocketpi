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
#include "crc.h"
#include "dma.h"
#include "fatfs.h"
#include "sdio.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

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
static void FATFS_Test(void);
static void FATFS_SpeedTest(uint32_t kilobytes);

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
  MX_SDIO_SD_Init();
  MX_CRC_Init();
  MX_FATFS_Init();
  /* USER CODE BEGIN 2 */
  HAL_Delay(200);
  FATFS_Test();
  const uint32_t speed_test_kbytes = 512U; 
  FATFS_SpeedTest(speed_test_kbytes);

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

static void FATFS_Test(void)
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
