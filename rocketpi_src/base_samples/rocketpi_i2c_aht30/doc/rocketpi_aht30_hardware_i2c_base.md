## rocketpi_aht30_hardware_i2c_base















## 效果展示















![image-20251116004609250](https://cloud.rocketpi.club/cloud/image-20251116004609250.png)















## 功能说明



面向 RocketPI STM32F401RE 开发板的 **AHT30温湿度 演示工程**。主要特性：



- 驱动AHT30在串口上打印温湿度。

- 提供 `driver_aht30` 基础驱动。

- `driver_aht30_test` 直接调用测试，自主选择轮询时间。
- `driver_aht30_config.h` ?????/??I2C?????????GPIO?????

- `soft_i2c` 提供通用GPIO bit-bang I2C主机，可复用到其它外设。



## 硬件连接















- AHT32 SCL ：PB8







- AHT32 SDA：PB9







- 电源：接3.3v或5v















## CubeMX配置















### 硬件i2c配置 















- （无特别说明不配置dma与中断）















![image-20251116002705838](https://cloud.rocketpi.club/cloud/image-20251116002705838.png)















![image-20251116002929989](https://cloud.rocketpi.club/cloud/image-20251116002929989.png)















### usart配置















![image-20251116004045780](https://cloud.rocketpi.club/cloud/image-20251116004045780.png)















![image-20251116003215745](https://cloud.rocketpi.club/cloud/image-20251116003215745.png)















<!-- BSP_DRIVERS_START -->







<!-- BSP_DRIVERS_HASH:7037e503a24c98c91abe2db8820af45c42417c8b4d72e19dbf2286706edd691c -->







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

#include "usart.h"

#include "gpio.h"

#include "driver_aht30_config.h"

#if !AHT30_USE_SOFT_I2C

#include "i2c.h"

#endif



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

#if !AHT30_USE_SOFT_I2C

  MX_I2C1_Init();

#endif

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

#endif /* USE_FULL_ASSERT */```















</details>















<details>







<summary>bsp/aht30/driver_aht30.c</summary>















```c

#include "driver_aht30.h"



#include "driver_aht30_config.h"



#if AHT30_USE_SOFT_I2C

#include "soft_i2c.h"

#include "gpio.h"

#else

#include "i2c.h"

#endif



/**

 * @file driver_aht30.c

 * @brief AHT30温湿度传感器的软件/硬件I2C驱动实现。

 */



#define AHT30_CMD_TRIGGER       0xACU

#define AHT30_CMD_CONFIG_0      0x33U

#define AHT30_CMD_CONFIG_1      0x00U

#define AHT30_CMD_RESET         0xBAU

#define AHT30_STATUS_BUSY       0x80U



#if AHT30_USE_SOFT_I2C



typedef struct {

    GPIO_TypeDef *port;

    uint16_t pin;

} aht30_gpio_pin_t;



static soft_i2c_status_t aht30_gpio_write(void *ctx, soft_i2c_pin_state_t state)

{

    aht30_gpio_pin_t *pin = (aht30_gpio_pin_t *)ctx;

    HAL_GPIO_WritePin(pin->port, pin->pin, (state == SOFT_I2C_PIN_SET) ? GPIO_PIN_SET : GPIO_PIN_RESET);

    return SOFT_I2C_STATUS_OK;

}



static soft_i2c_pin_state_t aht30_gpio_read(void *ctx)

{

    aht30_gpio_pin_t *pin = (aht30_gpio_pin_t *)ctx;

    return (HAL_GPIO_ReadPin(pin->port, pin->pin) == GPIO_PIN_SET) ? SOFT_I2C_PIN_SET : SOFT_I2C_PIN_RESET;

}



static aht30_gpio_pin_t s_scl_pin = {AHT30_SOFT_SCL_PORT, AHT30_SOFT_SCL_PIN};

static aht30_gpio_pin_t s_sda_pin = {AHT30_SOFT_SDA_PORT, AHT30_SOFT_SDA_PIN};



static soft_i2c_bus_t s_aht30_bus = {

    .scl = {aht30_gpio_write, aht30_gpio_read, &s_scl_pin},

    .sda = {aht30_gpio_write, aht30_gpio_read, &s_sda_pin},

    .delay_fn = NULL,

    .delay_ctx = NULL,

    .delay_ticks = AHT30_SOFT_DELAY_TICKS,

    .stretch_timeout_ticks = AHT30_SOFT_STRETCH_TICKS,

    .initialized = 0U,

};



static void aht30_soft_gpio_configure(void)

{

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;

    GPIO_InitStruct.Pull = GPIO_PULLUP;

    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;



    GPIO_InitStruct.Pin = AHT30_SOFT_SCL_PIN;

    HAL_GPIO_Init(AHT30_SOFT_SCL_PORT, &GPIO_InitStruct);

    HAL_GPIO_WritePin(AHT30_SOFT_SCL_PORT, AHT30_SOFT_SCL_PIN, GPIO_PIN_SET);



    GPIO_InitStruct.Pin = AHT30_SOFT_SDA_PIN;

    HAL_GPIO_Init(AHT30_SOFT_SDA_PORT, &GPIO_InitStruct);

    HAL_GPIO_WritePin(AHT30_SOFT_SDA_PORT, AHT30_SOFT_SDA_PIN, GPIO_PIN_SET);

}



static HAL_StatusTypeDef aht30_bus_init(void)

{

    aht30_soft_gpio_configure();

    soft_i2c_status_t status = soft_i2c_bus_init(&s_aht30_bus);

    return (status == SOFT_I2C_STATUS_OK) ? HAL_OK : HAL_ERROR;

}



static HAL_StatusTypeDef aht30_bus_transmit(const uint8_t *data, size_t size)

{

    soft_i2c_status_t status = soft_i2c_master_transmit(&s_aht30_bus, AHT30_I2C_ADDRESS, data, size);

    if (status == SOFT_I2C_STATUS_TIMEOUT) {

        return HAL_TIMEOUT;

    }

    return (status == SOFT_I2C_STATUS_OK) ? HAL_OK : HAL_ERROR;

}



static HAL_StatusTypeDef aht30_bus_receive(uint8_t *data, size_t size)

{

    soft_i2c_status_t status = soft_i2c_master_receive(&s_aht30_bus, AHT30_I2C_ADDRESS, data, size);

    if (status == SOFT_I2C_STATUS_TIMEOUT) {

        return HAL_TIMEOUT;

    }

    return (status == SOFT_I2C_STATUS_OK) ? HAL_OK : HAL_ERROR;

}



#else



static HAL_StatusTypeDef aht30_bus_init(void)

{

    return HAL_OK;

}



static HAL_StatusTypeDef aht30_bus_transmit(const uint8_t *data, size_t size)

{

    return HAL_I2C_Master_Transmit(&AHT30_I2C_HANDLE, AHT30_I2C_ADDRESS, (uint8_t *)data, size, HAL_MAX_DELAY);

}



static HAL_StatusTypeDef aht30_bus_receive(uint8_t *data, size_t size)

{

    return HAL_I2C_Master_Receive(&AHT30_I2C_HANDLE, AHT30_I2C_ADDRESS, data, size, HAL_MAX_DELAY);

}



#endif /* AHT30_USE_SOFT_I2C */



static HAL_StatusTypeDef aht30_soft_reset(void)

{

    uint8_t cmd = AHT30_CMD_RESET;

    return aht30_bus_transmit(&cmd, 1U);

}



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



HAL_StatusTypeDef aht30_init(void)

{

    HAL_StatusTypeDef status = aht30_bus_init();

    if (status != HAL_OK) {

        return status;

    }



    HAL_Delay(AHT30_POWER_ON_DELAY);

    status = aht30_soft_reset();

    HAL_Delay(AHT30_POST_RESET_DELAY);

    return status;

}



HAL_StatusTypeDef aht30_read_raw(uint8_t raw[6])

{

    uint8_t cmd[3] = {AHT30_CMD_TRIGGER, AHT30_CMD_CONFIG_0, AHT30_CMD_CONFIG_1};

    HAL_StatusTypeDef status = aht30_bus_transmit(cmd, sizeof(cmd));

    if (status != HAL_OK) {

        return status;

    }



    HAL_Delay(AHT30_MEASUREMENT_DELAY);

    status = aht30_bus_receive(raw, 6U);

    if (status != HAL_OK) {

        return status;

    }



    if ((raw[0] & AHT30_STATUS_BUSY) != 0U) {

        return HAL_BUSY;

    }



    return HAL_OK;

}



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

}```















</details>















<details>







<summary>bsp/aht30/driver_aht30.h</summary>















```c

#pragma once



#include <stdint.h>



#include "stm32f4xx_hal.h"



/**

 * @file driver_aht30.h

 * @brief AHT30温湿度传感器的软件I2C驱动声明。

 */



#define AHT30_I2C_ADDRESS        (0x38U << 1) /**< 为HAL使用而左移的7位地址。 */

#define AHT30_MEASUREMENT_DELAY  80U          /**< 典型的测量转换延时（毫秒）。 */

#define AHT30_POWER_ON_DELAY     20U          /**< 上电后发送命令前的最小等待时间（毫秒）。 */

#define AHT30_POST_RESET_DELAY   20U          /**< 发出软复位后的等待时间（毫秒）。 */



#ifdef __cplusplus

extern "C" {

#endif



HAL_StatusTypeDef aht30_init(void);

HAL_StatusTypeDef aht30_read_raw(uint8_t raw[6]);

HAL_StatusTypeDef aht30_read(float *temperature_c, float *humidity_pct);



#ifdef __cplusplus

}

#endif```















</details>



<details>

<summary>bsp/aht30/driver_aht30_config.h</summary>



```c

#pragma once



#include "main.h"



/**

 * @file driver_aht30_config.h

 * @brief 通过宏控制AHT30驱动所使用的I2C实现以及GPIO定义。

 */



#ifndef AHT30_USE_SOFT_I2C

#define AHT30_USE_SOFT_I2C 1 /**< 默认启用软件I2C。设为0可切换回HAL硬件I2C。 */

#endif



#if AHT30_USE_SOFT_I2C



#ifndef AHT30_SOFT_SCL_PORT

#define AHT30_SOFT_SCL_PORT AHT30_SCL_GPIO_Port

#endif



#ifndef AHT30_SOFT_SCL_PIN

#define AHT30_SOFT_SCL_PIN AHT30_SCL_Pin

#endif



#ifndef AHT30_SOFT_SDA_PORT

#define AHT30_SOFT_SDA_PORT AHT30_SDA_GPIO_Port

#endif



#ifndef AHT30_SOFT_SDA_PIN

#define AHT30_SOFT_SDA_PIN AHT30_SDA_Pin

#endif



#ifndef AHT30_SOFT_DELAY_TICKS

#define AHT30_SOFT_DELAY_TICKS 80U

#endif



#ifndef AHT30_SOFT_STRETCH_TICKS

#define AHT30_SOFT_STRETCH_TICKS 8000U

#endif



#else



#include "i2c.h"



#ifndef AHT30_I2C_HANDLE

#define AHT30_I2C_HANDLE hi2c1

#endif



#endif /* AHT30_USE_SOFT_I2C */```



</details>









<details>



<summary>bsp/soft_i2c/soft_i2c.c</summary>







```c

#include "soft_i2c.h"



#ifndef __NOP

#define __NOP() __asm volatile ("nop")

#endif



static soft_i2c_status_t soft_i2c_prepare_bus(soft_i2c_bus_t *bus);

static void soft_i2c_delay(const soft_i2c_bus_t *bus);

static soft_i2c_status_t soft_i2c_drive_scl(soft_i2c_bus_t *bus, soft_i2c_pin_state_t state);

static soft_i2c_status_t soft_i2c_drive_sda(soft_i2c_bus_t *bus, soft_i2c_pin_state_t state);

static soft_i2c_status_t soft_i2c_start(soft_i2c_bus_t *bus);

static soft_i2c_status_t soft_i2c_repeated_start(soft_i2c_bus_t *bus);

static void soft_i2c_stop(soft_i2c_bus_t *bus);

static soft_i2c_status_t soft_i2c_write_byte(soft_i2c_bus_t *bus, uint8_t value);

static soft_i2c_status_t soft_i2c_read_byte(soft_i2c_bus_t *bus, uint8_t *value, int last_byte);



soft_i2c_status_t soft_i2c_bus_init(soft_i2c_bus_t *bus)

{

    if ((bus == NULL) ||

        (bus->scl.write == NULL) || (bus->scl.read == NULL) ||

        (bus->sda.write == NULL) || (bus->sda.read == NULL)) {

        return SOFT_I2C_STATUS_INVALID_PARAM;

    }



    if (bus->delay_ticks == 0U) {

        bus->delay_ticks = SOFT_I2C_DEFAULT_DELAY_TICKS;

    }



    if (bus->stretch_timeout_ticks == 0U) {

        bus->stretch_timeout_ticks = SOFT_I2C_DEFAULT_STRETCH_TICKS;

    }



    soft_i2c_drive_scl(bus, SOFT_I2C_PIN_SET);

    soft_i2c_drive_sda(bus, SOFT_I2C_PIN_SET);



    bus->initialized = 1U;

    return SOFT_I2C_STATUS_OK;

}



soft_i2c_status_t soft_i2c_master_transmit(soft_i2c_bus_t *bus, uint8_t address,

                                           const uint8_t *data, size_t size)

{

    if ((data == NULL) && (size > 0U)) {

        return SOFT_I2C_STATUS_INVALID_PARAM;

    }



    soft_i2c_status_t status = soft_i2c_prepare_bus(bus);

    if (status != SOFT_I2C_STATUS_OK) {

        return status;

    }



    int started = 0;

    status = soft_i2c_start(bus);

    if (status != SOFT_I2C_STATUS_OK) {

        goto done;

    }

    started = 1;



    status = soft_i2c_write_byte(bus, address & (uint8_t)~0x01U);

    if (status != SOFT_I2C_STATUS_OK) {

        goto done;

    }



    for (size_t i = 0; i < size; ++i) {

        status = soft_i2c_write_byte(bus, data[i]);

        if (status != SOFT_I2C_STATUS_OK) {

            goto done;

        }

    }



done:

    if (started) {

        soft_i2c_stop(bus);

    }

    return status;

}



soft_i2c_status_t soft_i2c_master_receive(soft_i2c_bus_t *bus, uint8_t address,

                                          uint8_t *data, size_t size)

{

    if ((data == NULL) && (size > 0U)) {

        return SOFT_I2C_STATUS_INVALID_PARAM;

    }

    if (size == 0U) {

        return SOFT_I2C_STATUS_OK;

    }



    soft_i2c_status_t status = soft_i2c_prepare_bus(bus);

    if (status != SOFT_I2C_STATUS_OK) {

        return status;

    }



    int started = 0;

    status = soft_i2c_start(bus);

    if (status != SOFT_I2C_STATUS_OK) {

        goto done;

    }

    started = 1;



    status = soft_i2c_write_byte(bus, address | 0x01U);

    if (status != SOFT_I2C_STATUS_OK) {

        goto done;

    }



    for (size_t i = 0; i < size; ++i) {

        const int last_byte = (i + 1U) == size;

        status = soft_i2c_read_byte(bus, &data[i], last_byte);

        if (status != SOFT_I2C_STATUS_OK) {

            goto done;

        }

    }



done:

    if (started) {

        soft_i2c_stop(bus);

    }

    return status;

}



soft_i2c_status_t soft_i2c_master_write_read(soft_i2c_bus_t *bus, uint8_t address,

                                             const uint8_t *tx_data, size_t tx_size,

                                             uint8_t *rx_data, size_t rx_size)

{

    if (((tx_data == NULL) && (tx_size > 0U)) ||

        ((rx_data == NULL) && (rx_size > 0U))) {

        return SOFT_I2C_STATUS_INVALID_PARAM;

    }



    soft_i2c_status_t status = soft_i2c_prepare_bus(bus);

    if (status != SOFT_I2C_STATUS_OK) {

        return status;

    }



    int started = 0;

    if (tx_size > 0U) {

        status = soft_i2c_start(bus);

        if (status != SOFT_I2C_STATUS_OK) {

            goto cleanup;

        }

        started = 1;



        status = soft_i2c_write_byte(bus, address & (uint8_t)~0x01U);

        if (status != SOFT_I2C_STATUS_OK) {

            goto cleanup;

        }



        for (size_t i = 0; i < tx_size; ++i) {

            status = soft_i2c_write_byte(bus, tx_data[i]);

            if (status != SOFT_I2C_STATUS_OK) {

                goto cleanup;

            }

        }

    }



    if (rx_size > 0U) {

        if (!started) {

            status = soft_i2c_start(bus);

            if (status != SOFT_I2C_STATUS_OK) {

                goto cleanup;

            }

            started = 1;

        } else {

            status = soft_i2c_repeated_start(bus);

            if (status != SOFT_I2C_STATUS_OK) {

                goto cleanup;

            }

        }



        status = soft_i2c_write_byte(bus, address | 0x01U);

        if (status != SOFT_I2C_STATUS_OK) {

            goto cleanup;

        }



        for (size_t i = 0; i < rx_size; ++i) {

            const int last_byte = (i + 1U) == rx_size;

            status = soft_i2c_read_byte(bus, &rx_data[i], last_byte);

            if (status != SOFT_I2C_STATUS_OK) {

                goto cleanup;

            }

        }

    }



cleanup:

    if (started) {

        soft_i2c_stop(bus);

    }

    return status;

}



static soft_i2c_status_t soft_i2c_prepare_bus(soft_i2c_bus_t *bus)

{

    if (bus == NULL) {

        return SOFT_I2C_STATUS_INVALID_PARAM;

    }



    if (bus->initialized == 0U) {

        return soft_i2c_bus_init(bus);

    }



    return SOFT_I2C_STATUS_OK;

}



static void soft_i2c_delay(const soft_i2c_bus_t *bus)

{

    if ((bus != NULL) && (bus->delay_fn != NULL)) {

        bus->delay_fn(bus->delay_ticks, bus->delay_ctx);

        return;

    }



    uint32_t ticks = (bus == NULL || bus->delay_ticks == 0U) ? SOFT_I2C_DEFAULT_DELAY_TICKS : bus->delay_ticks;

    for (volatile uint32_t i = 0; i < ticks; ++i) {

        __NOP();

    }

}



static soft_i2c_status_t soft_i2c_drive_scl(soft_i2c_bus_t *bus, soft_i2c_pin_state_t state)

{

    soft_i2c_status_t status = bus->scl.write(bus->scl.ctx, state);

    if (status != SOFT_I2C_STATUS_OK) {

        return status;

    }



    if (state == SOFT_I2C_PIN_SET) {

        uint32_t timeout = (bus->stretch_timeout_ticks == 0U)

                               ? SOFT_I2C_DEFAULT_STRETCH_TICKS

                               : bus->stretch_timeout_ticks;

        while (bus->scl.read(bus->scl.ctx) == SOFT_I2C_PIN_RESET) {

            if (timeout-- == 0U) {

                return SOFT_I2C_STATUS_TIMEOUT;

            }

        }

    }



    return SOFT_I2C_STATUS_OK;

}



static soft_i2c_status_t soft_i2c_drive_sda(soft_i2c_bus_t *bus, soft_i2c_pin_state_t state)

{

    return bus->sda.write(bus->sda.ctx, state);

}



static soft_i2c_status_t soft_i2c_start(soft_i2c_bus_t *bus)

{

    soft_i2c_status_t status = soft_i2c_drive_sda(bus, SOFT_I2C_PIN_SET);

    if (status != SOFT_I2C_STATUS_OK) {

        return status;

    }



    status = soft_i2c_drive_scl(bus, SOFT_I2C_PIN_SET);

    if (status != SOFT_I2C_STATUS_OK) {

        return status;

    }

    soft_i2c_delay(bus);



    status = soft_i2c_drive_sda(bus, SOFT_I2C_PIN_RESET);

    if (status != SOFT_I2C_STATUS_OK) {

        return status;

    }

    soft_i2c_delay(bus);



    status = soft_i2c_drive_scl(bus, SOFT_I2C_PIN_RESET);

    if (status != SOFT_I2C_STATUS_OK) {

        return status;

    }

    soft_i2c_delay(bus);



    return SOFT_I2C_STATUS_OK;

}



static soft_i2c_status_t soft_i2c_repeated_start(soft_i2c_bus_t *bus)

{

    soft_i2c_status_t status = soft_i2c_drive_sda(bus, SOFT_I2C_PIN_SET);

    if (status != SOFT_I2C_STATUS_OK) {

        return status;

    }

    soft_i2c_delay(bus);



    status = soft_i2c_drive_scl(bus, SOFT_I2C_PIN_SET);

    if (status != SOFT_I2C_STATUS_OK) {

        return status;

    }

    soft_i2c_delay(bus);



    status = soft_i2c_drive_sda(bus, SOFT_I2C_PIN_RESET);

    if (status != SOFT_I2C_STATUS_OK) {

        return status;

    }

    soft_i2c_delay(bus);



    status = soft_i2c_drive_scl(bus, SOFT_I2C_PIN_RESET);

    if (status != SOFT_I2C_STATUS_OK) {

        return status;

    }

    soft_i2c_delay(bus);



    return SOFT_I2C_STATUS_OK;

}



static void soft_i2c_stop(soft_i2c_bus_t *bus)

{

    soft_i2c_drive_sda(bus, SOFT_I2C_PIN_RESET);

    soft_i2c_delay(bus);

    soft_i2c_drive_scl(bus, SOFT_I2C_PIN_SET);

    soft_i2c_delay(bus);

    soft_i2c_drive_sda(bus, SOFT_I2C_PIN_SET);

    soft_i2c_delay(bus);

}



static soft_i2c_status_t soft_i2c_write_byte(soft_i2c_bus_t *bus, uint8_t value)

{

    for (int8_t bit = 7; bit >= 0; --bit) {

        soft_i2c_pin_state_t state = ((value >> bit) & 0x01U) ? SOFT_I2C_PIN_SET : SOFT_I2C_PIN_RESET;

        soft_i2c_status_t status = soft_i2c_drive_sda(bus, state);

        if (status != SOFT_I2C_STATUS_OK) {

            return status;

        }

        soft_i2c_delay(bus);



        status = soft_i2c_drive_scl(bus, SOFT_I2C_PIN_SET);

        if (status != SOFT_I2C_STATUS_OK) {

            return status;

        }

        soft_i2c_delay(bus);



        status = soft_i2c_drive_scl(bus, SOFT_I2C_PIN_RESET);

        if (status != SOFT_I2C_STATUS_OK) {

            return status;

        }

        soft_i2c_delay(bus);

    }



    soft_i2c_status_t status = soft_i2c_drive_sda(bus, SOFT_I2C_PIN_SET);

    if (status != SOFT_I2C_STATUS_OK) {

        return status;

    }

    soft_i2c_delay(bus);



    status = soft_i2c_drive_scl(bus, SOFT_I2C_PIN_SET);

    if (status != SOFT_I2C_STATUS_OK) {

        return status;

    }

    soft_i2c_delay(bus);



    soft_i2c_pin_state_t ack = bus->sda.read(bus->sda.ctx);



    status = soft_i2c_drive_scl(bus, SOFT_I2C_PIN_RESET);

    if (status != SOFT_I2C_STATUS_OK) {

        return status;

    }

    soft_i2c_delay(bus);



    return (ack == SOFT_I2C_PIN_RESET) ? SOFT_I2C_STATUS_OK : SOFT_I2C_STATUS_ERROR;

}



static soft_i2c_status_t soft_i2c_read_byte(soft_i2c_bus_t *bus, uint8_t *value, int last_byte)

{

    uint8_t result = 0U;

    soft_i2c_status_t status = soft_i2c_drive_sda(bus, SOFT_I2C_PIN_SET);

    if (status != SOFT_I2C_STATUS_OK) {

        return status;

    }



    for (int8_t bit = 7; bit >= 0; --bit) {

        status = soft_i2c_drive_scl(bus, SOFT_I2C_PIN_SET);

        if (status != SOFT_I2C_STATUS_OK) {

            return status;

        }

        soft_i2c_delay(bus);



        if (bus->sda.read(bus->sda.ctx) == SOFT_I2C_PIN_SET) {

            result |= (uint8_t)(1U << bit);

        }



        status = soft_i2c_drive_scl(bus, SOFT_I2C_PIN_RESET);

        if (status != SOFT_I2C_STATUS_OK) {

            return status;

        }

        soft_i2c_delay(bus);

    }



    status = soft_i2c_drive_sda(bus, last_byte ? SOFT_I2C_PIN_SET : SOFT_I2C_PIN_RESET);

    if (status != SOFT_I2C_STATUS_OK) {

        return status;

    }

    soft_i2c_delay(bus);



    status = soft_i2c_drive_scl(bus, SOFT_I2C_PIN_SET);

    if (status != SOFT_I2C_STATUS_OK) {

        return status;

    }

    soft_i2c_delay(bus);



    status = soft_i2c_drive_scl(bus, SOFT_I2C_PIN_RESET);

    if (status != SOFT_I2C_STATUS_OK) {

        return status;

    }

    soft_i2c_delay(bus);



    soft_i2c_drive_sda(bus, SOFT_I2C_PIN_SET);



    if (value != NULL) {

        *value = result;

    }



    return SOFT_I2C_STATUS_OK;

}```







</details>







<details>



<summary>bsp/soft_i2c/soft_i2c.h</summary>







```c

#pragma once



#include <stddef.h>

#include <stdint.h>



/**

 * @file soft_i2c.h

 * @brief 通用软件I2C主机接口定义，与具体平台解耦。

 */



#define SOFT_I2C_DEFAULT_DELAY_TICKS      64U

#define SOFT_I2C_DEFAULT_STRETCH_TICKS  4000U



#ifdef __cplusplus

extern "C" {

#endif



typedef enum {

    SOFT_I2C_STATUS_OK = 0,

    SOFT_I2C_STATUS_ERROR = 1,

    SOFT_I2C_STATUS_TIMEOUT = 2,

    SOFT_I2C_STATUS_INVALID_PARAM = 3

} soft_i2c_status_t;



typedef enum {

    SOFT_I2C_PIN_RESET = 0,

    SOFT_I2C_PIN_SET = 1

} soft_i2c_pin_state_t;



typedef soft_i2c_status_t (*soft_i2c_pin_write_fn)(void *ctx, soft_i2c_pin_state_t state);

typedef soft_i2c_pin_state_t (*soft_i2c_pin_read_fn)(void *ctx);

typedef void (*soft_i2c_delay_fn)(uint32_t ticks, void *ctx);



typedef struct {

    soft_i2c_pin_write_fn write;

    soft_i2c_pin_read_fn read;

    void *ctx;

} soft_i2c_pin_io_t;



typedef struct {

    soft_i2c_pin_io_t scl;

    soft_i2c_pin_io_t sda;

    soft_i2c_delay_fn delay_fn;

    void *delay_ctx;

    uint32_t delay_ticks;

    uint32_t stretch_timeout_ticks;

    uint8_t initialized;

} soft_i2c_bus_t;



soft_i2c_status_t soft_i2c_bus_init(soft_i2c_bus_t *bus);

soft_i2c_status_t soft_i2c_master_transmit(soft_i2c_bus_t *bus, uint8_t address,

                                           const uint8_t *data, size_t size);

soft_i2c_status_t soft_i2c_master_receive(soft_i2c_bus_t *bus, uint8_t address,

                                          uint8_t *data, size_t size);

soft_i2c_status_t soft_i2c_master_write_read(soft_i2c_bus_t *bus, uint8_t address,

                                             const uint8_t *tx_data, size_t tx_size,

                                             uint8_t *rx_data, size_t rx_size);



#ifdef __cplusplus

}

#endif```







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

}```















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

#endif```















</details>















<details>







<summary>bsp/debug/debug_driver.c</summary>















```c







#include "debug_driver.h"















#include "usart.h"







#include <stdarg.h>







#include <stdio.h>







#include <string.h>















/**







 * @file debug_driver.c







 * @brief Retarget low-level stdio functions to USART2 for debugging output/input.







 * @author rocket







 * @copyright Copyright (c) 2025 rocket. Authorized use only.







 */















#ifdef __GNUC__  // GCC







/**







 * @brief Retarget newlib write syscall to the USART debug port.







 * @param file Logical file descriptor supplied by the C library (unused).







 * @param ptr Buffer containing the data to be transmitted.







 * @param len Number of bytes to be transmitted.







 * @return Number of bytes written.







 */







int _write(int file, char *ptr, int len)







{







    (void)file;







    HAL_UART_Transmit(&huart2, (uint8_t *)ptr, len, HAL_MAX_DELAY);







    return len;







}















/**







 * @brief Retarget newlib read syscall to the USART debug port.







 * @param file Logical file descriptor supplied by the C library (unused).







 * @param ptr Destination buffer.







 * @param len Number of bytes to be read.







 * @return Number of bytes read.







 */







int _read(int file, char *ptr, int len)







{







    (void)file;







    HAL_UART_Receive(&huart2, (uint8_t *)ptr, len, HAL_MAX_DELAY);







    return len;







}







#elif defined(__ARMCC_VERSION)  // Keil







/**







 * @brief Retarget fputc to the USART debug port for Arm Compiler/Keil.







 * @param ch Character to transmit.







 * @param f Ignored file handle.







 * @return The transmitted character.







 */







int fputc(int ch, FILE *f)







{







    (void)f;







    uint8_t data = (uint8_t)ch;







    HAL_UART_Transmit(&huart2, &data, 1U, HAL_MAX_DELAY);







    return ch;







}















/**







 * @brief Retarget fgetc to the USART debug port for Arm Compiler/Keil.







 * @param f Ignored file handle.







 * @return The received character.







 */







int fgetc(FILE *f)







{







    (void)f;







    uint8_t ch;







    HAL_UART_Receive(&huart2, &ch, 1U, HAL_MAX_DELAY);







    return (int)ch;







}















#ifndef __MICROLIB







  /* Disable semihosting when using the standard C library (non-Microlib). */







  #pragma import(__use_no_semihosting)















  struct __FILE







  {







      int handle;







  };















  FILE __stdout;







  FILE __stdin;















  /**







   * @brief Stub exit handler so Keil does not fall back to semihosting.







   * @param x Exit code (ignored).







   */







  void _sys_exit(int x)







  {







      (void)x;







  }







#endif







#else







    #error "Unsupported compiler"







#endif















/* ========= Configuration section ========= */







#ifndef UART_LOG_INSTANCE







#define UART_LOG_INSTANCE  huart2      // Replace with the UART handle you want to use







#endif















#ifndef UART_LOG_TIMEOUT







#define UART_LOG_TIMEOUT   1000        // Send timeout (ms)







#endif















#ifndef UART_LOG_BUF_SIZE







#define UART_LOG_BUF_SIZE  256         // Size of the formatting buffer







#endif







/* ======================================== */















/**







 * @brief Blocking UART transmit helper (call outside of ISR context).







 */







static inline void uart_write_blocking(const uint8_t *data, size_t len)







{







    HAL_UART_Transmit(&UART_LOG_INSTANCE, (uint8_t *)data, (uint16_t)len, UART_LOG_TIMEOUT);







}















/**







 * @brief Normalize newlines to CRLF before sending to the terminal.







 */







static void uart_write_with_crlf(const char *s, size_t len)







{







    for (size_t i = 0; i < len; ++i) {







        char c = s[i];







        if (c == '\n') {







            const char crlf[2] = {'\r','\n'};







            uart_write_blocking((const uint8_t*)crlf, 2);







        } else {







            uart_write_blocking((const uint8_t*)&c, 1);







        }







    }







}















/**







 * @brief Output a zero-terminated string (with automatic CRLF normalization).







 */







void uart_puts(const char *s)







{







    if (s == NULL) {







        return;







    }















    uart_write_with_crlf(s, strlen(s));







}















/**







 * @brief printf-style helper built on top of the debug UART.







 * @return Number of characters that would have been written, or a negative error.







 */







int uart_printf(const char *fmt, ...)







{







    char buf[UART_LOG_BUF_SIZE];







    va_list ap;







    va_start(ap, fmt);







    int n = vsnprintf(buf, sizeof(buf), fmt, ap);







    va_end(ap);







    if (n < 0) return n;  







    size_t out_len = (n < (int)sizeof(buf)) ? (size_t)n : (size_t)sizeof(buf) - 1;







    uart_write_with_crlf(buf, out_len);















    /* 







    if (n >= (int)sizeof(buf)) {







        uart_puts("...[truncated]\n");







    }







    */







    return n;







}















/**







 * @brief Hex dump helper for quick binary inspection.







 */







void uart_hexdump(const void *data, size_t len, const char *title)







{







    const uint8_t *p = (const uint8_t*)data;







    if (title) uart_printf("%s (len=%u):\n", title, (unsigned)len);















    char line[80];







    for (size_t i = 0; i < len; i += 16) {







        int pos = 0;







        pos += snprintf(line + pos, sizeof(line) - pos, "%08X  ", (unsigned)i);















        /* hex */







        for (size_t j = 0; j < 16; ++j) {







            if (i + j < len) pos += snprintf(line + pos, sizeof(line) - pos, "%02X ", p[i + j]);







            else              pos += snprintf(line + pos, sizeof(line) - pos, "   ");







            if (j == 7) pos += snprintf(line + pos, sizeof(line) - pos, " ");







        }















        /* ascii */







        pos += snprintf(line + pos, sizeof(line) - pos, " |");







        for (size_t j = 0; j < 16 && i + j < len; ++j) {







            uint8_t c = p[i + j];







            pos += snprintf(line + pos, sizeof(line) - pos, "%c", (c >= 32 && c <= 126) ? c : '.');







        }







        pos += snprintf(line + pos, sizeof(line) - pos, "|\n");















        uart_puts(line);







    }







}







```















</details>















<details>







<summary>bsp/debug/debug_driver.h</summary>















```c







#pragma once















#include <stddef.h>







#include <stdio.h>















/**







 * @file debug_driver.h







 * @brief Interfaces for redirecting stdio calls to the debug UART and lightweight log helpers.







 */















#ifdef __cplusplus







extern "C" {







#endif















#ifdef __GNUC__







/**







 * @brief Retarget newlib write syscall to the configured debug UART.







 * @param file Logical file descriptor supplied by the C library.







 * @param ptr Pointer to the transmit buffer.







 * @param len Number of bytes to be written.







 * @return Number of bytes reported as written.







 */







int _write(int file, char *ptr, int len);















/**







 * @brief Retarget newlib read syscall to the configured debug UART.







 * @param file Logical file descriptor supplied by the C library.







 * @param ptr Pointer to the receive buffer.







 * @param len Number of bytes requested.







 * @return Number of bytes actually read.







 */







int _read(int file, char *ptr, int len);







#elif defined(__ARMCC_VERSION)







/**







 * @brief Retarget fputc calls to the configured debug UART.







 * @param ch Character to emit.







 * @param f Ignored file handle.







 * @return The character that was transmitted.







 */







int fputc(int ch, FILE *f);















/**







 * @brief Retarget fgetc calls to the configured debug UART.







 * @param f Ignored file handle.







 * @return Received character cast to int.







 */







int fgetc(FILE *f);















/**







 * @brief Disable the semihosting exit hook when using the standard C library.







 * @param x Exit code (unused).







 */







void _sys_exit(int x);







#endif















/**







 * @brief Send a zero-terminated string over the debug UART with CRLF normalization.







 * @param s String to transmit (may be NULL).







 */







void uart_puts(const char *s);















/**







 * @brief printf-style helper forwarding formatted text to the debug UART.







 * @param fmt Format string describing the output.







 * @return Number of characters that would have been written, or negative on error.







 */







int uart_printf(const char *fmt, ...);















/**







 * @brief Hex dump utility for visualizing binary buffers.







 * @param data Buffer start pointer.







 * @param len Number of bytes to dump.







 * @param title Optional title string, may be NULL.







 */







void uart_hexdump(const void *data, size_t len, const char *title);















#ifdef __cplusplus







}







#endif







```















</details>















<!-- BSP_DRIVERS_END -->







