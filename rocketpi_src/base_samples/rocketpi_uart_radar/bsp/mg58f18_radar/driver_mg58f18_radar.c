/**
 * @file driver_mg58f18_radar.c
 * @brief MG58F18 雷达的协议解析与命令封装实现。
 *
 * 只关心协议语义（帧拼装/校验/解析/参数检查），硬件 UART/DMA 由接口层负责。
 */
#include "driver_mg58f18_radar.h"

#include <stdio.h>
#include <string.h>

#include "driver_mg58f18_radar_interface.h"

#define MG58F18_RADAR_CHECKSUM_INDEX 5U  /**< Data1^Data2^Data3^Data4 的放置位置 */
#define MG58F18_RADAR_TAIL_INDEX     6U  /**< 帧尾 FE 的偏移 */
#define MG58F18_RADAR_DELAY_SCALER   32U /**< 协议时间单位换算：raw/32 = 毫秒 */

/**
 * @brief 协议核心状态：记录收发上下文、解析状态机和最新帧。
 */
typedef struct
{
    uint8_t pending_command;      /**< 正在等待应答的命令码 */
    bool awaiting_reply;          /**< 是否正在等模组回复 */
    bool frame_available;         /**< 是否有未消费的完整帧 */
    mg58f18_radar_status_t last_error; /**< 最近一次解析/交互的状态 */
    mg58f18_radar_frame_t last_frame;  /**< 最近收到的有效帧 */
    struct
    {
        uint8_t buffer[MG58F18_RADAR_FRAME_SIZE]; /**< 接收缓冲 */
        uint8_t index;                            /**< 当前写入位置 */
    } parser;
} mg58f18_radar_core_t;

static mg58f18_radar_core_t s_core;
static bool s_initialised = false;

/**
 * @brief 计算协议规定的异或校验。
 */
static inline uint8_t mg58f18_radar_compute_checksum(uint8_t command,
                                                     uint8_t data2,
                                                     uint8_t data3,
                                                     uint8_t data4)
{
    return (uint8_t)(command ^ data2 ^ data3 ^ data4);
}

/**
 * @brief 清空解析状态机。
 */
static void mg58f18_radar_core_reset_parser(mg58f18_radar_core_t *ctx)
{
    ctx->parser.index = 0U;
    memset(ctx->parser.buffer, 0, sizeof(ctx->parser.buffer));
}

/**
 * @brief 将解析到的原始帧搬运到对外可读结构体。
 */
static void mg58f18_radar_core_store_frame(mg58f18_radar_core_t *ctx,
                                           const uint8_t buffer[MG58F18_RADAR_FRAME_SIZE])
{
    memcpy(ctx->last_frame.raw, buffer, MG58F18_RADAR_FRAME_SIZE);
    ctx->last_frame.command  = buffer[1];
    ctx->last_frame.data2    = buffer[2];
    ctx->last_frame.data3    = buffer[3];
    ctx->last_frame.data4    = buffer[4];
    ctx->last_frame.checksum = buffer[MG58F18_RADAR_CHECKSUM_INDEX];
    ctx->frame_available     = true;
}

/**
 * @brief 初始化核心上下文，默认状态为 OK。
 */
static void mg58f18_radar_core_init(mg58f18_radar_core_t *ctx)
{
    memset(ctx, 0, sizeof(*ctx));
    ctx->last_error = MG58F18_RADAR_STATUS_OK;
}

/**
 * @brief 发送前标记待匹配的命令，并预置超时状态。
 */
static void mg58f18_radar_core_begin_transaction(mg58f18_radar_core_t *ctx, uint8_t command)
{
    ctx->pending_command = command;
    ctx->awaiting_reply  = true;
    ctx->last_error      = MG58F18_RADAR_STATUS_TIMEOUT;
}

/**
 * @brief 手动终止一次事务并写入错误码。
 */
static void mg58f18_radar_core_abort(mg58f18_radar_core_t *ctx, mg58f18_radar_status_t reason)
{
    ctx->awaiting_reply = false;
    ctx->last_error     = reason;
}

/**
 * @brief 等待超时时的统一处理。
 */
static void mg58f18_radar_core_mark_timeout(mg58f18_radar_core_t *ctx)
{
    mg58f18_radar_core_abort(ctx, MG58F18_RADAR_STATUS_TIMEOUT);
}

/**
 * @brief 单字节推进解析状态机，识别完整帧并校验。
 */
static mg58f18_radar_status_t mg58f18_radar_core_process_byte(mg58f18_radar_core_t *ctx, uint8_t byte)
{
    if (ctx->parser.index == 0U)
    {
        if (byte != MG58F18_RADAR_HEAD_BYTE)
        {
            return ctx->last_error;
        }
        ctx->parser.buffer[ctx->parser.index++] = byte;
        return ctx->last_error;
    }

    ctx->parser.buffer[ctx->parser.index++] = byte;
    if (ctx->parser.index < MG58F18_RADAR_FRAME_SIZE)
    {
        return ctx->last_error;
    }

    const uint8_t checksum = mg58f18_radar_compute_checksum(ctx->parser.buffer[1],
                                                            ctx->parser.buffer[2],
                                                            ctx->parser.buffer[3],
                                                            ctx->parser.buffer[4]);
    const bool checksum_ok = (checksum == ctx->parser.buffer[MG58F18_RADAR_CHECKSUM_INDEX]);
    const bool tail_ok     = (ctx->parser.buffer[MG58F18_RADAR_TAIL_INDEX] == MG58F18_RADAR_TAIL_BYTE);

    if (checksum_ok && tail_ok)
    {
        mg58f18_radar_core_store_frame(ctx, ctx->parser.buffer);

        if (ctx->awaiting_reply && ctx->last_frame.command == ctx->pending_command)
        {
            ctx->awaiting_reply = false;
            ctx->last_error     = MG58F18_RADAR_STATUS_OK;
        }
        else
        {
            ctx->last_error = MG58F18_RADAR_STATUS_OK;
        }
    }
    else
    {
        ctx->last_error = MG58F18_RADAR_STATUS_FRAME_ERROR;
    }

    const bool restart = (ctx->parser.buffer[MG58F18_RADAR_TAIL_INDEX] == MG58F18_RADAR_HEAD_BYTE);
    mg58f18_radar_core_reset_parser(ctx);
    if (restart)
    {
        ctx->parser.buffer[0] = MG58F18_RADAR_HEAD_BYTE;
        ctx->parser.index     = 1U;
    }
    return ctx->last_error;
}

static mg58f18_radar_status_t mg58f18_radar_core_process_bytes(mg58f18_radar_core_t *ctx,
                                                               const uint8_t *data,
                                                               size_t length)
{
    /* DMA 回调可能一次上报多字节，这里逐个推进状态机 */
    if (data == NULL || length == 0U)
    {
        return ctx->last_error;
    }

    mg58f18_radar_status_t status = ctx->last_error;
    for (size_t i = 0U; i < length; ++i)
    {
        status = mg58f18_radar_core_process_byte(ctx, data[i]);
    }
    return status;
}

static bool mg58f18_radar_core_is_awaiting_reply(const mg58f18_radar_core_t *ctx)
{
    return ctx->awaiting_reply;
}

static mg58f18_radar_status_t mg58f18_radar_core_get_last_error(const mg58f18_radar_core_t *ctx)
{
    return ctx->last_error;
}

static const mg58f18_radar_frame_t *mg58f18_radar_core_last_frame(const mg58f18_radar_core_t *ctx)
{
    return ctx->frame_available ? &ctx->last_frame : NULL;
}

/**
 * @brief 轮询等待应答，直到解析线程清除 awaiting_reply 标志或超时。
 */
static mg58f18_radar_status_t mg58f18_radar_wait_for_reply(uint32_t timeout_ms)
{
    const uint32_t start = mg58f18_radar_interface_hw_get_tick();
    while (mg58f18_radar_core_is_awaiting_reply(&s_core))
    {
        const uint32_t now = mg58f18_radar_interface_hw_get_tick();
        if ((now - start) > timeout_ms)
        {
            mg58f18_radar_core_mark_timeout(&s_core);
            return MG58F18_RADAR_STATUS_TIMEOUT;
        }
    }
    return mg58f18_radar_core_get_last_error(&s_core);
}

/**
 * @brief 组帧 + 发送 + 等待应答的通用流程。
 */
static mg58f18_radar_status_t mg58f18_radar_send_command_raw(uint8_t command,
                                                             uint8_t data2,
                                                             uint8_t data3,
                                                             uint8_t data4,
                                                             mg58f18_radar_frame_t *response)
{
    if (!s_initialised)
    {
        return MG58F18_RADAR_STATUS_NOT_INITIALISED;
    }

    uint8_t frame[MG58F18_RADAR_FRAME_SIZE];
    frame[0] = MG58F18_RADAR_HEAD_BYTE;
    frame[1] = command;
    frame[2] = data2;
    frame[3] = data3;
    frame[4] = data4;
    frame[MG58F18_RADAR_CHECKSUM_INDEX] = mg58f18_radar_compute_checksum(command, data2, data3, data4);
    frame[MG58F18_RADAR_TAIL_INDEX]     = MG58F18_RADAR_TAIL_BYTE;

    mg58f18_radar_core_begin_transaction(&s_core, command);

    mg58f18_radar_status_t status =
        mg58f18_radar_interface_hw_send(frame, MG58F18_RADAR_FRAME_SIZE, MG58F18_RADAR_DEFAULT_TIMEOUT_MS);
    if (status != MG58F18_RADAR_STATUS_OK)
    {
        mg58f18_radar_core_abort(&s_core, status);
        return status;
    }

    status = mg58f18_radar_wait_for_reply(MG58F18_RADAR_DEFAULT_TIMEOUT_MS);
    if (status != MG58F18_RADAR_STATUS_OK)
    {
        return status;
    }

    const mg58f18_radar_frame_t *last = mg58f18_radar_core_last_frame(&s_core);
    if (last == NULL || last->command != command)
    {
        return MG58F18_RADAR_STATUS_FRAME_ERROR;
    }

    if (response != NULL)
    {
        *response = *last;
    }
    return status;
}

/**
 * @brief 将 24bit 原始值按协议高/中/低字节拆分并发送。
 */
static mg58f18_radar_status_t mg58f18_radar_send_command_u24(uint8_t command,
                                                             uint32_t value,
                                                             mg58f18_radar_frame_t *response)
{
    const uint8_t data2 = (uint8_t)((value >> 16) & 0xFFU);
    const uint8_t data3 = (uint8_t)((value >> 8) & 0xFFU);
    const uint8_t data4 = (uint8_t)(value & 0xFFU);
    return mg58f18_radar_send_command_raw(command, data2, data3, data4, response);
}

/**
 * @brief 发送命令并要求模组回显相同载荷，用于参数写入校验。
 */
static mg58f18_radar_status_t mg58f18_radar_expect_echo(uint8_t command,
                                                         uint8_t data2,
                                                         uint8_t data3,
                                                         uint8_t data4)
{
    mg58f18_radar_frame_t frame;
    const mg58f18_radar_status_t status =
        mg58f18_radar_send_command_raw(command, data2, data3, data4, &frame);
    if (status != MG58F18_RADAR_STATUS_OK)
    {
        return status;
    }

    if (frame.data2 != data2 || frame.data3 != data3 || frame.data4 != data4)
    {
        return MG58F18_RADAR_STATUS_FRAME_ERROR;
    }
    return MG58F18_RADAR_STATUS_OK;
}

/**
 * @brief 按协议拼出 24bit 整数（Data2:高 8 位）。
 */
uint32_t mg58f18_radar_frame_get_u24(const mg58f18_radar_frame_t *frame)
{
    if (frame == NULL)
    {
        return 0U;
    }

    return ((uint32_t)frame->data2 << 16)
         | ((uint32_t)frame->data3 << 8)
         | frame->data4;
}

/**
 * @brief 初始化协议栈并启动 DMA 接收。
 */
mg58f18_radar_status_t mg58f18_radar_init(void)
{
    mg58f18_radar_core_init(&s_core);
    mg58f18_radar_status_t status = mg58f18_radar_interface_hw_init();
    if (status != MG58F18_RADAR_STATUS_OK)
    {
        return status;
    }
    s_initialised = true;
    mg58f18_radar_interface_hw_restart_rx();
    return MG58F18_RADAR_STATUS_OK;
}

/**
 * @brief 反初始化协议栈并重置解析状态。
 */
void mg58f18_radar_deinit(void)
{
    s_initialised = false;
    mg58f18_radar_core_init(&s_core);
}

/**
 * @brief 设置感应距离阈值（100~65000，越小越远）。
 */
mg58f18_radar_status_t mg58f18_radar_set_distance_threshold(uint16_t threshold)
{
    if (threshold < 100U || threshold > 65000U)
    {
        return MG58F18_RADAR_STATUS_INVALID_ARGUMENT;
    }

    const uint8_t data3 = (uint8_t)((threshold >> 8) & 0xFFU);
    const uint8_t data4 = (uint8_t)(threshold & 0xFFU);
    return mg58f18_radar_expect_echo(MG58F18_RADAR_CMD_SET_DISTANCE_THRESHOLD, 0x00U, data3, data4);
}

/**
 * @brief 查询感应距离阈值。
 */
mg58f18_radar_status_t mg58f18_radar_get_distance_threshold(uint16_t *threshold)
{
    mg58f18_radar_frame_t frame;
    const mg58f18_radar_status_t status =
        mg58f18_radar_send_command_raw(MG58F18_RADAR_CMD_QUERY_DISTANCE_THRESHOLD, 0x00U, 0x00U, 0x00U, &frame);
    if (status != MG58F18_RADAR_STATUS_OK)
    {
        return status;
    }
    if (threshold != NULL)
    {
        *threshold = (uint16_t)(((uint16_t)frame.data3 << 8) | frame.data4);
    }
    return MG58F18_RADAR_STATUS_OK;
}

/**
 * @brief 设置输出保持/延迟时间（ms，协议单位 raw/32）。
 */
mg58f18_radar_status_t mg58f18_radar_set_delay_ms(uint32_t delay_ms)
{
    const uint32_t max_ms = 0xFFFFFFUL / MG58F18_RADAR_DELAY_SCALER;
    if (delay_ms > max_ms)
    {
        return MG58F18_RADAR_STATUS_INVALID_ARGUMENT;
    }

    const uint32_t value = delay_ms * MG58F18_RADAR_DELAY_SCALER;
    mg58f18_radar_frame_t frame;
    const mg58f18_radar_status_t status =
        mg58f18_radar_send_command_u24(MG58F18_RADAR_CMD_SET_DELAY_TIME, value, &frame);
    if (status != MG58F18_RADAR_STATUS_OK)
    {
        return status;
    }
    return (mg58f18_radar_frame_get_u24(&frame) == value)
               ? MG58F18_RADAR_STATUS_OK
               : MG58F18_RADAR_STATUS_FRAME_ERROR;
}

/**
 * @brief 查询输出保持/延迟时间（ms）。
 */
mg58f18_radar_status_t mg58f18_radar_get_delay_ms(uint32_t *delay_ms)
{
    mg58f18_radar_frame_t frame;
    const mg58f18_radar_status_t status =
        mg58f18_radar_send_command_raw(MG58F18_RADAR_CMD_QUERY_DELAY_TIME, 0x00U, 0x00U, 0x00U, &frame);
    if (status != MG58F18_RADAR_STATUS_OK)
    {
        return status;
    }
    if (delay_ms != NULL)
    {
        const uint32_t raw = mg58f18_radar_frame_get_u24(&frame);
        *delay_ms = raw / MG58F18_RADAR_DELAY_SCALER;
    }
    return MG58F18_RADAR_STATUS_OK;
}

/**
 * @brief 设置光感是否参与触发（false 关，true 开）。
 */
mg58f18_radar_status_t mg58f18_radar_set_light_sensor_enabled(bool enable)
{
    return mg58f18_radar_expect_echo(MG58F18_RADAR_CMD_SET_LIGHT_SENSOR_ENABLE,
                                     0x00U,
                                     0x00U,
                                     enable ? 0x01U : 0x00U);
}

/**
 * @brief 查询光感是否参与触发。
 */
mg58f18_radar_status_t mg58f18_radar_get_light_sensor_enabled(bool *enabled)
{
    mg58f18_radar_frame_t frame;
    const mg58f18_radar_status_t status =
        mg58f18_radar_send_command_raw(MG58F18_RADAR_CMD_QUERY_LIGHT_SENSOR_ENABLE, 0x00U, 0x00U, 0x00U, &frame);
    if (status != MG58F18_RADAR_STATUS_OK)
    {
        return status;
    }
    if (enabled != NULL)
    {
        *enabled = (frame.data4 == 0x01U);
    }
    return MG58F18_RADAR_STATUS_OK;
}

/**
 * @brief 设置报警后的屏蔽时间（ms，协议单位 raw/32）。
 */
mg58f18_radar_status_t mg58f18_radar_set_block_time_ms(uint32_t block_time_ms)
{
    const uint32_t max_ms = 0xFFFFFFUL / MG58F18_RADAR_DELAY_SCALER;
    if (block_time_ms > max_ms)
    {
        return MG58F18_RADAR_STATUS_INVALID_ARGUMENT;
    }
    const uint32_t value = block_time_ms * MG58F18_RADAR_DELAY_SCALER;
    mg58f18_radar_frame_t frame;
    const mg58f18_radar_status_t status =
        mg58f18_radar_send_command_u24(MG58F18_RADAR_CMD_SET_BLOCK_TIME, value, &frame);
    if (status != MG58F18_RADAR_STATUS_OK)
    {
        return status;
    }
    return (mg58f18_radar_frame_get_u24(&frame) == value)
               ? MG58F18_RADAR_STATUS_OK
               : MG58F18_RADAR_STATUS_FRAME_ERROR;
}

/**
 * @brief 查询屏蔽时间（ms）。
 */
mg58f18_radar_status_t mg58f18_radar_get_block_time_ms(uint32_t *block_time_ms)
{
    mg58f18_radar_frame_t frame;
    const mg58f18_radar_status_t status =
        mg58f18_radar_send_command_raw(MG58F18_RADAR_CMD_QUERY_BLOCK_TIME, 0x00U, 0x00U, 0x00U, &frame);
    if (status != MG58F18_RADAR_STATUS_OK)
    {
        return status;
    }
    if (block_time_ms != NULL)
    {
        const uint32_t raw = mg58f18_radar_frame_get_u24(&frame);
        *block_time_ms = raw / MG58F18_RADAR_DELAY_SCALER;
    }
    return MG58F18_RADAR_STATUS_OK;
}

/**
 * @brief 设置 OUT 有效极性（true 高有效，false 低有效）。
 */
mg58f18_radar_status_t mg58f18_radar_set_active_level(bool high_active)
{
    return mg58f18_radar_expect_echo(MG58F18_RADAR_CMD_SET_ACTIVE_LEVEL,
                                     0x00U,
                                     0x00U,
                                     high_active ? 0x01U : 0x00U);
}

/**
 * @brief 查询 OUT 有效极性。
 */
mg58f18_radar_status_t mg58f18_radar_get_active_level(bool *high_active)
{
    mg58f18_radar_frame_t frame;
    const mg58f18_radar_status_t status =
        mg58f18_radar_send_command_raw(MG58F18_RADAR_CMD_QUERY_ACTIVE_LEVEL, 0x00U, 0x00U, 0x00U, &frame);
    if (status != MG58F18_RADAR_STATUS_OK)
    {
        return status;
    }
    if (high_active != NULL)
    {
        *high_active = (frame.data4 == 0x01U);
    }
    return MG58F18_RADAR_STATUS_OK;
}

/**
 * @brief 设置功耗模式（true 增强 13mA，false 超低功耗 50/60uA）。
 */
mg58f18_radar_status_t mg58f18_radar_set_power_mode(bool normal_mode)
{
    return mg58f18_radar_expect_echo(MG58F18_RADAR_CMD_SET_POWER_MODE,
                                     0x00U,
                                     0x00U,
                                     normal_mode ? 0x01U : 0x00U);
}

/**
 * @brief 查询功耗模式。
 */
mg58f18_radar_status_t mg58f18_radar_get_power_mode(bool *normal_mode)
{
    mg58f18_radar_frame_t frame;
    const mg58f18_radar_status_t status =
        mg58f18_radar_send_command_raw(MG58F18_RADAR_CMD_QUERY_POWER_MODE, 0x00U, 0x00U, 0x00U, &frame);
    if (status != MG58F18_RADAR_STATUS_OK)
    {
        return status;
    }
    if (normal_mode != NULL)
    {
        *normal_mode = (frame.data4 == 0x01U);
    }
    return MG58F18_RADAR_STATUS_OK;
}

/**
 * @brief 查询当前触发状态（data4=1 表示触发）。
 */
mg58f18_radar_status_t mg58f18_radar_get_trigger_state(bool *triggered)
{
    mg58f18_radar_frame_t frame;
    const mg58f18_radar_status_t status =
        mg58f18_radar_send_command_raw(MG58F18_RADAR_CMD_QUERY_TRIGGER_STATE, 0x00U, 0x00U, 0x00U, &frame);
    if (status != MG58F18_RADAR_STATUS_OK)
    {
        return status;
    }
    if (triggered != NULL)
    {
        *triggered = (frame.data4 == 0x01U);
    }
    return MG58F18_RADAR_STATUS_OK;
}

/**
 * @brief 查询当前环境（true 夜间，false 白天）。
 */
mg58f18_radar_status_t mg58f18_radar_get_light_environment(bool *is_night)
{
    mg58f18_radar_frame_t frame;
    const mg58f18_radar_status_t status =
        mg58f18_radar_send_command_raw(MG58F18_RADAR_CMD_QUERY_LIGHT_ENVIRONMENT, 0x00U, 0x00U, 0x00U, &frame);
    if (status != MG58F18_RADAR_STATUS_OK)
    {
        return status;
    }
    if (is_night != NULL)
    {
        *is_night = (frame.data4 == 0x01U);
    }
    return MG58F18_RADAR_STATUS_OK;
}

/**
 * @brief 查询固件版本号（data4 高 4 位主版本，低 4 位次版本）。
 */
mg58f18_radar_status_t mg58f18_radar_get_firmware_version(uint8_t *major, uint8_t *minor)
{
    mg58f18_radar_frame_t frame;
    const mg58f18_radar_status_t status =
        mg58f18_radar_send_command_raw(MG58F18_RADAR_CMD_QUERY_FIRMWARE_VERSION, 0x00U, 0x00U, 0x00U, &frame);
    if (status != MG58F18_RADAR_STATUS_OK)
    {
        return status;
    }
    if (major != NULL)
    {
        *major = (uint8_t)((frame.data4 >> 4) & 0x0FU);
    }
    if (minor != NULL)
    {
        *minor = (uint8_t)(frame.data4 & 0x0FU);
    }
    return MG58F18_RADAR_STATUS_OK;
}
/**
 * @brief 设置触发模式（true 单次，false 连续）。
 */
mg58f18_radar_status_t mg58f18_radar_set_trigger_mode(bool single_trigger)
{
    return mg58f18_radar_expect_echo(MG58F18_RADAR_CMD_SET_TRIGGER_MODE,
                                     0x00U,
                                     0x00U,
                                     single_trigger ? 0x01U : 0x00U);
}
/**
 * @brief 查询触发模式。
 */
mg58f18_radar_status_t mg58f18_radar_get_trigger_mode(bool *single_trigger)
{
    mg58f18_radar_frame_t frame;
    const mg58f18_radar_status_t status =
        mg58f18_radar_send_command_raw(MG58F18_RADAR_CMD_QUERY_TRIGGER_MODE, 0x00U, 0x00U, 0x00U, &frame);
    if (status != MG58F18_RADAR_STATUS_OK)
    {
        return status;
    }
    if (single_trigger != NULL)
    {
        *single_trigger = (frame.data4 == 0x01U);
    }
    return MG58F18_RADAR_STATUS_OK;
}

/**
 * @brief 设置发射功率档位 0~7（值越小功率越大、距离越远）。
 */
mg58f18_radar_status_t mg58f18_radar_set_power_step(uint8_t step)
{
    if (step > 7U)
    {
        return MG58F18_RADAR_STATUS_INVALID_ARGUMENT;
    }
    return mg58f18_radar_expect_echo(MG58F18_RADAR_CMD_SET_TX_POWER_STEP, 0x00U, 0x00U, step);
}
/**
 * @brief 查询发射功率档位。
 */
mg58f18_radar_status_t mg58f18_radar_get_power_step(uint8_t *step)
{
    mg58f18_radar_frame_t frame;
    const mg58f18_radar_status_t status =
        mg58f18_radar_send_command_raw(MG58F18_RADAR_CMD_QUERY_TX_POWER_STEP, 0x00U, 0x00U, 0x00U, &frame);
    if (status != MG58F18_RADAR_STATUS_OK)
    {
        return status;
    }
    if (step != NULL)
    {
        *step = frame.data4;
    }
    return MG58F18_RADAR_STATUS_OK;
}

/**
 * @brief 设置光感阈值（0x00~0xFF，越小越灵敏）。
 */
mg58f18_radar_status_t mg58f18_radar_set_light_threshold(uint8_t threshold)
{
    return mg58f18_radar_expect_echo(MG58F18_RADAR_CMD_SET_LIGHT_SENSOR_THRESHOLD,
                                     0x00U,
                                     0x00U,
                                     threshold);
}
/**
 * @brief 查询光感阈值。
 */
mg58f18_radar_status_t mg58f18_radar_get_light_threshold(uint8_t *threshold)
{
    mg58f18_radar_frame_t frame;
    const mg58f18_radar_status_t status =
        mg58f18_radar_send_command_raw(MG58F18_RADAR_CMD_QUERY_LIGHT_SENSOR_THRESHOLD, 0x00U, 0x00U, 0x00U, &frame);
    if (status != MG58F18_RADAR_STATUS_OK)
    {
        return status;
    }
    if (threshold != NULL)
    {
        *threshold = frame.data4;
    }
    return MG58F18_RADAR_STATUS_OK;
}

/**
 * @brief 设置 PWM 使能状态。
 */
mg58f18_radar_status_t mg58f18_radar_set_pwm_enabled(bool enable)
{
    return mg58f18_radar_expect_echo(MG58F18_RADAR_CMD_SET_PWM_ENABLE,
                                     0x00U,
                                     0x00U,
                                     enable ? 0x01U : 0x00U);
}
/**
 * @brief 查询 PWM 使能状态。
 */
mg58f18_radar_status_t mg58f18_radar_get_pwm_enabled(bool *enable)
{
    mg58f18_radar_frame_t frame;
    const mg58f18_radar_status_t status =
        mg58f18_radar_send_command_raw(MG58F18_RADAR_CMD_QUERY_PWM_ENABLE, 0x00U, 0x00U, 0x00U, &frame);
    if (status != MG58F18_RADAR_STATUS_OK)
    {
        return status;
    }
    if (enable != NULL)
    {
        *enable = (frame.data4 == 0x01U);
    }
    return MG58F18_RADAR_STATUS_OK;
}

/**
 * @brief 设置 PWM 占空比 raw（默认 0~0x0DAC）。
 */
mg58f18_radar_status_t mg58f18_radar_set_pwm_duty_raw(uint16_t duty_raw)
{
    if (duty_raw > 0x0DACU)
    {
        return MG58F18_RADAR_STATUS_INVALID_ARGUMENT;
    }
    const uint8_t data3 = (uint8_t)((duty_raw >> 8) & 0xFFU);
    const uint8_t data4 = (uint8_t)(duty_raw & 0xFFU);
    return mg58f18_radar_expect_echo(MG58F18_RADAR_CMD_SET_PWM_DUTY, 0x00U, data3, data4);
}

/**
 * @brief 查询 PWM 占空比 raw。
 */
mg58f18_radar_status_t mg58f18_radar_get_pwm_duty_raw(uint16_t *duty_raw)
{
    mg58f18_radar_frame_t frame;
    const mg58f18_radar_status_t status =
        mg58f18_radar_send_command_raw(MG58F18_RADAR_CMD_QUERY_PWM_DUTY, 0x00U, 0x00U, 0x00U, &frame);
    if (status != MG58F18_RADAR_STATUS_OK)
    {
        return status;
    }
    if (duty_raw != NULL)
    {
        *duty_raw = (uint16_t)(((uint16_t)frame.data3 << 8) | frame.data4);
    }
    return MG58F18_RADAR_STATUS_OK;
}

/**
 * @brief 设置脉冲宽度（0x00~0xFF，值越大穿透/灵敏度越高）。
 */
mg58f18_radar_status_t mg58f18_radar_set_power_pulse_width(uint8_t width)
{
    return mg58f18_radar_expect_echo(MG58F18_RADAR_CMD_SET_POWER_PULSE_WIDTH,
                                     0x00U,
                                     0x00U,
                                     width);
}
/**
 * @brief 查询脉冲宽度。
 */
mg58f18_radar_status_t mg58f18_radar_get_power_pulse_width(uint8_t *width)
{
    mg58f18_radar_frame_t frame;
    const mg58f18_radar_status_t status =
        mg58f18_radar_send_command_raw(MG58F18_RADAR_CMD_QUERY_POWER_PULSE_WIDTH, 0x00U, 0x00U, 0x00U, &frame);
    if (status != MG58F18_RADAR_STATUS_OK)
    {
        return status;
    }
    if (width != NULL)
    {
        *width = frame.data4;
    }
    return MG58F18_RADAR_STATUS_OK;
}

/**
 * @brief 设置响应模式（true 手势/近距离，false 移动检测）。
 */
mg58f18_radar_status_t mg58f18_radar_set_sensing_mode(bool hand_mode)
{
    return mg58f18_radar_expect_echo(MG58F18_RADAR_CMD_SET_SENSING_MODE,
                                     0x00U,
                                     0x00U,
                                     hand_mode ? 0x01U : 0x00U);
}
/**
 * @brief 查询响应模式。
 */
mg58f18_radar_status_t mg58f18_radar_get_sensing_mode(bool *hand_mode)
{
    mg58f18_radar_frame_t frame;
    const mg58f18_radar_status_t status =
        mg58f18_radar_send_command_raw(MG58F18_RADAR_CMD_QUERY_SENSING_MODE, 0x00U, 0x00U, 0x00U, &frame);
    if (status != MG58F18_RADAR_STATUS_OK)
    {
        return status;
    }
    if (hand_mode != NULL)
    {
        *hand_mode = (frame.data4 == 0x01U);
    }
    return MG58F18_RADAR_STATUS_OK;
}

/**
 * @brief 将当前参数保存到模组 Flash（写后建议等待 ≥100ms）。
 */
mg58f18_radar_status_t mg58f18_radar_save_settings(void)
{
    return mg58f18_radar_expect_echo(MG58F18_RADAR_CMD_SAVE_SETTINGS, 0x00U, 0x00U, 0x01U);
}

/**
 * @brief 通过硬件接口层读取雷达 OUT 引脚电平。
 */
mg58f18_radar_status_t mg58f18_radar_read_io(bool *active)
{
    if (active == NULL)
    {
        return MG58F18_RADAR_STATUS_INVALID_ARGUMENT;
    }
    if (!s_initialised)
    {
        return MG58F18_RADAR_STATUS_NOT_INITIALISED;
    }

    *active = mg58f18_radar_interface_hw_read_io();
    return MG58F18_RADAR_STATUS_OK;
}

/**
 * @brief 取出最近收到但尚未消费的应答帧。
 */
bool mg58f18_radar_fetch_frame(mg58f18_radar_frame_t *frame)
{
    if (!s_initialised || !s_core.frame_available)
    {
        return false;
    }
    if (frame != NULL)
    {
        *frame = s_core.last_frame;
    }
    s_core.frame_available = false;
    return true;
}

/**
 * @brief 解析并打印单帧应答内容，便于串口调试。
 */
void mg58f18_radar_print_frame(const mg58f18_radar_frame_t *frame)
{
    if (frame == NULL)
    {
        return;
    }

    printf("[RADAR] cmd=0x%02X data2=0x%02X data3=0x%02X data4=0x%02X checksum=0x%02X\r\n",
           frame->command,
           frame->data2,
           frame->data3,
           frame->data4,
           frame->checksum);

    switch (frame->command)
    {
        case MG58F18_RADAR_CMD_SET_DISTANCE_THRESHOLD:
        case MG58F18_RADAR_CMD_QUERY_DISTANCE_THRESHOLD:
        {
            const uint16_t threshold = (uint16_t)(((uint16_t)frame->data3 << 8) | frame->data4);
            printf("        distance threshold: %u\r\n", threshold);
            break;
        }
        case MG58F18_RADAR_CMD_SET_DELAY_TIME:
        case MG58F18_RADAR_CMD_QUERY_DELAY_TIME:
        {
            const uint32_t raw = ((uint32_t)frame->data2 << 16)
                               | ((uint32_t)frame->data3 << 8)
                               | frame->data4;
            printf("        delay: %lu ms\r\n", (unsigned long)(raw / MG58F18_RADAR_DELAY_SCALER));
            break;
        }
        case MG58F18_RADAR_CMD_SET_LIGHT_SENSOR_ENABLE:
        case MG58F18_RADAR_CMD_QUERY_LIGHT_SENSOR_ENABLE:
            printf("        light sensor: %s\r\n", (frame->data4 == 0x01U) ? "ON" : "OFF");
            break;
        case MG58F18_RADAR_CMD_SET_BLOCK_TIME:
        case MG58F18_RADAR_CMD_QUERY_BLOCK_TIME:
        {
            const uint32_t raw = ((uint32_t)frame->data2 << 16)
                               | ((uint32_t)frame->data3 << 8)
                               | frame->data4;
            printf("        block time: %lu ms\r\n", (unsigned long)(raw / MG58F18_RADAR_DELAY_SCALER));
            break;
        }
        case MG58F18_RADAR_CMD_SET_ACTIVE_LEVEL:
        case MG58F18_RADAR_CMD_QUERY_ACTIVE_LEVEL:
            printf("        active level: %s\r\n", (frame->data4 == 0x01U) ? "HIGH" : "LOW");
            break;
        case MG58F18_RADAR_CMD_SET_POWER_MODE:
        case MG58F18_RADAR_CMD_QUERY_POWER_MODE:
            printf("        power mode: %s\r\n", (frame->data4 == 0x01U) ? "NORMAL (13mA)" : "ULTRA LOW (50/60uA)");
            break;
        case MG58F18_RADAR_CMD_QUERY_TRIGGER_STATE:
            printf("        trigger state: %s\r\n", (frame->data4 == 0x01U) ? "TRIGGERED" : "IDLE");
            break;
        case MG58F18_RADAR_CMD_QUERY_LIGHT_ENVIRONMENT:
            printf("        light environment: %s\r\n", (frame->data4 == 0x01U) ? "NIGHT" : "DAY");
            break;
        case MG58F18_RADAR_CMD_QUERY_FIRMWARE_VERSION:
        {
            const uint8_t major = (uint8_t)((frame->data4 >> 4) & 0x0FU);
            const uint8_t minor = (uint8_t)(frame->data4 & 0x0FU);
            printf("        firmware version: V%u.%u\r\n", major, minor);
            break;
        }
        case MG58F18_RADAR_CMD_SET_TRIGGER_MODE:
        case MG58F18_RADAR_CMD_QUERY_TRIGGER_MODE:
            printf("        trigger mode: %s\r\n", (frame->data4 == 0x01U) ? "SINGLE" : "CONTINUOUS");
            break;
        case MG58F18_RADAR_CMD_SET_TX_POWER_STEP:
        case MG58F18_RADAR_CMD_QUERY_TX_POWER_STEP:
            printf("        TX power step: %u\r\n", frame->data4);
            break;
        case MG58F18_RADAR_CMD_SET_LIGHT_SENSOR_THRESHOLD:
        case MG58F18_RADAR_CMD_QUERY_LIGHT_SENSOR_THRESHOLD:
            printf("        light threshold: 0x%02X\r\n", frame->data4);
            break;
        case MG58F18_RADAR_CMD_SET_PWM_ENABLE:
        case MG58F18_RADAR_CMD_QUERY_PWM_ENABLE:
            printf("        PWM state: %s\r\n", (frame->data4 == 0x01U) ? "ON" : "OFF");
            break;
        case MG58F18_RADAR_CMD_SET_PWM_DUTY:
        case MG58F18_RADAR_CMD_QUERY_PWM_DUTY:
        {
            const uint16_t duty = (uint16_t)(((uint16_t)frame->data3 << 8) | frame->data4);
            printf("        PWM duty raw: %u\r\n", duty);
            break;
        }
        case MG58F18_RADAR_CMD_SET_POWER_PULSE_WIDTH:
        case MG58F18_RADAR_CMD_QUERY_POWER_PULSE_WIDTH:
            printf("        pulse width: 0x%02X\r\n", frame->data4);
            break;
        case MG58F18_RADAR_CMD_SET_SENSING_MODE:
        case MG58F18_RADAR_CMD_QUERY_SENSING_MODE:
            printf("        sensing mode: %s\r\n", (frame->data4 == 0x01U) ? "HAND DETECTION" : "MOTION DETECTION");
            break;
        case MG58F18_RADAR_CMD_SAVE_SETTINGS:
            printf("        save settings ack\r\n");
            break;
        default:
            printf("        unhandled command\r\n");
            break;
    }
}

/**
 * @brief 轮询缓存中尚未消费的帧并逐条打印。
 */
void mg58f18_radar_poll_and_print(void)
{
    mg58f18_radar_frame_t frame;
    while (mg58f18_radar_fetch_frame(&frame))
    {
        mg58f18_radar_print_frame(&frame);
    }
}

/**
 * @brief 将状态码转为字符串（日志/调试用）。
 */
const char *mg58f18_radar_status_string(mg58f18_radar_status_t status)
{
    switch (status)
    {
        case MG58F18_RADAR_STATUS_OK:               return "ok";
        case MG58F18_RADAR_STATUS_NOT_INITIALISED:  return "not_initialised";
        case MG58F18_RADAR_STATUS_BUSY:             return "busy";
        case MG58F18_RADAR_STATUS_INVALID_ARGUMENT: return "invalid_argument";
        case MG58F18_RADAR_STATUS_TIMEOUT:          return "timeout";
        case MG58F18_RADAR_STATUS_FRAME_ERROR:      return "frame_error";
        case MG58F18_RADAR_STATUS_HAL_ERROR:        return "hal_error";
        default:                                    return "unknown";
    }
}

/**
 * @brief 串口 DMA/中断回调中调用，向协议层灌入收到的字节流。
 */
void mg58f18_radar_receive_bytes(const uint8_t *data, size_t length)
{
    if (!s_initialised || data == NULL || length == 0U)
    {
        return;
    }

    (void)mg58f18_radar_core_process_bytes(&s_core, data, length);
}
