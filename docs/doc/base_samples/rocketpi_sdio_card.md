## 效果展示

![image-20251119000347354](https://cloud.rocketpi.club/cloud/image-20251119000347354.png)

## 功能说明

面向 RocketPI STM32F401RE 开发板的  单线 **SDIO MicroSD卡读写 演示工程**。主要特性：

- 实现MicroSD卡的读写测试，可使用阻塞方式或DMA方式，只需一个宏定义即可选择

## 硬件连接

- SDIO_CLK
- SDIO_CMD
- SDIO_D0
- SDIO_CD
- 电源3.3V

## CubeMX配置

![image-20251119004637362](https://cloud.rocketpi.club/cloud/image-20251119004637362.png)

### DMA配置

发送和接收配置，注意内存到外设  /   外设到内存

![image-20251119004731858](https://cloud.rocketpi.club/cloud/image-20251119004731858.png)

### 开启全局中断，并确认使能

![image-20251119004930253](https://cloud.rocketpi.club/cloud/image-20251119004930253.png)

![image-20251119005101176](https://cloud.rocketpi.club/cloud/image-20251119005101176.png)

生成代码时将堆栈设置大一些 都设置为 0x1000





### 是否开启DMA   由文件 driver_sdcard_test.c   ---》    SDCARD_TEST_USE_DMA 宏定义决定

<!-- BSP_DRIVERS_START -->
<!-- BSP_DRIVERS_HASH:1233c5ab8c7bc178050316c0ac977170e56fef86910dd8980738453921356d59 -->
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
#include "crc.h"
#include "dma.h"
#include "sdio.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "driver_sdcard_test.h"

/* sdio 4bit bug ：https://community.st.com/t5/stm32cubemx-mcus/sdio-interface-not-working-in-4bits-with-stm32f4-firmware/m-p/591803#M26033 */

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
  /* USER CODE BEGIN 2 */
  HAL_Delay(500);
  sdcard_test_run(0x00,1024);
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
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
```

</details>

<details>
<summary>bsp/sdio_card/driver_sdcard_test.c</summary>

```c
/**
 * @file      driver_sdcard_test.c
 * @brief     driver sdcard test source file
 * @version   1.0.0
 */

#include "driver_sdcard_test.h"

#include <stdio.h>

#define SDCARD_TEST_BUFFER_BYTES           (4U * 1024U)
#define SDCARD_TEST_TIMEOUT_MS             5000U
#define SDCARD_TEST_PROGRESS_STEP_BLOCKS   65536U

/**
 * Set to 1 to use HAL SDIO DMA APIs instead of blocking transfers.
 * DMA mode still waits for completion via sdcard_test_wait_ready().
 */
#ifndef SDCARD_TEST_USE_DMA
#define SDCARD_TEST_USE_DMA                1U
#endif

static const char *sdcard_test_card_type_to_string(uint32_t type);
static const char *sdcard_test_card_version_to_string(uint32_t version);
static uint8_t sdcard_test_wait_ready(uint32_t timeout_ms);
static void sdcard_test_fill_pattern(uint8_t *buffer, uint32_t length, uint32_t seed);
static uint8_t sdcard_test_verify_pattern(const uint8_t *buffer, uint32_t length, uint32_t seed);
static void sdcard_test_print_info(const HAL_SD_CardInfoTypeDef *info);
static uint8_t sdcard_test_prepare_card(HAL_SD_CardInfoTypeDef *info);
static uint8_t sdcard_test_process_range(const HAL_SD_CardInfoTypeDef *info, uint32_t block_addr, uint32_t block_count);
static HAL_StatusTypeDef sdcard_test_write_blocks(uint8_t *buffer, uint32_t block_addr, uint32_t block_count);
static HAL_StatusTypeDef sdcard_test_read_blocks(uint8_t *buffer, uint32_t block_addr, uint32_t block_count);

static const char *sdcard_test_card_type_to_string(uint32_t type)
{
    switch (type)
    {
        case CARD_SDSC:
            return "SDSC";
        case CARD_SDHC_SDXC:
            return "SDHC/SDXC";
        case CARD_SECURED:
            return "Secured";
        default:
            return "Unknown";
    }
}

static const char *sdcard_test_card_version_to_string(uint32_t version)
{
    switch (version)
    {
        case CARD_V1_X:
            return "1.x";
        case CARD_V2_X:
            return "2.x+";
        default:
            return "Unknown";
    }
}

static uint8_t sdcard_test_wait_ready(uint32_t timeout_ms)
{
    const uint32_t start = HAL_GetTick();
    HAL_SD_CardStateTypeDef state;

    do
    {
        state = HAL_SD_GetCardState(&hsd);
        if (state == HAL_SD_CARD_TRANSFER)
        {
            return 0U;
        }
        if (state == HAL_SD_CARD_ERROR)
        {
            printf("sdcard: card reports error state\r\n");

            return 1U;
        }
        HAL_Delay(1U);
    }
    while ((HAL_GetTick() - start) < timeout_ms);

    printf("sdcard: wait ready timeout, last state=0x%08lX\r\n", (unsigned long)state);

    return 1U;
}

static HAL_StatusTypeDef sdcard_test_write_blocks(uint8_t *buffer, uint32_t block_addr, uint32_t block_count)
{
#if SDCARD_TEST_USE_DMA
    return HAL_SD_WriteBlocks_DMA(&hsd, buffer, block_addr, block_count);
#else
    return HAL_SD_WriteBlocks(&hsd, buffer, block_addr, block_count, HAL_MAX_DELAY);
#endif
}

static HAL_StatusTypeDef sdcard_test_read_blocks(uint8_t *buffer, uint32_t block_addr, uint32_t block_count)
{
#if SDCARD_TEST_USE_DMA
    return HAL_SD_ReadBlocks_DMA(&hsd, buffer, block_addr, block_count);
#else
    return HAL_SD_ReadBlocks(&hsd, buffer, block_addr, block_count, HAL_MAX_DELAY);
#endif
}

static void sdcard_test_fill_pattern(uint8_t *buffer, uint32_t length, uint32_t seed)
{
    for (uint32_t i = 0U; i < length; ++i)
    {
        buffer[i] = (uint8_t)((i + seed) & 0xFFU);
    }
}

static uint8_t sdcard_test_verify_pattern(const uint8_t *buffer, uint32_t length, uint32_t seed)
{
    for (uint32_t i = 0U; i < length; ++i)
    {
        const uint8_t expected = (uint8_t)((i + seed) & 0xFFU);

        if (buffer[i] != expected)
        {
            printf("sdcard: data mismatch at byte %lu (expected=0x%02X, read=0x%02X)\r\n",
                   (unsigned long)i,
                   expected,
                   buffer[i]);

            return 1U;
        }
    }

    return 0U;
}

static void sdcard_test_print_info(const HAL_SD_CardInfoTypeDef *info)
{
    const uint64_t capacity_bytes = (uint64_t)info->LogBlockNbr * (uint64_t)info->LogBlockSize;
    const uint32_t capacity_mb = (uint32_t)(capacity_bytes / (1024ULL * 1024ULL));

    printf("sdcard: type=%s, version=%s, class=%lu\r\n",
           sdcard_test_card_type_to_string(info->CardType),
           sdcard_test_card_version_to_string(info->CardVersion),
           (unsigned long)info->Class);
    printf("sdcard: RCA=%lu, logical blocks=%lu, logical block size=%lu bytes\r\n",
           (unsigned long)info->RelCardAdd,
           (unsigned long)info->LogBlockNbr,
           (unsigned long)info->LogBlockSize);
    printf("sdcard: capacity=%lu MB\r\n", (unsigned long)capacity_mb);
}

static uint8_t sdcard_test_prepare_card(HAL_SD_CardInfoTypeDef *info)
{
    HAL_StatusTypeDef status;

    status = HAL_SD_InitCard(&hsd);
    if (status != HAL_OK)
    {
        printf("sdcard: HAL_SD_InitCard failed (err=0x%08lX)\r\n", (unsigned long)hsd.ErrorCode);

        return 1U;
    }

    status = HAL_SD_ConfigWideBusOperation(&hsd, SDIO_BUS_WIDE_1B);
    if (status != HAL_OK)
    {
        printf("sdcard: HAL_SD_ConfigWideBusOperation failed (err=0x%08lX)\r\n", (unsigned long)hsd.ErrorCode);

        return 1U;
    }

    status = HAL_SD_GetCardInfo(&hsd, info);
    if (status != HAL_OK)
    {
        printf("sdcard: HAL_SD_GetCardInfo failed (err=0x%08lX)\r\n", (unsigned long)hsd.ErrorCode);

        return 1U;
    }

    return 0U;
}

static uint8_t sdcard_test_process_range(const HAL_SD_CardInfoTypeDef *info, uint32_t block_addr, uint32_t block_count)
{
    const uint32_t block_size = info->LogBlockSize;
    uint8_t buffer[SDCARD_TEST_BUFFER_BYTES];
    uint32_t remaining_blocks;
    uint32_t processed_blocks = 0U;
    uint32_t total_blocks;
    uint32_t max_chunk_blocks;
    uint32_t last_report = 0U;

    if (block_size == 0U)
    {
        printf("sdcard: invalid block size\r\n");

        return 1U;
    }

    max_chunk_blocks = SDCARD_TEST_BUFFER_BYTES / block_size;
    if (max_chunk_blocks == 0U)
    {
        printf("sdcard: configured buffer %lu bytes is smaller than a block (%lu bytes)\r\n",
               (unsigned long)SDCARD_TEST_BUFFER_BYTES,
               (unsigned long)block_size);

        return 1U;
    }

    if (block_addr >= info->LogBlockNbr)
    {
        printf("sdcard: start block %lu is outside the card (max %lu)\r\n",
               (unsigned long)block_addr,
               (unsigned long)(info->LogBlockNbr - 1U));

        return 1U;
    }

    if ((block_count == 0U) || (block_addr + block_count > info->LogBlockNbr))
    {
        remaining_blocks = info->LogBlockNbr - block_addr;
    }
    else
    {
        remaining_blocks = block_count;
    }

    if (remaining_blocks == 0U)
    {
        printf("sdcard: nothing to test\r\n");

        return 1U;
    }

    total_blocks = remaining_blocks;

    printf("sdcard: verifying %lu blocks from address %lu (chunk=%lu blocks)\r\n",
           (unsigned long)total_blocks,
           (unsigned long)block_addr,
           (unsigned long)max_chunk_blocks);

    while (remaining_blocks > 0U)
    {
        const uint32_t chunk_blocks = (remaining_blocks < max_chunk_blocks) ? remaining_blocks : max_chunk_blocks;
        const uint32_t chunk_bytes = chunk_blocks * block_size;
        const uint32_t current_block = block_addr + processed_blocks;
        HAL_StatusTypeDef status;

        sdcard_test_fill_pattern(buffer, chunk_bytes, current_block);

        status = sdcard_test_write_blocks(buffer, current_block, chunk_blocks);
        if (status != HAL_OK)
        {
            printf("sdcard: write failed at block %lu (err=0x%08lX)\r\n",
                   (unsigned long)current_block,
                   (unsigned long)hsd.ErrorCode);

            return 1U;
        }

        if (sdcard_test_wait_ready(SDCARD_TEST_TIMEOUT_MS) != 0U)
        {
            printf("sdcard: card not ready after write at block %lu\r\n", (unsigned long)current_block);

            return 1U;
        }

        status = sdcard_test_read_blocks(buffer, current_block, chunk_blocks);
        if (status != HAL_OK)
        {
            printf("sdcard: read failed at block %lu (err=0x%08lX)\r\n",
                   (unsigned long)current_block,
                   (unsigned long)hsd.ErrorCode);

            return 1U;
        }

        if (sdcard_test_wait_ready(SDCARD_TEST_TIMEOUT_MS) != 0U)
        {
            printf("sdcard: card not ready after read at block %lu\r\n", (unsigned long)current_block);

            return 1U;
        }

        if (sdcard_test_verify_pattern(buffer, chunk_bytes, current_block) != 0U)
        {
            return 1U;
        }

        processed_blocks += chunk_blocks;
        remaining_blocks -= chunk_blocks;

        if ((processed_blocks - last_report) >= SDCARD_TEST_PROGRESS_STEP_BLOCKS || remaining_blocks == 0U)
        {
            printf("sdcard: progress %lu/%lu blocks\r\n",
                   (unsigned long)processed_blocks,
                   (unsigned long)total_blocks);
            last_report = processed_blocks;
        }
    }

    return 0U;
}

uint8_t sdcard_test_card_info(void)
{
    HAL_SD_CardInfoTypeDef info;

    printf("sdcard: querying card information...\r\n");
    if (sdcard_test_prepare_card(&info) != 0U)
    {
        return 1U;
    }

    sdcard_test_print_info(&info);

    return 0U;
}

uint8_t sdcard_test_block_read_write(uint32_t block_addr, uint32_t block_count)
{
    HAL_SD_CardInfoTypeDef info;

    if (sdcard_test_prepare_card(&info) != 0U)
    {
        return 1U;
    }

    if (sdcard_test_process_range(&info, block_addr, block_count) != 0U)
    {
        return 1U;
    }

    printf("sdcard: block read/write test completed\r\n");

    return 0U;
}

uint8_t sdcard_test_run(uint32_t block_addr, uint32_t block_count)
{
    HAL_SD_CardInfoTypeDef info;

    printf("sdcard: starting full test sequence\r\n");
    if (sdcard_test_prepare_card(&info) != 0U)
    {
        return 1U;
    }

    sdcard_test_print_info(&info);

    if (sdcard_test_process_range(&info, block_addr, block_count) != 0U)
    {
        return 1U;
    }

    printf("sdcard: test completed successfully\r\n");

    return 0U;
}
```

</details>

<details>
<summary>bsp/sdio_card/driver_sdcard_test.h</summary>

```c
/**
 * Copyright (c) 2025
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * @file      driver_sdcard_test.h
 * @brief     driver sdcard test header file
 * @version   1.0.0
 * @date      2025-10-19
 */

#ifndef DRIVER_SDCARD_TEST_H
#define DRIVER_SDCARD_TEST_H

#include "sdio.h"

#ifdef __cplusplus
extern "C"{
#endif

/**
 * @brief default block address used by sdcard_test_run()
 */
#define SDCARD_TEST_DEFAULT_BLOCK_ADDR   0U

/**
 * @brief default block count used by sdcard_test_run()
 * @note  set to 0 to test the whole card from the start block
 */
#define SDCARD_TEST_DEFAULT_BLOCK_COUNT  0U

/**
 * @brief     print card information retrieved from HAL_SD_GetCardInfo()
 * @return    status code
 *            - 0 success
 *            - 1 operation failed
 */
uint8_t sdcard_test_card_info(void);

/**
 * @brief     perform blocking read/write verification on the SD card
 * @param[in] block_addr start block address
 * @param[in] block_count number of blocks to transfer, 0 means test to the end of the card
 * @return    status code
 *            - 0 success
 *            - 1 read/write failed
 */
uint8_t sdcard_test_block_read_write(uint32_t block_addr, uint32_t block_count);

/**
 * @brief     run the default SD card test sequence (info + read/write)
 * @param[in] block_addr start block address
 * @param[in] block_count number of blocks to transfer, 0 means test to the end of the card
 * @return    status code
 *            - 0 success
 *            - 1 test failed
 */
uint8_t sdcard_test_run(uint32_t block_addr, uint32_t block_count);

#ifdef __cplusplus
}
#endif

#endif
```

</details>

<!-- BSP_DRIVERS_END -->
