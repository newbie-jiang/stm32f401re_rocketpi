/**
 * Copyright (c) 2015 - present LibDriver All rights reserved
 *
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * @file      driver_at24cxx_interface.c
 * @brief     driver at24cxx interface source file (Zephyr software I2C)
 * @version   2.0.0
 * @author    Shifeng Li
 * @date      2021-02-17
 *
 * <h3>history</h3>
 * <table>
 * <tr><th>Date        <th>Version  <th>Author      <th>Description
 * <tr><td>2021/02/17  <td>2.0      <td>Shifeng Li  <td>format the code
 * <tr><td>2020/10/15  <td>1.0      <td>Shifeng Li  <td>first upload
 * </table>
 */

#include "driver_at24cxx_interface.h"

#include "soft_i2c.h"

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>

#include <stdarg.h>
#include <string.h>

LOG_MODULE_REGISTER(at24cxx_if, CONFIG_LOG_DEFAULT_LEVEL);

#define AT24CXX_INTERFACE_TX_STACK_BUFFER_SIZE 66U
#define AT24CXX_SOFT_I2C_DELAY_US              20U

#define AT24CXX_NODE DT_NODELABEL(at24_soft_i2c)
#if !DT_NODE_HAS_STATUS(AT24CXX_NODE, okay)
#error "at24_soft_i2c node is not enabled in the devicetree"
#endif
static struct gpio_dt_spec at24cxx_scl = GPIO_DT_SPEC_GET_OR(AT24CXX_NODE, scl_gpios, {0});
static struct gpio_dt_spec at24cxx_sda = GPIO_DT_SPEC_GET_OR(AT24CXX_NODE, sda_gpios, {0});

typedef struct
{
    const struct gpio_dt_spec *spec;
} at24cxx_gpio_ctx_t;

static at24cxx_gpio_ctx_t s_scl_ctx = { .spec = &at24cxx_scl };
static at24cxx_gpio_ctx_t s_sda_ctx = { .spec = &at24cxx_sda };
static soft_i2c_bus_t s_at24cxx_soft_i2c_bus;

static soft_i2c_status_t at24cxx_gpio_write(void *ctx, soft_i2c_pin_state_t state);
static soft_i2c_pin_state_t at24cxx_gpio_read(void *ctx);
static void at24cxx_soft_delay(uint32_t ticks, void *ctx);
static uint8_t at24cxx_interface_write_with_prefix(uint8_t addr,
                                                   const uint8_t *prefix,
                                                   size_t prefix_len,
                                                   const uint8_t *buf,
                                                   uint16_t len);

static soft_i2c_status_t at24cxx_gpio_write(void *ctx, soft_i2c_pin_state_t state)
{
    at24cxx_gpio_ctx_t *gpio = (at24cxx_gpio_ctx_t *)ctx;
    int ret = gpio_pin_set_raw(gpio->spec->port, gpio->spec->pin, (state == SOFT_I2C_PIN_SET) ? 1 : 0);

    return (ret == 0) ? SOFT_I2C_STATUS_OK : SOFT_I2C_STATUS_ERROR;
}

static soft_i2c_pin_state_t at24cxx_gpio_read(void *ctx)
{
    at24cxx_gpio_ctx_t *gpio = (at24cxx_gpio_ctx_t *)ctx;
    int level = gpio_pin_get_raw(gpio->spec->port, gpio->spec->pin);

    return (level > 0) ? SOFT_I2C_PIN_SET : SOFT_I2C_PIN_RESET;
}

static void at24cxx_soft_delay(uint32_t ticks, void *ctx)
{
    ARG_UNUSED(ctx);
    k_busy_wait(ticks);
}

static uint8_t at24cxx_interface_write_with_prefix(uint8_t addr,
                                                   const uint8_t *prefix,
                                                   size_t prefix_len,
                                                   const uint8_t *buf,
                                                   uint16_t len)
{
    size_t tx_len = prefix_len + (size_t)len;
    if (tx_len == 0U)
    {
        return 0;
    }

    uint8_t stack_buf[AT24CXX_INTERFACE_TX_STACK_BUFFER_SIZE];
    uint8_t *tx_buf = stack_buf;

    if (tx_len > sizeof(stack_buf))
    {
        tx_buf = (uint8_t *)k_malloc(tx_len);
        if (tx_buf == NULL)
        {
            return 1;
        }
    }

    if ((prefix_len > 0U) && (prefix != NULL))
    {
        memcpy(tx_buf, prefix, prefix_len);
    }
    if ((len > 0U) && (buf != NULL))
    {
        memcpy(tx_buf + prefix_len, buf, len);
    }

    soft_i2c_status_t status = soft_i2c_master_transmit(&s_at24cxx_soft_i2c_bus, addr, tx_buf, tx_len);

    if (tx_buf != stack_buf)
    {
        k_free(tx_buf);
    }

    return (status == SOFT_I2C_STATUS_OK) ? 0U : 1U;
}

/**
 * @brief  interface iic bus init
 * @return status code
 *         - 0 success
 *         - 1 iic init failed
 * @note   none
 */
uint8_t at24cxx_interface_iic_init(void)
{
    if ((at24cxx_scl.port == NULL) || (at24cxx_sda.port == NULL))
    {
#if DT_NODE_HAS_STATUS(DT_NODELABEL(gpioc), okay)
        at24cxx_scl.port = DEVICE_DT_GET(DT_NODELABEL(gpioc));
        at24cxx_scl.pin = 14;
        at24cxx_scl.dt_flags = GPIO_ACTIVE_HIGH;
        at24cxx_sda.port = DEVICE_DT_GET(DT_NODELABEL(gpioc));
        at24cxx_sda.pin = 15;
        at24cxx_sda.dt_flags = GPIO_ACTIVE_HIGH;
#else
        LOG_ERR("at24cxx: scl/sda dt node is missing");
        return 1;
#endif
    }

    if (!device_is_ready(at24cxx_scl.port) || !device_is_ready(at24cxx_sda.port))
    {
        LOG_ERR("at24cxx: gpio device not ready");
        return 1;
    }

    gpio_flags_t flags = GPIO_OUTPUT | GPIO_OPEN_DRAIN | GPIO_PULL_UP;
    int ret = gpio_pin_configure_dt(&at24cxx_scl, flags | at24cxx_scl.dt_flags);
    if (ret != 0)
    {
        LOG_ERR("at24cxx: scl config failed (%d)", ret);
        return 1;
    }
    ret = gpio_pin_configure_dt(&at24cxx_sda, flags | at24cxx_sda.dt_flags);
    if (ret != 0)
    {
        LOG_ERR("at24cxx: sda config failed (%d)", ret);
        return 1;
    }
    (void)gpio_pin_set_dt(&at24cxx_scl, 1);
    (void)gpio_pin_set_dt(&at24cxx_sda, 1);

    s_at24cxx_soft_i2c_bus.scl.write = at24cxx_gpio_write;
    s_at24cxx_soft_i2c_bus.scl.read = at24cxx_gpio_read;
    s_at24cxx_soft_i2c_bus.scl.ctx = &s_scl_ctx;
    s_at24cxx_soft_i2c_bus.sda.write = at24cxx_gpio_write;
    s_at24cxx_soft_i2c_bus.sda.read = at24cxx_gpio_read;
    s_at24cxx_soft_i2c_bus.sda.ctx = &s_sda_ctx;
    s_at24cxx_soft_i2c_bus.delay_fn = at24cxx_soft_delay;
    s_at24cxx_soft_i2c_bus.delay_ctx = NULL;
    s_at24cxx_soft_i2c_bus.delay_ticks = AT24CXX_SOFT_I2C_DELAY_US;
    s_at24cxx_soft_i2c_bus.stretch_timeout_ticks = 0U;
    s_at24cxx_soft_i2c_bus.initialized = 0U;

    return (soft_i2c_bus_init(&s_at24cxx_soft_i2c_bus) == SOFT_I2C_STATUS_OK) ? 0U : 1U;
}

/**
 * @brief  interface iic bus deinit
 * @return status code
 *         - 0 success
 *         - 1 iic deinit failed
 * @note   none
 */
uint8_t at24cxx_interface_iic_deinit(void)
{
    s_at24cxx_soft_i2c_bus.initialized = 0U;

    (void)gpio_pin_configure_dt(&at24cxx_scl, GPIO_INPUT | GPIO_PULL_UP);
    (void)gpio_pin_configure_dt(&at24cxx_sda, GPIO_INPUT | GPIO_PULL_UP);

    return 0;
}

/**
 * @brief      interface iic bus read
 * @param[in]  addr iic device write address
 * @param[in]  reg iic register address
 * @param[out] *buf pointer to a data buffer
 * @param[in]  len length of the data buffer
 * @return     status code
 *             - 0 success
 *             - 1 read failed
 * @note       none
 */
uint8_t at24cxx_interface_iic_read(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len)
{
    if (len == 0U)
    {
        return 0;
    }
    if (buf == NULL)
    {
        return 1;
    }

    uint8_t reg_buf[1];
    reg_buf[0] = reg;

    soft_i2c_status_t status = soft_i2c_master_write_read(&s_at24cxx_soft_i2c_bus,
                                                          addr,
                                                          reg_buf,
                                                          sizeof(reg_buf),
                                                          buf,
                                                          len);

    return (status == SOFT_I2C_STATUS_OK) ? 0U : 1U;
}

/**
 * @brief     interface iic bus write
 * @param[in] addr iic device write address
 * @param[in] reg iic register address
 * @param[in] *buf pointer to a data buffer
 * @param[in] len length of the data buffer
 * @return    status code
 *            - 0 success
 *            - 1 write failed
 * @note      none
 */
uint8_t at24cxx_interface_iic_write(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len)
{
    if (len == 0U)
    {
        return 0;
    }
    if (buf == NULL)
    {
        return 1;
    }

    uint8_t prefix[1];
    prefix[0] = reg;

    return at24cxx_interface_write_with_prefix(addr, prefix, sizeof(prefix), buf, len);
}

/**
 * @brief      interface iic bus read with 16 bits register address
 * @param[in]  addr iic device write address
 * @param[in]  reg iic register address
 * @param[out] *buf pointer to a data buffer
 * @param[in]  len length of the data buffer
 * @return     status code
 *             - 0 success
 *             - 1 read failed
 * @note       none
 */
uint8_t at24cxx_interface_iic_read_address16(uint8_t addr, uint16_t reg, uint8_t *buf, uint16_t len)
{
    if (len == 0U)
    {
        return 0;
    }
    if (buf == NULL)
    {
        return 1;
    }

    uint8_t reg_buf[2];
    reg_buf[0] = (uint8_t)(reg >> 8);
    reg_buf[1] = (uint8_t)(reg & 0xFFU);

    soft_i2c_status_t status = soft_i2c_master_write_read(&s_at24cxx_soft_i2c_bus,
                                                          addr,
                                                          reg_buf,
                                                          sizeof(reg_buf),
                                                          buf,
                                                          len);

    return (status == SOFT_I2C_STATUS_OK) ? 0U : 1U;
}

/**
 * @brief     interface iic bus write with 16 bits register address
 * @param[in] addr iic device write address
 * @param[in] reg iic register address
 * @param[in] *buf pointer to a data buffer
 * @param[in] len length of the data buffer
 * @return    status code
 *            - 0 success
 *            - 1 write failed
 * @note      none
 */
uint8_t at24cxx_interface_iic_write_address16(uint8_t addr, uint16_t reg, uint8_t *buf, uint16_t len)
{
    if (len == 0U)
    {
        return 0;
    }
    if (buf == NULL)
    {
        return 1;
    }

    uint8_t prefix[2];
    prefix[0] = (uint8_t)(reg >> 8);
    prefix[1] = (uint8_t)(reg & 0xFFU);

    return at24cxx_interface_write_with_prefix(addr, prefix, sizeof(prefix), buf, len);
}

/**
 * @brief     interface delay ms
 * @param[in] ms time
 * @note      none
 */
void at24cxx_interface_delay_ms(uint32_t ms)
{
    k_msleep(ms);
}

/**
 * @brief     interface print format data
 * @param[in] fmt format data
 * @note      none
 */
void at24cxx_interface_debug_print(const char *const fmt, ...)
{
    char buf[256];
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintk(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    if (n <= 0)
    {
        return;
    }
    if (n >= (int)sizeof buf)
    {
        n = (int)sizeof buf - 1;
        buf[n] = '\0';
    }

    printk("%s", buf);
}
