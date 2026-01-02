#include <zephyr/device.h>
#include <zephyr/dt-bindings/input/input-event-codes.h>
#include <zephyr/input/input.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>
#include <errno.h>

LOG_MODULE_REGISTER(adc_joystick, CONFIG_LOG_DEFAULT_LEVEL);

#define JOYSTICK_ANALOG_NODE DT_ALIAS(joystick0)
#define JOYSTICK_BUTTON_NODE DT_ALIAS(joystickbutton0)

#if !DT_NODE_HAS_STATUS(JOYSTICK_ANALOG_NODE, okay)
#error "joystick0 alias is not defined or disabled in the devicetree"
#endif

#if !DT_NODE_HAS_STATUS(JOYSTICK_BUTTON_NODE, okay)
#error "joystickbutton0 alias is not defined or disabled in the devicetree"
#endif

static const char *axis_name_from_code(uint16_t code)
{
	switch (code) {
	case INPUT_ABS_X:
		return "X";
	case INPUT_ABS_Y:
		return "Y";
	default:
		return "ABS";
	}
}

static const char *button_name_from_code(uint16_t code)
{
	switch (code) {
	case INPUT_BTN_SELECT:
		return "Select";
	default:
		return "BTN";
	}
}

static void joystick_event_cb(struct input_event *evt, void *user_data)
{
	ARG_UNUSED(user_data);

	const char *dev_name = evt->dev ? evt->dev->name : "unknown";

	switch (evt->type) {
	case INPUT_EV_ABS:
		LOG_INF("[%s] %s axis: %d", dev_name,
			axis_name_from_code(evt->code), evt->value);
		break;
	case INPUT_EV_KEY:
		LOG_INF("[%s] %s %s", dev_name,
			button_name_from_code(evt->code),
			evt->value ? "pressed" : "released");
		break;
	default:
		LOG_DBG("[%s] unhandled event type %u code %u value %d",
			dev_name, evt->type, evt->code, evt->value);
		break;
	}
}

INPUT_CALLBACK_DEFINE(NULL, joystick_event_cb, NULL);

int main(void)
{
	const struct device *joystick = DEVICE_DT_GET(JOYSTICK_ANALOG_NODE);
	const struct device *buttons = DEVICE_DT_GET(JOYSTICK_BUTTON_NODE);

	if (!device_is_ready(joystick)) {
		LOG_ERR("Analog joystick device %s is not ready", joystick->name);
		return -ENODEV;
	}

	if (!device_is_ready(buttons)) {
		LOG_ERR("Joystick button device %s is not ready", buttons->name);
		return -ENODEV;
	}

	LOG_INF("ADC joystick input demo is running");

	while (true) {
		k_sleep(K_SECONDS(1));
	}
}
