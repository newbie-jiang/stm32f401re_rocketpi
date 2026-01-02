/**
 * @file driver_esp8266_at.h
 * @brief ESP8266 AT command driver public API.
 */
#ifndef DRIVER_ESP8266_AT_H
#define DRIVER_ESP8266_AT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define ESP8266_AT_MAX_LINE_LENGTH          2048U
#define ESP8266_AT_MAX_ARGUMENTS            6U
#define ESP8266_AT_MAX_ARGUMENT_LENGTH      64U
#define ESP8266_AT_EVENT_QUEUE_DEPTH        4U
#define ESP8266_AT_DEFAULT_TIMEOUT_MS       2000U
#define ESP8266_AT_RESET_TIMEOUT_MS         3000U
#define ESP8266_AT_PROMPT_CHAR              '>'

/* 一些固件版本并不支持 MQTTCONNCFG。用这个宏控制是否发送该命令。 */
#ifndef ESP8266_AT_USE_MQTTCONNCFG
#define ESP8266_AT_USE_MQTTCONNCFG          0
#endif


typedef enum
{
    ESP8266_AT_STATUS_OK = 0,
    ESP8266_AT_STATUS_NOT_INITIALISED,
    ESP8266_AT_STATUS_BUSY,
    ESP8266_AT_STATUS_INVALID_ARGUMENT,
    ESP8266_AT_STATUS_TIMEOUT,
    ESP8266_AT_STATUS_HAL_ERROR,
    ESP8266_AT_STATUS_BUFFER_OVERFLOW,
    ESP8266_AT_STATUS_ERROR_RESPONSE,
    ESP8266_AT_STATUS_FAIL_RESPONSE,
    ESP8266_AT_STATUS_BUSY_RESPONSE,
    ESP8266_AT_STATUS_PROMPT,
    ESP8266_AT_STATUS_UNEXPECTED_RESPONSE
} esp8266_at_status_t;

typedef enum
{
    ESP8266_AT_COMMAND_MODE_EXECUTE = 0,
    ESP8266_AT_COMMAND_MODE_QUERY,
    ESP8266_AT_COMMAND_MODE_TEST,
    ESP8266_AT_COMMAND_MODE_SET,
    ESP8266_AT_COMMAND_MODE_UNKNOWN
} esp8266_at_command_mode_t;

typedef enum
{
    ESP8266_AT_EVENT_TYPE_UNKNOWN = 0,
    ESP8266_AT_EVENT_TYPE_INFO,
    ESP8266_AT_EVENT_TYPE_INDICATION,
    ESP8266_AT_EVENT_TYPE_RESPONSE,
    ESP8266_AT_EVENT_TYPE_OK,
    ESP8266_AT_EVENT_TYPE_ERROR,
    ESP8266_AT_EVENT_TYPE_FAIL,
    ESP8266_AT_EVENT_TYPE_BUSY,
    ESP8266_AT_EVENT_TYPE_PROMPT,
    ESP8266_AT_EVENT_TYPE_SEND_OK,
    ESP8266_AT_EVENT_TYPE_SEND_FAIL
} esp8266_at_event_type_t;

#define ESP8266_AT_COMMAND_TABLE(MACRO) \
    MACRO(0,  AT,               "AT",                 0, 0, 0, 1) \
    MACRO(1,  ATE0,             "ATE0",               0, 0, 0, 1) \
    MACRO(2,  ATE1,             "ATE1",               0, 0, 0, 1) \
    MACRO(3,  RST,              "AT+RST",             0, 0, 0, 1) \
    MACRO(4,  GMR,              "AT+GMR",             0, 0, 0, 1) \
    MACRO(5,  CMD,              "AT+CMD",             0, 1, 0, 0) \
    MACRO(6,  GSLP,             "AT+GSLP",            0, 0, 1, 0) \
    MACRO(7,  SYSTIMESTAMP,     "AT+SYSTIMESTAMP",    0, 1, 1, 0) \
    MACRO(8,  SLEEP,            "AT+SLEEP",           0, 1, 1, 0) \
    MACRO(9,  RESTORE,          "AT+RESTORE",         0, 0, 0, 1) \
    MACRO(10, SYSRAM,           "AT+SYSRAM",          0, 1, 0, 0) \
    MACRO(11, SYSFLASH,         "AT+SYSFLASH",        0, 1, 1, 0) \
    MACRO(12, RFPOWER,          "AT+RFPOWER",         0, 1, 1, 0) \
    MACRO(13, SYSMSG,           "AT+SYSMSG",          0, 1, 1, 0) \
    MACRO(14, SYSROLLBACK,      "AT+SYSROLLBACK",     0, 0, 0, 1) \
    MACRO(15, SYSLOG,           "AT+SYSLOG",          0, 1, 1, 0) \
    MACRO(16, SYSSTORE,         "AT+SYSSTORE",        0, 1, 1, 0) \
    MACRO(17, SLEEPWKCFG,       "AT+SLEEPWKCFG",      0, 0, 1, 0) \
    MACRO(18, SYSREG,           "AT+SYSREG",          0, 0, 1, 0) \
    MACRO(19, USERRAM,          "AT+USERRAM",         0, 1, 1, 0) \
    MACRO(20, CWMODE,           "AT+CWMODE",          0, 1, 1, 0) \
    MACRO(21, CWSTATE,          "AT+CWSTATE",         0, 1, 0, 0) \
    MACRO(22, CWJAP,            "AT+CWJAP",           0, 1, 1, 1) \
    MACRO(23, CWRECONNCFG,      "AT+CWRECONNCFG",     0, 1, 1, 0) \
    MACRO(24, CWLAP,            "AT+CWLAP",           0, 0, 1, 1) \
    MACRO(25, CWLAPOPT,         "AT+CWLAPOPT",        0, 0, 1, 0) \
    MACRO(26, CWQAP,            "AT+CWQAP",           0, 0, 0, 1) \
    MACRO(27, CWSAP,            "AT+CWSAP",           0, 1, 1, 0) \
    MACRO(28, CWLIF,            "AT+CWLIF",           0, 0, 0, 1) \
    MACRO(29, CWQIF,            "AT+CWQIF",           0, 0, 1, 1) \
    MACRO(30, CWDHCP,           "AT+CWDHCP",          0, 1, 1, 0) \
    MACRO(31, CWDHCPS,          "AT+CWDHCPS",         0, 1, 1, 0) \
    MACRO(32, CWSTAPROTO,       "AT+CWSTAPROTO",      0, 1, 1, 0) \
    MACRO(33, CWAPPROTO,        "AT+CWAPPROTO",       0, 1, 1, 0) \
    MACRO(34, CWAUTOCONN,       "AT+CWAUTOCONN",      0, 1, 1, 0) \
    MACRO(35, CWHOSTNAME,       "AT+CWHOSTNAME",      0, 1, 1, 0) \
    MACRO(36, CWCOUNTRY,        "AT+CWCOUNTRY",       0, 1, 1, 0) \
    MACRO(37, CIFSR,            "AT+CIFSR",           0, 0, 0, 1) \
    MACRO(38, CIPSTAMAC,        "AT+CIPSTAMAC",       0, 1, 1, 0) \
    MACRO(39, CIPAPMAC,         "AT+CIPAPMAC",        0, 1, 1, 0) \
    MACRO(40, CIPSTA,           "AT+CIPSTA",          0, 1, 1, 0) \
    MACRO(41, CIPAP,            "AT+CIPAP",           0, 1, 1, 0) \
    MACRO(42, CIPV6,            "AT+CIPV6",           0, 1, 1, 0) \
    MACRO(43, CIPDNS,           "AT+CIPDNS",          0, 1, 1, 0) \
    MACRO(44, CIPDOMAIN,        "AT+CIPDOMAIN",       0, 0, 1, 0) \
    MACRO(45, CIPSTATUS,        "AT+CIPSTATUS",       0, 0, 0, 1) \
    MACRO(46, CIPSTART,         "AT+CIPSTART",        0, 0, 1, 0) \
    MACRO(47, CIPSTARTEX,       "AT+CIPSTARTEX",      0, 0, 1, 0) \
    MACRO(48, CIPTCPOPT,        "AT+CIPTCPOPT",       0, 1, 1, 0) \
    MACRO(49, CIPCLOSE,         "AT+CIPCLOSE",        0, 0, 1, 1) \
    MACRO(50, CIPSEND,          "AT+CIPSEND",         0, 0, 1, 1) \
    MACRO(51, CIPSENDEX,        "AT+CIPSENDEX",       0, 0, 1, 0) \
    MACRO(52, CIPDINFO,         "AT+CIPDINFO",        0, 1, 1, 0) \
    MACRO(53, CIPMUX,           "AT+CIPMUX",          0, 1, 1, 0) \
    MACRO(54, CIPRECVMODE,      "AT+CIPRECVMODE",     0, 1, 1, 0) \
    MACRO(55, CIPRECVDATA,      "AT+CIPRECVDATA",     0, 0, 1, 0) \
    MACRO(56, CIPRECVLEN,       "AT+CIPRECVLEN",      0, 1, 0, 0) \
    MACRO(57, CIPSERVER,        "AT+CIPSERVER",       0, 1, 1, 0) \
    MACRO(58, CIPSERVERMAXCONN, "AT+CIPSERVERMAXCONN",0, 1, 1, 0) \
    MACRO(59, CIPSSLCCONF,      "AT+CIPSSLCCONF",     0, 1, 1, 0) \
    MACRO(60, CIPSSLCCN,        "AT+CIPSSLCCN",       0, 1, 1, 0) \
    MACRO(61, CIPSSLCSNI,       "AT+CIPSSLCSNI",      0, 1, 1, 0) \
    MACRO(62, CIPSSLCALPN,      "AT+CIPSSLCALPN",     0, 1, 1, 0) \
    MACRO(63, CIPSSLCPSK,       "AT+CIPSSLCPSK",      0, 1, 1, 0) \
    MACRO(64, CIPMODE,          "AT+CIPMODE",         0, 1, 1, 0) \
    MACRO(65, CIPSTO,           "AT+CIPSTO",          0, 1, 1, 0) \
    MACRO(66, SAVETRANSLINK,    "AT+SAVETRANSLINK",   0, 0, 1, 0) \
    MACRO(67, CIPSNTPCFG,       "AT+CIPSNTPCFG",      0, 1, 1, 0) \
    MACRO(68, CIPSNTPTIME,      "AT+CIPSNTPTIME",     0, 1, 0, 0) \
    MACRO(69, CIPRECONNINTV,    "AT+CIPRECONNINTV",   0, 1, 1, 0) \
    MACRO(70, MQTTUSERCFG,      "AT+MQTTUSERCFG",     0, 0, 1, 0) \
    MACRO(71, MQTTCLIENTID,     "AT+MQTTCLIENTID",    0, 0, 1, 0) \
    MACRO(72, MQTTUSERNAME,     "AT+MQTTUSERNAME",    0, 0, 1, 0) \
    MACRO(73, MQTTPASSWORD,     "AT+MQTTPASSWORD",    0, 0, 1, 0) \
    MACRO(74, MQTTCONNCFG,      "AT+MQTTCONNCFG",     0, 0, 1, 0) \
    MACRO(75, MQTTCONN,         "AT+MQTTCONN",        0, 1, 1, 0) \
    MACRO(76, MQTTPUB,          "AT+MQTTPUB",         0, 0, 1, 0) \
    MACRO(77, MQTTPUBRAW,       "AT+MQTTPUBRAW",      0, 0, 1, 0) \
    MACRO(78, MQTTSUB,          "AT+MQTTSUB",         0, 1, 1, 0) \
    MACRO(79, MQTTUNSUB,        "AT+MQTTUNSUB",       0, 0, 1, 0) \
    MACRO(80, MQTTCLEAN,        "AT+MQTTCLEAN",       0, 0, 1, 0) \
    MACRO(81, MDNS,             "AT+MDNS",            0, 0, 1, 0) \
    MACRO(82, WPS,              "AT+WPS",             0, 0, 1, 0) \
    MACRO(83, CWSTARTSMART,     "AT+CWSTARTSMART",    0, 0, 1, 1) \
    MACRO(84, CWSTOPSMART,      "AT+CWSTOPSMART",     0, 0, 0, 1) \
    MACRO(85, PING,             "AT+PING",            0, 0, 1, 0) \
    MACRO(86, FACTPLCP,         "AT+FACTPLCP",        0, 0, 1, 0) \
    MACRO(87, LEDTEST,          "AT+LEDTEST",         0, 0, 1, 0) \
    MACRO(88, MCUTEST,          "AT+MCUTEST",         0, 0, 1, 0) \
    MACRO(89, UART,             "AT+UART",            0, 1, 1, 0) \
    MACRO(90, UART_CUR,         "AT+UART_CUR",        0, 1, 1, 0) \
    MACRO(91, UART_DEF,         "AT+UART_DEF",        0, 1, 1, 0)

typedef enum
{
#define ESP8266_AT_DECLARE_ENUM(ID, NAME, LITERAL, T, Q, S, E) ESP8266_AT_CMD_##NAME = (ID),
    ESP8266_AT_COMMAND_TABLE(ESP8266_AT_DECLARE_ENUM)
#undef ESP8266_AT_DECLARE_ENUM
    ESP8266_AT_CMD_COUNT
} esp8266_at_command_id_t;

typedef struct
{
    esp8266_at_command_id_t id;
    const char             *literal;
    bool                    supports_test;
    bool                    supports_query;
    bool                    supports_set;
    bool                    supports_execute;
} esp8266_at_command_def_t;

typedef struct
{
    char value[ESP8266_AT_MAX_ARGUMENT_LENGTH];
    bool quoted;
} esp8266_at_argument_t;

typedef struct
{
    esp8266_at_event_type_t   type;
    esp8266_at_command_id_t   command;
    esp8266_at_command_mode_t mode;
    char                      raw_line[ESP8266_AT_MAX_LINE_LENGTH];
    char                      prefix[32];
    char                      payload[ESP8266_AT_MAX_LINE_LENGTH];
    size_t                    argument_count;
    esp8266_at_argument_t     arguments[ESP8266_AT_MAX_ARGUMENTS];
} esp8266_at_event_t;

typedef struct
{
    char station_ip[48];
    char station_gateway[48];
    char station_netmask[48];
    char softap_ip[48];
    char softap_gateway[48];
    char softap_netmask[48];
} esp8266_at_ip_info_t;

const esp8266_at_command_def_t *esp8266_at_get_command_def(esp8266_at_command_id_t command);
const char *esp8266_at_command_name(esp8266_at_command_id_t command);
const char *esp8266_at_status_string(esp8266_at_status_t status);

esp8266_at_status_t esp8266_at_init(void);
void esp8266_at_deinit(void);
bool esp8266_at_is_ready(void);

esp8266_at_status_t esp8266_at_reset(uint32_t ready_timeout_ms);
esp8266_at_status_t esp8266_at_disable_echo(bool disable);
esp8266_at_status_t esp8266_at_send_command(esp8266_at_command_id_t command,
                                            esp8266_at_command_mode_t mode,
                                            const char *arguments,
                                            uint32_t timeout_ms,
                                            bool expect_prompt);
esp8266_at_status_t esp8266_at_wait_for_completion(uint32_t timeout_ms,
                                                   esp8266_at_event_t *terminal_event);
esp8266_at_status_t esp8266_at_send_raw_line(const char *line, uint32_t timeout_ms);
esp8266_at_status_t esp8266_at_send_data(const uint8_t *payload,
                                         size_t length,
                                         uint32_t timeout_ms);

esp8266_at_status_t esp8266_at_set_wifi_mode(uint8_t mode, bool store);
esp8266_at_status_t esp8266_at_connect_ap(const char *ssid,
                                          const char *password,
                                          uint32_t timeout_ms,
                                          bool save_config);
esp8266_at_status_t esp8266_at_disconnect_ap(void);
esp8266_at_status_t esp8266_at_query_ip_info(esp8266_at_ip_info_t *info);

esp8266_at_status_t esp8266_at_mqtt_user_config(uint8_t link_id,
                                                const char *client_id,
                                                const char *username,
                                                const char *password);
esp8266_at_status_t esp8266_at_mqtt_connect(uint8_t link_id,
                                            const char *host,
                                            uint16_t port,
                                            uint16_t keep_alive_s,
                                            bool clean_session);
esp8266_at_status_t esp8266_at_mqtt_publish(uint8_t link_id,
                                            const char *topic,
                                            const char *payload,
                                            uint8_t qos,
                                            bool retain);
esp8266_at_status_t esp8266_at_mqtt_publish_raw(uint8_t link_id,
                                                const char *topic,
                                                const uint8_t *payload,
                                                size_t length,
                                                uint8_t qos,
                                                bool retain);
esp8266_at_status_t esp8266_at_mqtt_disconnect(uint8_t link_id);

bool esp8266_at_fetch_event(esp8266_at_event_t *event);
size_t esp8266_at_pending_event_count(void);
void esp8266_at_poll(void);
void esp8266_at_receive_bytes(const uint8_t *data, size_t length);
void esp8266_at_debug_print_events(bool keep_events);
void esp8266_at_clear_events(void);

#ifdef __cplusplus
}
#endif

#endif /* DRIVER_ESP8266_AT_H */
											
