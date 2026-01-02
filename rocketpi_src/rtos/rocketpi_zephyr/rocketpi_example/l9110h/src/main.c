#include <errno.h>
#include <stdbool.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/sys/printk.h>

#if !DT_NODE_EXISTS(DT_ALIAS(motorina)) || !DT_NODE_EXISTS(DT_ALIAS(motorinb))
#error "Missing motorina or motorinb alias in devicetree."
#endif

#define PWM_FREQ_HZ           20000U
#define DUTY_MAX_PCT          100U
#define RAMP_STEP_PCT         5U
#define RAMP_DELAY_MS         120U
#define HOLD_DELAY_MS         1200U
#define STOP_DELAY_MS         800U

static const struct pwm_dt_spec motor_ina = PWM_DT_SPEC_GET(DT_ALIAS(motorina));
static const struct pwm_dt_spec motor_inb = PWM_DT_SPEC_GET(DT_ALIAS(motorinb));

static int pwm_apply_percent(const struct pwm_dt_spec *chan, uint32_t percent)
{
	uint32_t pulse = (chan->period * percent) / DUTY_MAX_PCT;

	return pwm_set_dt(chan, chan->period, pulse);
}

static int pwm_channel_off(const struct pwm_dt_spec *chan)
{
	return pwm_set_dt(chan, chan->period, 0U);
}

static int motor_stop(void)
{
	int ret = pwm_channel_off(&motor_ina);

	if (ret) {
		return ret;
	}

	return pwm_channel_off(&motor_inb);
}

static int ramp_channel(const struct pwm_dt_spec *chan,
			uint32_t start_pct,
			uint32_t stop_pct)
{
	if (start_pct > DUTY_MAX_PCT || stop_pct > DUTY_MAX_PCT) {
		return -EINVAL;
	}

	if (start_pct == stop_pct) {
		return pwm_apply_percent(chan, start_pct);
	}

	int32_t duty = (int32_t)start_pct;
	const int32_t target = (int32_t)stop_pct;
	const int32_t step = (duty < target) ? (int32_t)RAMP_STEP_PCT : -(int32_t)RAMP_STEP_PCT;

	while (true) {
		int ret = pwm_apply_percent(chan, (uint32_t)duty);

		if (ret) {
			return ret;
		}

		if (duty == target) {
			break;
		}

		k_msleep(RAMP_DELAY_MS);

		duty += step;
		if ((step > 0 && duty > target) || (step < 0 && duty < target)) {
			duty = target;
		}
	}

	return 0;
}

static int motor_profile(bool forward)
{
	const struct pwm_dt_spec *active = forward ? &motor_ina : &motor_inb;
	const struct pwm_dt_spec *inactive = forward ? &motor_inb : &motor_ina;
	const char *dir = forward ? "CW" : "CCW";
	int ret;

	ret = pwm_channel_off(inactive);
	if (ret) {
		return ret;
	}

	printk("Motor %s ramp-up\n", dir);
	ret = ramp_channel(active, 0U, DUTY_MAX_PCT);
	if (ret) {
		return ret;
	}

	k_msleep(HOLD_DELAY_MS);

	printk("Motor %s ramp-down\n", dir);
	ret = ramp_channel(active, DUTY_MAX_PCT, 0U);
	if (ret) {
		return ret;
	}

	ret = motor_stop();
	if (ret) {
		return ret;
	}

	printk("Motor stop\n");
	k_msleep(STOP_DELAY_MS);
	return 0;
}

int main(void)
{
	printk("L9110H PWM demo @ %u Hz\n", PWM_FREQ_HZ);

	if (!pwm_is_ready_dt(&motor_ina) || !pwm_is_ready_dt(&motor_inb)) {
		printk("PWM device not ready\n");
		return 0;
	}

	while (true) {
		if (motor_profile(true)) {
			break;
		}

		if (motor_profile(false)) {
			break;
		}
	}

	(void)motor_stop();
	printk("Motor control stopped due to error\n");
	return 0;
}
