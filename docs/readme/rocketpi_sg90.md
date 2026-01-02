## 效果展示

视频待更新。。。。。。。。。。。。。。。

## 功能说明[¶](https://rocketpi.club/readme/rocketpi_ws2812b/#_2)

面向 RocketPI STM32F401RE 开发板的 sg90舵机驱动工程。主要特性：

- 使用 TIM3_CH4  驱动舵机
- 提供 `driver_sg90_test` 驱动测试，可自行设置任意角度值，或180°来回转动。
- 同时串口实时打印角度值

## 硬件连接

- 控制线接 PC9
- 电源：接 5V

## sg90驱动原理

### 1. **PWM 信号的工作方式**

- **PWM（脉宽调制）** 信号是一种周期性的方波信号，其周期保持固定，波形的宽度（高电平持续的时间）决定了舵机的角度。
- 舵机的标准控制信号是一个 **50 Hz 的 PWM 信号**，即每 20 毫秒（20 ms）为一个周期。每个周期中的高电平持续时间决定舵机转动的角度。

### 2. **控制信号与角度的关系**

SG90 通常使用 **1 ms 到 2 ms** 的脉宽来控制角度：

- **1 ms** 的脉宽信号通常对应舵机转到 **0°**。
- **1.5 ms** 的脉宽信号通常对应舵机转到 **90°**。
- **2 ms** 的脉宽信号通常对应舵机转到 **180°**。

因此，控制信号的脉宽越长，舵机旋转的角度越大。

**举例**：

- 每 20 毫秒一个周期，高电平 1 毫秒时，舵机转到 0°。
- 每 20 毫秒一个周期，高电平 2 毫秒时，舵机转到 180°。

## CubeMX配置

### 定时器配置

![image-20251116235249239](https://cloud.rocketpi.club/cloud/image-20251116235249239.png)

### USART配置 

开启DMA发送并配置全局中断 （不使用dma会阻塞舵机运行时间）

![image-20251117003159796](https://cloud.rocketpi.club/cloud/image-20251117003159796.png)

![image-20251117005159252](https://cloud.rocketpi.club/cloud/image-20251117005159252.png)

![image-20251117005300250](https://cloud.rocketpi.club/cloud/image-20251117005300250.png)



## 逻辑分析仪波形测量

![image-20251117003519946](https://cloud.rocketpi.club/cloud/image-20251117003519946.png)

## 串口实时角度值

![image-20251117005335158](https://cloud.rocketpi.club/cloud/image-20251117005335158.png)

<!-- BSP_DRIVERS_START -->
<!-- BSP_DRIVERS_HASH:f311d4b84a8462198cef38450effdaf1fecf045085e761aa2d0c58b9b8199e54 -->
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
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "driver_sg90_test.h"
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
  MX_TIM3_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  SG90_Test_Init();
//	SG90_Test_Sweep(500);
	SG90_Test_Scan(2);
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
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
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
<summary>bsp/sg90/driver_sg90_test.c</summary>

```c
#include "driver_sg90_test.h"

#include "tim.h"
#include "usart.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

/**
 * @file driver_sg90_test.c
 * @brief Minimal SG90 servo routines built on TIM3 CH4 with optional UART logging.
 * @details TIM3 is prescaled for 1 us ticks and a 20 ms auto-reload, so the CCR value
 *          directly maps to the high pulse duration in microseconds.
 * @author rocket
 */

/**
 * @brief Clamp a requested pulse width to the safe SG90 range.
 * @param pulse_us Candidate pulse high time in microseconds.
 * @return The clamped pulse width.
 */
static uint16_t clamp_pulse(uint16_t pulse_us)
{
    if (pulse_us < SG90_MIN_PULSE_US)
    {
        return SG90_MIN_PULSE_US;
    }
    if (pulse_us > SG90_MAX_PULSE_US)
    {
        return SG90_MAX_PULSE_US;
    }
    return pulse_us;
}

/**
 * @brief Enable PWM output and park the servo at mid-stick (90 degrees).
 */
void SG90_Test_Init(void)
{
    /* Ensure PWM output is running before updating the duty cycle */
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);
    /* Move the horn to the mid position as a safe default */
    (void)SG90_Test_SetAngle(90.0f);
}

static char uart_log_buffer[2][32];
static uint8_t uart_active_buffer;

/**
 * @brief Emit the current angle over UART (DMA) if the peripheral is idle.
 * @param angle_deg Angle in degrees to report.
 */
static void log_angle(float angle_deg)
{
    if (huart2.gState != HAL_UART_STATE_READY)
    {
        return;
    }

    uint8_t next_buffer = uart_active_buffer ^ 1U;
    int len = snprintf(uart_log_buffer[next_buffer], sizeof(uart_log_buffer[next_buffer]),
                       "SG90 angle: %.2f\r\n", angle_deg);
    if (len <= 0)
    {
        return;
    }

    if (HAL_UART_Transmit_DMA(&huart2,
                              (uint8_t *)uart_log_buffer[next_buffer],
                              (uint16_t)len) == HAL_OK)
    {
        uart_active_buffer = next_buffer;
    }
}

/**
 * @brief Set the raw PWM pulse width.
 * @param pulse_width_us Desired high time in microseconds.
 * @return HAL_OK when the request was in range, HAL_ERROR after clamping.
 */
HAL_StatusTypeDef SG90_Test_SetPulse(uint16_t pulse_width_us)
{
    uint16_t clamped = clamp_pulse(pulse_width_us);

    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, clamped);

    return (clamped == pulse_width_us) ? HAL_OK : HAL_ERROR;
}

/**
 * @brief Convert a requested angle into a pulse width and apply it.
 * @param angle_deg Target angle in degrees.
 * @return HAL_OK when the input is a finite number, HAL_ERROR on NaN.
 */
HAL_StatusTypeDef SG90_Test_SetAngle(float angle_deg)
{
    if (angle_deg != angle_deg)
    {
        return HAL_ERROR;
    }

    if (angle_deg < 0.0f)
    {
        angle_deg = 0.0f;
    }
    else if (angle_deg > 180.0f)
    {
        angle_deg = 180.0f;
    }

    float pulse = (float)SG90_MIN_PULSE_US +
                  (angle_deg / 180.0f) * (float)(SG90_MAX_PULSE_US - SG90_MIN_PULSE_US);

    return SG90_Test_SetPulse((uint16_t)(pulse + 0.5f));
}

/**
 * @brief Step through a handful of showcase positions with a fixed dwell time.
 * @param dwell_ms Delay in milliseconds to hold each stop.
 */
void SG90_Test_Sweep(uint32_t dwell_ms)
{
    static const float positions[] = {0.0f, 45.0f, 90.0f, 135.0f, 180.0f};
    for (size_t i = 0; i < (sizeof(positions) / sizeof(positions[0])); ++i)
    {
        (void)SG90_Test_SetAngle(positions[i]);
        HAL_Delay(dwell_ms);
    }
}

/**
 * @brief Continuously sweep from 0-180-0 degrees in 0.25 degree steps with UART logging.
 * @param step_delay_ms Delay budget per degree (smaller is faster).
 */
void SG90_Test_Scan(uint32_t step_delay_ms)
{
    const uint32_t steps_per_degree = 4U; /* 0.25 degree increments */
    const uint32_t log_stride = steps_per_degree * 5U; /* log every 5 degrees */
    const uint32_t max_index = 180U * steps_per_degree;
    const float inv_steps = 1.0f / (float)steps_per_degree;
    uint32_t base_delay = step_delay_ms / steps_per_degree;
    uint32_t remainder = step_delay_ms % steps_per_degree;

    while (1)
    {
        /* Forward sweep 0 -> 180 */
        for (uint32_t idx = 0U; idx <= max_index; ++idx)
        {
            float angle = (float)idx * inv_steps;
            (void)SG90_Test_SetAngle(angle);
            if ((idx % log_stride) == 0U || idx == max_index)
            {
                log_angle(angle);
            }
            uint32_t delay_ms = base_delay + ((idx % steps_per_degree) < remainder ? 1U : 0U);
            HAL_Delay(delay_ms);
        }

        /* Reverse sweep 179.75 -> 0.25 to avoid pausing twice at the endpoints */
        for (uint32_t idx = max_index - 1U; idx > 0U; --idx)
        {
            float angle = (float)idx * inv_steps;
            (void)SG90_Test_SetAngle(angle);
            if ((idx % log_stride) == 0U)
            {
                log_angle(angle);
            }
            uint32_t delay_ms = base_delay + ((idx % steps_per_degree) < remainder ? 1U : 0U);
            HAL_Delay(delay_ms);
        }
    }
}
```

</details>

<details>
<summary>bsp/sg90/driver_sg90_test.h</summary>

```c
/**
 * @file driver_sg90_test.h
 * @brief Public API for exercising an SG90 servo via TIM3 CH4 (PC9).
 * @author rocket
 */

#ifndef DRIVER_SG90_TEST_H
#define DRIVER_SG90_TEST_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"

/**
 * @brief SG90 low end stop in microseconds (~0 degrees).
 */
#define SG90_MIN_PULSE_US     500U

/**
 * @brief SG90 high end stop in microseconds (~180 degrees).
 */
#define SG90_MAX_PULSE_US    2500U

/**
 * @brief SG90 refresh period in microseconds (20 ms frame).
 */
#define SG90_FRAME_PERIOD_US 20000U

/**
 * @brief Start TIM3 CH4 PWM output and park the servo at 90 degrees.
 */
void SG90_Test_Init(void);

/**
 * @brief Set the SG90 pulse width directly.
 * @param pulse_width_us Desired high time in microseconds, expected 500-2500.
 * @return HAL_OK when the requested pulse is within range, otherwise HAL_ERROR after clamping.
 */
HAL_StatusTypeDef SG90_Test_SetPulse(uint16_t pulse_width_us);

/**
 * @brief Position the SG90 by angle.
 * @param angle_deg Target angle in degrees, nominal range 0-180.
 * @return HAL_OK for valid numbers (after saturation), HAL_ERROR for NaN input.
 */
HAL_StatusTypeDef SG90_Test_SetAngle(float angle_deg);

/**
 * @brief Sweep through a set of demo angles.
 * @param dwell_ms Delay in milliseconds to hold each position (>=100 for visible motion).
 */
void SG90_Test_Sweep(uint32_t dwell_ms);

/**
 * @brief Scan 0-180-0 degrees in 0.25 degree steps while logging angles over UART (DMA TX).
 *        This routine runs continuously until interrupted with logs throttled to once every ~5 degrees.
 * @param step_delay_ms Delay budget per degree; use at least 10 ms if the servo struggles to keep up.
 */
void SG90_Test_Scan(uint32_t step_delay_ms);

#ifdef __cplusplus
}
#endif

#endif /* DRIVER_SG90_TEST_H */
```

</details>

<!-- BSP_DRIVERS_END -->
