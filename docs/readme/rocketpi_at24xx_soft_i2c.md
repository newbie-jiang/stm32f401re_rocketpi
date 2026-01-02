## 效果展示

![image-20251118032435186](https://cloud.rocketpi.club/cloud/image-20251118032435186.png)

## 功能说明

面向 RocketPI STM32F401RE 开发板的 **AT24C02读写 演示工程**。主要特性：

- 使用软件i2c
- 移植libdriver

## 硬件连接

- AT24Cxx_SCL   PB1
- AT24Cxx_SDA   PB2

## CubeMX配置

配置为推挽输出

![image-20251118032749120](https://cloud.rocketpi.club/cloud/image-20251118032749120.png)

<!-- BSP_DRIVERS_START -->
<!-- BSP_DRIVERS_HASH:98bad5a6c6b12dac445635d63ab28e8b04fd40df6d7b10c977be9097502bd18d -->
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "driver_at24cxx.h"
#include "driver_at24cxx_read_test.h"
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
  /* USER CODE BEGIN 2 */

  at24cxx_read_test(AT24C02, AT24CXX_ADDRESS_A000);
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
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
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
<summary>bsp/at24xx/driver_at24cxx.c</summary>

```c
/**
 * Copyright (c) 2015 - present LibDriver All rights reserved
 * 
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE. 
 *
 * @file      driver_at24cxx.c
 * @brief     driver at24cxx source file
 * @version   2.0.0
 * @author    Shifeng Li
 * @date      2021-02-17
 *
 * <h3>history</h3>
 * <table>
 * <tr><th>Date        <th>Version  <th>Author      <th>Description
 * <tr><td>2021/02/17  <td>2.0      <td>Shifeng Li  <td>format the code
 * <tr><td>2020/10/15  <td>1.0      <td>Shifeng Li  <td>first upload
 * </table>
 */

#include "driver_at24cxx.h"

/**
 * @brief chip information definition
 */
#define CHIP_NAME                 "Microchip AT24CXX"       /**< chip name */
#define MANUFACTURER_NAME         "Microchip"               /**< manufacturer name */
#define SUPPLY_VOLTAGE_MIN        1.7f                      /**< chip min supply voltage */
#define SUPPLY_VOLTAGE_MAX        5.5f                      /**< chip max supply voltage */
#define MAX_CURRENT               5.0f                      /**< chip max current */
#define TEMPERATURE_MIN           -40.0f                    /**< chip min operating temperature */
#define TEMPERATURE_MAX           85.0f                     /**< chip max operating temperature */
#define DRIVER_VERSION            2000                      /**< driver version */

/**
 * @brief     initialize the chip
 * @param[in] *handle pointer to an at24cxx handle structure
 * @return    status code
 *            - 0 success
 *            - 1 iic initialization failed
 *            - 2 handle is NULL
 *            - 3 linked functions is NULL
 * @note      none
 */
uint8_t at24cxx_init(at24cxx_handle_t *handle)
{
    if (handle == NULL)                                                        /* check handle */
    {
        return 2;                                                              /* return error */
    }
    if (handle->debug_print == NULL)                                           /* check debug_print */
    {
        return 3;                                                              /* return error */
    }
    if (handle->iic_init == NULL)                                              /* check iic_init */
    {
        handle->debug_print("at24cxx: iic_init is null.\n");                   /* iic_init is null */
       
        return 3;                                                              /* return error */
    }
    if (handle->iic_deinit == NULL)                                            /* check iic_deinit */
    {
        handle->debug_print("at24cxx: iic_deinit is null.\n");                 /* iic_deinit is null */
       
        return 3;                                                              /* return error */
    }
    if (handle->iic_read == NULL)                                              /* check iic_read */
    {
        handle->debug_print("at24cxx: iic_read is null.\n");                   /* iic_read is null */
       
        return 3;                                                              /* return error */
    }
    if (handle->iic_write == NULL)                                             /* check iic_write */
    {
        handle->debug_print("at24cxx: iic_write is null.\n");                  /* iic_write is null */
       
        return 3;                                                              /* return error */
    }
    if (handle->iic_read_address16 == NULL)                                    /* check iic_read_address16 */
    {
        handle->debug_print("at24cxx: iic_read_address16 is null.\n");         /* iic_read_address16 is null */
       
        return 3;                                                              /* return error */
    }
    if (handle->iic_write_address16 == NULL)                                   /* check iic_write_address16 */
    {
        handle->debug_print("at24cxx: iic_write_address16 is null.\n");        /* iic_write_address16 is null */
       
        return 3;                                                              /* return error */
    }
    if (handle->delay_ms == NULL)                                              /* check delay_ms */
    {
        handle->debug_print("at24cxx: delay_ms is null.\n");                   /* delay_ms is null */
       
        return 3;                                                              /* return error */
    }

    if (handle->iic_init() != 0)                                               /* iic init */
    {
        handle->debug_print("at24cxx: iic init failed.\n");                    /* iic init failed */
       
        return 1;                                                              /* return error */
    }
    handle->inited = 1;                                                        /* flag finish initialization */

    return 0;                                                                  /* success return 0 */
}

/**
 * @brief     close the chip
 * @param[in] *handle pointer to an at24cxx handle structure
 * @return    status code
 *            - 0 success
 *            - 1 iic deinit failed
 *            - 2 handle is NULL
 *            - 3 handle is not initialized
 * @note      none
 */
uint8_t at24cxx_deinit(at24cxx_handle_t *handle)
{
    if (handle == NULL)                                              /* check handle */
    {
        return 2;                                                    /* return error */
    }
    if (handle->inited != 1)                                         /* check handle initialization */
    {
        return 3;                                                    /* return error */
    }
    
    if (handle->iic_deinit() != 0)                                   /* iic deinit */
    {
        handle->debug_print("at24cxx: iic deinit failed.\n");        /* iic deinit failed */
        
        return 1;                                                    /* return error */
    }   
    handle->inited = 0;                                              /* flag close */
    
    return 0;                                                        /* success return 0 */
}

/**
 * @brief     set the chip type
 * @param[in] *handle pointer to an at24cxx handle structure
 * @param[in] type chip type
 * @return    status code
 *            - 0 success
 *            - 2 handle is NULL
 * @note      none
 */
uint8_t at24cxx_set_type(at24cxx_handle_t *handle, at24cxx_t type)
{
    if (handle == NULL)                 /* check handle */
    {
        return 2;                       /* return error */
    }

    handle->id = (uint32_t)type;        /* set id */
    
    return 0;                           /* success return 0 */
}

/**
 * @brief      get the chip type
 * @param[in]  *handle pointer to an at24cxx handle structure
 * @param[out] *type pointer to a chip type buffer
 * @return     status code
 *             - 0 success
 *             - 2 handle is NULL
 * @note       none
 */
uint8_t at24cxx_get_type(at24cxx_handle_t *handle, at24cxx_t *type)
{
    if (handle == NULL)                     /* check handle */
    {
        return 2;                           /* return error */
    }

    *type = (at24cxx_t)(handle->id);        /* get id */
    
    return 0;                               /* success return 0 */
}

/**
 * @brief     set the chip address pin
 * @param[in] *handle pointer to an at24cxx handle structure
 * @param[in] addr_pin chip address pin
 * @return    status code
 *            - 0 success
 *            - 2 handle is NULL
 * @note      none
 */
uint8_t at24cxx_set_addr_pin(at24cxx_handle_t *handle, at24cxx_address_t addr_pin)
{
    if (handle == NULL)                       /* check handle */
    {
        return 2;                             /* return error */
    }

    handle->iic_addr = 0xA0;                  /* set iic addr */
    handle->iic_addr |= addr_pin << 1;        /* set iic address */
    
    return 0;                                 /* success return 0 */
}

/**
 * @brief      get the chip address pin
 * @param[in]  *handle pointer to an at24cxx handle structure
 * @param[out] *addr_pin pointer to a chip address pin
 * @return     status code
 *             - 0 success
 *             - 2 handle is NULL
 * @note       none
 */
uint8_t at24cxx_get_addr_pin(at24cxx_handle_t *handle, at24cxx_address_t *addr_pin)
{
    if (handle == NULL)                                                        /* check handle */
    {
        return 2;                                                              /* return error */
    }

    *addr_pin = (at24cxx_address_t)((handle->iic_addr & (~0xA0)) >> 1);        /* get iic address */
    
    return 0;                                                                  /* success return 0 */
}

/**
 * @brief      read bytes from the chip
 * @param[in]  *handle pointer to an at24cxx handle structure
 * @param[in]  address register address
 * @param[out] *buf pointer to a data buffer
 * @param[in]  len buffer length
 * @return     status code
 *             - 0 success
 *             - 1 read data failed
 *             - 2 handle is NULL
 *             - 3 handle is not initialized
 *             - 4 end address is over the max address
 * @note       none
 */
uint8_t at24cxx_read(at24cxx_handle_t *handle, uint32_t address, uint8_t *buf, uint16_t len)
{
    uint8_t page_remain;
    
    if (handle == NULL)                                                                                      /* check handle */
    {
        return 2;                                                                                            /* return error */
    }
    if (handle->inited != 1)                                                                                 /* check handle initialization */
    {
        return 3;                                                                                            /* return error */
    }

    if ((address + len) > handle->id)                                                                        /* check length */
    {
        handle->debug_print("at24cxx: read out of range.\n");                                                /* read out of range */
       
        return 4;                                                                                            /* return error */
    }
    page_remain = (uint8_t)(8 - address % 8);                                                                /* get page remain */
    if (len <= page_remain)                                                                                  /* page remain */
    {
        page_remain = (uint8_t)len;                                                                          /* set page remain */
    }
    if (handle->id > (uint32_t)AT24C16)                                                                      /* choose id to set different address */
    {
        while (1)
        {
            if (handle->iic_read_address16((uint8_t)(handle->iic_addr + ((address / 65536) << 1)), 
                                           address % 65536, buf,
                                           page_remain) != 0)                                                /* read page */
            {
                handle->debug_print("at24cxx: read failed.\n");                                              /* read failed */
               
                return 1;                                                                                    /* return error */
            }
            if (page_remain == len)                                                                          /* check break */
            {
                break;                                                                                       /* break loop */
            }
            else
            {
                address += page_remain;                                                                      /* address increase */
                buf += page_remain;                                                                          /* buffer point increase */
                len -= page_remain;                                                                          /* length decrease */
                if (len < 8)                                                                                 /* check length */
                {
                    page_remain = (uint8_t)len;                                                              /* set the reset length */
                }
                else
                {
                    page_remain = 8;                                                                         /* set page */
                }
            }
        }
    }
    else
    {
        while (1)
        {
            if (handle->iic_read((uint8_t)(handle->iic_addr + ((address / 256) << 1)), address % 256, buf,
                                  page_remain) != 0)                                                         /* read page */
            {
                handle->debug_print("at24cxx: read failed.\n");                                              /* read failed */
               
                return 1;                                                                                    /* return error */
            }
            if (page_remain == len)                                                                          /* check break */
            {
                break;                                                                                       /* break loop */
            }
            else
            {
                address += page_remain;                                                                      /* address increase */
                buf += page_remain;                                                                          /* buffer point increase */
                len -= page_remain;                                                                          /* length decrease */
                if (len < 8)                                                                                 /* check length */
                {
                    page_remain = (uint8_t)len;                                                              /* set the reset length */
                }
                else
                {
                    page_remain = 8;                                                                         /* set page */
                }
            }
        }
    }
    
    return 0;                                                                                                /* success return 0 */
}

/**
 * @brief     write bytes to the chip
 * @param[in] *handle pointer to an at24cxx handle structure
 * @param[in] address register address
 * @param[in] *buf pointer to a data buffer
 * @param[in] len buffer length
 * @return    status code
 *            - 0 success
 *            - 1 write data failed
 *            - 2 handle is NULL
 *            - 3 handle is not initialized
 *            - 4 end address is over the max address
 * @note      none
 */
uint8_t at24cxx_write(at24cxx_handle_t *handle, uint32_t address, uint8_t *buf, uint16_t len)
{
    uint8_t page_remain;
    
    if (handle == NULL)                                                                                       /* check handle */
    {
        return 2;                                                                                             /* return error */
    }
    if (handle->inited != 1)                                                                                  /* check handle initialization */
    {
        return 3;                                                                                             /* return error */
    }

    if ((address + len) > handle->id)                                                                         /* check length */
    {
        handle->debug_print("at24cxx: write out of range.\n");                                                /* write out of range */
       
        return 1;                                                                                             /* return error */
    }
    page_remain = (uint8_t)(8 - address % 8);                                                                 /* set page remain */
    if (len <= page_remain)                                                                                   /* check length */
    {
        page_remain = (uint8_t)len;                                                                           /* set page remain */
    }
    if (handle->id > (uint32_t)AT24C16)                                                                       /* check id */
    {
        while (1)
        {
            if (handle->iic_write_address16((uint8_t)(handle->iic_addr + ((address / 65536) << 1)), 
                                            address % 65536, buf,
                                            page_remain) != 0)                                                /* write page */
            {
                handle->debug_print("at24cxx: write failed.\n");                                              /* write failed */
               
                return 1;                                                                                     /* return error */
            }
            handle->delay_ms(6);                                                                              /* wait 6 ms */
            if (page_remain == len)                                                                           /* check break */
            {
                break;                                                                                        /* break */
            }
            else
            {
                address += page_remain;                                                                       /* address increase */
                buf += page_remain;                                                                           /* buffer point increase */
                len -= page_remain;                                                                           /* length decrease */
                if (len < 8)                                                                                  /* check length */
                {
                    page_remain = (uint8_t)len;                                                               /* set the rest length */
                }
                else
                {
                    page_remain = 8;                                                                          /* set page */
                }
            }
        }
    }
    else
    {
        while (1)
        {
            if (handle->iic_write((uint8_t)(handle->iic_addr + ((address / 256) << 1)), address % 256, buf,
                                  page_remain) != 0)                                                          /* write page */
            {
                handle->debug_print("at24cxx: write failed.\n");                                              /* write failed */
               
                return 1;                                                                                     /* return error */
            }
            handle->delay_ms(6);                                                                              /* wait 6 ms */
            if (page_remain == len)                                                                           /* check break */
            {
                break;                                                                                        /* break */
            }
            else
            {
                address += page_remain;                                                                       /* address increase */
                buf += page_remain;                                                                           /* buffer point increase */
                len -= page_remain;                                                                           /* length decrease */
                if (len < 8)                                                                                  /* check length */
                {
                    page_remain = (uint8_t)len;                                                               /* set the rest length */
                }
                else
                {
                    page_remain = 8;                                                                          /* set page */
                }
            }
        }
    }
    
    return 0;                                                                                                 /* success return 0 */
}

/**
 * @brief      get chip's information
 * @param[out] *info pointer to an at24cxx info structure
 * @return     status code
 *             - 0 success
 *             - 2 handle is NULL
 * @note       none
 */
uint8_t at24cxx_info(at24cxx_info_t *info)
{
    if (info == NULL)                                               /* check handle */
    {
        return 2;                                                   /* return error */
    }
    
    memset(info, 0, sizeof(at24cxx_info_t));                        /* initialize at24cxx info structure */
    strncpy(info->chip_name, CHIP_NAME, 32);                        /* copy chip name */
    strncpy(info->manufacturer_name, MANUFACTURER_NAME, 32);        /* copy manufacturer name */
    strncpy(info->interface, "IIC", 8);                             /* copy interface name */
    info->supply_voltage_min_v = SUPPLY_VOLTAGE_MIN;                /* set minimal supply voltage */
    info->supply_voltage_max_v = SUPPLY_VOLTAGE_MAX;                /* set maximum supply voltage */
    info->max_current_ma = MAX_CURRENT;                             /* set maximum current */
    info->temperature_max = TEMPERATURE_MAX;                        /* set minimal temperature */
    info->temperature_min = TEMPERATURE_MIN;                        /* set maximum temperature */
    info->driver_version = DRIVER_VERSION;                          /* set driver version */
    
    return 0;                                                       /* success return 0 */
}
```

</details>

<details>
<summary>bsp/at24xx/driver_at24cxx.h</summary>

```c
/**
 * Copyright (c) 2015 - present LibDriver All rights reserved
 * 
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE. 
 *
 * @file      driver_at24cxx.h
 * @brief     driver at24cxx header file
 * @version   2.0.0
 * @author    Shifeng Li
 * @date      2021-02-17
 *
 * <h3>history</h3>
 * <table>
 * <tr><th>Date        <th>Version  <th>Author      <th>Description
 * <tr><td>2021/02/17  <td>2.0      <td>Shifeng Li  <td>format the code
 * <tr><td>2020/10/15  <td>1.0      <td>Shifeng Li  <td>first upload
 * </table>
 */

#ifndef DRIVER_AT24CXX_H
#define DRIVER_AT24CXX_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C"{
#endif

/**
 * @defgroup at24cxx_driver at24cxx driver function
 * @brief    at24cxx driver modules
 * @{
 */

/**
 * @addtogroup at24cxx_base_driver
 * @{
 */

/**
 * @brief at24cxx type enumeration definition
 */
typedef enum
{
    AT24C01  = 128,          /**< AT24C01 type */
    AT24C02  = 256,          /**< AT24C02 type */
    AT24C04  = 512,          /**< AT24C04 type */
    AT24C08  = 1024,         /**< AT24C08 type */
    AT24C16  = 2048,         /**< AT24C16 type */
    AT24C32  = 4096,         /**< AT24C32 type */
    AT24C64  = 8192,         /**< AT24C64 type */
    AT24C128 = 16384,        /**< AT24C128 type */
    AT24C256 = 32768,        /**< AT24C256 type */
    AT24C512 = 65536,        /**< AT24C512 type */
    AT24CM01 = 131072,       /**< AT24CM01 type */
    AT24CM02 = 262144,       /**< AT24CM02 type */
} at24cxx_t;

/**
 * @brief at24cxx address enumeration definition
 */
typedef enum
{
    AT24CXX_ADDRESS_A000 = 0,        /**< A2A1A0 000 */
    AT24CXX_ADDRESS_A001 = 1,        /**< A2A1A0 001 */
    AT24CXX_ADDRESS_A010 = 2,        /**< A2A1A0 010 */
    AT24CXX_ADDRESS_A011 = 3,        /**< A2A1A0 011 */
    AT24CXX_ADDRESS_A100 = 4,        /**< A2A1A0 100 */
    AT24CXX_ADDRESS_A101 = 5,        /**< A2A1A0 101 */
    AT24CXX_ADDRESS_A110 = 6,        /**< A2A1A0 110 */
    AT24CXX_ADDRESS_A111 = 7,        /**< A2A1A0 111 */
} at24cxx_address_t;

/**
 * @brief at24cxx handle structure definition
 */
typedef struct at24cxx_handle_s
{
    uint8_t iic_addr;                                                                              /**< iic device address */
    uint8_t (*iic_init)(void);                                                                     /**< point to an iic_init function address */
    uint8_t (*iic_deinit)(void);                                                                   /**< point to an iic_deinit function address */
    uint8_t (*iic_read)(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len);                    /**< point to an iic_read function address */
    uint8_t (*iic_write)(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len);                   /**< point to an iic_write function address */
    uint8_t (*iic_read_address16)(uint8_t addr, uint16_t reg, uint8_t *buf, uint16_t len);         /**< point to an iic_read_address16 function address */
    uint8_t (*iic_write_address16)(uint8_t addr, uint16_t reg, uint8_t *buf, uint16_t len);        /**< point to an iic_write_address16 function address */
    void (*delay_ms)(uint32_t ms);                                                                 /**< point to a delay_ms function address */
    void (*debug_print)(const char *const fmt, ...);                                               /**< point to a debug_print function address */
    uint32_t id;                                                                                   /**< chip id */
    uint8_t inited;                                                                                /**< inited flag */
} at24cxx_handle_t;

/**
 * @brief at24cxx information structure definition
 */
typedef struct at24cxx_info_s
{
    char chip_name[32];                /**< chip name */
    char manufacturer_name[32];        /**< manufacturer name */
    char interface[8];                 /**< chip interface name */
    float supply_voltage_min_v;        /**< chip min supply voltage */
    float supply_voltage_max_v;        /**< chip max supply voltage */
    float max_current_ma;              /**< chip max current */
    float temperature_min;             /**< chip min operating temperature */
    float temperature_max;             /**< chip max operating temperature */
    uint32_t driver_version;           /**< driver version */
} at24cxx_info_t;

/**
 * @}
 */

/**
 * @defgroup at24cxx_link_driver at24cxx link driver function
 * @brief    at24cxx link driver modules
 * @ingroup  at24cxx_driver
 * @{
 */

/**
 * @brief     initialize at24cxx_handle_t structure
 * @param[in] HANDLE pointer to an at24cxx handle structure
 * @param[in] STRUCTURE at24cxx_handle_t
 * @note      none
 */
#define DRIVER_AT24CXX_LINK_INIT(HANDLE, STRUCTURE)           memset(HANDLE, 0, sizeof(STRUCTURE))

/**
 * @brief     link iic_init function
 * @param[in] HANDLE pointer to an at24cxx handle structure
 * @param[in] FUC pointer to an iic_init function address
 * @note      none
 */
#define DRIVER_AT24CXX_LINK_IIC_INIT(HANDLE, FUC)             (HANDLE)->iic_init = FUC

/**
 * @brief     link iic_deinit function
 * @param[in] HANDLE pointer to an at24cxx handle structure
 * @param[in] FUC pointer to an iic_deinit function address
 * @note      none
 */
#define DRIVER_AT24CXX_LINK_IIC_DEINIT(HANDLE, FUC)           (HANDLE)->iic_deinit = FUC

/**
 * @brief     link iic_read function
 * @param[in] HANDLE pointer to an at24cxx handle structure
 * @param[in] FUC pointer to an iic_read function address
 * @note      none
 */
#define DRIVER_AT24CXX_LINK_IIC_READ(HANDLE, FUC)             (HANDLE)->iic_read = FUC

/**
 * @brief     link iic_write function
 * @param[in] HANDLE pointer to an at24cxx handle structure
 * @param[in] FUC pointer to an iic_write function address
 * @note      none
 */
#define DRIVER_AT24CXX_LINK_IIC_WRITE(HANDLE, FUC)            (HANDLE)->iic_write = FUC

/**
 * @brief     link iic_read_address16 function
 * @param[in] HANDLE pointer to an at24cxx handle structure
 * @param[in] FUC pointer to an iic_read_address16 function address
 * @note      none
 */
#define DRIVER_AT24CXX_LINK_IIC_READ_ADDRESS16(HANDLE, FUC)   (HANDLE)->iic_read_address16 = FUC

/**
 * @brief     link iic_write_address16 function
 * @param[in] HANDLE pointer to an at24cxx handle structure
 * @param[in] FUC pointer to an iic_write_address16 function address
 * @note      none
 */
#define DRIVER_AT24CXX_LINK_IIC_WRITE_ADDRESS16(HANDLE, FUC)  (HANDLE)->iic_write_address16 = FUC

/**
 * @brief     link delay_ms function
 * @param[in] HANDLE pointer to an at24cxx handle structure
 * @param[in] FUC pointer to a delay_ms function address
 * @note      none
 */
#define DRIVER_AT24CXX_LINK_DELAY_MS(HANDLE, FUC)             (HANDLE)->delay_ms = FUC

/**
 * @brief     link debug_print function
 * @param[in] HANDLE pointer to an at24cxx handle structure
 * @param[in] FUC pointer to a debug_print function address
 * @note      none
 */
#define DRIVER_AT24CXX_LINK_DEBUG_PRINT(HANDLE, FUC)          (HANDLE)->debug_print = FUC

/**
 * @}
 */

/**
 * @defgroup at24cxx_base_driver at24cxx base driver function
 * @brief    at24cxx base driver modules
 * @ingroup  at24cxx_driver
 * @{
 */

/**
 * @brief      get chip's information
 * @param[out] *info pointer to an at24cxx info structure
 * @return     status code
 *             - 0 success
 *             - 2 handle is NULL
 * @note       none
 */
uint8_t at24cxx_info(at24cxx_info_t *info);

/**
 * @brief     initialize the chip
 * @param[in] *handle pointer to an at24cxx handle structure
 * @return    status code
 *            - 0 success
 *            - 1 iic initialization failed
 *            - 2 handle is NULL
 *            - 3 linked functions is NULL
 * @note      none
 */
uint8_t at24cxx_init(at24cxx_handle_t *handle);

/**
 * @brief     close the chip
 * @param[in] *handle pointer to an at24cxx handle structure
 * @return    status code
 *            - 0 success
 *            - 1 iic deinit failed
 *            - 2 handle is NULL
 *            - 3 handle is not initialized
 * @note      none
 */
uint8_t at24cxx_deinit(at24cxx_handle_t *handle);

/**
 * @brief     set the chip type
 * @param[in] *handle pointer to an at24cxx handle structure
 * @param[in] type chip type
 * @return    status code
 *            - 0 success
 *            - 2 handle is NULL
 * @note      none
 */
uint8_t at24cxx_set_type(at24cxx_handle_t *handle, at24cxx_t type);

/**
 * @brief      get the chip type
 * @param[in]  *handle pointer to an at24cxx handle structure
 * @param[out] *type pointer to a chip type buffer
 * @return     status code
 *             - 0 success
 *             - 2 handle is NULL
 * @note       none
 */
uint8_t at24cxx_get_type(at24cxx_handle_t *handle, at24cxx_t *type);

/**
 * @brief     set the chip address pin
 * @param[in] *handle pointer to an at24cxx handle structure
 * @param[in] addr_pin chip address pin
 * @return    status code
 *            - 0 success
 *            - 2 handle is NULL
 * @note      none
 */
uint8_t at24cxx_set_addr_pin(at24cxx_handle_t *handle, at24cxx_address_t addr_pin);

/**
 * @brief      get the chip address pin
 * @param[in]  *handle pointer to an at24cxx handle structure
 * @param[out] *addr_pin pointer to a chip address pin
 * @return     status code
 *             - 0 success
 *             - 2 handle is NULL
 * @note       none
 */
uint8_t at24cxx_get_addr_pin(at24cxx_handle_t *handle, at24cxx_address_t *addr_pin);

/**
 * @brief      read bytes from the chip
 * @param[in]  *handle pointer to an at24cxx handle structure
 * @param[in]  address register address
 * @param[out] *buf pointer to a data buffer
 * @param[in]  len buffer length
 * @return     status code
 *             - 0 success
 *             - 1 read data failed
 *             - 2 handle is NULL
 *             - 3 handle is not initialized
 *             - 4 end address is over the max address
 * @note       none
 */
uint8_t at24cxx_read(at24cxx_handle_t *handle, uint32_t address, uint8_t *buf, uint16_t len);

/**
 * @brief     write bytes to the chip
 * @param[in] *handle pointer to an at24cxx handle structure
 * @param[in] address register address
 * @param[in] *buf pointer to a data buffer
 * @param[in] len buffer length
 * @return    status code
 *            - 0 success
 *            - 1 write data failed
 *            - 2 handle is NULL
 *            - 3 handle is not initialized
 *            - 4 end address is over the max address
 * @note      none
 */
uint8_t at24cxx_write(at24cxx_handle_t *handle, uint32_t address, uint8_t *buf, uint16_t len);

/**
 * @}
 */

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
```

</details>

<details>
<summary>bsp/at24xx/driver_at24cxx_basic.c</summary>

```c
/**
 * Copyright (c) 2015 - present LibDriver All rights reserved
 * 
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE. 
 *
 * @file      driver_at24cxx_basic.c
 * @brief     driver at24cxx basic source file
 * @version   2.0.0
 * @author    Shifeng Li
 * @date      2021-02-17
 *
 * <h3>history</h3>
 * <table>
 * <tr><th>Date        <th>Version  <th>Author      <th>Description
 * <tr><td>2021/02/17  <td>2.0      <td>Shifeng Li  <td>format the code
 * <tr><td>2020/10/15  <td>1.0      <td>Shifeng Li  <td>first upload
 * </table>
 */

#include "driver_at24cxx_basic.h"

static at24cxx_handle_t gs_handle;        /**< at24cxx handle */

/**
 * @brief     basic example init
 * @param[in] type chip type
 * @param[in] address chip address pin
 * @return    status code
 *            - 0 success
 *            - 1 init failed
 * @note      none
 */
uint8_t at24cxx_basic_init(at24cxx_t type, at24cxx_address_t address)
{
    uint8_t res;
    
    /* link interface function */
    DRIVER_AT24CXX_LINK_INIT(&gs_handle, at24cxx_handle_t);
    DRIVER_AT24CXX_LINK_IIC_INIT(&gs_handle, at24cxx_interface_iic_init);
    DRIVER_AT24CXX_LINK_IIC_DEINIT(&gs_handle, at24cxx_interface_iic_deinit);
    DRIVER_AT24CXX_LINK_IIC_READ(&gs_handle, at24cxx_interface_iic_read);
    DRIVER_AT24CXX_LINK_IIC_WRITE(&gs_handle, at24cxx_interface_iic_write);
    DRIVER_AT24CXX_LINK_IIC_READ_ADDRESS16(&gs_handle, at24cxx_interface_iic_read_address16);
    DRIVER_AT24CXX_LINK_IIC_WRITE_ADDRESS16(&gs_handle, at24cxx_interface_iic_write_address16);
    DRIVER_AT24CXX_LINK_DELAY_MS(&gs_handle, at24cxx_interface_delay_ms);
    DRIVER_AT24CXX_LINK_DEBUG_PRINT(&gs_handle, at24cxx_interface_debug_print);
    
    /* set chip type */
    res = at24cxx_set_type(&gs_handle, type);
    if (res != 0)
    {
        at24cxx_interface_debug_print("at24cxx: set type failed.\n");
       
        return 1;
    }
    
    /* set addr pin */
    res = at24cxx_set_addr_pin(&gs_handle, address);
    if (res != 0)
    {
        at24cxx_interface_debug_print("at24cxx: set address pin failed.\n");
       
        return 1;
    }
    
    /* at24cxx init */
    res = at24cxx_init(&gs_handle);
    if (res != 0)
    {
        at24cxx_interface_debug_print("at24cxx: init failed.\n");
       
        return 1;
    }

    return 0;
}

/**
 * @brief      basic example read
 * @param[in]  address register address
 * @param[out] *buf pointer to a data buffer
 * @param[in]  len buffer length
 * @return     status code
 *             - 0 success
 *             - 1 read data failed
 * @note       none
 */
uint8_t at24cxx_basic_read(uint32_t address, uint8_t *buf, uint16_t len)
{
    /* read data */
    if (at24cxx_read(&gs_handle, address, buf, len) != 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/**
 * @brief     basic example write
 * @param[in] address register address
 * @param[in] *buf pointer to a data buffer
 * @param[in] len buffer length
 * @return    status code
 *            - 0 success
 *            - 1 write data failed
 * @note      none
 */
uint8_t at24cxx_basic_write(uint32_t address, uint8_t *buf, uint16_t len)
{
    /* write data */
    if (at24cxx_write(&gs_handle, address, buf, len) != 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/**
 * @brief  basic example deinit
 * @return status code
 *         - 0 success
 *         - 1 deinit failed
 * @note   none
 */
uint8_t at24cxx_basic_deinit(void)
{
    /* at24cxx deinit */
    if (at24cxx_deinit(&gs_handle) != 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
```

</details>

<details>
<summary>bsp/at24xx/driver_at24cxx_basic.h</summary>

```c
/**
 * Copyright (c) 2015 - present LibDriver All rights reserved
 * 
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE. 
 *
 * @file      driver_at24cxx_basic.h
 * @brief     driver at24cxx basic header file
 * @version   2.0.0
 * @author    Shifeng Li
 * @date      2021-02-17
 *
 * <h3>history</h3>
 * <table>
 * <tr><th>Date        <th>Version  <th>Author      <th>Description
 * <tr><td>2021/02/17  <td>2.0      <td>Shifeng Li  <td>format the code
 * <tr><td>2020/10/15  <td>1.0      <td>Shifeng Li  <td>first upload
 * </table>
 */

#ifndef DRIVER_AT24CXX_BASIC_H
#define DRIVER_AT24CXX_BASIC_H

#include "driver_at24cxx_interface.h"

#ifdef __cplusplus
extern "C"{
#endif

/**
 * @defgroup at24cxx_example_driver at24cxx example driver function
 * @brief    at24cxx example driver modules
 * @ingroup  at24cxx_driver
 * @{
 */

/**
 * @brief     basic example init
 * @param[in] type chip type
 * @param[in] address chip address pin
 * @return    status code
 *            - 0 success
 *            - 1 init failed
 * @note      none
 */
uint8_t at24cxx_basic_init(at24cxx_t type, at24cxx_address_t address);

/**
 * @brief  basic example deinit
 * @return status code
 *         - 0 success
 *         - 1 deinit failed
 * @note   none
 */
uint8_t at24cxx_basic_deinit(void);

/**
 * @brief      basic example read
 * @param[in]  address register address
 * @param[out] *buf pointer to a data buffer
 * @param[in]  len buffer length
 * @return     status code
 *             - 0 success
 *             - 1 read data failed
 * @note       none
 */
uint8_t at24cxx_basic_read(uint32_t address, uint8_t *buf, uint16_t len);

/**
 * @brief     basic example write
 * @param[in] address register address
 * @param[in] *buf pointer to a data buffer
 * @param[in] len buffer length
 * @return    status code
 *            - 0 success
 *            - 1 write data failed
 * @note      none
 */
uint8_t at24cxx_basic_write(uint32_t address, uint8_t *buf, uint16_t len);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
```

</details>

<details>
<summary>bsp/at24xx/driver_at24cxx_interface.c</summary>

```c
#include "driver_at24cxx_interface.h"

#include "main.h"

#include <stdarg.h>
#include <stdio.h>

#define SOFT_I2C_DELAY_CYCLES  120U

static uint8_t s_soft_i2c_sda_is_output = 0U;

static void soft_i2c_delay(void)
{
    for (volatile uint32_t i = 0; i < SOFT_I2C_DELAY_CYCLES; ++i)
    {
        __NOP();
    }
}

static void soft_i2c_scl_write(GPIO_PinState state)
{
    HAL_GPIO_WritePin(AT24CXX_SCL_GPIO_Port, AT24CXX_SCL_Pin, state);
}

static void soft_i2c_sda_write(GPIO_PinState state)
{
    HAL_GPIO_WritePin(AT24CXX_SDA_GPIO_Port, AT24CXX_SDA_Pin, state);
}

static GPIO_PinState soft_i2c_sda_read(void)
{
    return HAL_GPIO_ReadPin(AT24CXX_SDA_GPIO_Port, AT24CXX_SDA_Pin);
}

static void soft_i2c_sda_mode_output(void)
{
    GPIO_InitTypeDef init = {0};

    if (s_soft_i2c_sda_is_output == 0U)
    {
        init.Pin = AT24CXX_SDA_Pin;
        init.Mode = GPIO_MODE_OUTPUT_OD;
        init.Pull = GPIO_PULLUP;
        init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        HAL_GPIO_Init(AT24CXX_SDA_GPIO_Port, &init);
        s_soft_i2c_sda_is_output = 1U;
    }
}

static void soft_i2c_sda_mode_input(void)
{
    GPIO_InitTypeDef init = {0};

    if (s_soft_i2c_sda_is_output != 0U)
    {
        init.Pin = AT24CXX_SDA_Pin;
        init.Mode = GPIO_MODE_INPUT;
        init.Pull = GPIO_PULLUP;
        HAL_GPIO_Init(AT24CXX_SDA_GPIO_Port, &init);
        s_soft_i2c_sda_is_output = 0U;
    }
}

static void soft_i2c_start(void)
{
    soft_i2c_sda_mode_output();
    soft_i2c_sda_write(GPIO_PIN_SET);
    soft_i2c_scl_write(GPIO_PIN_SET);
    soft_i2c_delay();
    soft_i2c_sda_write(GPIO_PIN_RESET);
    soft_i2c_delay();
    soft_i2c_scl_write(GPIO_PIN_RESET);
    soft_i2c_delay();
}

static void soft_i2c_stop(void)
{
    soft_i2c_sda_mode_output();
    soft_i2c_scl_write(GPIO_PIN_RESET);
    soft_i2c_sda_write(GPIO_PIN_RESET);
    soft_i2c_delay();
    soft_i2c_scl_write(GPIO_PIN_SET);
    soft_i2c_delay();
    soft_i2c_sda_write(GPIO_PIN_SET);
    soft_i2c_delay();
}

static uint8_t soft_i2c_wait_ack(void)
{
    uint8_t ack;

    soft_i2c_sda_mode_input();
    soft_i2c_delay();
    soft_i2c_scl_write(GPIO_PIN_SET);
    soft_i2c_delay();
    ack = (soft_i2c_sda_read() == GPIO_PIN_RESET) ? 0U : 1U;
    soft_i2c_scl_write(GPIO_PIN_RESET);
    soft_i2c_sda_mode_output();
    soft_i2c_sda_write(GPIO_PIN_SET);
    return ack;
}

static void soft_i2c_send_ack(uint8_t nack)
{
    soft_i2c_sda_mode_output();
    soft_i2c_sda_write(nack ? GPIO_PIN_SET : GPIO_PIN_RESET);
    soft_i2c_delay();
    soft_i2c_scl_write(GPIO_PIN_SET);
    soft_i2c_delay();
    soft_i2c_scl_write(GPIO_PIN_RESET);
    soft_i2c_sda_write(GPIO_PIN_SET);
}

static uint8_t soft_i2c_send_byte(uint8_t data)
{
    soft_i2c_sda_mode_output();
    for (uint8_t i = 0; i < 8; ++i)
    {
        soft_i2c_scl_write(GPIO_PIN_RESET);
        soft_i2c_delay();
        soft_i2c_sda_write((data & 0x80U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
        data <<= 1;
        soft_i2c_delay();
        soft_i2c_scl_write(GPIO_PIN_SET);
        soft_i2c_delay();
    }
    soft_i2c_scl_write(GPIO_PIN_RESET);
    soft_i2c_sda_mode_output();
    soft_i2c_sda_write(GPIO_PIN_SET);
    return soft_i2c_wait_ack();
}

static uint8_t soft_i2c_receive_byte(uint8_t last)
{
    uint8_t data = 0U;

    soft_i2c_sda_mode_input();
    for (uint8_t i = 0; i < 8; ++i)
    {
        data <<= 1;
        soft_i2c_scl_write(GPIO_PIN_RESET);
        soft_i2c_delay();
        soft_i2c_scl_write(GPIO_PIN_SET);
        soft_i2c_delay();
        if (soft_i2c_sda_read() == GPIO_PIN_SET)
        {
            data |= 0x01U;
        }
    }
    soft_i2c_scl_write(GPIO_PIN_RESET);
    soft_i2c_send_ack(last ? 1U : 0U);
    return data;
}

uint8_t at24cxx_interface_iic_init(void)
{
    GPIO_InitTypeDef init = {0};

    __HAL_RCC_GPIOB_CLK_ENABLE();

    init.Pin = AT24CXX_SCL_Pin;
    init.Mode = GPIO_MODE_OUTPUT_OD;
    init.Pull = GPIO_PULLUP;
    init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(AT24CXX_SCL_GPIO_Port, &init);

    init.Pin = AT24CXX_SDA_Pin;
    HAL_GPIO_Init(AT24CXX_SDA_GPIO_Port, &init);

    s_soft_i2c_sda_is_output = 1U;
    soft_i2c_scl_write(GPIO_PIN_SET);
    soft_i2c_sda_write(GPIO_PIN_SET);

    return 0;
}

uint8_t at24cxx_interface_iic_deinit(void)
{
    HAL_GPIO_DeInit(AT24CXX_SCL_GPIO_Port, AT24CXX_SCL_Pin);
    HAL_GPIO_DeInit(AT24CXX_SDA_GPIO_Port, AT24CXX_SDA_Pin);
    s_soft_i2c_sda_is_output = 0U;

    return 0;
}

uint8_t at24cxx_interface_iic_read(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len)
{
    if (len == 0U)
    {
        return 0;
    }
    if (buf == NULL)
    {
        return 1;
    }

    soft_i2c_start();
    if (soft_i2c_send_byte(addr))
    {
        soft_i2c_stop();
        return 1;
    }
    if (soft_i2c_send_byte(reg))
    {
        soft_i2c_stop();
        return 1;
    }
    soft_i2c_start();
    if (soft_i2c_send_byte((uint8_t)(addr | 0x01U)))
    {
        soft_i2c_stop();
        return 1;
    }
    for (uint16_t i = 0; i < len; ++i)
    {
        buf[i] = soft_i2c_receive_byte((i == (len - 1U)) ? 1U : 0U);
    }
    soft_i2c_stop();

    return 0;
}

uint8_t at24cxx_interface_iic_write(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len)
{
    if (len == 0U)
    {
        return 0;
    }
    if (buf == NULL)
    {
        return 1;
    }

    soft_i2c_start();
    if (soft_i2c_send_byte(addr))
    {
        soft_i2c_stop();
        return 1;
    }
    if (soft_i2c_send_byte(reg))
    {
        soft_i2c_stop();
        return 1;
    }
    for (uint16_t i = 0; i < len; ++i)
    {
        if (soft_i2c_send_byte(buf[i]))
        {
            soft_i2c_stop();
            return 1;
        }
    }
    soft_i2c_stop();

    return 0;
}

uint8_t at24cxx_interface_iic_read_address16(uint8_t addr, uint16_t reg, uint8_t *buf, uint16_t len)
{
    if (len == 0U)
    {
        return 0;
    }
    if (buf == NULL)
    {
        return 1;
    }

    soft_i2c_start();
    if (soft_i2c_send_byte(addr))
    {
        soft_i2c_stop();
        return 1;
    }
    if (soft_i2c_send_byte((uint8_t)(reg >> 8)))
    {
        soft_i2c_stop();
        return 1;
    }
    if (soft_i2c_send_byte((uint8_t)(reg & 0xFFU)))
    {
        soft_i2c_stop();
        return 1;
    }
    soft_i2c_start();
    if (soft_i2c_send_byte((uint8_t)(addr | 0x01U)))
    {
        soft_i2c_stop();
        return 1;
    }
    for (uint16_t i = 0; i < len; ++i)
    {
        buf[i] = soft_i2c_receive_byte((i == (len - 1U)) ? 1U : 0U);
    }
    soft_i2c_stop();

    return 0;
}

uint8_t at24cxx_interface_iic_write_address16(uint8_t addr, uint16_t reg, uint8_t *buf, uint16_t len)
{
    if (len == 0U)
    {
        return 0;
    }
    if (buf == NULL)
    {
        return 1;
    }

    soft_i2c_start();
    if (soft_i2c_send_byte(addr))
    {
        soft_i2c_stop();
        return 1;
    }
    if (soft_i2c_send_byte((uint8_t)(reg >> 8)))
    {
        soft_i2c_stop();
        return 1;
    }
    if (soft_i2c_send_byte((uint8_t)(reg & 0xFFU)))
    {
        soft_i2c_stop();
        return 1;
    }
    for (uint16_t i = 0; i < len; ++i)
    {
        if (soft_i2c_send_byte(buf[i]))
        {
            soft_i2c_stop();
            return 1;
        }
    }
    soft_i2c_stop();

    return 0;
}

void at24cxx_interface_delay_ms(uint32_t ms)
{
    HAL_Delay(ms);
}

//void at24cxx_interface_debug_print(const char *const fmt, ...)
//{
//    va_list args;

//    va_start(args, fmt);
//    vprintf(fmt, args);
//    va_end(args);
//}


void at24cxx_interface_debug_print(const char *const fmt, ...)
{
    char buf[256];
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(buf, sizeof buf, fmt, ap);
    va_end(ap);
    if (n < 0) return;
    if (n >= (int)sizeof buf) n = sizeof buf - 1;

    /* 输出时把 \n 规范为 \r\n；末尾无换行则补一行 */
    for (int i = 0; i < n; ++i) {
        if (buf[i] == '\n') fputc('\r', stdout);
        fputc((unsigned char)buf[i], stdout);
    }
    if (n == 0 || buf[n - 1] != '\n') { fputc('\r', stdout); fputc('\n', stdout); }
}
```

</details>

<details>
<summary>bsp/at24xx/driver_at24cxx_interface.h</summary>

```c
/**
 * Copyright (c) 2015 - present LibDriver All rights reserved
 * 
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE. 
 *
 * @file      driver_at24cxx_interface.h
 * @brief     driver at24cxx interface header file
 * @version   2.0.0
 * @author    Shifeng Li
 * @date      2021-02-17
 *
 * <h3>history</h3>
 * <table>
 * <tr><th>Date        <th>Version  <th>Author      <th>Description
 * <tr><td>2021/02/17  <td>2.0      <td>Shifeng Li  <td>format the code
 * <tr><td>2020/10/15  <td>1.0      <td>Shifeng Li  <td>first upload
 * </table>
 */

#ifndef DRIVER_AT24CXX_INTERFACE_H
#define DRIVER_AT24CXX_INTERFACE_H

#include "driver_at24cxx.h"

#ifdef __cplusplus
extern "C"{
#endif

/**
 * @defgroup at24cxx_interface_driver at24cxx interface driver function
 * @brief    at24cxx interface driver modules
 * @ingroup  at24cxx_driver
 * @{
 */

/**
 * @brief  interface iic bus init
 * @return status code
 *         - 0 success
 *         - 1 iic init failed
 * @note   none
 */
uint8_t at24cxx_interface_iic_init(void);

/**
 * @brief  interface iic bus deinit
 * @return status code
 *         - 0 success
 *         - 1 iic deinit failed
 * @note   none
 */
uint8_t at24cxx_interface_iic_deinit(void);

/**
 * @brief      interface iic bus read
 * @param[in]  addr iic device write address
 * @param[in]  reg iic register address
 * @param[out] *buf pointer to a data buffer
 * @param[in]  len length of the data buffer
 * @return     status code
 *             - 0 success
 *             - 1 read failed
 * @note       none
 */
uint8_t at24cxx_interface_iic_read(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len);

/**
 * @brief     interface iic bus write
 * @param[in] addr iic device write address
 * @param[in] reg iic register address
 * @param[in] *buf pointer to a data buffer
 * @param[in] len length of the data buffer
 * @return    status code
 *            - 0 success
 *            - 1 write failed
 * @note      none
 */
uint8_t at24cxx_interface_iic_write(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len);

/**
 * @brief      interface iic bus read with 16 bits register address
 * @param[in]  addr iic device write address
 * @param[in]  reg iic register address
 * @param[out] *buf pointer to a data buffer
 * @param[in]  len length of the data buffer
 * @return     status code
 *             - 0 success
 *             - 1 read failed
 * @note       none
 */
uint8_t at24cxx_interface_iic_read_address16(uint8_t addr, uint16_t reg, uint8_t *buf, uint16_t len);

/**
 * @brief     interface iic bus write with 16 bits register address
 * @param[in] addr iic device write address
 * @param[in] reg iic register address
 * @param[in] *buf pointer to a data buffer
 * @param[in] len length of the data buffer
 * @return    status code
 *            - 0 success
 *            - 1 write failed
 * @note      none
 */
uint8_t at24cxx_interface_iic_write_address16(uint8_t addr, uint16_t reg, uint8_t *buf, uint16_t len);

/**
 * @brief     interface delay ms
 * @param[in] ms time
 * @note      none
 */
void at24cxx_interface_delay_ms(uint32_t ms);

/**
 * @brief     interface print format data
 * @param[in] fmt format data
 * @note      none
 */
void at24cxx_interface_debug_print(const char *const fmt, ...);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
```

</details>

<details>
<summary>bsp/at24xx/driver_at24cxx_interface_template.c</summary>

```c
/**
 * Copyright (c) 2015 - present LibDriver All rights reserved
 * 
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE. 
 *
 * @file      driver_at24cxx_interface_template.c
 * @brief     driver at24cxx interface template source file
 * @version   2.0.0
 * @author    Shifeng Li
 * @date      2021-02-17
 *
 * <h3>history</h3>
 * <table>
 * <tr><th>Date        <th>Version  <th>Author      <th>Description
 * <tr><td>2021/02/17  <td>2.0      <td>Shifeng Li  <td>format the code
 * <tr><td>2020/10/15  <td>1.0      <td>Shifeng Li  <td>first upload
 * </table>
 */

#include "driver_at24cxx_interface.h"

/**
 * @brief  interface iic bus init
 * @return status code
 *         - 0 success
 *         - 1 iic init failed
 * @note   none
 */
uint8_t at24cxx_interface_iic_init(void)
{
    return 0;
}

/**
 * @brief  interface iic bus deinit
 * @return status code
 *         - 0 success
 *         - 1 iic deinit failed
 * @note   none
 */
uint8_t at24cxx_interface_iic_deinit(void)
{
    return 0;
}

/**
 * @brief      interface iic bus read
 * @param[in]  addr iic device write address
 * @param[in]  reg iic register address
 * @param[out] *buf pointer to a data buffer
 * @param[in]  len length of the data buffer
 * @return     status code
 *             - 0 success
 *             - 1 read failed
 * @note       none
 */
uint8_t at24cxx_interface_iic_read(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len)
{
    return 0;
}

/**
 * @brief     interface iic bus write
 * @param[in] addr iic device write address
 * @param[in] reg iic register address
 * @param[in] *buf pointer to a data buffer
 * @param[in] len length of the data buffer
 * @return    status code
 *            - 0 success
 *            - 1 write failed
 * @note      none
 */
uint8_t at24cxx_interface_iic_write(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len)
{
    return 0;
}

/**
 * @brief      interface iic bus read with 16 bits register address
 * @param[in]  addr iic device write address
 * @param[in]  reg iic register address
 * @param[out] *buf pointer to a data buffer
 * @param[in]  len length of the data buffer
 * @return     status code
 *             - 0 success
 *             - 1 read failed
 * @note       none
 */
uint8_t at24cxx_interface_iic_read_address16(uint8_t addr, uint16_t reg, uint8_t *buf, uint16_t len)
{
    return 0;
}

/**
 * @brief     interface iic bus write with 16 bits register address
 * @param[in] addr iic device write address
 * @param[in] reg iic register address
 * @param[in] *buf pointer to a data buffer
 * @param[in] len length of the data buffer
 * @return    status code
 *            - 0 success
 *            - 1 write failed
 * @note      none
 */
uint8_t at24cxx_interface_iic_write_address16(uint8_t addr, uint16_t reg, uint8_t *buf, uint16_t len)
{
    return 0;
}

/**
 * @brief     interface delay ms
 * @param[in] ms time
 * @note      none
 */
void at24cxx_interface_delay_ms(uint32_t ms)
{

}

/**
 * @brief     interface print format data
 * @param[in] fmt format data
 * @note      none
 */
void at24cxx_interface_debug_print(const char *const fmt, ...)
{
    
}
```

</details>

<details>
<summary>bsp/at24xx/driver_at24cxx_read_test.c</summary>

```c
/**
 * Copyright (c) 2015 - present LibDriver All rights reserved
 * 
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE. 
 *
 * @file      driver_at24cxx_read_test.c
 * @brief     driver at24cxx read test source file
 * @version   2.0.0
 * @author    Shifeng Li
 * @date      2021-02-17
 *
 * <h3>history</h3>
 * <table>
 * <tr><th>Date        <th>Version  <th>Author      <th>Description
 * <tr><td>2021/02/17  <td>2.0      <td>Shifeng Li  <td>format the code
 * <tr><td>2020/10/15  <td>1.0      <td>Shifeng Li  <td>first upload
 * </table>
 */
 
#include "driver_at24cxx_read_test.h"
#include <stdlib.h>

static at24cxx_handle_t gs_handle;        /**< at24cxx handle */

/**
 * @brief     read test
 * @param[in] type chip type
 * @param[in] address chip address pin
 * @return    status code
 *            - 0 success
 *            - 1 test failed
 * @note      none
 */
uint8_t at24cxx_read_test(at24cxx_t type, at24cxx_address_t address)
{
    uint8_t res, i, j;
    uint8_t buf[12];
    uint8_t buf_check[12];
    uint32_t inc;
    at24cxx_info_t info;
    
    /* link interface function */
    DRIVER_AT24CXX_LINK_INIT(&gs_handle, at24cxx_handle_t);
    DRIVER_AT24CXX_LINK_IIC_INIT(&gs_handle, at24cxx_interface_iic_init);
    DRIVER_AT24CXX_LINK_IIC_DEINIT(&gs_handle, at24cxx_interface_iic_deinit);
    DRIVER_AT24CXX_LINK_IIC_READ(&gs_handle, at24cxx_interface_iic_read);
    DRIVER_AT24CXX_LINK_IIC_WRITE(&gs_handle, at24cxx_interface_iic_write);
    DRIVER_AT24CXX_LINK_IIC_READ_ADDRESS16(&gs_handle, at24cxx_interface_iic_read_address16);
    DRIVER_AT24CXX_LINK_IIC_WRITE_ADDRESS16(&gs_handle, at24cxx_interface_iic_write_address16);
    DRIVER_AT24CXX_LINK_DELAY_MS(&gs_handle, at24cxx_interface_delay_ms);
    DRIVER_AT24CXX_LINK_DEBUG_PRINT(&gs_handle, at24cxx_interface_debug_print);
    
    /* get information */
    res = at24cxx_info(&info);
    if (res != 0)
    {
        at24cxx_interface_debug_print("at24cxx: get info failed.\n");
       
        return 1;
    }
    else
    {
        /* print chip information */
        at24cxx_interface_debug_print("at24cxx: chip is %s.\n", info.chip_name);
        at24cxx_interface_debug_print("at24cxx: manufacturer is %s.\n", info.manufacturer_name);
        at24cxx_interface_debug_print("at24cxx: interface is %s.\n", info.interface);
        at24cxx_interface_debug_print("at24cxx: driver version is %d.%d.\n", info.driver_version / 1000, (info.driver_version % 1000) / 100);
        at24cxx_interface_debug_print("at24cxx: min supply voltage is %0.1fV.\n", info.supply_voltage_min_v);
        at24cxx_interface_debug_print("at24cxx: max supply voltage is %0.1fV.\n", info.supply_voltage_max_v);
        at24cxx_interface_debug_print("at24cxx: max current is %0.2fmA.\n", info.max_current_ma);
        at24cxx_interface_debug_print("at24cxx: max temperature is %0.1fC.\n", info.temperature_max);
        at24cxx_interface_debug_print("at24cxx: min temperature is %0.1fC.\n", info.temperature_min);
    }
    
    /* set chip type */
    res = at24cxx_set_type(&gs_handle, type);
    if (res != 0)
    {
        at24cxx_interface_debug_print("at24cxx: set type failed.\n");
       
        return 1;
    }
    
    /* set iic addr pin */
    res = at24cxx_set_addr_pin(&gs_handle, address);
    if (res != 0)
    {
        at24cxx_interface_debug_print("at24cxx: set address pin failed.\n");
       
        return 1;
    }
    
    /* at24cxx init */
    res = at24cxx_init(&gs_handle);
    if (res != 0)
    {
        at24cxx_interface_debug_print("at24cxx: init failed.\n");
       
        return 1;
    }
    
    /* start read test */
    at24cxx_interface_debug_print("at24cxx: start read test.\n");
    inc = ((uint32_t)type + 1) / 8;
    for (i = 0; i < 8; i++)
    {
        for (j = 0; j < 12; j++)
        {
            buf[j] = (uint8_t)(rand() % 256);
        }
    
        /* write data */
        res = at24cxx_write(&gs_handle, i*inc, (uint8_t *)buf, 12);
        if (res != 0)
        {
            at24cxx_interface_debug_print("at24cxx: write failed.\n");
            (void)at24cxx_deinit(&gs_handle);
            
            return 1;
        }

        /* read data */
        res = at24cxx_read(&gs_handle, i*inc, (uint8_t *)buf_check, 12);
        if (res != 0)
        {
            at24cxx_interface_debug_print("at24cxx: read failed.\n");
            (void)at24cxx_deinit(&gs_handle);
            
            return 1;
        }
        for (j = 0; j < 12; j++)
        {
            /* check data */
            if (buf[j] != buf_check[j])
            {
                at24cxx_interface_debug_print("at24cxx: check error.\n");
                (void)at24cxx_deinit(&gs_handle);
                
                return 1;
            }
        }
        at24cxx_interface_debug_print("at24cxx: 0x%04X read write test passed.\n", i*inc);
    }

    /* finish read test */
    at24cxx_interface_debug_print("at24cxx: finish read test.\n");
    (void)at24cxx_deinit(&gs_handle);
    
    return 0;
}
```

</details>

<details>
<summary>bsp/at24xx/driver_at24cxx_read_test.h</summary>

```c
/**
 * Copyright (c) 2015 - present LibDriver All rights reserved
 * 
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE. 
 *
 * @file      driver_at24cxx_read_test.h
 * @brief     driver at24cxx read test header file
 * @version   2.0.0
 * @author    Shifeng Li
 * @date      2021-02-17
 *
 * <h3>history</h3>
 * <table>
 * <tr><th>Date        <th>Version  <th>Author      <th>Description
 * <tr><td>2021/02/17  <td>2.0      <td>Shifeng Li  <td>format the code
 * <tr><td>2020/10/15  <td>1.0      <td>Shifeng Li  <td>first upload
 * </table>
 */

#ifndef DRIVER_AT24CXX_READ_TEST_H
#define DRIVER_AT24CXX_READ_TEST_H

#include "driver_at24cxx_interface.h"

#ifdef __cplusplus
extern "C"{
#endif

/**
 * @defgroup at24cxx_test_driver at24cxx test driver function
 * @brief    at24cxx test driver modules
 * @ingroup  at24cxx_driver
 * @{
 */

/**
 * @brief     read test
 * @param[in] type chip type
 * @param[in] address chip address pin
 * @return    status code
 *            - 0 success
 *            - 1 test failed
 * @note      none
 */
uint8_t at24cxx_read_test(at24cxx_t type, at24cxx_address_t address);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
```

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
