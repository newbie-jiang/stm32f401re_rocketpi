/**
 * @file driver_adc_joystick_test.h
 * @brief 双轴摇杆采样辅助（ADC 通道 14/15 加按键输入）。
 * @version 1.0.0
 * @date 2025-11-16
 */
#pragma once

#include "adc.h"
#include "usart.h"

/**
 * @brief 轮询每次 ADC 转换时的超时时间（毫秒）。
 */
#define ADC_JOYSTICK_TEST_POLL_TIMEOUT_MS        10U

/**
 * @brief 传入 0 时使用的默认采样次数。
 */
#define ADC_JOYSTICK_TEST_DEFAULT_SAMPLE_COUNT   20U

/**
 * @brief 传入 0 时默认的采样间隔（毫秒）。
 */
#define ADC_JOYSTICK_TEST_DEFAULT_DELAY_MS       50U

/**
 * @brief 摇杆安装方向（以“左=4/右=6/上=8/下=5”的逻辑方向为基准）。
 *        当摇杆模块顺时针旋转 90/180/270 度安装时，选择对应宏即可自动映射方向。
 */
#define ADC_JOYSTICK_ORIENTATION_0_DEG    0U  /**< 默认方向，无旋转。 */
#define ADC_JOYSTICK_ORIENTATION_90_DEG   1U  /**< 顺时针旋转 90 度。 */
#define ADC_JOYSTICK_ORIENTATION_180_DEG  2U  /**< 顺时针旋转 180 度。 */
#define ADC_JOYSTICK_ORIENTATION_270_DEG  3U  /**< 顺时针旋转 270 度。 */

/**
 * @brief 预设的摇杆方向选择，若未覆盖则使用 0 度。
 *        用户可在项目配置中覆写该宏以适配硬件放置方向。
 */
#ifndef ADC_JOYSTICK_DEFAULT_ORIENTATION
#define ADC_JOYSTICK_DEFAULT_ORIENTATION ADC_JOYSTICK_ORIENTATION_180_DEG
#endif

/**
 * @brief 单个摇杆轴的快照
 */
typedef struct
{
    uint16_t raw;        /**< ADC 原始值（0..4095） */
    uint16_t permille;   /**< 位置缩放到 0..1000（0.1% 分辨率） */
} adc_joystick_axis_sample_t;

/**
 * @brief 摇杆 X/Y 双轴的快照
 */
typedef struct
{
    adc_joystick_axis_sample_t x; /**< X 轴数据（ADC 通道 14） */
    adc_joystick_axis_sample_t y; /**< Y 轴数据（ADC 通道 15） */
    uint8_t key_pressed;          /**< 摇杆按键被按下时为 1 */
} adc_joystick_sample_t;

/**
 * @brief     采集一次摇杆数据（双轴加按键）。
 * @param[out] sample 填充原始计数、千分比以及按键标志。
 * @return    状态码
 *            - 0 成功
 *            - 1 采集失败
 */
uint8_t adc_joystick_test_sample(adc_joystick_sample_t *sample);

/**
 * @brief     持续采样并通过 printf 打印摇杆读数。
 * @param[in] sample_count 打印的采样次数，0 使用默认值。
 * @param[in] delay_ms     采样间隔（毫秒），0 使用默认值。
 * @return    状态码
 *            - 0 成功
 *            - 1 采集失败
 */
uint8_t adc_joystick_test_run(uint32_t sample_count, uint32_t delay_ms);

/**
 * @brief     持续采样摇杆，并通过 UART 发送小键盘方向码。
 * @param[in] sample_count 处理的采样次数，0 表示默认/持续运行。
 * @param[in] delay_ms     采样间隔（毫秒），0 使用默认值。
 * @param[in] huart        用于发送的已初始化 UART 句柄。
 * @return    状态码
 *            - 0 成功
 *            - 1 采集失败（摇杆采样错误）
 *            - 2 无效的 UART 句柄（空指针）
 *            - 3 UART 发送失败
 */
uint8_t adc_joystick_send_udlr_uart(uint32_t sample_count, uint32_t delay_ms, UART_HandleTypeDef *huart);


/**
 * @brief 读取摇杆按键状态。
 * @return 按下返回 1，否则返回 0
 */
uint8_t adc_joystick_test_read_key(void);
