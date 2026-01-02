/**
 * Copyright (c) 2025
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * @file      driver_adc_joystick_lcd.c
 * @brief     Joystick visualization on ST7789 LCD
 * @version   1.0.0
 * @date      2025-10-19
 */

#include "driver_adc_joystick_lcd.h"

#include "st7789.h"

#include <stdio.h>

#define ADC_JOYSTICK_LCD_ADC_MAX_VALUE    4095U
#define ADC_JOYSTICK_LCD_DOT_RADIUS       8U
#define ADC_JOYSTICK_LCD_BACKGROUND_COLOR BLACK
#define ADC_JOYSTICK_LCD_ACTIVE_COLOR     CYAN

static uint16_t s_prev_x = 0U;
static uint16_t s_prev_y = 0U;
static uint8_t s_has_dot = 0U;
static uint8_t s_lcd_ready = 0U;

static void adc_joystick_lcd_draw_filled_circle(uint16_t cx, uint16_t cy, uint16_t radius, uint16_t color);
static uint16_t adc_joystick_lcd_map_axis(uint16_t raw, uint16_t extent);

static void adc_joystick_lcd_draw_filled_circle(uint16_t cx, uint16_t cy, uint16_t radius, uint16_t color)
{
    const int32_t r = (int32_t)radius;
    const int32_t r_sq = r * r;

    for (int32_t dy = -r; dy <= r; ++dy)
    {
        const int32_t y = (int32_t)cy + dy;
        if (y < 0 || y >= (int32_t)ST7789_HIGHT)
        {
            continue;
        }

        for (int32_t dx = -r; dx <= r; ++dx)
        {
            if ((dx * dx + dy * dy) > r_sq)
            {
                continue;
            }

            const int32_t x = (int32_t)cx + dx;
            if (x < 0 || x >= (int32_t)ST7789_WIDTH)
            {
                continue;
            }

            ST7789_DrawPixel((uint16_t)x, (uint16_t)y, color);
        }
    }
}

static uint16_t adc_joystick_lcd_map_axis(uint16_t raw, uint16_t extent)
{
    const uint16_t margin = ADC_JOYSTICK_LCD_DOT_RADIUS;
    const uint16_t max_coord = (extent == 0U) ? 0U : (extent - 1U);
    uint32_t usable_range;
    uint32_t coord;

    if (extent <= (margin * 2U))
    {
        return margin;
    }

    usable_range = (uint32_t)max_coord - (uint32_t)margin * 2U;
    coord = ((uint32_t)raw * usable_range + (ADC_JOYSTICK_LCD_ADC_MAX_VALUE / 2U)) / ADC_JOYSTICK_LCD_ADC_MAX_VALUE;
    coord += margin;
    if (coord > max_coord)
    {
        coord = max_coord;
    }

    return (uint16_t)coord;
}

void adc_joystick_lcd_init(void)
{
    ST7789_Init();
    ST7789_Clear(ADC_JOYSTICK_LCD_BACKGROUND_COLOR);

    s_prev_x = ST7789_WIDTH / 2U;
    s_prev_y = ST7789_HIGHT / 2U;
    s_has_dot = 0U;
    s_lcd_ready = 1U;

    printf("adc joystick lcd: initialized (screen %ux%u)\r\n",
           (unsigned int)ST7789_WIDTH,
           (unsigned int)ST7789_HIGHT);
}

void adc_joystick_lcd_draw(const adc_joystick_sample_t *sample)
{
    uint16_t x;
    uint16_t y;

    if (sample == NULL)
    {
        return;
    }
    if (s_lcd_ready == 0U)
    {
        adc_joystick_lcd_init();
    }

    x = adc_joystick_lcd_map_axis(sample->x.raw, ST7789_WIDTH);
    y = adc_joystick_lcd_map_axis(sample->y.raw, ST7789_HIGHT);

    if (s_has_dot != 0U)
    {
        adc_joystick_lcd_draw_filled_circle(s_prev_x, s_prev_y, ADC_JOYSTICK_LCD_DOT_RADIUS, ADC_JOYSTICK_LCD_BACKGROUND_COLOR);
    }

    adc_joystick_lcd_draw_filled_circle(x, y, ADC_JOYSTICK_LCD_DOT_RADIUS, ADC_JOYSTICK_LCD_ACTIVE_COLOR);

    printf("adc joystick lcd: pos=(%3u,%3u) raw=(%4u,%4u) permille=(%u,%u) KEY=%s\r\n",
           (unsigned int)x,
           (unsigned int)y,
           (unsigned int)sample->x.raw,
           (unsigned int)sample->y.raw,
           (unsigned int)sample->x.permille,
           (unsigned int)sample->y.permille,
           (sample->key_pressed != 0U) ? "DOWN" : "UP");

    s_prev_x = x;
    s_prev_y = y;
    s_has_dot = 1U;
}

uint8_t adc_joystick_lcd_run(uint32_t frame_count, uint32_t delay_ms)
{
    const uint32_t delay = (delay_ms == 0U) ? ADC_JOYSTICK_LCD_DEFAULT_DELAY_MS : delay_ms;
    uint32_t frames = 0U;

    if (s_lcd_ready == 0U)
    {
        adc_joystick_lcd_init();
    }

    while ((frame_count == 0U) || (frames < frame_count))
    {
        adc_joystick_sample_t sample;

        if (adc_joystick_test_sample(&sample) != 0U)
        {
            printf("adc joystick lcd: failed to read sample at frame %lu\r\n", (unsigned long)(frames + 1U));

            return 1U;
        }

        adc_joystick_lcd_draw(&sample);

        if (delay > 0U)
        {
            HAL_Delay(delay);
        }

        ++frames;
    }

    return 0U;
}
