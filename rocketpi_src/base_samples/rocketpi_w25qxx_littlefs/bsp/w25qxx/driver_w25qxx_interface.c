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
 * @file      driver_w25qxx_interface_template.c
 * @brief     driver w25qxx interface template source file
 * @version   1.0.0
 * @author    Shifeng Li
 * @date      2021-07-15
 *
 * <h3>history</h3>
 * <table>
 * <tr><th>Date        <th>Version  <th>Author      <th>Description
 * <tr><td>2021/07/15  <td>1.0      <td>Shifeng Li  <td>first upload
 * </table>
 */

#include "driver_w25qxx_interface.h"

#include "main.h"
#include "spi.h"

#include <stdarg.h>
#include <stdio.h>

#define W25QXX_CMD_PAGE_PROGRAM           0x02U
#define W25QXX_CMD_QUAD_PAGE_PROGRAM      0x32U
#define W25QXX_CMD_FAST_READ              0x0BU
#define W25QXX_CMD_FAST_READ_QUAD_OUTPUT  0x6BU
#define W25QXX_CMD_FAST_READ_QUAD_IO      0xEBU
#define W25QXX_CMD_DEVICE_ID_QUAD_IO      0x94U

/* SPI2 handle comes from STM32CubeMX generated code */
extern SPI_HandleTypeDef hspi2;

/* ensure DWT based delay is configured before first use */
static void w25qxx_interface_dwt_delay_prepare(void)
{
    /* enable tracing and the cycle counter if not already enabled */
    if ((CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk) == 0U)
    {
        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    }
    if ((DWT->CTRL & DWT_CTRL_CYCCNTENA_Msk) == 0U)
    {
        DWT->CYCCNT = 0U;
        DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    }
}

/* chunked transmit helper since HAL uses uint16_t length */
static HAL_StatusTypeDef w25qxx_interface_spi_transmit(const uint8_t *data, uint32_t len)
{
    while (len > 0U)
    {
        uint16_t chunk = (len > 0xFFFFU) ? 0xFFFFU : (uint16_t)len;
        if (HAL_SPI_Transmit(&hspi2, (uint8_t *)data, chunk, HAL_MAX_DELAY) != HAL_OK)
        {
            return HAL_ERROR;
        }
        data += chunk;
        len  -= chunk;
    }

    return HAL_OK;
}

/* chunked receive helper since HAL uses uint16_t length */
static HAL_StatusTypeDef w25qxx_interface_spi_receive(uint8_t *data, uint32_t len)
{
    while (len > 0U)
    {
        uint16_t chunk = (len > 0xFFFFU) ? 0xFFFFU : (uint16_t)len;
        if (HAL_SPI_Receive(&hspi2, data, chunk, HAL_MAX_DELAY) != HAL_OK)
        {
            return HAL_ERROR;
        }
        data += chunk;
        len  -= chunk;
    }

    return HAL_OK;
}

/**
 * @brief  interface spi qspi bus init
 * @return status code
 *         - 0 success
 *         - 1 spi qspi init failed
 * @note   none
 */
uint8_t w25qxx_interface_spi_qspi_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* ensure chip-select GPIO is configured and idles high */
    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIO_InitStruct.Pin   = W25QXX_CS_Pin;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(W25QXX_CS_GPIO_Port, &GPIO_InitStruct);
    HAL_GPIO_WritePin(W25QXX_CS_GPIO_Port, W25QXX_CS_Pin, GPIO_PIN_SET);

    /* initialise SPI2 if it hasn't been configured yet */
    if (HAL_SPI_GetState(&hspi2) == HAL_SPI_STATE_RESET)
    {
        MX_SPI2_Init();
    }

    if (HAL_SPI_GetState(&hspi2) != HAL_SPI_STATE_READY)
    {
        return 1;
    }

    return 0;
}

/**
 * @brief  interface spi qspi bus deinit
 * @return status code
 *         - 0 success
 *         - 1 spi qspi deinit failed
 * @note   none
 */
uint8_t w25qxx_interface_spi_qspi_deinit(void)
{
    if (HAL_SPI_DeInit(&hspi2) != HAL_OK)
    {
        return 1;
    }

    HAL_GPIO_WritePin(W25QXX_CS_GPIO_Port, W25QXX_CS_Pin, GPIO_PIN_SET);

    return 0;
}

/**
 * @brief      interface spi qspi bus write read
 * @param[in]  instruction sent instruction
 * @param[in]  instruction_line instruction phy lines
 * @param[in]  address register address
 * @param[in]  address_line address phy lines
 * @param[in]  address_len address length
 * @param[in]  alternate register address
 * @param[in]  alternate_line alternate phy lines
 * @param[in]  alternate_len alternate length
 * @param[in]  dummy dummy cycle
 * @param[in]  *in_buf pointer to a input buffer
 * @param[in]  in_len input length
 * @param[out] *out_buf pointer to a output buffer
 * @param[in]  out_len output length
 * @param[in]  data_line data phy lines
 * @return     status code
 *             - 0 success
 *             - 1 write read failed
 * @note       none
 */
uint8_t w25qxx_interface_spi_qspi_write_read(uint8_t instruction, uint8_t instruction_line,
                                             uint32_t address, uint8_t address_line, uint8_t address_len,
                                             uint32_t alternate, uint8_t alternate_line, uint8_t alternate_len,
                                             uint8_t dummy, uint8_t *in_buf, uint32_t in_len,
                                             uint8_t *out_buf, uint32_t out_len, uint8_t data_line)
{
    uint8_t header[16];
    uint32_t header_len = 0U;
    uint32_t dummy_bytes = dummy / 8U;
    HAL_StatusTypeDef status;
    uint8_t effective_instruction = instruction;
    uint8_t effective_instruction_line = instruction_line;
    uint8_t effective_address_line = address_line;
    uint8_t effective_alternate_line = alternate_line;
    uint8_t effective_data_line = data_line;

    if ((effective_data_line > 1U) && (in_len > 0U))
    {
        if (effective_instruction == W25QXX_CMD_QUAD_PAGE_PROGRAM)
        {
            effective_instruction = W25QXX_CMD_PAGE_PROGRAM;
        }
    }
    if ((effective_data_line > 1U) && (out_len > 0U))
    {
        if ((effective_instruction == W25QXX_CMD_FAST_READ_QUAD_OUTPUT) ||
            (effective_instruction == W25QXX_CMD_FAST_READ_QUAD_IO))
        {
            effective_instruction = W25QXX_CMD_FAST_READ;
        }
    }
    if ((effective_instruction_line > 1U) || (effective_address_line > 1U) ||
        (effective_alternate_line > 1U) || (effective_data_line > 1U))
    {
        effective_instruction_line = (effective_instruction_line > 0U) ? 1U : 0U;
        effective_address_line = (address_len > 0U) ? 1U : 0U;
        effective_alternate_line = (alternate_len > 0U) ? 1U : 0U;
        effective_data_line = (in_len > 0U || out_len > 0U) ? 1U : 0U;
    }
    if ((in_len > 0U && in_buf == NULL) || (out_len > 0U && out_buf == NULL))
    {
        return 1;
    }
    if ((address_len > sizeof(uint32_t)) || (alternate_len > sizeof(uint32_t)))
    {
        return 1;
    }
    if ((dummy % 8U) != 0U)
    {
        dummy_bytes += 1U;
    }
    if ((header_len < sizeof(header)) && (effective_instruction_line != 0U))
    {
        header[header_len++] = effective_instruction;
    }

    if (address_len != 0U)
    {
        if ((header_len + address_len) > sizeof(header))
        {
            return 1;
        }
        for (uint8_t i = 0U; i < address_len; ++i)
        {
            uint8_t shift = (uint8_t)((address_len - 1U - i) * 8U);
            header[header_len++] = (uint8_t)((address >> shift) & 0xFFU);
        }
    }

    if (alternate_len != 0U)
    {
        if ((header_len + alternate_len) > sizeof(header))
        {
            return 1;
        }
        for (uint8_t i = 0U; i < alternate_len; ++i)
        {
            uint8_t shift = (uint8_t)((alternate_len - 1U - i) * 8U);
            header[header_len++] = (uint8_t)((alternate >> shift) & 0xFFU);
        }
    }

    HAL_GPIO_WritePin(W25QXX_CS_GPIO_Port, W25QXX_CS_Pin, GPIO_PIN_RESET);

    if (header_len > 0U)
    {
        status = w25qxx_interface_spi_transmit(header, header_len);
        if (status != HAL_OK)
        {
            HAL_GPIO_WritePin(W25QXX_CS_GPIO_Port, W25QXX_CS_Pin, GPIO_PIN_SET);
            return 1;
        }
    }

    if (in_len > 0U)
    {
        status = w25qxx_interface_spi_transmit(in_buf, in_len);
        if (status != HAL_OK)
        {
            HAL_GPIO_WritePin(W25QXX_CS_GPIO_Port, W25QXX_CS_Pin, GPIO_PIN_SET);
            return 1;
        }
    }

    if (dummy_bytes > 0U)
    {
        uint8_t dummy_buf[8] = {0};
        uint32_t remaining = dummy_bytes;
        while (remaining > 0U)
        {
            uint32_t send = (remaining > sizeof(dummy_buf)) ? sizeof(dummy_buf) : remaining;
            status = w25qxx_interface_spi_transmit(dummy_buf, send);
            if (status != HAL_OK)
            {
                HAL_GPIO_WritePin(W25QXX_CS_GPIO_Port, W25QXX_CS_Pin, GPIO_PIN_SET);
                return 1;
            }
            remaining -= send;
        }
    }

    if (out_len > 0U)
    {
        status = w25qxx_interface_spi_receive(out_buf, out_len);
        if (status != HAL_OK)
        {
            HAL_GPIO_WritePin(W25QXX_CS_GPIO_Port, W25QXX_CS_Pin, GPIO_PIN_SET);
            return 1;
        }
    }

    HAL_GPIO_WritePin(W25QXX_CS_GPIO_Port, W25QXX_CS_Pin, GPIO_PIN_SET);

    return 0;
}

/**
 * @brief     interface delay ms
 * @param[in] ms time
 * @note      none
 */
void w25qxx_interface_delay_ms(uint32_t ms)
{
    HAL_Delay(ms);
}

/**
 * @brief     interface delay us
 * @param[in] us time
 * @note      none
 */
void w25qxx_interface_delay_us(uint32_t us)
{
    if (us == 0U)
    {
        return;
    }

    w25qxx_interface_dwt_delay_prepare();

    uint32_t cycles_per_us = HAL_RCC_GetHCLKFreq() / 1000000U;
    uint32_t total_cycles = cycles_per_us * us;
    uint32_t start = DWT->CYCCNT;

    while ((DWT->CYCCNT - start) < total_cycles)
    {
        __NOP();
    }
}

/**
 * @brief     interface print format data
 * @param[in] fmt format data
 * @note      none
 */
//void w25qxx_interface_debug_print(const char *const fmt, ...)
//{
//   va_list args;
//	 va_start(args, fmt);
//   vprintf(fmt, args);
//   va_end(args);	
//}

void w25qxx_interface_debug_print(const char *const fmt, ...)
{
	  char buf[256];
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(buf, sizeof buf, fmt, ap);
    va_end(ap);
    if (n < 0) return;
    if (n >= (int)sizeof buf) n = sizeof buf - 1;

    /* 输出时把 \n 规范为 \r\n；末尾无换行则补一行 */
    for (int i = 0; i < n; ++i) {
        if (buf[i] == '\n') fputc('\r', stdout);
        fputc((unsigned char)buf[i], stdout);
    }
    if (n == 0 || buf[n - 1] != '\n') { fputc('\r', stdout); fputc('\n', stdout); } 
}
