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
 * @file      driver_hcsr04_interface_template.c
 * @brief     driver hcsr04 interface template source file
 * @version   2.0.0
 * @author    Shifeng Li
 * @date      2021-04-15
 *
 * <h3>history</h3>
 * <table>
 * <tr><th>Date        <th>Version  <th>Author      <th>Description
 * <tr><td>2021/04/15  <td>2.0      <td>Shifeng Li  <td>format the code
 * <tr><td>2020/12/21  <td>1.0      <td>Shifeng Li  <td>first upload
 * </table>
 */

#include "driver_hcsr04_interface.h"

#include "main.h"
#include "tim.h"

#include <stdarg.h>
#include <stdio.h>

/* TIM1 rolls over every 0x10000 ticks with 1 us resolution */
#define TIM1_AUTORELOAD_TICKS   (0x10000UL)

static volatile uint32_t s_tim1_overflow_count = 0;   /* number of completed TIM1 periods */
static uint8_t s_timer_started = 0;                   /* flag to avoid re-starting TIM1 */

/**
 * @brief  Start TIM1 in interrupt mode if it isn't running yet.
 * @return 0 on success, 1 on failure
 */
static uint8_t hcsr04_interface_start_timer_if_needed(void)
{
    if (s_timer_started != 0)
    {
        return 0;
    }

    if (htim1.Instance != TIM1)
    {
        hcsr04_interface_debug_print("hcsr04: TIM1 not initialized.\n");
        return 1;
    }

    /* make sure the IRQ is ready before the timer starts generating events */
    HAL_NVIC_SetPriority(TIM1_UP_TIM10_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(TIM1_UP_TIM10_IRQn);

    s_tim1_overflow_count = 0;
    __HAL_TIM_SET_COUNTER(&htim1, 0);
    if (HAL_TIM_Base_Start_IT(&htim1) != HAL_OK)
    {
        hcsr04_interface_debug_print("hcsr04: start TIM1 failed.\n");
        return 1;
    }

    s_timer_started = 1;

    return 0;
}

/**
 * @brief Get the current monotonic time in microseconds using TIM1.
 * @return 64-bit timestamp in microseconds
 */
static uint64_t hcsr04_interface_get_time_us(void)
{
    if (htim1.Instance != TIM1)
    {
        return 0ULL;
    }

    uint32_t overflow_before;
    uint32_t counter;

    do
    {
        overflow_before = s_tim1_overflow_count;
        counter = __HAL_TIM_GET_COUNTER(&htim1);
    } while (overflow_before != s_tim1_overflow_count);

    return ((uint64_t)overflow_before * TIM1_AUTORELOAD_TICKS) + counter;
}
/**
 * @brief  interface trig init
 * @return status code
 *         - 0 success
 *         - 1 trig init failed
 * @note   none
 */
uint8_t hcsr04_interface_trig_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOC_CLK_ENABLE();

    GPIO_InitStruct.Pin = TRIGGER_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(TRIGGER_GPIO_Port, &GPIO_InitStruct);

    HAL_GPIO_WritePin(TRIGGER_GPIO_Port, TRIGGER_Pin, GPIO_PIN_RESET);

    return 0;
}

/**
 * @brief  interface trig deinit
 * @return status code
 *         - 0 success
 *         - 1 trig deinit failed
 * @note   none
 */
uint8_t hcsr04_interface_trig_deinit(void)
{
    HAL_GPIO_DeInit(TRIGGER_GPIO_Port, TRIGGER_Pin);

    if (s_timer_started != 0)
    {
        if (HAL_TIM_Base_Stop_IT(&htim1) != HAL_OK)
        {
            return 1;
        }
        HAL_NVIC_DisableIRQ(TIM1_UP_TIM10_IRQn);
        s_timer_started = 0;
        s_tim1_overflow_count = 0;
    }

    return 0;
}

/**
 * @brief     interface trig write
 * @param[in] value written value
 * @return    status code
 *            - 0 success
 *            - 1 trig write failed
 * @note      none
 */
uint8_t hcsr04_interface_trig_write(uint8_t value)
{
    HAL_GPIO_WritePin(TRIGGER_GPIO_Port,
                      TRIGGER_Pin,
                      (value != 0U) ? GPIO_PIN_SET : GPIO_PIN_RESET);

    return 0;
}

/**
 * @brief  interface echo init
 * @return status code
 *         - 0 success
 *         - 1 echo init failed
 * @note   none
 */
uint8_t hcsr04_interface_echo_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOC_CLK_ENABLE();

    GPIO_InitStruct.Pin = ECHO_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(ECHO_GPIO_Port, &GPIO_InitStruct);

    return 0;
}

/**
 * @brief  interface echo deinit
 * @return status code
 *         - 0 success
 *         - 1 trig echo failed
 * @note   none
 */
uint8_t hcsr04_interface_echo_deinit(void)
{
    HAL_GPIO_DeInit(ECHO_GPIO_Port, ECHO_Pin);

    return 0;
}

/**
 * @brief      interface echo read
 * @param[out] *value pointer to a value buffer
 * @return     status code
 *             - 0 success
 *             - 1 echo read failed
 * @note       none
 */
uint8_t hcsr04_interface_echo_read(uint8_t *value)
{
    if (value == NULL)
    {
        return 1;
    }

    *value = (uint8_t)HAL_GPIO_ReadPin(ECHO_GPIO_Port, ECHO_Pin);

    return 0;
}

/**
 * @brief      interface timestamp read
 * @param[out] *t pointer to a time structure
 * @return     status code
 *             - 0 success
 *             - 1 timestamp read failed
 * @note       none
 */
uint8_t hcsr04_interface_timestamp_read(hcsr04_time_t *t)
{
    if (t == NULL)
    {
        return 1;
    }

    if (hcsr04_interface_start_timer_if_needed() != 0)
    {
        return 1;
    }

    uint64_t now_us = hcsr04_interface_get_time_us();

    t->millisecond = (uint32_t)(now_us / 1000ULL);
    t->microsecond = now_us % 1000ULL;

    return 0;
}

/**
 * @brief     interface delay us
 * @param[in] us time
 * @note      none
 */
void hcsr04_interface_delay_us(uint32_t us)
{
    if (us == 0U)
    {
        return;
    }

    if (hcsr04_interface_start_timer_if_needed() != 0)
    {
        return;
    }

    uint64_t start = hcsr04_interface_get_time_us();
    while ((hcsr04_interface_get_time_us() - start) < us)
    {
        /* busy wait to preserve microsecond precision */
    }

}

/**
 * @brief     interface delay ms
 * @param[in] ms time
 * @note      none
 */
void hcsr04_interface_delay_ms(uint32_t ms)
{
    HAL_Delay(ms);

}

/**
 * @brief  TIM period elapsed callback used to extend the 16-bit timer counter.
 * @param  htim pointer to the TIM handle
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM1)
    {
        s_tim1_overflow_count++;
    }
}

/**
 * @brief     interface print format data
 * @param[in] fmt format data
 * @note      none
 */

//void hcsr04_interface_debug_print(const char *const fmt, ...)
//{
//	 va_list args;
//	 va_start(args, fmt);
//   vprintf(fmt, args);
//   va_end(args);	
//}


void hcsr04_interface_debug_print(const char *const fmt, ...)
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
