#include "app.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>

#include "cJSON.h"
#include "driver_aht30.h"
#include "driver_buzzer_test.h"
#include "driver_esp8266_at.h"
#include "driver_motor_l9110.h"
#include "main.h"
#include "stm32f4xx_hal.h"
#include "st7789.h"
#include "font16x24_ascii.h"

#ifndef APP_WIFI_SSID
#define APP_WIFI_SSID "ZTE-45c476"
#endif
#ifndef APP_WIFI_PASSWORD
#define APP_WIFI_PASSWORD "88888888"
#endif

#ifndef APP_MQTT_BROKER
#define APP_MQTT_BROKER "broker.emqx.io"
#endif
#ifndef APP_MQTT_PORT
#define APP_MQTT_PORT 1883
#endif
#ifndef APP_MQTT_USERNAME
#define APP_MQTT_USERNAME ""
#endif
#ifndef APP_MQTT_PASSWORD
#define APP_MQTT_PASSWORD ""
#endif
#ifndef APP_MQTT_BASE_TOPIC
#define APP_MQTT_BASE_TOPIC "rocketpi"
#endif
#ifndef APP_MQTT_CLIENT_ID_PREFIX
#define APP_MQTT_CLIENT_ID_PREFIX "rocketpi-"
#endif
#ifndef APP_MQTT_QOS
#define APP_MQTT_QOS 1U
#endif
#ifndef APP_PUBLISH_INTERVAL_MS
#define APP_PUBLISH_INTERVAL_MS 2000U
#endif
#ifndef APP_MQTT_RECONNECT_MS
#define APP_MQTT_RECONNECT_MS 5000U
#endif
#ifndef APP_MOTOR_DEFAULT_SPEED
#define APP_MOTOR_DEFAULT_SPEED 60U
#endif
#ifndef APP_BUZZER_MIN_FREQ_HZ
#define APP_BUZZER_MIN_FREQ_HZ 1000U
#endif
#ifndef APP_BUZZER_MAX_FREQ_HZ
#define APP_BUZZER_MAX_FREQ_HZ 3000U
#endif
#ifndef APP_BUZZER_DEFAULT_FREQ_HZ
#define APP_BUZZER_DEFAULT_FREQ_HZ 2000U
#endif
#ifndef APP_BUZZER_DUTY_PERCENT
#define APP_BUZZER_DUTY_PERCENT 50U
#endif
#ifndef APP_LCD_STATUS_REFRESH_MS
#define APP_LCD_STATUS_REFRESH_MS 2000U
#endif
#ifndef APP_DEVICE_ID_LEN
#define APP_DEVICE_ID_LEN 12U
#endif
#ifndef APP_MQTT_CLIENT_ID_MAX_LEN
#define APP_MQTT_CLIENT_ID_MAX_LEN 32U
#endif
#ifndef APP_MQTT_TOPIC_MAX_LEN
#define APP_MQTT_TOPIC_MAX_LEN 96U
#endif

#ifndef APP_LED_GPIO_PORT
#define APP_LED_GPIO_PORT LED_G_GPIO_Port
#endif
#ifndef APP_LED_PIN
#define APP_LED_PIN LED_G_Pin
#endif

typedef struct
{
    bool     wifi_ready;
    bool     mqtt_ready;
    uint32_t last_publish_ms;
    bool     loop_started;
    bool     wait_logged;
    bool     lcd_ready;
    bool     lcd_showing_status;
    uint32_t last_lcd_update_ms;
    int16_t  last_temp10;
    uint16_t last_hum10;
    uint32_t last_mqtt_attempt_ms;
    bool     mqtt_reconnect_pending;
    char     device_id[APP_DEVICE_ID_LEN + 1U];
    char     mqtt_client_id[APP_MQTT_CLIENT_ID_MAX_LEN];
    char     mqtt_sensor_topic[APP_MQTT_TOPIC_MAX_LEN];
    char     mqtt_control_topic[APP_MQTT_TOPIC_MAX_LEN];
    char     mqtt_motor_topic[APP_MQTT_TOPIC_MAX_LEN];
    char     mqtt_buzzer_topic[APP_MQTT_TOPIC_MAX_LEN];
} app_state_t;

static app_state_t s_app;

static void app_make_device_id(char *out, size_t out_size)
{
    if (out == NULL || out_size < (APP_DEVICE_ID_LEN + 1U))
    {
        return;
    }

    const uint8_t *uid = (const uint8_t *)UID_BASE;
    uint64_t hash = 1469598103934665603ULL;
    for (size_t i = 0; i < 12U; ++i)
    {
        hash ^= (uint64_t)uid[i];
        hash *= 1099511628211ULL;
    }

    for (size_t i = 0; i < APP_DEVICE_ID_LEN; ++i)
    {
        out[APP_DEVICE_ID_LEN - 1U - i] = (char)('a' + (hash % 26U));
        hash /= 26U;
    }
    out[APP_DEVICE_ID_LEN] = '\0';
}

static void app_build_mqtt_identifiers(void)
{
    app_make_device_id(s_app.device_id, sizeof(s_app.device_id));

    (void)snprintf(s_app.mqtt_client_id,
                   sizeof(s_app.mqtt_client_id),
                   "%s%s",
                   APP_MQTT_CLIENT_ID_PREFIX,
                   s_app.device_id);

    (void)snprintf(s_app.mqtt_sensor_topic,
                   sizeof(s_app.mqtt_sensor_topic),
                   "%s/sensors/%s/aht30",
                   APP_MQTT_BASE_TOPIC,
                   s_app.device_id);

    (void)snprintf(s_app.mqtt_control_topic,
                   sizeof(s_app.mqtt_control_topic),
                   "%s/actuators/%s/led/cmd",
                   APP_MQTT_BASE_TOPIC,
                   s_app.device_id);

    (void)snprintf(s_app.mqtt_motor_topic,
                   sizeof(s_app.mqtt_motor_topic),
                   "%s/actuators/%s/motor/cmd",
                   APP_MQTT_BASE_TOPIC,
                   s_app.device_id);

    (void)snprintf(s_app.mqtt_buzzer_topic,
                   sizeof(s_app.mqtt_buzzer_topic),
                   "%s/actuators/%s/buzzer/cmd",
                   APP_MQTT_BASE_TOPIC,
                   s_app.device_id);
}

static void app_log_status(const char *label, esp8266_at_status_t status)
{
    printf("[APP][%s] %s\r\n", label, esp8266_at_status_string(status));
}

static bool app_is_placeholder(const char *value, const char *placeholder)
{
    if (value == NULL || placeholder == NULL)
    {
        return true;
    }
    return (strcmp(value, placeholder) == 0);
}

static void app_drain_events(uint32_t delay_ms)
{
    if (delay_ms > 0U)
    {
        HAL_Delay(delay_ms);
    }

    esp8266_at_poll();

    esp8266_at_event_t event;
    while (esp8266_at_fetch_event(&event))
    {
    }
}

static void app_set_led(bool on)
{
    HAL_GPIO_WritePin(APP_LED_GPIO_PORT, APP_LED_PIN,
                      on ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

static float app_round_1dp(float value)
{
    const float scaled = value * 10.0f;
    const float biased = (scaled >= 0.0f) ? (scaled + 0.5f) : (scaled - 0.5f);
    const int32_t rounded = (int32_t)biased;
    return ((float)rounded) / 10.0f;
}

static void app_lcd_show_status(void)
{
    if (!s_app.lcd_ready)
    {
        return;
    }

    char line[24];

    ST7789_Clear(BLACK);
    ST7789_ShowString(10, 20, "ESP8266", Font16x24, WHITE, BLACK);

    (void)snprintf(line, sizeof(line), "WiFi:%s", s_app.wifi_ready ? "OK" : "OFF");
    ST7789_ShowString(10, 60, line, Font16x24,
                      s_app.wifi_ready ? GREEN : YELLOW, BLACK);

    (void)snprintf(line, sizeof(line), "MQTT:%s", s_app.mqtt_ready ? "OK" : "WAIT");
    ST7789_ShowString(10, 100, line, Font16x24,
                      s_app.mqtt_ready ? GREEN : YELLOW, BLACK);

    (void)snprintf(line, sizeof(line), "ID:%s", s_app.device_id);
    ST7789_ShowString(0, 140, line, Font16x24, WHITE, BLACK);

    s_app.lcd_showing_status = true;
    s_app.last_lcd_update_ms = HAL_GetTick();
}

static void app_lcd_show_sensor(int16_t temp10, uint16_t hum10)
{
    if (!s_app.lcd_ready)
    {
        return;
    }

    char line[24];

    ST7789_Clear(BLACK);
    if (temp10 == INT16_MIN || hum10 == UINT16_MAX)
    {
        ST7789_ShowString(10, 40, "T:--.-C", Font16x24, WHITE, BLACK);
        ST7789_ShowString(10, 80, "H:--.-%", Font16x24, WHITE, BLACK);
    }
    else
    {
        (void)snprintf(line, sizeof(line), "T:%d.%01dC",
                       temp10 / 10, abs(temp10 % 10));
        ST7789_ShowString(10, 40, line, Font16x24, WHITE, BLACK);

        (void)snprintf(line, sizeof(line), "H:%u.%01u%%",
                       (unsigned int)(hum10 / 10),
                       (unsigned int)(hum10 % 10));
        ST7789_ShowString(10, 80, line, Font16x24, WHITE, BLACK);
    }

    (void)snprintf(line, sizeof(line), "ID:%s", s_app.device_id);
    ST7789_ShowString(0, 120, line, Font16x24, WHITE, BLACK);

    s_app.lcd_showing_status = false;
    s_app.last_lcd_update_ms = HAL_GetTick();
    s_app.last_temp10 = temp10;
    s_app.last_hum10 = hum10;
}

static void app_lcd_show_sensor_placeholder(void)
{
    app_lcd_show_sensor(INT16_MIN, UINT16_MAX);
}

static void app_lcd_update_status(void)
{
    if (!s_app.lcd_ready)
    {
        return;
    }

    const uint32_t now = HAL_GetTick();
    if (!s_app.lcd_showing_status
        || (uint32_t)(now - s_app.last_lcd_update_ms) > APP_LCD_STATUS_REFRESH_MS)
    {
        app_lcd_show_status();
    }
}

static void app_set_led_channel(GPIO_TypeDef *port, uint16_t pin, bool on)
{
    HAL_GPIO_WritePin(port, pin, on ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

static bool app_get_bool_value(const cJSON *item, bool *value)
{
    if (value == NULL)
    {
        return false;
    }
    if (cJSON_IsBool(item))
    {
        *value = cJSON_IsTrue(item);
        return true;
    }
    if (cJSON_IsNumber(item))
    {
        *value = (item->valueint != 0);
        return true;
    }
    return false;
}

static bool app_get_bool_field(const cJSON *root, const char *key, bool *value)
{
    if (root == NULL || key == NULL)
    {
        return false;
    }
    return app_get_bool_value(cJSON_GetObjectItem(root, key), value);
}

static const char *app_get_string_field(const cJSON *root, const char *key)
{
    const cJSON *item = cJSON_GetObjectItem(root, key);
    if (cJSON_IsString(item) && item->valuestring != NULL)
    {
        return item->valuestring;
    }
    return NULL;
}

static bool app_get_int_field(const cJSON *root, const char *key, int *value)
{
    const cJSON *item = cJSON_GetObjectItem(root, key);
    if (!cJSON_IsNumber(item) || value == NULL)
    {
        return false;
    }
    *value = item->valueint;
    return true;
}

static bool app_set_led_by_id(const char *id, bool on)
{
    if (id == NULL)
    {
        return false;
    }
    if (strcmp(id, "b") == 0 || strcmp(id, "blue") == 0)
    {
        app_set_led_channel(LED_B_GPIO_Port, LED_B_Pin, on);
        return true;
    }
    if (strcmp(id, "g") == 0 || strcmp(id, "green") == 0)
    {
        app_set_led_channel(LED_G_GPIO_Port, LED_G_Pin, on);
        return true;
    }
    if (strcmp(id, "p") == 0 || strcmp(id, "pink") == 0)
    {
        app_set_led_channel(LED_P_GPIO_Port, LED_P_Pin, on);
        return true;
    }
    return false;
}

static const char *app_cmd_from_topic(const char *topic)
{
    if (topic == NULL)
    {
        return NULL;
    }
    if (strcmp(topic, s_app.mqtt_motor_topic) == 0)
    {
        return "motor";
    }
    if (strcmp(topic, s_app.mqtt_buzzer_topic) == 0)
    {
        return "buzzer";
    }
    if (strcmp(topic, s_app.mqtt_control_topic) == 0)
    {
        return "led";
    }
    return NULL;
}

static bool app_is_control_topic(const char *topic)
{
    if (topic == NULL)
    {
        return false;
    }
    return (strcmp(topic, s_app.mqtt_control_topic) == 0
            || strcmp(topic, s_app.mqtt_motor_topic) == 0
            || strcmp(topic, s_app.mqtt_buzzer_topic) == 0);
}

static esp8266_at_status_t app_mqtt_subscribe_topic(const char *topic, const char *label)
{
    if (topic == NULL || topic[0] == '\0')
    {
        return ESP8266_AT_STATUS_INVALID_ARGUMENT;
    }

    char sub_args[ESP8266_AT_MAX_LINE_LENGTH];
    (void)snprintf(sub_args,
                   sizeof(sub_args),
                   "%u,\"%s\",%u",
                   0U,
                   topic,
                   (unsigned int)APP_MQTT_QOS);

    esp8266_at_status_t status = esp8266_at_send_command(ESP8266_AT_CMD_MQTTSUB,
                                                         ESP8266_AT_COMMAND_MODE_SET,
                                                         sub_args,
                                                         ESP8266_AT_DEFAULT_TIMEOUT_MS,
                                                         false);
    if (label != NULL)
    {
        app_log_status(label, status);
    }
    app_drain_events(150U);
    return status;
}

static void app_mqtt_subscribe_all(void)
{
    (void)app_mqtt_subscribe_topic(s_app.mqtt_control_topic, "mqtt_sub_led");

    if (strcmp(s_app.mqtt_motor_topic, s_app.mqtt_control_topic) != 0)
    {
        (void)app_mqtt_subscribe_topic(s_app.mqtt_motor_topic, "mqtt_sub_motor");
    }

    if (strcmp(s_app.mqtt_buzzer_topic, s_app.mqtt_control_topic) != 0
        && strcmp(s_app.mqtt_buzzer_topic, s_app.mqtt_motor_topic) != 0)
    {
        (void)app_mqtt_subscribe_topic(s_app.mqtt_buzzer_topic, "mqtt_sub_buzzer");
    }
}

static void app_mqtt_schedule_reconnect(void)
{
    s_app.mqtt_reconnect_pending = true;
}

static void app_mqtt_try_reconnect(void)
{
    if (!s_app.wifi_ready || s_app.mqtt_ready)
    {
        return;
    }

    const uint32_t now = HAL_GetTick();
    if ((uint32_t)(now - s_app.last_mqtt_attempt_ms) < APP_MQTT_RECONNECT_MS)
    {
        return;
    }
    if (!esp8266_at_is_ready())
    {
        return;
    }

    s_app.last_mqtt_attempt_ms = now;
    printf("[APP][mqtt] reconnecting...\r\n");

    const esp8266_at_status_t status =
        esp8266_at_mqtt_connect(0U,
                                APP_MQTT_BROKER,
                                APP_MQTT_PORT,
                                120U,
                                true);
    app_log_status("mqtt_reconn", status);
    app_drain_events(200U);

    if (status == ESP8266_AT_STATUS_OK)
    {
        s_app.mqtt_ready = true;
        s_app.mqtt_reconnect_pending = false;
        app_mqtt_subscribe_all();
        app_lcd_show_sensor_placeholder();
    }
}

static bool app_parse_mqtt_subrecv(const char *payload,
                                   char *topic,
                                   size_t topic_size,
                                   const char **data_ptr,
                                   size_t *data_len)
{
    if (payload == NULL || topic == NULL || topic_size == 0U
        || data_ptr == NULL || data_len == NULL)
    {
        return false;
    }

    const char *cursor = payload;
    while (*cursor == ' ')
    {
        ++cursor;
    }

    char *endptr = NULL;
    (void)strtol(cursor, &endptr, 10);
    if (endptr == cursor || *endptr != ',')
    {
        return false;
    }
    cursor = endptr + 1;

    if (*cursor != '"')
    {
        return false;
    }
    ++cursor;

    const char *topic_start = cursor;
    while (*cursor != '\0' && *cursor != '"')
    {
        ++cursor;
    }
    if (*cursor != '"')
    {
        return false;
    }

    size_t topic_len = (size_t)(cursor - topic_start);
    if (topic_len >= topic_size)
    {
        topic_len = topic_size - 1U;
    }
    memcpy(topic, topic_start, topic_len);
    topic[topic_len] = '\0';

    ++cursor;
    if (*cursor != ',')
    {
        return false;
    }
    ++cursor;

    long parsed_len = strtol(cursor, &endptr, 10);
    if (endptr == cursor || *endptr != ',')
    {
        return false;
    }
    if (parsed_len < 0)
    {
        return false;
    }
    cursor = endptr + 1;

    *data_ptr = cursor;
    *data_len = (size_t)parsed_len;
    return true;
}

static void app_apply_led_command(const cJSON *root)
{
    const cJSON *state = cJSON_GetObjectItem(root, "state");
    bool handled = false;
    const cJSON *id = cJSON_GetObjectItem(root, "id");
    bool on = false;

    if (cJSON_IsString(id) && app_get_bool_value(state, &on))
    {
        if (app_set_led_by_id(id->valuestring, on))
        {
            printf("[APP][cmd] led_%s=%s\r\n",
                   id->valuestring,
                   on ? "on" : "off");
            handled = true;
        }
    }

    bool b_on = false;
    bool g_on = false;
    bool p_on = false;
    bool b_set = app_get_bool_field(root, "b", &b_on);
    if (!b_set)
    {
        b_set = app_get_bool_field(root, "led_b", &b_on);
    }
    bool g_set = app_get_bool_field(root, "g", &g_on);
    if (!g_set)
    {
        g_set = app_get_bool_field(root, "led_g", &g_on);
    }
    bool p_set = app_get_bool_field(root, "p", &p_on);
    if (!p_set)
    {
        p_set = app_get_bool_field(root, "led_p", &p_on);
    }

    if (b_set)
    {
        app_set_led_channel(LED_B_GPIO_Port, LED_B_Pin, b_on);
        printf("[APP][cmd] led_b=%s\r\n", b_on ? "on" : "off");
        handled = true;
    }
    if (g_set)
    {
        app_set_led_channel(LED_G_GPIO_Port, LED_G_Pin, g_on);
        printf("[APP][cmd] led_g=%s\r\n", g_on ? "on" : "off");
        handled = true;
    }
    if (p_set)
    {
        app_set_led_channel(LED_P_GPIO_Port, LED_P_Pin, p_on);
        printf("[APP][cmd] led_p=%s\r\n", p_on ? "on" : "off");
        handled = true;
    }

    if (!handled && app_get_bool_value(state, &on))
    {
        app_set_led_channel(LED_B_GPIO_Port, LED_B_Pin, on);
        app_set_led_channel(LED_G_GPIO_Port, LED_G_Pin, on);
        app_set_led_channel(LED_P_GPIO_Port, LED_P_Pin, on);
        printf("[APP][cmd] led_all=%s\r\n", on ? "on" : "off");
    }
}

static void app_apply_motor_command(const cJSON *root)
{
    const char *direction = app_get_string_field(root, "dir");
    if (direction == NULL)
    {
        direction = app_get_string_field(root, "action");
    }
    if (direction == NULL)
    {
        direction = app_get_string_field(root, "state");
    }

    int speed_value = (int)APP_MOTOR_DEFAULT_SPEED;
    bool speed_set = app_get_int_field(root, "speed", &speed_value);
    if (!speed_set)
    {
        speed_set = app_get_int_field(root, "duty", &speed_value);
    }

    motor_l9110_direction_t dir = MOTOR_L9110_DIR_BRAKE;
    bool dir_set = false;

    if (direction != NULL)
    {
        if ((strcmp(direction, "forward") == 0) || (strcmp(direction, "fwd") == 0))
        {
            dir = MOTOR_L9110_DIR_FORWARD;
            dir_set = true;
        }
        else if ((strcmp(direction, "reverse") == 0) || (strcmp(direction, "rev") == 0))
        {
            dir = MOTOR_L9110_DIR_REVERSE;
            dir_set = true;
        }
        else if ((strcmp(direction, "stop") == 0) || (strcmp(direction, "brake") == 0)
                 || (strcmp(direction, "off") == 0))
        {
            dir = MOTOR_L9110_DIR_BRAKE;
            dir_set = true;
        }
    }

    if (!dir_set)
    {
        bool on = false;
        if (app_get_bool_value(cJSON_GetObjectItem(root, "state"), &on))
        {
            dir = on ? MOTOR_L9110_DIR_FORWARD : MOTOR_L9110_DIR_BRAKE;
            dir_set = true;
        }
        else if (speed_set)
        {
            if (speed_value < 0)
            {
                dir = MOTOR_L9110_DIR_REVERSE;
                speed_value = -speed_value;
                dir_set = true;
            }
            else if (speed_value > 0)
            {
                dir = MOTOR_L9110_DIR_FORWARD;
                dir_set = true;
            }
            else
            {
                dir = MOTOR_L9110_DIR_BRAKE;
                dir_set = true;
            }
        }
    }

    if (!dir_set)
    {
        printf("[APP][cmd] motor missing direction\r\n");
        return;
    }

    if (speed_value < 0)
    {
        speed_value = -speed_value;
    }
    if (speed_value > 100)
    {
        speed_value = 100;
    }

    if (dir == MOTOR_L9110_DIR_BRAKE || speed_value == 0)
    {
        motor_l9110_brake();
        printf("[APP][cmd] motor stop\r\n");
        return;
    }

    motor_l9110_drive(dir, (uint8_t)speed_value);
    printf("[APP][cmd] motor %s speed=%d\r\n",
           (dir == MOTOR_L9110_DIR_FORWARD) ? "forward" : "reverse",
           speed_value);
}

static void app_apply_buzzer_command(const cJSON *root)
{
    int freq_value = (int)APP_BUZZER_DEFAULT_FREQ_HZ;
    bool freq_set = app_get_int_field(root, "freq", &freq_value);
    if (!freq_set)
    {
        freq_set = app_get_int_field(root, "frequency", &freq_value);
    }

    bool on = false;
    bool state_set = app_get_bool_value(cJSON_GetObjectItem(root, "state"), &on);
    if (!state_set && freq_set)
    {
        on = true;
        state_set = true;
    }

    if (!state_set)
    {
        printf("[APP][cmd] buzzer missing state\r\n");
        return;
    }

    if (freq_value < (int)APP_BUZZER_MIN_FREQ_HZ)
    {
        freq_value = (int)APP_BUZZER_MIN_FREQ_HZ;
    }
    if (freq_value > (int)APP_BUZZER_MAX_FREQ_HZ)
    {
        freq_value = (int)APP_BUZZER_MAX_FREQ_HZ;
    }

    if (on)
    {
        (void)buzzer_test_start((uint32_t)freq_value, APP_BUZZER_DUTY_PERCENT);
        printf("[APP][cmd] buzzer on freq=%d\r\n", freq_value);
    }
    else
    {
        (void)buzzer_test_stop();
        printf("[APP][cmd] buzzer off\r\n");
    }
}

static void app_handle_control_command(const char *topic, const char *json, size_t length)
{
    /* Supported MQTT commands (topics: led/motor/buzzer control topics)
     * LED:
     * - {"cmd":"led","id":"b","state":1}              // single: b/g/p or blue/green/pink
     * - {"cmd":"led","b":1,"g":0,"p":1}              // multi: short keys
     * - {"cmd":"led","led_b":1,"led_g":0,"led_p":1}  // multi: long keys
     * - {"cmd":"led","state":1}                      // all on/off
     * Motor (speed 0-100, forward/reverse/stop):
     * - {"cmd":"motor","dir":"forward","speed":60}
     * - {"cmd":"motor","dir":"reverse","speed":60}
     * - {"cmd":"motor","dir":"stop"}
     * Buzzer (frequency 1000-3000 Hz):
     * - {"cmd":"buzzer","state":1,"freq":2000}
     * - {"cmd":"buzzer","state":0}
     * Note: if cmd is omitted, the topic decides the device.
     */
    char buffer[ESP8266_AT_MAX_LINE_LENGTH];
    if (length >= sizeof(buffer))
    {
        length = sizeof(buffer) - 1U;
    }
    memcpy(buffer, json, length);
    buffer[length] = '\0';

    cJSON *root = cJSON_Parse(buffer);
    if (root == NULL)
    {
        printf("[APP][cmd] invalid json: %s\r\n", buffer);
        return;
    }

    const char *cmd = app_get_string_field(root, "cmd");
    if (cmd == NULL)
    {
        cmd = app_cmd_from_topic(topic);
    }

    if (cmd == NULL)
    {
        printf("[APP][cmd] unknown command: %s\r\n", buffer);
        cJSON_Delete(root);
        return;
    }

    if (strcmp(cmd, "led") == 0)
    {
        app_apply_led_command(root);
    }
    else if (strcmp(cmd, "motor") == 0)
    {
        app_apply_motor_command(root);
    }
    else if (strcmp(cmd, "buzzer") == 0)
    {
        app_apply_buzzer_command(root);
    }
    else
    {
        printf("[APP][cmd] unsupported cmd=%s\r\n", cmd);
    }

    cJSON_Delete(root);
}

static esp8266_at_status_t app_publish_aht30(float temperature_c, float humidity_pct)
{
    esp8266_at_status_t status = ESP8266_AT_STATUS_INVALID_ARGUMENT;
    cJSON *root = cJSON_CreateObject();
    if (root == NULL)
    {
        return status;
    }

    cJSON_AddStringToObject(root, "sensor", "aht30");
    const float temperature_rounded = app_round_1dp(temperature_c);
    const float humidity_rounded = app_round_1dp(humidity_pct);
    char temperature_raw[16];
    char humidity_raw[16];
    (void)snprintf(temperature_raw, sizeof(temperature_raw), "%.1f", (double)temperature_rounded);
    (void)snprintf(humidity_raw, sizeof(humidity_raw), "%.1f", (double)humidity_rounded);
    cJSON_AddRawToObject(root, "temperature", temperature_raw);
    cJSON_AddRawToObject(root, "humidity", humidity_raw);

    char *payload = cJSON_PrintUnformatted(root);
    if (payload != NULL)
    {
        const size_t payload_len = strlen(payload);
        status = esp8266_at_mqtt_publish_raw(0U,
                                             s_app.mqtt_sensor_topic,
                                             (const uint8_t *)payload,
                                             payload_len,
                                             APP_MQTT_QOS,
                                             false);
        if (status != ESP8266_AT_STATUS_OK)
        {
            printf("[APP][mqtt_pub] payload_len=%u payload=%s\r\n",
                   (unsigned int)payload_len,
                   payload);
        }
        free(payload);
    }

    cJSON_Delete(root);
    return status;
}

static void app_process_mqtt_event(const esp8266_at_event_t *event)
{
    if (event == NULL)
    {
        return;
    }

    if (strcmp(event->prefix, "+MQTTSUBRECV") == 0)
    {
        char topic[96];
        const char *data = NULL;
        size_t data_len = 0U;
        if (!app_parse_mqtt_subrecv(event->payload,
                                    topic,
                                    sizeof(topic),
                                    &data,
                                    &data_len))
        {
            printf("[APP][mqtt_rx] parse failed: %s\r\n", event->payload);
            return;
        }

        size_t available = strlen(data);
        if (data_len > available)
        {
            data_len = available;
        }

        char preview[80];
        size_t copy_len = data_len;
        if (copy_len >= sizeof(preview))
        {
            copy_len = sizeof(preview) - 1U;
        }
        memcpy(preview, data, copy_len);
        preview[copy_len] = '\0';
        printf("[APP][mqtt_rx] topic=%s len=%u payload=%s\r\n",
               topic,
               (unsigned int)data_len,
               preview);

        if (app_is_control_topic(topic))
        {
            app_handle_control_command(topic, data, data_len);
        }
    }
    else if (strcmp(event->prefix, "+MQTTDISCONNECTED") == 0)
    {
        s_app.mqtt_ready = false;
        s_app.wait_logged = false;
        printf("[APP][mqtt] disconnected\r\n");
        app_mqtt_schedule_reconnect();
        app_lcd_show_status();
    }
    else if (strcmp(event->prefix, "+MQTTCONNECTED") == 0)
    {
        s_app.mqtt_ready = true;
        s_app.wait_logged = false;
        printf("[APP][mqtt] connected\r\n");
        app_lcd_show_sensor_placeholder();
    }
    else if (event->type == ESP8266_AT_EVENT_TYPE_ERROR
             || event->type == ESP8266_AT_EVENT_TYPE_FAIL
             || event->type == ESP8266_AT_EVENT_TYPE_BUSY)
    {
        printf("[APP][event] %s (cmd=%s)\r\n",
               event->raw_line,
               esp8266_at_command_name(event->command));
    }
}

void App_Init(void)
{
    memset(&s_app, 0, sizeof(s_app));
    s_app.last_temp10 = INT16_MIN;
    s_app.last_hum10 = UINT16_MAX;

    app_build_mqtt_identifiers();
    printf("[APP] device id: %s\r\n", s_app.device_id);

    HAL_Delay(2000U);

    ST7789_Init();
    ST7789_Clear(BLACK);
    s_app.lcd_ready = true;
    app_lcd_show_status();

    motor_l9110_init();
    (void)buzzer_test_stop();
    (void)buzzer_test_beep(APP_BUZZER_DEFAULT_FREQ_HZ, APP_BUZZER_DUTY_PERCENT, 120U);
    HAL_Delay(80U);
    (void)buzzer_test_beep(APP_BUZZER_DEFAULT_FREQ_HZ, APP_BUZZER_DUTY_PERCENT, 120U);

    if (aht30_init() != HAL_OK)
    {
        printf("[APP] AHT30 init failed\r\n");
    }

    esp8266_at_status_t status = esp8266_at_init();
    app_log_status("esp_init", status);
    if (status != ESP8266_AT_STATUS_OK)
    {
        return;
    }

    status = esp8266_at_reset(2000U);
    app_log_status("esp_reset", status);
    if (status != ESP8266_AT_STATUS_OK)
    {
        return;
    }
    esp8266_at_clear_events();
    app_drain_events(200U);

    status = esp8266_at_disable_echo(true);
    app_log_status("echo_off", status);
    if (status != ESP8266_AT_STATUS_OK)
    {
        return;
    }
    app_drain_events(50U);

    const bool wifi_configured =
        !app_is_placeholder(APP_WIFI_SSID, "YOUR_WIFI_SSID")
        && !app_is_placeholder(APP_WIFI_PASSWORD, "YOUR_WIFI_PASSWORD");
    if (wifi_configured)
    {
        status = esp8266_at_set_wifi_mode(1U, false);
        app_log_status("wifi_mode", status);
        app_drain_events(50U);

        status = esp8266_at_connect_ap(APP_WIFI_SSID,
                                       APP_WIFI_PASSWORD,
                                       20000U,
                                       false);
        app_log_status("wifi_join", status);
        app_drain_events(500U);
        s_app.wifi_ready = (status == ESP8266_AT_STATUS_OK);
        app_lcd_show_status();
    }
    else
    {
        printf("[APP] wifi skipped (set APP_WIFI_SSID/PASSWORD)\r\n");
        app_lcd_show_status();
    }

    const bool mqtt_configured =
        !app_is_placeholder(APP_MQTT_BROKER, "YOUR_MQTT_BROKER");
    if (mqtt_configured)
    {
        status = esp8266_at_mqtt_user_config(0U,
                                             s_app.mqtt_client_id,
                                             APP_MQTT_USERNAME,
                                             APP_MQTT_PASSWORD);
        app_log_status("mqtt_usercfg", status);
        app_drain_events(50U);

        status = esp8266_at_mqtt_connect(0U,
                                         APP_MQTT_BROKER,
                                         APP_MQTT_PORT,
                                         120U,
                                         true);
        app_log_status("mqtt_conn", status);
        app_drain_events(200U);
        s_app.mqtt_ready = (status == ESP8266_AT_STATUS_OK);
        if (s_app.mqtt_ready)
        {
            app_lcd_show_sensor_placeholder();
        }
        else
        {
            app_lcd_show_status();
        }

        if (s_app.mqtt_ready)
        {
            app_mqtt_subscribe_all();
        }
    }
    else
    {
        printf("[APP] mqtt skipped (set APP_MQTT_* macros)\r\n");
        app_lcd_show_status();
    }

    printf("[APP] init done (wifi=%u mqtt=%u)\r\n",
           s_app.wifi_ready ? 1U : 0U,
           s_app.mqtt_ready ? 1U : 0U);
}

void App_Loop(void)
{
    if (!s_app.loop_started)
    {
        s_app.loop_started = true;
        printf("[APP] loop start\r\n");
    }

    esp8266_at_poll();

    esp8266_at_event_t event;
    while (esp8266_at_fetch_event(&event))
    {
        app_process_mqtt_event(&event);
    }

    if (!s_app.mqtt_ready)
    {
        if (!s_app.wait_logged)
        {
            s_app.wait_logged = true;
            printf("[APP] mqtt not ready, waiting...\r\n");
        }
        app_lcd_update_status();
        if (s_app.mqtt_reconnect_pending)
        {
            app_mqtt_try_reconnect();
        }
        return;
    }

    const uint32_t now = HAL_GetTick();
    if ((uint32_t)(now - s_app.last_publish_ms) < APP_PUBLISH_INTERVAL_MS)
    {
        return;
    }
    s_app.last_publish_ms = now;

    float temperature = 0.0f;
    float humidity = 0.0f;
    const HAL_StatusTypeDef status = aht30_read(&temperature, &humidity);
    if (status == HAL_OK)
    {
        const esp8266_at_status_t pub_status = app_publish_aht30(temperature, humidity);
        app_log_status("mqtt_pub", pub_status);

        int16_t temp10 = (int16_t)(temperature * 10.0f);
        uint16_t hum10 = (uint16_t)(humidity * 10.0f);
        printf("[APP] aht30 T=%d.%01dC RH=%d.%01d%%\r\n",
               temp10 / 10, abs(temp10 % 10),
               hum10 / 10, hum10 % 10);

        if (s_app.lcd_ready
            && (s_app.lcd_showing_status
                || temp10 != s_app.last_temp10
                || hum10 != s_app.last_hum10))
        {
            app_lcd_show_sensor(temp10, hum10);
        }
    }
    else if (status == HAL_BUSY)
    {
        printf("[APP] aht30 busy\r\n");
    }
    else
    {
        printf("[APP] aht30 read error=%d\r\n", (int)status);
    }
}
