## 效果展示

<img src="https://cloud.rocketpi.club/cloud/print.gif"
     width="800"
     height="480"
     alt="loading GIF" />

## 功能说明

使用三种不同的方式打印输出

### 第一种打印方式

- 使用microlib 微库（此处方式需要勾选 USE micro LIB）

![image-20251209230009587](https://cloud.rocketpi.club/cloud/image-20251209230009587.png)

### 第二种打印方式

- 使用标准库
- 取消此处方式需要取消勾选 USE micro LIB

### 第三种打印方式

- 使用HAL_UART_Transmit 自己实现打印函数

## 硬件连接

![image-20251209223749997](https://cloud.rocketpi.club/cloud/image-20251209223749997.png)

![image-20251209224405337](https://cloud.rocketpi.club/cloud/image-20251209224405337.png)

- 硬件上确保uart的跳线帽存在，以及连接到下载调试一体USB口

![image-20251209225722397](https://cloud.rocketpi.club/cloud/image-20251209225722397.png)

<!-- BSP_DRIVERS_START -->
<!-- BSP_DRIVERS_HASH:f0efd99b213c384b188b776aa141f11114b1a68292cfaefaa3837eeea9fc9c10 -->
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
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
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
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
//		printf("printf test\r\n");
		
		
		uart_puts("Hello, UART!\n");

		static int val = 123;
		uart_printf("value=%d, hex=0x%X\n", val, val);
		
		static uint8_t rxbuf[32] = {0x12,0x34,0x56,0x78, 'A','B','C'};
		uart_hexdump(rxbuf, sizeof(rxbuf), "RX BUF");

		HAL_Delay(1000);
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
 * @brief 将底层 stdio 函数重定向到 USART2，用于调试收发。
 * @author rocket
 */

#ifdef __GNUC__  // GCC
/**
 * @brief 将 newlib 的 write 系统调用重定向到调试 USART。
 * @param file C 库提供的文件描述符（未使用）。
 * @param ptr  待发送数据缓冲区。
 * @param len  待发送字节数。
 * @return 实际写入的字节数。
 */
int _write(int file, char *ptr, int len)
{
    (void)file;
    HAL_UART_Transmit(&huart2, (uint8_t *)ptr, len, HAL_MAX_DELAY);
    return len;
}

/**
 * @brief 将 newlib 的 read 系统调用重定向到调试 USART。
 * @param file C 库提供的文件描述符（未使用）。
 * @param ptr  接收缓冲区。
 * @param len  待读取字节数。
 * @return 实际读取的字节数。
 */
int _read(int file, char *ptr, int len)
{
    (void)file;
    HAL_UART_Receive(&huart2, (uint8_t *)ptr, len, HAL_MAX_DELAY);
    return len;
}
#elif defined(__ARMCC_VERSION)  // Keil
/**
 * @brief 在 Arm Compiler/Keil 下，将 fputc 重定向到调试 USART。
 * @param ch 待发送字符。
 * @param f  被忽略的文件句柄。
 * @return 实际发送的字符。
 */
int fputc(int ch, FILE *f)
{
    (void)f;
    uint8_t data = (uint8_t)ch;
    HAL_UART_Transmit(&huart2, &data, 1U, HAL_MAX_DELAY);
    return ch;
}

/**
 * @brief 在 Arm Compiler/Keil 下，将 fgetc 重定向到调试 USART。
 * @param f 被忽略的文件句柄。
 * @return 接收到的字符。
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
   * @brief 空的退出处理，避免 Keil 回退到半主机。
   * @param x 退出码（忽略）。
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
#define UART_LOG_INSTANCE  huart2      // 可替换为你希望使用的 UART 句柄
#endif

#ifndef UART_LOG_TIMEOUT
#define UART_LOG_TIMEOUT   1000        // 发送超时（毫秒）
#endif

#ifndef UART_LOG_BUF_SIZE
#define UART_LOG_BUF_SIZE  256         // 格式化缓冲区大小
#endif
/* ======================================== */

/**
 * @brief 阻塞式 UART 发送辅助（需在非中断上下文调用）。
 */
static inline void uart_write_blocking(const uint8_t *data, size_t len)
{
    HAL_UART_Transmit(&UART_LOG_INSTANCE, (uint8_t *)data, (uint16_t)len, UART_LOG_TIMEOUT);
}

/**
 * @brief 发送前将换行符规范化为 CRLF。
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
 * @brief 输出以 0 结尾的字符串（自动做 CRLF 规范化）。
 */
void uart_puts(const char *s)
{
    if (s == NULL) {
        return;
    }

    uart_write_with_crlf(s, strlen(s));
}

/**
 * @brief 基于调试 UART 的 printf 风格输出。
 * @return 预期写入的字符数，错误返回负值。
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
 * @brief 十六进制转储，便于快速检查二进制内容。
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
 * @brief 将 stdio 重定向到调试 UART 的接口，以及轻量级日志辅助函数。
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __GNUC__
/**
 * @brief 将 newlib 的 write 系统调用重定向到调试 UART。
 * @param file C 库提供的文件描述符。
 * @param ptr  待发送缓冲区指针。
 * @param len  发送的字节数。
 * @return 已写入的字节数。
 */
int _write(int file, char *ptr, int len);

/**
 * @brief 将 newlib 的 read 系统调用重定向到调试 UART。
 * @param file C 库提供的文件描述符。
 * @param ptr  接收缓冲区指针。
 * @param len  期望读取的字节数。
 * @return 实际读取的字节数。
 */
int _read(int file, char *ptr, int len);
#elif defined(__ARMCC_VERSION)
/**
 * @brief 将 fputc 重定向到调试 UART。
 * @param ch 待发送字符。
 * @param f  被忽略的文件句柄。
 * @return 实际发送的字符。
 */
int fputc(int ch, FILE *f);

/**
 * @brief 将 fgetc 重定向到调试 UART。
 * @param f 被忽略的文件句柄。
 * @return 接收到的字符（int）。
 */
int fgetc(FILE *f);

/**
 * @brief 使用标准 C 库时，关闭半主机退出钩子。
 * @param x 退出码（未使用）。
 */
void _sys_exit(int x);
#endif

/**
 * @brief 通过调试 UART 发送以 0 结尾的字符串，并规范化 CRLF。
 * @param s 待发送的字符串（可为 NULL）。
 */
void uart_puts(const char *s);

/**
 * @brief printf 风格的调试 UART 输出辅助。
 * @param fmt 格式化描述字符串。
 * @return 预期写入的字符数，错误返回负值。
 */
int uart_printf(const char *fmt, ...);

/**
 * @brief 十六进制转储工具，便于查看二进制缓冲区。
 * @param data 缓冲区起始指针。
 * @param len  要转储的字节数。
 * @param title 可选标题字符串，可为 NULL。
 */
void uart_hexdump(const void *data, size_t len, const char *title);

#ifdef __cplusplus
}
#endif
```

</details>

<!-- BSP_DRIVERS_END -->
