#include "unit_test.h"

#define LED_BLINK_PERIOD_DEFAULT_MS 500U
#define LED_BLINK_PERIOD_MIN_MS 50U
#define LED_BLINK_PERIOD_MAX_MS 5000U
#define LED_ALL_MASK ((1U << UNIT_TEST_LED_COUNT) - 1U)
#define LED_ON_STATE GPIO_PIN_RESET
#define LED_OFF_STATE GPIO_PIN_SET
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

typedef struct {
  GPIO_TypeDef *port;
  uint16_t pin;
  const char *label;
} led_hw_t;

typedef struct {
  const char *name;
  uint32_t mask;
} led_target_t;

typedef struct {
  bool blink_enabled;
  uint32_t blink_period_ms;
  bool manual_state;
  bool current_state;
  osTimerId_t timer;
} led_runtime_t;

static const led_hw_t kLedHw[UNIT_TEST_LED_COUNT] = {
  [UNIT_TEST_LED_BLUE] = {LED_B_GPIO_Port, LED_B_Pin, "Blue"},
  [UNIT_TEST_LED_GREEN] = {LED_G_GPIO_Port, LED_G_Pin, "Green"},
  [UNIT_TEST_LED_PURPLE] = {LED_P_GPIO_Port, LED_P_Pin, "Pink"},
};

static const led_target_t kLedTargets[] = {
  {"blue", 1U << UNIT_TEST_LED_BLUE},
  {"b", 1U << UNIT_TEST_LED_BLUE},
  {"green", 1U << UNIT_TEST_LED_GREEN},
  {"g", 1U << UNIT_TEST_LED_GREEN},
  {"pink", 1U << UNIT_TEST_LED_PURPLE},
  {"purple", 1U << UNIT_TEST_LED_PURPLE},
  {"p", 1U << UNIT_TEST_LED_PURPLE},
  {"all", LED_ALL_MASK},
};

static volatile led_runtime_t s_led_state[UNIT_TEST_LED_COUNT];

static int led_shell_command(int argc, const char * const *argv);
static void led_register_shell_command(void);
static void led_print_usage(void);
static bool led_parse_bool(const char *text, bool *value);
static bool led_parse_period(const char *text, uint32_t *period_ms);
static bool led_parse_target(const char *text, uint32_t *mask);
static int shell_stricmp(const char *a, const char *b);
static void led_write_hw(size_t index, bool state);
static void led_init_defaults(void);
static uint32_t led_half_period(uint32_t period_ms);
static uint32_t led_ms_to_ticks(uint32_t period_ms);
static void led_timer_callback(void *argument);

static uint32_t led_clamp_period(uint32_t period_ms)
{
  if (period_ms < LED_BLINK_PERIOD_MIN_MS) {
    return LED_BLINK_PERIOD_MIN_MS;
  }
  if (period_ms > LED_BLINK_PERIOD_MAX_MS) {
    return LED_BLINK_PERIOD_MAX_MS;
  }
  return period_ms;
}

void unit_test_led_set_blink(unit_test_led_id_t led, bool enable, uint32_t period_ms)
{
  if (led >= UNIT_TEST_LED_COUNT) {
    return;
  }

  osTimerId_t timer = NULL;
  uint32_t start_ticks = 0U;

  taskENTER_CRITICAL();
  led_runtime_t *ch = &s_led_state[led];
  timer = ch->timer;

  if (enable) {
    uint32_t new_period = period_ms;
    if (new_period == 0U) {
      new_period = ch->blink_period_ms;
    }
    new_period = led_clamp_period(new_period);
    ch->blink_enabled = true;
    ch->blink_period_ms = new_period;
    start_ticks = led_ms_to_ticks(led_half_period(new_period));
  } else {
    ch->blink_enabled = false;
    ch->manual_state = ch->current_state;
  }
  taskEXIT_CRITICAL();

  if (timer != NULL) {
    osTimerStop(timer);
    if (enable) {
      if (start_ticks == 0U) {
        start_ticks = 1U;
      }
      osTimerStart(timer, start_ticks);
    } else {
      led_write_hw((size_t)led, s_led_state[led].current_state);
    }
  }
}

void unit_test_led_set_color(unit_test_led_id_t led, bool on)
{
  if (led >= UNIT_TEST_LED_COUNT) {
    return;
  }

  osTimerId_t timer = NULL;

  taskENTER_CRITICAL();
  led_runtime_t *ch = &s_led_state[led];
  timer = ch->timer;
  ch->blink_enabled = false;
  ch->manual_state = on;
  ch->current_state = on;
  taskEXIT_CRITICAL();

  if (timer != NULL) {
    osTimerStop(timer);
  }
  led_write_hw((size_t)led, on);
}

void unit_test_led_get_status(unit_test_led_status_t *status)
{
  if (status == NULL) {
    return;
  }

  taskENTER_CRITICAL();
  for (size_t i = 0; i < UNIT_TEST_LED_COUNT; ++i) {
    status->blink_enabled[i] = s_led_state[i].blink_enabled;
    status->blink_period_ms[i] = s_led_state[i].blink_period_ms;
    status->led_on[i] = s_led_state[i].current_state;
  }
  taskEXIT_CRITICAL();
}

static void led_write_hw(size_t index, bool state)
{
  if (index >= UNIT_TEST_LED_COUNT) {
    return;
  }
  HAL_GPIO_WritePin(kLedHw[index].port, kLedHw[index].pin, state ? LED_ON_STATE : LED_OFF_STATE);
}

static void led_init_defaults(void)
{
  taskENTER_CRITICAL();
  for (size_t i = 0; i < UNIT_TEST_LED_COUNT; ++i) {
    s_led_state[i].blink_enabled = true;
    s_led_state[i].blink_period_ms = LED_BLINK_PERIOD_DEFAULT_MS;
    s_led_state[i].manual_state = false;
    s_led_state[i].current_state = false;
    s_led_state[i].timer = NULL;
  }
  taskEXIT_CRITICAL();

  for (size_t i = 0; i < UNIT_TEST_LED_COUNT; ++i) {
    led_write_hw(i, false);
  }
}

void led_Task(void *argument)
{
  (void)argument;
  led_init_defaults();
  led_register_shell_command();

  for (size_t i = 0; i < UNIT_TEST_LED_COUNT; ++i) {
    osTimerId_t timer = osTimerNew(led_timer_callback, osTimerOnce, (void *)(uintptr_t)i, NULL);
    if (timer == NULL) {
      Error_Handler();
    }
    s_led_state[i].timer = timer;
    uint32_t ticks = led_ms_to_ticks(led_half_period(s_led_state[i].blink_period_ms));
    if (ticks == 0U) {
      ticks = 1U;
    }
    osTimerStart(timer, ticks);
  }

  for (;;) {
    osDelay(1000U);
  }
}

static void led_print_usage(void)
{
  shell_write_line("Usage:");
  shell_write_line("  led blink <blue|green|pink|all> <on|off> [period_ms]");
  shell_write_line("  led set <blue|green|pink|all> <on|off>");
  shell_write_line("  led status");
}

static bool led_parse_bool(const char *text, bool *value)
{
  if ((text == NULL) || (value == NULL)) {
    return false;
  }
  if ((shell_stricmp(text, "on") == 0) || (shell_stricmp(text, "true") == 0) || (shell_stricmp(text, "1") == 0)) {
    *value = true;
    return true;
  }
  if ((shell_stricmp(text, "off") == 0) || (shell_stricmp(text, "false") == 0) || (shell_stricmp(text, "0") == 0)) {
    *value = false;
    return true;
  }
  return false;
}

static bool led_parse_period(const char *text, uint32_t *period_ms)
{
  if ((text == NULL) || (period_ms == NULL)) {
    return false;
  }
  char *end = NULL;
  unsigned long value = strtoul(text, &end, 10);
  if ((end == text) || (*end != '\0')) {
    return false;
  }
  if ((value < LED_BLINK_PERIOD_MIN_MS) || (value > LED_BLINK_PERIOD_MAX_MS)) {
    return false;
  }
  *period_ms = (uint32_t)value;
  return true;
}

static bool led_parse_target(const char *text, uint32_t *mask)
{
  if ((text == NULL) || (mask == NULL)) {
    return false;
  }
  for (size_t i = 0; i < ARRAY_SIZE(kLedTargets); ++i) {
    if (shell_stricmp(text, kLedTargets[i].name) == 0) {
      *mask = kLedTargets[i].mask;
      return true;
    }
  }
  return false;
}

static void led_register_shell_command(void)
{
  static bool registered = false;
  if (!registered) {
    if (!shell_register_command("led", "Control LEDs (blink/set/status)", led_shell_command)) {
      elog_w("led", "Failed to register shell command.");
    }
    registered = true;
  }
}

static void led_timer_callback(void *argument)
{
  size_t index = (size_t)(uintptr_t)argument;
  if (index >= UNIT_TEST_LED_COUNT) {
    return;
  }

  bool new_state = false;
  uint32_t next_ticks = 0U;

  taskENTER_CRITICAL();
  led_runtime_t *ch = &s_led_state[index];
  if (!ch->blink_enabled) {
    taskEXIT_CRITICAL();
    return;
  }
  ch->current_state = !ch->current_state;
  ch->manual_state = ch->current_state;
  new_state = ch->current_state;
  next_ticks = led_ms_to_ticks(led_half_period(ch->blink_period_ms));
  taskEXIT_CRITICAL();

  led_write_hw(index, new_state);

  if (next_ticks == 0U) {
    next_ticks = 1U;
  }
  osTimerStart(ch->timer, next_ticks);
}

static uint32_t led_half_period(uint32_t period_ms)
{
  uint32_t clamped = led_clamp_period(period_ms);
  uint32_t half = clamped / 2U;
  if (half == 0U) {
    half = 1U;
  }
  return half;
}

static int led_shell_command(int argc, const char * const *argv)
{
  if (argc < 2) {
    led_print_usage();
    return -1;
  }

  if (shell_stricmp(argv[1], "blink") == 0) {
    if (argc < 3) {
      led_print_usage();
      return -1;
    }

    uint32_t mask = 0U;
    const char *state_token = NULL;
    const char *period_token = NULL;
    size_t arg_index = 2U;

    if (!led_parse_target(argv[arg_index], &mask)) {
      shell_write_line("Unknown LED target. Use blue/green/pink/all.");
      return -1;
    }
    arg_index++;
    if (argc <= (int)arg_index) {
      led_print_usage();
      return -1;
    }
    state_token = argv[arg_index++];

    if (argc > (int)arg_index) {
      period_token = argv[arg_index];
    }

    bool enable = false;
    if (!led_parse_bool(state_token, &enable)) {
      shell_write_line("Invalid blink state. Use on/off.");
      return -1;
    }

    uint32_t period = 0U; /* keep current per LED period by default */
    if (period_token != NULL) {
      if (!led_parse_period(period_token, &period)) {
        shell_write_line("Invalid period (50-5000 ms).");
        return -1;
      }
    }

    for (size_t i = 0; i < UNIT_TEST_LED_COUNT; ++i) {
      if ((mask & (1U << i)) == 0U) {
        continue;
      }
      unit_test_led_set_blink((unit_test_led_id_t)i, enable, period);
    }
    shell_write_line("Blink configuration updated.");
    return 0;
  }

  if (shell_stricmp(argv[1], "set") == 0) {
    if (argc < 4) {
      led_print_usage();
      return -1;
    }

    uint32_t mask = 0U;
    if (!led_parse_target(argv[2], &mask)) {
      shell_write_line("Unknown LED target. Use blue/green/pink/all.");
      return -1;
    }

    bool on = false;
    if (!led_parse_bool(argv[3], &on)) {
      shell_write_line("Invalid LED state. Use on/off.");
      return -1;
    }

    for (size_t i = 0; i < UNIT_TEST_LED_COUNT; ++i) {
      if ((mask & (1U << i)) != 0U) {
        unit_test_led_set_color((unit_test_led_id_t)i, on);
      }
    }
    shell_write_line("LED state updated.");
    return 0;
  }

  if (shell_stricmp(argv[1], "status") == 0) {
    unit_test_led_status_t status;
    unit_test_led_get_status(&status);
    char buf[64];
    for (size_t i = 0; i < UNIT_TEST_LED_COUNT; ++i) {
      if (status.blink_enabled[i]) {
        snprintf(buf, sizeof(buf), "  %-5s: blink (%lu ms)", kLedHw[i].label, (unsigned long)status.blink_period_ms[i]);
      } else {
        snprintf(buf, sizeof(buf), "  %-5s: %s", kLedHw[i].label, status.led_on[i] ? "ON" : "OFF");
      }
      shell_write_line(buf);
    }
    return 0;
  }

  led_print_usage();
  return -1;
}

static int shell_stricmp(const char *a, const char *b)
{
  if (a == b) {
    return 0;
  }
  if (a == NULL) {
    return -1;
  }
  if (b == NULL) {
    return 1;
  }
  while ((*a != '\0') && (*b != '\0')) {
    int ca = tolower((unsigned char)*a);
    int cb = tolower((unsigned char)*b);
    if (ca != cb) {
      return ca - cb;
    }
    ++a;
    ++b;
  }
  return tolower((unsigned char)*a) - tolower((unsigned char)*b);
}

static uint32_t led_ms_to_ticks(uint32_t period_ms)
{
  uint32_t tick_freq = osKernelGetTickFreq();
  if (tick_freq == 0U) {
    return period_ms;
  }
  uint64_t ticks = ((uint64_t)period_ms * tick_freq + 999U) / 1000U;
  if (ticks == 0U) {
    ticks = 1U;
  }
  return (uint32_t)ticks;
}
