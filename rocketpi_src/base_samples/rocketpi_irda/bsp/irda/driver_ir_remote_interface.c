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
 * @file      driver_ir_remote_interface_template.c
 * @brief     driver ir_remote interface template source file
 * @version   1.0.0
 * @author    Shifeng Li
 * @date      2023-03-31
 *
 * <h3>history</h3>
 * <table>
 * <tr><th>Date        <th>Version  <th>Author      <th>Description
 * <tr><td>2023/03/31  <td>1.0      <td>Shifeng Li  <td>first upload
 * </table>
 */

#include "driver_ir_remote_interface.h"
#include "main.h"
#include "tim.h"

#include <stdarg.h>
#include <stdio.h>

#define IR_REMOTE_TIM1_PERIOD_US    ((uint32_t)(htim1.Init.Period + 1U))

static volatile uint64_t s_tim1_elapsed_us = 0U;
static volatile uint8_t s_tim1_started = 0U;

static uint8_t ir_remote_interface_tim1_start(void)
{
    if (s_tim1_started != 0U)
    {
        return 0U;
    }

    HAL_TIM_StateTypeDef state = HAL_TIM_Base_GetState(&htim1);
    if (state == HAL_TIM_STATE_BUSY)
    {
        if (HAL_TIM_Base_Stop_IT(&htim1) != HAL_OK)
        {
            return 1U;
        }
        state = HAL_TIM_Base_GetState(&htim1);
    }
    if (state != HAL_TIM_STATE_READY)
    {
        return 1U;
    }

    s_tim1_elapsed_us = 0U;
    __HAL_TIM_SET_COUNTER(&htim1, 0U);
    __HAL_TIM_CLEAR_FLAG(&htim1, TIM_FLAG_UPDATE);

    if (HAL_TIM_Base_Start_IT(&htim1) != HAL_OK)
    {
        return 1U;
    }

    s_tim1_started = 1U;

    return 0U;
}

/**
 * @brief  interface timer init
 * @return status code
 *         - 0 success
 *         - 1 init failed
 * @note   none
 */
uint8_t ir_remote_interface_timer_init(void)
{
    s_tim1_started = 0U;

    return ir_remote_interface_tim1_start();
}

/**
 * @brief     interface timestamp read
 * @param[in] *t pointer to an ir_remote_time structure
 * @return    status code
 *            - 0 success
 *            - 1 read failed
 * @note      none
 */
uint8_t ir_remote_interface_timestamp_read(ir_remote_time_t *t)
{
    if (t == NULL)
    {
        return 1;
    }

    if (ir_remote_interface_tim1_start() != 0U)
    {
        return 1;
    }

    uint32_t primask = __get_PRIMASK();
    __disable_irq();

    uint64_t base_us = s_tim1_elapsed_us;
    uint32_t counter = __HAL_TIM_GET_COUNTER(&htim1);

    if (__HAL_TIM_GET_FLAG(&htim1, TIM_FLAG_UPDATE) != RESET)
    {
        base_us += IR_REMOTE_TIM1_PERIOD_US;
        counter = __HAL_TIM_GET_COUNTER(&htim1);
    }

    if (primask == 0U)
    {
        __enable_irq();
    }

    uint64_t total_us = base_us + (uint64_t)counter;
    t->s = total_us / 1000000ULL;
    t->us = (uint32_t)(total_us % 1000000ULL);

    return 0;
}

/**
 * @brief     interface delay ms
 * @param[in] ms time
 * @note      none
 */
void ir_remote_interface_delay_ms(uint32_t ms)
{
    HAL_Delay(ms);
}

/**
 * @brief     interface print format data
 * @param[in] fmt format data
 * @note      none
 */
void ir_remote_interface_debug_print(const char *const fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    (void)vprintf(fmt, args);
    va_end(args);
}

/**
 * @brief     interface receive callback
 * @param[in] *data pointer to an ir_remote_t structure
 * @note      none
 */
void ir_remote_interface_receive_callback(ir_remote_t *data)
{
    switch (data->status)
    {
        case IR_REMOTE_STATUS_OK :
        {
            ir_remote_interface_debug_print("ir_remote: irq ok.\n");
            ir_remote_interface_debug_print("ir_remote: add is 0x%02X and cmd is 0x%02X.\n", data->address, data->command);
            
            break;
        }
        case IR_REMOTE_STATUS_REPEAT :
        {
            ir_remote_interface_debug_print("ir_remote: irq repeat.\n");
            ir_remote_interface_debug_print("ir_remote: add is 0x%02X and cmd is 0x%02X.\n", data->address, data->command);
            
            break;
        }
        case IR_REMOTE_STATUS_ADDR_ERR :
        {
            ir_remote_interface_debug_print("ir_remote: irq addr error.\n");
            
            break;
        }
        case IR_REMOTE_STATUS_CMD_ERR :
        {
            ir_remote_interface_debug_print("ir_remote: irq cmd error.\n");
            
            break;
        }
        case IR_REMOTE_STATUS_FRAME_INVALID :
        {
            ir_remote_interface_debug_print("ir_remote: irq frame invalid.\n");
            
            break;
        }
        default :
        {
            ir_remote_interface_debug_print("ir_remote: irq unknown status.\n");
            
            break;
        }
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM1)
    {
        s_tim1_elapsed_us += IR_REMOTE_TIM1_PERIOD_US;
    }
}
