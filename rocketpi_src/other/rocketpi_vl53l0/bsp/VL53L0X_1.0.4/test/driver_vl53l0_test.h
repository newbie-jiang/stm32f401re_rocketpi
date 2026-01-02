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
 * @file      driver_vl53l0_test.h
 * @brief     VL53L0X driver test helper
 * @version   1.0.0
 * @date      2025-10-29
 */

#ifndef DRIVER_VL53L0_TEST_H
#define DRIVER_VL53L0_TEST_H

#include "vl53l0x_api.h"

#ifdef __cplusplus
extern "C"{
#endif

/**
 * @brief default 8-bit I2C address used by the test helpers
 */
#define VL53L0X_TEST_DEFAULT_I2C_ADDR          (0x52U)

/**
 * @brief default I2C speed (kHz) used by the test helpers
 */
#define VL53L0X_TEST_DEFAULT_I2C_SPEED_KHZ     (400U)

/**
 * @brief initialise the VL53L0X using the default configuration
 * @return status code
 *         - 0 success
 *         - 1 initialisation failed
 */
uint8_t vl53l0x_test_init_default(void);

/**
 * @brief perform a single blocking ranging measurement
 * @param[out] measurement filled with the raw measurement data
 * @return status code
 *         - 0 success
 *         - 1 measurement failed
 */
uint8_t vl53l0x_test_perform_single(VL53L0X_RangingMeasurementData_t *measurement);

/**
 * @brief run a basic ranging loop and print the results to the default console
 * @param[in] sample_count number of samples to capture (0 => treat as 1)
 * @param[in] interval_ms delay between samples in milliseconds
 * @return status code
 *         - 0 success
 *         - 1 test failed
 */
uint8_t vl53l0x_test_run(uint32_t sample_count, uint32_t interval_ms);

/**
 * @brief release resources acquired by vl53l0x_test_init_default()
 */
void vl53l0x_test_deinit(void);

#ifdef __cplusplus
}
#endif

#endif
