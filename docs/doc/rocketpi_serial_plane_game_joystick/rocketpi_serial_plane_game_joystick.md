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
<!-- BSP_DRIVERS_HASH:f2ffc2ff02b3c398c2e0a7ae36b48e03fb8740de240f03da4c5cc520ee75fead -->
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
//            if (sample.x.raw < ADC_X_LEFT_THR)  { out[n++] = '4'; }  // Left
//            if (sample.x.raw > ADC_X_RIGHT_THR) { out[n++] = '6'; }  // Right
//            if (sample.y.raw < ADC_Y_UP_THR)    { out[n++] = '8'; }  // Up
//            if (sample.y.raw > ADC_Y_DOWN_THR)  { out[n++] = '5'; }  // Down
						
						 if (sample.x.raw < ADC_X_LEFT_THR)  { out[n++] = '6'; }  // Right
            if (sample.x.raw > ADC_X_RIGHT_THR) { out[n++] = '4'; }  // Left
            if (sample.y.raw < ADC_Y_UP_THR)    { out[n++] = '5'; }  // Down
            if (sample.y.raw > ADC_Y_DOWN_THR)  { out[n++] = '8'; }  // Up
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

<!-- BSP_DRIVERS_END -->
