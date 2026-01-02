/**
 * @file driver_ws2812b.c
 * @brief DMA-based WS2812B strip driver implementation.
 * @author rocket
 */
#include "driver_ws2812b.h"
#include <string.h>
#include "tim.h"

#define WS2812B_MAX_LEDS       300U   /**< Maximum number of LEDs supported by this build. */
#define WS2812B_RESET_SLOTS    80U   /**< Number of low slots appended to provide reset (>50us). */
#define WS2812B_T0H_TICKS      32U   /**< Timer ticks representing a logic-0 high pulse. */
#define WS2812B_T1H_TICKS      70U   /**< Timer ticks representing a logic-1 high pulse. */
#define WS2812B_TIMER_CHANNEL  TIM_CHANNEL_4 /**< TIM channel used for PWM output. */

static TIM_HandleTypeDef *ws_tim = NULL;
static uint16_t ws_led_count = 0U;
static uint8_t ws_pixels[WS2812B_MAX_LEDS][3];
static uint16_t ws_dma_buf[WS2812B_MAX_LEDS * 24U + WS2812B_RESET_SLOTS];
static volatile bool ws_dma_busy = false;

/**
 * @brief Encode a byte (MSB first) into DMA halfwords.
 * @param value Byte to emit.
 * @param ptr Moving DMA buffer pointer.
 */
static void ws_pack_byte(uint8_t value, uint16_t **ptr)
{
    for (int bit = 7; bit >= 0; --bit) {
        **ptr = (value & (1U << bit)) ? WS2812B_T1H_TICKS : WS2812B_T0H_TICKS;
        (*ptr)++;
    }
}

/**
 * @brief Populate the DMA buffer from the staging pixel array.
 * @return Number of halfwords populated.
 */
static uint16_t ws_build_buffer(void)
{
    uint16_t *p = ws_dma_buf;
    for (uint16_t i = 0; i < ws_led_count; ++i) {
        uint8_t red = ws_pixels[i][0];
        uint8_t green = ws_pixels[i][1];
        uint8_t blue = ws_pixels[i][2];
        ws_pack_byte(green, &p);
        ws_pack_byte(red, &p);
        ws_pack_byte(blue, &p);
    }

    for (uint16_t i = 0; i < WS2812B_RESET_SLOTS; ++i) {
        *p++ = 0U;
    }

    return (uint16_t)(p - ws_dma_buf);
}

/**
 * @brief Initialize driver resources.
 */
bool ws2812b_init(uint16_t led_count)
{
    if ((led_count == 0U) || (led_count > WS2812B_MAX_LEDS)) {
        return false;
    }

    ws_tim = &htim3;
    ws_led_count = led_count;
    ws_dma_busy = false;
    memset(ws_pixels, 0, sizeof(ws_pixels));

    return true;
}

/**
 * @brief Set a single pixel in the staging buffer.
 */
bool ws2812b_set_pixel(uint16_t index, uint8_t red, uint8_t green, uint8_t blue)
{
    if ((index >= ws_led_count) || (ws_tim == NULL)) {
        return false;
    }

    ws_pixels[index][0] = red;
    ws_pixels[index][1] = green;
    ws_pixels[index][2] = blue;
    return true;
}

/**
 * @brief Fill entire staging buffer with a single color.
 */
void ws2812b_fill(uint8_t red, uint8_t green, uint8_t blue)
{
    for (uint16_t i = 0; i < ws_led_count; ++i) {
        (void)ws2812b_set_pixel(i, red, green, blue);
    }
}

/**
 * @brief Commit current pixel buffer via DMA.
 */
bool ws2812b_refresh(void)
{
    if ((ws_tim == NULL) || (ws_led_count == 0U) || ws_dma_busy) {
        return false;
    }

    uint16_t payload = ws_build_buffer();
    ws_dma_busy = true;

    if (HAL_TIM_PWM_Start_DMA(ws_tim,
                              WS2812B_TIMER_CHANNEL,
                              (uint32_t *)ws_dma_buf,
                              payload) != HAL_OK) {
        ws_dma_busy = false;
        return false;
    }

    return true;
}

/**
 * @brief Check DMA busy state.
 */
bool ws2812b_is_busy(void)
{
    return ws_dma_busy;
}

/**
 * @brief Return configured LED count.
 */
uint16_t ws2812b_get_led_count(void)
{
    return ws_led_count;
}

/**
 * @brief HAL callback invoked at DMA completion.
 */
void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim)
{
    if ((ws_tim != NULL) && (htim == ws_tim)) {
        (void)HAL_TIM_PWM_Stop_DMA(ws_tim, WS2812B_TIMER_CHANNEL);
        __HAL_TIM_SET_COMPARE(ws_tim, WS2812B_TIMER_CHANNEL, 0U);
        ws_dma_busy = false;
    }
}
