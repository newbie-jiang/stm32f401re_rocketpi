#include "driver_esp8266_at.h"

#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "driver_esp8266_at_interface.h"


/**
 * @file driver_esp8266_at.c
 * @brief Platform-neutral ESP8266 AT command driver.
 */

typedef enum
{
    ESP8266_AT_PHASE_IDLE = 0,
    ESP8266_AT_PHASE_WAITING_PROMPT,
    ESP8266_AT_PHASE_READY_FOR_DATA,
    ESP8266_AT_PHASE_WAITING_FINAL
} esp8266_at_phase_t;


typedef struct
{
    esp8266_at_event_type_t   type;
    esp8266_at_command_id_t   command;
    esp8266_at_command_mode_t mode;
    char                      raw_line[ESP8266_AT_MAX_LINE_LENGTH];
} esp8266_at_event_compact_t;

typedef struct
{
    bool                    initialised;
    bool                    awaiting_reply;
    esp8266_at_phase_t      phase;
    bool                    expect_prompt;
    esp8266_at_command_id_t pending_command;
    esp8266_at_command_mode_t pending_mode;
    uint32_t                command_timeout_ms;
    uint32_t                command_start_tick;
    esp8266_at_status_t     last_status;
    esp8266_at_event_t      last_terminal_event;
    bool                    last_terminal_event_valid;
    struct
    {
        char   buffer[ESP8266_AT_MAX_LINE_LENGTH];
        size_t length;
    } line;
    struct
    {
        esp8266_at_event_compact_t events[ESP8266_AT_EVENT_QUEUE_DEPTH];
        uint8_t            head;
        uint8_t            tail;
        uint8_t            count;
    } queue;
} esp8266_at_core_t;

static esp8266_at_core_t s_core;

static const esp8266_at_command_def_t s_command_table[ESP8266_AT_CMD_COUNT] =
{
#define ESP8266_AT_BUILD_DEF(ID, NAME, LITERAL, T, Q, S, E) \
    [ID] = { ESP8266_AT_CMD_##NAME, LITERAL, (T) != 0, (Q) != 0, (S) != 0, (E) != 0 },
    ESP8266_AT_COMMAND_TABLE(ESP8266_AT_BUILD_DEF)
#undef ESP8266_AT_BUILD_DEF
};

static void esp8266_at_core_reset_line(esp8266_at_core_t *ctx);
static void esp8266_at_core_reset_state(esp8266_at_core_t *ctx);
static void esp8266_at_core_process_byte(esp8266_at_core_t *ctx, uint8_t byte);
static void esp8266_at_core_handle_line(esp8266_at_core_t *ctx, const char *line);
static void esp8266_at_core_process_prompt(esp8266_at_core_t *ctx);
static bool esp8266_at_parse_line_to_event(const char *line, esp8266_at_event_t *event);
static void esp8266_at_compact_from_event(esp8266_at_event_compact_t *dst, const esp8266_at_event_t *src);
static bool esp8266_at_event_from_compact(const esp8266_at_event_compact_t *src, esp8266_at_event_t *dst);
static void esp8266_at_queue_restore_compact(esp8266_at_core_t *ctx,
                                             const esp8266_at_event_compact_t *events,
                                             size_t count);
static bool esp8266_at_queue_push(esp8266_at_core_t *ctx, const esp8266_at_event_t *event);
static bool esp8266_at_queue_pop(esp8266_at_core_t *ctx, esp8266_at_event_t *event);
static void esp8266_at_queue_restore(esp8266_at_core_t *ctx,
                                     const esp8266_at_event_t *events,
                                     size_t count);
static uint32_t esp8266_at_tick_elapsed(uint32_t start, uint32_t now);
static size_t esp8266_at_strnlen(const char *str, size_t max_len);
static int esp8266_at_strcasecmp(const char *lhs, const char *rhs);
static int esp8266_at_strncasecmp(const char *lhs, const char *rhs, size_t length);
static void esp8266_at_trim(char *str);
static void esp8266_at_copy_lower(char *dest, size_t dest_size, const char *source);
static void esp8266_at_parse_response_payload(esp8266_at_event_t *event);
static void esp8266_at_parse_ip_descriptor(const esp8266_at_event_t *event,
                                           bool station,
                                           esp8266_at_ip_info_t *info);
static esp8266_at_command_id_t esp8266_at_match_command_by_prefix(const char *prefix);
static size_t esp8266_at_escape_field_length(const char *input);
static void esp8266_at_escape_field(const char *input, char *output, size_t output_size);
static esp8266_at_status_t esp8266_at_send_and_wait_internal(esp8266_at_command_id_t command,
                                                             esp8266_at_command_mode_t mode,
                                                             const char *arguments,
                                                             uint32_t timeout_ms,
                                                             bool expect_prompt,
                                                             esp8266_at_event_t *terminal_event);
static bool esp8266_at_pop_event_by_prefix(const char *prefix, esp8266_at_event_t *out_event);
static const char *esp8266_at_event_type_name(esp8266_at_event_type_t type);
static esp8266_at_event_compact_t s_reset_event_stash[ESP8266_AT_EVENT_QUEUE_DEPTH];

const esp8266_at_command_def_t *esp8266_at_get_command_def(esp8266_at_command_id_t command)
{
    if (command >= ESP8266_AT_CMD_COUNT)
    {
        return NULL;
    }
    return &s_command_table[command];
}

const char *esp8266_at_command_name(esp8266_at_command_id_t command)
{
    const esp8266_at_command_def_t *def = esp8266_at_get_command_def(command);
    return (def != NULL) ? def->literal : "UNKNOWN";
}

const char *esp8266_at_status_string(esp8266_at_status_t status)
{
    switch (status)
    {
        case ESP8266_AT_STATUS_OK:                  return "OK";
        case ESP8266_AT_STATUS_NOT_INITIALISED:     return "not_initialised";
        case ESP8266_AT_STATUS_BUSY:                return "busy";
        case ESP8266_AT_STATUS_INVALID_ARGUMENT:    return "invalid_argument";
        case ESP8266_AT_STATUS_TIMEOUT:             return "timeout";
        case ESP8266_AT_STATUS_HAL_ERROR:           return "hal_error";
        case ESP8266_AT_STATUS_BUFFER_OVERFLOW:     return "buffer_overflow";
        case ESP8266_AT_STATUS_ERROR_RESPONSE:      return "error_response";
        case ESP8266_AT_STATUS_FAIL_RESPONSE:       return "fail_response";
        case ESP8266_AT_STATUS_BUSY_RESPONSE:       return "busy_response";
        case ESP8266_AT_STATUS_PROMPT:              return "prompt";
        case ESP8266_AT_STATUS_UNEXPECTED_RESPONSE: return "unexpected_response";
        default:                                    return "unknown";
    }
}

esp8266_at_status_t esp8266_at_init(void)
{
    if (s_core.initialised)
    {
        return ESP8266_AT_STATUS_OK;
    }

    memset(&s_core, 0, sizeof(s_core));
    esp8266_at_core_reset_state(&s_core);

    const esp8266_at_status_t status = esp8266_at_interface_hw_init();
    if (status != ESP8266_AT_STATUS_OK)
    {
        memset(&s_core, 0, sizeof(s_core));
        return status;
    }

    s_core.initialised = true;
    return ESP8266_AT_STATUS_OK;
}

void esp8266_at_deinit(void)
{
    memset(&s_core, 0, sizeof(s_core));
}

bool esp8266_at_is_ready(void)
{
    return s_core.initialised && s_core.phase == ESP8266_AT_PHASE_IDLE && !s_core.awaiting_reply;
}

esp8266_at_status_t esp8266_at_reset(uint32_t ready_timeout_ms)
{
    if (!s_core.initialised)
    {
        return ESP8266_AT_STATUS_NOT_INITIALISED;
    }

    const uint32_t timeout = (ready_timeout_ms != 0U)
                           ? ready_timeout_ms
                           : ESP8266_AT_RESET_TIMEOUT_MS;

    esp8266_at_status_t status =
        esp8266_at_send_and_wait_internal(ESP8266_AT_CMD_RST,
                                          ESP8266_AT_COMMAND_MODE_EXECUTE,
                                          NULL,
                                          timeout,
                                          false,
                                          NULL);
    if (status != ESP8266_AT_STATUS_OK)
    {
        return status;
    }

    const uint32_t start = esp8266_at_interface_hw_get_tick();
    esp8266_at_event_t event;
    size_t stash_count = 0U;
    bool ready_received = false;

    while (esp8266_at_tick_elapsed(start, esp8266_at_interface_hw_get_tick()) < timeout)
    {
        esp8266_at_poll();
        if (!esp8266_at_fetch_event(&event))
        {
            continue;
        }

        if (esp8266_at_strcasecmp(event.payload, "ready") == 0
            || esp8266_at_strcasecmp(event.raw_line, "ready") == 0)
        {
            ready_received = true;
            break;
        }

        if (stash_count < ESP8266_AT_EVENT_QUEUE_DEPTH)
        {
            esp8266_at_compact_from_event(&s_reset_event_stash[stash_count], &event);
            stash_count++;
        }
    }

    if (stash_count > 0U)
    {
        esp8266_at_queue_restore_compact(&s_core, s_reset_event_stash, stash_count);
    }

    if (!ready_received)
    {
        s_core.awaiting_reply = false;
        s_core.phase          = ESP8266_AT_PHASE_IDLE;
        s_core.last_status    = ESP8266_AT_STATUS_OK;
    }

    return ESP8266_AT_STATUS_OK;
}

esp8266_at_status_t esp8266_at_disable_echo(bool disable)
{
    const esp8266_at_command_id_t command =
        disable ? ESP8266_AT_CMD_ATE0 : ESP8266_AT_CMD_ATE1;
    return esp8266_at_send_and_wait_internal(command,
                                             ESP8266_AT_COMMAND_MODE_EXECUTE,
                                             NULL,
                                             ESP8266_AT_DEFAULT_TIMEOUT_MS,
                                             false,
                                             NULL);
}

esp8266_at_status_t esp8266_at_send_command(esp8266_at_command_id_t command,
                                            esp8266_at_command_mode_t mode,
                                            const char *arguments,
                                            uint32_t timeout_ms,
                                            bool expect_prompt)
{
    return esp8266_at_send_and_wait_internal(command,
                                             mode,
                                             arguments,
                                             timeout_ms,
                                             expect_prompt,
                                             NULL);
}

esp8266_at_status_t esp8266_at_wait_for_completion(uint32_t timeout_ms,
                                                   esp8266_at_event_t *terminal_event)
{
    if (!s_core.initialised)
    {
        return ESP8266_AT_STATUS_NOT_INITIALISED;
    }

    const uint32_t timeout =
        (timeout_ms != 0U) ? timeout_ms : s_core.command_timeout_ms;
    const uint32_t start = esp8266_at_interface_hw_get_tick();

    while (true)
    {
        esp8266_at_poll();

        if (!s_core.awaiting_reply)
        {
            if (s_core.phase == ESP8266_AT_PHASE_READY_FOR_DATA
                && s_core.last_status == ESP8266_AT_STATUS_PROMPT)
            {
                break;
            }
            if (s_core.phase == ESP8266_AT_PHASE_IDLE)
            {
                break;
            }
        }

        if (esp8266_at_tick_elapsed(start, esp8266_at_interface_hw_get_tick()) > timeout)
        {
            s_core.awaiting_reply = false;
            s_core.phase          = ESP8266_AT_PHASE_IDLE;
            s_core.last_status    = ESP8266_AT_STATUS_TIMEOUT;
            break;
        }
    }

    if (terminal_event != NULL && s_core.last_terminal_event_valid)
    {
        *terminal_event = s_core.last_terminal_event;
    }

    return s_core.last_status;
}

esp8266_at_status_t esp8266_at_send_raw_line(const char *line, uint32_t timeout_ms)
{
    if (!s_core.initialised)
    {
        return ESP8266_AT_STATUS_NOT_INITIALISED;
    }
    if (line == NULL)
    {
        return ESP8266_AT_STATUS_INVALID_ARGUMENT;
    }

    char buffer[ESP8266_AT_MAX_LINE_LENGTH];
    const size_t length = esp8266_at_strnlen(line, sizeof(buffer) - 3U);
    if (length == 0U)
    {
        return ESP8266_AT_STATUS_INVALID_ARGUMENT;
    }

    memcpy(buffer, line, length);
    buffer[length]     = '\r';
    buffer[length + 1] = '\n';
    buffer[length + 2] = '\0';

    const esp8266_at_status_t status =
        esp8266_at_interface_hw_send((const uint8_t *)buffer, length + 2U, timeout_ms);
    return status;
}

esp8266_at_status_t esp8266_at_send_data(const uint8_t *payload,
                                         size_t length,
                                         uint32_t timeout_ms)
{
    if (!s_core.initialised)
    {
        return ESP8266_AT_STATUS_NOT_INITIALISED;
    }
    if (payload == NULL || length == 0U)
    {
        return ESP8266_AT_STATUS_INVALID_ARGUMENT;
    }
    if (s_core.phase != ESP8266_AT_PHASE_READY_FOR_DATA)
    {
        return ESP8266_AT_STATUS_UNEXPECTED_RESPONSE;
    }

    esp8266_at_status_t status =
        esp8266_at_interface_hw_send(payload, length, timeout_ms);
    if (status != ESP8266_AT_STATUS_OK)
    {
        return status;
    }

    s_core.awaiting_reply            = true;
    s_core.phase                     = ESP8266_AT_PHASE_WAITING_FINAL;
    s_core.command_timeout_ms        = (timeout_ms != 0U) ? timeout_ms : ESP8266_AT_DEFAULT_TIMEOUT_MS;
    s_core.command_start_tick        = esp8266_at_interface_hw_get_tick();
    s_core.last_status               = ESP8266_AT_STATUS_TIMEOUT;
    s_core.last_terminal_event_valid = false;

    return ESP8266_AT_STATUS_OK;
}

esp8266_at_status_t esp8266_at_set_wifi_mode(uint8_t mode, bool store)
{
    if (!s_core.initialised)
    {
        return ESP8266_AT_STATUS_NOT_INITIALISED;
    }
    if (mode > 3U)
    {
        return ESP8266_AT_STATUS_INVALID_ARGUMENT;
    }

    char args[16];
    if (store)
    {
        (void)snprintf(args, sizeof(args), "%u,1", (unsigned int)mode);
    }
    else
    {
        (void)snprintf(args, sizeof(args), "%u", (unsigned int)mode);
    }

    esp8266_at_status_t status =
        esp8266_at_send_and_wait_internal(ESP8266_AT_CMD_CWMODE,
                                          ESP8266_AT_COMMAND_MODE_SET,
                                          args,
                                          ESP8266_AT_DEFAULT_TIMEOUT_MS,
                                          false,
                                          NULL);
    if (status != ESP8266_AT_STATUS_OK)
    {
        return status;
    }

    if (store)
    {
        status = esp8266_at_send_and_wait_internal(ESP8266_AT_CMD_SYSSTORE,
                                                   ESP8266_AT_COMMAND_MODE_SET,
                                                   "1",
                                                   ESP8266_AT_DEFAULT_TIMEOUT_MS,
                                                   false,
                                                   NULL);
    }

    return status;
}

esp8266_at_status_t esp8266_at_connect_ap(const char *ssid,
                                          const char *password,
                                          uint32_t timeout_ms,
                                          bool save_config)
{
    if (!s_core.initialised)
    {
        return ESP8266_AT_STATUS_NOT_INITIALISED;
    }
    if (ssid == NULL || password == NULL)
    {
        return ESP8266_AT_STATUS_INVALID_ARGUMENT;
    }

    char ssid_field[96];
    char password_field[96];
    esp8266_at_escape_field(ssid, ssid_field, sizeof(ssid_field));
    esp8266_at_escape_field(password, password_field, sizeof(password_field));

    char args[ESP8266_AT_MAX_LINE_LENGTH];
    (void)snprintf(args,
                   sizeof(args),
                   "\"%s\",\"%s\"",
                   ssid_field,
                   password_field);

    esp8266_at_status_t status =
        esp8266_at_send_and_wait_internal(ESP8266_AT_CMD_CWJAP,
                                          ESP8266_AT_COMMAND_MODE_SET,
                                          args,
                                          (timeout_ms != 0U) ? timeout_ms : (ESP8266_AT_DEFAULT_TIMEOUT_MS * 5U),
                                          false,
                                          NULL);
    if (status != ESP8266_AT_STATUS_OK)
    {
        return status;
    }

    if (save_config)
    {
        status = esp8266_at_send_and_wait_internal(ESP8266_AT_CMD_SYSSTORE,
                                                   ESP8266_AT_COMMAND_MODE_SET,
                                                   "1",
                                                   ESP8266_AT_DEFAULT_TIMEOUT_MS,
                                                   false,
                                                   NULL);
    }

    return status;
}

esp8266_at_status_t esp8266_at_disconnect_ap(void)
{
    return esp8266_at_send_and_wait_internal(ESP8266_AT_CMD_CWQAP,
                                             ESP8266_AT_COMMAND_MODE_EXECUTE,
                                             NULL,
                                             ESP8266_AT_DEFAULT_TIMEOUT_MS,
                                             false,
                                             NULL);
}

esp8266_at_status_t esp8266_at_query_ip_info(esp8266_at_ip_info_t *info)
{
    if (!s_core.initialised)
    {
        return ESP8266_AT_STATUS_NOT_INITIALISED;
    }
    if (info == NULL)
    {
        return ESP8266_AT_STATUS_INVALID_ARGUMENT;
    }

    memset(info, 0, sizeof(*info));

    esp8266_at_event_t event;
    esp8266_at_status_t status =
        esp8266_at_send_and_wait_internal(ESP8266_AT_CMD_CIPSTA,
                                          ESP8266_AT_COMMAND_MODE_QUERY,
                                          NULL,
                                          ESP8266_AT_DEFAULT_TIMEOUT_MS,
                                          false,
                                          NULL);
    if (status != ESP8266_AT_STATUS_OK)
    {
        return status;
    }

    while (esp8266_at_pop_event_by_prefix("+CIPSTA", &event))
    {
        esp8266_at_parse_ip_descriptor(&event, true, info);
    }

    status = esp8266_at_send_and_wait_internal(ESP8266_AT_CMD_CIPAP,
                                               ESP8266_AT_COMMAND_MODE_QUERY,
                                               NULL,
                                               ESP8266_AT_DEFAULT_TIMEOUT_MS,
                                               false,
                                               NULL);
    if (status != ESP8266_AT_STATUS_OK)
    {
        return status;
    }

    while (esp8266_at_pop_event_by_prefix("+CIPAP", &event))
    {
        esp8266_at_parse_ip_descriptor(&event, false, info);
    }

    return ESP8266_AT_STATUS_OK;
}

esp8266_at_status_t esp8266_at_mqtt_user_config(uint8_t link_id,
                                                const char *client_id,
                                                const char *username,
                                                const char *password)
{
    if (!s_core.initialised)
    {
        return ESP8266_AT_STATUS_NOT_INITIALISED;
    }

    char client_field[96];
    char username_field[96];
    char password_field[96];

    esp8266_at_escape_field((client_id != NULL) ? client_id : "",
                            client_field,
                            sizeof(client_field));
    esp8266_at_escape_field((username != NULL) ? username : "",
                            username_field,
                            sizeof(username_field));
    esp8266_at_escape_field((password != NULL) ? password : "",
                            password_field,
                            sizeof(password_field));

  char args[ESP8266_AT_MAX_LINE_LENGTH];
  (void)snprintf(args,
                 sizeof(args),
                 "%u,1,\"%s\",\"%s\",\"%s\",0,0,\"\"",
                 (unsigned int)link_id,
                 client_field,
                 username_field,
                 password_field);


  printf("[ESP8266][cmd] AT+MQTTUSERCFG=%s\r\n", args);


  return esp8266_at_send_and_wait_internal(ESP8266_AT_CMD_MQTTUSERCFG,
                                           ESP8266_AT_COMMAND_MODE_SET,
                                           args,
                                           ESP8266_AT_DEFAULT_TIMEOUT_MS,
                                           false,
                                             NULL);
}

esp8266_at_status_t esp8266_at_mqtt_connect(uint8_t link_id,
                                            const char *host,
                                            uint16_t port,
                                            uint16_t keep_alive_s,
                                            bool clean_session)
{
    if (!s_core.initialised)
    {
        return ESP8266_AT_STATUS_NOT_INITIALISED;
    }
    if (host == NULL || port == 0U)
    {
        return ESP8266_AT_STATUS_INVALID_ARGUMENT;
    }

    char host_field[128];
    esp8266_at_escape_field(host, host_field, sizeof(host_field));

#if ESP8266_AT_USE_MQTTCONNCFG
    const unsigned int disable_clean_session = clean_session ? 0U : 1U;
    const unsigned int clean_session_flag    = clean_session ? 1U : 0U;

    char cfg_args[ESP8266_AT_MAX_LINE_LENGTH];
    (void)snprintf(cfg_args,
                   sizeof(cfg_args),
                   "%u,%u,%u,%u,0,0,\"\",\"\"",
                   (unsigned int)link_id,
                   (unsigned int)keep_alive_s,
                   disable_clean_session,
                   clean_session_flag);

    esp8266_at_status_t status =
        esp8266_at_send_and_wait_internal(ESP8266_AT_CMD_MQTTCONNCFG,
                                          ESP8266_AT_COMMAND_MODE_SET,
                                          cfg_args,
                                          ESP8266_AT_DEFAULT_TIMEOUT_MS,
                                          false,
                                          NULL);
    if (status != ESP8266_AT_STATUS_OK)
    {
        return status;
    }
#else
    (void)keep_alive_s;
    (void)clean_session;
#endif

  char conn_args[ESP8266_AT_MAX_LINE_LENGTH];
  (void)snprintf(conn_args,
                 sizeof(conn_args),
                 "%u,\"%s\",%u,0",
                 (unsigned int)link_id,
                 host_field,
                 (unsigned int)port);

  printf("[ESP8266][cmd] AT+MQTTCONN=%s\r\n", conn_args);


  return esp8266_at_send_and_wait_internal(ESP8266_AT_CMD_MQTTCONN,
                                           ESP8266_AT_COMMAND_MODE_SET,
                                           conn_args,
                                           ESP8266_AT_DEFAULT_TIMEOUT_MS * 3U,
                                           false,
                                             NULL);
}

esp8266_at_status_t esp8266_at_mqtt_publish(uint8_t link_id,
                                            const char *topic,
                                            const char *payload,
                                            uint8_t qos,
                                            bool retain)
{
    if (!s_core.initialised)
    {
        return ESP8266_AT_STATUS_NOT_INITIALISED;
    }
    if (topic == NULL || payload == NULL)
    {
        return ESP8266_AT_STATUS_INVALID_ARGUMENT;
    }

    char topic_field[128];
    char payload_field[256];

    const size_t payload_plain_length = strlen(payload);
    const size_t topic_escaped_length = esp8266_at_escape_field_length(topic);
    const size_t payload_escaped_length = esp8266_at_escape_field_length(payload);
    if (topic_escaped_length >= sizeof(topic_field)
        || payload_escaped_length >= sizeof(payload_field))
    {
        return esp8266_at_mqtt_publish_raw(link_id,
                                           topic,
                                           (const uint8_t *)payload,
                                           payload_plain_length,
                                           qos,
                                           retain);
    }

    esp8266_at_escape_field(topic, topic_field, sizeof(topic_field));
    esp8266_at_escape_field(payload, payload_field, sizeof(payload_field));

    char args[ESP8266_AT_MAX_LINE_LENGTH];
    const int arg_length = snprintf(args,
                                    sizeof(args),
                                    "%u,\"%s\",\"%s\",%u,%u",
                                    (unsigned int)link_id,
                                    topic_field,
                                    payload_field,
                                    (unsigned int)qos,
                                    retain ? 1U : 0U);

    const esp8266_at_command_def_t *cmd_def = esp8266_at_get_command_def(ESP8266_AT_CMD_MQTTPUB);
    size_t literal_length = 0U;
    if (cmd_def != NULL && cmd_def->literal != NULL)
    {
        literal_length = strlen(cmd_def->literal);
    }
    const size_t max_args_length =
        (literal_length + 4U < ESP8266_AT_MAX_LINE_LENGTH)
            ? (ESP8266_AT_MAX_LINE_LENGTH - literal_length - 4U)
            : 0U;

    if (arg_length < 0
        || (size_t)arg_length >= sizeof(args)
        || (size_t)arg_length > max_args_length)
    {
        return esp8266_at_mqtt_publish_raw(link_id,
                                           topic,
                                           (const uint8_t *)payload,
                                           payload_plain_length,
                                           qos,
                                           retain);
    }

    return esp8266_at_send_and_wait_internal(ESP8266_AT_CMD_MQTTPUB,
                                             ESP8266_AT_COMMAND_MODE_SET,
                                             args,
                                             ESP8266_AT_DEFAULT_TIMEOUT_MS * 2U,
                                             false,
                                             NULL);
}

esp8266_at_status_t esp8266_at_mqtt_publish_raw(uint8_t link_id,
                                                const char *topic,
                                                const uint8_t *payload,
                                                size_t length,
                                                uint8_t qos,
                                                bool retain)
{
    if (!s_core.initialised)
    {
        return ESP8266_AT_STATUS_NOT_INITIALISED;
    }
    if (topic == NULL || payload == NULL || length == 0U)
    {
        return ESP8266_AT_STATUS_INVALID_ARGUMENT;
    }

    char topic_field[128];
    esp8266_at_escape_field(topic, topic_field, sizeof(topic_field));

    char args[ESP8266_AT_MAX_LINE_LENGTH];
    (void)snprintf(args,
                   sizeof(args),
                   "%u,\"%s\",%u,%u,%u",
                   (unsigned int)link_id,
                   topic_field,
                   (unsigned int)length,
                   (unsigned int)qos,
                   retain ? 1U : 0U);

    esp8266_at_status_t status =
        esp8266_at_send_and_wait_internal(ESP8266_AT_CMD_MQTTPUBRAW,
                                          ESP8266_AT_COMMAND_MODE_SET,
                                          args,
                                          ESP8266_AT_DEFAULT_TIMEOUT_MS,
                                          true,
                                          NULL);
    if (status != ESP8266_AT_STATUS_PROMPT)
    {
        return status;
    }

    status = esp8266_at_send_data(payload, length, ESP8266_AT_DEFAULT_TIMEOUT_MS * 2U);
    if (status != ESP8266_AT_STATUS_OK)
    {
        return status;
    }

    status = esp8266_at_wait_for_completion(ESP8266_AT_DEFAULT_TIMEOUT_MS * 2U, NULL);
    return status;
}

esp8266_at_status_t esp8266_at_mqtt_disconnect(uint8_t link_id)
{
    (void)link_id;
    const char args[] = "0";
    return esp8266_at_send_and_wait_internal(ESP8266_AT_CMD_MQTTCLEAN,
                                             ESP8266_AT_COMMAND_MODE_SET,
                                             args,
                                             ESP8266_AT_DEFAULT_TIMEOUT_MS,
                                             false,
                                             NULL);
}

bool esp8266_at_fetch_event(esp8266_at_event_t *event)
{
    if (!s_core.initialised || event == NULL)
    {
        return false;
    }
    return esp8266_at_queue_pop(&s_core, event);
}

size_t esp8266_at_pending_event_count(void)
{
    return s_core.queue.count;
}

void esp8266_at_poll(void)
{
    esp8266_at_interface_flush_trace();

    if (!s_core.initialised || !s_core.awaiting_reply)
    {
        return;
    }

    const uint32_t now = esp8266_at_interface_hw_get_tick();
    if (esp8266_at_tick_elapsed(s_core.command_start_tick, now) > s_core.command_timeout_ms)
    {
        s_core.awaiting_reply = false;
        s_core.phase          = ESP8266_AT_PHASE_IDLE;
        s_core.last_status    = ESP8266_AT_STATUS_TIMEOUT;
    }
}

void esp8266_at_receive_bytes(const uint8_t *data, size_t length)
{
    if (!s_core.initialised || data == NULL || length == 0U)
    {
        return;
    }

    for (size_t i = 0U; i < length; ++i)
    {
        esp8266_at_core_process_byte(&s_core, data[i]);
    }
}

void esp8266_at_debug_print_events(bool keep_events)
{
    if (!s_core.initialised)
    {
        return;
    }

    /* Current implementation does not print; it effectively clears the queue unless asked to keep events. */
    if (!keep_events)
    {
        s_core.queue.head  = 0U;
        s_core.queue.tail  = 0U;
        s_core.queue.count = 0U;
    }
}


void esp8266_at_clear_events(void)
{
    if (!s_core.initialised)
    {
        return;
    }

    s_core.queue.head = 0U;
    s_core.queue.tail = 0U;
    s_core.queue.count = 0U;
    s_core.last_terminal_event_valid = false;
}

static esp8266_at_status_t esp8266_at_send_and_wait_internal(esp8266_at_command_id_t command,
                                                             esp8266_at_command_mode_t mode,
                                                             const char *arguments,
                                                             uint32_t timeout_ms,
                                                             bool expect_prompt,
                                                             esp8266_at_event_t *terminal_event)
{
    if (!s_core.initialised)
    {
        return ESP8266_AT_STATUS_NOT_INITIALISED;
    }
    if (command >= ESP8266_AT_CMD_COUNT)
    {
        return ESP8266_AT_STATUS_INVALID_ARGUMENT;
    }

    const esp8266_at_command_def_t *def = &s_command_table[command];

    switch (mode)
    {
        case ESP8266_AT_COMMAND_MODE_EXECUTE:
            if (!def->supports_execute)
            {
                return ESP8266_AT_STATUS_INVALID_ARGUMENT;
            }
            break;
        case ESP8266_AT_COMMAND_MODE_QUERY:
            if (!def->supports_query)
            {
                return ESP8266_AT_STATUS_INVALID_ARGUMENT;
            }
            break;
        case ESP8266_AT_COMMAND_MODE_TEST:
            if (!def->supports_test)
            {
                return ESP8266_AT_STATUS_INVALID_ARGUMENT;
            }
            break;
        case ESP8266_AT_COMMAND_MODE_SET:
            if (!def->supports_set || arguments == NULL)
            {
                return ESP8266_AT_STATUS_INVALID_ARGUMENT;
            }
            break;
        default:
            return ESP8266_AT_STATUS_INVALID_ARGUMENT;
    }

    if (s_core.awaiting_reply
        || s_core.phase == ESP8266_AT_PHASE_WAITING_PROMPT
        || s_core.phase == ESP8266_AT_PHASE_WAITING_FINAL)
    {
        return ESP8266_AT_STATUS_BUSY;
    }

    char tx_buffer[ESP8266_AT_MAX_LINE_LENGTH];
    size_t offset = 0U;

    const size_t literal_length = esp8266_at_strnlen(def->literal, sizeof(tx_buffer) - 3U);
    if (literal_length == 0U)
    {
        return ESP8266_AT_STATUS_INVALID_ARGUMENT;
    }

    memcpy(&tx_buffer[offset], def->literal, literal_length);
    offset += literal_length;

    if (mode == ESP8266_AT_COMMAND_MODE_QUERY)
    {
        tx_buffer[offset++] = '?';
    }
    else if (mode == ESP8266_AT_COMMAND_MODE_TEST)
    {
        tx_buffer[offset++] = '=';
        tx_buffer[offset++] = '?';
    }
    else if (mode == ESP8266_AT_COMMAND_MODE_SET)
    {
        tx_buffer[offset++] = '=';
        const size_t arg_len = esp8266_at_strnlen(arguments,
                                                  sizeof(tx_buffer) - offset - 3U);
        memcpy(&tx_buffer[offset], arguments, arg_len);
        offset += arg_len;
    }
    else if (mode == ESP8266_AT_COMMAND_MODE_EXECUTE
             && arguments != NULL
             && arguments[0] != '\0')
    {
        const size_t arg_len = esp8266_at_strnlen(arguments,
                                                  sizeof(tx_buffer) - offset - 3U);
        memcpy(&tx_buffer[offset], arguments, arg_len);
        offset += arg_len;
    }

    tx_buffer[offset++] = '\r';
    tx_buffer[offset++] = '\n';

    const esp8266_at_status_t send_status =
        esp8266_at_interface_hw_send((const uint8_t *)tx_buffer, offset, timeout_ms);
    if (send_status != ESP8266_AT_STATUS_OK)
    {
        return send_status;
    }

    s_core.awaiting_reply            = true;
    s_core.expect_prompt             = expect_prompt;
    s_core.pending_command           = command;
    s_core.pending_mode              = mode;
    s_core.command_timeout_ms        = (timeout_ms != 0U) ? timeout_ms : ESP8266_AT_DEFAULT_TIMEOUT_MS;
    s_core.command_start_tick        = esp8266_at_interface_hw_get_tick();
    s_core.last_status               = expect_prompt ? ESP8266_AT_STATUS_PROMPT : ESP8266_AT_STATUS_TIMEOUT;
    s_core.last_terminal_event_valid = false;
    s_core.phase                     = expect_prompt ? ESP8266_AT_PHASE_WAITING_PROMPT
                                                     : ESP8266_AT_PHASE_WAITING_FINAL;

    return esp8266_at_wait_for_completion(timeout_ms, terminal_event);
}

static void esp8266_at_escape_field(const char *input, char *output, size_t output_size)
{
    if (output == NULL || output_size == 0U)
    {
        return;
    }

    size_t out_index = 0U;
    const char *cursor = (input != NULL) ? input : "";

    while (*cursor != '\0' && out_index < output_size - 1U)
    {
        const char ch = *cursor++;
        if (ch == '"' || ch == '\\')
        {
            if (out_index + 2U >= output_size)
            {
                break;
            }
            output[out_index++] = '\\';
            output[out_index++] = ch;
        }
        else
        {
            output[out_index++] = ch;
        }
    }

    output[out_index] = '\0';
}

static size_t esp8266_at_escape_field_length(const char *input)
{
    if (input == NULL)
    {
        return 0U;
    }

    size_t length = 0U;
    while (*input != '\0')
    {
        length += (*input == '"' || *input == '\\') ? 2U : 1U;
        ++input;
    }

    return length;
}

static void esp8266_at_core_reset_state(esp8266_at_core_t *ctx)
{
    ctx->awaiting_reply            = false;
    ctx->expect_prompt             = false;
    ctx->phase                     = ESP8266_AT_PHASE_IDLE;
    ctx->pending_command           = ESP8266_AT_CMD_AT;
    ctx->pending_mode              = ESP8266_AT_COMMAND_MODE_UNKNOWN;
    ctx->command_timeout_ms        = ESP8266_AT_DEFAULT_TIMEOUT_MS;
    ctx->command_start_tick        = 0U;
    ctx->last_status               = ESP8266_AT_STATUS_OK;
    ctx->last_terminal_event_valid = false;
    ctx->queue.head = ctx->queue.tail = ctx->queue.count = 0U;
    esp8266_at_core_reset_line(ctx);
}

static void esp8266_at_core_reset_line(esp8266_at_core_t *ctx)
{
    ctx->line.length = 0U;
    ctx->line.buffer[0] = '\0';
}

static void esp8266_at_core_process_prompt(esp8266_at_core_t *ctx)
{
    esp8266_at_event_t event;
    memset(&event, 0, sizeof(event));

    event.type    = ESP8266_AT_EVENT_TYPE_PROMPT;
    event.command = ctx->pending_command;
    event.raw_line[0] = ESP8266_AT_PROMPT_CHAR;
    event.raw_line[1] = '\0';
    strncpy(event.payload, event.raw_line, sizeof(event.payload) - 1U);

    ctx->awaiting_reply            = false;
    ctx->expect_prompt             = false;
    ctx->phase                     = ESP8266_AT_PHASE_READY_FOR_DATA;
    ctx->last_status               = ESP8266_AT_STATUS_PROMPT;
    ctx->last_terminal_event       = event;
    ctx->last_terminal_event_valid = true;

    (void)esp8266_at_queue_push(ctx, &event);
}

static void esp8266_at_core_process_byte(esp8266_at_core_t *ctx, uint8_t byte)
{
    if (ctx == NULL)
    {
        return;
    }

    if (ctx->phase == ESP8266_AT_PHASE_WAITING_PROMPT
        && byte == (uint8_t)ESP8266_AT_PROMPT_CHAR
        && ctx->line.length == 0U)
    {
        esp8266_at_core_process_prompt(ctx);
        return;
    }

    if (ctx->phase == ESP8266_AT_PHASE_READY_FOR_DATA
        && ctx->line.length == 0U
        && byte == ' ')
    {
        return;
    }

    if (byte == '\r')
    {
        return;
    }

    if (byte == '\n')
    {
        if (ctx->line.length == 0U)
        {
            return;
        }
        ctx->line.buffer[ctx->line.length] = '\0';
        esp8266_at_core_handle_line(ctx, ctx->line.buffer);
        esp8266_at_core_reset_line(ctx);
        return;
    }

    if (ctx->line.length >= (sizeof(ctx->line.buffer) - 1U))
    {
        ctx->last_status = ESP8266_AT_STATUS_BUFFER_OVERFLOW;
        esp8266_at_core_reset_line(ctx);
        return;
    }

    ctx->line.buffer[ctx->line.length++] = (char)byte;
}


static bool esp8266_at_parse_line_to_event(const char *line, esp8266_at_event_t *event)
{
    if (line == NULL || event == NULL || line[0] == '\0')
    {
        return false;
    }

    char trimmed[ESP8266_AT_MAX_LINE_LENGTH];
    strncpy(trimmed, line, sizeof(trimmed) - 1U);
    trimmed[sizeof(trimmed) - 1U] = '\0';
    esp8266_at_trim(trimmed);
    if (trimmed[0] == '\0')
    {
        return false;
    }

    memset(event, 0, sizeof(*event));
    strncpy(event->raw_line, trimmed, sizeof(event->raw_line) - 1U);
    event->raw_line[sizeof(event->raw_line) - 1U] = '\0';
    strncpy(event->payload, trimmed, sizeof(event->payload) - 1U);
    event->payload[sizeof(event->payload) - 1U] = '\0';

    if (esp8266_at_strcasecmp(trimmed, "OK") == 0)
    {
        event->type = ESP8266_AT_EVENT_TYPE_OK;
    }
    else if (esp8266_at_strcasecmp(trimmed, "ERROR") == 0)
    {
        event->type = ESP8266_AT_EVENT_TYPE_ERROR;
    }
    else if (esp8266_at_strcasecmp(trimmed, "FAIL") == 0)
    {
        event->type = ESP8266_AT_EVENT_TYPE_FAIL;
    }
    else if (esp8266_at_strcasecmp(trimmed, "SEND OK") == 0)
    {
        event->type = ESP8266_AT_EVENT_TYPE_SEND_OK;
    }
    else if (esp8266_at_strcasecmp(trimmed, "SEND FAIL") == 0)
    {
        event->type = ESP8266_AT_EVENT_TYPE_SEND_FAIL;
    }
    else if (esp8266_at_strncasecmp(trimmed, "busy", 4U) == 0)
    {
        event->type = ESP8266_AT_EVENT_TYPE_BUSY;
    }
    else if (trimmed[0] == '+')
    {
        event->type = ESP8266_AT_EVENT_TYPE_RESPONSE;

        char local[ESP8266_AT_MAX_LINE_LENGTH];
        strncpy(local, trimmed, sizeof(local) - 1U);
        local[sizeof(local) - 1U] = '\0';

        char *separator = strpbrk(local, ":=");
        char *payload   = NULL;
        if (separator != NULL)
        {
            *separator = '\0';
            payload = separator + 1;
            while (payload[0] == ' ')
            {
                ++payload;
            }
        }

        strncpy(event->prefix, local, sizeof(event->prefix) - 1U);
        event->prefix[sizeof(event->prefix) - 1U] = '\0';
        if (payload != NULL)
        {
            strncpy(event->payload, payload, sizeof(event->payload) - 1U);
            event->payload[sizeof(event->payload) - 1U] = '\0';
        }

        event->command = esp8266_at_match_command_by_prefix(event->prefix);
        esp8266_at_parse_response_payload(event);
    }
    else if (esp8266_at_strcasecmp(trimmed, "ready") == 0)
    {
        event->type = ESP8266_AT_EVENT_TYPE_INDICATION;
        strncpy(event->prefix, "READY", sizeof(event->prefix) - 1U);
        event->prefix[sizeof(event->prefix) - 1U] = '\0';
    }
    else if (esp8266_at_strncasecmp(trimmed, "WIFI ", 5U) == 0)
    {
        event->type = ESP8266_AT_EVENT_TYPE_INDICATION;
        strncpy(event->prefix, "WIFI", sizeof(event->prefix) - 1U);
        event->prefix[sizeof(event->prefix) - 1U] = '\0';
    }
    else
    {
        event->type = ESP8266_AT_EVENT_TYPE_INFO;
    }

    return true;
}

static void esp8266_at_compact_from_event(esp8266_at_event_compact_t *dst, const esp8266_at_event_t *src)
{
    if (dst == NULL || src == NULL)
    {
        return;
    }

    memset(dst, 0, sizeof(*dst));
    dst->type    = src->type;
    dst->command = src->command;
    dst->mode    = src->mode;
    strncpy(dst->raw_line, src->raw_line, sizeof(dst->raw_line) - 1U);
    dst->raw_line[sizeof(dst->raw_line) - 1U] = '\0';
}

static bool esp8266_at_event_from_compact(const esp8266_at_event_compact_t *src, esp8266_at_event_t *dst)
{
    if (src == NULL || dst == NULL)
    {
        return false;
    }

    /* Parse the raw line into a full event (prefix/payload/arguments). */
    bool ok = esp8266_at_parse_line_to_event(src->raw_line, dst);
    if (!ok)
    {
        /* Keep a minimal, still-useful event. */
        memset(dst, 0, sizeof(*dst));
        strncpy(dst->raw_line, src->raw_line, sizeof(dst->raw_line) - 1U);
        dst->raw_line[sizeof(dst->raw_line) - 1U] = '\0';
        strncpy(dst->payload, src->raw_line, sizeof(dst->payload) - 1U);
        dst->payload[sizeof(dst->payload) - 1U] = '\0';
    }

    /* Restore metadata captured at enqueue time (important for terminal events). */
    dst->type    = src->type;
    dst->command = src->command;
    dst->mode    = src->mode;

    if (dst->type == ESP8266_AT_EVENT_TYPE_PROMPT)
    {
        /* For prompts, payload is the prompt character. */
        strncpy(dst->payload, dst->raw_line, sizeof(dst->payload) - 1U);
        dst->payload[sizeof(dst->payload) - 1U] = '\0';
    }

    return true;
}

static void esp8266_at_queue_restore_compact(esp8266_at_core_t *ctx,
                                             const esp8266_at_event_compact_t *events,
                                             size_t count)
{
    if (ctx == NULL || events == NULL)
    {
        return;
    }

    for (size_t i = 0U; i < count; ++i)
    {
        if (ctx->queue.count >= ESP8266_AT_EVENT_QUEUE_DEPTH)
        {
            ctx->queue.head = (uint8_t)((ctx->queue.head + 1U) % ESP8266_AT_EVENT_QUEUE_DEPTH);
            ctx->queue.count--;
        }

        ctx->queue.events[ctx->queue.tail] = events[i];
        ctx->queue.tail = (uint8_t)((ctx->queue.tail + 1U) % ESP8266_AT_EVENT_QUEUE_DEPTH);
        ctx->queue.count++;
    }
}


static void esp8266_at_core_handle_line(esp8266_at_core_t *ctx, const char *line)
{
    if (ctx == NULL || line == NULL || line[0] == '\0')
    {
        return;
    }

    esp8266_at_event_t event;
    if (!esp8266_at_parse_line_to_event(line, &event))
    {
        return;
    }

switch (event.type)
    {
        case ESP8266_AT_EVENT_TYPE_OK:
        case ESP8266_AT_EVENT_TYPE_SEND_OK:
            event.command                 = ctx->pending_command;
            ctx->awaiting_reply            = false;
            ctx->phase                     = ESP8266_AT_PHASE_IDLE;
            ctx->last_status               = ESP8266_AT_STATUS_OK;
            ctx->last_terminal_event       = event;
            ctx->last_terminal_event_valid = true;
            break;

        case ESP8266_AT_EVENT_TYPE_ERROR:
            event.command                 = ctx->pending_command;
            ctx->awaiting_reply            = false;
            ctx->phase                     = ESP8266_AT_PHASE_IDLE;
            ctx->last_status               = ESP8266_AT_STATUS_ERROR_RESPONSE;
            ctx->last_terminal_event       = event;
            ctx->last_terminal_event_valid = true;
            break;

        case ESP8266_AT_EVENT_TYPE_FAIL:
        case ESP8266_AT_EVENT_TYPE_SEND_FAIL:
            event.command                 = ctx->pending_command;
            ctx->awaiting_reply            = false;
            ctx->phase                     = ESP8266_AT_PHASE_IDLE;
            ctx->last_status               = ESP8266_AT_STATUS_FAIL_RESPONSE;
            ctx->last_terminal_event       = event;
            ctx->last_terminal_event_valid = true;
            break;

        case ESP8266_AT_EVENT_TYPE_BUSY:
            event.command                 = ctx->pending_command;
            ctx->awaiting_reply            = false;
            ctx->phase                     = ESP8266_AT_PHASE_IDLE;
            ctx->last_status               = ESP8266_AT_STATUS_BUSY_RESPONSE;
            ctx->last_terminal_event       = event;
            ctx->last_terminal_event_valid = true;
            break;

        default:
            break;
    }

        (void)esp8266_at_queue_push(ctx, &event);
}


static bool esp8266_at_queue_push(esp8266_at_core_t *ctx, const esp8266_at_event_t *event)
{
    if (ctx == NULL || event == NULL)
    {
        return false;
    }

    if (ctx->queue.count >= ESP8266_AT_EVENT_QUEUE_DEPTH)
    {
        ctx->queue.head = (uint8_t)((ctx->queue.head + 1U) % ESP8266_AT_EVENT_QUEUE_DEPTH);
        ctx->queue.count--;
    }

    esp8266_at_compact_from_event(&ctx->queue.events[ctx->queue.tail], event);
    ctx->queue.tail = (uint8_t)((ctx->queue.tail + 1U) % ESP8266_AT_EVENT_QUEUE_DEPTH);
    ctx->queue.count++;
    return true;
}


static bool esp8266_at_queue_pop(esp8266_at_core_t *ctx, esp8266_at_event_t *event)
{
    if (ctx == NULL || event == NULL || ctx->queue.count == 0U)
    {
        return false;
    }

    const esp8266_at_event_compact_t compact = ctx->queue.events[ctx->queue.head];
    ctx->queue.head = (uint8_t)((ctx->queue.head + 1U) % ESP8266_AT_EVENT_QUEUE_DEPTH);
    ctx->queue.count--;

    return esp8266_at_event_from_compact(&compact, event);
}


static void esp8266_at_queue_restore(esp8266_at_core_t *ctx,
                                     const esp8266_at_event_t *events,
                                     size_t count)
{
    if (ctx == NULL || events == NULL)
    {
        return;
    }

    for (size_t i = 0U; i < count; ++i)
    {
        (void)esp8266_at_queue_push(ctx, &events[i]);
    }
}

static uint32_t esp8266_at_tick_elapsed(uint32_t start, uint32_t now)
{
    return (now >= start)
           ? (now - start)
           : ((uint32_t)(UINT32_MAX - start + now + 1U));
}

static const char *esp8266_at_event_type_name(esp8266_at_event_type_t type)
{
    switch (type)
    {
        case ESP8266_AT_EVENT_TYPE_INFO:       return "info";
        case ESP8266_AT_EVENT_TYPE_INDICATION: return "ind";
        case ESP8266_AT_EVENT_TYPE_RESPONSE:   return "resp";
        case ESP8266_AT_EVENT_TYPE_OK:         return "ok";
        case ESP8266_AT_EVENT_TYPE_ERROR:      return "error";
        case ESP8266_AT_EVENT_TYPE_FAIL:       return "fail";
        case ESP8266_AT_EVENT_TYPE_BUSY:       return "busy";
        case ESP8266_AT_EVENT_TYPE_PROMPT:     return "prompt";
        case ESP8266_AT_EVENT_TYPE_SEND_OK:    return "send_ok";
        case ESP8266_AT_EVENT_TYPE_SEND_FAIL:  return "send_fail";
        default:                               return "unknown";
    }
}

static size_t esp8266_at_strnlen(const char *str, size_t max_len)
{
    size_t length = 0U;
    if (str == NULL || max_len == 0U)
    {
        return 0U;
    }

    while (length < max_len && str[length] != '\0')
    {
        ++length;
    }
    return length;
}

static int esp8266_at_strcasecmp(const char *lhs, const char *rhs)
{
    if (lhs == NULL && rhs == NULL)
    {
        return 0;
    }
    if (lhs == NULL)
    {
        return -1;
    }
    if (rhs == NULL)
    {
        return 1;
    }

    while (*lhs != '\0' || *rhs != '\0')
    {
        const int a = tolower((int)(unsigned char)*lhs++);
        const int b = tolower((int)(unsigned char)*rhs++);
        if (a != b)
        {
            return a - b;
        }
    }
    return 0;
}

static int esp8266_at_strncasecmp(const char *lhs, const char *rhs, size_t length)
{
    if (length == 0U)
    {
        return 0;
    }
    if (lhs == NULL && rhs == NULL)
    {
        return 0;
    }
    if (lhs == NULL)
    {
        return -1;
    }
    if (rhs == NULL)
    {
        return 1;
    }

    for (size_t i = 0U; i < length; ++i)
    {
        const int a = tolower((int)(unsigned char)lhs[i]);
        const int b = tolower((int)(unsigned char)rhs[i]);
        if (a != b)
        {
            return a - b;
        }
        if (lhs[i] == '\0' || rhs[i] == '\0')
        {
            return 0;
        }
    }
    return 0;
}

static void esp8266_at_trim(char *str)
{
    if (str == NULL)
    {
        return;
    }

    size_t length = strlen(str);
    size_t start  = 0U;
    while (start < length && (unsigned char)str[start] <= ' ')
    {
        ++start;
    }

    size_t end = length;
    while (end > start && (unsigned char)str[end - 1U] <= ' ')
    {
        --end;
    }

    if (start > 0U)
    {
        memmove(str, &str[start], end - start);
    }
    str[end - start] = '\0';
}

static void esp8266_at_copy_lower(char *dest, size_t dest_size, const char *source)
{
    if (dest == NULL || dest_size == 0U)
    {
        return;
    }

    size_t index = 0U;
    if (source != NULL)
    {
        while (source[index] != '\0' && index < dest_size - 1U)
        {
            dest[index] = (char)tolower((int)(unsigned char)source[index]);
            ++index;
        }
    }
    dest[index] = '\0';
}

static void esp8266_at_parse_response_payload(esp8266_at_event_t *event)
{
    if (event == NULL)
    {
        return;
    }

    for (size_t i = 0U; i < ESP8266_AT_MAX_ARGUMENTS; ++i)
    {
        event->arguments[i].value[0] = '\0';
        event->arguments[i].quoted   = false;
    }

    if (event->payload[0] == '\0')
    {
        event->argument_count = 0U;
        return;
    }

    size_t arg_index = 0U;
    size_t out_index = 0U;
    bool in_quote = false;
    const char *cursor = event->payload;

    event->argument_count = 1U;
    event->arguments[0].value[0] = '\0';

    while (*cursor != '\0' && arg_index < ESP8266_AT_MAX_ARGUMENTS)
    {
        const char ch = *cursor++;
        if (ch == '"')
        {
            in_quote = !in_quote;
            event->arguments[arg_index].quoted = true;
            continue;
        }
        if (ch == ',' && !in_quote)
        {
            esp8266_at_trim(event->arguments[arg_index].value);
            ++arg_index;
            if (arg_index >= ESP8266_AT_MAX_ARGUMENTS)
            {
                break;
            }
            event->arguments[arg_index].value[0] = '\0';
            event->arguments[arg_index].quoted   = false;
            out_index = 0U;
            event->argument_count = arg_index + 1U;
            continue;
        }
        if (out_index < (ESP8266_AT_MAX_ARGUMENT_LENGTH - 1U))
        {
            event->arguments[arg_index].value[out_index++] = ch;
            event->arguments[arg_index].value[out_index]   = '\0';
        }
    }

    esp8266_at_trim(event->arguments[arg_index].value);
    event->argument_count = arg_index + 1U;
}

static void esp8266_at_parse_ip_descriptor(const esp8266_at_event_t *event,
                                           bool station,
                                           esp8266_at_ip_info_t *info)
{
    if (event == NULL || info == NULL)
    {
        return;
    }

    const char *payload = (event->payload[0] != '\0')
                        ? event->payload
                        : event->raw_line;

    const char *separator = strchr(payload, ':');
    if (separator == NULL)
    {
        return;
    }

    size_t key_len = (size_t)(separator - payload);
    if (key_len == 0U)
    {
        return;
    }

    char key[16];
    if (key_len >= sizeof(key))
    {
        key_len = sizeof(key) - 1U;
    }
    memcpy(key, payload, key_len);
    key[key_len] = '\0';
    esp8266_at_trim(key);
    esp8266_at_copy_lower(key, sizeof(key), key);

    const char *value = separator + 1;
    while (*value == ' ')
    {
        ++value;
    }

    bool quoted = false;
    if (*value == '"')
    {
        quoted = true;
        ++value;
    }

    char buffer[48];
    size_t index = 0U;
    while (*value != '\0' && index < sizeof(buffer) - 1U)
    {
        if (quoted && *value == '"')
        {
            break;
        }
        if (!quoted && (*value == ' ' || *value == ','))
        {
            break;
        }
        buffer[index++] = *value++;
    }
    buffer[index] = '\0';
    esp8266_at_trim(buffer);

    char *target = NULL;
    size_t target_size = 0U;
    if (station)
    {
        if (strcmp(key, "ip") == 0)
        {
            target = info->station_ip;
            target_size = sizeof(info->station_ip);
        }
        else if (strcmp(key, "gateway") == 0)
        {
            target = info->station_gateway;
            target_size = sizeof(info->station_gateway);
        }
        else if (strcmp(key, "netmask") == 0)
        {
            target = info->station_netmask;
            target_size = sizeof(info->station_netmask);
        }
    }
    else
    {
        if (strcmp(key, "ip") == 0)
        {
            target = info->softap_ip;
            target_size = sizeof(info->softap_ip);
        }
        else if (strcmp(key, "gateway") == 0)
        {
            target = info->softap_gateway;
            target_size = sizeof(info->softap_gateway);
        }
        else if (strcmp(key, "netmask") == 0)
        {
            target = info->softap_netmask;
            target_size = sizeof(info->softap_netmask);
        }
    }

    if (target != NULL && target_size > 0U)
    {
        strncpy(target, buffer, target_size - 1U);
        target[target_size - 1U] = '\0';
    }
}

static esp8266_at_command_id_t esp8266_at_match_command_by_prefix(const char *prefix)
{
    if (prefix == NULL || prefix[0] == '\0')
    {
        return ESP8266_AT_CMD_COUNT;
    }

    for (size_t i = 0U; i < ESP8266_AT_CMD_COUNT; ++i)
    {
        const char *literal = s_command_table[i].literal;
        const char *plus    = strchr(literal, '+');
        const char *candidate = (plus != NULL) ? plus : literal;
        if (esp8266_at_strcasecmp(candidate, prefix) == 0)
        {
            return (esp8266_at_command_id_t)i;
        }
    }
    return ESP8266_AT_CMD_COUNT;
}

static bool esp8266_at_pop_event_by_prefix(const char *prefix, esp8266_at_event_t *out_event)
{
    if (prefix == NULL || out_event == NULL)
    {
        return false;
    }

    if (s_core.queue.count == 0U)
    {
        return false;
    }

    const uint8_t initial_count = s_core.queue.count;
    bool found = false;

    for (uint8_t i = 0U; i < initial_count; ++i)
    {
        esp8266_at_event_t event;
        if (!esp8266_at_queue_pop(&s_core, &event))
        {
            break;
        }

        if (!found && esp8266_at_strcasecmp(event.prefix, prefix) == 0)
        {
            *out_event = event;
            found = true;
            continue;
        }

        (void)esp8266_at_queue_push(&s_core, &event);
    }

    return found;
}
