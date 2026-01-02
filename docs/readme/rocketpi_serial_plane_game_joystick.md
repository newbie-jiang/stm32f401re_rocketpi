## 效果展示

视频待更新。。。。。。。。。。。。。。。

![image-20251116200846693](https://cloud.rocketpi.club/cloud/image-20251116200846693.png)

## 功能说明

使用ADC双轴遥控杆 发送控制指令，通过串口连接到电脑上打飞机游戏

发送8前进，发送4左移，发送6右移，发送5后退出

## 硬件连接

PC4--y轴

 PC5--x轴

PC13 --key

## CubeMX配置

配置双通道 IN14 IN15对应PC4 PC5

分频配置PCLK2 =84M     84/4 = 21M   =  47.6ns

对 STM32F4 来说，**总转换时间 ≈ 采样时间 + 12.5 个周期**     （112+12.5）x 47.6   每个通道一次转换大约 6 µs 左右

![image-20251116192037813](https://cloud.rocketpi.club/cloud/image-20251116192037813.png)

该双轴遥杆还带一个按键，低电平触发

![image-20251116193237219](https://cloud.rocketpi.club/cloud/image-20251116193237219.png)

<!-- BSP_DRIVERS_START -->
<!-- BSP_DRIVERS_HASH:c02f66a991d53f6a61c784a93fd46a9bc2f487b36f322fa158c39363c16dcdd8 -->
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
#include "adc.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "driver_adc_joystick_test.h"
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
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */
	
//		adc_joystick_test_run(102400,100);
   adc_joystick_send_udlr_uart(0,20,&huart2);
	 
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
<summary>bsp/adc_joystick/driver_adc_joystick_test.c</summary>

```c
/**
 * @file driver_adc_joystick_test.c
 * @brief Blocking helpers for sampling the dual-axis analog joystick plus key.
 * @version 1.0.0
 * @date 2025-10-19
 *
 * The module offers three utilities:
 *   1. Capture raw and permille-scaled readings for the X/Y axes.
 *   2. Print formatted telemetry frames for manual tuning.
 *   3. Convert joystick deflection into keypad-style UART codes (4/6/8/5).
 */
#include "driver_adc_joystick_test.h"

#include <stdio.h>
#include "gpio.h"

/** @brief Maximum 12-bit ADC reading reported by hadc1. */
#define ADC_JOYSTICK_TEST_ADC_MAX_VALUE   4095U

static uint16_t adc_joystick_test_scale_permille(uint16_t raw);

/**
 * @brief Convert a raw ADC sample to 0..1000 permille (0.1%) resolution.
 * @param raw Raw ADC value reported by hadc1.
 * @return Scaled permille reading.
 */
static uint16_t adc_joystick_test_scale_permille(uint16_t raw)
{
    uint32_t permille = ((uint32_t)raw * 1000U + (ADC_JOYSTICK_TEST_ADC_MAX_VALUE / 2U)) /
                        ADC_JOYSTICK_TEST_ADC_MAX_VALUE;

    if (permille > 1000U)
    {
        permille = 1000U;
    }

    return (uint16_t)permille;
}

/**
 * @brief Check whether the joystick push-button is pressed.
 * @return 1 when pressed (pin pulled low), otherwise 0.
 */
static uint8_t adc_joystick_test_read_key_internal(void)
{
    GPIO_PinState state = HAL_GPIO_ReadPin(ADC_JOYSTICK_KEY_GPIO_Port, ADC_JOYSTICK_KEY_Pin);

    return (state == GPIO_PIN_RESET) ? 1U : 0U;
}

/**
 * @brief Acquire raw joystick readings for both axes plus key state.
 * @param[out] sample Receives raw counts, permille scaling, and button flag.
 * @return 0 on success, 1 when any ADC operation fails.
 */
uint8_t adc_joystick_test_sample(adc_joystick_sample_t *sample)
{
    HAL_StatusTypeDef status;
    uint16_t raw_x = 0U;
    uint16_t raw_y = 0U;

    if (sample == NULL)
    {
        return 1U;
    }

    status = HAL_ADC_Start(&hadc1);
    if (status != HAL_OK)
    {
        return 1U;
    }

    status = HAL_ADC_PollForConversion(&hadc1, ADC_JOYSTICK_TEST_POLL_TIMEOUT_MS);
    if (status != HAL_OK)
    {
        (void)HAL_ADC_Stop(&hadc1);

        return 1U;
    }
    raw_x = (uint16_t)HAL_ADC_GetValue(&hadc1);

    status = HAL_ADC_PollForConversion(&hadc1, ADC_JOYSTICK_TEST_POLL_TIMEOUT_MS);
    if (status != HAL_OK)
    {
        (void)HAL_ADC_Stop(&hadc1);

        return 1U;
    }
    raw_y = (uint16_t)HAL_ADC_GetValue(&hadc1);

    status = HAL_ADC_Stop(&hadc1);
    if (status != HAL_OK)
    {
        return 1U;
    }

    sample->x.raw = raw_x;
    sample->x.permille = adc_joystick_test_scale_permille(raw_x);
    sample->y.raw = raw_y;
    sample->y.permille = adc_joystick_test_scale_permille(raw_y);
    sample->key_pressed = adc_joystick_test_read_key_internal();

    return 0U;
}

/**
 * @brief Print joystick samples to the console for manual observation.
 * @param sample_count Number of samples to capture (0 uses default count).
 * @param delay_ms     Delay between samples in milliseconds (0 uses default).
 * @return 0 when all samples are captured, otherwise 1 on ADC failure.
 */
uint8_t adc_joystick_test_run(uint32_t sample_count, uint32_t delay_ms)
{
    const uint32_t count = (sample_count == 0U) ? ADC_JOYSTICK_TEST_DEFAULT_SAMPLE_COUNT : sample_count;
    const uint32_t delay = (delay_ms == 0U) ? ADC_JOYSTICK_TEST_DEFAULT_DELAY_MS : delay_ms;

    for (uint32_t i = 0U; i < count; ++i)
    {
        adc_joystick_sample_t sample;

        if (adc_joystick_test_sample(&sample) != 0U)
        {
            printf("adc joystick: sample %lu failed\r\n", (unsigned long)(i + 1U));

            return 1U;
        }

        printf("adc joystick: #%lu X=%4u (%3lu.%01lu%%) Y=%4u (%3lu.%01lu%%) KEY=%s\r\n",
               (unsigned long)(i + 1U),
               (unsigned int)sample.x.raw,
               (unsigned long)(sample.x.permille / 10U),
               (unsigned long)(sample.x.permille % 10U),
               (unsigned int)sample.y.raw,
               (unsigned long)(sample.y.permille / 10U),
               (unsigned long)(sample.y.permille % 10U),
               (sample.key_pressed != 0U) ? "DOWN" : "UP");

        HAL_Delay(delay);
    }

    return 0U;
}

/** @brief Empirical ADC thresholds used to derive U/D/L/R events. */
#define ADC_X_LEFT_THR    1600U  /**< Lower X readings indicate LEFT. */
#define ADC_X_RIGHT_THR   2900U  /**< Higher X readings indicate RIGHT. */
#define ADC_Y_DOWN_THR    2900U  /**< Higher Y readings indicate DOWN. */
#define ADC_Y_UP_THR      1600U  /**< Lower Y readings indicate UP. */

/**
 * @brief Stream U/D/L/R ASCII codes that represent joystick direction.
 * @param sample_count Number of samples to emit (0 keeps running).
 * @param delay_ms     Delay between samples in milliseconds (0 uses default).
 * @param huart        UART handle used to transmit the codes.
 * @return 0 success, 1 sample failure, 2 invalid UART, 3 UART TX failure.
 */
uint8_t adc_joystick_send_udlr_uart(uint32_t sample_count, uint32_t delay_ms, UART_HandleTypeDef *huart)
{
    const uint32_t delay = (delay_ms == 0U) ? ADC_JOYSTICK_TEST_DEFAULT_DELAY_MS : delay_ms;

    if (huart == NULL) {
        return 2U;
    }

    if (sample_count == 0U)
    {
        /* 0 indicates "run forever" for quick interactive tests. */
        while (1)
        {
            adc_joystick_sample_t sample;

            if (adc_joystick_test_sample(&sample) != 0U) {
                return 1U;
            }

            char out[4];
            uint32_t n = 0U;

            /* Direction mapping follows the keypad: 4=L, 6=R, 8=U, 5=D. */
            if (sample.x.raw < ADC_X_LEFT_THR)  { out[n++] = '4'; }  // Left
            if (sample.x.raw > ADC_X_RIGHT_THR) { out[n++] = '6'; }  // Right
            if (sample.y.raw < ADC_Y_UP_THR)    { out[n++] = '8'; }  // Up
            if (sample.y.raw > ADC_Y_DOWN_THR)  { out[n++] = '5'; }  // Down
            if (n > 0U) {
                if (HAL_UART_Transmit(huart, (uint8_t *)out, (uint16_t)n, 10U) != HAL_OK) {
                    return 3U;
                }
            }

            HAL_Delay(delay);
        }
    }
    else
    {
        /* Finite run: iterate a fixed number of samples. */
        for (uint32_t i = 0U; i < sample_count; ++i)
        {
            adc_joystick_sample_t sample;

            if (adc_joystick_test_sample(&sample) != 0U) {
                return 1U;
            }

            char out[4];
            uint32_t n = 0U;

            if (sample.x.raw < ADC_X_LEFT_THR)  { out[n++] = '4'; }  // Left
            if (sample.x.raw > ADC_X_RIGHT_THR) { out[n++] = '6'; }  // Right
            if (sample.y.raw < ADC_Y_UP_THR)    { out[n++] = '8'; }  // Up
            if (sample.y.raw > ADC_Y_DOWN_THR)  { out[n++] = '5'; }  // Down
            if (n > 0U) {
                if (HAL_UART_Transmit(huart, (uint8_t *)out, (uint16_t)n, 10U) != HAL_OK) {
                    return 3U;
                }
            }

            HAL_Delay(delay);
        }

        return 0U;
    }
}

/**
 * @brief Convenience wrapper that only returns the joystick key state.
 * @return 1 when pressed, otherwise 0.
 */
uint8_t adc_joystick_test_read_key(void)
{
    return adc_joystick_test_read_key_internal();
}
```

</details>

<details>
<summary>bsp/adc_joystick/driver_adc_joystick_test.h</summary>

```c
/**
 * @file driver_adc_joystick_test.h
 * @brief Dual-axis joystick sampling helpers (ADC channels 14/15 + key input).
 * @version 1.0.0
 * @date 2025-11-16
 */
#pragma once

#include "adc.h"
#include "usart.h"

/**
 * @brief Timeout (ms) used while polling each ADC conversion.
 */
#define ADC_JOYSTICK_TEST_POLL_TIMEOUT_MS        10U

/**
 * @brief Default sample-count fallback when the caller passes 0.
 */
#define ADC_JOYSTICK_TEST_DEFAULT_SAMPLE_COUNT   20U

/**
 * @brief Default inter-sample delay (ms) when the caller passes 0.
 */
#define ADC_JOYSTICK_TEST_DEFAULT_DELAY_MS       50U

/**
 * @brief Single joystick axis snapshot
 */
typedef struct
{
    uint16_t raw;        /**< Raw ADC reading (0..4095) */
    uint16_t permille;   /**< Position scaled to 0..1000 (0.1 % resolution) */
} adc_joystick_axis_sample_t;

/**
 * @brief Combined joystick X/Y snapshot
 */
typedef struct
{
    adc_joystick_axis_sample_t x; /**< X axis data (ADC channel 14) */
    adc_joystick_axis_sample_t y; /**< Y axis data (ADC channel 15) */
    uint8_t key_pressed;          /**< 1 when the joystick button is pressed */
} adc_joystick_sample_t;

/**
 * @brief     Acquire one joystick sample (both axes plus the push button).
 * @param[out] sample Populated with raw counts, permille scaling, and key flag.
 * @return    Status code
 *            - 0 success
 *            - 1 acquisition failed
 */
uint8_t adc_joystick_test_sample(adc_joystick_sample_t *sample);

/**
 * @brief     Continuously sample and print joystick readings via printf.
 * @param[in] sample_count Number of samples to print (0 uses the default).
 * @param[in] delay_ms     Delay between samples in milliseconds (0 uses default).
 * @return    Status code
 *            - 0 success
 *            - 1 acquisition failed
 */
uint8_t adc_joystick_test_run(uint32_t sample_count, uint32_t delay_ms);

/**
 * @brief     Continuously sample the joystick and send keypad codes over UART.
 * @param[in] sample_count Number of samples to process (0 uses the default / run forever).
 * @param[in] delay_ms     Delay between samples in milliseconds (0 uses default).
 * @param[in] huart        Initialized UART handle used for transmission.
 * @return    Status code
 *            - 0 success
 *            - 1 acquisition failed (joystick sampling error)
 *            - 2 invalid UART handle (NULL pointer)
 *            - 3 UART transmit failed
 */
uint8_t adc_joystick_send_udlr_uart(uint32_t sample_count, uint32_t delay_ms, UART_HandleTypeDef *huart);


/**
 * @brief Read the joystick button state.
 * @return 1 if the button is pressed, otherwise 0
 */
uint8_t adc_joystick_test_read_key(void);
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
