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
 * @file      driver_at24cxx_interface_template.c
 * @brief     driver at24cxx interface template source file
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
#include "main.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define AT24CXX_SOFT_I2C_DELAY_TICKS          120U
#define AT24CXX_INTERFACE_TX_STACK_BUFFER_SIZE 66U

static soft_i2c_bus_t s_at24cxx_soft_i2c_bus;

static void at24cxx_enable_gpio_clock(GPIO_TypeDef *port);
static soft_i2c_status_t at24cxx_scl_write(void *ctx, soft_i2c_pin_state_t state);
static soft_i2c_pin_state_t at24cxx_scl_read(void *ctx);
static soft_i2c_status_t at24cxx_sda_write(void *ctx, soft_i2c_pin_state_t state);
static soft_i2c_pin_state_t at24cxx_sda_read(void *ctx);
static uint8_t at24cxx_interface_write_with_prefix(uint8_t addr,
                                                   const uint8_t *prefix,
                                                   size_t prefix_len,
                                                   const uint8_t *buf,
                                                   uint16_t len);

static void at24cxx_enable_gpio_clock(GPIO_TypeDef *port)
{
#if defined(GPIOA) && defined(__HAL_RCC_GPIOA_CLK_ENABLE)
    if (port == GPIOA)
    {
        __HAL_RCC_GPIOA_CLK_ENABLE();
        return;
    }
#endif
#if defined(GPIOB) && defined(__HAL_RCC_GPIOB_CLK_ENABLE)
    if (port == GPIOB)
    {
        __HAL_RCC_GPIOB_CLK_ENABLE();
        return;
    }
#endif
#if defined(GPIOC) && defined(__HAL_RCC_GPIOC_CLK_ENABLE)
    if (port == GPIOC)
    {
        __HAL_RCC_GPIOC_CLK_ENABLE();
        return;
    }
#endif
#if defined(GPIOD) && defined(__HAL_RCC_GPIOD_CLK_ENABLE)
    if (port == GPIOD)
    {
        __HAL_RCC_GPIOD_CLK_ENABLE();
        return;
    }
#endif
#if defined(GPIOE) && defined(__HAL_RCC_GPIOE_CLK_ENABLE)
    if (port == GPIOE)
    {
        __HAL_RCC_GPIOE_CLK_ENABLE();
        return;
    }
#endif
#if defined(GPIOF) && defined(__HAL_RCC_GPIOF_CLK_ENABLE)
    if (port == GPIOF)
    {
        __HAL_RCC_GPIOF_CLK_ENABLE();
        return;
    }
#endif
#if defined(GPIOG) && defined(__HAL_RCC_GPIOG_CLK_ENABLE)
    if (port == GPIOG)
    {
        __HAL_RCC_GPIOG_CLK_ENABLE();
        return;
    }
#endif
#if defined(GPIOH) && defined(__HAL_RCC_GPIOH_CLK_ENABLE)
    if (port == GPIOH)
    {
        __HAL_RCC_GPIOH_CLK_ENABLE();
        return;
    }
#endif
#if defined(GPIOI) && defined(__HAL_RCC_GPIOI_CLK_ENABLE)
    if (port == GPIOI)
    {
        __HAL_RCC_GPIOI_CLK_ENABLE();
        return;
    }
#endif
#if defined(GPIOJ) && defined(__HAL_RCC_GPIOJ_CLK_ENABLE)
    if (port == GPIOJ)
    {
        __HAL_RCC_GPIOJ_CLK_ENABLE();
        return;
    }
#endif
#if defined(GPIOK) && defined(__HAL_RCC_GPIOK_CLK_ENABLE)
    if (port == GPIOK)
    {
        __HAL_RCC_GPIOK_CLK_ENABLE();
        return;
    }
#endif
}

static soft_i2c_status_t at24cxx_scl_write(void *ctx, soft_i2c_pin_state_t state)
{
    (void)ctx;
    HAL_GPIO_WritePin(AT24CXX_SCL_GPIO_Port, AT24CXX_SCL_Pin,
                      (state == SOFT_I2C_PIN_SET) ? GPIO_PIN_SET : GPIO_PIN_RESET);

    return SOFT_I2C_STATUS_OK;
}

static soft_i2c_pin_state_t at24cxx_scl_read(void *ctx)
{
    (void)ctx;
    return (HAL_GPIO_ReadPin(AT24CXX_SCL_GPIO_Port, AT24CXX_SCL_Pin) == GPIO_PIN_SET)
               ? SOFT_I2C_PIN_SET
               : SOFT_I2C_PIN_RESET;
}

static soft_i2c_status_t at24cxx_sda_write(void *ctx, soft_i2c_pin_state_t state)
{
    (void)ctx;
    HAL_GPIO_WritePin(AT24CXX_SDA_GPIO_Port, AT24CXX_SDA_Pin,
                      (state == SOFT_I2C_PIN_SET) ? GPIO_PIN_SET : GPIO_PIN_RESET);

    return SOFT_I2C_STATUS_OK;
}

static soft_i2c_pin_state_t at24cxx_sda_read(void *ctx)
{
    (void)ctx;
    return (HAL_GPIO_ReadPin(AT24CXX_SDA_GPIO_Port, AT24CXX_SDA_Pin) == GPIO_PIN_SET)
               ? SOFT_I2C_PIN_SET
               : SOFT_I2C_PIN_RESET;
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
        tx_buf = (uint8_t *)malloc(tx_len);
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
        free(tx_buf);
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
    GPIO_InitTypeDef init = {0};

    at24cxx_enable_gpio_clock(AT24CXX_SCL_GPIO_Port);
    at24cxx_enable_gpio_clock(AT24CXX_SDA_GPIO_Port);

    init.Pin = AT24CXX_SCL_Pin;
    init.Mode = GPIO_MODE_OUTPUT_OD;
    init.Pull = GPIO_PULLUP;
    init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(AT24CXX_SCL_GPIO_Port, &init);

    init.Pin = AT24CXX_SDA_Pin;
    HAL_GPIO_Init(AT24CXX_SDA_GPIO_Port, &init);

    HAL_GPIO_WritePin(AT24CXX_SCL_GPIO_Port, AT24CXX_SCL_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(AT24CXX_SDA_GPIO_Port, AT24CXX_SDA_Pin, GPIO_PIN_SET);

    s_at24cxx_soft_i2c_bus.scl.write = at24cxx_scl_write;
    s_at24cxx_soft_i2c_bus.scl.read = at24cxx_scl_read;
    s_at24cxx_soft_i2c_bus.scl.ctx = NULL;
    s_at24cxx_soft_i2c_bus.sda.write = at24cxx_sda_write;
    s_at24cxx_soft_i2c_bus.sda.read = at24cxx_sda_read;
    s_at24cxx_soft_i2c_bus.sda.ctx = NULL;
    s_at24cxx_soft_i2c_bus.delay_fn = NULL;
    s_at24cxx_soft_i2c_bus.delay_ctx = NULL;
    s_at24cxx_soft_i2c_bus.delay_ticks = AT24CXX_SOFT_I2C_DELAY_TICKS;
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
    if (AT24CXX_SCL_GPIO_Port == AT24CXX_SDA_GPIO_Port)
    {
        HAL_GPIO_DeInit(AT24CXX_SCL_GPIO_Port, AT24CXX_SCL_Pin | AT24CXX_SDA_Pin);
    }
    else
    {
        HAL_GPIO_DeInit(AT24CXX_SCL_GPIO_Port, AT24CXX_SCL_Pin);
        HAL_GPIO_DeInit(AT24CXX_SDA_GPIO_Port, AT24CXX_SDA_Pin);
    }

    s_at24cxx_soft_i2c_bus.initialized = 0U;

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
    HAL_Delay(ms);
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
    int n = vsnprintf(buf, sizeof buf, fmt, ap);
    va_end(ap);
    if (n < 0) return;
    if (n >= (int)sizeof buf) n = sizeof buf - 1;

    for (int i = 0; i < n; ++i) {
        if (buf[i] == '\n') fputc('\r', stdout);
        fputc((unsigned char)buf[i], stdout);
    }
    if (n == 0 || buf[n - 1] != '\n') { fputc('\r', stdout); fputc('\n', stdout); }
}
