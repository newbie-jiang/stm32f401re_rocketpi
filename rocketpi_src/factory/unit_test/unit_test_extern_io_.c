#include "unit_test.h"

static void extern_io_init(void);


/*
LED1-LED18 顺序

PB1
PB2
PB4
PC9
PA6
PA8
PB7
PC11
PB6
PC10
PC2
PC7
PB15
PC6
PB13
PA10
PB12
PA9

*/
typedef struct {
    GPIO_TypeDef *port;
    uint16_t pin;
} led_t;

/* LED1-LED18 顺序 */
static const led_t g_leds[] = {
    {GPIOB, GPIO_PIN_1},   // LED1
    {GPIOB, GPIO_PIN_2},   // LED2
    {GPIOB, GPIO_PIN_4},   // LED3
    {GPIOC, GPIO_PIN_9},   // LED4
    {GPIOA, GPIO_PIN_6},   // LED5
    {GPIOA, GPIO_PIN_8},   // LED6
    {GPIOB, GPIO_PIN_7},   // LED7
    {GPIOC, GPIO_PIN_11},  // LED8
    {GPIOB, GPIO_PIN_6},   // LED9
    {GPIOC, GPIO_PIN_10},  // LED10
    {GPIOC, GPIO_PIN_2},   // LED11
    {GPIOC, GPIO_PIN_7},   // LED12
    {GPIOB, GPIO_PIN_15},  // LED13
    {GPIOC, GPIO_PIN_6},   // LED14
    {GPIOB, GPIO_PIN_13},  // LED15
    {GPIOA, GPIO_PIN_10},  // LED16
    {GPIOB, GPIO_PIN_12},  // LED17
    {GPIOA, GPIO_PIN_9},   // LED18
};

#define LED_COUNT (sizeof(g_leds) / sizeof(g_leds[0]))

static void led_all_off(void)
{
    for (uint32_t i = 0; i < LED_COUNT; i++) {
        HAL_GPIO_WritePin(g_leds[i].port, g_leds[i].pin, GPIO_PIN_RESET);
    }
}

static void led_set_one(uint32_t idx)
{
    led_all_off();
    if (idx < LED_COUNT) {
        HAL_GPIO_WritePin(g_leds[idx].port, g_leds[idx].pin, GPIO_PIN_SET);
    }
}



static void led_all_toggle(void)
{
    for (uint32_t i = 0; i < LED_COUNT; i++) {
        HAL_GPIO_TogglePin(g_leds[i].port, g_leds[i].pin);
    }
}

/* 来回流水一次：LED1->LED18->LED1，算“完整一次” */
static void led_chaser_pingpong_once(uint32_t step_delay_ms)
{
    int32_t idx = 0;
    int32_t dir = 1;

    /* 从 LED1 走到 LED18，再回到 LED1 结束 */
    for (;;) {
        led_set_one((uint32_t)idx);
        osDelay(step_delay_ms);

        /* 下一步 */
        idx += dir;

        /* 到最右端，反向 */
        if (idx >= (int32_t)LED_COUNT) {
            idx = (int32_t)LED_COUNT - 2;
            dir = -1;
        }

        /* 回到最左端（LED1），并且刚刚已经点亮过 idx==0 的那一步，则结束 */
        if (dir < 0 && idx < 0) {
            break;
        }
    }

    led_all_off();
}

/* 全部闪烁 count 次（一次=亮->灭） */
static void led_blink_all(uint32_t count, uint32_t on_ms, uint32_t off_ms)
{
    led_all_off();
    for (uint32_t i = 0; i < count; i++) {
        /* 全亮 */
        for (uint32_t k = 0; k < LED_COUNT; k++) {
            HAL_GPIO_WritePin(g_leds[k].port, g_leds[k].pin, GPIO_PIN_SET);
        }
        osDelay(on_ms);

        /* 全灭 */
        led_all_off();
        osDelay(off_ms);
    }
}

void extern_io_Task(void *argument)
{
    extern_io_init();
    led_all_off();

    for (;;) {
				
			  /* 模式2：全体闪烁 5 次 */
        led_blink_all(5, 500, 500);
			
        /* 模式1：流水灯跑一整轮 */
        led_chaser_pingpong_once(200);
    }
}



void extern_io_init(void)
{
	
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2|GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_9
                          |GPIO_PIN_10|GPIO_PIN_11, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6|GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_12|GPIO_PIN_13
                          |GPIO_PIN_15|GPIO_PIN_4|GPIO_PIN_6|GPIO_PIN_7, GPIO_PIN_RESET);

  /*Configure GPIO pins : PC2 PC6 PC7 PC9
                           PC10 PC11 */
  GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_9
                          |GPIO_PIN_10|GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PA6 PA8 PA9 PA10 */
  GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB1 PB2 PB12 PB13
                           PB15 PB4 PB6 PB7 */
  GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_12|GPIO_PIN_13
                          |GPIO_PIN_15|GPIO_PIN_4|GPIO_PIN_6|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}




	
