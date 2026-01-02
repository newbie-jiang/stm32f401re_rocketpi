/**
 * @file driver_mg58f18_radar.h
 * @brief MG58F18 微波雷达的协议层 API（参考《通信协议.txt》）。
 *
 * 主要完成协议相关的帧封装、校验和解析，处理参数合法性；具体的 UART/DMA
 * 收发由 driver_mg58f18_radar_interface.c 适配到底层硬件。
 */

#ifndef DRIVER_MG58F18_RADAR_H
#define DRIVER_MG58F18_RADAR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define MG58F18_RADAR_FRAME_SIZE            7U   /**< 协议固定帧长：5A + 5 字节载荷 + FE */
#define MG58F18_RADAR_HEAD_BYTE             0x5AU /**< 帧头（Head） */
#define MG58F18_RADAR_TAIL_BYTE             0xFEU /**< 帧尾（Tail） */
#define MG58F18_RADAR_RX_BUFFER_SIZE        64U  /**< DMA 接收环形缓冲区大小 */
#define MG58F18_RADAR_DEFAULT_TIMEOUT_MS    150U /**< 发送等待应答的默认超时（毫秒） */

typedef enum
{
    MG58F18_RADAR_CMD_SET_DISTANCE_THRESHOLD       = 0x01, /**< 设置感应距离阈值，Data3/Data4=高/低 8 位 */
    MG58F18_RADAR_CMD_QUERY_DISTANCE_THRESHOLD     = 0x81, /**< 查询感应距离阈值 */
    MG58F18_RADAR_CMD_SET_DELAY_TIME               = 0x02, /**< 设置输出保持/延迟时间，单位 raw/32ms */
    MG58F18_RADAR_CMD_QUERY_DELAY_TIME             = 0x82, /**< 查询输出保持/延迟时间 */
    MG58F18_RADAR_CMD_SET_LIGHT_SENSOR_ENABLE      = 0x03, /**< 开/关光感参与报警（Data4:0 关，1 开） */
    MG58F18_RADAR_CMD_QUERY_LIGHT_SENSOR_ENABLE    = 0x83, /**< 查询光感使能状态 */
    MG58F18_RADAR_CMD_SET_BLOCK_TIME               = 0x04, /**< 设置遮挡/屏蔽时间，单位 raw/32ms */
    MG58F18_RADAR_CMD_QUERY_BLOCK_TIME             = 0x84, /**< 查询遮挡/屏蔽时间 */
    MG58F18_RADAR_CMD_SET_ACTIVE_LEVEL             = 0x05, /**< 设置 OUT 有效电平（0 低有效，1 高有效） */
    MG58F18_RADAR_CMD_QUERY_ACTIVE_LEVEL           = 0x85, /**< 查询 OUT 有效电平 */
    MG58F18_RADAR_CMD_SET_POWER_MODE               = 0x06, /**< 设置功耗模式（0 超低功耗 50/60uA，1 增强 13mA） */
    MG58F18_RADAR_CMD_QUERY_POWER_MODE             = 0x86, /**< 查询功耗模式 */
    MG58F18_RADAR_CMD_QUERY_TRIGGER_STATE          = 0x87, /**< 查询当前是否有触发 */
    MG58F18_RADAR_CMD_QUERY_LIGHT_ENVIRONMENT      = 0x88, /**< 查询当前环境（0 白天，1 黑夜） */
    MG58F18_RADAR_CMD_QUERY_FIRMWARE_VERSION       = 0x89, /**< 查询固件版本号（data4 高 4 位=主版本，低 4 位=次版本） */
    MG58F18_RADAR_CMD_SET_TRIGGER_MODE             = 0x0A, /**< 设置触发模式（0 连续触发，1 单次触发） */
    MG58F18_RADAR_CMD_QUERY_TRIGGER_MODE           = 0x8A, /**< 查询触发模式 */
    MG58F18_RADAR_CMD_SET_TX_POWER_STEP            = 0x0B, /**< 设置发射功率档位 0~7（档位越小功率越大、距离越远） */
    MG58F18_RADAR_CMD_QUERY_TX_POWER_STEP          = 0x8B, /**< 查询发射功率档位 */
    MG58F18_RADAR_CMD_SET_LIGHT_SENSOR_THRESHOLD   = 0x0C, /**< 设置光感阈值，0x00~0xFF（越小越灵敏） */
    MG58F18_RADAR_CMD_QUERY_LIGHT_SENSOR_THRESHOLD = 0x8C, /**< 查询光感阈值 */
    MG58F18_RADAR_CMD_SET_PWM_ENABLE               = 0x0D, /**< 开关 PWM 输出（0 关，1 开） */
    MG58F18_RADAR_CMD_QUERY_PWM_ENABLE             = 0x8D, /**< 查询 PWM 使能状态 */
    MG58F18_RADAR_CMD_SET_PWM_DUTY                 = 0x0E, /**< 设置 PWM 占空比 raw，默认范围 0~0x0DAC */
    MG58F18_RADAR_CMD_QUERY_PWM_DUTY               = 0x8E, /**< 查询 PWM 占空比 raw */
    MG58F18_RADAR_CMD_SET_POWER_PULSE_WIDTH        = 0x0F, /**< 设置脉冲宽度（0x00~0xFF，越大穿透越强） */
    MG58F18_RADAR_CMD_QUERY_POWER_PULSE_WIDTH      = 0x8F, /**< 查询脉冲宽度 */
    MG58F18_RADAR_CMD_SET_SENSING_MODE             = 0x10, /**< 选择响应模式（0 移动检测，1 手势/近距离检测） */
    MG58F18_RADAR_CMD_QUERY_SENSING_MODE           = 0x11, /**< 查询响应模式 */
    MG58F18_RADAR_CMD_SAVE_SETTINGS                = 0x20  /**< 保存当前配置到模组 Flash（需等待 100ms） */
} mg58f18_radar_command_t;

typedef enum
{
    MG58F18_RADAR_STATUS_OK = 0,              /**< 成功 */
    MG58F18_RADAR_STATUS_NOT_INITIALISED,     /**< 驱动未初始化 */
    MG58F18_RADAR_STATUS_BUSY,                /**< 底层 UART/DMA 忙 */
    MG58F18_RADAR_STATUS_INVALID_ARGUMENT,    /**< 入参越界或空指针 */
    MG58F18_RADAR_STATUS_TIMEOUT,             /**< 等待模组应答超时 */
    MG58F18_RADAR_STATUS_FRAME_ERROR,         /**< 校验和/尾字节异常 */
    MG58F18_RADAR_STATUS_HAL_ERROR            /**< 底层 HAL 返回错误 */
} mg58f18_radar_status_t;

typedef struct
{
    uint8_t raw[MG58F18_RADAR_FRAME_SIZE]; /**< 原始帧缓存：5A | cmd | data2 | data3 | data4 | xor | FE */
    uint8_t command;                       /**< Data1：命令码 */
    uint8_t data2;                         /**< Data2：载荷高 8 位/保留字段 */
    uint8_t data3;                         /**< Data3：载荷中 8 位 */
    uint8_t data4;                         /**< Data4：载荷低 8 位/枚举值 */
    uint8_t checksum;                      /**< 校验和（cmd^data2^data3^data4） */
} mg58f18_radar_frame_t;

/**
 * @brief 初始化雷达协议栈并启动 DMA 接收（默认使用 USART1）。
 */
mg58f18_radar_status_t mg58f18_radar_init(void);
void mg58f18_radar_deinit(void);

/**
 * @brief 设置/查询感应距离阈值（100~65000，值越小越远）。
 */
mg58f18_radar_status_t mg58f18_radar_set_distance_threshold(uint16_t threshold);
mg58f18_radar_status_t mg58f18_radar_get_distance_threshold(uint16_t *threshold);

/**
 * @brief 设置/查询输出保持延迟（raw/32=毫秒，0~0xFFFFFF）。
 */
mg58f18_radar_status_t mg58f18_radar_set_delay_ms(uint32_t delay_ms);
mg58f18_radar_status_t mg58f18_radar_get_delay_ms(uint32_t *delay_ms);

/**
 * @brief 使能/关闭光感参与触发（0 关闭，1 打开）。
 */
mg58f18_radar_status_t mg58f18_radar_set_light_sensor_enabled(bool enable);
mg58f18_radar_status_t mg58f18_radar_get_light_sensor_enabled(bool *enabled);

/**
 * @brief 设置/查询屏蔽时间（raw/32=毫秒，报警后暂停检测的时长）。
 */
mg58f18_radar_status_t mg58f18_radar_set_block_time_ms(uint32_t block_time_ms);
mg58f18_radar_status_t mg58f18_radar_get_block_time_ms(uint32_t *block_time_ms);

/**
 * @brief 设置/查询 OUT 有效极性（0 低电平有效，1 高电平有效）。
 */
mg58f18_radar_status_t mg58f18_radar_set_active_level(bool high_active);
mg58f18_radar_status_t mg58f18_radar_get_active_level(bool *high_active);

/**
 * @brief 设置/查询功耗模式（0=超低功耗 50/60uA，1=增强 13mA）。
 */
mg58f18_radar_status_t mg58f18_radar_set_power_mode(bool normal_mode);
mg58f18_radar_status_t mg58f18_radar_get_power_mode(bool *normal_mode);

/**
 * @brief 查询当前触发/环境/固件信息。
 */
mg58f18_radar_status_t mg58f18_radar_get_trigger_state(bool *triggered);
mg58f18_radar_status_t mg58f18_radar_get_light_environment(bool *is_night);
mg58f18_radar_status_t mg58f18_radar_get_firmware_version(uint8_t *major, uint8_t *minor);

/**
 * @brief 设置/查询触发模式（0 连续，1 单次）及功率档位 0~7。
 */
mg58f18_radar_status_t mg58f18_radar_set_trigger_mode(bool single_trigger);
mg58f18_radar_status_t mg58f18_radar_get_trigger_mode(bool *single_trigger);

mg58f18_radar_status_t mg58f18_radar_set_power_step(uint8_t step);
mg58f18_radar_status_t mg58f18_radar_get_power_step(uint8_t *step);

/**
 * @brief 设置/查询光感阈值（0x00~0xFF，越小越灵敏）。
 */
mg58f18_radar_status_t mg58f18_radar_set_light_threshold(uint8_t threshold);
mg58f18_radar_status_t mg58f18_radar_get_light_threshold(uint8_t *threshold);

/**
 * @brief 设置/查询 PWM 使能及占空比 raw（默认 0~0x0DAC）。
 */
mg58f18_radar_status_t mg58f18_radar_set_pwm_enabled(bool enable);
mg58f18_radar_status_t mg58f18_radar_get_pwm_enabled(bool *enable);

mg58f18_radar_status_t mg58f18_radar_set_pwm_duty_raw(uint16_t duty_raw);
mg58f18_radar_status_t mg58f18_radar_get_pwm_duty_raw(uint16_t *duty_raw);

/**
 * @brief 设置/查询脉冲宽度（0x00~0xFF，值越大穿透/灵敏度越高）。
 */
mg58f18_radar_status_t mg58f18_radar_set_power_pulse_width(uint8_t width);
mg58f18_radar_status_t mg58f18_radar_get_power_pulse_width(uint8_t *width);

/**
 * @brief 设置/查询响应模式（0 移动检测，1 手势/近距离检测）。
 */
mg58f18_radar_status_t mg58f18_radar_set_sensing_mode(bool hand_mode);
mg58f18_radar_status_t mg58f18_radar_get_sensing_mode(bool *hand_mode);

/**
 * @brief 将当前参数烧录到模组 Flash（协议建议写入后等待 >=100ms）。
 */
mg58f18_radar_status_t mg58f18_radar_save_settings(void);

/**
 * @brief 直接读取雷达 OUT 引脚的当前电平。
 *
 * @param[out] active GPIO 为高电平时返回 true。
 * @return active 为空返回 MG58F18_RADAR_STATUS_INVALID_ARGUMENT；
 *         驱动未初始化返回 MG58F18_RADAR_STATUS_NOT_INITIALISED；
 *         否则返回 MG58F18_RADAR_STATUS_OK。
 */
mg58f18_radar_status_t mg58f18_radar_read_io(bool *active);

/**
 * @brief 拉取最新一帧应答数据（有则拷贝到 @p frame 并消费）。
 */
bool mg58f18_radar_fetch_frame(mg58f18_radar_frame_t *frame);
/**
 * @brief 以人类可读格式打印一帧应答。
 */
void mg58f18_radar_print_frame(const mg58f18_radar_frame_t *frame);
/**
 * @brief 轮询缓存区内的所有帧并逐条打印。
 */
void mg58f18_radar_poll_and_print(void);
/**
 * @brief 将状态码转为字符串（调试打印用）。
 */
const char *mg58f18_radar_status_string(mg58f18_radar_status_t status);

/**
 * @brief DMA 回调向协议层馈入收到的字节流。
 */
void mg58f18_radar_receive_bytes(const uint8_t *data, size_t length);

#ifdef __cplusplus
}
#endif

#endif /* DRIVER_MG58F18_RADAR_H */
