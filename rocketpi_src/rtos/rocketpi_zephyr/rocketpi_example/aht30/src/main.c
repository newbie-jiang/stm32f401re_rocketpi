#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

LOG_MODULE_REGISTER(aht30_sample, CONFIG_LOG_DEFAULT_LEVEL);

#define AHT30_NODE DT_ALIAS(aht30)
#if !DT_NODE_HAS_STATUS(AHT30_NODE, okay)
#error "No alias 'aht30' found in the devicetree. Check your board overlay."
#endif

static const struct device *const aht30 = DEVICE_DT_GET(AHT30_NODE);

static inline void log_sensor_value(const char *label, const struct sensor_value *val)
{
	int32_t fractional = val->val2;

	if (fractional < 0) {
		fractional = -fractional;
	}

	LOG_INF("%s: %d.%06d", label, val->val1, fractional);
}

int main(void)
{
	struct sensor_value temperature;
	struct sensor_value humidity;
	int ret;

	if (!device_is_ready(aht30)) {
		LOG_ERR("AHT30 device %s is not ready", aht30->name);
		return 0;
	}

	while (1) {
		ret = sensor_sample_fetch(aht30);
		if (ret) {
			LOG_ERR("Failed to trigger measurement (%d)", ret);
			k_msleep(1000);
			continue;
		}

		ret = sensor_channel_get(aht30, SENSOR_CHAN_AMBIENT_TEMP, &temperature);
		if (ret) {
			LOG_ERR("Failed to read temperature (%d)", ret);
			k_msleep(1000);
			continue;
		}

		ret = sensor_channel_get(aht30, SENSOR_CHAN_HUMIDITY, &humidity);
		if (ret) {
			LOG_ERR("Failed to read humidity (%d)", ret);
			k_msleep(1000);
			continue;
		}

		LOG_INF("AHT30 sample:");
		log_sensor_value("  Temperature [C]", &temperature);
		log_sensor_value("  Humidity    [%]", &humidity);

		k_msleep(1000);
	}
}
