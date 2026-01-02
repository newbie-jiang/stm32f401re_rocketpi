#include "driver_sg90_test.h"

#include "tim.h"
#include "usart.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

/**
 * TIM3 is configured for a 20 ms period with 1 µs resolution, so the CCR value
 * directly represents the pulse width in microseconds.
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

void SG90_Test_Init(void)
{
    /* Ensure PWM output is running before updating the duty cycle */
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);
    /* Move the horn to the mid position as a safe default */
    (void)SG90_Test_SetAngle(90.0f);
}

static char uart_log_buffer[2][32];
static uint8_t uart_active_buffer;

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

HAL_StatusTypeDef SG90_Test_SetPulse(uint16_t pulse_width_us)
{
    uint16_t clamped = clamp_pulse(pulse_width_us);

    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, clamped);

    return (clamped == pulse_width_us) ? HAL_OK : HAL_ERROR;
}

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

void SG90_Test_Sweep(uint32_t dwell_ms)
{
    static const float positions[] = {0.0f, 45.0f, 90.0f, 135.0f, 180.0f};
    for (size_t i = 0; i < (sizeof(positions) / sizeof(positions[0])); ++i)
    {
        (void)SG90_Test_SetAngle(positions[i]);
        HAL_Delay(dwell_ms);
    }
}

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
