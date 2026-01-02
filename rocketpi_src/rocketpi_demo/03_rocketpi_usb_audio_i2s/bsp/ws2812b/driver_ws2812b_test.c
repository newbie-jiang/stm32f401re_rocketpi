/**
 * @file driver_ws2812b_test.c
 * @brief Predefined visual patterns for WS2812B demo and validation.
 * @author rocket
 * @copyright Copyright (c) 2025 rocket. Authorized use only.
 */

#include "driver_ws2812b_test.h"

#include "main.h"

/**
 * @brief Push staged data to the strip and wait until DMA finishes.
 */
static void ws_test_commit_and_wait(void)
{
    while (!ws2812b_refresh()) {
        while (ws2812b_is_busy()) {
        }
    }
    while (ws2812b_is_busy()) {
    }
}

/**
 * @brief Determine whether a pattern should exit due to runtime limit.
 * @param start_tick HAL tick captured when the pattern started.
 * @param duration_ms Allowed run time in milliseconds (0 => infinite).
 * @return true if duration has elapsed, otherwise false.
 */
static bool ws_test_should_stop(uint32_t start_tick, uint32_t duration_ms)
{
    if (duration_ms == 0U) {
        return false;
    }
    return (HAL_GetTick() - start_tick) >= duration_ms;
}

/**
 * @brief Repeatedly toggle the entire strip between on/off states.
 * @param red Red component for the "on" state (0-255).
 * @param green Green component for the "on" state (0-255).
 * @param blue Blue component for the "on" state (0-255).
 * @param on_time_ms Milliseconds the color stays lit per cycle.
 * @param off_time_ms Milliseconds the strip stays dark per cycle.
 * @param duration_ms Total runtime (0 = infinite loop).
 */
void ws2812b_test_blink(uint8_t red, uint8_t green, uint8_t blue,
                        uint32_t on_time_ms, uint32_t off_time_ms, uint32_t duration_ms)
{
    const uint32_t start_tick = HAL_GetTick();
    while (!ws_test_should_stop(start_tick, duration_ms)) {
        ws2812b_fill(red, green, blue);
        ws_test_commit_and_wait();
        HAL_Delay(on_time_ms);
        if (ws_test_should_stop(start_tick, duration_ms)) {
            break;
        }

        ws2812b_fill(0U, 0U, 0U);
        ws_test_commit_and_wait();
        HAL_Delay(off_time_ms);
    }
}

/**
 * @brief Move a single illuminated pixel along the strip.
 * @param red Red component for the moving pixel.
 * @param green Green component for the moving pixel.
 * @param blue Blue component for the moving pixel.
 * @param step_delay_ms Delay between each pixel hop.
 * @param duration_ms Total runtime (0 = infinite loop).
 */
void ws2812b_test_chase(uint8_t red, uint8_t green, uint8_t blue,
                        uint32_t step_delay_ms, uint32_t duration_ms)
{
    const uint16_t count = ws2812b_get_led_count();
    const uint32_t start_tick = HAL_GetTick();
    while (!ws_test_should_stop(start_tick, duration_ms)) {
        for (uint16_t idx = 0; idx < count; ++idx) {
            ws2812b_fill(0U, 0U, 0U);
            (void)ws2812b_set_pixel(idx, red, green, blue);
            ws_test_commit_and_wait();
            HAL_Delay(step_delay_ms);
            if (ws_test_should_stop(start_tick, duration_ms)) {
                break;
            }
        }
    }
}

/**
 * @brief Convert a wheel position (0-255) to RGB components.
 * @param pos Color wheel index.
 * @param r Output red component pointer.
 * @param g Output green component pointer.
 * @param b Output blue component pointer.
 */
static void ws_test_color_wheel(uint8_t pos, uint8_t *r, uint8_t *g, uint8_t *b)
{
    if (pos < 85U) {
        *r = (uint8_t)(pos * 3U);
        *g = (uint8_t)(255U - pos * 3U);
        *b = 0U;
    } else if (pos < 170U) {
        pos -= 85U;
        *r = (uint8_t)(255U - pos * 3U);
        *g = 0U;
        *b = (uint8_t)(pos * 3U);
    } else {
        pos -= 170U;
        *r = 0U;
        *g = (uint8_t)(pos * 3U);
        *b = (uint8_t)(255U - pos * 3U);
    }
}

/**
 * @brief Cycle a rainbow gradient across the strip.
 * @param step_delay_ms Delay between hue shifts.
 * @param duration_ms Total runtime (0 = infinite loop).
 */
void ws2812b_test_rainbow(uint32_t step_delay_ms, uint32_t duration_ms)
{
    const uint16_t count = ws2812b_get_led_count();
    if (count == 0U) {
        return;
    }

    const uint32_t start_tick = HAL_GetTick();
    while (!ws_test_should_stop(start_tick, duration_ms)) {
        for (uint16_t offset = 0; offset < 255U; ++offset) {
            for (uint16_t led = 0; led < count; ++led) {
                uint8_t r, g, b;
                ws_test_color_wheel((uint8_t)((led * 256U / count + offset) & 0xFFU), &r, &g, &b);
                (void)ws2812b_set_pixel(led, r, g, b);
            }
            ws_test_commit_and_wait();
            HAL_Delay(step_delay_ms);
            if (ws_test_should_stop(start_tick, duration_ms)) {
                return;
            }
        }
    }
}

/**
 * @brief Fade a single color up and down to mimic breathing.
 * @param red Base color red component.
 * @param green Base color green component.
 * @param blue Base color blue component.
 * @param step_delay_ms Delay between brightness level changes.
 * @param duration_ms Total runtime (0 = infinite loop).
 */
void ws2812b_test_breathe(uint8_t red, uint8_t green, uint8_t blue,
                          uint32_t step_delay_ms, uint32_t duration_ms)
{
    const uint32_t start_tick = HAL_GetTick();
    while (!ws_test_should_stop(start_tick, duration_ms)) {
        for (uint16_t level = 0; level <= 255U; ++level) {
            ws2812b_fill((uint8_t)((red * level) / 255U),
                         (uint8_t)((green * level) / 255U),
                         (uint8_t)((blue * level) / 255U));
            ws_test_commit_and_wait();
            HAL_Delay(step_delay_ms);
            if (ws_test_should_stop(start_tick, duration_ms)) {
                return;
            }
        }
        for (int level = 255; level >= 0; --level) {
            ws2812b_fill((uint8_t)((red * level) / 255U),
                         (uint8_t)((green * level) / 255U),
                         (uint8_t)((blue * level) / 255U));
            ws_test_commit_and_wait();
            HAL_Delay(step_delay_ms);
            if (ws_test_should_stop(start_tick, duration_ms)) {
                return;
            }
        }
    }
}

/**
 * @brief Theater chase effect with every third pixel lit.
 * @param red Red component for the lit pixels.
 * @param green Green component for the lit pixels.
 * @param blue Blue component for the lit pixels.
 * @param step_delay_ms Delay between offset shifts.
 * @param duration_ms Total runtime (0 = infinite loop).
 */
void ws2812b_test_theater_chase(uint8_t red, uint8_t green, uint8_t blue,
                                uint32_t step_delay_ms, uint32_t duration_ms)
{
    const uint16_t count = ws2812b_get_led_count();
    const uint8_t spacing = 3U;
    const uint32_t start_tick = HAL_GetTick();

    while (!ws_test_should_stop(start_tick, duration_ms)) {
        for (uint8_t offset = 0; offset < spacing; ++offset) {
            ws2812b_fill(0U, 0U, 0U);
            for (uint16_t led = offset; led < count; led += spacing) {
                (void)ws2812b_set_pixel(led, red, green, blue);
            }
            ws_test_commit_and_wait();
            HAL_Delay(step_delay_ms);
            if (ws_test_should_stop(start_tick, duration_ms)) {
                return;
            }
        }
    }
}

/**
 * @brief Sweep the strip while blending from a start to an end color.
 * @param start_r Starting color red channel (0-255).
 * @param start_g Starting color green channel (0-255).
 * @param start_b Starting color blue channel (0-255).
 * @param end_r Ending color red channel (0-255).
 * @param end_g Ending color green channel (0-255).
 * @param end_b Ending color blue channel (0-255).
 * @param step_delay_ms Delay between wipe steps.
 * @param duration_ms Total runtime (0 = infinite loop).
 */
void ws2812b_test_gradient_wipe(uint8_t start_r, uint8_t start_g, uint8_t start_b,
                                uint8_t end_r, uint8_t end_g, uint8_t end_b,
                                uint32_t step_delay_ms, uint32_t duration_ms)
{
    const uint16_t count = ws2812b_get_led_count();
    if (count == 0U) {
        return;
    }

    const uint32_t start_tick = HAL_GetTick();
    while (!ws_test_should_stop(start_tick, duration_ms)) {
        for (uint16_t head = 0; head < count; ++head) {
            for (uint16_t led = 0; led <= head; ++led) {
                float ratio = (head == 0U) ? 1.0f : (float)led / (float)head;
                uint8_t r = (uint8_t)(start_r + (end_r - start_r) * ratio);
                uint8_t g = (uint8_t)(start_g + (end_g - start_g) * ratio);
                uint8_t b = (uint8_t)(start_b + (end_b - start_b) * ratio);
                (void)ws2812b_set_pixel(led, r, g, b);
            }
            ws_test_commit_and_wait();
            HAL_Delay(step_delay_ms);
            if (ws_test_should_stop(start_tick, duration_ms)) {
                return;
            }
        }
    }
}
