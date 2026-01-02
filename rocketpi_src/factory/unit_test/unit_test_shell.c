#include "unit_test.h"
#include "microrl.h"
#include "usbd_cdc_if.h"

extern USBD_HandleTypeDef hUsbDeviceFS;

typedef struct {
  const char *name;
  const char *description;
  shell_cmd_handler_t handler;
} shell_cmd_entry_t;

#define SHELL_RX_BUFFER_SIZE 128U
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define LED_ON_STATE GPIO_PIN_RESET
#define LED_OFF_STATE GPIO_PIN_SET
#define SHELL_USB_TX_CHUNK_SIZE CDC_DATA_FS_MAX_PACKET_SIZE
#define SHELL_USB_TX_MAX_RETRY 5U
#define SHELL_MAX_COMMANDS 32U
#define SHELL_TX_BUFFER_SIZE 1024U
#define SHELL_TX_FLUSH_CHUNK 128U


static microrl_t s_shell;
static uint8_t s_uart2_rx;
static volatile uint8_t s_shell_rx_buffer[SHELL_RX_BUFFER_SIZE];
static volatile uint16_t s_shell_rx_head;
static volatile uint16_t s_shell_rx_tail;
static shell_cmd_entry_t s_shell_commands[SHELL_MAX_COMMANDS];
static size_t s_shell_cmd_count;
static const char *s_shell_completion[_COMMAND_TOKEN_NMB + 1];
static uint8_t s_shell_tx_buffer[SHELL_TX_BUFFER_SIZE];
static volatile uint16_t s_shell_tx_head = 0U;
static volatile uint16_t s_shell_tx_tail = 0U;

static bool shell_usb_ready(void)
{
  return (hUsbDeviceFS.dev_state == USBD_STATE_CONFIGURED) &&
         (hUsbDeviceFS.pClassData != NULL);
}

static const shell_cmd_entry_t *shell_find_command(const char *name)
{
  if (name == NULL) {
    return NULL;
  }
  for (size_t i = 0; i < s_shell_cmd_count; ++i) {
    if ((s_shell_commands[i].name != NULL) && (strcmp(s_shell_commands[i].name, name) == 0)) {
      return &s_shell_commands[i];
    }
  }
  return NULL;
}

bool shell_register_command(const char *name, const char *description, shell_cmd_handler_t handler)
{
  if ((name == NULL) || (handler == NULL)) {
    return false;
  }

  bool added = false;
  taskENTER_CRITICAL();
  if (shell_find_command(name) == NULL) {
    if (s_shell_cmd_count < SHELL_MAX_COMMANDS) {
      s_shell_commands[s_shell_cmd_count].name = name;
      s_shell_commands[s_shell_cmd_count].description = description;
      s_shell_commands[s_shell_cmd_count].handler = handler;
      s_shell_cmd_count++;
      added = true;
    }
  }
  taskEXIT_CRITICAL();
  return added;
}

static void shell_uart_write_blocking(const uint8_t *data, size_t len)
{
  if ((data == NULL) || (len == 0U) || (huart2.gState == HAL_UART_STATE_RESET)) {
    return;
  }

  while (len > 0U) {
    uint16_t chunk = (len > 0xFFFFU) ? 0xFFFFU : (uint16_t)len;
    HAL_UART_Transmit(&huart2, (uint8_t *)data, chunk, HAL_MAX_DELAY);
    data += chunk;
    len -= chunk;
  }
}

static void shell_usb_write_blocking(const uint8_t *data, size_t len)
{
  if ((data == NULL) || (len == 0U) || !shell_usb_ready()) {
    return;
  }

  while (len > 0U) {
    uint16_t chunk = (len > SHELL_USB_TX_CHUNK_SIZE) ? SHELL_USB_TX_CHUNK_SIZE : (uint16_t)len;
    uint32_t attempt = 0U;
    while (attempt < SHELL_USB_TX_MAX_RETRY) {
      uint8_t status = CDC_Transmit_FS((uint8_t *)data, chunk);
      if (status == USBD_OK) {
        break;
      }
      if (status != USBD_BUSY) {
        return;
      }
      ++attempt;
      osDelay(1);
    }

    if (attempt >= SHELL_USB_TX_MAX_RETRY) {
      /* USB CDC is still busy (e.g., host port unopened); skip remaining data to keep USART responsive. */
      break;
    }

    data += chunk;
    len -= chunk;
  }
}

static void shell_tx_enqueue_bytes(const uint8_t *data, size_t len)
{
  if ((data == NULL) || (len == 0U)) {
    return;
  }

  taskENTER_CRITICAL();
  for (size_t i = 0; i < len; ++i) {
    uint16_t next_head = (uint16_t)((s_shell_tx_head + 1U) % SHELL_TX_BUFFER_SIZE);
    if (next_head == s_shell_tx_tail) {
      s_shell_tx_tail = (uint16_t)((s_shell_tx_tail + 1U) % SHELL_TX_BUFFER_SIZE);
    }
    s_shell_tx_buffer[s_shell_tx_head] = data[i];
    s_shell_tx_head = next_head;
  }
  taskEXIT_CRITICAL();
}

static size_t shell_tx_dequeue_bytes(uint8_t *out, size_t max_len)
{
  if ((out == NULL) || (max_len == 0U)) {
    return 0U;
  }

  size_t count = 0U;
  taskENTER_CRITICAL();
  while ((count < max_len) && (s_shell_tx_tail != s_shell_tx_head)) {
    out[count++] = s_shell_tx_buffer[s_shell_tx_tail];
    s_shell_tx_tail = (uint16_t)((s_shell_tx_tail + 1U) % SHELL_TX_BUFFER_SIZE);
  }
  taskEXIT_CRITICAL();
  return count;
}

static void shell_flush_output(void)
{
  uint8_t chunk[SHELL_TX_FLUSH_CHUNK];
  size_t read = shell_tx_dequeue_bytes(chunk, sizeof(chunk));
  while (read > 0U) {
    shell_uart_write_blocking(chunk, read);
    shell_usb_write_blocking(chunk, read);
    read = shell_tx_dequeue_bytes(chunk, sizeof(chunk));
  }
}

/* 通过 USART2/USB CDC 输出字符串（LF 自动转换为 CRLF） */
static void shell_output_raw(const char *str)
{
  if (str == NULL) {
    return;
  }

  static const uint8_t newline[2] = {'\r', '\n'};
  while (*str != '\0') {
    if (*str == '\n') {
      shell_tx_enqueue_bytes(newline, sizeof(newline));
    } else {
      uint8_t ch = (uint8_t)(*str);
      shell_tx_enqueue_bytes(&ch, 1U);
    }
    str++;
  }
}

void shell_write(const char *text)
{
  shell_output_raw(text);
}

void shell_write_line(const char *text)
{
  if (text != NULL) {
    shell_output_raw(text);
  }
  shell_output_raw(ENDL);
}

static int shell_cmd_help(int argc, const char * const *argv)
{
  (void)argc;
  (void)argv;
  shell_write_line("Available commands:");
  for (size_t i = 0; i < s_shell_cmd_count; ++i) {
    const shell_cmd_entry_t *entry = &s_shell_commands[i];
    if (entry->name == NULL) {
      continue;
    }
    shell_write("  ");
    shell_write(entry->name);
    if (entry->description != NULL) {
      shell_write(" - ");
      shell_write(entry->description);
    }
    shell_write(ENDL);
  }
  return 0;
}

static int shell_cmd_version(int argc, const char * const *argv)
{
  (void)argc;
  (void)argv;
  shell_write("microrl ");
  shell_write(MICRORL_LIB_VER);
  shell_write_line(" on USART2 (115200 8N1) & USB CDC (FS)");
  return 0;
}

static int shell_cmd_echo(int argc, const char * const *argv)
{
  if (argc <= 1) {
    shell_write(ENDL);
    return 0;
  }

  for (int i = 1; i < argc; ++i) {
    shell_write(argv[i]);
    if (i < (argc - 1)) {
      shell_write(" ");
    }
  }
  shell_write(ENDL);
  return 0;
}

static void shell_register_builtin_commands(void)
{
  shell_register_command("help", "List available commands", shell_cmd_help);
  shell_register_command("version", "Show shell information", shell_cmd_version);
  shell_register_command("echo", "Echo back the provided text", shell_cmd_echo);
}

/* 输出开机提示语 */
static void shell_show_banner(void)
{
//  shell_write_line("");
//  shell_write_line("Rocket-Pi microrl shell ready on USART2 & USB CDC.");
	elog_i("unit_test_shell", "Rocket-Pi microrl shell ready on USART2 & USB CDC.");
}

/* 启动一次 USART2 中断接收 */
static void shell_start_rx(void)
{
  HAL_UART_Receive_IT(&huart2, &s_uart2_rx, 1);
}

/* 将接收的字节写入环形缓冲区 */
static void shell_queue_char(uint8_t value)
{
  uint32_t primask = __get_PRIMASK();
  __disable_irq();
  uint16_t next_head = (uint16_t)((s_shell_rx_head + 1U) % SHELL_RX_BUFFER_SIZE);
  if (next_head != s_shell_rx_tail) {
    s_shell_rx_buffer[s_shell_rx_head] = value;
    s_shell_rx_head = next_head;
  }
  if (primask == 0U) {
    __enable_irq();
  }
}

void shell_push_input_stream(const uint8_t *buffer, size_t length)
{
  if (buffer == NULL) {
    return;
  }

  for (size_t i = 0; i < length; ++i) {
    shell_queue_char(buffer[i]);
  }
}

/* 在主循环中解析缓冲区数据 */
static void shell_process_input(void)
{
  while (s_shell_rx_head != s_shell_rx_tail) {
    uint8_t ch = s_shell_rx_buffer[s_shell_rx_tail];
    s_shell_rx_tail = (uint16_t)((s_shell_rx_tail + 1U) % SHELL_RX_BUFFER_SIZE);
    if (ch == '\r') {
      ch = '\n';
      if (s_shell_rx_head != s_shell_rx_tail) {
        uint8_t peek = s_shell_rx_buffer[s_shell_rx_tail];
        if (peek == '\n') {
          s_shell_rx_tail = (uint16_t)((s_shell_rx_tail + 1U) % SHELL_RX_BUFFER_SIZE);
        }
      }
    }
    microrl_insert_char(&s_shell, ch);
  }
}

/* microrl 执行回调，根据命令执行功能 */
static int shell_execute(int argc, const char * const * argv)
{
  if (argc <= 0) {
    return 0;
  }

  const shell_cmd_entry_t *cmd = shell_find_command(argv[0]);
  if ((cmd != NULL) && (cmd->handler != NULL)) {
    return cmd->handler(argc, argv);
  }

  shell_write("Unknown command: ");
  shell_write(argv[0]);
  shell_write(ENDL);
  shell_write_line("Type 'help' to list commands.");
  return -1;
}



/* 把候选词添加到补全数组 */
static size_t shell_add_completion(const char *candidate, size_t index)
{
  if ((candidate != NULL) && (index < (ARRAY_SIZE(s_shell_completion) - 1U))) {
    s_shell_completion[index++] = candidate;
  }
  return index;
}

/* 判断 token 是否是候选词的前缀 */
static int shell_prefix_match(const char *token, const char *candidate)
{
  if ((token == NULL) || (*token == '\0')) {
    return 1;
  }
  size_t prefix_len = strlen(token);
  return strncmp(candidate, token, prefix_len) == 0;
}

/* TAB 自动补全回调 */
static char **shell_complete(int argc, const char * const * argv)
{
  size_t idx = 0;
  memset((void *)s_shell_completion, 0, sizeof(s_shell_completion));

  const char *current = "";
  if (argc > 0) {
    current = argv[argc - 1];
    if (current == NULL) {
      current = "";
    }
  }

  if (argc <= 1) {
    for (size_t i = 0; i < s_shell_cmd_count; ++i) {
      const char *name = s_shell_commands[i].name;
      if ((name != NULL) && shell_prefix_match(current, name)) {
        idx = shell_add_completion(name, idx);
      }
    }
  }

  return (char **)s_shell_completion;
}



/* UART 接收完成回调：收集数据并继续接收 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART2) {
    shell_queue_char(s_uart2_rx);
    shell_start_rx();
  }
}


 extern bool uart_dma_idle;
/* UART 出错回调：重启接收以保持 shell 可用 */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART2) {
    shell_start_rx();
		uart_dma_idle = true; /* easylogger */
  }
}



static void elog_demo_all_levels(void) {
  elog_raw("EasyLogger raw log output demo.");
  elog_a("main", "Assert level demo log.");
  elog_e("main", "Error level demo log.");
  elog_w("main", "Warn level demo log.");
  elog_i("main", "Info level demo log.");
  elog_d("main", "Debug level demo log.");
  elog_v("main", "Verbose level demo log.");
}


void shell_Task(void *argument)
{
	shell_show_banner();
  microrl_init(&s_shell, shell_write);
  microrl_set_execute_callback(&s_shell, shell_execute);
  microrl_set_complete_callback(&s_shell, shell_complete);
  shell_register_builtin_commands();
  shell_start_rx();
	
	 if (elog_init() == ELOG_NO_ERR) {
    elog_set_fmt(ELOG_LVL_ASSERT, ELOG_FMT_ALL & ~ELOG_FMT_P_INFO);
    elog_set_fmt(ELOG_LVL_ERROR, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
    elog_set_fmt(ELOG_LVL_WARN, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
    elog_set_fmt(ELOG_LVL_INFO, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
    elog_set_fmt(ELOG_LVL_DEBUG, ELOG_FMT_ALL & ~(ELOG_FMT_FUNC | ELOG_FMT_P_INFO));
    elog_set_fmt(ELOG_LVL_VERBOSE, ELOG_FMT_ALL & ~(ELOG_FMT_FUNC | ELOG_FMT_P_INFO));
    elog_start();
    elog_i("unit_test_shell", "EasyLogger initialized. DMA UART logging ready.");
//    elog_demo_all_levels();
  } else {
    Error_Handler();
  }
	
	
  for(;;)
  {
		shell_process_input();
    shell_flush_output();
    osDelay(1);
  }
}
