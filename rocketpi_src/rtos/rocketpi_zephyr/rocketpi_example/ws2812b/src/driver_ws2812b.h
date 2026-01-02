#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

uint16_t ws2812b_get_led_count(void);

/* 颜色顺序：传入 r,g,b（0-255），内部会按 GRB 编码发送 */
int ws2812b_set_pixel(uint16_t idx, uint8_t r, uint8_t g, uint8_t b);
void ws2812b_fill(uint8_t r, uint8_t g, uint8_t b);

/* 推送到灯带：阻塞直到 DMA 发送完（内部会等待） */
int ws2812b_show(void);

#ifdef __cplusplus
}
#endif
