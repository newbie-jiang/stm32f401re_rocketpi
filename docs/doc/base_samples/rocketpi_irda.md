## 效果展示

![image-20251118025119702](https://cloud.rocketpi.club/cloud/image-20251118025119702.png)



## 功能说明

面向 RocketPI STM32F401RE 开发板的 **IRDA NEC红外解码  演示工程**。主要特性：

- 使用libdriver库实现解码 ，使用定时器1us计数  做一个红外解码

## 硬件连接

- IRDA_IO --- PA15  

## CubeMX配置

### 定时器配置

![image-20251118024922610](https://cloud.rocketpi.club/cloud/image-20251118024922610.png)

### 配置双边沿中断

![image-20251118025426011](https://cloud.rocketpi.club/cloud/image-20251118025426011.png)

<!-- BSP_DRIVERS_START -->
<!-- BSP_DRIVERS_HASH:eaef3fe1c3889ea0137b9443c19dfc81c04262477de8073e8b4eba1c2753ce5b -->
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
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "driver_ir_remote_receive_test.h"
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
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */
  if (ir_remote_interface_timer_init() != 0U)
  {
    ir_remote_interface_debug_print("ir_remote: timer init failed.\n");
    Error_Handler();
  }
  if (ir_remote_receive_test(100) != 0)
  {
    ir_remote_interface_debug_print("ir_remote: receive test failed.\n");
  }
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
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == IRDA_IO_Pin)
  {
    (void)ir_remote_receive_test_irq_handler();
  }
}

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
<summary>bsp/irda/driver_ir_remote.c</summary>

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
 * @file      driver_ir_remote.c
 * @brief     driver ir remote source file
 * @version   1.0.0
 * @author    Shifeng Li
 * @date      2023-03-31
 *
 * <h3>history</h3>
 * <table>
 * <tr><th>Date        <th>Version  <th>Author      <th>Description
 * <tr><td>2023/03/31  <td>1.0      <td>Shifeng Li  <td>first upload
 * </table>
 */

#include "driver_ir_remote.h"
#include <stdlib.h>

/**
 * @brief chip information definition
 */
#define CHIP_NAME                 "NEC IR REMOTE"        /**< chip name */
#define MANUFACTURER_NAME         "NEC"                  /**< manufacturer name */
#define SUPPLY_VOLTAGE_MIN        2.7f                   /**< chip min supply voltage */
#define SUPPLY_VOLTAGE_MAX        5.5f                   /**< chip max supply voltage */
#define MAX_CURRENT               1.5f                   /**< chip max current */
#define TEMPERATURE_MIN           -40.0f                 /**< chip min operating temperature */
#define TEMPERATURE_MAX           125.0f                 /**< chip max operating temperature */
#define DRIVER_VERSION            1000                   /**< driver version */

/**
 * @brief frame check definition
 */
#define IR_REMOTE_CHECK_START_HIGH        9000        /**< start frame high time */
#define IR_REMOTE_CHECK_START_LOW         4500        /**< start frame low time */
#define IR_REMOTE_CHECK_DATA0_HIGH        560         /**< bit 0 frame high time */
#define IR_REMOTE_CHECK_DATA0_LOW         560         /**< bit 0 frame low time */
#define IR_REMOTE_CHECK_DATA1_HIGH        560         /**< bit 1 frame high time */
#define IR_REMOTE_CHECK_DATA1_LOW         1680        /**< bit 1 frame low time */
#define IR_REMOTE_CHECK_DATA_0_1_EDGE     1000        /**< data 0 and 1 edge */
#define IR_REMOTE_CHECK_STOP              560         /**< stop time */
#define IR_REMOTE_CHECK_REPEAT            2250        /**< repeat time */

/**
 * @brief     check the frame time
 * @param[in] check checked time
 * @param[in] t standard time
 * @return    status code
 *            - 0 success
 *            - 1 checked failed
 * @note      none
 */
static inline uint8_t a_check_frame(uint16_t check, uint16_t t)
{
    if (abs((int)check - (int)t) > (int)((float)(t) * IR_REMOTE_MAX_RANGE))        /* check the time */
    {
        return 1;                                                                  /* check failed */
    }
    else
    {
        return 0;                                                                  /* success return 0 */
    }
}

/**
 * @brief     ir_remote nec repeat decode
 * @param[in] *handle pointer to an ir_remote handle structure
 * @note      none
 */
static void a_ir_remote_nec_repeat_decode(ir_remote_handle_t *handle)
{
    uint16_t i;
    uint16_t len;
    ir_remote_t data;
    
    len = handle->decode_len - 1;                                                         /* len - 1 */
    for (i = 0; i < len; i++)                                                             /* diff all time */
    {
        int64_t diff;
        
        diff = (int64_t)((int64_t)handle->decode[i + 1].t.s -
               (int64_t)handle->decode[i].t.s) * 1000000 + 
               (int64_t)((int64_t)handle->decode[i + 1].t.us -
               (int64_t)handle->decode[i].t.us);                                          /* diff time */
        handle->decode[i].diff_us = (uint32_t)diff;                                       /* save the time diff */
    }
    
    if (a_check_frame(handle->decode[0].diff_us, IR_REMOTE_CHECK_START_HIGH) != 0)        /* check start diff */
    {
        if (handle->receive_callback != NULL)                                             /* check the receive callback */
        {
            data.address = 0x00;                                                          /* set address 0x00 */
            data.command = 0x00;                                                          /* set command 0x00 */
            data.status = IR_REMOTE_STATUS_FRAME_INVALID;                                 /* frame invalid */
            handle->receive_callback(&data);                                              /* run the callback */
        }
        
        return;                                                                           /* return */
    }
    if (a_check_frame(handle->decode[1].diff_us, IR_REMOTE_CHECK_REPEAT) != 0)            /* check repeat */
    {
        if (handle->receive_callback != NULL)                                             /* check the receive callback */
        {
            data.address = 0x00;                                                          /* set address 0x00 */
            data.command = 0x00;                                                          /* set command 0x00 */
            data.status = IR_REMOTE_STATUS_FRAME_INVALID;                                 /* frame invalid */
            handle->receive_callback(&data);                                              /* run the callback */
        }
        
        return;                                                                           /* return */
    }
    if (a_check_frame(handle->decode[2].diff_us, IR_REMOTE_CHECK_STOP) != 0)              /* check stop */
    {
        if (handle->receive_callback != NULL)                                             /* check the receive callback */
        {
            data.address = 0x00;                                                          /* set address 0x00 */
            data.command = 0x00;                                                          /* set command 0x00 */
            data.status = IR_REMOTE_STATUS_FRAME_INVALID;                                 /* frame invalid */
            handle->receive_callback(&data);                                              /* run the callback */
        }
        
        return;                                                                           /* return */
    }
    
    if (handle->receive_callback != NULL)                                                 /* check the receive callback */
    {
        data.address = handle->last_code.address;                                         /* set address 0x00 */
        data.command = handle->last_code.command;                                         /* set command 0x00 */
        data.status = IR_REMOTE_STATUS_REPEAT;                                            /* frame invalid */
        handle->receive_callback(&data);                                                  /* run the callback */
    }
    handle->decode_len = 0;                                                               /* in order to trigger repeat decoding for the next four edges */
}

/**
 * @brief     ir_remote nec decode
 * @param[in] *handle pointer to an ir_remote handle structure
 * @note      none
 */
static void a_ir_remote_nec_decode(ir_remote_handle_t *handle)
{
    uint8_t tmp;
    uint8_t tmp_r;
    uint8_t tmp_cmp;
    uint16_t i;
    uint16_t len;
    ir_remote_t data;
    
    len = handle->decode_len - 1;                                                                             /* len - 1 */
    for (i = 0; i < len; i++)                                                                                 /* diff all time */
    {
        int64_t diff;
        
        diff = (int64_t)((int64_t)handle->decode[i + 1].t.s -
               (int64_t)handle->decode[i].t.s) * 1000000 + 
               (int64_t)((int64_t)handle->decode[i + 1].t.us -
               (int64_t)handle->decode[i].t.us);                                                              /* diff time */
        handle->decode[i].diff_us = (uint32_t)diff;                                                           /* save the time diff */
    }
    
    if (a_check_frame(handle->decode[0].diff_us, IR_REMOTE_CHECK_START_HIGH) != 0)                            /* check start diff */
    {
        if (handle->receive_callback != NULL)                                                                 /* check the receive callback */
        {
            data.address = 0x00;                                                                              /* set address 0x00 */
            data.command = 0x00;                                                                              /* set command 0x00 */
            data.status = IR_REMOTE_STATUS_FRAME_INVALID;                                                     /* frame invalid */
            handle->receive_callback(&data);                                                                  /* run the callback */
        }
        
        return;                                                                                               /* return */
    }
    if (a_check_frame(handle->decode[1].diff_us, IR_REMOTE_CHECK_START_LOW) != 0)                             /* check start low */
    {
        if (handle->receive_callback != NULL)                                                                 /* check the receive callback */
        {
            data.address = 0x00;                                                                              /* set address 0x00 */
            data.command = 0x00;                                                                              /* set command 0x00 */
            data.status = IR_REMOTE_STATUS_FRAME_INVALID;                                                     /* frame invalid */
            handle->receive_callback(&data);                                                                  /* run the callback */
        }
        
        return;                                                                                               /* return */
    }
    
    tmp = 0;                                                                                                  /* init 0 */
    for (i = 0; i < 8; i++)                                                                                   /* parse 8 bit */
    {
        if (a_check_frame(handle->decode[2 + i * 2 + 0].diff_us, IR_REMOTE_CHECK_DATA1_HIGH) != 0)            /* check data high */
        {
            if (handle->receive_callback != NULL)                                                             /* check the receive callback */
            {
                data.address = 0x00;                                                                          /* set address 0x00 */
                data.command = 0x00;                                                                          /* set command 0x00 */
                data.status = IR_REMOTE_STATUS_ADDR_ERR;                                                      /* address error */
                handle->receive_callback(&data);                                                              /* run the callback */
            }
            
            return;                                                                                           /* return */
        }
        if (handle->decode[2 + i * 2 + 1].diff_us > IR_REMOTE_CHECK_DATA_0_1_EDGE)                            /* check data0 and data1 */
        {
            if (a_check_frame(handle->decode[2 + i * 2 + 1].diff_us, IR_REMOTE_CHECK_DATA1_LOW) != 0)         /* check data 1 */
            {
                data.address = 0x00;                                                                          /* set address 0x00 */
                data.command = 0x00;                                                                          /* set command 0x00 */
                data.status = IR_REMOTE_STATUS_ADDR_ERR;                                                      /* address error */
                if (handle->receive_callback != NULL)                                                         /* check the receive callback */
                {
                    handle->receive_callback(&data);                                                          /* run the callback */
                }
                
                return;                                                                                       /* return */
            }
            tmp |= 1 << i;                                                                                    /* set bit */
        }
        else                                                                                                  /* check data 0 */
        {
            if (a_check_frame(handle->decode[2 + i * 2 + 1].diff_us, IR_REMOTE_CHECK_DATA0_LOW) != 0)         /* check data 0 */
            {
                data.address = 0x00;                                                                          /* set address 0x00 */
                data.command = 0x00;                                                                          /* set command 0x00 */
                data.status = IR_REMOTE_STATUS_ADDR_ERR;                                                      /* address error */
                if (handle->receive_callback != NULL)                                                         /* check the receive callback */
                {
                    handle->receive_callback(&data);                                                          /* run the callback */
                }
                
                return;                                                                                       /* return */
            }
            tmp |= 0 << i;                                                                                    /* set bit */
        }
    }
    tmp_r = 0;                                                                                                /* init 0 */
    for (i = 0; i < 8; i++)                                                                                   /* parse 8 bit */
    {
        if (a_check_frame(handle->decode[18 + i * 2 + 0].diff_us, IR_REMOTE_CHECK_DATA1_HIGH) != 0)           /* check data high */
        {
            if (handle->receive_callback != NULL)                                                             /* check the receive callback */
            {
                data.address = 0x00;                                                                          /* set address 0x00 */
                data.command = 0x00;                                                                          /* set command 0x00 */
                data.status = IR_REMOTE_STATUS_ADDR_ERR;                                                      /* address error */
                handle->receive_callback(&data);                                                              /* run the callback */
            }
            
            return;                                                                                           /* return */
        }
        if (handle->decode[18 + i * 2 + 1].diff_us > IR_REMOTE_CHECK_DATA_0_1_EDGE)                           /* check data0 and data1 */
        {
            if (a_check_frame(handle->decode[18 + i * 2 + 1].diff_us, IR_REMOTE_CHECK_DATA1_LOW) != 0)        /* check data 1 */
            {
                data.address = 0x00;                                                                          /* set address 0x00 */
                data.command = 0x00;                                                                          /* set command 0x00 */
                data.status = IR_REMOTE_STATUS_ADDR_ERR;                                                      /* address error */
                if (handle->receive_callback != NULL)                                                         /* check the receive callback */
                {
                    handle->receive_callback(&data);                                                          /* run the callback */
                }
                
                return;                                                                                       /* return */
            }
            tmp_r |= 1 << i;                                                                                  /* set bit */
        }
        else                                                                                                  /* check data 0 */
        {
            if (a_check_frame(handle->decode[18 + i * 2 + 1].diff_us, IR_REMOTE_CHECK_DATA0_LOW) != 0)        /* check data 0 */
            {
                data.address = 0x00;                                                                          /* set address 0x00 */
                data.command = 0x00;                                                                          /* set command 0x00 */
                data.status = IR_REMOTE_STATUS_ADDR_ERR;                                                      /* address error */
                if (handle->receive_callback != NULL)                                                         /* check the receive callback */
                {
                    handle->receive_callback(&data);                                                          /* run the callback */
                }
                
                return;                                                                                       /* return */
            }
            tmp_r |= 0 << i;                                                                                  /* set bit */
        }
    }
    tmp_cmp = ~tmp_r;                                                                                         /* get the check value */
    if (tmp != tmp_cmp)                                                                                       /* check the value */
    {
        data.address = 0x00;                                                                                  /* set address 0x00 */
        data.command = 0x00;                                                                                  /* set command 0x00 */
        data.status = IR_REMOTE_STATUS_ADDR_ERR;                                                              /* address error */
        if (handle->receive_callback != NULL)                                                                 /* check the receive callback */
        {
            handle->receive_callback(&data);                                                                  /* run the callback */
        }
        
        return;                                                                                               /* return */
    }
    data.address = tmp;                                                                                       /* set the address */
    
    tmp = 0;                                                                                                  /* init 0 */
    for (i = 0; i < 8; i++)                                                                                   /* parse 8 bit */
    {
        if (a_check_frame(handle->decode[34 + i * 2 + 0].diff_us, IR_REMOTE_CHECK_DATA1_HIGH) != 0)           /* check data high */
        {
            if (handle->receive_callback != NULL)                                                             /* check the receive callback */
            {
                data.command = 0x00;                                                                          /* set command 0x00 */
                data.status = IR_REMOTE_STATUS_CMD_ERR;                                                       /* command error */
                handle->receive_callback(&data);                                                              /* run the callback */
            }
            
            return;                                                                                           /* return */
        }
        if (handle->decode[34 + i * 2 + 1].diff_us > IR_REMOTE_CHECK_DATA_0_1_EDGE)                           /* check data0 and data1 */
        {
            if (a_check_frame(handle->decode[34 + i * 2 + 1].diff_us, IR_REMOTE_CHECK_DATA1_LOW) != 0)        /* check data 1 */
            {
                data.command = 0x00;                                                                          /* set command 0x00 */
                data.status = IR_REMOTE_STATUS_CMD_ERR;                                                       /* command error */
                if (handle->receive_callback != NULL)                                                         /* check the receive callback */
                {
                    handle->receive_callback(&data);                                                          /* run the callback */
                }
                
                return;                                                                                       /* return */
            }
            tmp |= 1 << i;                                                                                    /* set bit */
        }
        else                                                                                                  /* check data 0 */
        {
            if (a_check_frame(handle->decode[34 + i * 2 + 1].diff_us, IR_REMOTE_CHECK_DATA0_LOW) != 0)        /* check data 0 */
            {
                data.command = 0x00;                                                                          /* set command 0x00 */
                data.status = IR_REMOTE_STATUS_CMD_ERR;                                                       /* command error */
                if (handle->receive_callback != NULL)                                                         /* check the receive callback */
                {
                    handle->receive_callback(&data);                                                          /* run the callback */
                }
                
                return;                                                                                       /* return */
            }
            tmp |= 0 << i;                                                                                    /* set bit */
        }
    }
    tmp_r = 0;                                                                                                /* init 0 */
    for (i = 0; i < 8; i++)                                                                                   /* parse 8 bit */
    {
        if (a_check_frame(handle->decode[50 + i * 2 + 0].diff_us, IR_REMOTE_CHECK_DATA1_HIGH) != 0)           /* check data high */
        {
            if (handle->receive_callback != NULL)                                                             /* check the receive callback */
            {
                data.command = 0x00;                                                                          /* set command 0x00 */
                data.status = IR_REMOTE_STATUS_CMD_ERR;                                                       /* command error */
                handle->receive_callback(&data);                                                              /* run the callback */
            }
            
            return;                                                                                           /* return */
        }
        if (handle->decode[50 + i * 2 + 1].diff_us > IR_REMOTE_CHECK_DATA_0_1_EDGE)                           /* check data0 and data1 */
        {
            if (a_check_frame(handle->decode[50 + i * 2 + 1].diff_us, IR_REMOTE_CHECK_DATA1_LOW) != 0)        /* check data 1 */
            {
                data.command = 0x00;                                                                          /* set command 0x00 */
                data.status = IR_REMOTE_STATUS_CMD_ERR;                                                       /* command error */
                if (handle->receive_callback != NULL)                                                         /* check the receive callback */
                {
                    handle->receive_callback(&data);                                                          /* run the callback */
                }
                
                return;                                                                                       /* return */
            }
            tmp_r |= 1 << i;                                                                                  /* set bit */
        }
        else                                                                                                  /* check data 0 */
        {
            if (a_check_frame(handle->decode[50 + i * 2 + 1].diff_us, IR_REMOTE_CHECK_DATA0_LOW) != 0)        /* check data 0 */
            {
                data.command = 0x00;                                                                          /* set command 0x00 */
                data.status = IR_REMOTE_STATUS_CMD_ERR;                                                       /* command error */
                if (handle->receive_callback != NULL)                                                         /* check the receive callback */
                {
                    handle->receive_callback(&data);                                                          /* run the callback */
                }
                
                return;                                                                                       /* return */
            }
            tmp_r |= 0 << i;                                                                                  /* set bit */
        }
    }
    tmp_cmp = ~tmp_r;                                                                                         /* get the check value */
    if (tmp != tmp_cmp)                                                                                       /* check the value */
    {
        data.command = 0x00;                                                                                  /* set command 0x00 */
        data.status = IR_REMOTE_STATUS_CMD_ERR;                                                               /* command error */
        if (handle->receive_callback != NULL)                                                                 /* check the receive callback */
        {
            handle->receive_callback(&data);                                                                  /* run the callback */
        }
        
        return;                                                                                               /* return */
    }
    data.command = tmp;                                                                                       /* set the command */
    
    if (a_check_frame(handle->decode[66].diff_us, IR_REMOTE_CHECK_STOP) != 0)                                 /* check stop frame */
    {
        if (handle->receive_callback != NULL)                                                                 /* check the receive callback */
        {
            data.status = IR_REMOTE_STATUS_FRAME_INVALID;                                                     /* frame invalid */
            handle->receive_callback(&data);                                                                  /* run the callback */
        }
        
        return;                                                                                               /* return */
    }
    
    if (handle->receive_callback != NULL)                                                                     /* check the receive callback */
    {
        data.status = IR_REMOTE_STATUS_OK;                                                                    /* frame ok */
        handle->receive_callback(&data);                                                                      /* run the callback */
    }
    handle->last_code.address = data.address;                                                                 /* save address */
    handle->last_code.command = data.command;                                                                 /* save command */
    handle->last_code.status = data.status;                                                                   /* save status */
    handle->decode_len = 0;                                                                                   /* clear the buffer */
}

/**
 * @brief     irq handler
 * @param[in] *handle pointer to an ir_remote handle structure
 * @return    status code
 *            - 0 success
 *            - 1 run failed
 *            - 2 handle is NULL
 *            - 3 handle is not initialized
 * @note      none
 */
uint8_t ir_remote_irq_handler(ir_remote_handle_t *handle)
{
    uint8_t res;
    int64_t diff;
    ir_remote_time_t t;
    
    if (handle == NULL)                                                    /* check handle */
    {
        return 2;                                                          /* return error */
    }
    if (handle->inited != 1)                                               /* check handle initialization */
    {
        return 3;                                                          /* return error */
    }
    
    res = handle->timestamp_read(&t);                                      /* timestamp read */
    if (res != 0)                                                          /* check result */
    {
        handle->debug_print("ir_remote: timestamp read failed.\n");        /* timestamp read failed */
        
        return 1;                                                          /* return error */
    }
    diff = (int64_t)((int64_t)t.s - 
           (int64_t)handle->last_time.s) * 1000000 + 
           (int64_t)((int64_t)t.us - (int64_t)handle->last_time.us);       /* now - last time */
    if (diff - (int64_t)200000L >= 0)                                      /* if over 1s, force reset */
    {
        handle->decode_len = 0;                                            /* reset the decode */
    }
    if (handle->decode_len >= 127)                                         /* check the max length */
    {
        handle->decode_len = 0;                                            /* reset the decode */
    }
    handle->decode[handle->decode_len].t.s = t.s;                          /* save s */
    handle->decode[handle->decode_len].t.us = t.us;                        /* save us */
    handle->decode_len++;                                                  /* length++ */
    handle->last_time.s = t.s;                                             /* save last time */
    handle->last_time.us = t.us;                                           /* save last time */
    if (handle->decode_len >= 68)                                          /* check the end length */
    {
        diff = (int64_t)((int64_t)handle->decode[2].t.s -
               (int64_t)handle->decode[1].t.s) * 1000000 + 
               (int64_t)((int64_t)handle->decode[2].t.us -
               (int64_t)handle->decode[1].t.us);                           /* diff time */
        if (a_check_frame((uint16_t)diff, IR_REMOTE_CHECK_START_LOW) == 0) /* check the frame */
        {
            a_ir_remote_nec_decode(handle);                                /* try to decode */
        }
    }
    if (handle->decode_len == 4)                                           /* check the end length */
    {
        diff = (int64_t)((int64_t)handle->decode[2].t.s -
               (int64_t)handle->decode[1].t.s) * 1000000 + 
               (int64_t)((int64_t)handle->decode[2].t.us -
               (int64_t)handle->decode[1].t.us);                           /* diff time */
        if (a_check_frame((uint16_t)diff, IR_REMOTE_CHECK_REPEAT) == 0)    /* check the frame */
        {
            a_ir_remote_nec_repeat_decode(handle);                         /* try to decode */
        }
    }
    
    return 0;                                                              /* success return 0 */
}

/**
 * @brief     initialize the chip
 * @param[in] *handle pointer to an ir_remote handle structure
 * @return    status code
 *            - 0 success
 *            - 1 gpio initialization failed
 *            - 2 handle is NULL
 *            - 3 linked functions is NULL
 * @note      none
 */
uint8_t ir_remote_init(ir_remote_handle_t *handle)
{
    uint8_t res;
    ir_remote_time_t t;
    
    if (handle == NULL)                                                       /* check handle */
    {
        return 2;                                                             /* return error */
    }
    if (handle->debug_print == NULL)                                          /* check debug_print */
    {
        return 3;                                                             /* return error */
    }
    if (handle->timestamp_read == NULL)                                       /* check timestamp_read */
    {
        handle->debug_print("ir_remote: timestamp_read is null.\n");          /* timestamp_read is null */
        
        return 3;                                                             /* return error */
    }
    if (handle->delay_ms == NULL)                                             /* check delay_ms */
    {
        handle->debug_print("ir_remote: delay_ms is null.\n");                /* delay_ms is null */
        
        return 3;                                                             /* return error */
    }
    if (handle->receive_callback == NULL)                                     /* check receive_callback */
    {
        handle->debug_print("ir_remote: receive_callback is null.\n");        /* receive_callback is null */
        
        return 3;                                                             /* return error */
    }
    
    res = handle->timestamp_read(&t);                                         /* timestamp read */
    if (res != 0)                                                             /* check result */
    {
        handle->debug_print("ir_remote: timestamp read failed.\n");           /* timestamp read failed */
        
        return 1;                                                             /* return error */
    }
    handle->last_time.s = t.s;                                                /* save last time */
    handle->last_time.us = t.us;                                              /* save last time */
    handle->last_code.address = 0x00;                                         /* init address 0 */
    handle->last_code.command = 0x00;                                         /* init command 0 */
    handle->last_code.status = 0x00;                                          /* init status 0 */
    handle->decode_len = 0;                                                   /* init 0 */
    handle->inited = 1;                                                       /* flag inited */
    
    return 0;                                                                 /* success return 0 */
}

/**
 * @brief     close the chip
 * @param[in] *handle pointer to an ir_remote handle structure
 * @return    status code
 *            - 0 success
 *            - 2 handle is NULL
 *            - 3 handle is not initialized
 * @note      none
 */
uint8_t ir_remote_deinit(ir_remote_handle_t *handle)
{
    if (handle == NULL)             /* check handle */
    {
        return 2;                   /* return error */
    }
    if (handle->inited != 1)        /* check handle initialization */
    {
        return 3;                   /* return error */
    }
    
    handle->inited = 0;             /* flag close */
    
    return 0;                       /* success return 0 */
}

/**
 * @brief      get chip's information
 * @param[out] *info pointer to an ir_remote info structure
 * @return     status code
 *             - 0 success
 *             - 2 handle is NULL
 * @note       none
 */
uint8_t ir_remote_info(ir_remote_info_t *info)
{
    if (info == NULL)                                               /* check handle */
    {
        return 2;                                                   /* return error */
    }
    
    memset(info, 0, sizeof(ir_remote_info_t));                      /* initialize ir_remote info structure */
    strncpy(info->chip_name, CHIP_NAME, 32);                        /* copy chip name */
    strncpy(info->manufacturer_name, MANUFACTURER_NAME, 32);        /* copy manufacturer name */
    strncpy(info->interface, "GPIO", 8);                            /* copy interface name */
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
<summary>bsp/irda/driver_ir_remote.h</summary>

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
 * @file      driver_ir_remote.h
 * @brief     driver ir remote header file
 * @version   1.0.0
 * @author    Shifeng Li
 * @date      2023-03-31
 *
 * <h3>history</h3>
 * <table>
 * <tr><th>Date        <th>Version  <th>Author      <th>Description
 * <tr><td>2023/03/31  <td>1.0      <td>Shifeng Li  <td>first upload
 * </table>
 */

#ifndef DRIVER_IR_REMOTE_H
#define DRIVER_IR_REMOTE_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C"{
#endif

/**
 * @defgroup ir_remote_driver ir_remote driver function
 * @brief    ir_remote driver modules
 * @{
 */

/**
 * @addtogroup ir_remote_basic_driver
 * @{
 */

/**
 * @brief ir_remote max range definition
 */
#ifndef IR_REMOTE_MAX_RANGE
    #define IR_REMOTE_MAX_RANGE        0.20f        /**< 20% */
#endif

/**
 * @brief ir_remote status enumeration definition
 */
typedef enum
{
    IR_REMOTE_STATUS_OK            = 0x00,        /**< ok */
    IR_REMOTE_STATUS_REPEAT        = 0x01,        /**< repeat */
    IR_REMOTE_STATUS_ADDR_ERR      = 0x02,        /**< addr error */
    IR_REMOTE_STATUS_CMD_ERR       = 0x03,        /**< cmd error */
    IR_REMOTE_STATUS_FRAME_INVALID = 0x04,        /**< frame invalid */
} ir_remote_status_t;

/**
 * @brief ir_remote structure definition
 */
typedef struct ir_remote_s
{
    uint8_t status;        /**< status */
    uint8_t address;       /**< address */
    uint8_t command;       /**< command */
} ir_remote_t;

/**
 * @brief ir_remote time structure definition
 */
typedef struct ir_remote_time_s
{
    uint64_t s;         /**< second */
    uint32_t us;        /**< microsecond */
} ir_remote_time_t;

/**
 * @brief ir_remote decode structure definition
 */
typedef struct ir_remote_decode_s
{
    ir_remote_time_t t;        /**< timestamp */
    uint32_t diff_us;          /**< diff us */
} ir_remote_decode_t;

/**
 * @brief ir_remote handle structure definition
 */
typedef struct ir_remote_handle_s
{
    uint8_t (*timestamp_read)(ir_remote_time_t *t);         /**< point to an timestamp_read function address */
    void (*delay_ms)(uint32_t ms);                          /**< point to a delay_ms function address */
    void (*debug_print)(const char *const fmt, ...);        /**< point to a debug_print function address */
    void (*receive_callback)(ir_remote_t *data);            /**< point to a receive_callback function address */
    uint8_t inited;                                         /**< inited flag */
    ir_remote_decode_t decode[128];                         /**< decode buffer */
    uint16_t decode_len;                                    /**< decode length */
    ir_remote_time_t last_time;                             /**< last time */
    ir_remote_t last_code;                                  /**< last code */
} ir_remote_handle_t;

/**
 * @brief ir_remote information structure definition
 */
typedef struct ir_remote_info_s
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
} ir_remote_info_t;

/**
 * @}
 */

/**
 * @defgroup ir_remote_link_driver ir_remote link driver function
 * @brief    ir_remote link driver modules
 * @ingroup  ir_remote_driver
 * @{
 */

/**
 * @brief     initialize ir_remote_handle_t structure
 * @param[in] HANDLE pointer to an ir_remote handle structure
 * @param[in] STRUCTURE ir_remote_handle_t
 * @note      none
 */
#define DRIVER_IR_REMOTE_LINK_INIT(HANDLE, STRUCTURE)         memset(HANDLE, 0, sizeof(STRUCTURE))

/**
 * @brief     link timestamp_read function
 * @param[in] HANDLE pointer to an ir_remote handle structure
 * @param[in] FUC pointer to a timestamp_read function address
 * @note      none
 */
#define DRIVER_IR_REMOTE_LINK_TIMESTAMP_READ(HANDLE, FUC)    (HANDLE)->timestamp_read = FUC

/**
 * @brief     link delay_ms function
 * @param[in] HANDLE pointer to an ir_remote handle structure
 * @param[in] FUC pointer to a delay_ms function address
 * @note      none
 */
#define DRIVER_IR_REMOTE_LINK_DELAY_MS(HANDLE, FUC)          (HANDLE)->delay_ms = FUC

/**
 * @brief     link debug_print function
 * @param[in] HANDLE pointer to an ir_remote handle structure
 * @param[in] FUC pointer to a debug_print function address
 * @note      none
 */
#define DRIVER_IR_REMOTE_LINK_DEBUG_PRINT(HANDLE, FUC)       (HANDLE)->debug_print = FUC

/**
 * @brief     link receive_callback function
 * @param[in] HANDLE pointer to an ir_remote handle structure
 * @param[in] FUC pointer to a receive_callback function address
 * @note      none
 */
#define DRIVER_IR_REMOTE_LINK_RECEIVE_CALLBACK(HANDLE, FUC)  (HANDLE)->receive_callback = FUC

/**
 * @}
 */

/**
 * @defgroup ir_remote_basic_driver ir_remote basic driver function
 * @brief    ir_remote basic driver modules
 * @ingroup  ir_remote_driver
 * @{
 */

/**
 * @brief      get chip's information
 * @param[out] *info pointer to an ir_remote info structure
 * @return     status code
 *             - 0 success
 *             - 2 handle is NULL
 * @note       none
 */
uint8_t ir_remote_info(ir_remote_info_t *info);

/**
 * @brief     irq handler
 * @param[in] *handle pointer to an ir_remote handle structure
 * @return    status code
 *            - 0 success
 *            - 1 run failed
 *            - 2 handle is NULL
 *            - 3 handle is not initialized
 * @note      none
 */
uint8_t ir_remote_irq_handler(ir_remote_handle_t *handle);

/**
 * @brief     initialize the chip
 * @param[in] *handle pointer to an ir_remote handle structure
 * @return    status code
 *            - 0 success
 *            - 1 gpio initialization failed
 *            - 2 handle is NULL
 *            - 3 linked functions is NULL
 * @note      none
 */
uint8_t ir_remote_init(ir_remote_handle_t *handle);

/**
 * @brief     close the chip
 * @param[in] *handle pointer to an ir_remote handle structure
 * @return    status code
 *            - 0 success
 *            - 2 handle is NULL
 *            - 3 handle is not initialized
 * @note      none
 */
uint8_t ir_remote_deinit(ir_remote_handle_t *handle);

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
<summary>bsp/irda/driver_ir_remote_basic.c</summary>

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
 * @file      driver_ir_remote_basic.c
 * @brief     driver ir_remote basic source file
 * @version   1.0.0
 * @author    Shifeng Li
 * @date      2023-03-31
 *
 * <h3>history</h3>
 * <table>
 * <tr><th>Date        <th>Version  <th>Author      <th>Description
 * <tr><td>2023/03/31  <td>1.0      <td>Shifeng Li  <td>first upload
 * </table>
 */

#include "driver_ir_remote_basic.h"

static ir_remote_handle_t gs_handle;        /**< ir_remote handle */

/**
 * @brief  basic irq
 * @return status code
 *         - 0 success
 *         - 1 run failed
 * @note   none
 */
uint8_t ir_remote_basic_irq_handler(void)
{
    if (ir_remote_irq_handler(&gs_handle) != 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/**
 * @brief     basic example init
 * @param[in] *callback pointer to an irq callback address
 * @return    status code
 *            - 0 success
 *            - 1 init failed
 * @note      none
 */
uint8_t ir_remote_basic_init(void (*callback)(ir_remote_t *data))
{
    uint8_t res;
    
    /* link interface function */
    DRIVER_IR_REMOTE_LINK_INIT(&gs_handle, ir_remote_handle_t);
    DRIVER_IR_REMOTE_LINK_TIMESTAMP_READ(&gs_handle, ir_remote_interface_timestamp_read);
    DRIVER_IR_REMOTE_LINK_DELAY_MS(&gs_handle, ir_remote_interface_delay_ms);
    DRIVER_IR_REMOTE_LINK_DEBUG_PRINT(&gs_handle, ir_remote_interface_debug_print);
    DRIVER_IR_REMOTE_LINK_RECEIVE_CALLBACK(&gs_handle, callback);
    
    /* init */
    res = ir_remote_init(&gs_handle);
    if (res != 0)
    {
        ir_remote_interface_debug_print("ir_remote: init failed.\n");
       
        return 1;
    }
    
    return 0;
}

/**
 * @brief  basic example deinit
 * @return status code
 *         - 0 success
 *         - 1 deinit failed
 * @note   none
 */
uint8_t ir_remote_basic_deinit(void)
{
    if (ir_remote_deinit(&gs_handle) != 0)
    {
        return 1;
    }
    
    return 0;
}
```

</details>

<details>
<summary>bsp/irda/driver_ir_remote_basic.h</summary>

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
 * @file      driver_ir_remote_basic.h
 * @brief     driver ir_remote basic header file
 * @version   1.0.0
 * @author    Shifeng Li
 * @date      2023-03-31
 *
 * <h3>history</h3>
 * <table>
 * <tr><th>Date        <th>Version  <th>Author      <th>Description
 * <tr><td>2023/03/31  <td>1.0      <td>Shifeng Li  <td>first upload
 * </table>
 */

#ifndef DRIVER_IR_REMOTE_BASIC_H
#define DRIVER_IR_REMOTE_BASIC_H

#include "driver_ir_remote_interface.h"

#ifdef __cplusplus
extern "C"{
#endif

/**
 * @defgroup ir_remote_example_driver ir_remote example driver function
 * @brief    ir_remote example driver modules
 * @ingroup  ir_remote_driver
 * @{
 */

/**
 * @brief  basic irq
 * @return status code
 *         - 0 success
 *         - 1 run failed
 * @note   none
 */
uint8_t ir_remote_basic_irq_handler(void);

/**
 * @brief     basic example init
 * @param[in] *callback pointer to an irq callback address
 * @return    status code
 *            - 0 success
 *            - 1 init failed
 * @note      none
 */
uint8_t ir_remote_basic_init(void (*callback)(ir_remote_t *data));

/**
 * @brief  basic example deinit
 * @return status code
 *         - 0 success
 *         - 1 deinit failed
 * @note   none
 */
uint8_t ir_remote_basic_deinit(void);

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
<summary>bsp/irda/driver_ir_remote_interface.c</summary>

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
 * @file      driver_ir_remote_interface_template.c
 * @brief     driver ir_remote interface template source file
 * @version   1.0.0
 * @author    Shifeng Li
 * @date      2023-03-31
 *
 * <h3>history</h3>
 * <table>
 * <tr><th>Date        <th>Version  <th>Author      <th>Description
 * <tr><td>2023/03/31  <td>1.0      <td>Shifeng Li  <td>first upload
 * </table>
 */

#include "driver_ir_remote_interface.h"
#include "main.h"
#include "tim.h"

#include <stdarg.h>
#include <stdio.h>

#define IR_REMOTE_TIM1_PERIOD_US    ((uint32_t)(htim1.Init.Period + 1U))

static volatile uint64_t s_tim1_elapsed_us = 0U;
static volatile uint8_t s_tim1_started = 0U;

static uint8_t ir_remote_interface_tim1_start(void)
{
    if (s_tim1_started != 0U)
    {
        return 0U;
    }

    HAL_TIM_StateTypeDef state = HAL_TIM_Base_GetState(&htim1);
    if (state == HAL_TIM_STATE_BUSY)
    {
        if (HAL_TIM_Base_Stop_IT(&htim1) != HAL_OK)
        {
            return 1U;
        }
        state = HAL_TIM_Base_GetState(&htim1);
    }
    if (state != HAL_TIM_STATE_READY)
    {
        return 1U;
    }

    s_tim1_elapsed_us = 0U;
    __HAL_TIM_SET_COUNTER(&htim1, 0U);
    __HAL_TIM_CLEAR_FLAG(&htim1, TIM_FLAG_UPDATE);

    if (HAL_TIM_Base_Start_IT(&htim1) != HAL_OK)
    {
        return 1U;
    }

    s_tim1_started = 1U;

    return 0U;
}

/**
 * @brief  interface timer init
 * @return status code
 *         - 0 success
 *         - 1 init failed
 * @note   none
 */
uint8_t ir_remote_interface_timer_init(void)
{
    s_tim1_started = 0U;

    return ir_remote_interface_tim1_start();
}

/**
 * @brief     interface timestamp read
 * @param[in] *t pointer to an ir_remote_time structure
 * @return    status code
 *            - 0 success
 *            - 1 read failed
 * @note      none
 */
uint8_t ir_remote_interface_timestamp_read(ir_remote_time_t *t)
{
    if (t == NULL)
    {
        return 1;
    }

    if (ir_remote_interface_tim1_start() != 0U)
    {
        return 1;
    }

    uint32_t primask = __get_PRIMASK();
    __disable_irq();

    uint64_t base_us = s_tim1_elapsed_us;
    uint32_t counter = __HAL_TIM_GET_COUNTER(&htim1);

    if (__HAL_TIM_GET_FLAG(&htim1, TIM_FLAG_UPDATE) != RESET)
    {
        base_us += IR_REMOTE_TIM1_PERIOD_US;
        counter = __HAL_TIM_GET_COUNTER(&htim1);
    }

    if (primask == 0U)
    {
        __enable_irq();
    }

    uint64_t total_us = base_us + (uint64_t)counter;
    t->s = total_us / 1000000ULL;
    t->us = (uint32_t)(total_us % 1000000ULL);

    return 0;
}

/**
 * @brief     interface delay ms
 * @param[in] ms time
 * @note      none
 */
void ir_remote_interface_delay_ms(uint32_t ms)
{
    HAL_Delay(ms);
}

/**
 * @brief     interface print format data
 * @param[in] fmt format data
 * @note      none
 */
void ir_remote_interface_debug_print(const char *const fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    (void)vprintf(fmt, args);
    va_end(args);
}

/**
 * @brief     interface receive callback
 * @param[in] *data pointer to an ir_remote_t structure
 * @note      none
 */
void ir_remote_interface_receive_callback(ir_remote_t *data)
{
    switch (data->status)
    {
        case IR_REMOTE_STATUS_OK :
        {
            ir_remote_interface_debug_print("ir_remote: irq ok.\n");
            ir_remote_interface_debug_print("ir_remote: add is 0x%02X and cmd is 0x%02X.\n", data->address, data->command);
            
            break;
        }
        case IR_REMOTE_STATUS_REPEAT :
        {
            ir_remote_interface_debug_print("ir_remote: irq repeat.\n");
            ir_remote_interface_debug_print("ir_remote: add is 0x%02X and cmd is 0x%02X.\n", data->address, data->command);
            
            break;
        }
        case IR_REMOTE_STATUS_ADDR_ERR :
        {
            ir_remote_interface_debug_print("ir_remote: irq addr error.\n");
            
            break;
        }
        case IR_REMOTE_STATUS_CMD_ERR :
        {
            ir_remote_interface_debug_print("ir_remote: irq cmd error.\n");
            
            break;
        }
        case IR_REMOTE_STATUS_FRAME_INVALID :
        {
            ir_remote_interface_debug_print("ir_remote: irq frame invalid.\n");
            
            break;
        }
        default :
        {
            ir_remote_interface_debug_print("ir_remote: irq unknown status.\n");
            
            break;
        }
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM1)
    {
        s_tim1_elapsed_us += IR_REMOTE_TIM1_PERIOD_US;
    }
}
```

</details>

<details>
<summary>bsp/irda/driver_ir_remote_interface.h</summary>

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
 * @file      driver_ir_remote_interface.h
 * @brief     driver ir_remote interface header file
 * @version   1.0.0
 * @author    Shifeng Li
 * @date      2023-03-31
 *
 * <h3>history</h3>
 * <table>
 * <tr><th>Date        <th>Version  <th>Author      <th>Description
 * <tr><td>2023/03/31  <td>1.0      <td>Shifeng Li  <td>first upload
 * </table>
 */

#ifndef DRIVER_IR_REMOTE_INTERFACE_H
#define DRIVER_IR_REMOTE_INTERFACE_H

#include "driver_ir_remote.h"

#ifdef __cplusplus
extern "C"{
#endif

/**
 * @defgroup ir_remote_interface_driver ir_remote interface driver function
 * @brief    ir_remote interface driver modules
 * @ingroup  ir_remote_driver
 * @{
 */

/**
 * @brief  interface timer init
 * @return status code
 *         - 0 success
 *         - 1 init failed
 * @note   none
 */
uint8_t ir_remote_interface_timer_init(void);

/**
 * @brief     interface timestamp read
 * @param[in] *t pointer to an ir_remote_time structure
 * @return    status code
 *            - 0 success
 *            - 1 read failed
 * @note      none
 */
uint8_t ir_remote_interface_timestamp_read(ir_remote_time_t *t);

/**
 * @brief     interface delay ms
 * @param[in] ms time
 * @note      none
 */
void ir_remote_interface_delay_ms(uint32_t ms);

/**
 * @brief     interface print format data
 * @param[in] fmt format data
 * @note      none
 */
void ir_remote_interface_debug_print(const char *const fmt, ...);

/**
 * @brief     interface receive callback
 * @param[in] *data pointer to an ir_remote_t structure
 * @note      none
 */
void ir_remote_interface_receive_callback(ir_remote_t *data);

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
<summary>bsp/irda/driver_ir_remote_interface_template.c</summary>

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
 * @file      driver_ir_remote_interface_template.c
 * @brief     driver ir_remote interface template source file
 * @version   1.0.0
 * @author    Shifeng Li
 * @date      2023-03-31
 *
 * <h3>history</h3>
 * <table>
 * <tr><th>Date        <th>Version  <th>Author      <th>Description
 * <tr><td>2023/03/31  <td>1.0      <td>Shifeng Li  <td>first upload
 * </table>
 */

#include "driver_ir_remote_interface.h"

/**
 * @brief     interface timestamp read
 * @param[in] *t pointer to an ir_remote_time structure
 * @return    status code
 *            - 0 success
 *            - 1 read failed
 * @note      none
 */
uint8_t ir_remote_interface_timestamp_read(ir_remote_time_t *t)
{
    return 0;
}

/**
 * @brief     interface delay ms
 * @param[in] ms time
 * @note      none
 */
void ir_remote_interface_delay_ms(uint32_t ms)
{

}

/**
 * @brief     interface print format data
 * @param[in] fmt format data
 * @note      none
 */
void ir_remote_interface_debug_print(const char *const fmt, ...)
{

}

/**
 * @brief     interface receive callback
 * @param[in] *data pointer to an ir_remote_t structure
 * @note      none
 */
void ir_remote_interface_receive_callback(ir_remote_t *data)
{
    switch (data->status)
    {
        case IR_REMOTE_STATUS_OK :
        {
            ir_remote_interface_debug_print("ir_remote: irq ok.\n");
            ir_remote_interface_debug_print("ir_remote: add is 0x%02X and cmd is 0x%02X.\n", data->address, data->command);
            
            break;
        }
        case IR_REMOTE_STATUS_REPEAT :
        {
            ir_remote_interface_debug_print("ir_remote: irq repeat.\n");
            ir_remote_interface_debug_print("ir_remote: add is 0x%02X and cmd is 0x%02X.\n", data->address, data->command);
            
            break;
        }
        case IR_REMOTE_STATUS_ADDR_ERR :
        {
            ir_remote_interface_debug_print("ir_remote: irq addr error.\n");
            
            break;
        }
        case IR_REMOTE_STATUS_CMD_ERR :
        {
            ir_remote_interface_debug_print("ir_remote: irq cmd error.\n");
            
            break;
        }
        case IR_REMOTE_STATUS_FRAME_INVALID :
        {
            ir_remote_interface_debug_print("ir_remote: irq frame invalid.\n");
            
            break;
        }
        default :
        {
            ir_remote_interface_debug_print("ir_remote: irq unknown status.\n");
            
            break;
        }
    }
}
```

</details>

<details>
<summary>bsp/irda/driver_ir_remote_receive_test.c</summary>

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
 * @file      driver_ir_remote_receive_test.c
 * @brief     driver ir_remote receive test source file
 * @version   1.0.0
 * @author    Shifeng Li
 * @date      2023-03-31
 *
 * <h3>history</h3>
 * <table>
 * <tr><th>Date        <th>Version  <th>Author      <th>Description
 * <tr><td>2023/03/31  <td>1.0      <td>Shifeng Li  <td>first upload
 * </table>
 */

#include "driver_ir_remote_receive_test.h"

static ir_remote_handle_t gs_handle;        /**< ir_remote handle */
static volatile uint8_t gs_flag;            /**< flag */

/**
 * @brief  receive test irq
 * @return status code
 *         - 0 success
 *         - 1 run failed
 * @note   none
 */
uint8_t ir_remote_receive_test_irq_handler(void)
{
    if (ir_remote_irq_handler(&gs_handle) != 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/**
 * @brief     interface receive callback
 * @param[in] *data pointer to an ir_remote_t structure
 * @note      none
 */
static void a_receive_callback(ir_remote_t *data)
{
    switch (data->status)
    {
        case IR_REMOTE_STATUS_OK :
        {
            ir_remote_interface_debug_print("ir_remote: irq ok.\n");
            ir_remote_interface_debug_print("ir_remote: add is 0x%02X and cmd is 0x%02X.\n", data->address, data->command);
            gs_flag = 1;
            
            break;
        }
        case IR_REMOTE_STATUS_REPEAT :
        {
            ir_remote_interface_debug_print("ir_remote: irq repeat.\n");
            ir_remote_interface_debug_print("ir_remote: add is 0x%02X and cmd is 0x%02X.\n", data->address, data->command);
            
            break;
        }
        case IR_REMOTE_STATUS_ADDR_ERR :
        {
            ir_remote_interface_debug_print("ir_remote: irq addr error.\n");
            
            break;
        }
        case IR_REMOTE_STATUS_CMD_ERR :
        {
            ir_remote_interface_debug_print("ir_remote: irq cmd error.\n");
            
            break;
        }
        case IR_REMOTE_STATUS_FRAME_INVALID :
        {
            ir_remote_interface_debug_print("ir_remote: irq frame invalid.\n");
            
            break;
        }
        default :
        {
            ir_remote_interface_debug_print("ir_remote: irq unknown status.\n");
            
            break;
        }
    }
}

/**
 * @brief     receive test
 * @param[in] times test times
 * @return    status code
 *            - 0 success
 *            - 1 test failed
 * @note      none
 */
uint8_t ir_remote_receive_test(uint32_t times)
{
    uint8_t res;
    uint16_t timeout;
    uint32_t i;
    ir_remote_info_t info;
    
    /* link interface function */
    DRIVER_IR_REMOTE_LINK_INIT(&gs_handle, ir_remote_handle_t);
    DRIVER_IR_REMOTE_LINK_TIMESTAMP_READ(&gs_handle, ir_remote_interface_timestamp_read);
    DRIVER_IR_REMOTE_LINK_DELAY_MS(&gs_handle, ir_remote_interface_delay_ms);
    DRIVER_IR_REMOTE_LINK_DEBUG_PRINT(&gs_handle, ir_remote_interface_debug_print);
    DRIVER_IR_REMOTE_LINK_RECEIVE_CALLBACK(&gs_handle, a_receive_callback);
    
    /* get information */
    res = ir_remote_info(&info);
    if (res != 0)
    {
        ir_remote_interface_debug_print("ir_remote: get info failed.\n");
       
        return 1;
    }
    else
    {
        /* print chip info */
        ir_remote_interface_debug_print("ir_remote: chip is %s.\n", info.chip_name);
        ir_remote_interface_debug_print("ir_remote: manufacturer is %s.\n", info.manufacturer_name);
        ir_remote_interface_debug_print("ir_remote: interface is %s.\n", info.interface);
        ir_remote_interface_debug_print("ir_remote: driver version is %d.%d.\n", info.driver_version / 1000, (info.driver_version % 1000) / 100);
        ir_remote_interface_debug_print("ir_remote: min supply voltage is %0.1fV.\n", info.supply_voltage_min_v);
        ir_remote_interface_debug_print("ir_remote: max supply voltage is %0.1fV.\n", info.supply_voltage_max_v);
        ir_remote_interface_debug_print("ir_remote: max current is %0.2fmA.\n", info.max_current_ma);
        ir_remote_interface_debug_print("ir_remote: max temperature is %0.1fC.\n", info.temperature_max);
        ir_remote_interface_debug_print("ir_remote: min temperature is %0.1fC.\n", info.temperature_min);
    }
    
    /* init */
    res = ir_remote_init(&gs_handle);
    if (res != 0)
    {
        ir_remote_interface_debug_print("ir_remote: init failed.\n");
       
        return 1;
    }
    
    /* start receive test */
    ir_remote_interface_debug_print("ir_remote: start receive test.\n");
    
    /* loop */
    for (i = 0; i < times; i++)
    {
        /* 5s timeout */
        timeout = 500;
        
        /* init 0 */
        gs_flag = 0;
        
        /* check timeout */
        while (timeout != 0)
        {
            /* check the flag */
            if (gs_flag != 0)
            {
                break;
            }
            
            /* timeout -- */
            timeout--;
            
            /* delay 10ms */
            ir_remote_interface_delay_ms(10);
        }
        
        /* check the timeout */
        if (timeout == 0)
        {
            /* receive timeout */
            ir_remote_interface_debug_print("ir_remote: receive timeout.\n");
            (void)ir_remote_deinit(&gs_handle);
                
            return 1;
        }
    }
    
    /* finish receive test */
    ir_remote_interface_debug_print("ir_remote: finish receive test.\n");
    (void)ir_remote_deinit(&gs_handle);
    
    return 0;
}
```

</details>

<details>
<summary>bsp/irda/driver_ir_remote_receive_test.h</summary>

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
 * @file      driver_ir_remote_receive_test.h
 * @brief     driver ir remote receive test header file
 * @version   1.0.0
 * @author    Shifeng Li
 * @date      2023-03-31
 *
 * <h3>history</h3>
 * <table>
 * <tr><th>Date        <th>Version  <th>Author      <th>Description
 * <tr><td>2023/03/31  <td>1.0      <td>Shifeng Li  <td>first upload
 * </table>
 */

#ifndef DRIVER_IR_REMOTE_RECEIVE_TEST_H
#define DRIVER_IR_REMOTE_RECEIVE_TEST_H

#include "driver_ir_remote_interface.h"

#ifdef __cplusplus
extern "C"{
#endif

/**
 * @defgroup ir_remote_test_driver ir_remote test driver function
 * @brief    ir_remote test driver modules
 * @ingroup  ir_remote_driver
 * @{
 */

/**
 * @brief  receive test irq
 * @return status code
 *         - 0 success
 *         - 1 run failed
 * @note   none
 */
uint8_t ir_remote_receive_test_irq_handler(void);

/**
 * @brief     receive test
 * @param[in] times test times
 * @return    status code
 *            - 0 success
 *            - 1 test failed
 * @note      none
 */
uint8_t ir_remote_receive_test(uint32_t times);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
```

</details>

<!-- BSP_DRIVERS_END -->
