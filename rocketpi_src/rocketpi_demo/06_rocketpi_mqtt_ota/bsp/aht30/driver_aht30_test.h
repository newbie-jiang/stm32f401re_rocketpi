#pragma once

#include "driver_aht30.h"

/**
 * @file driver_aht30_test.h
 * @brief 用于测试和记录AHT30驱动的辅助函数。
 * @author rocket
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 进行一次测量并通过printf输出转换值。
 * @return 采样有效时返回HAL_OK，传感器仍在测量时返回HAL_BUSY，否则返回HAL错误码。
 */
HAL_StatusTypeDef aht30_test_log_measurement(void);

/**
 * @brief 读取原始帧并输出状态/字节序列以便调试。
 * @return 成功获取原始字节返回HAL_OK，传感器仍在测量时返回HAL_BUSY，否则返回HAL错误码。
 */
HAL_StatusTypeDef aht30_test_log_raw(void);

#ifdef __cplusplus
}
#endif
