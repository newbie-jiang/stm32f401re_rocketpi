#include "driver_ws2812b.h"
#include <string.h>

#ifndef LED_NUM
#define LED_NUM 30
#endif

/* 公开帧缓存：给 encode() 用 */
uint8_t ws2812b_frame[LED_NUM][3]; /* [i][0]=G,[1]=R,[2]=B */

extern int ws2812_ll_send(void);

uint16_t ws2812b_get_led_count(void)
{
    return (uint16_t)LED_NUM;
}

int ws2812b_set_pixel(uint16_t idx, uint8_t r, uint8_t g, uint8_t b)
{
    if (idx >= (uint16_t)LED_NUM) {
        return -1;
    }
    ws2812b_frame[idx][0] = g;
    ws2812b_frame[idx][1] = r;
    ws2812b_frame[idx][2] = b;
    return 0;
}

void ws2812b_fill(uint8_t r, uint8_t g, uint8_t b)
{
    for (uint16_t i = 0; i < (uint16_t)LED_NUM; i++) {
        ws2812b_frame[i][0] = g;
        ws2812b_frame[i][1] = r;
        ws2812b_frame[i][2] = b;
    }
}

int ws2812b_show(void)
{
    return ws2812_ll_send();
}
