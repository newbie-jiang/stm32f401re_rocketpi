#ifndef __UNIT_TEST
#define __UNIT_TEST

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdio.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "usart.h"
#include "cmsis_os.h"
#include "app.h"
#include "elog.h"



typedef struct
{
    float temperature;
    float humidity;
} unit_test_aht30_data_t;

typedef enum
{
    UNIT_TEST_BUTTON_STATE_IDLE = 0,
    UNIT_TEST_BUTTON_STATE_SHORT,
    UNIT_TEST_BUTTON_STATE_DOUBLE,
    UNIT_TEST_BUTTON_STATE_LONG
} unit_test_button_state_t;

typedef struct
{
    bool initialized;
    bool filesystem_available;
    bool test_passed;
    uint32_t total_mb;
    uint32_t free_mb;
} unit_test_card_info_t;

typedef enum
{
    UNIT_TEST_EEPROM_STATUS_NOT_TESTED = 0,
    UNIT_TEST_EEPROM_STATUS_TESTING,
    UNIT_TEST_EEPROM_STATUS_PASS,
    UNIT_TEST_EEPROM_STATUS_FAIL
} unit_test_eeprom_status_t;

typedef int (*shell_cmd_handler_t)(int argc, const char * const *argv);

typedef enum
{
    UNIT_TEST_LED_BLUE = 0,
    UNIT_TEST_LED_GREEN,
    UNIT_TEST_LED_PURPLE,
    UNIT_TEST_LED_COUNT
} unit_test_led_id_t;

typedef struct
{
    bool blink_enabled[UNIT_TEST_LED_COUNT];
    uint32_t blink_period_ms[UNIT_TEST_LED_COUNT];
    bool led_on[UNIT_TEST_LED_COUNT];
} unit_test_led_status_t;

void unit_test_aht30_channel_init(void);
bool unit_test_aht30_publish(const unit_test_aht30_data_t *data);
bool unit_test_aht30_receive(unit_test_aht30_data_t *data, uint32_t timeout_ms);
void shell_push_input_stream(const uint8_t *buffer, size_t length);
bool shell_register_command(const char *name, const char *description, shell_cmd_handler_t handler);
void shell_write(const char *text);
void shell_write_line(const char *text);
void unit_test_led_set_blink(unit_test_led_id_t led, bool enable, uint32_t period_ms);
void unit_test_led_set_color(unit_test_led_id_t led, bool on);
void unit_test_led_get_status(unit_test_led_status_t *status);
unit_test_button_state_t unit_test_button_get_state(void);
void unit_test_card_get_info(unit_test_card_info_t *info);
unit_test_eeprom_status_t unit_test_eeprom_get_status(void);
bool unit_test_usb_cdc_is_connected(void);
bool unit_test_irda_take_command(uint8_t *command);
bool unit_test_buzzer_request_beep(uint32_t frequency_hz, uint8_t duty_percent, uint32_t duration_ms);

#endif
