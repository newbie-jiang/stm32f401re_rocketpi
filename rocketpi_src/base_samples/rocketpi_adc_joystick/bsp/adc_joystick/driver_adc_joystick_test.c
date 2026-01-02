/**
 * @file driver_adc_joystick_test.c
 * @brief 双轴模拟摇杆及按键的阻塞采样辅助函数。
 * @version 1.0.0
 * @date 2025-11-16
 *
 * 本模块提供三项能力：
 *   1. 获取 X/Y 轴的原始值与千分比缩放值。
 *   2. 打印格式化的遥测数据，方便手动调试。
 *   3. 将摇杆偏转转换为小键盘风格的 UART 码（4/6/8/5）。
 */
#include "driver_adc_joystick_test.h"

#include <stdio.h>
#include "gpio.h"

/** @brief hadc1 采集的 12 位 ADC 最大值。 */
#define ADC_JOYSTICK_TEST_ADC_MAX_VALUE   4095U

static uint16_t adc_joystick_test_scale_permille(uint16_t raw);
static void adc_joystick_test_map_direction(const adc_joystick_sample_t *sample, char *out, uint32_t *n);

/**
 * @brief 将 ADC 原始值转换为 0..1000 的千分比（0.1% 分辨率）。
 * @param raw hadc1 返回的 ADC 原始值。
 * @return 千分比缩放值。
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
 * @brief 检查摇杆按键是否被按下。
 * @return 按下返回 1（引脚为低），否则返回 0。
 */
static uint8_t adc_joystick_test_read_key_internal(void)
{
    GPIO_PinState state = HAL_GPIO_ReadPin(ADC_JOYSTICK_KEY_GPIO_Port, ADC_JOYSTICK_KEY_Pin);

    return (state == GPIO_PIN_RESET) ? 1U : 0U;
}

/**
 * @brief 获取摇杆两个轴的原始值、千分比和按键状态。
 * @param[out] sample 接收原始计数、千分比和按键标志。
 * @return 成功返回 0，任一 ADC 操作失败返回 1。
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
 * @brief 将摇杆采样打印到控制台，便于人工观察。
 * @param sample_count 采样次数，传入 0 则使用默认值。
 * @param delay_ms     采样间隔（毫秒），传入 0 则使用默认值。
 * @return 全部采样成功返回 0，ADC 失败返回 1。
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

/** @brief 基于实验数据的 ADC 阈值，用于判定上下左右。 */
#define ADC_X_LEFT_THR    1600U  /**< X 轴偏低判定为左。 */
#define ADC_X_RIGHT_THR   2900U  /**< X 轴偏高判定为右。 */
#define ADC_Y_DOWN_THR    2900U  /**< Y 轴偏高判定为下。 */
#define ADC_Y_UP_THR      1600U  /**< Y 轴偏低判定为上。 */

/**
 * @brief 根据安装方向将物理方向映射为逻辑的左/右/上/下码。
 * @param[in]  sample  当前摇杆采样。
 * @param[out] out     方向输出缓冲区。
 * @param[in,out] n    当前已写入的长度，会根据触发的方向递增。
 */
static void adc_joystick_test_map_direction(const adc_joystick_sample_t *sample, char *out, uint32_t *n)
{
    const uint8_t phys_left  = (sample->x.raw < ADC_X_LEFT_THR)  ? 1U : 0U;
    const uint8_t phys_right = (sample->x.raw > ADC_X_RIGHT_THR) ? 1U : 0U;
    const uint8_t phys_down  = (sample->y.raw > ADC_Y_DOWN_THR)  ? 1U : 0U;
    const uint8_t phys_up    = (sample->y.raw < ADC_Y_UP_THR)    ? 1U : 0U;

#if ADC_JOYSTICK_DEFAULT_ORIENTATION == ADC_JOYSTICK_ORIENTATION_0_DEG
    const uint8_t logical_left  = phys_left;
    const uint8_t logical_right = phys_right;
    const uint8_t logical_up    = phys_up;
    const uint8_t logical_down  = phys_down;
#elif ADC_JOYSTICK_DEFAULT_ORIENTATION == ADC_JOYSTICK_ORIENTATION_90_DEG
    /* 顺时针旋转 90 度：物理左->上，右->下，上->右，下->左。 */
    const uint8_t logical_left  = phys_down;
    const uint8_t logical_right = phys_up;
    const uint8_t logical_up    = phys_left;
    const uint8_t logical_down  = phys_right;
#elif ADC_JOYSTICK_DEFAULT_ORIENTATION == ADC_JOYSTICK_ORIENTATION_180_DEG
    /* 顺时针旋转 180 度：左右、上下互换。 */
    const uint8_t logical_left  = phys_right;
    const uint8_t logical_right = phys_left;
    const uint8_t logical_up    = phys_down;
    const uint8_t logical_down  = phys_up;
#elif ADC_JOYSTICK_DEFAULT_ORIENTATION == ADC_JOYSTICK_ORIENTATION_270_DEG
    /* 顺时针旋转 270 度：物理左->下，右->上，上->左，下->右。 */
    const uint8_t logical_left  = phys_up;
    const uint8_t logical_right = phys_down;
    const uint8_t logical_up    = phys_right;
    const uint8_t logical_down  = phys_left;
#else
#error "ADC_JOYSTICK_DEFAULT_ORIENTATION 配置无效，请检查宏定义。"
#endif

    if (logical_left  != 0U) { out[(*n)++] = '4'; }
    if (logical_right != 0U) { out[(*n)++] = '6'; }
    if (logical_up    != 0U) { out[(*n)++] = '8'; }
    if (logical_down  != 0U) { out[(*n)++] = '5'; }
}

/**
 * @brief 持续发送代表方向的 U/D/L/R ASCII 码。
 * @param sample_count 要发送的采样次数，0 表示持续运行。
 * @param delay_ms     采样间隔（毫秒），0 表示使用默认值。
 * @param huart        用于发送数据的 UART 句柄。
 * @return 返回 0 成功；1 采样失败；2 UART 句柄非法；3 UART 发送失败。
 */
uint8_t adc_joystick_send_udlr_uart(uint32_t sample_count, uint32_t delay_ms, UART_HandleTypeDef *huart)
{
    const uint32_t delay = (delay_ms == 0U) ? ADC_JOYSTICK_TEST_DEFAULT_DELAY_MS : delay_ms;

    if (huart == NULL) {
        return 2U;
    }

    if (sample_count == 0U)
    {
        /* 0 表示一直运行，便于快速交互测试。 */
        while (1)
        {
            adc_joystick_sample_t sample;

            if (adc_joystick_test_sample(&sample) != 0U) {
                return 1U;
            }

            char out[4];
            uint32_t n = 0U;

            /* 方向映射遵循小键盘：4=左，6=右，8=上，5=下，宏可配置旋转角度。 */
            adc_joystick_test_map_direction(&sample, out, &n);
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
        /* 有限运行：按固定次数采样。 */
        for (uint32_t i = 0U; i < sample_count; ++i)
        {
            adc_joystick_sample_t sample;

            if (adc_joystick_test_sample(&sample) != 0U) {
                return 1U;
            }

            char out[4];
            uint32_t n = 0U;

            adc_joystick_test_map_direction(&sample, out, &n);
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
 * @brief 仅读取摇杆按键状态的便捷封装。
 * @return 按下返回 1，否则返回 0。
 */
uint8_t adc_joystick_test_read_key(void)
{
    return adc_joystick_test_read_key_internal();
}
