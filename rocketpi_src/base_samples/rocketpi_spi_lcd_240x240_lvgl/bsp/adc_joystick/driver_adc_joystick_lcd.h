/**
 * Copyright (c) 2025
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to do so, subject to the
 * following conditions:
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
 * @file      driver_adc_joystick_lcd.h
 * @brief     Joystick visualization on ST7789 LCD
 * @version   1.0.0
 * @date      2025-10-19
 */

#ifndef DRIVER_ADC_JOYSTICK_LCD_H
#define DRIVER_ADC_JOYSTICK_LCD_H

#include "driver_adc_joystick_test.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Default delay between frames used by adc_joystick_lcd_run()
 */
#define ADC_JOYSTICK_LCD_DEFAULT_DELAY_MS   20U

/**
 * @brief Initialize LCD canvas for joystick visualization
 */
void adc_joystick_lcd_init(void);

/**
 * @brief     Draw joystick position on LCD using the provided sample
 * @param[in] sample Pointer to joystick data
 */
void adc_joystick_lcd_draw(const adc_joystick_sample_t *sample);

/**
 * @brief     Continuously sample joystick and animate a dot on the LCD
 * @param[in] frame_count Number of frames to draw (0 keeps running)
 * @param[in] delay_ms    Delay between frames in milliseconds (0 uses default)
 * @return    Status code
 *            - 0 success
 *            - 1 acquisition failed
 */
uint8_t adc_joystick_lcd_run(uint32_t frame_count, uint32_t delay_ms);

#ifdef __cplusplus
}
#endif

#endif

