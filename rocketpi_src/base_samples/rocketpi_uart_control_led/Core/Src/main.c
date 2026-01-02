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
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define UART_RX_BUFFER_SIZE 128U
#define LED_NAME_MAX_LEN    16U
#define LED_TOKEN_COUNT     3U

#define LED_ACTIVE_LOW      1

#if (LED_ACTIVE_LOW == 1)
#define LED_ON_STATE        GPIO_PIN_RESET
#define LED_OFF_STATE       GPIO_PIN_SET
#else
#define LED_ON_STATE        GPIO_PIN_SET
#define LED_OFF_STATE       GPIO_PIN_RESET
#endif
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
typedef struct
{
  GPIO_TypeDef *port;
  uint16_t pin;
  const char *tokens[LED_TOKEN_COUNT];
} LedConfig_t;

static const LedConfig_t g_led_config[] =
{
  {LED_B_GPIO_Port, LED_B_Pin, {"B", "BLUE", "LED_B"}},
  {LED_G_GPIO_Port, LED_G_Pin, {"G", "GREEN", "LED_G"}},
  {LED_P_GPIO_Port, LED_P_Pin, {"P", "PINK", "LED_P"}}
};

enum { LED_TOTAL = sizeof(g_led_config) / sizeof(g_led_config[0]) };

typedef struct
{
  uint8_t led_index[LED_TOTAL];
  bool states[LED_TOTAL];
  size_t led_count;
  size_t state_count;
} LedCommand_t;

static char g_rx_buffer[UART_RX_BUFFER_SIZE];
static size_t g_rx_length = 0U;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
static void console_send_string(const char *msg);
static void console_send_prompt(void);
static void console_print_examples(void);
static void console_print_gpio_map(void);
static const char *console_skip_spaces(const char *ptr);
static const char *console_find_json_value(const char *json, const char *key);
static bool console_parse_led_targets(const char *json, LedCommand_t *cmd);
static bool console_parse_state_values(const char *json, LedCommand_t *cmd);
static bool console_parse_led_token(const char **cursor, LedCommand_t *cmd);
static bool console_parse_state_token(const char **cursor, LedCommand_t *cmd);
static bool console_add_led_index(LedCommand_t *cmd, uint8_t index);
static uint32_t console_pin_index(uint16_t pin);
static const char *console_port_name(GPIO_TypeDef *port);
static bool console_str_case_equal(const char *lhs, const char *rhs);
static int8_t console_find_led_by_token(const char *token);
static void console_set_led_state(uint8_t index, bool enabled);
static void console_apply_command(const LedCommand_t *cmd);
static void console_report_result(const LedCommand_t *cmd);
static void console_report_error(const char *message);
static void console_handle_input_byte(uint8_t data);
static void console_process_command_buffer(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* 阻塞方式发送字符串到调试串口 */
static void console_send_string(const char *msg)
{
  if (msg == NULL)
  {
    return;
  }
  HAL_UART_Transmit(&huart2, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
}

/* 输出下一条命令的提示符 */
static void console_send_prompt(void)
{
  console_send_string("\r\n> ");
}

/* 打印串口示例命令 */
static void console_print_examples(void)
{
  console_send_string("RocketPi UART LED console ready.\r\n");
  console_send_string("Example commands:\r\n");
  console_send_string("  {\"led\":\"B\",\"state\":1}\r\n");
  console_send_string("  {\"led\":[\"B\",\"G\"],\"state\":[1,0]}\r\n");
}

/* 返回 GPIO 端口的可读名称 */
static const char *console_port_name(GPIO_TypeDef *port)
{
  if (port == GPIOA) return "GPIOA";
  if (port == GPIOB) return "GPIOB";
  if (port == GPIOC) return "GPIOC";
  if (port == GPIOD) return "GPIOD";
  if (port == GPIOE) return "GPIOE";
  if (port == GPIOH) return "GPIOH";
  return "UNKNOWN";
}

/* 将位图形式的引脚值转换成编号 */
static uint32_t console_pin_index(uint16_t pin)
{
  for (uint32_t i = 0U; i < 16U; ++i)
  {
    if (pin == (uint16_t)(1U << i))
    {
      return i;
    }
  }
  return 0xFFFFFFFFU;
}

/* 上电时输出 LED 与 GPIO 的映射信息 */
static void console_print_gpio_map(void)
{
  console_send_string("{\"led_config\":[");
  for (size_t i = 0U; i < LED_TOTAL; ++i)
  {
    char buffer[96];
    const uint32_t pin_index = console_pin_index(g_led_config[i].pin);
    const char *delimiter = (i + 1U < LED_TOTAL) ? "," : "";
    (void)snprintf(buffer, sizeof(buffer),
                   "{\"name\":\"%s\",\"port\":\"%s\",\"pin\":%lu}%s",
                   g_led_config[i].tokens[0],
                   console_port_name(g_led_config[i].port),
                   (unsigned long)((pin_index <= 15U) ? pin_index : g_led_config[i].pin),
                   delimiter);
    console_send_string(buffer);
  }
  console_send_string("]}\r\n");
}

/* 跳过当前字符串指针中的所有空白字符 */
static const char *console_skip_spaces(const char *ptr)
{
  while ((ptr != NULL) && (*ptr != '\0') && isspace((unsigned char)*ptr))
  {
    ++ptr;
  }
  return ptr;
}

/* 在伪 JSON 字符串中定位指定键的值 */
static const char *console_find_json_value(const char *json, const char *key)
{
  if ((json == NULL) || (key == NULL))
  {
    return NULL;
  }
  const char *location = strstr(json, key);
  if (location == NULL)
  {
    return NULL;
  }
  location += strlen(key);
  location = console_skip_spaces(location);
  if (*location != ':')
  {
    return NULL;
  }
  ++location;
  return console_skip_spaces(location);
}

/* 不区分大小写比较两个字符串 */
static bool console_str_case_equal(const char *lhs, const char *rhs)
{
  if ((lhs == NULL) || (rhs == NULL))
  {
    return false;
  }
  while ((*lhs != '\0') && (*rhs != '\0'))
  {
    const int ca = toupper((unsigned char)*lhs);
    const int cb = toupper((unsigned char)*rhs);
    if (ca != cb)
    {
      return false;
    }
    ++lhs;
    ++rhs;
  }
  return (*lhs == '\0') && (*rhs == '\0');
}

/* 根据别名查找 LED 在配置表中的索引 */
static int8_t console_find_led_by_token(const char *token)
{
  if (token == NULL)
  {
    return -1;
  }

  for (size_t i = 0U; i < LED_TOTAL; ++i)
  {
    for (size_t alias = 0U; alias < LED_TOKEN_COUNT; ++alias)
    {
      const char *candidate = g_led_config[i].tokens[alias];
      if ((candidate != NULL) && console_str_case_equal(token, candidate))
      {
        return (int8_t)i;
      }
    }
  }

  return -1;
}

/* 向解析结果中追加一个 LED 索引（避免重复） */
static bool console_add_led_index(LedCommand_t *cmd, uint8_t index)
{
  if (cmd == NULL)
  {
    return false;
  }

  for (size_t i = 0U; i < cmd->led_count; ++i)
  {
    if (cmd->led_index[i] == index)
    {
      return true;
    }
  }

  if (cmd->led_count >= LED_TOTAL)
  {
    return false;
  }

  cmd->led_index[cmd->led_count++] = index;
  return true;
}

/* 解析单个字符串 token 并写入 LED 列表 */
static bool console_parse_led_token(const char **cursor, LedCommand_t *cmd)
{
  if ((cursor == NULL) || (*cursor == NULL) || (cmd == NULL))
  {
    return false;
  }

  const char *ptr = *cursor;
  if (*ptr != '"')
  {
    return false;
  }
  ++ptr;

  const char *start = ptr;
  while ((*ptr != '\0') && (*ptr != '"'))
  {
    ++ptr;
  }
  if (*ptr != '"')
  {
    return false;
  }

  const size_t length = (size_t)(ptr - start);
  if ((length == 0U) || (length >= LED_NAME_MAX_LEN))
  {
    return false;
  }

  char token[LED_NAME_MAX_LEN] = {0};
  memcpy(token, start, length);
  token[length] = '\0';
  ++ptr;

  if (console_str_case_equal(token, "ALL"))
  {
    for (size_t i = 0U; i < LED_TOTAL; ++i)
    {
      if (!console_add_led_index(cmd, (uint8_t)i))
      {
        return false;
      }
    }
    *cursor = ptr;
    return true;
  }

  const int8_t led_index = console_find_led_by_token(token);
  if (led_index < 0)
  {
    return false;
  }

  if (!console_add_led_index(cmd, (uint8_t)led_index))
  {
    return false;
  }

  *cursor = ptr;
  return true;
}

/* 解析 led 字段（支持字符串或数组） */
static bool console_parse_led_targets(const char *json, LedCommand_t *cmd)
{
  if ((json == NULL) || (cmd == NULL))
  {
    return false;
  }

  const char *value = console_find_json_value(json, "\"led\"");
  if (value == NULL)
  {
    value = console_find_json_value(json, "\"LED\"");
  }
  if (value == NULL)
  {
    return false;
  }

  if (*value == '[')
  {
    ++value;
    while (true)
    {
      value = console_skip_spaces(value);
      if (*value == ']')
      {
        ++value;
        break;
      }
      if (!console_parse_led_token(&value, cmd))
      {
        return false;
      }
      value = console_skip_spaces(value);
      if (*value == ',')
      {
        ++value;
        continue;
      }
      if (*value == ']')
      {
        ++value;
        break;
      }
      return false;
    }
  }
  else if (*value == '"')
  {
    if (!console_parse_led_token(&value, cmd))
    {
      return false;
    }
  }
  else
  {
    return false;
  }

  return (cmd->led_count > 0U);
}

/* 解析 state 的单个值，支持数字与文本 */
static bool console_parse_state_token(const char **cursor, LedCommand_t *cmd)
{
  if ((cursor == NULL) || (*cursor == NULL) || (cmd == NULL))
  {
    return false;
  }

  const char *ptr = *cursor;
  bool value = false;
  bool parsed = false;

  if (*ptr == '"')
  {
    ++ptr;
    const char *start = ptr;
    while ((*ptr != '\0') && (*ptr != '"'))
    {
      ++ptr;
    }
    if (*ptr != '"')
    {
      return false;
    }
    const size_t length = (size_t)(ptr - start);
    if ((length == 0U) || (length >= LED_NAME_MAX_LEN))
    {
      return false;
    }
    char token[LED_NAME_MAX_LEN] = {0};
    memcpy(token, start, length);
    token[length] = '\0';
    ++ptr;

    if (console_str_case_equal(token, "on") || console_str_case_equal(token, "true") ||
        console_str_case_equal(token, "enable"))
    {
      value = true;
      parsed = true;
    }
    else if (console_str_case_equal(token, "off") || console_str_case_equal(token, "false") ||
             console_str_case_equal(token, "disable"))
    {
      value = false;
      parsed = true;
    }
  }
  else if (isalpha((unsigned char)*ptr))
  {
    const char *start = ptr;
    while (isalpha((unsigned char)*ptr))
    {
      ++ptr;
    }
    const size_t length = (size_t)(ptr - start);
    if ((length == 0U) || (length >= LED_NAME_MAX_LEN))
    {
      return false;
    }
    char token[LED_NAME_MAX_LEN] = {0};
    memcpy(token, start, length);
    token[length] = '\0';
    if (console_str_case_equal(token, "on") || console_str_case_equal(token, "true"))
    {
      value = true;
      parsed = true;
    }
    else if (console_str_case_equal(token, "off") || console_str_case_equal(token, "false"))
    {
      value = false;
      parsed = true;
    }
  }
  else
  {
    bool negative = false;
    if (*ptr == '-')
    {
      negative = true;
      ++ptr;
    }
    if (!isdigit((unsigned char)*ptr))
    {
      return false;
    }
    uint32_t number = 0U;
    while (isdigit((unsigned char)*ptr))
    {
      number = (number * 10U) + (uint32_t)(*ptr - '0');
      ++ptr;
    }
    value = (!negative) && (number != 0U);
    parsed = true;
  }

  if (!parsed)
  {
    return false;
  }

  if (cmd->state_count >= LED_TOTAL)
  {
    return false;
  }

  cmd->states[cmd->state_count++] = value;
  *cursor = ptr;
  return true;
}

/* 解析 state 字段（单值或数组） */
static bool console_parse_state_values(const char *json, LedCommand_t *cmd)
{
  if ((json == NULL) || (cmd == NULL))
  {
    return false;
  }

  const char *value = console_find_json_value(json, "\"state\"");
  if (value == NULL)
  {
    value = console_find_json_value(json, "\"STATE\"");
  }
  if (value == NULL)
  {
    return false;
  }

  if (*value == '[')
  {
    ++value;
    while (true)
    {
      value = console_skip_spaces(value);
      if (*value == ']')
      {
        ++value;
        break;
      }
      if (!console_parse_state_token(&value, cmd))
      {
        return false;
      }
      value = console_skip_spaces(value);
      if (*value == ',')
      {
        ++value;
        continue;
      }
      if (*value == ']')
      {
        ++value;
        break;
      }
      return false;
    }
  }
  else
  {
    if (!console_parse_state_token(&value, cmd))
    {
      return false;
    }
  }

  return (cmd->state_count > 0U);
}

/* 操作具体 LED 引脚输出状态 */
static void console_set_led_state(uint8_t index, bool enabled)
{
  if (index >= LED_TOTAL)
  {
    return;
  }
  HAL_GPIO_WritePin(g_led_config[index].port,
                    g_led_config[index].pin,
                    enabled ? LED_ON_STATE : LED_OFF_STATE);
}

/* 根据解析结果批量设置 LED */
static void console_apply_command(const LedCommand_t *cmd)
{
  if (cmd == NULL)
  {
    return;
  }

  for (size_t i = 0U; i < cmd->led_count; ++i)
  {
    const size_t state_index = (cmd->state_count == 1U) ? 0U : i;
    const bool desired = cmd->states[state_index];
    console_set_led_state(cmd->led_index[i], desired);
  }
}

/* 输出成功执行后的状态 JSON */
static void console_report_result(const LedCommand_t *cmd)
{
  if (cmd == NULL)
  {
    return;
  }

  char buffer[192];
  int length = snprintf(buffer, sizeof(buffer), "{\"status\":\"ok\",\"led\":[");
  for (size_t i = 0U; i < cmd->led_count; ++i)
  {
    const char *delimiter = (i + 1U < cmd->led_count) ? "," : "";
    length += snprintf(&buffer[length], (size_t)(sizeof(buffer) - (size_t)length),
                       "\"%s\"%s",
                       g_led_config[cmd->led_index[i]].tokens[0],
                       delimiter);
  }
  length += snprintf(&buffer[length], (size_t)(sizeof(buffer) - (size_t)length),
                     "],\"state\":[");
  for (size_t i = 0U; i < cmd->led_count; ++i)
  {
    const size_t state_index = (cmd->state_count == 1U) ? 0U : i;
    const char *delimiter = (i + 1U < cmd->led_count) ? "," : "";
    length += snprintf(&buffer[length], (size_t)(sizeof(buffer) - (size_t)length),
                       "%u%s",
                       cmd->states[state_index] ? 1U : 0U,
                       delimiter);
  }
  (void)snprintf(&buffer[length], (size_t)(sizeof(buffer) - (size_t)length), "]}\r\n");
  console_send_string(buffer);
}

/* 输出错误 JSON 信息 */
static void console_report_error(const char *message)
{
  char buffer[160];
  (void)snprintf(buffer, sizeof(buffer),
                 "{\"status\":\"error\",\"msg\":\"%s\"}\r\n",
                 (message != NULL) ? message : "unknown");
  console_send_string(buffer);
}

/* 处理完整命令字符串并执行 */
static void console_process_command_buffer(void)
{
  g_rx_buffer[g_rx_length] = '\0';
  if (g_rx_length == 0U)
  {
    return;
  }

  LedCommand_t command = {0};
  if (!console_parse_led_targets(g_rx_buffer, &command))
  {
    console_report_error("missing led field");
    return;
  }
  if (!console_parse_state_values(g_rx_buffer, &command))
  {
    console_report_error("missing state field");
    return;
  }
  if (!((command.state_count == 1U) || (command.state_count == command.led_count)))
  {
    console_report_error("state count mismatch");
    return;
  }

  console_apply_command(&command);
  console_report_result(&command);
}

/* 串口逐字节处理函数，负责拼接命令 */
static void console_handle_input_byte(uint8_t data)
{
  if (data == '\r')
  {
    return;
  }
  if (data == '\n')
  {
    console_process_command_buffer();
    g_rx_length = 0U;
    console_send_prompt();
    return;
  }

  if (g_rx_length >= (UART_RX_BUFFER_SIZE - 1U))
  {
    g_rx_length = 0U;
    console_report_error("command too long");
    console_send_prompt();
    return;
  }

  g_rx_buffer[g_rx_length++] = (char)data;
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
/* 主程序入口：初始化外设并处理串口命令 */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  console_print_examples();
  console_print_gpio_map();
  console_send_prompt();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    uint8_t byte = 0U;
    if (HAL_UART_Receive(&huart2, &byte, 1U, HAL_MAX_DELAY) == HAL_OK)
    {
      console_handle_input_byte(byte);
    }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
/* 配置芯片的时钟树参数 */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
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

  /** Initializes the CPU, AHB and APB buses clocks
  */
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

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* 用户可在此处扩展错误处理逻辑 */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
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
  /* USER CODE BEGIN 6 */
  /* 可以在此处打印断言失败的源码文件和行号 */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
