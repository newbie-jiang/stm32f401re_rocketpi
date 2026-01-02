## rocketpi_aht30_hardware_i2c_base

## 效果展示

![image-20251116004609250](https://cloud.rocketpi.club/cloud/image-20251116004609250.png)

## 功能说明

面向 RocketPI STM32F401RE 开发板的 **AHT30温湿度 演示工程**。主要特性：

- 驱动AHT30在串口上打印温湿度。
- 提供 `driver_aht30` 基础驱动。
- `driver_aht30_test` 直接调用测试，自主选择轮询时间。

## 硬件连接

- AHT32 SCL ：PB8
- AHT32 SDA：PB9
- 电源：接3.3v或5v

## CubeMX配置

### 硬件i2c配置 

- （无特别说明不配置dma与中断）

![image-20251116002705838](https://cloud.rocketpi.club/cloud/image-20251116002705838.png)

![image-20251116002929989](https://cloud.rocketpi.club/cloud/image-20251116002929989.png)

### usart配置

![image-20251116004045780](https://cloud.rocketpi.club/cloud/image-20251116004045780.png)

![image-20251116003215745](https://cloud.rocketpi.club/cloud/image-20251116003215745.png)

<!-- BSP_DRIVERS_START -->
<!-- BSP_DRIVERS_HASH:7037e503a24c98c91abe2db8820af45c42417c8b4d72e19dbf2286706edd691c -->
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
#include "i2c.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "driver_aht30_test.h"
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
  MX_I2C1_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  printf("Initializing AHT30...\r\n");
  if (aht30_init() != HAL_OK)
  {
    printf("AHT30 init failed\r\n");
  }
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    (void)aht30_test_log_measurement();
    HAL_Delay(1000);
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
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
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
<summary>bsp/aht30/driver_aht30.c</summary>

```c
#include "driver_aht30.h"

#include "i2c.h"

/**
 * @file driver_aht30.c
 * @brief AHT30 temperature/humidity sensor driver implementation over hardware I2C.
 * @author rocket
 * @copyright Copyright (c) 2025 rocket. Authorized use only.
 */

#define AHT30_CMD_TRIGGER     0xACU /**< Measurement trigger command ID. */
#define AHT30_CMD_CONFIG_0    0x33U /**< Default trigger configuration byte 0. */
#define AHT30_CMD_CONFIG_1    0x00U /**< Default trigger configuration byte 1. */
#define AHT30_CMD_RESET       0xBAU /**< Soft-reset command ID. */
#define AHT30_STATUS_BUSY     0x80U /**< Status bit indicating an ongoing measurement. */

/**
 * @brief Issue a soft reset over I2C.
 * @return HAL status returned by HAL_I2C_Master_Transmit.
 */
static HAL_StatusTypeDef aht30_soft_reset(void)
{
    uint8_t cmd = AHT30_CMD_RESET;
    return HAL_I2C_Master_Transmit(&hi2c1, AHT30_I2C_ADDRESS, &cmd, 1U, HAL_MAX_DELAY);
}

/**
 * @brief Convert raw 20-bit humidity and temperature samples to engineering units.
 * @param raw Input buffer read from the sensor.
 * @param temperature_c Output temperature in Celsius.
 * @param humidity_pct Output relative humidity percentage.
 */
static void aht30_convert_samples(const uint8_t raw[6], float *temperature_c, float *humidity_pct)
{
    uint32_t raw_humidity = ((uint32_t)raw[1] << 12)
                          | ((uint32_t)raw[2] << 4)
                          | (uint32_t)(raw[3] >> 4);

    uint32_t raw_temperature = (((uint32_t)raw[3] & 0x0FU) << 16)
                             | ((uint32_t)raw[4] << 8)
                             | (uint32_t)raw[5];

    *humidity_pct = (raw_humidity * 100.0f) / 1048576.0f;
    *temperature_c = (raw_temperature * 200.0f) / 1048576.0f - 50.0f;
}

/**
 * @brief Perform the basic initialization sequence (power-on delay + soft reset).
 */
HAL_StatusTypeDef aht30_init(void)
{
    HAL_Delay(AHT30_POWER_ON_DELAY);
    HAL_StatusTypeDef status = aht30_soft_reset();
    HAL_Delay(AHT30_POST_RESET_DELAY);
    return status;
}

/**
 * @brief Trigger one measurement and retrieve the raw 6-byte payload.
 */
HAL_StatusTypeDef aht30_read_raw(uint8_t raw[6])
{
    uint8_t cmd[3] = {AHT30_CMD_TRIGGER, AHT30_CMD_CONFIG_0, AHT30_CMD_CONFIG_1};
    HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(&hi2c1, AHT30_I2C_ADDRESS, cmd, sizeof(cmd), HAL_MAX_DELAY);
    if (status != HAL_OK) {
        return status;
    }

    HAL_Delay(AHT30_MEASUREMENT_DELAY);
    status = HAL_I2C_Master_Receive(&hi2c1, AHT30_I2C_ADDRESS, raw, 6U, HAL_MAX_DELAY);
    if (status != HAL_OK) {
        return status;
    }

    if ((raw[0] & AHT30_STATUS_BUSY) != 0U) {
        return HAL_BUSY;
    }

    return HAL_OK;
}

/**
 * @brief Read and convert a temperature/humidity sample.
 */
HAL_StatusTypeDef aht30_read(float *temperature_c, float *humidity_pct)
{
    if ((temperature_c == NULL) || (humidity_pct == NULL)) {
        return HAL_ERROR;
    }

    uint8_t raw[6] = {0};
    HAL_StatusTypeDef status = aht30_read_raw(raw);
    if (status != HAL_OK) {
        return status;
    }

    aht30_convert_samples(raw, temperature_c, humidity_pct);
    return HAL_OK;
}
```

</details>

<details>
<summary>bsp/aht30/driver_aht30.h</summary>

```c
#pragma once

#include <stdint.h>

#include "stm32f4xx_hal.h"

/**
 * @file driver_aht30.h
 * @brief Hardware I2C driver for the AHT30 temperature and humidity sensor.
 * @author rocket
 * @copyright Copyright (c) 2025 rocket. Authorized use only.
 */

#define AHT30_I2C_ADDRESS        (0x38U << 1) /**< 7-bit address shifted for HAL usage. */
#define AHT30_MEASUREMENT_DELAY  80U          /**< Typical measurement conversion delay (ms). */
#define AHT30_POWER_ON_DELAY     20U          /**< Minimum wait after power-up before commands (ms). */
#define AHT30_POST_RESET_DELAY   20U          /**< Wait time after issuing a soft reset (ms). */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the sensor by issuing a soft reset and respecting timing constraints.
 * @return HAL_OK on success, otherwise a HAL error/busy code from the I2C transaction.
 */
HAL_StatusTypeDef aht30_init(void);

/**
 * @brief Trigger a measurement and read the raw 6-byte payload.
 * @param raw Pointer to a 6-byte buffer to receive the sensor output.
 * @return HAL_OK on success, HAL_BUSY if the sensor reports busy state, otherwise HAL error codes.
 */
HAL_StatusTypeDef aht30_read_raw(uint8_t raw[6]);

/**
 * @brief Read and convert the current temperature and humidity.
 * @param temperature_c Output pointer for the temperature in degrees Celsius.
 * @param humidity_pct Output pointer for the relative humidity in percent.
 * @return HAL_OK on success, HAL_BUSY while the sensor is measuring, or HAL error codes on failure.
 */
HAL_StatusTypeDef aht30_read(float *temperature_c, float *humidity_pct);

#ifdef __cplusplus
}
#endif
```

</details>

<details>
<summary>bsp/aht30/driver_aht30_test.c</summary>

```c
#include "driver_aht30_test.h"

#include <stdio.h>
#include <stdlib.h>

/**
 * @file driver_aht30_test.c
 * @brief Logging helpers and measurement demos for the AHT30 driver.
 * @author rocket
 * @copyright Copyright (c) 2025 rocket. Authorized use only.
 */

/**
 * @brief Print a formatted HAL status code with a short prefix.
 * @param phase Text describing the failed operation.
 * @param status HAL status returned by the driver.
 */
static void aht30_test_print_status(const char *phase, HAL_StatusTypeDef status)
{
    printf("AHT30 %s error (status=%d)\r\n", phase, (int)status);
}

/**
 * @brief Take one measurement, convert it, and print the human-readable results.
 * @return HAL_OK on success, HAL_BUSY if sensor is measuring, or HAL error codes on failure.
 */
HAL_StatusTypeDef aht30_test_log_measurement(void)
{
    float temperature = 0.0f;
    float humidity = 0.0f;
    HAL_StatusTypeDef status = aht30_read(&temperature, &humidity);
    if (status == HAL_OK) {
        int16_t temp10 = (int16_t)(temperature * 10.0f);
        uint16_t hum10 = (uint16_t)(humidity * 10.0f);
        printf("AHT30 -> T=%d.%01dC  RH=%d.%01d%%\r\n",
               temp10 / 10, abs(temp10 % 10),
               hum10 / 10, hum10 % 10);
    } else if (status == HAL_BUSY) {
        printf("AHT30 measurement busy\r\n");
    } else {
        aht30_test_print_status("read", status);
    }

    return status;
}

/**
 * @brief Request the raw 6-byte payload and dump it for debugging.
 * @return HAL_OK when data is captured, HAL_BUSY if the device is still measuring, otherwise HAL error codes.
 */
HAL_StatusTypeDef aht30_test_log_raw(void)
{
    uint8_t raw[6] = {0};
    HAL_StatusTypeDef status = aht30_read_raw(raw);
    if (status == HAL_OK) {
        printf("AHT30 raw -> %02X %02X %02X %02X %02X %02X (cal=%s)\r\n",
               raw[0], raw[1], raw[2], raw[3], raw[4], raw[5],
               ((raw[0] & 0x08U) != 0U) ? "yes" : "no");
    } else if (status == HAL_BUSY) {
        printf("AHT30 raw request busy\r\n");
    } else {
        aht30_test_print_status("raw", status);
    }

    return status;
}
```

</details>

<details>
<summary>bsp/aht30/driver_aht30_test.h</summary>

```c
#pragma once

#include "driver_aht30.h"

/**
 * @file driver_aht30_test.h
 * @brief Convenience helpers for exercising and logging the AHT30 driver.
 * @author rocket
 * @copyright Copyright (c) 2025 rocket. Authorized use only.
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Take a measurement and print the converted values via printf.
 * @return HAL_OK when the sample is valid, HAL_BUSY if the sensor is still measuring, otherwise HAL error codes.
 */
HAL_StatusTypeDef aht30_test_log_measurement(void);

/**
 * @brief Read the raw frame and dump the status/byte stream for debugging.
 * @return HAL_OK when the raw bytes are retrieved, HAL_BUSY if the sensor is still measuring, otherwise HAL error codes.
 */
HAL_StatusTypeDef aht30_test_log_raw(void);

#ifdef __cplusplus
}
#endif
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

<!-- BSP_DRIVERS_END -->
