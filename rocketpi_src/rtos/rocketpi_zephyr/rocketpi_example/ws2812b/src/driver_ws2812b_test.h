#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void ws2812b_test_blink(uint8_t red, uint8_t green, uint8_t blue,
                        uint32_t on_time_ms, uint32_t off_time_ms, uint32_t duration_ms);

void ws2812b_test_chase(uint8_t red, uint8_t green, uint8_t blue,
                        uint32_t step_delay_ms, uint32_t duration_ms);

void ws2812b_test_rainbow(uint32_t step_delay_ms, uint32_t duration_ms);

void ws2812b_test_breathe(uint8_t red, uint8_t green, uint8_t blue,
                          uint32_t step_delay_ms, uint32_t duration_ms);

void ws2812b_test_theater_chase(uint8_t red, uint8_t green, uint8_t blue,
                                uint32_t step_delay_ms, uint32_t duration_ms);

void ws2812b_test_gradient_wipe(uint8_t start_r, uint8_t start_g, uint8_t start_b,
                                uint8_t end_r, uint8_t end_g, uint8_t end_b,
                                uint32_t step_delay_ms, uint32_t duration_ms);

#ifdef __cplusplus
}
#endif
