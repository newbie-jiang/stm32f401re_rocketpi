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
<!-- BSP_DRIVERS_HASH:dd41c426c9d73ac1fee0b8727aabd57e8cfb84a604690964c9059ced76195ede -->
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
<summary>bsp/debug/debug_driver.c</summary>

```c
#include "debug_driver.h"

#include "usart.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

/**
 * @file debug_driver.c
 * @brief Retarget low-level stdio functions to USART2 for debugging output/input.
 * @author rocket
 * @copyright Copyright (c) 2025 rocket. Authorized use only.
 */

#ifdef __GNUC__  // GCC
/**
 * @brief Retarget newlib write syscall to the USART debug port.
 * @param file Logical file descriptor supplied by the C library (unused).
 * @param ptr Buffer containing the data to be transmitted.
 * @param len Number of bytes to be transmitted.
 * @return Number of bytes written.
 */
int _write(int file, char *ptr, int len)
{
    (void)file;
    HAL_UART_Transmit(&huart2, (uint8_t *)ptr, len, HAL_MAX_DELAY);
    return len;
}

/**
 * @brief Retarget newlib read syscall to the USART debug port.
 * @param file Logical file descriptor supplied by the C library (unused).
 * @param ptr Destination buffer.
 * @param len Number of bytes to be read.
 * @return Number of bytes read.
 */
int _read(int file, char *ptr, int len)
{
    (void)file;
    HAL_UART_Receive(&huart2, (uint8_t *)ptr, len, HAL_MAX_DELAY);
    return len;
}
#elif defined(__ARMCC_VERSION)  // Keil
/**
 * @brief Retarget fputc to the USART debug port for Arm Compiler/Keil.
 * @param ch Character to transmit.
 * @param f Ignored file handle.
 * @return The transmitted character.
 */
int fputc(int ch, FILE *f)
{
    (void)f;
    uint8_t data = (uint8_t)ch;
    HAL_UART_Transmit(&huart2, &data, 1U, HAL_MAX_DELAY);
    return ch;
}

/**
 * @brief Retarget fgetc to the USART debug port for Arm Compiler/Keil.
 * @param f Ignored file handle.
 * @return The received character.
 */
int fgetc(FILE *f)
{
    (void)f;
    uint8_t ch;
    HAL_UART_Receive(&huart2, &ch, 1U, HAL_MAX_DELAY);
    return (int)ch;
}

#ifndef __MICROLIB
  /* Disable semihosting when using the standard C library (non-Microlib). */
  #pragma import(__use_no_semihosting)

  struct __FILE
  {
      int handle;
  };

  FILE __stdout;
  FILE __stdin;

  /**
   * @brief Stub exit handler so Keil does not fall back to semihosting.
   * @param x Exit code (ignored).
   */
  void _sys_exit(int x)
  {
      (void)x;
  }
#endif
#else
    #error "Unsupported compiler"
#endif

/* ========= Configuration section ========= */
#ifndef UART_LOG_INSTANCE
#define UART_LOG_INSTANCE  huart2      // Replace with the UART handle you want to use
#endif

#ifndef UART_LOG_TIMEOUT
#define UART_LOG_TIMEOUT   1000        // Send timeout (ms)
#endif

#ifndef UART_LOG_BUF_SIZE
#define UART_LOG_BUF_SIZE  256         // Size of the formatting buffer
#endif
/* ======================================== */

/**
 * @brief Blocking UART transmit helper (call outside of ISR context).
 */
static inline void uart_write_blocking(const uint8_t *data, size_t len)
{
    HAL_UART_Transmit(&UART_LOG_INSTANCE, (uint8_t *)data, (uint16_t)len, UART_LOG_TIMEOUT);
}

/**
 * @brief Normalize newlines to CRLF before sending to the terminal.
 */
static void uart_write_with_crlf(const char *s, size_t len)
{
    for (size_t i = 0; i < len; ++i) {
        char c = s[i];
        if (c == '\n') {
            const char crlf[2] = {'\r','\n'};
            uart_write_blocking((const uint8_t*)crlf, 2);
        } else {
            uart_write_blocking((const uint8_t*)&c, 1);
        }
    }
}

/**
 * @brief Output a zero-terminated string (with automatic CRLF normalization).
 */
void uart_puts(const char *s)
{
    if (s == NULL) {
        return;
    }

    uart_write_with_crlf(s, strlen(s));
}

/**
 * @brief printf-style helper built on top of the debug UART.
 * @return Number of characters that would have been written, or a negative error.
 */
int uart_printf(const char *fmt, ...)
{
    char buf[UART_LOG_BUF_SIZE];
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    if (n < 0) return n;  
    size_t out_len = (n < (int)sizeof(buf)) ? (size_t)n : (size_t)sizeof(buf) - 1;
    uart_write_with_crlf(buf, out_len);

    /* 
    if (n >= (int)sizeof(buf)) {
        uart_puts("...[truncated]\n");
    }
    */
    return n;
}

/**
 * @brief Hex dump helper for quick binary inspection.
 */
void uart_hexdump(const void *data, size_t len, const char *title)
{
    const uint8_t *p = (const uint8_t*)data;
    if (title) uart_printf("%s (len=%u):\n", title, (unsigned)len);

    char line[80];
    for (size_t i = 0; i < len; i += 16) {
        int pos = 0;
        pos += snprintf(line + pos, sizeof(line) - pos, "%08X  ", (unsigned)i);

        /* hex */
        for (size_t j = 0; j < 16; ++j) {
            if (i + j < len) pos += snprintf(line + pos, sizeof(line) - pos, "%02X ", p[i + j]);
            else              pos += snprintf(line + pos, sizeof(line) - pos, "   ");
            if (j == 7) pos += snprintf(line + pos, sizeof(line) - pos, " ");
        }

        /* ascii */
        pos += snprintf(line + pos, sizeof(line) - pos, " |");
        for (size_t j = 0; j < 16 && i + j < len; ++j) {
            uint8_t c = p[i + j];
            pos += snprintf(line + pos, sizeof(line) - pos, "%c", (c >= 32 && c <= 126) ? c : '.');
        }
        pos += snprintf(line + pos, sizeof(line) - pos, "|\n");

        uart_puts(line);
    }
}
```

</details>

<details>
<summary>bsp/debug/debug_driver.h</summary>

```c
#pragma once

#include <stddef.h>
#include <stdio.h>

/**
 * @file debug_driver.h
 * @brief Interfaces for redirecting stdio calls to the debug UART and lightweight log helpers.
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __GNUC__
/**
 * @brief Retarget newlib write syscall to the configured debug UART.
 * @param file Logical file descriptor supplied by the C library.
 * @param ptr Pointer to the transmit buffer.
 * @param len Number of bytes to be written.
 * @return Number of bytes reported as written.
 */
int _write(int file, char *ptr, int len);

/**
 * @brief Retarget newlib read syscall to the configured debug UART.
 * @param file Logical file descriptor supplied by the C library.
 * @param ptr Pointer to the receive buffer.
 * @param len Number of bytes requested.
 * @return Number of bytes actually read.
 */
int _read(int file, char *ptr, int len);
#elif defined(__ARMCC_VERSION)
/**
 * @brief Retarget fputc calls to the configured debug UART.
 * @param ch Character to emit.
 * @param f Ignored file handle.
 * @return The character that was transmitted.
 */
int fputc(int ch, FILE *f);

/**
 * @brief Retarget fgetc calls to the configured debug UART.
 * @param f Ignored file handle.
 * @return Received character cast to int.
 */
int fgetc(FILE *f);

/**
 * @brief Disable the semihosting exit hook when using the standard C library.
 * @param x Exit code (unused).
 */
void _sys_exit(int x);
#endif

/**
 * @brief Send a zero-terminated string over the debug UART with CRLF normalization.
 * @param s String to transmit (may be NULL).
 */
void uart_puts(const char *s);

/**
 * @brief printf-style helper forwarding formatted text to the debug UART.
 * @param fmt Format string describing the output.
 * @return Number of characters that would have been written, or negative on error.
 */
int uart_printf(const char *fmt, ...);

/**
 * @brief Hex dump utility for visualizing binary buffers.
 * @param data Buffer start pointer.
 * @param len Number of bytes to dump.
 * @param title Optional title string, may be NULL.
 */
void uart_hexdump(const void *data, size_t len, const char *title);

#ifdef __cplusplus
}
#endif
```

</details>

<details>
<summary>bsp/sdio_card/driver_sdcard_test.c</summary>

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
 * @file      driver_sdcard_test.c
 * @brief     driver sdcard test source file
 * @version   1.0.0
 * @date      2025-10-19
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
