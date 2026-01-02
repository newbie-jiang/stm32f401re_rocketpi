#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/sys/printk.h>

#if !DT_NODE_EXISTS(DT_ALIAS(buzzer0))
#error "No alias 'buzzer0' found in devicetree (aliases { buzzer0 = &...; })."
#endif

/* buzzer0 指向的节点必须带 pwms 属性 */
static const struct pwm_dt_spec buzzer = PWM_DT_SPEC_GET(DT_ALIAS(buzzer0));

static int buzzer_on(uint32_t on_ms)
{
    if (!device_is_ready(buzzer.dev)) {
        printk("PWM device not ready\n");
        return -ENODEV;
    }

    /* 频率由 DTS 中 PWM_HZ(2700) 决定，buzzer.period 已经带出来了 */
    uint32_t period = buzzer.period;

    /* 50% 占空比：无源蜂鸣器一般够响；不响可改成 30%~70% 试试 */
    uint32_t pulse = period / 2U;

    int ret = pwm_set_dt(&buzzer, period, pulse);
    if (ret) {
        printk("pwm_set_dt(on) failed: %d\n", ret);
        return ret;
    }

    k_msleep(on_ms);
    return 0;
}

static void buzzer_off(uint32_t off_ms)
{
    /* 关声：把 pulse 设为 0 */
    (void)pwm_set_dt(&buzzer, buzzer.period, 0);
    k_msleep(off_ms);
}

int main(void)
{
    printk("Buzzer test start\n");

    /* 哔哔两声：响 150ms，停 120ms，响 150ms */
    (void)buzzer_on(150);
    buzzer_off(120);
    (void)buzzer_on(150);
    buzzer_off(0);

    printk("Buzzer test done\n");

    while (1) {
        k_sleep(K_SECONDS(1));
    }
    return 0;
}
