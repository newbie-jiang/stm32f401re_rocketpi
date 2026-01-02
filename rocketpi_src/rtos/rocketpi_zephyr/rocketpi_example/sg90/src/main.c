#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/sys/printk.h>

#ifndef DT_ALIAS
#error "DT_ALIAS not available"
#endif

#if !DT_NODE_EXISTS(DT_ALIAS(servo0))
#error "No alias 'servo0' found in devicetree (aliases { servo0 = &...; })."
#endif

static const struct pwm_dt_spec servo = PWM_DT_SPEC_GET(DT_ALIAS(servo0));

/* SG90 常见范围：0.5ms~2.5ms 更通用 */
#define SG90_MIN_PULSE_US  500U
#define SG90_MAX_PULSE_US  2500U


static uint32_t clamp_u32(uint32_t v, uint32_t lo, uint32_t hi)
{
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

/* angle(0..180) -> pulse_us */
static uint32_t angle_to_pulse_us(int angle_deg)
{
    angle_deg = (int)clamp_u32((uint32_t)angle_deg, 0U, 180U);

    /* 线性映射：pulse = min + angle/180*(max-min) */
    const uint32_t span = SG90_MAX_PULSE_US - SG90_MIN_PULSE_US;

    /* 用整数实现四舍五入： +90 相当于 +0.5*180 */
    uint32_t pulse = SG90_MIN_PULSE_US + (uint32_t)((angle_deg * (int)span + 90) / 180);

    return clamp_u32(pulse, SG90_MIN_PULSE_US, SG90_MAX_PULSE_US);
}

static int servo_set_pulse_us(uint32_t pulse_us)
{
    if (!device_is_ready(servo.dev)) {
        printk("PWM device not ready\n");
        return -ENODEV;
    }

    /* servo.period 来自 DTS 的 PWM_HZ(50) => 20ms */
    const uint32_t period_ns = servo.period;

    /* us -> ns */
    uint32_t pulse_ns = pulse_us * 1000U;

    if (pulse_ns > period_ns) {
        pulse_ns = period_ns;
    }

    return pwm_set_dt(&servo, period_ns, pulse_ns);
}

static int servo_set_angle(int angle_deg)
{
    uint32_t pulse_us = angle_to_pulse_us(angle_deg);
    return servo_set_pulse_us(pulse_us);
}

int main(void)
{
    printk("SG90 sweep start (50Hz). device=%s\n", servo.dev->name);

    /* 先到中位 */
    (void)servo_set_angle(90);
    k_msleep(500);

    const int step_deg = 1;      /* 步进角度：1° */
    const int dwell_ms = 5;     /* 每步停留时间：影响转动速度 */

    while (1) {
        /* 0 -> 180 */
        for (int a = 0; a <= 180; a += step_deg) {
            int ret = servo_set_angle(a);
            if (ret) {
                printk("[FAIL] set %d deg ret=%d\n", a, ret);
            } else {
                printk("angle=%d deg\n", a);
            }
            k_msleep(dwell_ms);
        }

        /* 180 -> 0（避免端点重复停两次） */
        for (int a = 180 - step_deg; a >= 0 + step_deg; a -= step_deg) {
            int ret = servo_set_angle(a);
            if (ret) {
                printk("[FAIL] set %d deg ret=%d\n", a, ret);
            } else {
                printk("angle=%d deg\n", a);
            }
            k_msleep(dwell_ms);
        }
    }

    return 0;
}
