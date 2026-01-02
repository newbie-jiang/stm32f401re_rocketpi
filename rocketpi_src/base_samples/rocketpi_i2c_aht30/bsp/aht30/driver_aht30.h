#pragma once

#include <stdint.h>

#include "stm32f4xx_hal.h"

/**
 * @file driver_aht30.h
 * @brief AHT30温湿度传感器的软件I2C驱动声明。
 */

#define AHT30_I2C_ADDRESS        (0x38U << 1) /**< 为HAL使用而左移的7位地址。 */
#define AHT30_MEASUREMENT_DELAY  80U          /**< 典型的测量转换延时（毫秒）。 */
#define AHT30_POWER_ON_DELAY     20U          /**< 上电后发送命令前的最小等待时间（毫秒）。 */
#define AHT30_POST_RESET_DELAY   20U          /**< 发出软复位后的等待时间（毫秒）。 */

#ifdef __cplusplus
extern "C" {
#endif

HAL_StatusTypeDef aht30_init(void);
HAL_StatusTypeDef aht30_read_raw(uint8_t raw[6]);
HAL_StatusTypeDef aht30_read(float *temperature_c, float *humidity_pct);

#ifdef __cplusplus
}
#endif
