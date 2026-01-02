## 功能说明

面向 RocketPI STM32F401RE 开发板的 **无源蜂鸣器 演示工程**。主要特性：

- 实现无源蜂鸣器演奏音乐

## 硬件连接

- 控制脚---PB0

## CubeMX配置

频率2khz

![image-20251117234230687](https://cloud.rocketpi.club/cloud/image-20251117234230687.png)

<!-- BSP_DRIVERS_START -->
<!-- BSP_DRIVERS_HASH:9a639b33bacb503a8eb4ea77d0f72f47c1685eeda9fdbc4faeda2c4474f115eb -->
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
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "driver_buzzer_test.h"
#include "driver_buzzer_songs.h"
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
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */

//  buzzer_test_beep(3000,50,2000);
//	buzzer_test_play_sequence(buzzer_song_twinkle_intro, 300, 500);
	buzzer_test_play_sequence(buzzer_song_ode_to_joy, 300, 500);



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
<summary>bsp/passive_buzzer/driver_buzzer_songs.h</summary>

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
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * @file      driver_buzzer_songs.h
 * @brief     Sample melodies for the TIM3/PB0 buzzer helper
 * @version   1.0.0
 * @date      2025-10-19
 *
 * Include this header in your test file (e.g. main.c) and feed the provided
 * note arrays into buzzer_test_play_sequence(). Feel free to add or modify
 * melodies following the same structure.
 */

#ifndef DRIVER_BUZZER_SONGS_H
#define DRIVER_BUZZER_SONGS_H

#include "driver_buzzer_test.h"

/* Common note frequencies (equal temperament, rounded to the nearest Hertz) */
#define BUZZER_NOTE_REST   0U
#define BUZZER_NOTE_C4     262U
#define BUZZER_NOTE_D4     294U
#define BUZZER_NOTE_E4     330U
#define BUZZER_NOTE_F4     349U
#define BUZZER_NOTE_G4     392U
#define BUZZER_NOTE_A4     440U
#define BUZZER_NOTE_B4     494U
#define BUZZER_NOTE_C5     523U
#define BUZZER_NOTE_D5     587U
#define BUZZER_NOTE_E5     659U
#define BUZZER_NOTE_F5     698U
#define BUZZER_NOTE_G5     784U

/* Straightforward ascending scale (do re mi...) */
static const buzzer_test_note_t buzzer_song_scale_demo[] =
{
    { BUZZER_NOTE_C4, 0U, 200U },
    { BUZZER_NOTE_D4, 0U, 200U },
    { BUZZER_NOTE_E4, 0U, 200U },
    { BUZZER_NOTE_F4, 0U, 200U },
    { BUZZER_NOTE_G4, 0U, 200U },
    { BUZZER_NOTE_A4, 0U, 200U },
    { BUZZER_NOTE_B4, 0U, 200U },
    { BUZZER_NOTE_C5, 0U, 400U },
};
#define BUZZER_SONG_SCALE_DEMO_LENGTH   ((uint32_t)(sizeof(buzzer_song_scale_demo) / sizeof(buzzer_song_scale_demo[0])))

/* Opening phrase of "Twinkle Twinkle Little Star" */
static const buzzer_test_note_t buzzer_song_twinkle_intro[] =
{
    { BUZZER_NOTE_C4, 0U, 300U },
    { BUZZER_NOTE_C4, 0U, 300U },
    { BUZZER_NOTE_G4, 0U, 300U },
    { BUZZER_NOTE_G4, 0U, 300U },
    { BUZZER_NOTE_A4, 0U, 300U },
    { BUZZER_NOTE_A4, 0U, 300U },
    { BUZZER_NOTE_G4, 0U, 600U },
    { BUZZER_NOTE_F4, 0U, 300U },
    { BUZZER_NOTE_F4, 0U, 300U },
    { BUZZER_NOTE_E4, 0U, 300U },
    { BUZZER_NOTE_E4, 0U, 300U },
    { BUZZER_NOTE_D4, 0U, 300U },
    { BUZZER_NOTE_D4, 0U, 300U },
    { BUZZER_NOTE_C4, 0U, 600U },
};
#define BUZZER_SONG_TWINKLE_INTRO_LENGTH   ((uint32_t)(sizeof(buzzer_song_twinkle_intro) / sizeof(buzzer_song_twinkle_intro[0])))



/* Extended excerpt of Beethoven's "Ode to Joy" */
#ifndef BUZZER_SONG_ODE_TO_JOY_TEMPO_COEFF_MS
#define BUZZER_SONG_ODE_TO_JOY_TEMPO_COEFF_MS   8U  /* Lower values speed up playback */
#endif

#define ODE_TO_JOY_DURATION(units)              ((units) * BUZZER_SONG_ODE_TO_JOY_TEMPO_COEFF_MS)

static const buzzer_test_note_t buzzer_song_ode_to_joy[] =
{
    { BUZZER_NOTE_E4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_E4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_F4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_G4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_G4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_F4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_E4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_D4, 0U, ODE_TO_JOY_DURATION(36U) },
    { BUZZER_NOTE_C4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_C4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_D4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_E4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_E4, 0U, ODE_TO_JOY_DURATION(22U) },
    { BUZZER_NOTE_D4, 0U, ODE_TO_JOY_DURATION(22U) },
    { BUZZER_NOTE_D4, 0U, ODE_TO_JOY_DURATION(42U) },
    { BUZZER_NOTE_E4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_E4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_F4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_G4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_G4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_F4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_E4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_D4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_C4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_C4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_D4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_E4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_D4, 0U, ODE_TO_JOY_DURATION(22U) },
    { BUZZER_NOTE_C4, 0U, ODE_TO_JOY_DURATION(22U) },
    { BUZZER_NOTE_C4, 0U, ODE_TO_JOY_DURATION(42U) },
    { BUZZER_NOTE_D4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_D4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_E4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_C4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_D4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_E4, 0U, ODE_TO_JOY_DURATION(36U) },
    { BUZZER_NOTE_F4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_E4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_D4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_C4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_D4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_G4, 0U, ODE_TO_JOY_DURATION(40U) },
    { BUZZER_NOTE_E4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_E4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_F4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_G4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_G4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_F4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_E4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_D4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_C4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_C4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_D4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_E4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_D4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_C4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_C4, 0U, ODE_TO_JOY_DURATION(48U) },
};
#define BUZZER_SONG_ODE_TO_JOY_LENGTH   ((uint32_t)(sizeof(buzzer_song_ode_to_joy) / sizeof(buzzer_song_ode_to_joy[0])))

#undef ODE_TO_JOY_DURATION


/* Simple notification chime (two tones with a rest) */
static const buzzer_test_note_t buzzer_song_notify_chime[] =
{
    { BUZZER_NOTE_G5, 60U, 120U },
    { BUZZER_NOTE_REST, 0U, 80U },
    { BUZZER_NOTE_C5, 60U, 200U },
};
#define BUZZER_SONG_NOTIFY_CHIME_LENGTH   ((uint32_t)(sizeof(buzzer_song_notify_chime) / sizeof(buzzer_song_notify_chime[0])))

#endif /* DRIVER_BUZZER_SONGS_H */
```

</details>

<details>
<summary>bsp/passive_buzzer/driver_buzzer_test.c</summary>

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
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * @file      driver_buzzer_test.c
 * @brief     TIM3 driven active buzzer test helper
 * @version   1.0.0
 * @date      2025-10-19
 */

#include "driver_buzzer_test.h"

#include "tim.h"

#include <stdio.h>

#define BUZZER_TEST_TIMER_MAX_PERIOD_TICKS        0x10000UL
#define BUZZER_TEST_TIMER_MAX_PRESCALER           0x10000UL

static uint8_t s_buzzer_running = 0U;
static uint32_t s_last_freq_hz = 0U;
static uint8_t s_last_duty_percent = 0U;

static uint32_t buzzer_test_get_timer_clock(void);
static uint8_t buzzer_test_apply(uint32_t frequency_hz, uint8_t duty_percent);

static uint32_t buzzer_test_get_timer_clock(void)
{
    RCC_ClkInitTypeDef clk_config;
    uint32_t flash_latency;
    uint32_t pclk1;

    HAL_RCC_GetClockConfig(&clk_config, &flash_latency);
    pclk1 = HAL_RCC_GetPCLK1Freq();

    if (clk_config.APB1CLKDivider == RCC_HCLK_DIV1)
    {
        return pclk1;
    }

    return pclk1 * 2U;
}

static uint8_t buzzer_test_apply(uint32_t frequency_hz, uint8_t duty_percent)
{
    uint64_t ticks_per_period;
    uint32_t prescaler = 1U;
    uint32_t auto_reload = 0U;
    uint32_t compare = 0U;
    HAL_StatusTypeDef status;

    if (frequency_hz == 0U)
    {
        frequency_hz = BUZZER_TEST_DEFAULT_FREQUENCY_HZ;
    }
    if (frequency_hz == 0U)
    {
        printf("buzzer: invalid frequency (0 Hz)\r\n");

        return 1U;
    }
    if (duty_percent == 0U)
    {
        duty_percent = BUZZER_TEST_DEFAULT_DUTY_PERCENT;
    }
    if (duty_percent > BUZZER_TEST_MAX_DUTY_PERCENT)
    {
        duty_percent = BUZZER_TEST_MAX_DUTY_PERCENT;
    }

    ticks_per_period = (uint64_t)buzzer_test_get_timer_clock();
    ticks_per_period = (ticks_per_period + (frequency_hz / 2U)) / frequency_hz;
    if (ticks_per_period == 0ULL)
    {
        ticks_per_period = 1ULL;
    }

    if (ticks_per_period > BUZZER_TEST_TIMER_MAX_PERIOD_TICKS)
    {
        prescaler = (uint32_t)((ticks_per_period + BUZZER_TEST_TIMER_MAX_PERIOD_TICKS - 1ULL) / BUZZER_TEST_TIMER_MAX_PERIOD_TICKS);
        if (prescaler > BUZZER_TEST_TIMER_MAX_PRESCALER)
        {
            prescaler = BUZZER_TEST_TIMER_MAX_PRESCALER;
        }
        ticks_per_period = (ticks_per_period + prescaler - 1ULL) / prescaler;
    }

    auto_reload = (uint32_t)ticks_per_period;
    if (auto_reload == 0U)
    {
        auto_reload = 1U;
    }
    if (auto_reload > BUZZER_TEST_TIMER_MAX_PERIOD_TICKS)
    {
        auto_reload = BUZZER_TEST_TIMER_MAX_PERIOD_TICKS;
    }

    compare = (uint32_t)(((uint64_t)auto_reload * duty_percent) / 100ULL);
    if (compare > 0U)
    {
        /* Keep the compare register strictly inside the counter range */
        if (compare >= auto_reload)
        {
            compare = auto_reload - 1U;
        }
    }

    status = HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_3);
    if ((status != HAL_OK) && (status != HAL_ERROR))
    {
        printf("buzzer: unable to stop pwm (err=%ld)\r\n", (long)status);

        return 1U;
    }

    __HAL_TIM_SET_PRESCALER(&htim3, prescaler - 1U);
    __HAL_TIM_SET_AUTORELOAD(&htim3, auto_reload - 1U);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, compare);
    __HAL_TIM_SET_COUNTER(&htim3, 0U);
    htim3.Init.Prescaler = (uint32_t)(prescaler - 1U);
    htim3.Init.Period = (uint32_t)(auto_reload - 1U);

    status = HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
    if (status != HAL_OK)
    {
        printf("buzzer: failed to start pwm (err=%ld)\r\n", (long)status);

        return 1U;
    }

    s_buzzer_running = 1U;
    s_last_freq_hz = frequency_hz;
    s_last_duty_percent = duty_percent;

    printf("buzzer: start freq=%lu Hz duty=%u%% PSC=%lu ARR=%lu CCR=%lu\r\n",
           (unsigned long)s_last_freq_hz,
           (unsigned int)s_last_duty_percent,
           (unsigned long)(prescaler - 1U),
           (unsigned long)(auto_reload - 1U),
           (unsigned long)compare);

    return 0U;
}

uint8_t buzzer_test_start(uint32_t frequency_hz, uint8_t duty_percent)
{
    return buzzer_test_apply(frequency_hz, duty_percent);
}

uint8_t buzzer_test_stop(void)
{
    HAL_StatusTypeDef status;

    status = HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_3);
    if ((status != HAL_OK) && (status != HAL_ERROR))
    {
        printf("buzzer: failed to stop pwm (err=%ld)\r\n", (long)status);

        return 1U;
    }

    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, 0U);
    s_buzzer_running = 0U;

    printf("buzzer: stop\r\n");

    return 0U;
}

uint8_t buzzer_test_beep(uint32_t frequency_hz, uint8_t duty_percent, uint32_t duration_ms)
{
    const uint32_t duration = (duration_ms == 0U) ? BUZZER_TEST_DEFAULT_DURATION_MS : duration_ms;

    if (buzzer_test_apply(frequency_hz, duty_percent) != 0U)
    {
        return 1U;
    }

    if (duration > 0U)
    {
        HAL_Delay(duration);

        if (buzzer_test_stop() != 0U)
        {
            return 1U;
        }
    }

    return 0U;
}

uint8_t buzzer_test_play_sequence(const buzzer_test_note_t *notes, uint32_t length, uint32_t gap_ms)
{
    const uint32_t gap = (gap_ms == 0U) ? BUZZER_TEST_DEFAULT_NOTE_GAP_MS : gap_ms;

    if ((notes == NULL) || (length == 0U))
    {
        return 1U;
    }

    for (uint32_t i = 0U; i < length; ++i)
    {
        uint32_t frequency = notes[i].frequency_hz;
        uint8_t duty = notes[i].duty_percent;
        uint32_t duration = notes[i].duration_ms;

        if (frequency == 0U)
        {
            /* Rest */
            if (buzzer_test_stop() != 0U)
            {
                return 2U;
            }
            HAL_Delay((duration == 0U) ? BUZZER_TEST_DEFAULT_DURATION_MS : duration);
        }
        else
        {
            if (duration == 0U)
            {
                duration = BUZZER_TEST_DEFAULT_DURATION_MS;
            }
            if (buzzer_test_apply(frequency, duty) != 0U)
            {
                (void)buzzer_test_stop();

                return 2U;
            }
            HAL_Delay(duration);
            if (buzzer_test_stop() != 0U)
            {
                return 2U;
            }
        }

        if ((gap > 0U) && (i + 1U < length))
        {
            HAL_Delay(gap);
        }
    }

    return 0U;
}
```

</details>

<details>
<summary>bsp/passive_buzzer/driver_buzzer_test.h</summary>

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
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * @file      driver_buzzer_test.h
 * @brief     TIM3 driven active buzzer test helper
 * @version   1.0.0
 * @date      2025-10-19
 */

#ifndef DRIVER_BUZZER_TEST_H
#define DRIVER_BUZZER_TEST_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Default PWM tone frequency used when an explicit frequency is not provided (Hz)
 */
#define BUZZER_TEST_DEFAULT_FREQUENCY_HZ          2000U

/**
 * @brief Default PWM duty cycle (percentage) applied to the buzzer output
 */
#define BUZZER_TEST_DEFAULT_DUTY_PERCENT          50U

/**
 * @brief Default tone duration used by buzzer_test_beep() when duration_ms is zero (milliseconds)
 */
#define BUZZER_TEST_DEFAULT_DURATION_MS           200U

/**
 * @brief Default gap inserted between notes in buzzer_test_play_sequence() when gap_ms is zero (milliseconds)
 */
#define BUZZER_TEST_DEFAULT_NOTE_GAP_MS           20U

/**
 * @brief Maximum duty cycle accepted by the helper (percentage)
 */
#define BUZZER_TEST_MAX_DUTY_PERCENT              100U

/**
 * @brief Note descriptor used by buzzer_test_play_sequence()
 *
 * A frequency of 0 produces a rest (silence) for the indicated duration.
 * A duty cycle of 0 adopts BUZZER_TEST_DEFAULT_DUTY_PERCENT.
 * A duration of 0 adopts BUZZER_TEST_DEFAULT_DURATION_MS.
 */
typedef struct
{
    uint32_t frequency_hz; /**< Tone frequency in Hertz (0 selects silence) */
    uint8_t duty_percent;  /**< PWM duty cycle percentage (0 selects the default) */
    uint32_t duration_ms;  /**< Tone (or rest) duration in milliseconds (0 selects the default) */
} buzzer_test_note_t;

/**
 * @brief     Configure TIM3 CH3 and start driving the buzzer
 * @param[in] frequency_hz Desired output frequency in Hertz (0 selects BUZZER_TEST_DEFAULT_FREQUENCY_HZ)
 * @param[in] duty_percent PWM duty cycle (0..100). Values above 100 are clamped.
 * @return    Status code
 *            - 0 success
 *            - 1 configuration or HAL error
 */
uint8_t buzzer_test_start(uint32_t frequency_hz, uint8_t duty_percent);

/**
 * @brief     Stop the PWM output driving the buzzer
 * @return    Status code
 *            - 0 success
 *            - 1 HAL error
 */
uint8_t buzzer_test_stop(void);

/**
 * @brief     Play a single tone on the buzzer
 * @param[in] frequency_hz Tone frequency in Hertz (0 selects the default)
 * @param[in] duty_percent PWM duty cycle percentage (0 selects the default)
 * @param[in] duration_ms  Tone duration in milliseconds (0 selects the default)
 * @return    Status code
 *            - 0 success
 *            - 1 configuration or HAL error
 */
uint8_t buzzer_test_beep(uint32_t frequency_hz, uint8_t duty_percent, uint32_t duration_ms);

/**
 * @brief     Play a sequence of tones and rests on the buzzer
 * @param[in] notes    Pointer to an array of @ref buzzer_test_note_t descriptors
 * @param[in] length   Number of entries in the notes array
 * @param[in] gap_ms   Pause added between successive notes (0 selects the default)
 * @return    Status code
 *            - 0 success
 *            - 1 invalid parameters
 *            - 2 playback aborted due to HAL error
 */
uint8_t buzzer_test_play_sequence(const buzzer_test_note_t *notes, uint32_t length, uint32_t gap_ms);

#ifdef __cplusplus
}
#endif

#endif
```

</details>

<!-- BSP_DRIVERS_END -->
