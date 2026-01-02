/**
 * @file driver_esp8266_at_test.c
 * @brief Integration smoke tests for the ESP8266 AT driver.
 */
#include "driver_esp8266_at_test.h"

#include <stdio.h>
#include <string.h>

#include "stm32f4xx_hal.h"

#include "driver_esp8266_at.h"

#ifndef ESP8266_AT_TEST_WIFI_SSID
#define ESP8266_AT_TEST_WIFI_SSID        "ZTE-45c476"
#endif
#ifndef ESP8266_AT_TEST_WIFI_PASSWORD
#define ESP8266_AT_TEST_WIFI_PASSWORD    "88888888"
#endif

#ifndef ESP8266_AT_TEST_MQTT_BROKER
#define ESP8266_AT_TEST_MQTT_BROKER      "broker.emqx.io"
#endif
#ifndef ESP8266_AT_TEST_MQTT_PORT
#define ESP8266_AT_TEST_MQTT_PORT        1883
#endif
#ifndef ESP8266_AT_TEST_MQTT_CLIENT_ID
#define ESP8266_AT_TEST_MQTT_CLIENT_ID   "esp8266-emqx"
#endif
#ifndef ESP8266_AT_TEST_MQTT_USERNAME
#define ESP8266_AT_TEST_MQTT_USERNAME    ""
#endif
#ifndef ESP8266_AT_TEST_MQTT_PASSWORD
#define ESP8266_AT_TEST_MQTT_PASSWORD    ""
#endif
#ifndef ESP8266_AT_TEST_MQTT_TOPIC
#define ESP8266_AT_TEST_MQTT_TOPIC       "/test/esp8266"
#endif
#ifndef ESP8266_AT_TEST_MQTT_MESSAGE
#define ESP8266_AT_TEST_MQTT_MESSAGE     "hello from rocketpi"
#endif

static void esp8266_at_test_log_status(const char *label, esp8266_at_status_t status)
{
    printf("[ESP8266][%s] %s\r\n", label, esp8266_at_status_string(status));
}

static const char *esp8266_at_test_event_type_string(esp8266_at_event_type_t type)
{
    switch (type)
    {
        case ESP8266_AT_EVENT_TYPE_INFO:        return "info";
        case ESP8266_AT_EVENT_TYPE_INDICATION:  return "ind";
        case ESP8266_AT_EVENT_TYPE_RESPONSE:    return "resp";
        case ESP8266_AT_EVENT_TYPE_OK:          return "ok";
        case ESP8266_AT_EVENT_TYPE_ERROR:       return "error";
        case ESP8266_AT_EVENT_TYPE_FAIL:        return "fail";
        case ESP8266_AT_EVENT_TYPE_BUSY:        return "busy";
        case ESP8266_AT_EVENT_TYPE_PROMPT:      return "prompt";
        case ESP8266_AT_EVENT_TYPE_SEND_OK:     return "send_ok";
        case ESP8266_AT_EVENT_TYPE_SEND_FAIL:   return "send_fail";
        default:                                return "unknown";
    }
}

static void esp8266_at_test_print_event(const esp8266_at_event_t *event)
{
    if (event == NULL)
    {
        return;
    }

    if (event->command == ESP8266_AT_CMD_CMD &&
        event->type != ESP8266_AT_EVENT_TYPE_OK)
    {
        return;
    }

    printf("[ESP8266][event][%s] %s",
           esp8266_at_test_event_type_string(event->type),
           event->raw_line);

    if (event->command < ESP8266_AT_CMD_COUNT)
    {
        printf(" (cmd=%s)", esp8266_at_command_name(event->command));
    }
    printf("\r\n");

    if (event->argument_count > 0U)
    {
        for (size_t i = 0U; i < event->argument_count; ++i)
        {
            printf("        arg%u%s: %s\r\n",
                   (unsigned int)i,
                   event->arguments[i].quoted ? "(q)" : "",
                   event->arguments[i].value);
        }
    }
}

static void esp8266_at_test_drain_events(uint32_t delay_ms)
{
    if (delay_ms > 0U)
    {
        HAL_Delay(delay_ms);
    }

    esp8266_at_test_poll();
}

static bool esp8266_at_test_is_placeholder(const char *value,
                                           const char *placeholder)
{
    if (value == NULL || placeholder == NULL)
    {
        return true;
    }
    return (strcmp(value, placeholder) == 0);
}


void esp8266_at_test_run(void)
{
    printf("\r\n=== ESP8266 AT smoke test ===\r\n");
    esp8266_at_status_t status = esp8266_at_init();
    esp8266_at_test_log_status("init", status);
    if (status != ESP8266_AT_STATUS_OK)
    {
        return;
    }

    status = esp8266_at_reset(2000U);
    esp8266_at_test_log_status("RST", status);
    if (status != ESP8266_AT_STATUS_OK)
    {
        esp8266_at_debug_print_events(true);
        esp8266_at_deinit();
        printf("=== ESP8266 AT smoke test done ===\r\n");
        return;
    }

    esp8266_at_clear_events();
    esp8266_at_test_drain_events(200U);

    status = esp8266_at_disable_echo(true);
    esp8266_at_test_log_status("echo_off", status);
    if (status != ESP8266_AT_STATUS_OK)
    {
        esp8266_at_debug_print_events(true);
        esp8266_at_deinit();
        printf("=== ESP8266 AT smoke test done ===\r\n");
        return;
    }
    esp8266_at_test_drain_events(50U);

    status = esp8266_at_send_command(ESP8266_AT_CMD_AT,
                                     ESP8266_AT_COMMAND_MODE_EXECUTE,
                                     NULL,
                                     ESP8266_AT_DEFAULT_TIMEOUT_MS,
                                     false);
    esp8266_at_test_log_status("AT", status);
    if (status != ESP8266_AT_STATUS_OK)
    {
        esp8266_at_debug_print_events(true);
        esp8266_at_deinit();
        printf("=== ESP8266 AT smoke test done ===\r\n");
        return;
    }
    esp8266_at_test_drain_events(50U);
    status = esp8266_at_send_command(ESP8266_AT_CMD_GMR,
                                     ESP8266_AT_COMMAND_MODE_EXECUTE,
                                     NULL,
                                     ESP8266_AT_DEFAULT_TIMEOUT_MS,
                                     false);
    esp8266_at_test_log_status("AT+GMR", status);
    esp8266_at_test_drain_events(50U);

    status = esp8266_at_send_command(ESP8266_AT_CMD_CMD,
                                     ESP8266_AT_COMMAND_MODE_QUERY,
                                     NULL,
                                     ESP8266_AT_DEFAULT_TIMEOUT_MS * 2U,
                                     false);
    esp8266_at_test_log_status("AT+CMD?", status);
    esp8266_at_test_drain_events(100U);

    const bool wifi_configured =
        !esp8266_at_test_is_placeholder(ESP8266_AT_TEST_WIFI_SSID, "YOUR_WIFI_SSID")
        && !esp8266_at_test_is_placeholder(ESP8266_AT_TEST_WIFI_PASSWORD, "YOUR_WIFI_PASSWORD");
    if (wifi_configured)
    {
        status = esp8266_at_set_wifi_mode(1U, false);
        esp8266_at_test_log_status("CWMODE", status);
        esp8266_at_test_drain_events(50U);

        status = esp8266_at_connect_ap(ESP8266_AT_TEST_WIFI_SSID,
                                       ESP8266_AT_TEST_WIFI_PASSWORD,
                                       20000U,
                                       false);
        esp8266_at_test_log_status("CWJAP", status);
        esp8266_at_test_drain_events(500U);

        esp8266_at_ip_info_t ip_info;
        status = esp8266_at_query_ip_info(&ip_info);
        esp8266_at_test_log_status("CIPSTA?", status);
        if (status == ESP8266_AT_STATUS_OK)
        {
            printf("        STA ip=%s gateway=%s netmask=%s\r\n",
                   ip_info.station_ip,
                   ip_info.station_gateway,
                   ip_info.station_netmask);
            printf("        AP ip=%s gateway=%s netmask=%s\r\n",
                   ip_info.softap_ip,
                   ip_info.softap_gateway,
                   ip_info.softap_netmask);
        }
        esp8266_at_test_drain_events(100U);
    }
    else
    {
        printf("[ESP8266][wifi] skipped (define ESP8266_AT_TEST_WIFI_SSID/PASSWORD)\r\n");
    }

    const bool mqtt_configured =
        !esp8266_at_test_is_placeholder(ESP8266_AT_TEST_MQTT_BROKER, "YOUR_MQTT_BROKER")
        && !esp8266_at_test_is_placeholder(ESP8266_AT_TEST_MQTT_CLIENT_ID, "YOUR_MQTT_CLIENT_ID");
    if (mqtt_configured)
    {
        const char *mqtt_host = ESP8266_AT_TEST_MQTT_BROKER;

        status = esp8266_at_mqtt_user_config(0U,
                                             ESP8266_AT_TEST_MQTT_CLIENT_ID,
                                             ESP8266_AT_TEST_MQTT_USERNAME,
                                             ESP8266_AT_TEST_MQTT_PASSWORD);
        esp8266_at_test_log_status("MQTTUSERCFG", status);
        esp8266_at_test_drain_events(50U);

        status = esp8266_at_mqtt_connect(0U,
                                         mqtt_host,
                                         ESP8266_AT_TEST_MQTT_PORT,
                                         120U,
                                         true);
        esp8266_at_test_log_status("MQTTCONN", status);
        esp8266_at_test_drain_events(250U);

        if (status == ESP8266_AT_STATUS_OK)
        {
            char sub_args[ESP8266_AT_MAX_LINE_LENGTH];
            (void)snprintf(sub_args,
                           sizeof(sub_args),
                           "%u,\"%s\",1",
                           0U,
                           ESP8266_AT_TEST_MQTT_TOPIC);

            status = esp8266_at_send_command(ESP8266_AT_CMD_MQTTSUB,
                                             ESP8266_AT_COMMAND_MODE_SET,
                                             sub_args,
                                             ESP8266_AT_DEFAULT_TIMEOUT_MS,
                                             false);
            esp8266_at_test_log_status("MQTTSUB", status);
            esp8266_at_test_drain_events(150U);

            status = esp8266_at_mqtt_publish(0U,
                                             ESP8266_AT_TEST_MQTT_TOPIC,
                                             ESP8266_AT_TEST_MQTT_MESSAGE,
                                             1U,
                                             false);
            esp8266_at_test_log_status("MQTTPUB", status);
            esp8266_at_test_drain_events(150U);

            char unsub_args[ESP8266_AT_MAX_LINE_LENGTH];
            (void)snprintf(unsub_args,
                           sizeof(unsub_args),
                           "%u,\"%s\"",
                           0U,
                           ESP8266_AT_TEST_MQTT_TOPIC);

            status = esp8266_at_send_command(ESP8266_AT_CMD_MQTTUNSUB,
                                             ESP8266_AT_COMMAND_MODE_SET,
                                             unsub_args,
                                             ESP8266_AT_DEFAULT_TIMEOUT_MS,
                                             false);
            esp8266_at_test_log_status("MQTTUNSUB", status);
            esp8266_at_test_drain_events(150U);

            status = esp8266_at_mqtt_disconnect(0U);
            esp8266_at_test_log_status("MQTTCLEAN", status);
            esp8266_at_test_drain_events(150U);
        }
    }
    else
    {
        printf("[ESP8266][mqtt] skipped (define ESP8266_AT_TEST_MQTT_* macros)\r\n");
    }

    if (wifi_configured)
    {
        status = esp8266_at_disconnect_ap();
        esp8266_at_test_log_status("CWQAP", status);
        esp8266_at_test_drain_events(200U);
    }

    printf("=== ESP8266 AT smoke test done ===\r\n");
}

void esp8266_at_test_poll(void)
{
    esp8266_at_poll();

    esp8266_at_event_t event;
    while (esp8266_at_fetch_event(&event))
    {
        esp8266_at_test_print_event(&event);
    }
}
