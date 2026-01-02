## 效果展示

视频待更新。。。。。。。。。。。。。。。

<img src="https://cloud.rocketpi.club/cloud/image-20251116200846693.png" width="400" height="800" />

## 功能说明

使用ADC双轴遥控杆 发送控制指令，通过USB串口连接到电脑上,即可开启打飞机游戏

遥杆上滑，串口发送8，飞机前进，

遥杆下滑，串口发送4，飞机左移，

遥杆右滑，串口发送6，飞机右移，

遥杆左滑，串口发送5，飞机后退

## 使用方法

下载程序到开发板

访问 https://game.rocketpi.club/ （复制到浏览器打开，最好是chrome浏览器或者edg浏览器）  （若画面不全，则刷新一下，等待加载完成即可）

- 点击连接串口

![image-20251205011835874](https://cloud.rocketpi.club/cloud/image-20251205011835874.png)

- 选择ST-Link的串口并连接

![image-20251205012211477](https://cloud.rocketpi.club/cloud/image-20251205012211477.png)

- 确保串口成功连接 ，点击开始游戏即可进入游戏界面，使用遥杆就可以开始飞行大作战了

![image-20251205012357605](https://cloud.rocketpi.club/cloud/image-20251205012357605.png)

## 所需外设

- uart
- adc_joystick(ADC遥杆)

## 硬件连接

PC4--y轴

PC5--x轴

PC13 --key

## CubeMX配置

配置双通道 IN14 IN15对应PC4 PC5

分频配置PCLK2 =84M     84/4 = 21M   =  47.6ns

对 STM32F4 来说，**总转换时间 ≈ 采样时间 + 12.5 个周期**     （112+12.5）x 47.6   每个通道一次转换大约 6 µs 左右

![image-20251116192037813](https://cloud.rocketpi.club/cloud/image-20251116192037813.png)

该双轴遥杆还带一个按键，低电平触发

![image-20251205011146912](https://cloud.rocketpi.club/cloud/image-20251205011146912.png)

<!-- BSP_DRIVERS_START -->
<!-- BSP_DRIVERS_HASH:153604518f3e93b103c556f2cd1f989ee58833ceabdcd62f893b88f87c9c48ca -->
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
#include "adc.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "driver_adc_joystick_test.h"
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
  MX_USART2_UART_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */
	
  adc_joystick_send_udlr_uart(0,20,&huart2); /* 飞行大作战游戏 */
	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
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
```

</details>

<details>
<summary>bsp/adc_joystick/driver_adc_joystick_test.c</summary>

```c
/**
 * @file driver_adc_joystick_test.c
 * @brief 双轴模拟摇杆及按键的阻塞采样辅助函数。
 * @version 1.0.0
 * @date 2025-11-16
 *
 * 本模块提供三项能力：
 *   1. 获取 X/Y 轴的原始值与千分比缩放值。
 *   2. 打印格式化的遥测数据，方便手动调试。
 *   3. 将摇杆偏转转换为小键盘风格的 UART 码（4/6/8/5）。
 */
#include "driver_adc_joystick_test.h"

#include <stdio.h>
#include "gpio.h"

/** @brief hadc1 采集的 12 位 ADC 最大值。 */
#define ADC_JOYSTICK_TEST_ADC_MAX_VALUE   4095U

static uint16_t adc_joystick_test_scale_permille(uint16_t raw);
static void adc_joystick_test_map_direction(const adc_joystick_sample_t *sample, char *out, uint32_t *n);

/**
 * @brief 将 ADC 原始值转换为 0..1000 的千分比（0.1% 分辨率）。
 * @param raw hadc1 返回的 ADC 原始值。
 * @return 千分比缩放值。
 */
static uint16_t adc_joystick_test_scale_permille(uint16_t raw)
{
    uint32_t permille = ((uint32_t)raw * 1000U + (ADC_JOYSTICK_TEST_ADC_MAX_VALUE / 2U)) /
                        ADC_JOYSTICK_TEST_ADC_MAX_VALUE;

    if (permille > 1000U)
    {
        permille = 1000U;
    }

    return (uint16_t)permille;
}

/**
 * @brief 检查摇杆按键是否被按下。
 * @return 按下返回 1（引脚为低），否则返回 0。
 */
static uint8_t adc_joystick_test_read_key_internal(void)
{
    GPIO_PinState state = HAL_GPIO_ReadPin(ADC_JOYSTICK_KEY_GPIO_Port, ADC_JOYSTICK_KEY_Pin);

    return (state == GPIO_PIN_RESET) ? 1U : 0U;
}

/**
 * @brief 获取摇杆两个轴的原始值、千分比和按键状态。
 * @param[out] sample 接收原始计数、千分比和按键标志。
 * @return 成功返回 0，任一 ADC 操作失败返回 1。
 */
uint8_t adc_joystick_test_sample(adc_joystick_sample_t *sample)
{
    HAL_StatusTypeDef status;
    uint16_t raw_x = 0U;
    uint16_t raw_y = 0U;

    if (sample == NULL)
    {
        return 1U;
    }

    status = HAL_ADC_Start(&hadc1);
    if (status != HAL_OK)
    {
        return 1U;
    }

    status = HAL_ADC_PollForConversion(&hadc1, ADC_JOYSTICK_TEST_POLL_TIMEOUT_MS);
    if (status != HAL_OK)
    {
        (void)HAL_ADC_Stop(&hadc1);

        return 1U;
    }
    raw_x = (uint16_t)HAL_ADC_GetValue(&hadc1);

    status = HAL_ADC_PollForConversion(&hadc1, ADC_JOYSTICK_TEST_POLL_TIMEOUT_MS);
    if (status != HAL_OK)
    {
        (void)HAL_ADC_Stop(&hadc1);

        return 1U;
    }
    raw_y = (uint16_t)HAL_ADC_GetValue(&hadc1);

    status = HAL_ADC_Stop(&hadc1);
    if (status != HAL_OK)
    {
        return 1U;
    }

    sample->x.raw = raw_x;
    sample->x.permille = adc_joystick_test_scale_permille(raw_x);
    sample->y.raw = raw_y;
    sample->y.permille = adc_joystick_test_scale_permille(raw_y);
    sample->key_pressed = adc_joystick_test_read_key_internal();

    return 0U;
}

/**
 * @brief 将摇杆采样打印到控制台，便于人工观察。
 * @param sample_count 采样次数，传入 0 则使用默认值。
 * @param delay_ms     采样间隔（毫秒），传入 0 则使用默认值。
 * @return 全部采样成功返回 0，ADC 失败返回 1。
 */
uint8_t adc_joystick_test_run(uint32_t sample_count, uint32_t delay_ms)
{
    const uint32_t count = (sample_count == 0U) ? ADC_JOYSTICK_TEST_DEFAULT_SAMPLE_COUNT : sample_count;
    const uint32_t delay = (delay_ms == 0U) ? ADC_JOYSTICK_TEST_DEFAULT_DELAY_MS : delay_ms;

    for (uint32_t i = 0U; i < count; ++i)
    {
        adc_joystick_sample_t sample;

        if (adc_joystick_test_sample(&sample) != 0U)
        {
            printf("adc joystick: sample %lu failed\r\n", (unsigned long)(i + 1U));

            return 1U;
        }

        printf("adc joystick: #%lu X=%4u (%3lu.%01lu%%) Y=%4u (%3lu.%01lu%%) KEY=%s\r\n",
               (unsigned long)(i + 1U),
               (unsigned int)sample.x.raw,
               (unsigned long)(sample.x.permille / 10U),
               (unsigned long)(sample.x.permille % 10U),
               (unsigned int)sample.y.raw,
               (unsigned long)(sample.y.permille / 10U),
               (unsigned long)(sample.y.permille % 10U),
               (sample.key_pressed != 0U) ? "DOWN" : "UP");

        HAL_Delay(delay);
    }

    return 0U;
}

/** @brief 基于实验数据的 ADC 阈值，用于判定上下左右。 */
#define ADC_X_LEFT_THR    1600U  /**< X 轴偏低判定为左。 */
#define ADC_X_RIGHT_THR   2900U  /**< X 轴偏高判定为右。 */
#define ADC_Y_DOWN_THR    2900U  /**< Y 轴偏高判定为下。 */
#define ADC_Y_UP_THR      1600U  /**< Y 轴偏低判定为上。 */

/**
 * @brief 根据安装方向将物理方向映射为逻辑的左/右/上/下码。
 * @param[in]  sample  当前摇杆采样。
 * @param[out] out     方向输出缓冲区。
 * @param[in,out] n    当前已写入的长度，会根据触发的方向递增。
 */
static void adc_joystick_test_map_direction(const adc_joystick_sample_t *sample, char *out, uint32_t *n)
{
    const uint8_t phys_left  = (sample->x.raw < ADC_X_LEFT_THR)  ? 1U : 0U;
    const uint8_t phys_right = (sample->x.raw > ADC_X_RIGHT_THR) ? 1U : 0U;
    const uint8_t phys_down  = (sample->y.raw > ADC_Y_DOWN_THR)  ? 1U : 0U;
    const uint8_t phys_up    = (sample->y.raw < ADC_Y_UP_THR)    ? 1U : 0U;

#if ADC_JOYSTICK_DEFAULT_ORIENTATION == ADC_JOYSTICK_ORIENTATION_0_DEG
    const uint8_t logical_left  = phys_left;
    const uint8_t logical_right = phys_right;
    const uint8_t logical_up    = phys_up;
    const uint8_t logical_down  = phys_down;
#elif ADC_JOYSTICK_DEFAULT_ORIENTATION == ADC_JOYSTICK_ORIENTATION_90_DEG
    /* 顺时针旋转 90 度：物理左->上，右->下，上->右，下->左。 */
    const uint8_t logical_left  = phys_down;
    const uint8_t logical_right = phys_up;
    const uint8_t logical_up    = phys_left;
    const uint8_t logical_down  = phys_right;
#elif ADC_JOYSTICK_DEFAULT_ORIENTATION == ADC_JOYSTICK_ORIENTATION_180_DEG
    /* 顺时针旋转 180 度：左右、上下互换。 */
    const uint8_t logical_left  = phys_right;
    const uint8_t logical_right = phys_left;
    const uint8_t logical_up    = phys_down;
    const uint8_t logical_down  = phys_up;
#elif ADC_JOYSTICK_DEFAULT_ORIENTATION == ADC_JOYSTICK_ORIENTATION_270_DEG
    /* 顺时针旋转 270 度：物理左->下，右->上，上->左，下->右。 */
    const uint8_t logical_left  = phys_up;
    const uint8_t logical_right = phys_down;
    const uint8_t logical_up    = phys_right;
    const uint8_t logical_down  = phys_left;
#else
#error "ADC_JOYSTICK_DEFAULT_ORIENTATION 配置无效，请检查宏定义。"
#endif

    if (logical_left  != 0U) { out[(*n)++] = '4'; }
    if (logical_right != 0U) { out[(*n)++] = '6'; }
    if (logical_up    != 0U) { out[(*n)++] = '8'; }
    if (logical_down  != 0U) { out[(*n)++] = '5'; }
}

/**
 * @brief 持续发送代表方向的 U/D/L/R ASCII 码。
 * @param sample_count 要发送的采样次数，0 表示持续运行。
 * @param delay_ms     采样间隔（毫秒），0 表示使用默认值。
 * @param huart        用于发送数据的 UART 句柄。
 * @return 返回 0 成功；1 采样失败；2 UART 句柄非法；3 UART 发送失败。
 */
uint8_t adc_joystick_send_udlr_uart(uint32_t sample_count, uint32_t delay_ms, UART_HandleTypeDef *huart)
{
    const uint32_t delay = (delay_ms == 0U) ? ADC_JOYSTICK_TEST_DEFAULT_DELAY_MS : delay_ms;

    if (huart == NULL) {
        return 2U;
    }

    if (sample_count == 0U)
    {
        /* 0 表示一直运行，便于快速交互测试。 */
        while (1)
        {
            adc_joystick_sample_t sample;

            if (adc_joystick_test_sample(&sample) != 0U) {
                return 1U;
            }

            char out[4];
            uint32_t n = 0U;

            /* 方向映射遵循小键盘：4=左，6=右，8=上，5=下，宏可配置旋转角度。 */
            adc_joystick_test_map_direction(&sample, out, &n);
            if (n > 0U) {
                if (HAL_UART_Transmit(huart, (uint8_t *)out, (uint16_t)n, 10U) != HAL_OK) {
                    return 3U;
                }
            }

            HAL_Delay(delay);
        }
    }
    else
    {
        /* 有限运行：按固定次数采样。 */
        for (uint32_t i = 0U; i < sample_count; ++i)
        {
            adc_joystick_sample_t sample;

            if (adc_joystick_test_sample(&sample) != 0U) {
                return 1U;
            }

            char out[4];
            uint32_t n = 0U;

            adc_joystick_test_map_direction(&sample, out, &n);
            if (n > 0U) {
                if (HAL_UART_Transmit(huart, (uint8_t *)out, (uint16_t)n, 10U) != HAL_OK) {
                    return 3U;
                }
            }

            HAL_Delay(delay);
        }

        return 0U;
    }
}

/**
 * @brief 仅读取摇杆按键状态的便捷封装。
 * @return 按下返回 1，否则返回 0。
 */
uint8_t adc_joystick_test_read_key(void)
{
    return adc_joystick_test_read_key_internal();
}
```

</details>

<details>
<summary>bsp/adc_joystick/driver_adc_joystick_test.h</summary>

```c
/**
 * @file driver_adc_joystick_test.h
 * @brief 双轴摇杆采样辅助（ADC 通道 14/15 加按键输入）。
 * @version 1.0.0
 * @date 2025-11-16
 */
#pragma once

#include "adc.h"
#include "usart.h"

/**
 * @brief 轮询每次 ADC 转换时的超时时间（毫秒）。
 */
#define ADC_JOYSTICK_TEST_POLL_TIMEOUT_MS        10U

/**
 * @brief 传入 0 时使用的默认采样次数。
 */
#define ADC_JOYSTICK_TEST_DEFAULT_SAMPLE_COUNT   20U

/**
 * @brief 传入 0 时默认的采样间隔（毫秒）。
 */
#define ADC_JOYSTICK_TEST_DEFAULT_DELAY_MS       50U

/**
 * @brief 摇杆安装方向（以“左=4/右=6/上=8/下=5”的逻辑方向为基准）。
 *        当摇杆模块顺时针旋转 90/180/270 度安装时，选择对应宏即可自动映射方向。
 */
#define ADC_JOYSTICK_ORIENTATION_0_DEG    0U  /**< 默认方向，无旋转。 */
#define ADC_JOYSTICK_ORIENTATION_90_DEG   1U  /**< 顺时针旋转 90 度。 */
#define ADC_JOYSTICK_ORIENTATION_180_DEG  2U  /**< 顺时针旋转 180 度。 */
#define ADC_JOYSTICK_ORIENTATION_270_DEG  3U  /**< 顺时针旋转 270 度。 */

/**
 * @brief 预设的摇杆方向选择，若未覆盖则使用 0 度。
 *        用户可在项目配置中覆写该宏以适配硬件放置方向。
 */
#ifndef ADC_JOYSTICK_DEFAULT_ORIENTATION
#define ADC_JOYSTICK_DEFAULT_ORIENTATION ADC_JOYSTICK_ORIENTATION_180_DEG
#endif

/**
 * @brief 单个摇杆轴的快照
 */
typedef struct
{
    uint16_t raw;        /**< ADC 原始值（0..4095） */
    uint16_t permille;   /**< 位置缩放到 0..1000（0.1% 分辨率） */
} adc_joystick_axis_sample_t;

/**
 * @brief 摇杆 X/Y 双轴的快照
 */
typedef struct
{
    adc_joystick_axis_sample_t x; /**< X 轴数据（ADC 通道 14） */
    adc_joystick_axis_sample_t y; /**< Y 轴数据（ADC 通道 15） */
    uint8_t key_pressed;          /**< 摇杆按键被按下时为 1 */
} adc_joystick_sample_t;

/**
 * @brief     采集一次摇杆数据（双轴加按键）。
 * @param[out] sample 填充原始计数、千分比以及按键标志。
 * @return    状态码
 *            - 0 成功
 *            - 1 采集失败
 */
uint8_t adc_joystick_test_sample(adc_joystick_sample_t *sample);

/**
 * @brief     持续采样并通过 printf 打印摇杆读数。
 * @param[in] sample_count 打印的采样次数，0 使用默认值。
 * @param[in] delay_ms     采样间隔（毫秒），0 使用默认值。
 * @return    状态码
 *            - 0 成功
 *            - 1 采集失败
 */
uint8_t adc_joystick_test_run(uint32_t sample_count, uint32_t delay_ms);

/**
 * @brief     持续采样摇杆，并通过 UART 发送小键盘方向码。
 * @param[in] sample_count 处理的采样次数，0 表示默认/持续运行。
 * @param[in] delay_ms     采样间隔（毫秒），0 使用默认值。
 * @param[in] huart        用于发送的已初始化 UART 句柄。
 * @return    状态码
 *            - 0 成功
 *            - 1 采集失败（摇杆采样错误）
 *            - 2 无效的 UART 句柄（空指针）
 *            - 3 UART 发送失败
 */
uint8_t adc_joystick_send_udlr_uart(uint32_t sample_count, uint32_t delay_ms, UART_HandleTypeDef *huart);


/**
 * @brief 读取摇杆按键状态。
 * @return 按下返回 1，否则返回 0
 */
uint8_t adc_joystick_test_read_key(void);
```

</details>

<!-- BSP_DRIVERS_END -->
