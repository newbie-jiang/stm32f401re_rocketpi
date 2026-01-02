#include <zephyr/kernel.h>
#include <zephyr/irq.h>
#include <zephyr/sys/printk.h>

#include <stdbool.h>
#include <stdint.h>

#include <stm32f4xx.h>
#include <stm32f4xx_ll_bus.h>
#include <stm32f4xx_ll_dma.h>
#include <stm32f4xx_ll_gpio.h>
#include <stm32f4xx_ll_tim.h>

#include "driver_ws2812b.h"
#include "driver_ws2812b_test.h"

#define LED_NUM            30
#define BITS_PER_LED       24
#define WS2812_BIT_HZ      800000UL          /* 800kHz */
#define T0H_NS             350UL
#define T1H_NS             700UL
#define RESET_US           80UL              /* > 50us，取 80us 更稳 */

/* 来自 driver_ws2812b.c：帧缓存（GRB 顺序） */
extern uint8_t ws2812b_frame[LED_NUM][3]; /* [i][0]=G,[1]=R,[2]=B */

/* 每 bit 一个 CCR 值；末尾加 reset 槽位（占空比 0） */
static uint16_t pwm_buf[LED_NUM * BITS_PER_LED + (RESET_US * WS2812_BIT_HZ / 1000000UL) + 16];
static size_t pwm_len;

static K_SEM_DEFINE(dma_done, 0, 1);

static void ws2812_gpio_init_pb1_tim1_ch3n(void)
{
    /* PB1 -> TIM1_CH3N (AF1) */
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);

    LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_1, LL_GPIO_MODE_ALTERNATE);
    LL_GPIO_SetPinSpeed(GPIOB, LL_GPIO_PIN_1, LL_GPIO_SPEED_FREQ_VERY_HIGH);
    LL_GPIO_SetPinOutputType(GPIOB, LL_GPIO_PIN_1, LL_GPIO_OUTPUT_PUSHPULL);
    LL_GPIO_SetPinPull(GPIOB, LL_GPIO_PIN_1, LL_GPIO_PULL_NO);

    /* PB1 属于 0..7 */
    LL_GPIO_SetAFPin_0_7(GPIOB, LL_GPIO_PIN_1, LL_GPIO_AF_1);
}

static uint32_t tim1_clk_hz(void)
{
    /* Zephyr 下 SystemCoreClock 通常等于 HCLK */
    uint32_t hclk = SystemCoreClock;

    /* APB2 分频（PPRE2） */
    uint32_t ppre2 = (RCC->CFGR & RCC_CFGR_PPRE2) >> RCC_CFGR_PPRE2_Pos;
    uint32_t apb2_div = 1U;

    /* 参考 RM: 0xx=1, 100=2, 101=4, 110=8, 111=16 */
    switch (ppre2) {
    case 0b100: apb2_div = 2U;  break;
    case 0b101: apb2_div = 4U;  break;
    case 0b110: apb2_div = 8U;  break;
    case 0b111: apb2_div = 16U; break;
    default:    apb2_div = 1U;  break;
    }

    uint32_t pclk2 = hclk / apb2_div;

    /* F4: APB prescaler != 1 时，TIMxCLK = 2 * PCLK */
    return (apb2_div == 1U) ? pclk2 : (pclk2 * 2U);
}

static uint16_t ns_to_ticks(uint32_t timclk, uint32_t ns)
{
    uint64_t t = (uint64_t)timclk * (uint64_t)ns;
    t = (t + 999999999ULL) / 1000000000ULL; /* 向上取整 */
    if (t > 0xFFFFu) {
        t = 0xFFFFu;
    }
    return (uint16_t)t;
}

/* 用 driver_ws2812b.c 的 ws2812b_frame[] 生成 PWM CCR 序列 */
static void ws2812_encode(uint16_t t0h, uint16_t t1h)
{
    size_t k = 0;

    for (int i = 0; i < LED_NUM; i++) {
        uint32_t grb = ((uint32_t)ws2812b_frame[i][0] << 16) |
                       ((uint32_t)ws2812b_frame[i][1] << 8)  |
                       ((uint32_t)ws2812b_frame[i][2] << 0);

        for (int bit = 23; bit >= 0; bit--) {
            pwm_buf[k++] = (grb & (1UL << bit)) ? t1h : t0h;
        }
    }

    /* reset: 发送若干个“0占空比周期”，等效拉低 > 50us */
    uint32_t reset_slots = (RESET_US * WS2812_BIT_HZ) / 1000000UL;
    for (uint32_t i = 0; i < reset_slots + 8U; i++) {
        pwm_buf[k++] = 0;
    }

    pwm_len = k;
}

static void dma2_stream5_isr(const void *arg)
{
    ARG_UNUSED(arg);

    if (LL_DMA_IsActiveFlag_TC5(DMA2)) {
        LL_DMA_ClearFlag_TC5(DMA2);

        /* 停 DMA，关 TIM1 的更新 DMA 请求；保持 CCR3=0 让引脚继续为低 */
        LL_TIM_DisableDMAReq_UPDATE(TIM1);
        LL_DMA_DisableStream(DMA2, LL_DMA_STREAM_5);
        LL_TIM_OC_SetCompareCH3(TIM1, 0);

        k_sem_give(&dma_done);
    }

    if (LL_DMA_IsActiveFlag_TE5(DMA2)) {
        LL_DMA_ClearFlag_TE5(DMA2);

        LL_TIM_DisableDMAReq_UPDATE(TIM1);
        LL_DMA_DisableStream(DMA2, LL_DMA_STREAM_5);
        LL_TIM_OC_SetCompareCH3(TIM1, 0);

        k_sem_give(&dma_done);
    }
}

static int ws2812_ll_init(void)
{
    ws2812_gpio_init_pb1_tim1_ch3n();

    /* clocks */
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA2);
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM1);

    /* TIM1: 800kHz PWM base */
    uint32_t timclk = tim1_clk_hz();
    uint32_t period_ticks = timclk / WS2812_BIT_HZ;
    if (period_ticks < 10U || period_ticks > 0xFFFFu) {
        printk("bad period_ticks=%u (timclk=%u)\n", period_ticks, timclk);
        return -EINVAL;
    }

    LL_TIM_DisableCounter(TIM1);

    LL_TIM_SetPrescaler(TIM1, 0);
    LL_TIM_SetAutoReload(TIM1, (uint16_t)(period_ticks - 1U));
    LL_TIM_SetCounterMode(TIM1, LL_TIM_COUNTERMODE_UP);
    LL_TIM_EnableARRPreload(TIM1);

    /* CH3 PWM1，DMA 更新 CCR3；输出使用 CH3N (PB1) */
    LL_TIM_OC_SetMode(TIM1, LL_TIM_CHANNEL_CH3, LL_TIM_OCMODE_PWM1);
    LL_TIM_OC_EnablePreload(TIM1, LL_TIM_CHANNEL_CH3);
    LL_TIM_OC_SetCompareCH3(TIM1, 0);

    /* 只开 CH3N，别开 CH3 */
    LL_TIM_CC_DisableChannel(TIM1, LL_TIM_CHANNEL_CH3);
    LL_TIM_CC_EnableChannel(TIM1, LL_TIM_CHANNEL_CH3N);

    /* 高级定时器需要 MOE */
    LL_TIM_EnableAllOutputs(TIM1);

    /* 刷新预装载 */
    LL_TIM_GenerateEvent_UPDATE(TIM1);

    /* DMA2 Stream5 / Channel6 = TIM1_UP -> 写 TIM1->CCR3 */
    LL_DMA_DisableStream(DMA2, LL_DMA_STREAM_5);
    while (LL_DMA_IsEnabledStream(DMA2, LL_DMA_STREAM_5)) {}

    LL_DMA_SetChannelSelection(DMA2, LL_DMA_STREAM_5, LL_DMA_CHANNEL_6);
    LL_DMA_SetDataTransferDirection(DMA2, LL_DMA_STREAM_5, LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
    LL_DMA_SetStreamPriorityLevel(DMA2, LL_DMA_STREAM_5, LL_DMA_PRIORITY_VERYHIGH);
    LL_DMA_SetMode(DMA2, LL_DMA_STREAM_5, LL_DMA_MODE_NORMAL);
    LL_DMA_SetPeriphIncMode(DMA2, LL_DMA_STREAM_5, LL_DMA_PERIPH_NOINCREMENT);
    LL_DMA_SetMemoryIncMode(DMA2, LL_DMA_STREAM_5, LL_DMA_MEMORY_INCREMENT);
    LL_DMA_SetPeriphSize(DMA2, LL_DMA_STREAM_5, LL_DMA_PDATAALIGN_HALFWORD);
    LL_DMA_SetMemorySize(DMA2, LL_DMA_STREAM_5, LL_DMA_MDATAALIGN_HALFWORD);

    LL_DMA_SetPeriphAddress(DMA2, LL_DMA_STREAM_5, (uint32_t)&TIM1->CCR3);

    /* DMA IRQ */
    IRQ_CONNECT(DMA2_Stream5_IRQn, 0, dma2_stream5_isr, NULL, 0);
    irq_enable(DMA2_Stream5_IRQn);

    printk("WS2812 LL init OK. TIM1 clk=%u Hz, period_ticks=%u\n", timclk, period_ticks);
    return 0;
}

/* 给 driver_ws2812b.c 调用：阻塞发送一帧 */
int ws2812_ll_send(void)
{
    uint32_t timclk = tim1_clk_hz();
    uint16_t t0h = ns_to_ticks(timclk, T0H_NS);
    uint16_t t1h = ns_to_ticks(timclk, T1H_NS);

    ws2812_encode(t0h, t1h);

    /* 配 DMA */
    LL_DMA_DisableStream(DMA2, LL_DMA_STREAM_5);
    while (LL_DMA_IsEnabledStream(DMA2, LL_DMA_STREAM_5)) {}

    /* 清标志 */
    LL_DMA_ClearFlag_TC5(DMA2);
    LL_DMA_ClearFlag_TE5(DMA2);
    LL_DMA_ClearFlag_HT5(DMA2);
    LL_DMA_ClearFlag_DME5(DMA2);
    LL_DMA_ClearFlag_FE5(DMA2);

    LL_DMA_SetMemoryAddress(DMA2, LL_DMA_STREAM_5, (uint32_t)pwm_buf);
    LL_DMA_SetDataLength(DMA2, LL_DMA_STREAM_5, (uint32_t)pwm_len);

    LL_DMA_EnableIT_TC(DMA2, LL_DMA_STREAM_5);
    LL_DMA_EnableIT_TE(DMA2, LL_DMA_STREAM_5);

    k_sem_reset(&dma_done);

    /* update 事件触发 DMA */
    LL_DMA_EnableStream(DMA2, LL_DMA_STREAM_5);
    LL_TIM_EnableDMAReq_UPDATE(TIM1);

    /* 启动计数器 */
    LL_TIM_SetCounter(TIM1, 0);
    LL_TIM_EnableCounter(TIM1);

    if (k_sem_take(&dma_done, K_MSEC(200)) != 0) {
        printk("ws2812 send timeout\n");
        LL_TIM_DisableDMAReq_UPDATE(TIM1);
        LL_DMA_DisableStream(DMA2, LL_DMA_STREAM_5);
        LL_TIM_OC_SetCompareCH3(TIM1, 0);
        return -ETIMEDOUT;
    }

    return 0;
}


int main(void)
{
    int ret = ws2812_ll_init();
    if (ret) {
        printk("ws2812 init failed: %d\n", ret);
        return 0;
    }

    /* 先清屏 */
    ws2812b_fill(0, 0, 0);
    (void)ws2812b_show();
    k_msleep(300);

    while (1) {
        /* 1) 闪烁：白色，亮200ms灭200ms，跑3秒 */
        ws2812b_test_blink(255, 255, 255, 200, 200, 3000);
        k_msleep(300);

        /* 2) 追逐：红色跑马，步进50ms，跑5秒 */
        ws2812b_test_chase(255, 0, 0, 50, 5000);
        k_msleep(300);

        /* 3) 彩虹：每步20ms，跑6秒 */
        ws2812b_test_rainbow(20, 6000);
        k_msleep(300);

        /* 4) 呼吸：蓝色，步进5ms，跑6秒 */
        ws2812b_test_breathe(0, 0, 255, 5, 6000);
        k_msleep(300);

        /* 5) 剧院追逐：绿色，步进80ms，跑6秒 */
        ws2812b_test_theater_chase(0, 255, 0, 80, 6000);
        k_msleep(300);

        /* 6) 渐变擦除：从紫色到青色，步进60ms，跑一次大概(LED_NUM*60ms)，给8秒足够 */
        ws2812b_test_gradient_wipe(128, 0, 128,   0, 255, 255, 60, 8000);
        k_msleep(800);

        /* 结束再清屏 */
        ws2812b_fill(0, 0, 0);
        (void)ws2812b_show();
        k_msleep(1000);
    }

    return 0;
}

