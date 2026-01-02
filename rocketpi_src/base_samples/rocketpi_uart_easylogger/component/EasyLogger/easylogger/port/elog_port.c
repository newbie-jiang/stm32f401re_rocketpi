/*
 * This file is part of the EasyLogger Library.
 *
 * Copyright (c) 2015, Armink, <armink.ztl@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * 'Software'), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Function: Portable interface for each platform.
 * Created on: 2015-04-28
 */
 
#include <elog.h>
#include <stdio.h>
#include "usart.h"

static volatile bool uart_dma_idle = true;
static volatile uint8_t elog_lock_flag = 0;

static void wait_uart_idle(void) {
    while (!uart_dma_idle) {
        /* wait for ongoing DMA transfer to finish */
    }
}

static bool uart_tx_dma(const uint8_t *buf, uint16_t len) {
    uart_dma_idle = false;
    if (HAL_UART_Transmit_DMA(&huart2, (uint8_t *)buf, len) != HAL_OK) {
        uart_dma_idle = true;
        return false;
    }
    return true;
}

/**
 * EasyLogger port initialize
 *
 * @return result
 */
ElogErrCode elog_port_init(void) {
    ElogErrCode result = ELOG_NO_ERR;

    uart_dma_idle = true;
    elog_lock_flag = 0;

    return result;
}

/**
 * EasyLogger port deinitialize
 *
 */
void elog_port_deinit(void) {

    wait_uart_idle();
}

/**
 * output log port interface
 *
 * @param log output of log
 * @param size log size
 */
void elog_port_output(const char *log, size_t size) {
    const uint8_t *cur = (const uint8_t *)log;

    if ((log == NULL) || (size == 0U)) {
        return;
    }

    while (size > 0U) {
        uint16_t chunk = (size > 0xFFFFU) ? 0xFFFFU : (uint16_t)size;

        wait_uart_idle();
        if (uart_tx_dma(cur, chunk)) {
            wait_uart_idle();
        } else {
            /* fallback to blocking transmit when DMA start fails */
            HAL_UART_Transmit(&huart2, (uint8_t *)cur, chunk, HAL_MAX_DELAY);
        }

        cur += chunk;
        size -= chunk;
    }
}

/**
 * output lock
 */
void elog_port_output_lock(void) {
    while (1) {
        uint32_t primask = __get_PRIMASK();
        __disable_irq();
        if (!elog_lock_flag) {
            elog_lock_flag = 1;
            if (!primask) {
                __enable_irq();
            }
            break;
        }
        if (!primask) {
            __enable_irq();
        }
    }
}

/**
 * output unlock
 */
void elog_port_output_unlock(void) {
    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    elog_lock_flag = 0;
    if (!primask) {
        __enable_irq();
    }
}

/**
 * get current time interface
 *
 * @return current time
 */
const char *elog_port_get_time(void) {
    static char cur_time[16];

    snprintf(cur_time, sizeof(cur_time), "%lu", HAL_GetTick());

    return cur_time;
}

/**
 * get current process name interface
 *
 * @return current process name
 */
const char *elog_port_get_p_info(void) {
    return "";
}

/**
 * get current thread name interface
 *
 * @return current thread name
 */
const char *elog_port_get_t_info(void) {
    return "";
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart == &huart2) {
        uart_dma_idle = true;
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart) {
    if (huart == &huart2) {
        uart_dma_idle = true;
    }
}
