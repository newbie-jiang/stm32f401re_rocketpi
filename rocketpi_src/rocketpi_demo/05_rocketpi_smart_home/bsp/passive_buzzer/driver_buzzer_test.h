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
 * @file      driver_buzzer_test.h
 * @brief     TIM3 driven active buzzer test helper
 * @version   1.0.0
 * @date      2025-10-19
 */

#ifndef DRIVER_BUZZER_TEST_H
#define DRIVER_BUZZER_TEST_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Default PWM tone frequency used when an explicit frequency is not provided (Hz)
 */
#define BUZZER_TEST_DEFAULT_FREQUENCY_HZ          2000U

/**
 * @brief Default PWM duty cycle (percentage) applied to the buzzer output
 */
#define BUZZER_TEST_DEFAULT_DUTY_PERCENT          50U

/**
 * @brief Default tone duration used by buzzer_test_beep() when duration_ms is zero (milliseconds)
 */
#define BUZZER_TEST_DEFAULT_DURATION_MS           200U

/**
 * @brief Default gap inserted between notes in buzzer_test_play_sequence() when gap_ms is zero (milliseconds)
 */
#define BUZZER_TEST_DEFAULT_NOTE_GAP_MS           20U

/**
 * @brief Maximum duty cycle accepted by the helper (percentage)
 */
#define BUZZER_TEST_MAX_DUTY_PERCENT              100U

/**
 * @brief Note descriptor used by buzzer_test_play_sequence()
 *
 * A frequency of 0 produces a rest (silence) for the indicated duration.
 * A duty cycle of 0 adopts BUZZER_TEST_DEFAULT_DUTY_PERCENT.
 * A duration of 0 adopts BUZZER_TEST_DEFAULT_DURATION_MS.
 */
typedef struct
{
    uint32_t frequency_hz; /**< Tone frequency in Hertz (0 selects silence) */
    uint8_t duty_percent;  /**< PWM duty cycle percentage (0 selects the default) */
    uint32_t duration_ms;  /**< Tone (or rest) duration in milliseconds (0 selects the default) */
} buzzer_test_note_t;

/**
 * @brief     Configure TIM3 CH3 and start driving the buzzer
 * @param[in] frequency_hz Desired output frequency in Hertz (0 selects BUZZER_TEST_DEFAULT_FREQUENCY_HZ)
 * @param[in] duty_percent PWM duty cycle (0..100). Values above 100 are clamped.
 * @return    Status code
 *            - 0 success
 *            - 1 configuration or HAL error
 */
uint8_t buzzer_test_start(uint32_t frequency_hz, uint8_t duty_percent);

/**
 * @brief     Stop the PWM output driving the buzzer
 * @return    Status code
 *            - 0 success
 *            - 1 HAL error
 */
uint8_t buzzer_test_stop(void);

/**
 * @brief     Play a single tone on the buzzer
 * @param[in] frequency_hz Tone frequency in Hertz (0 selects the default)
 * @param[in] duty_percent PWM duty cycle percentage (0 selects the default)
 * @param[in] duration_ms  Tone duration in milliseconds (0 selects the default)
 * @return    Status code
 *            - 0 success
 *            - 1 configuration or HAL error
 */
uint8_t buzzer_test_beep(uint32_t frequency_hz, uint8_t duty_percent, uint32_t duration_ms);

/**
 * @brief     Play a sequence of tones and rests on the buzzer
 * @param[in] notes    Pointer to an array of @ref buzzer_test_note_t descriptors
 * @param[in] length   Number of entries in the notes array
 * @param[in] gap_ms   Pause added between successive notes (0 selects the default)
 * @return    Status code
 *            - 0 success
 *            - 1 invalid parameters
 *            - 2 playback aborted due to HAL error
 */
uint8_t buzzer_test_play_sequence(const buzzer_test_note_t *notes, uint32_t length, uint32_t gap_ms);

#ifdef __cplusplus
}
#endif

#endif
