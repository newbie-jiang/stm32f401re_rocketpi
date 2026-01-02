/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "microrl.h"
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct {
  const char *name;
  const char *description;
} shell_cmd_desc_t;

typedef struct {
  const char *name;
  GPIO_TypeDef *port;
  uint16_t pin;
} led_desc_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define SHELL_RX_BUFFER_SIZE 128U
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define LED_ON_STATE GPIO_PIN_RESET
#define LED_OFF_STATE GPIO_PIN_SET
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
static microrl_t s_shell;
static uint8_t s_uart2_rx;
static volatile uint8_t s_shell_rx_buffer[SHELL_RX_BUFFER_SIZE];
static volatile uint16_t s_shell_rx_head;
static volatile uint16_t s_shell_rx_tail;

static const shell_cmd_desc_t kShellCmdTable[] = {
  {"help", "List available commands"},
  {"version", "Show shell information"},
  {"echo", "Echo back the provided text"},
  {"led", "Control on-board LEDs"},
};

static const led_desc_t kLedTable[] = {
  {"blue", LED_B_GPIO_Port, LED_B_Pin},
  {"green", LED_G_GPIO_Port, LED_G_Pin},
  {"pink", LED_P_GPIO_Port, LED_P_Pin},
};

static const char *kLedActionTable[] = {"on", "off", "toggle"};
static const char *s_shell_completion[_COMMAND_TOKEN_NMB + 1];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
static void shell_print(const char *str);
static void shell_print_line(const char *text);
static void shell_show_banner(void);
static void shell_start_rx(void);
static void shell_queue_char(uint8_t value);
static void shell_process_input(void);
static int shell_execute(int argc, const char * const * argv);
static int shell_cmd_led(int argc, const char * const * argv);
static void shell_print_led_usage(void);
static int shell_str_casecmp(const char *lhs, const char *rhs);
static const led_desc_t *shell_find_led(const char *name);
static size_t shell_add_completion(const char *candidate, size_t index);
static int shell_prefix_match(const char *token, const char *candidate);
static char **shell_complete(int argc, const char * const * argv);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* 通过 USART2 输出字符串（LF 自动转换为 CRLF） */
static void shell_print(const char *str)
{
  if ((str == NULL) || (huart2.gState == HAL_UART_STATE_RESET)) {
    return;
  }
  while (*str != '\0') {
    if (*str == '\n') {
      const uint8_t newline[2] = {'\r', '\n'};
      HAL_UART_Transmit(&huart2, (uint8_t *)newline, sizeof(newline), HAL_MAX_DELAY);
    } else {
      uint8_t ch = (uint8_t)(*str);
      HAL_UART_Transmit(&huart2, &ch, 1, HAL_MAX_DELAY);
    }
    str++;
  }
}

/* 打印一行文本并追加换行 */
static void shell_print_line(const char *text)
{
  if (text != NULL) {
    shell_print(text);
  }
  shell_print(ENDL);
}

/* 输出开机提示语 */
static void shell_show_banner(void)
{
  shell_print_line("");
  shell_print_line("RocketPi USART2 microrl shell ready.");
}

/* 启动一次 USART2 中断接收 */
static void shell_start_rx(void)
{
  HAL_UART_Receive_IT(&huart2, &s_uart2_rx, 1);
}

/* 将接收的字节写入环形缓冲区 */
static void shell_queue_char(uint8_t value)
{
  uint16_t next_head = (uint16_t)((s_shell_rx_head + 1U) % SHELL_RX_BUFFER_SIZE);
  if (next_head != s_shell_rx_tail) {
    s_shell_rx_buffer[s_shell_rx_head] = value;
    s_shell_rx_head = next_head;
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

  if (strcmp(argv[0], "help") == 0) {
    shell_print_line("Available commands:");
    for (size_t i = 0; i < ARRAY_SIZE(kShellCmdTable); ++i) {
      shell_print("  ");
      shell_print(kShellCmdTable[i].name);
      shell_print(" - ");
      shell_print(kShellCmdTable[i].description);
      shell_print(ENDL);
    }
    return 0;
  }

  if (strcmp(argv[0], "version") == 0) {
    shell_print("microrl ");
    shell_print(MICRORL_LIB_VER);
    shell_print_line(" on USART2 (115200 8N1)");
    return 0;
  }

  if (strcmp(argv[0], "echo") == 0) {
    for (int i = 1; i < argc; ++i) {
      shell_print(argv[i]);
      if (i < (argc - 1)) {
        shell_print(" ");
      }
    }
    shell_print(ENDL);
    return 0;
  }

  if (strcmp(argv[0], "led") == 0) {
    return shell_cmd_led(argc, argv);
  }

  shell_print("Unknown command: ");
  shell_print(argv[0]);
  shell_print(ENDL);
  shell_print_line("Type 'help' to list commands.");
  return 0;
}

/* LED 命令：led <颜色> <on|off|toggle> */
static int shell_cmd_led(int argc, const char * const * argv)
{
  if (argc < 3) {
    shell_print_led_usage();
    return -1;
  }

  const led_desc_t *led = shell_find_led(argv[1]);
  if (led == NULL) {
    shell_print("Unknown LED name: ");
    shell_print(argv[1]);
    shell_print(ENDL);
    shell_print_led_usage();
    return -1;
  }

  const char *action = argv[2];
  if (shell_str_casecmp(action, "on") == 0) {
    HAL_GPIO_WritePin(led->port, led->pin, LED_ON_STATE);
    shell_print("LED ");
    shell_print(led->name);
    shell_print_line(" is ON");
    return 0;
  }

  if (shell_str_casecmp(action, "off") == 0) {
    HAL_GPIO_WritePin(led->port, led->pin, LED_OFF_STATE);
    shell_print("LED ");
    shell_print(led->name);
    shell_print_line(" is OFF");
    return 0;
  }

  if (shell_str_casecmp(action, "toggle") == 0) {
    HAL_GPIO_TogglePin(led->port, led->pin);
    shell_print("LED ");
    shell_print(led->name);
    shell_print_line(" toggled");
    return 0;
  }

  shell_print("Unknown LED action: ");
  shell_print(action);
  shell_print(ENDL);
  shell_print_led_usage();
  return -1;
}

/* 打印 LED 命令可选参数 */
static void shell_print_led_usage(void)
{
  shell_print_line("Usage: led <blue|green|pink> <on|off|toggle>");
}

/* 忽略大小写比较字符串 */
static int shell_str_casecmp(const char *lhs, const char *rhs)
{
  while ((*lhs != '\0') && (*rhs != '\0')) {
    char lc = (*lhs >= 'A' && *lhs <= 'Z') ? (char)(*lhs + ('a' - 'A')) : *lhs;
    char rc = (*rhs >= 'A' && *rhs <= 'Z') ? (char)(*rhs + ('a' - 'A')) : *rhs;
    if (lc != rc) {
      return (int)(lc - rc);
    }
    ++lhs;
    ++rhs;
  }
  return (int)((unsigned char)*lhs - (unsigned char)*rhs);
}

/* 在 LED 描述表中查找指定名称 */
static const led_desc_t *shell_find_led(const char *name)
{
  for (size_t i = 0; i < ARRAY_SIZE(kLedTable); ++i) {
    if (shell_str_casecmp(name, kLedTable[i].name) == 0) {
      return &kLedTable[i];
    }
  }
  return NULL;
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
    for (size_t i = 0; i < ARRAY_SIZE(kShellCmdTable); ++i) {
      const char *name = kShellCmdTable[i].name;
      if (shell_prefix_match(current, name)) {
        idx = shell_add_completion(name, idx);
      }
    }
    return (char **)s_shell_completion;
  }

  if (strcmp(argv[0], "led") != 0) {
    return (char **)s_shell_completion;
  }

  if (argc == 2) {
    for (size_t i = 0; i < ARRAY_SIZE(kLedTable); ++i) {
      if (shell_prefix_match(current, kLedTable[i].name)) {
        idx = shell_add_completion(kLedTable[i].name, idx);
      }
    }
  } else if (argc == 3) {
    for (size_t i = 0; i < ARRAY_SIZE(kLedActionTable); ++i) {
      if (shell_prefix_match(current, kLedActionTable[i])) {
        idx = shell_add_completion(kLedActionTable[i], idx);
      }
    }
  }

  return (char **)s_shell_completion;
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_USART2_UART_Init();

  shell_show_banner();
  microrl_init(&s_shell, shell_print);
  microrl_set_execute_callback(&s_shell, shell_execute);
  microrl_set_complete_callback(&s_shell, shell_complete);
  shell_start_rx();

  while (1)
  {
    shell_process_input();
  }
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 84;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
/* UART 接收完成回调：收集数据并继续接收 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART2) {
    shell_queue_char(s_uart2_rx);
    shell_start_rx();
  }
}

/* UART 出错回调：重启接收以保持 shell 可用 */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART2) {
    shell_start_rx();
  }
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
}
#endif /* USE_FULL_ASSERT */
