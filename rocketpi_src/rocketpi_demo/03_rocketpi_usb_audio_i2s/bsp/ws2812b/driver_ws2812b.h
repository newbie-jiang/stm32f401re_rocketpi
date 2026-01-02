#pragma once

#include <stdbool.h>
#include <stdint.h>

/**
 * @file driver_ws2812b.h
 * @brief Minimal WS2812B strip driver API.
 * @author rocket
 * @copyright Copyright (c) 2025 rocket. Authorized use only.
 */

/**
 * @brief Initialize the driver and internal buffers.
 * @param led_count Total number of LEDs on the strip (1-60 supported by this build).
 * @return true when the driver is ready, false on invalid LED count or unavailable resources.
 */
bool ws2812b_init(uint16_t led_count);

/**
 * @brief Update one pixel in the staging buffer.
 * @param index Zero-based LED index to update.
 * @param red Red intensity (0-255).
 * @param green Green intensity (0-255).
 * @param blue Blue intensity (0-255).
 * @return true on success, false if index is out of range or driver not initialized.
 */
bool ws2812b_set_pixel(uint16_t index, uint8_t red, uint8_t green, uint8_t blue);

/**
 * @brief Fill the staging buffer with a single RGB color.
 * @param red Red intensity (0-255).
 * @param green Green intensity (0-255).
 * @param blue Blue intensity (0-255).
 */
void ws2812b_fill(uint8_t red, uint8_t green, uint8_t blue);

/**
 * @brief Kick off a DMA transfer that pushes the staging buffer to the strip.
 * @return true if DMA started, false if the driver is busy or uninitialized.
 */
bool ws2812b_refresh(void);

/**
 * @brief Query whether a DMA transfer is still in progress.
 * @return true while DMA is active, otherwise false.
 */
bool ws2812b_is_busy(void);

/**
 * @brief Retrieve the LED count configured at init time.
 * @return Number of LEDs currently supported by the driver (0 if uninitialized).
 */
uint16_t ws2812b_get_led_count(void);
