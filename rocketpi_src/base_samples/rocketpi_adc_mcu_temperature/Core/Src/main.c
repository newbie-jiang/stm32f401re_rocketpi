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
#include "adc.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
static HAL_StatusTypeDef read_mcu_temperature(float *temperature_c);
static void print_temperature(float temperature_c);
static void print_temperature_read_error(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#define ADC_CALIB_VREF_VOLTAGE     3.3f
#define TEMPSENSOR_CAL1_TEMP_C    30.0f
#define TEMPSENSOR_CAL2_TEMP_C   110.0f
#define TEMPSENSOR_TYP_V25         0.76f
#define TEMPSENSOR_TYP_AVG_SLOPE   0.0025f
#define ADC_12BIT_FULL_SCALE    4095.0f
#ifndef VREFINT_CAL_ADDR
#define VREFINT_CAL_ADDR      ((uint16_t*)(0x1FFF7A2AU))
#endif
#ifndef TEMPSENSOR_CAL1_ADDR
#define TEMPSENSOR_CAL1_ADDR  ((uint16_t*)(0x1FFF7A2CU))
#endif
#ifndef TEMPSENSOR_CAL2_ADDR
#define TEMPSENSOR_CAL2_ADDR  ((uint16_t*)(0x1FFF7A2EU))
#endif
#define ADC_CALIB_READ16(addr) (*((volatile uint16_t *)(addr)))
#define ADC_MEDIAN_SAMPLE_COUNT    5U
#define ADC_CONVERSION_TIMEOUT_MS 10U

/* 中值滤波中使用的简单插入排序，样本数较小时效率尚可。 */
static void insertion_sort(uint32_t *data, uint32_t length)
{
  for (uint32_t i = 1U; i < length; ++i)
  {
    uint32_t key = data[i];
    int32_t j = (int32_t)i - 1;
    while ((j >= 0) && (data[j] > key))
    {
      data[j + 1] = data[j];
      --j;
    }
    data[j + 1] = key;
  }
}

/*
 * 读取 MCU 内置温度与 VREFINT，结合校准常数计算校准后的摄氏温度：
 * 1. 连续触发 ADC，获取多组温度/基准电压结果。
 * 2. 对两组原始数据分别做插入排序并取中值，抑制瞬时噪声。
 * 3. 使用 VREFINT 工厂校准常数推算当前 VDDA 后，将温度采样值映射到
 *    TS_CAL1/TS_CAL2 所定义的 30°C~110°C 直线，得到实际温度。
 * 4. 若校准常数缺失，则退化为使用典型 V25/斜率的粗略计算。
 */
static HAL_StatusTypeDef read_mcu_temperature(float *temperature_c)
{
  if (temperature_c == NULL)
  {
    return HAL_ERROR;
  }

  HAL_StatusTypeDef status = HAL_OK;
  uint32_t temp_samples[ADC_MEDIAN_SAMPLE_COUNT];
  uint32_t vref_samples[ADC_MEDIAN_SAMPLE_COUNT];

  for (uint32_t i = 0U; i < ADC_MEDIAN_SAMPLE_COUNT; ++i)
  {
    status = HAL_ADC_Start(&hadc1);
    if (status != HAL_OK)
    {
      goto cleanup;
    }

    status = HAL_ADC_PollForConversion(&hadc1, ADC_CONVERSION_TIMEOUT_MS);
    if (status != HAL_OK)
    {
      goto cleanup;
    }

    temp_samples[i] = HAL_ADC_GetValue(&hadc1);

    status = HAL_ADC_PollForConversion(&hadc1, ADC_CONVERSION_TIMEOUT_MS);
    if (status != HAL_OK)
    {
      goto cleanup;
    }

    vref_samples[i] = HAL_ADC_GetValue(&hadc1);
  }

cleanup:
  HAL_ADC_Stop(&hadc1);
  if (status != HAL_OK)
  {
    return status;
  }

  insertion_sort(temp_samples, ADC_MEDIAN_SAMPLE_COUNT);
  insertion_sort(vref_samples, ADC_MEDIAN_SAMPLE_COUNT);

  uint32_t raw_temp = temp_samples[ADC_MEDIAN_SAMPLE_COUNT / 2U];
  uint32_t raw_vrefint = vref_samples[ADC_MEDIAN_SAMPLE_COUNT / 2U];

  if (raw_vrefint == 0U)
  {
    return HAL_ERROR;
  }

  uint16_t vrefint_cal = ADC_CALIB_READ16(VREFINT_CAL_ADDR);
  uint16_t ts_cal1 = ADC_CALIB_READ16(TEMPSENSOR_CAL1_ADDR);
  uint16_t ts_cal2 = ADC_CALIB_READ16(TEMPSENSOR_CAL2_ADDR);

  if (vrefint_cal == 0U)
  {
    return HAL_ERROR;
  }

  float vdda = ADC_CALIB_VREF_VOLTAGE * ((float)vrefint_cal / (float)raw_vrefint);

  if ((ts_cal1 != 0U) && (ts_cal2 != 0U) && (ts_cal2 != ts_cal1))
  {
    float temp_at_cal_vdda = ((float)raw_temp * vdda) / ADC_CALIB_VREF_VOLTAGE;
    *temperature_c = ((temp_at_cal_vdda - (float)ts_cal1) *
                      (TEMPSENSOR_CAL2_TEMP_C - TEMPSENSOR_CAL1_TEMP_C) /
                      ((float)ts_cal2 - (float)ts_cal1)) +
                     TEMPSENSOR_CAL1_TEMP_C;
  }
  else
  {
    float vsense = ((float)raw_temp / ADC_12BIT_FULL_SCALE) * vdda;
    *temperature_c = ((TEMPSENSOR_TYP_V25 - vsense) / TEMPSENSOR_TYP_AVG_SLOPE) + 25.0f;
  }

  return HAL_OK;
}

/* 将温度格式化成字符串并通过 USART2 输出。 */
static void print_temperature(float temperature_c)
{
  char buffer[64];
  int length = snprintf(buffer, sizeof(buffer), "MCU Temp: %.2f C\r\n", temperature_c);
  if (length <= 0)
  {
    return;
  }

  uint16_t bytes_to_send = (length < (int)sizeof(buffer)) ? (uint16_t)length : (uint16_t)(sizeof(buffer) - 1U);
  HAL_UART_Transmit(&huart2, (uint8_t *)buffer, bytes_to_send, HAL_MAX_DELAY);
}

/* 当温度采集失败时输出错误信息，便于调试。 */
static void print_temperature_read_error(void)
{
  static const char error_message[] = "MCU Temp read failed\r\n";
  HAL_UART_Transmit(&huart2, (uint8_t *)error_message, sizeof(error_message) - 1U, HAL_MAX_DELAY);
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
  MX_USART2_UART_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    float temperature_c = 0.0f;
    if (read_mcu_temperature(&temperature_c) == HAL_OK)
    {
      print_temperature(temperature_c);
    }
    else
    {
      print_temperature_read_error();
    }

    HAL_Delay(1000);
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
