#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/sys/printk.h>

#if DT_NODE_EXISTS(DT_ALIAS(hcsr04))
#define HCSR04_NODE DT_ALIAS(hcsr04)
#elif DT_NODE_EXISTS(DT_NODELABEL(hcsr04_0))
#define HCSR04_NODE DT_NODELABEL(hcsr04_0)
#else
#error "No HC-SR04 node found. Define alias 'hcsr04' or node label 'hcsr04_0' in your overlay."
#endif

int main(void)
{
    const struct device *dev = DEVICE_DT_GET(HCSR04_NODE);

    if (!device_is_ready(dev)) {
        printk("HC-SR04 device not ready: %s\n", dev->name);
        return 0;
    }

    printk("HC-SR04 test started. device=%s\n", dev->name);

    const uint32_t period_ms = 200;

    uint32_t ok_cnt = 0;
    uint32_t fail_cnt = 0;
    uint32_t consecutive_fail = 0;

    while (1) {
        int ret;
        struct sensor_value dist;

        ret = sensor_sample_fetch(dev);
        if (ret) {
            fail_cnt++;
            consecutive_fail++;
            printk("[FAIL] fetch=%d (consecutive=%u)\n", ret, consecutive_fail);
            k_sleep(K_MSEC(period_ms));
            continue;
        }

        ret = sensor_channel_get(dev, SENSOR_CHAN_DISTANCE, &dist);
        if (ret) {
            fail_cnt++;
            consecutive_fail++;
            printk("[FAIL] get=%d (consecutive=%u)\n", ret, consecutive_fail);
            k_sleep(K_MSEC(period_ms));
            continue;
        }

        consecutive_fail = 0;
        ok_cnt++;

        /* SENSOR_CHAN_DISTANCE unit is meters; convert to centimeters for display */
        double cm = sensor_value_to_double(&dist) * 100.0;

        printk("[OK %u] distance = %.2f cm  ok=%u fail=%u\n",
               ok_cnt, cm, ok_cnt, fail_cnt);

        k_sleep(K_MSEC(period_ms));
    }

    return 0;
}
