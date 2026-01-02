/*
 * COPYRIGHT (C) STMicroelectronics 2015. All rights reserved.
 *
 * This software is the confidential and proprietary information of
 * STMicroelectronics ("Confidential Information").  You shall not
 * disclose such Confidential Information and shall use it only in
 * accordance with the terms of the license agreement you entered into
 * with STMicroelectronics
 *
 * Programming Golden Rule: Keep it Simple!
 *
 */

/*!
 * \file   vl53l0x_i2c_platform.c
 * \brief  VL53L0X platform layer port for STM32F401 (HAL I2C)
 */

#include "vl53l0x_i2c_platform.h"
#include "vl53l0x_platform_log.h"

#include "i2c.h"
#include "main.h"

#include <string.h>

#define STATUS_OK                   ((int32_t)0)
#define STATUS_FAIL                 ((int32_t)1)

#define VL53L0X_I2C_TIMEOUT_MS      ((uint32_t)100)
#define VL53L0X_I2C_MAX_TRANSFER    ((uint32_t)64)
#define VL53L0X_I2C_RETRY_COUNT     ((uint32_t)2)

#ifndef NULL
#define NULL ((void *)0)
#endif

static uint16_t vl53l0x_prepare_address(uint8_t i2c_addr);
static HAL_StatusTypeDef vl53l0x_i2c_write(uint16_t dev, uint8_t index,
                                           uint8_t *buffer, uint16_t length);
static HAL_StatusTypeDef vl53l0x_i2c_read(uint16_t dev, uint8_t index,
                                          uint8_t *buffer, uint16_t length);
static void vl53l0x_delay_us(uint32_t usec);

int32_t VL53L0X_comms_initialise(uint8_t comms_type, uint16_t comms_speed_khz)
{
    (void)comms_type;
    (void)comms_speed_khz;

    if (HAL_I2C_GetState(&hi2c3) == HAL_I2C_STATE_RESET)
    {
        MX_I2C3_Init();
    }

    /* Give the sensor time to boot after power-up or XSHUT release. */
    HAL_Delay(5U);

    return STATUS_OK;
}

int32_t VL53L0X_comms_close(void)
{
    return STATUS_OK;
}

int32_t VL53L0X_cycle_power(void)
{
    /* Add XSHUT toggling here if the board exposes it. */
    return STATUS_OK;
}

int32_t VL53L0X_write_multi(uint8_t address, uint8_t index, uint8_t *pdata, int32_t count)
{
    HAL_StatusTypeDef hal_status;
    uint32_t attempt = 0;

    if ((pdata == NULL) || (count <= 0) || ((uint32_t)count > VL53L0X_I2C_MAX_TRANSFER))
    {
        return STATUS_FAIL;
    }

    do
    {
        hal_status = vl53l0x_i2c_write(vl53l0x_prepare_address(address),
                                       index, pdata, (uint16_t)count);
        if (hal_status == HAL_BUSY)
        {
            (void)HAL_I2C_DeInit(&hi2c3);
            MX_I2C3_Init();
        }
        attempt++;
    } while ((hal_status == HAL_BUSY) && (attempt < VL53L0X_I2C_RETRY_COUNT));

    return (hal_status == HAL_OK) ? STATUS_OK : STATUS_FAIL;
}

int32_t VL53L0X_read_multi(uint8_t address, uint8_t index, uint8_t *pdata, int32_t count)
{
    HAL_StatusTypeDef hal_status;
    uint32_t attempt = 0;

    if ((pdata == NULL) || (count <= 0) || ((uint32_t)count > VL53L0X_I2C_MAX_TRANSFER))
    {
        return STATUS_FAIL;
    }

    do
    {
        hal_status = vl53l0x_i2c_read(vl53l0x_prepare_address(address),
                                      index, pdata, (uint16_t)count);
        if (hal_status == HAL_BUSY)
        {
            (void)HAL_I2C_DeInit(&hi2c3);
            MX_I2C3_Init();
        }
        attempt++;
    } while ((hal_status == HAL_BUSY) && (attempt < VL53L0X_I2C_RETRY_COUNT));

    return (hal_status == HAL_OK) ? STATUS_OK : STATUS_FAIL;
}

int32_t VL53L0X_write_byte(uint8_t address, uint8_t index, uint8_t data)
{
    return VL53L0X_write_multi(address, index, &data, 1);
}

int32_t VL53L0X_write_word(uint8_t address, uint8_t index, uint16_t data)
{
    uint8_t buffer[BYTES_PER_WORD];

    buffer[0] = (uint8_t)(data >> 8);
    buffer[1] = (uint8_t)(data & 0xFFU);

    return VL53L0X_write_multi(address, index, buffer, BYTES_PER_WORD);
}

int32_t VL53L0X_write_dword(uint8_t address, uint8_t index, uint32_t data)
{
    uint8_t buffer[BYTES_PER_DWORD];

    buffer[0] = (uint8_t)(data >> 24);
    buffer[1] = (uint8_t)(data >> 16);
    buffer[2] = (uint8_t)(data >> 8);
    buffer[3] = (uint8_t)(data & 0xFFU);

    return VL53L0X_write_multi(address, index, buffer, BYTES_PER_DWORD);
}

int32_t VL53L0X_read_byte(uint8_t address, uint8_t index, uint8_t *pdata)
{
    return VL53L0X_read_multi(address, index, pdata, 1);
}

int32_t VL53L0X_read_word(uint8_t address, uint8_t index, uint16_t *pdata)
{
    uint8_t buffer[BYTES_PER_WORD];
    int32_t status;

    status = VL53L0X_read_multi(address, index, buffer, BYTES_PER_WORD);
    if (status == STATUS_OK)
    {
        *pdata = ((uint16_t)buffer[0] << 8) | (uint16_t)buffer[1];
    }

    return status;
}

int32_t VL53L0X_read_dword(uint8_t address, uint8_t index, uint32_t *pdata)
{
    uint8_t buffer[BYTES_PER_DWORD];
    int32_t status;

    status = VL53L0X_read_multi(address, index, buffer, BYTES_PER_DWORD);
    if (status == STATUS_OK)
    {
        *pdata = ((uint32_t)buffer[0] << 24) |
                 ((uint32_t)buffer[1] << 16) |
                 ((uint32_t)buffer[2] << 8) |
                 (uint32_t)buffer[3];
    }

    return status;
}

int32_t VL53L0X_platform_wait_us(int32_t wait_us)
{
    if (wait_us <= 0)
    {
        return STATUS_OK;
    }

    vl53l0x_delay_us((uint32_t)wait_us);
    return STATUS_OK;
}

int32_t VL53L0X_wait_ms(int32_t wait_ms)
{
    if (wait_ms <= 0)
    {
        return STATUS_OK;
    }

    HAL_Delay((uint32_t)wait_ms);
    return STATUS_OK;
}

int32_t VL53L0X_set_gpio(uint8_t level)
{
    (void)level;
    return STATUS_OK;
}

int32_t VL53L0X_get_gpio(uint8_t *plevel)
{
    if (plevel != NULL)
    {
        *plevel = 0U;
    }
    return STATUS_OK;
}

int32_t VL53L0X_release_gpio(void)
{
    return STATUS_OK;
}

int32_t VL53L0X_get_timer_frequency(int32_t *ptimer_freq_hz)
{
    if (ptimer_freq_hz == NULL)
    {
        return STATUS_FAIL;
    }

    *ptimer_freq_hz = (int32_t)HAL_GetTickFreq();
    return STATUS_OK;
}

int32_t VL53L0X_get_timer_value(int32_t *ptimer_count)
{
    if (ptimer_count == NULL)
    {
        return STATUS_FAIL;
    }

    *ptimer_count = (int32_t)HAL_GetTick();
    return STATUS_OK;
}

static uint16_t vl53l0x_prepare_address(uint8_t i2c_addr)
{
    return (uint16_t)(i2c_addr & 0xFEU);
}

static HAL_StatusTypeDef vl53l0x_i2c_write(uint16_t dev, uint8_t index,
                                           uint8_t *buffer, uint16_t length)
{
    return HAL_I2C_Mem_Write(&hi2c3, dev, (uint16_t)index, I2C_MEMADD_SIZE_8BIT,
                             buffer, length, VL53L0X_I2C_TIMEOUT_MS);
}

static HAL_StatusTypeDef vl53l0x_i2c_read(uint16_t dev, uint8_t index,
                                          uint8_t *buffer, uint16_t length)
{
    return HAL_I2C_Mem_Read(&hi2c3, dev, (uint16_t)index, I2C_MEMADD_SIZE_8BIT,
                            buffer, length, VL53L0X_I2C_TIMEOUT_MS);
}

static void vl53l0x_delay_us(uint32_t usec)
{
#if defined(DWT)
    uint32_t start;
    uint32_t cycles;

    if ((CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk) == 0U)
    {
        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    }

    if ((DWT->CTRL & DWT_CTRL_CYCCNTENA_Msk) == 0U)
    {
        DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
        DWT->CYCCNT = 0U;
    }

    cycles = (SystemCoreClock / 1000000U) * usec;
    start = DWT->CYCCNT;
    while ((DWT->CYCCNT - start) < cycles)
    {
        /* busy wait */
    }
#else
    uint32_t wait_ms = (usec + 999U) / 1000U;
    HAL_Delay(wait_ms);
#endif
}
