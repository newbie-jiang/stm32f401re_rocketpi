/**
 * @file      driver_buzzer_test.c
 * @brief     TIM3 driven active buzzer test helper
 * @version   1.0.0
 */

#include "driver_buzzer_test.h"

#include "tim.h"

#include <stdio.h>

#define BUZZER_TEST_TIMER_MAX_PERIOD_TICKS        0x10000UL
#define BUZZER_TEST_TIMER_MAX_PRESCALER           0x10000UL

volatile static uint8_t s_buzzer_running = 0U;
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
