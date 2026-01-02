#include "soft_i2c.h"

#ifndef __NOP
#define __NOP() __asm volatile ("nop")
#endif

static soft_i2c_status_t soft_i2c_prepare_bus(soft_i2c_bus_t *bus);
static void soft_i2c_delay(const soft_i2c_bus_t *bus);
static soft_i2c_status_t soft_i2c_drive_scl(soft_i2c_bus_t *bus, soft_i2c_pin_state_t state);
static soft_i2c_status_t soft_i2c_drive_sda(soft_i2c_bus_t *bus, soft_i2c_pin_state_t state);
static soft_i2c_status_t soft_i2c_start(soft_i2c_bus_t *bus);
static soft_i2c_status_t soft_i2c_repeated_start(soft_i2c_bus_t *bus);
static void soft_i2c_stop(soft_i2c_bus_t *bus);
static soft_i2c_status_t soft_i2c_write_byte(soft_i2c_bus_t *bus, uint8_t value);
static soft_i2c_status_t soft_i2c_read_byte(soft_i2c_bus_t *bus, uint8_t *value, int last_byte);

/* 初始化软I2C总线：确认GPIO操作回调存在并设置默认延时 */
soft_i2c_status_t soft_i2c_bus_init(soft_i2c_bus_t *bus)
{
    if ((bus == NULL) ||
        (bus->scl.write == NULL) || (bus->scl.read == NULL) ||
        (bus->sda.write == NULL) || (bus->sda.read == NULL)) {
        return SOFT_I2C_STATUS_INVALID_PARAM;
    }

    if (bus->delay_ticks == 0U) {
        bus->delay_ticks = SOFT_I2C_DEFAULT_DELAY_TICKS;
    }

    if (bus->stretch_timeout_ticks == 0U) {
        bus->stretch_timeout_ticks = SOFT_I2C_DEFAULT_STRETCH_TICKS;
    }

    soft_i2c_drive_scl(bus, SOFT_I2C_PIN_SET);
    soft_i2c_drive_sda(bus, SOFT_I2C_PIN_SET);

    bus->initialized = 1U;
    return SOFT_I2C_STATUS_OK;
}

/* 执行起始+地址+数据的写入流程，遇到NACK立即停止 */
soft_i2c_status_t soft_i2c_master_transmit(soft_i2c_bus_t *bus, uint8_t address,
                                           const uint8_t *data, size_t size)
{
    if ((data == NULL) && (size > 0U)) {
        return SOFT_I2C_STATUS_INVALID_PARAM;
    }

    soft_i2c_status_t status = soft_i2c_prepare_bus(bus);
    if (status != SOFT_I2C_STATUS_OK) {
        return status;
    }

    int started = 0;
    status = soft_i2c_start(bus);
    if (status != SOFT_I2C_STATUS_OK) {
        goto done;
    }
    started = 1;

    status = soft_i2c_write_byte(bus, address & (uint8_t)~0x01U);
    if (status != SOFT_I2C_STATUS_OK) {
        goto done;
    }

    for (size_t i = 0; i < size; ++i) {
        status = soft_i2c_write_byte(bus, data[i]);
        if (status != SOFT_I2C_STATUS_OK) {
            goto done;
        }
    }

done:
    if (started) {
        soft_i2c_stop(bus);
    }
    return status;
}

/* 仅执行读流程，负责发送最后一个NACK并停止 */
soft_i2c_status_t soft_i2c_master_receive(soft_i2c_bus_t *bus, uint8_t address,
                                          uint8_t *data, size_t size)
{
    if ((data == NULL) && (size > 0U)) {
        return SOFT_I2C_STATUS_INVALID_PARAM;
    }
    if (size == 0U) {
        return SOFT_I2C_STATUS_OK;
    }

    soft_i2c_status_t status = soft_i2c_prepare_bus(bus);
    if (status != SOFT_I2C_STATUS_OK) {
        return status;
    }

    int started = 0;
    status = soft_i2c_start(bus);
    if (status != SOFT_I2C_STATUS_OK) {
        goto done;
    }
    started = 1;

    status = soft_i2c_write_byte(bus, address | 0x01U);
    if (status != SOFT_I2C_STATUS_OK) {
        goto done;
    }

    for (size_t i = 0; i < size; ++i) {
        const int last_byte = (i + 1U) == size;
        status = soft_i2c_read_byte(bus, &data[i], last_byte);
        if (status != SOFT_I2C_STATUS_OK) {
            goto done;
        }
    }

done:
    if (started) {
        soft_i2c_stop(bus);
    }
    return status;
}

/* 先写后读的组合事务，实现典型寄存器访问 */
soft_i2c_status_t soft_i2c_master_write_read(soft_i2c_bus_t *bus, uint8_t address,
                                             const uint8_t *tx_data, size_t tx_size,
                                             uint8_t *rx_data, size_t rx_size)
{
    if (((tx_data == NULL) && (tx_size > 0U)) ||
        ((rx_data == NULL) && (rx_size > 0U))) {
        return SOFT_I2C_STATUS_INVALID_PARAM;
    }

    soft_i2c_status_t status = soft_i2c_prepare_bus(bus);
    if (status != SOFT_I2C_STATUS_OK) {
        return status;
    }

    int started = 0;
    if (tx_size > 0U) {
        status = soft_i2c_start(bus);
        if (status != SOFT_I2C_STATUS_OK) {
            goto cleanup;
        }
        started = 1;

        status = soft_i2c_write_byte(bus, address & (uint8_t)~0x01U);
        if (status != SOFT_I2C_STATUS_OK) {
            goto cleanup;
        }

        for (size_t i = 0; i < tx_size; ++i) {
            status = soft_i2c_write_byte(bus, tx_data[i]);
            if (status != SOFT_I2C_STATUS_OK) {
                goto cleanup;
            }
        }
    }

    if (rx_size > 0U) {
        if (!started) {
            status = soft_i2c_start(bus);
            if (status != SOFT_I2C_STATUS_OK) {
                goto cleanup;
            }
            started = 1;
        } else {
            status = soft_i2c_repeated_start(bus);
            if (status != SOFT_I2C_STATUS_OK) {
                goto cleanup;
            }
        }

        status = soft_i2c_write_byte(bus, address | 0x01U);
        if (status != SOFT_I2C_STATUS_OK) {
            goto cleanup;
        }

        for (size_t i = 0; i < rx_size; ++i) {
            const int last_byte = (i + 1U) == rx_size;
            status = soft_i2c_read_byte(bus, &rx_data[i], last_byte);
            if (status != SOFT_I2C_STATUS_OK) {
                goto cleanup;
            }
        }
    }

cleanup:
    if (started) {
        soft_i2c_stop(bus);
    }
    return status;
}

/* 确保总线已经初始化，必要时报错 */
static soft_i2c_status_t soft_i2c_prepare_bus(soft_i2c_bus_t *bus)
{
    if (bus == NULL) {
        return SOFT_I2C_STATUS_INVALID_PARAM;
    }

    if (bus->initialized == 0U) {
        return soft_i2c_bus_init(bus);
    }

    return SOFT_I2C_STATUS_OK;
}

/* 根据配置执行半周期延时，可自定义回调或默认NOP循环 */
static void soft_i2c_delay(const soft_i2c_bus_t *bus)
{
    if ((bus != NULL) && (bus->delay_fn != NULL)) {
        bus->delay_fn(bus->delay_ticks, bus->delay_ctx);
        return;
    }

    uint32_t ticks = (bus == NULL || bus->delay_ticks == 0U) ? SOFT_I2C_DEFAULT_DELAY_TICKS : bus->delay_ticks;
    for (volatile uint32_t i = 0; i < ticks; ++i) {
        __NOP();
    }
}

/* 设置SCL电平并处理clock stretching */
static soft_i2c_status_t soft_i2c_drive_scl(soft_i2c_bus_t *bus, soft_i2c_pin_state_t state)
{
    soft_i2c_status_t status = bus->scl.write(bus->scl.ctx, state);
    if (status != SOFT_I2C_STATUS_OK) {
        return status;
    }

    if (state == SOFT_I2C_PIN_SET) {
        uint32_t timeout = (bus->stretch_timeout_ticks == 0U)
                               ? SOFT_I2C_DEFAULT_STRETCH_TICKS
                               : bus->stretch_timeout_ticks;
        while (bus->scl.read(bus->scl.ctx) == SOFT_I2C_PIN_RESET) {
            if (timeout-- == 0U) {
                return SOFT_I2C_STATUS_TIMEOUT;
            }
        }
    }

    return SOFT_I2C_STATUS_OK;
}

/* 设置SDA电平 */
static soft_i2c_status_t soft_i2c_drive_sda(soft_i2c_bus_t *bus, soft_i2c_pin_state_t state)
{
    return bus->sda.write(bus->sda.ctx, state);
}

/* 产生I2C起始条件 */
static soft_i2c_status_t soft_i2c_start(soft_i2c_bus_t *bus)
{
    soft_i2c_status_t status = soft_i2c_drive_sda(bus, SOFT_I2C_PIN_SET);
    if (status != SOFT_I2C_STATUS_OK) {
        return status;
    }

    status = soft_i2c_drive_scl(bus, SOFT_I2C_PIN_SET);
    if (status != SOFT_I2C_STATUS_OK) {
        return status;
    }
    soft_i2c_delay(bus);

    status = soft_i2c_drive_sda(bus, SOFT_I2C_PIN_RESET);
    if (status != SOFT_I2C_STATUS_OK) {
        return status;
    }
    soft_i2c_delay(bus);

    status = soft_i2c_drive_scl(bus, SOFT_I2C_PIN_RESET);
    if (status != SOFT_I2C_STATUS_OK) {
        return status;
    }
    soft_i2c_delay(bus);

    return SOFT_I2C_STATUS_OK;
}

/* 产生重复起始条件，用于读写切换 */
static soft_i2c_status_t soft_i2c_repeated_start(soft_i2c_bus_t *bus)
{
    soft_i2c_status_t status = soft_i2c_drive_sda(bus, SOFT_I2C_PIN_SET);
    if (status != SOFT_I2C_STATUS_OK) {
        return status;
    }
    soft_i2c_delay(bus);

    status = soft_i2c_drive_scl(bus, SOFT_I2C_PIN_SET);
    if (status != SOFT_I2C_STATUS_OK) {
        return status;
    }
    soft_i2c_delay(bus);

    status = soft_i2c_drive_sda(bus, SOFT_I2C_PIN_RESET);
    if (status != SOFT_I2C_STATUS_OK) {
        return status;
    }
    soft_i2c_delay(bus);

    status = soft_i2c_drive_scl(bus, SOFT_I2C_PIN_RESET);
    if (status != SOFT_I2C_STATUS_OK) {
        return status;
    }
    soft_i2c_delay(bus);

    return SOFT_I2C_STATUS_OK;
}

/* 产生停止条件，将总线释放到空闲 */
static void soft_i2c_stop(soft_i2c_bus_t *bus)
{
    soft_i2c_drive_sda(bus, SOFT_I2C_PIN_RESET);
    soft_i2c_delay(bus);
    soft_i2c_drive_scl(bus, SOFT_I2C_PIN_SET);
    soft_i2c_delay(bus);
    soft_i2c_drive_sda(bus, SOFT_I2C_PIN_SET);
    soft_i2c_delay(bus);
}

/* 写入单字节并读取对方ACK */
static soft_i2c_status_t soft_i2c_write_byte(soft_i2c_bus_t *bus, uint8_t value)
{
    for (int8_t bit = 7; bit >= 0; --bit) {
        soft_i2c_pin_state_t state = ((value >> bit) & 0x01U) ? SOFT_I2C_PIN_SET : SOFT_I2C_PIN_RESET;
        soft_i2c_status_t status = soft_i2c_drive_sda(bus, state);
        if (status != SOFT_I2C_STATUS_OK) {
            return status;
        }
        soft_i2c_delay(bus);

        status = soft_i2c_drive_scl(bus, SOFT_I2C_PIN_SET);
        if (status != SOFT_I2C_STATUS_OK) {
            return status;
        }
        soft_i2c_delay(bus);

        status = soft_i2c_drive_scl(bus, SOFT_I2C_PIN_RESET);
        if (status != SOFT_I2C_STATUS_OK) {
            return status;
        }
        soft_i2c_delay(bus);
    }

    soft_i2c_status_t status = soft_i2c_drive_sda(bus, SOFT_I2C_PIN_SET);
    if (status != SOFT_I2C_STATUS_OK) {
        return status;
    }
    soft_i2c_delay(bus);

    status = soft_i2c_drive_scl(bus, SOFT_I2C_PIN_SET);
    if (status != SOFT_I2C_STATUS_OK) {
        return status;
    }
    soft_i2c_delay(bus);

    soft_i2c_pin_state_t ack = bus->sda.read(bus->sda.ctx);

    status = soft_i2c_drive_scl(bus, SOFT_I2C_PIN_RESET);
    if (status != SOFT_I2C_STATUS_OK) {
        return status;
    }
    soft_i2c_delay(bus);

    return (ack == SOFT_I2C_PIN_RESET) ? SOFT_I2C_STATUS_OK : SOFT_I2C_STATUS_ERROR;
}

/* 读取单字节，并根据last_byte决定发送ACK或NACK */
static soft_i2c_status_t soft_i2c_read_byte(soft_i2c_bus_t *bus, uint8_t *value, int last_byte)
{
    uint8_t result = 0U;
    soft_i2c_status_t status = soft_i2c_drive_sda(bus, SOFT_I2C_PIN_SET);
    if (status != SOFT_I2C_STATUS_OK) {
        return status;
    }

    for (int8_t bit = 7; bit >= 0; --bit) {
        status = soft_i2c_drive_scl(bus, SOFT_I2C_PIN_SET);
        if (status != SOFT_I2C_STATUS_OK) {
            return status;
        }
        soft_i2c_delay(bus);

        if (bus->sda.read(bus->sda.ctx) == SOFT_I2C_PIN_SET) {
            result |= (uint8_t)(1U << bit);
        }

        status = soft_i2c_drive_scl(bus, SOFT_I2C_PIN_RESET);
        if (status != SOFT_I2C_STATUS_OK) {
            return status;
        }
        soft_i2c_delay(bus);
    }

    status = soft_i2c_drive_sda(bus, last_byte ? SOFT_I2C_PIN_SET : SOFT_I2C_PIN_RESET);
    if (status != SOFT_I2C_STATUS_OK) {
        return status;
    }
    soft_i2c_delay(bus);

    status = soft_i2c_drive_scl(bus, SOFT_I2C_PIN_SET);
    if (status != SOFT_I2C_STATUS_OK) {
        return status;
    }
    soft_i2c_delay(bus);

    status = soft_i2c_drive_scl(bus, SOFT_I2C_PIN_RESET);
    if (status != SOFT_I2C_STATUS_OK) {
        return status;
    }
    soft_i2c_delay(bus);

    soft_i2c_drive_sda(bus, SOFT_I2C_PIN_SET);

    if (value != NULL) {
        *value = result;
    }

    return SOFT_I2C_STATUS_OK;
}
