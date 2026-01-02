#include "driver_aht30_test.h"

#include <stdio.h>
#include <stdlib.h>

/**
 * @file driver_aht30_test.c
 * @brief AHT30驱动的日志辅助与测量演示。
 * @author rocket
 */

/**
 * @brief 打印带简短前缀的HAL状态码。
 * @param phase 描述失败操作的文本。
 * @param status 驱动返回的HAL状态。
 */
static void aht30_test_print_status(const char *phase, HAL_StatusTypeDef status)
{
    printf("AHT30 %s error (status=%d)\r\n", phase, (int)status);
}

/**
 * @brief 获取一次测量、完成转换并输出可读结果。
 * @return 成功返回HAL_OK，传感器测量中返回HAL_BUSY，失败则返回HAL错误码。
 */
HAL_StatusTypeDef aht30_test_log_measurement(void)
{
    float temperature = 0.0f;
    float humidity = 0.0f;
    HAL_StatusTypeDef status = aht30_read(&temperature, &humidity);
    if (status == HAL_OK) {
        int16_t temp10 = (int16_t)(temperature * 10.0f);
        uint16_t hum10 = (uint16_t)(humidity * 10.0f);
        printf("AHT30 -> T=%d.%01dC  RH=%d.%01d%%\r\n",
               temp10 / 10, abs(temp10 % 10),
               hum10 / 10, hum10 % 10);
    } else if (status == HAL_BUSY) {
        printf("AHT30 measurement busy\r\n");
    } else {
        aht30_test_print_status("read", status);
    }

    return status;
}

/**
 * @brief 读取6字节原始数据并打印，便于调试。
 * @return 成功捕获数据返回HAL_OK，设备仍在测量时返回HAL_BUSY，否则返回HAL错误码。
 */
HAL_StatusTypeDef aht30_test_log_raw(void)
{
    uint8_t raw[6] = {0};
    HAL_StatusTypeDef status = aht30_read_raw(raw);
    if (status == HAL_OK) {
        printf("AHT30 raw -> %02X %02X %02X %02X %02X %02X (cal=%s)\r\n",
               raw[0], raw[1], raw[2], raw[3], raw[4], raw[5],
               ((raw[0] & 0x08U) != 0U) ? "yes" : "no");
    } else if (status == HAL_BUSY) {
        printf("AHT30 raw request busy\r\n");
    } else {
        aht30_test_print_status("raw", status);
    }

    return status;
}
