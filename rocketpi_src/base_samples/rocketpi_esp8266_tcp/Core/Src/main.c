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
#include "dma.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#define MQTT_TRACE_ENABLED 1
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "driver_esp8266_at.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#ifndef ESP8266_TCP_TEST_WIFI_SSID
#define ESP8266_TCP_TEST_WIFI_SSID        "ZTE-45c476"
#endif
#ifndef ESP8266_TCP_TEST_WIFI_PASSWORD
#define ESP8266_TCP_TEST_WIFI_PASSWORD    "88888888"
#endif
#ifndef ESP8266_TCP_TEST_REMOTE_HOST
#define ESP8266_TCP_TEST_REMOTE_HOST      "192.168.1.77"
#endif
#ifndef ESP8266_TCP_TEST_REMOTE_PORT
#define ESP8266_TCP_TEST_REMOTE_PORT      8899U
#endif

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
static bool s_esp8266_tcp_link_ready = false;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
static void esp8266_tcp_test_run(void);
static void esp8266_tcp_test_poll(void);
static void esp8266_tcp_print_ipd_payload(const esp8266_at_event_t *event);
static void esp8266_tcp_log_status(const char *label, esp8266_at_status_t status);
static void esp8266_tcp_print_ip_info(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static void esp8266_tcp_log_status(const char *label, esp8266_at_status_t status)
{
  if (label == NULL)
  {
    return;
  }
  printf("[ESP8266][TCP][%s] %s\r\n", label, esp8266_at_status_string(status));
}

static void esp8266_tcp_print_ip_info(void)
{
  esp8266_at_ip_info_t ip_info;
  esp8266_at_status_t status = esp8266_at_query_ip_info(&ip_info);
  esp8266_tcp_log_status("CIPSTA?", status);
  if (status != ESP8266_AT_STATUS_OK)
  {
    return;
  }

  printf("[ESP8266][IP] STA ip=%s gateway=%s netmask=%s\r\n",
         ip_info.station_ip,
         ip_info.station_gateway,
         ip_info.station_netmask);
  printf("[ESP8266][IP] AP  ip=%s gateway=%s netmask=%s\r\n",
         ip_info.softap_ip,
         ip_info.softap_gateway,
         ip_info.softap_netmask);
}

static void esp8266_tcp_print_ipd_payload(const esp8266_at_event_t *event)
{
  if (event == NULL)
  {
    return;
  }

  const char *raw_line = event->raw_line;
  if (raw_line == NULL || raw_line[0] == '\0')
  {
    return;
  }

  const char *payload = NULL;
  unsigned int channel = 0U;
  unsigned int payload_length = 0U;
  bool channel_valid = false;
  bool length_valid = false;
  int payload_offset = 0;

  if (sscanf(raw_line, "+IPD,%u,%u:%n", &channel, &payload_length, &payload_offset) == 2)
  {
    channel_valid = true;
    length_valid = true;
    payload = raw_line + payload_offset;
  }
  else if (sscanf(raw_line, "+IPD,%u:%n", &payload_length, &payload_offset) == 1)
  {
    length_valid = true;
    payload = raw_line + payload_offset;
  }
  else
  {
    const char *colon = strchr(raw_line, ':');
    if (colon != NULL)
    {
      payload = colon + 1;
    }
  }

  if (payload == NULL || *payload == '\0')
  {
    printf("[ESP8266][TCP RX] (empty payload)\r\n");
    return;
  }

  char buffer[ESP8266_AT_MAX_LINE_LENGTH];
  size_t available = strlen(payload);
  size_t copy_len = available;

  if (length_valid && payload_length < copy_len)
  {
    copy_len = payload_length;
  }

  if (copy_len >= sizeof(buffer))
  {
    copy_len = sizeof(buffer) - 1U;
  }

  memcpy(buffer, payload, copy_len);
  buffer[copy_len] = '\0';

  if (!length_valid)
  {
    payload_length = (unsigned int)copy_len;
  }

  if (channel_valid)
  {
    printf("[ESP8266][TCP RX][id=%u len=%u] %s\r\n",
           channel,
           payload_length,
           buffer);
  }
  else
  {
    printf("[ESP8266][TCP RX][len=%u] %s\r\n",
           payload_length,
           buffer);
  }
}

static void esp8266_tcp_test_run(void)
{
  printf("\r\n=== ESP8266 TCP client test ===\r\n");
  s_esp8266_tcp_link_ready = false;

  esp8266_at_status_t status = esp8266_at_init();
  esp8266_tcp_log_status("init", status);
  if (status != ESP8266_AT_STATUS_OK)
  {
    return;
  }

  esp8266_at_clear_events();

  status = esp8266_at_disable_echo(true);
  esp8266_tcp_log_status("ATE0", status);
  if (status != ESP8266_AT_STATUS_OK)
  {
    return;
  }

  status = esp8266_at_set_wifi_mode(1U, false);
  esp8266_tcp_log_status("CWMODE", status);
  if (status != ESP8266_AT_STATUS_OK)
  {
    return;
  }

  status = esp8266_at_connect_ap(ESP8266_TCP_TEST_WIFI_SSID,
                                 ESP8266_TCP_TEST_WIFI_PASSWORD,
                                 20000U,
                                 false);
  esp8266_tcp_log_status("CWJAP", status);
  if (status != ESP8266_AT_STATUS_OK)
  {
    return;
  }
  esp8266_tcp_print_ip_info();

  status = esp8266_at_send_command(ESP8266_AT_CMD_CIPMUX,
                                   ESP8266_AT_COMMAND_MODE_SET,
                                   "0",
                                   ESP8266_AT_DEFAULT_TIMEOUT_MS,
                                   false);
  esp8266_tcp_log_status("CIPMUX", status);
  if (status != ESP8266_AT_STATUS_OK)
  {
    return;
  }

  char start_args[ESP8266_AT_MAX_LINE_LENGTH];
  (void)snprintf(start_args,
                 sizeof(start_args),
                 "\"TCP\",\"%s\",%u",
                 ESP8266_TCP_TEST_REMOTE_HOST,
                 (unsigned int)ESP8266_TCP_TEST_REMOTE_PORT);

  status = esp8266_at_send_command(ESP8266_AT_CMD_CIPSTART,
                                   ESP8266_AT_COMMAND_MODE_SET,
                                   start_args,
                                   ESP8266_AT_DEFAULT_TIMEOUT_MS * 5U,
                                   false);
  esp8266_tcp_log_status("CIPSTART", status);
  if (status == ESP8266_AT_STATUS_OK)
  {
    s_esp8266_tcp_link_ready = true;
    printf("[ESP8266][TCP] connected to %s:%u\r\n",
           ESP8266_TCP_TEST_REMOTE_HOST,
           (unsigned int)ESP8266_TCP_TEST_REMOTE_PORT);
    printf("[ESP8266][TCP] waiting for incoming data...\r\n");
  }
}

static void esp8266_tcp_test_poll(void)
{
  esp8266_at_event_t event;

  esp8266_at_poll();

  while (esp8266_at_fetch_event(&event))
  {
    if (strncmp(event.raw_line, "+IPD", 4) == 0)
    {
      if (s_esp8266_tcp_link_ready)
      {
        esp8266_tcp_print_ipd_payload(&event);
      }
      continue;
    }

    if (strstr(event.raw_line, "CLOSED") != NULL)
    {
      if (s_esp8266_tcp_link_ready)
      {
        printf("[ESP8266][TCP] connection closed\r\n");
      }
      s_esp8266_tcp_link_ready = false;
      continue;
    }
  }
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
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
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_USART6_UART_Init();
  /* USER CODE BEGIN 2 */
	HAL_Delay(2000);
	
  esp8266_tcp_test_run();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		
		esp8266_tcp_test_poll();
		HAL_Delay(10);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
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
  RCC_OscInitStruct.PLL.PLLQ = 7;
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
  /* User can add his own implementation to report the HAL error return state */
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
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
