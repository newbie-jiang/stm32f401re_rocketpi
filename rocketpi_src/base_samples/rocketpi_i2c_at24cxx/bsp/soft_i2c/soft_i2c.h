#pragma once

#include <stddef.h>
#include <stdint.h>

/**
 * @file soft_i2c.h
 * @brief 通用软件I2C主机接口定义，与具体平台解耦。
 */

#define SOFT_I2C_DEFAULT_DELAY_TICKS      64U
#define SOFT_I2C_DEFAULT_STRETCH_TICKS  4000U

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SOFT_I2C_STATUS_OK = 0,
    SOFT_I2C_STATUS_ERROR = 1,
    SOFT_I2C_STATUS_TIMEOUT = 2,
    SOFT_I2C_STATUS_INVALID_PARAM = 3
} soft_i2c_status_t;

typedef enum {
    SOFT_I2C_PIN_RESET = 0,
    SOFT_I2C_PIN_SET = 1
} soft_i2c_pin_state_t;

typedef soft_i2c_status_t (*soft_i2c_pin_write_fn)(void *ctx, soft_i2c_pin_state_t state);
typedef soft_i2c_pin_state_t (*soft_i2c_pin_read_fn)(void *ctx);
typedef void (*soft_i2c_delay_fn)(uint32_t ticks, void *ctx);

typedef struct {
    soft_i2c_pin_write_fn write;
    soft_i2c_pin_read_fn read;
    void *ctx;
} soft_i2c_pin_io_t;

typedef struct {
    soft_i2c_pin_io_t scl;
    soft_i2c_pin_io_t sda;
    soft_i2c_delay_fn delay_fn;
    void *delay_ctx;
    uint32_t delay_ticks;
    uint32_t stretch_timeout_ticks;
    uint8_t initialized;
} soft_i2c_bus_t;

/** 初始化总线结构体并拉高SCL/SDA，准备进入空闲状态。 */
soft_i2c_status_t soft_i2c_bus_init(soft_i2c_bus_t *bus);

/** 发送一帧起始+写数据，若任何字节NACK则返回错误。 */
soft_i2c_status_t soft_i2c_master_transmit(soft_i2c_bus_t *bus, uint8_t address,
                                           const uint8_t *data, size_t size);

/** 发送起始后进入读模式，并根据长度自动发送ACK/NACK。 */
soft_i2c_status_t soft_i2c_master_receive(soft_i2c_bus_t *bus, uint8_t address,
                                          uint8_t *data, size_t size);

/** 常见的先写后读组合操作，可用于寄存器访问。 */
soft_i2c_status_t soft_i2c_master_write_read(soft_i2c_bus_t *bus, uint8_t address,
                                             const uint8_t *tx_data, size_t tx_size,
                                             uint8_t *rx_data, size_t rx_size);

#ifdef __cplusplus
}
#endif
