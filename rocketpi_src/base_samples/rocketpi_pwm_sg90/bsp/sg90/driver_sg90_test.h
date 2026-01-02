/**
 * @file driver_sg90_test.h
 * @brief Simple SG90 servo helper functions built on TIM3 CH4 (PC9).
 */

#ifndef DRIVER_SG90_TEST_H
#define DRIVER_SG90_TEST_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"

/**
 * SG90 pulse parameters in microseconds.
 * TIM3 is configured for 1 us ticks (84 MHz / 84 prescaler).
 */
#define SG90_MIN_PULSE_US   500U
#define SG90_MAX_PULSE_US  2500U
#define SG90_FRAME_PERIOD_US 20000U

/**
 * @brief Start TIM3 CH4 PWM output and park the servo at 90 degrees.
 */
void SG90_Test_Init(void);

/**
 * @brief Set the SG90 pulse width directly.
 * @param pulse_width_us Desired high time in microseconds, expected 500-2500.
 *        Values outside the range are clamped and return HAL_ERROR.
 */
HAL_StatusTypeDef SG90_Test_SetPulse(uint16_t pulse_width_us);

/**
 * @brief Position the SG90 by angle.
 * @param angle_deg Target angle in degrees, nominal range 0-180.
 *        Out-of-range values are saturated and report HAL_OK for the clamped setpoint.
 */
HAL_StatusTypeDef SG90_Test_SetAngle(float angle_deg);

/**
 * @brief Sweep through a set of demo angles.
 * @param dwell_ms Delay in milliseconds to hold each position; use >=100 for visible motion.
 */
void SG90_Test_Sweep(uint32_t dwell_ms);

/**
 * @brief Scan 0-180-0 degrees in fine (0.25°) steps while logging each angle over UART (DMA TX).
 *        This routine runs continuously until interrupted.
 *        Angle logs are throttled to once every ~5 degrees to keep motion smooth.
 * @param step_delay_ms Delay budget per degree; smaller values increase the sweep speed.
 *        Use at least 10 ms if the servo struggles to keep up.
 */
void SG90_Test_Scan(uint32_t step_delay_ms);

#ifdef __cplusplus
}
#endif

#endif /* DRIVER_SG90_TEST_H */
