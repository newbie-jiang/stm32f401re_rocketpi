## rocketpi_aht30_hardware_i2c_base

## 效果展示

![image-20251116004609250](https://cloud.rocketpi.club/cloud/image-20251116004609250.png)

## 功能说明

面向 RocketPI STM32F401RE 开发板的 **AHT30温湿度 演示工程**。主要特性：

- 驱动AHT30在串口上打印温湿度。
- 提供 `driver_aht30` 基础驱动。
- `driver_aht30_test` 直接调用测试，自主选择轮询时间。

## 硬件连接

- AHT32 SCL ：PB8
- AHT32 SDA：PB9

## CubeMX配置

### 硬件i2c配置 

- （无特别说明不配置dma与中断）

![image-20251116002705838](https://cloud.rocketpi.club/cloud/image-20251116002705838.png)

![image-20251116002929989](https://cloud.rocketpi.club/cloud/image-20251116002929989.png)

### usart配置

![image-20251116004045780](https://cloud.rocketpi.club/cloud/image-20251116004045780.png)

![image-20251116003215745](https://cloud.rocketpi.club/cloud/image-20251116003215745.png)

<!-- BSP_DRIVERS_START -->
<!-- BSP_DRIVERS_HASH:c60800ac1d8a324c7a982b181d9f67cf18edb1efaa50224cca5fe056f2658f0d -->
## 驱动以及测试代码

<details>
<summary>Core/Src/main.c</summary>

```c
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
#include "i2c.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "driver_aht30_test.h"
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

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
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
  MX_I2C1_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  printf("Initializing AHT30...\r\n");
  if (aht30_init() != HAL_OK)
  {
    printf("AHT30 init failed\r\n");
  }
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    aht30_test_log_measurement();
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
```

</details>

<details>
<summary>bsp/aht30/driver_aht30.c</summary>

```c
#include "driver_aht30.h"

#include "i2c.h"

/**
 * @file driver_aht30.c
 * @brief AHT30温湿度传感器的硬件I2C驱动实现。
 * @author rocket
 */

#define AHT30_CMD_TRIGGER     0xACU /**< 测量触发指令ID。 */
#define AHT30_CMD_CONFIG_0    0x33U /**< 触发命令配置字节0的默认值。 */
#define AHT30_CMD_CONFIG_1    0x00U /**< 触发命令配置字节1的默认值。 */
#define AHT30_CMD_RESET       0xBAU /**< 软复位指令ID。 */
#define AHT30_STATUS_BUSY     0x80U /**< 表示正在测量的状态位。 */

/**
 * @brief 通过I2C执行软复位。
 * @return HAL_I2C_Master_Transmit返回的HAL状态码。
 */
static HAL_StatusTypeDef aht30_soft_reset(void)
{
    uint8_t cmd = AHT30_CMD_RESET;
    return HAL_I2C_Master_Transmit(&hi2c1, AHT30_I2C_ADDRESS, &cmd, 1U, HAL_MAX_DELAY);
}

/**
 * @brief 将原始20位湿度与温度数据转换为工程量单位。
 * @param raw 从传感器读取的输入缓冲区。
 * @param temperature_c 输出的摄氏温度。
 * @param humidity_pct 输出的相对湿度百分比。
 */
static void aht30_convert_samples(const uint8_t raw[6], float *temperature_c, float *humidity_pct)
{
    uint32_t raw_humidity = ((uint32_t)raw[1] << 12)
                          | ((uint32_t)raw[2] << 4)
                          | (uint32_t)(raw[3] >> 4);

    uint32_t raw_temperature = (((uint32_t)raw[3] & 0x0FU) << 16)
                             | ((uint32_t)raw[4] << 8)
                             | (uint32_t)raw[5];

    *humidity_pct = (raw_humidity * 100.0f) / 1048576.0f;
    *temperature_c = (raw_temperature * 200.0f) / 1048576.0f - 50.0f;
}

/**
 * @brief 执行基本初始化流程（上电延时+软复位）。
 */
HAL_StatusTypeDef aht30_init(void)
{
    HAL_Delay(AHT30_POWER_ON_DELAY);
    HAL_StatusTypeDef status = aht30_soft_reset();
    HAL_Delay(AHT30_POST_RESET_DELAY);
    return status;
}

/**
 * @brief 触发一次测量并读取6字节原始数据。
 */
HAL_StatusTypeDef aht30_read_raw(uint8_t raw[6])
{
    uint8_t cmd[3] = {AHT30_CMD_TRIGGER, AHT30_CMD_CONFIG_0, AHT30_CMD_CONFIG_1};
    HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(&hi2c1, AHT30_I2C_ADDRESS, cmd, sizeof(cmd), HAL_MAX_DELAY);
    if (status != HAL_OK) {
        return status;
    }

    HAL_Delay(AHT30_MEASUREMENT_DELAY);
    status = HAL_I2C_Master_Receive(&hi2c1, AHT30_I2C_ADDRESS, raw, 6U, HAL_MAX_DELAY);
    if (status != HAL_OK) {
        return status;
    }

    if ((raw[0] & AHT30_STATUS_BUSY) != 0U) {
        return HAL_BUSY;
    }

    return HAL_OK;
}

/**
 * @brief 读取并转换一组温湿度数据。
 */
HAL_StatusTypeDef aht30_read(float *temperature_c, float *humidity_pct)
{
    if ((temperature_c == NULL) || (humidity_pct == NULL)) {
        return HAL_ERROR;
    }

    uint8_t raw[6] = {0};
    HAL_StatusTypeDef status = aht30_read_raw(raw);
    if (status != HAL_OK) {
        return status;
    }

    aht30_convert_samples(raw, temperature_c, humidity_pct);
    return HAL_OK;
}
```

</details>

<details>
<summary>bsp/aht30/driver_aht30.h</summary>

```c
#pragma once

#include <stdint.h>

#include "stm32f4xx_hal.h"

/**
 * @file driver_aht30.h
 * @brief AHT30温湿度传感器的硬件I2C驱动。
 * @author rocket
 * @copyright 2025 rocket版权所有，仅限授权使用。
 */

#define AHT30_I2C_ADDRESS        (0x38U << 1) /**< 为HAL使用而左移的7位地址。 */
#define AHT30_MEASUREMENT_DELAY  80U          /**< 典型的测量转换延时（毫秒）。 */
#define AHT30_POWER_ON_DELAY     20U          /**< 上电后发送命令前的最小等待时间（毫秒）。 */
#define AHT30_POST_RESET_DELAY   20U          /**< 发出软复位后的等待时间（毫秒）。 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 通过软复位并遵守时序要求来初始化传感器。
 * @return 成功返回HAL_OK，否则返回I2C事务产生的HAL错误/忙状态码。
 */
HAL_StatusTypeDef aht30_init(void);

/**
 * @brief 触发测量并读取6字节原始数据。
 * @param raw 指向6字节缓冲区的指针，用于存放传感器输出。
 * @return 成功返回HAL_OK，若传感器报告忙则返回HAL_BUSY，否则返回HAL错误码。
 */
HAL_StatusTypeDef aht30_read_raw(uint8_t raw[6]);

/**
 * @brief 读取并转换当前温度和湿度。
 * @param temperature_c 输出的摄氏温度指针。
 * @param humidity_pct 输出的相对湿度（百分比）指针。
 * @return 成功返回HAL_OK，测量中返回HAL_BUSY，失败则返回HAL错误码。
 */
HAL_StatusTypeDef aht30_read(float *temperature_c, float *humidity_pct);

#ifdef __cplusplus
}
#endif
```

</details>

<details>
<summary>bsp/aht30/driver_aht30_test.c</summary>

```c
#include "driver_aht30_test.h"

#include <stdio.h>
#include <stdlib.h>

/**
 * @file driver_aht30_test.c
 * @brief AHT30驱动的日志辅助与测量演示。
 * @author rocket
 */

/**
 * @brief 打印带简短前缀的HAL状态码。
 * @param phase 描述失败操作的文本。
 * @param status 驱动返回的HAL状态。
 */
static void aht30_test_print_status(const char *phase, HAL_StatusTypeDef status)
{
    printf("AHT30 %s error (status=%d)\r\n", phase, (int)status);
}

/**
 * @brief 获取一次测量、完成转换并输出可读结果。
 * @return 成功返回HAL_OK，传感器测量中返回HAL_BUSY，失败则返回HAL错误码。
 */
HAL_StatusTypeDef aht30_test_log_measurement(void)
{
    float temperature = 0.0f;
    float humidity = 0.0f;
    HAL_StatusTypeDef status = aht30_read(&temperature, &humidity);
    if (status == HAL_OK) {
        int16_t temp10 = (int16_t)(temperature * 10.0f);
        uint16_t hum10 = (uint16_t)(humidity * 10.0f);
        printf("AHT30 -> T=%d.%01dC  RH=%d.%01d%%\r\n",
               temp10 / 10, abs(temp10 % 10),
               hum10 / 10, hum10 % 10);
    } else if (status == HAL_BUSY) {
        printf("AHT30 measurement busy\r\n");
    } else {
        aht30_test_print_status("read", status);
    }

    return status;
}

/**
 * @brief 读取6字节原始数据并打印，便于调试。
 * @return 成功捕获数据返回HAL_OK，设备仍在测量时返回HAL_BUSY，否则返回HAL错误码。
 */
HAL_StatusTypeDef aht30_test_log_raw(void)
{
    uint8_t raw[6] = {0};
    HAL_StatusTypeDef status = aht30_read_raw(raw);
    if (status == HAL_OK) {
        printf("AHT30 raw -> %02X %02X %02X %02X %02X %02X (cal=%s)\r\n",
               raw[0], raw[1], raw[2], raw[3], raw[4], raw[5],
               ((raw[0] & 0x08U) != 0U) ? "yes" : "no");
    } else if (status == HAL_BUSY) {
        printf("AHT30 raw request busy\r\n");
    } else {
        aht30_test_print_status("raw", status);
    }

    return status;
}
```

</details>

<details>
<summary>bsp/aht30/driver_aht30_test.h</summary>

```c
#pragma once

#include "driver_aht30.h"

/**
 * @file driver_aht30_test.h
 * @brief 用于测试和记录AHT30驱动的辅助函数。
 * @author rocket
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 进行一次测量并通过printf输出转换值。
 * @return 采样有效时返回HAL_OK，传感器仍在测量时返回HAL_BUSY，否则返回HAL错误码。
 */
HAL_StatusTypeDef aht30_test_log_measurement(void);

/**
 * @brief 读取原始帧并输出状态/字节序列以便调试。
 * @return 成功获取原始字节返回HAL_OK，传感器仍在测量时返回HAL_BUSY，否则返回HAL错误码。
 */
HAL_StatusTypeDef aht30_test_log_raw(void);

#ifdef __cplusplus
}
#endif
```

</details>

<!-- BSP_DRIVERS_END -->
