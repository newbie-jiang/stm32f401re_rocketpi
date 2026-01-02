#include "driver_aht30.h"

#include "driver_aht30_config.h"

#include <stddef.h>

#if AHT30_USE_SOFT_I2C
#include "soft_i2c.h"
#include "gpio.h"
#else
#include "i2c.h"
#endif

/**
 * @file driver_aht30.c
 * @brief AHT30温湿度传感器的软件/硬件I2C驱动实现。
 */

#define AHT30_CMD_TRIGGER       0xACU
#define AHT30_CMD_CONFIG_0      0x33U
#define AHT30_CMD_CONFIG_1      0x00U
#define AHT30_CMD_RESET         0xBAU
#define AHT30_STATUS_BUSY       0x80U

#if AHT30_USE_SOFT_I2C

typedef struct {
    GPIO_TypeDef *port;
    uint16_t pin;
} aht30_gpio_pin_t;

static soft_i2c_status_t aht30_gpio_write(void *ctx, soft_i2c_pin_state_t state)
{
    aht30_gpio_pin_t *pin = (aht30_gpio_pin_t *)ctx;
    HAL_GPIO_WritePin(pin->port, pin->pin, (state == SOFT_I2C_PIN_SET) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    return SOFT_I2C_STATUS_OK;
}

static soft_i2c_pin_state_t aht30_gpio_read(void *ctx)
{
    aht30_gpio_pin_t *pin = (aht30_gpio_pin_t *)ctx;
    return (HAL_GPIO_ReadPin(pin->port, pin->pin) == GPIO_PIN_SET) ? SOFT_I2C_PIN_SET : SOFT_I2C_PIN_RESET;
}

static aht30_gpio_pin_t s_scl_pin = {AHT30_SOFT_SCL_PORT, AHT30_SOFT_SCL_PIN};
static aht30_gpio_pin_t s_sda_pin = {AHT30_SOFT_SDA_PORT, AHT30_SOFT_SDA_PIN};

static soft_i2c_bus_t s_aht30_bus = {
    .scl = {aht30_gpio_write, aht30_gpio_read, &s_scl_pin},
    .sda = {aht30_gpio_write, aht30_gpio_read, &s_sda_pin},
    .delay_fn = NULL,
    .delay_ctx = NULL,
    .delay_ticks = AHT30_SOFT_DELAY_TICKS,
    .stretch_timeout_ticks = AHT30_SOFT_STRETCH_TICKS,
    .initialized = 0U,
};

static void aht30_soft_gpio_configure(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    GPIO_InitStruct.Pin = AHT30_SOFT_SCL_PIN;
    HAL_GPIO_Init(AHT30_SOFT_SCL_PORT, &GPIO_InitStruct);
    HAL_GPIO_WritePin(AHT30_SOFT_SCL_PORT, AHT30_SOFT_SCL_PIN, GPIO_PIN_SET);

    GPIO_InitStruct.Pin = AHT30_SOFT_SDA_PIN;
    HAL_GPIO_Init(AHT30_SOFT_SDA_PORT, &GPIO_InitStruct);
    HAL_GPIO_WritePin(AHT30_SOFT_SDA_PORT, AHT30_SOFT_SDA_PIN, GPIO_PIN_SET);
}

static HAL_StatusTypeDef aht30_bus_init(void)
{
    aht30_soft_gpio_configure();
    soft_i2c_status_t status = soft_i2c_bus_init(&s_aht30_bus);
    return (status == SOFT_I2C_STATUS_OK) ? HAL_OK : HAL_ERROR;
}

static HAL_StatusTypeDef aht30_bus_transmit(const uint8_t *data, size_t size)
{
    soft_i2c_status_t status = soft_i2c_master_transmit(&s_aht30_bus, AHT30_I2C_ADDRESS, data, size);
    if (status == SOFT_I2C_STATUS_TIMEOUT) {
        return HAL_TIMEOUT;
    }
    return (status == SOFT_I2C_STATUS_OK) ? HAL_OK : HAL_ERROR;
}

static HAL_StatusTypeDef aht30_bus_receive(uint8_t *data, size_t size)
{
    soft_i2c_status_t status = soft_i2c_master_receive(&s_aht30_bus, AHT30_I2C_ADDRESS, data, size);
    if (status == SOFT_I2C_STATUS_TIMEOUT) {
        return HAL_TIMEOUT;
    }
    return (status == SOFT_I2C_STATUS_OK) ? HAL_OK : HAL_ERROR;
}

#else

static HAL_StatusTypeDef aht30_bus_init(void)
{
    return HAL_OK;
}

static HAL_StatusTypeDef aht30_bus_transmit(const uint8_t *data, size_t size)
{
    return HAL_I2C_Master_Transmit(&AHT30_I2C_HANDLE, AHT30_I2C_ADDRESS, (uint8_t *)data, size, HAL_MAX_DELAY);
}

static HAL_StatusTypeDef aht30_bus_receive(uint8_t *data, size_t size)
{
    return HAL_I2C_Master_Receive(&AHT30_I2C_HANDLE, AHT30_I2C_ADDRESS, data, size, HAL_MAX_DELAY);
}

#endif /* AHT30_USE_SOFT_I2C */

static HAL_StatusTypeDef aht30_soft_reset(void)
{
    uint8_t cmd = AHT30_CMD_RESET;
    return aht30_bus_transmit(&cmd, 1U);
}

static void aht30_convert_samples(const uint8_t raw[6], float *temperature_c, float *humidity_pct)
{
    uint32_t raw_humidity = ((uint32_t)raw[1] << 12)
                          | ((uint32_t)raw[2] << 4)
                          | (uint32_t)(raw[3] >> 4);

    uint32_t raw_temperature = (((uint32_t)raw[3] & 0x0FU) << 16)
                             | ((uint32_t)raw[4] << 8)
                             | (uint32_t)raw[5];

    *humidity_pct = (raw_humidity * 100.0f) / 1048576.0f;
    *temperature_c = (raw_temperature * 200.0f) / 1048576.0f - 50.0f;
}

HAL_StatusTypeDef aht30_init(void)
{
    HAL_StatusTypeDef status = aht30_bus_init();
    if (status != HAL_OK) {
        return status;
    }

    HAL_Delay(AHT30_POWER_ON_DELAY);
    status = aht30_soft_reset();
    HAL_Delay(AHT30_POST_RESET_DELAY);
    return status;
}

HAL_StatusTypeDef aht30_read_raw(uint8_t raw[6])
{
    uint8_t cmd[3] = {AHT30_CMD_TRIGGER, AHT30_CMD_CONFIG_0, AHT30_CMD_CONFIG_1};
    HAL_StatusTypeDef status = aht30_bus_transmit(cmd, sizeof(cmd));
    if (status != HAL_OK) {
        return status;
    }

    HAL_Delay(AHT30_MEASUREMENT_DELAY);
    status = aht30_bus_receive(raw, 6U);
    if (status != HAL_OK) {
        return status;
    }

    if ((raw[0] & AHT30_STATUS_BUSY) != 0U) {
        return HAL_BUSY;
    }

    return HAL_OK;
}

HAL_StatusTypeDef aht30_read(float *temperature_c, float *humidity_pct)
{
    if ((temperature_c == NULL) || (humidity_pct == NULL)) {
        return HAL_ERROR;
    }

    uint8_t raw[6] = {0};
    HAL_StatusTypeDef status = aht30_read_raw(raw);
    if (status != HAL_OK) {
        return status;
    }

    aht30_convert_samples(raw, temperature_c, humidity_pct);
    return HAL_OK;
}
