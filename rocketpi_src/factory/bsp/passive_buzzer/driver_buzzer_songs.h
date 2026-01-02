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
 * @file      driver_buzzer_songs.h
 * @brief     Sample melodies for the TIM3/PB0 buzzer helper
 * @version   1.0.0
 * @date      2025-10-19
 *
 * Include this header in your test file (e.g. main.c) and feed the provided
 * note arrays into buzzer_test_play_sequence(). Feel free to add or modify
 * melodies following the same structure.
 */

#ifndef DRIVER_BUZZER_SONGS_H
#define DRIVER_BUZZER_SONGS_H

#include "driver_buzzer_test.h"

/* Common note frequencies (equal temperament, rounded to the nearest Hertz) */
#define BUZZER_NOTE_REST   0U
#define BUZZER_NOTE_C4     262U
#define BUZZER_NOTE_D4     294U
#define BUZZER_NOTE_E4     330U
#define BUZZER_NOTE_F4     349U
#define BUZZER_NOTE_G4     392U
#define BUZZER_NOTE_A4     440U
#define BUZZER_NOTE_B4     494U
#define BUZZER_NOTE_C5     523U
#define BUZZER_NOTE_D5     587U
#define BUZZER_NOTE_E5     659U
#define BUZZER_NOTE_F5     698U
#define BUZZER_NOTE_G5     784U

/* Straightforward ascending scale (do re mi...) */
static const buzzer_test_note_t buzzer_song_scale_demo[] =
{
    { BUZZER_NOTE_C4, 0U, 200U },
    { BUZZER_NOTE_D4, 0U, 200U },
    { BUZZER_NOTE_E4, 0U, 200U },
    { BUZZER_NOTE_F4, 0U, 200U },
    { BUZZER_NOTE_G4, 0U, 200U },
    { BUZZER_NOTE_A4, 0U, 200U },
    { BUZZER_NOTE_B4, 0U, 200U },
    { BUZZER_NOTE_C5, 0U, 400U },
};
#define BUZZER_SONG_SCALE_DEMO_LENGTH   ((uint32_t)(sizeof(buzzer_song_scale_demo) / sizeof(buzzer_song_scale_demo[0])))

/* Opening phrase of "Twinkle Twinkle Little Star" */
static const buzzer_test_note_t buzzer_song_twinkle_intro[] =
{
    { BUZZER_NOTE_C4, 0U, 300U },
    { BUZZER_NOTE_C4, 0U, 300U },
    { BUZZER_NOTE_G4, 0U, 300U },
    { BUZZER_NOTE_G4, 0U, 300U },
    { BUZZER_NOTE_A4, 0U, 300U },
    { BUZZER_NOTE_A4, 0U, 300U },
    { BUZZER_NOTE_G4, 0U, 600U },
    { BUZZER_NOTE_F4, 0U, 300U },
    { BUZZER_NOTE_F4, 0U, 300U },
    { BUZZER_NOTE_E4, 0U, 300U },
    { BUZZER_NOTE_E4, 0U, 300U },
    { BUZZER_NOTE_D4, 0U, 300U },
    { BUZZER_NOTE_D4, 0U, 300U },
    { BUZZER_NOTE_C4, 0U, 600U },
};
#define BUZZER_SONG_TWINKLE_INTRO_LENGTH   ((uint32_t)(sizeof(buzzer_song_twinkle_intro) / sizeof(buzzer_song_twinkle_intro[0])))



/* Extended excerpt of Beethoven's "Ode to Joy" */
#ifndef BUZZER_SONG_ODE_TO_JOY_TEMPO_COEFF_MS
#define BUZZER_SONG_ODE_TO_JOY_TEMPO_COEFF_MS   8U  /* Lower values speed up playback */
#endif

#define ODE_TO_JOY_DURATION(units)              ((units) * BUZZER_SONG_ODE_TO_JOY_TEMPO_COEFF_MS)

static const buzzer_test_note_t buzzer_song_ode_to_joy[] =
{
    { BUZZER_NOTE_E4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_E4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_F4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_G4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_G4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_F4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_E4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_D4, 0U, ODE_TO_JOY_DURATION(36U) },
    { BUZZER_NOTE_C4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_C4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_D4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_E4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_E4, 0U, ODE_TO_JOY_DURATION(22U) },
    { BUZZER_NOTE_D4, 0U, ODE_TO_JOY_DURATION(22U) },
    { BUZZER_NOTE_D4, 0U, ODE_TO_JOY_DURATION(42U) },
    { BUZZER_NOTE_E4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_E4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_F4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_G4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_G4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_F4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_E4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_D4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_C4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_C4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_D4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_E4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_D4, 0U, ODE_TO_JOY_DURATION(22U) },
    { BUZZER_NOTE_C4, 0U, ODE_TO_JOY_DURATION(22U) },
    { BUZZER_NOTE_C4, 0U, ODE_TO_JOY_DURATION(42U) },
    { BUZZER_NOTE_D4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_D4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_E4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_C4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_D4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_E4, 0U, ODE_TO_JOY_DURATION(36U) },
    { BUZZER_NOTE_F4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_E4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_D4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_C4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_D4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_G4, 0U, ODE_TO_JOY_DURATION(40U) },
    { BUZZER_NOTE_E4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_E4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_F4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_G4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_G4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_F4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_E4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_D4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_C4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_C4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_D4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_E4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_D4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_C4, 0U, ODE_TO_JOY_DURATION(25U) },
    { BUZZER_NOTE_C4, 0U, ODE_TO_JOY_DURATION(48U) },
};
#define BUZZER_SONG_ODE_TO_JOY_LENGTH   ((uint32_t)(sizeof(buzzer_song_ode_to_joy) / sizeof(buzzer_song_ode_to_joy[0])))

#undef ODE_TO_JOY_DURATION


/* Simple notification chime (two tones with a rest) */
static const buzzer_test_note_t buzzer_song_notify_chime[] =
{
    { BUZZER_NOTE_G5, 60U, 120U },
    { BUZZER_NOTE_REST, 0U, 80U },
    { BUZZER_NOTE_C5, 60U, 200U },
};
#define BUZZER_SONG_NOTIFY_CHIME_LENGTH   ((uint32_t)(sizeof(buzzer_song_notify_chime) / sizeof(buzzer_song_notify_chime[0])))

#endif /* DRIVER_BUZZER_SONGS_H */
