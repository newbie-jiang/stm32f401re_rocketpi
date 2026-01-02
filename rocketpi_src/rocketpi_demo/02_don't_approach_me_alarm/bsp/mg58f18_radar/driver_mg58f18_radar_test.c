/**
 * @file driver_mg58f18_radar_test.c
 * @brief MG58F18 雷达驱动的基础联调用例。
 */

#include "driver_mg58f18_radar_test.h"

#include <stdbool.h>
#include <stdio.h>

#include "stm32f4xx_hal.h"

#include "driver_mg58f18_radar.h"

static void mg58f18_radar_test_delay_between_commands(void)
{
    /* 协议建议两帧之间等待 >100ms，防止处理不及或写 Flash 超时 */
    mg58f18_radar_poll_and_print();
    HAL_Delay(120U);
}

/**
 * @brief 打印命令执行结果，便于串口观察。
 */
static void mg58f18_radar_test_print_status(const char *label, mg58f18_radar_status_t status)
{
    printf("[RADAR][%s] %s\r\n", label, mg58f18_radar_status_string(status));
}

/**
 * @brief 串行执行一次全量寄存器/参数读写，验证协议是否正常。
 */
void mg58f18_radar_test_run(void)
{
    printf("\r\n=== MG58F18 radar test ===\r\n");
    mg58f18_radar_status_t status = mg58f18_radar_init();
    mg58f18_radar_test_print_status("init", status);
    if (status != MG58F18_RADAR_STATUS_OK)
    {
        return;
    }

    HAL_Delay(720U);

    uint16_t distance_threshold = 0U;
    status = mg58f18_radar_get_distance_threshold(&distance_threshold);
    if (status == MG58F18_RADAR_STATUS_OK)
    {
        printf("Distance threshold: %u\r\n", distance_threshold);
        status = mg58f18_radar_set_distance_threshold(distance_threshold);
        mg58f18_radar_test_print_status("set_distance_threshold", status);
    }
    else
    {
        mg58f18_radar_test_print_status("get_distance_threshold", status);
    }
    mg58f18_radar_test_delay_between_commands();

    uint32_t delay_ms = 0U;
    status = mg58f18_radar_get_delay_ms(&delay_ms);
    if (status == MG58F18_RADAR_STATUS_OK)
    {
        printf("Output delay: %lu ms\r\n", (unsigned long)delay_ms);
        status = mg58f18_radar_set_delay_ms(delay_ms);
        mg58f18_radar_test_print_status("set_delay_ms", status);
    }
    else
    {
        mg58f18_radar_test_print_status("get_delay_ms", status);
    }
    mg58f18_radar_test_delay_between_commands();

    uint32_t block_ms = 0U;
    status = mg58f18_radar_get_block_time_ms(&block_ms);
    if (status == MG58F18_RADAR_STATUS_OK)
    {
        printf("Block time: %lu ms\r\n", (unsigned long)block_ms);
        status = mg58f18_radar_set_block_time_ms(block_ms);
        mg58f18_radar_test_print_status("set_block_time_ms", status);
    }
    else
    {
        mg58f18_radar_test_print_status("get_block_time_ms", status);
    }
    mg58f18_radar_test_delay_between_commands();

    bool light_enabled = false;
    status = mg58f18_radar_get_light_sensor_enabled(&light_enabled);
    if (status == MG58F18_RADAR_STATUS_OK)
    {
        printf("Light sensor: %s\r\n", light_enabled ? "ON" : "OFF");
        status = mg58f18_radar_set_light_sensor_enabled(light_enabled);
        mg58f18_radar_test_print_status("set_light_sensor_enabled", status);
    }
    else
    {
        mg58f18_radar_test_print_status("get_light_sensor_enabled", status);
    }
    mg58f18_radar_test_delay_between_commands();
    bool high_active = false;
    status = mg58f18_radar_get_active_level(&high_active);
    if (status == MG58F18_RADAR_STATUS_OK)
    {
        printf("Active level: %s\r\n", high_active ? "HIGH" : "LOW");
        status = mg58f18_radar_set_active_level(high_active);
        mg58f18_radar_test_print_status("set_active_level", status);
    }
    else
    {
        mg58f18_radar_test_print_status("get_active_level", status);
    }
    mg58f18_radar_test_delay_between_commands();
    bool normal_mode = false;
    status = mg58f18_radar_get_power_mode(&normal_mode);
    if (status == MG58F18_RADAR_STATUS_OK)
    {
        printf("Power mode: %s\r\n", normal_mode ? "NORMAL" : "ULTRA LOW");
        status = mg58f18_radar_set_power_mode(normal_mode);
        mg58f18_radar_test_print_status("set_power_mode", status);
    }
    else
    {
        mg58f18_radar_test_print_status("get_power_mode", status);
    }
    mg58f18_radar_test_delay_between_commands();

    bool triggered = false;
    status = mg58f18_radar_get_trigger_state(&triggered);
    if (status == MG58F18_RADAR_STATUS_OK)
    {
        printf("Trigger state: %s\r\n", triggered ? "TRIGGERED" : "IDLE");
    }
    else
    {
        mg58f18_radar_test_print_status("get_trigger_state", status);
    }
    mg58f18_radar_test_delay_between_commands();
    bool is_night = false;
    status = mg58f18_radar_get_light_environment(&is_night);
    if (status == MG58F18_RADAR_STATUS_OK)
    {
        printf("Environment: %s\r\n", is_night ? "NIGHT" : "DAY");
    }
    else
    {
        mg58f18_radar_test_print_status("get_light_environment", status);
    }
    mg58f18_radar_test_delay_between_commands();
    uint8_t fw_major = 0U;
    uint8_t fw_minor = 0U;
    status = mg58f18_radar_get_firmware_version(&fw_major, &fw_minor);
    if (status == MG58F18_RADAR_STATUS_OK)
    {
        printf("Firmware version: V%u.%u\r\n", fw_major, fw_minor);
    }
    else
    {
        mg58f18_radar_test_print_status("get_firmware_version", status);
    }
    mg58f18_radar_test_delay_between_commands();
    bool single_trigger = false;
    status = mg58f18_radar_get_trigger_mode(&single_trigger);
    if (status == MG58F18_RADAR_STATUS_OK)
    {
        printf("Trigger mode: %s\r\n", single_trigger ? "SINGLE" : "CONTINUOUS");
        status = mg58f18_radar_set_trigger_mode(single_trigger);
        mg58f18_radar_test_print_status("set_trigger_mode", status);
    }
    else
    {
        mg58f18_radar_test_print_status("get_trigger_mode", status);
    }
    mg58f18_radar_test_delay_between_commands();

    uint8_t power_step = 0U;
    status = mg58f18_radar_get_power_step(&power_step);
    if (status == MG58F18_RADAR_STATUS_OK)
    {
        printf("TX power step: %u\r\n", power_step);
        status = mg58f18_radar_set_power_step(power_step);
        mg58f18_radar_test_print_status("set_power_step", status);
    }
    else
    {
        mg58f18_radar_test_print_status("get_power_step", status);
    }
    mg58f18_radar_test_delay_between_commands();

    uint8_t light_threshold = 0U;
    status = mg58f18_radar_get_light_threshold(&light_threshold);
    if (status == MG58F18_RADAR_STATUS_OK)
    {
        printf("Light threshold: 0x%02X\r\n", light_threshold);
        status = mg58f18_radar_set_light_threshold(light_threshold);
        mg58f18_radar_test_print_status("set_light_threshold", status);
    }
    else
    {
        mg58f18_radar_test_print_status("get_light_threshold", status);
    }
    mg58f18_radar_test_delay_between_commands();

    bool pwm_enabled = false;
    status = mg58f18_radar_get_pwm_enabled(&pwm_enabled);
    if (status == MG58F18_RADAR_STATUS_OK)
    {
        printf("PWM enabled: %s\r\n", pwm_enabled ? "YES" : "NO");
        status = mg58f18_radar_set_pwm_enabled(pwm_enabled);
        mg58f18_radar_test_print_status("set_pwm_enabled", status);
    }
    else
    {
        mg58f18_radar_test_print_status("get_pwm_enabled", status);
    }
    mg58f18_radar_test_delay_between_commands();

    uint16_t pwm_duty = 0U;
    status = mg58f18_radar_get_pwm_duty_raw(&pwm_duty);
    if (status == MG58F18_RADAR_STATUS_OK)
    {
        printf("PWM duty raw: %u\r\n", pwm_duty);
        status = mg58f18_radar_set_pwm_duty_raw(pwm_duty);
        mg58f18_radar_test_print_status("set_pwm_duty_raw", status);
    }
    else
    {
        mg58f18_radar_test_print_status("get_pwm_duty_raw", status);
    }
    mg58f18_radar_test_delay_between_commands();

    uint8_t pulse_width = 0U;
    status = mg58f18_radar_get_power_pulse_width(&pulse_width);
    if (status == MG58F18_RADAR_STATUS_OK)
    {
        printf("Power pulse width: 0x%02X\r\n", pulse_width);
        status = mg58f18_radar_set_power_pulse_width(pulse_width);
        mg58f18_radar_test_print_status("set_power_pulse_width", status);
    }
    else
    {
        mg58f18_radar_test_print_status("get_power_pulse_width", status);
    }
    mg58f18_radar_test_delay_between_commands();
    bool hand_mode = false;
    status = mg58f18_radar_get_sensing_mode(&hand_mode);
    if (status == MG58F18_RADAR_STATUS_OK)
    {
        printf("Sensing mode: %s\r\n", hand_mode ? "HAND" : "MOTION");
        status = mg58f18_radar_set_sensing_mode(hand_mode);
        mg58f18_radar_test_print_status("set_sensing_mode", status);
    }
    else
    {
        mg58f18_radar_test_print_status("get_sensing_mode", status);
    }
    mg58f18_radar_test_delay_between_commands();

    status = mg58f18_radar_save_settings();
    mg58f18_radar_test_print_status("save_settings", status);

    printf("=== MG58F18 radar smoke test done ===\r\n");
}

/**
 * @brief 周期调用：打印串口收到的帧并监视 OUT 引脚跳变。
 */
void mg58f18_radar_test_poll(void)
{
    mg58f18_radar_poll_and_print();

    static bool io_state_initialised = false;
    static bool last_state = false;

    bool current_state = false;
    mg58f18_radar_status_t status = mg58f18_radar_read_io(&current_state);
    if (status != MG58F18_RADAR_STATUS_OK)
    {
        return;
    }

    if (!io_state_initialised)
    {
        last_state = current_state;
        io_state_initialised = true;
        printf("[RADAR][IO] initial state: %s\r\n", current_state ? "HIGH" : "LOW");
        return;
    }

    if (current_state != last_state)
    {
        printf("[RADAR][IO] state changed: %s -> %s\r\n",
               last_state ? "HIGH" : "LOW",
               current_state ? "HIGH" : "LOW");
        last_state = current_state;
    }
}
