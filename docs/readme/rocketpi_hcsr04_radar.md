## 效果展示

![image-20251117211339600](https://cloud.rocketpi.club/cloud/image-20251117211339600.png)

![image-20251117211359067](https://cloud.rocketpi.club/cloud/image-20251117211359067.png)**

## 功能说明

- HCSR04雷达+SG90电机+串口  结合上位机，实现简单的毫米波雷达

## 硬件连接

- SG90  PC9
- TRIGGER ---PC10
- ECHO --- PC11

## CubeMX配置

### SG90配置

![image-20251117212649305](https://cloud.rocketpi.club/cloud/image-20251117212649305.png)

### HCSR04  IO配置

![image-20251117212930408](https://cloud.rocketpi.club/cloud/image-20251117212930408.png)

定时器1配置为1us计数

![image-20251117213131232](https://cloud.rocketpi.club/cloud/image-20251117213131232.png)

<!-- BSP_DRIVERS_START -->
<!-- BSP_DRIVERS_HASH:af097a8611f85172e8ffe707049a55573eada9a10991c223550441826bac63f6 -->
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
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "../app/app.h"
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
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_TIM1_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  App_Init();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    App_Update();
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
<summary>app/app.c</summary>

```c
#include "app.h"

#include "main.h"
#include "driver_sg90_test.h"
#include "driver_hcsr04_basic.h"
#include "ring_buffer.h"
#include "usart.h"

#include <math.h>
#include <stdint.h>

#define SERVO_MIN_ANGLE_DEG      (0.0f)    /* minimum servo angle in degrees */
#define SERVO_MAX_ANGLE_DEG      (180.0f)  /* maximum servo angle in degrees */
#define SERVO_STEP_DEG           (1.0f)    /* servo step increment in degrees */
#define SERVO_SETTLE_MS          (5U)      /* delay after moving servo (ms) */

#define REPORT_TIMEOUT_MS        (1U)      /* delay between measurement cycles (ms) */

#define PACKET_HEAD              (0xAAU)   /* packet header byte */
#define PACKET_STATUS_NO_TARGET  (0x00U)   /* indicates no echo detected */
#define PACKET_STATUS_TARGET     (0x01U)   /* indicates valid echo detected */
#define UART_REPORT_ENABLE       (1)       /* enable (1) or disable (0) UART reporting */

typedef enum
{
    SERVO_PHASE_MOVE = 0,
    SERVO_PHASE_MEASURE
} ServoPhase;

static uint8_t s_app_initialised;
static uint8_t s_sensor_ready;
static float s_current_angle_deg = SERVO_MIN_ANGLE_DEG;
static int8_t s_servo_direction = 1;
static ServoPhase s_phase = SERVO_PHASE_MOVE;
static uint32_t s_next_action_tick;

static float clampf(float value, float min_value, float max_value);
static uint8_t build_distance_byte(float distance_cm, uint8_t status);
static void send_packet(float angle_deg, uint8_t status, float distance_cm);
static void enqueue_frame(const uint8_t *data, uint8_t len);
static void start_tx_dma(void);



#define TX_BUFFER_CAPACITY 1024U
static RingBuffer s_tx_ring;
static uint8_t s_tx_storage[TX_BUFFER_CAPACITY];
static volatile uint8_t s_tx_busy;
static uint16_t s_tx_active_len;

void App_Init(void)
{
    if (s_app_initialised != 0U)
    {
        return;
    }

    SG90_Test_Init();
    (void)SG90_Test_SetAngle(SERVO_MIN_ANGLE_DEG);
    HAL_Delay(200U);

    if (hcsr04_basic_init() == 0U)
    {
        s_sensor_ready = 1U;
    }
    else
    {
        s_sensor_ready = 0U;
    }

    s_current_angle_deg = SERVO_MIN_ANGLE_DEG;
    s_servo_direction = 1;
    s_phase = SERVO_PHASE_MOVE;
    s_next_action_tick = HAL_GetTick();
    RingBuffer_Init(&s_tx_ring, s_tx_storage, sizeof(s_tx_storage));
    s_tx_busy = 0U;
    s_tx_active_len = 0U;
    s_app_initialised = 1U;
}

void App_Update(void)
{
    if (s_app_initialised == 0U)
    {
        return;
    }

    uint32_t now = HAL_GetTick();
    if ((int32_t)(now - s_next_action_tick) < 0)
    {
        return;
    }

    if (s_phase == SERVO_PHASE_MOVE)
    {
        (void)SG90_Test_SetAngle(s_current_angle_deg);
        s_phase = SERVO_PHASE_MEASURE;
        s_next_action_tick = now + SERVO_SETTLE_MS;
        return;
    }

    float distance_cm = -1.0f;
    uint8_t status = PACKET_STATUS_NO_TARGET;

    if (s_sensor_ready != 0U)
    {
        float distance_m = 0.0f;
        if (hcsr04_basic_read(&distance_m) == 0U)
        {
            distance_cm = distance_m * 100.0f;
            if ((distance_cm >= 0.0f) && (distance_cm <= 50.0f))
            {
                status = PACKET_STATUS_TARGET;
            }
            else
            {
                distance_cm = -1.0f;
            }
        }
    }

    send_packet(s_current_angle_deg, status, distance_cm);

    if (s_servo_direction > 0)
    {
        s_current_angle_deg += SERVO_STEP_DEG;
        if (s_current_angle_deg >= SERVO_MAX_ANGLE_DEG)
        {
            s_current_angle_deg = SERVO_MAX_ANGLE_DEG;
            s_servo_direction = -1;
        }
    }
    else
    {
        s_current_angle_deg -= SERVO_STEP_DEG;
        if (s_current_angle_deg <= SERVO_MIN_ANGLE_DEG)
        {
            s_current_angle_deg = SERVO_MIN_ANGLE_DEG;
            s_servo_direction = 1;
        }
    }

    s_phase = SERVO_PHASE_MOVE;
    s_next_action_tick = now + REPORT_TIMEOUT_MS;
}

void App_Run(void)
{
    App_Init();
    while (1)
    {
        App_Update();
    }
}

static float clampf(float value, float min_value, float max_value)
{
    if (value < min_value)
    {
        return min_value;
    }
    if (value > max_value)
    {
        return max_value;
    }
    return value;
}

static uint8_t build_distance_byte(float distance_cm, uint8_t status)
{
    if ((status == PACKET_STATUS_TARGET) && (distance_cm >= 0.0f))
    {
        float clamped = clampf(distance_cm, 0.0f, 255.0f);
        return (uint8_t)(clamped + 0.5f);
    }
    return 0xFFU;
}

static void enqueue_frame(const uint8_t *data, uint8_t len)
{
    while (RingBuffer_Space(&s_tx_ring) < len)
    {
        start_tx_dma();
        if (RingBuffer_Space(&s_tx_ring) < len)
        {
            HAL_Delay(1);
        }
    }

    __disable_irq();
    (void)RingBuffer_Write(&s_tx_ring, data, len);
    __enable_irq();

    start_tx_dma();
}

static void start_tx_dma(void)
{
    if (s_tx_busy != 0U)
    {
        return;
    }

    uint8_t *chunk;
    size_t chunk_len;

    __disable_irq();
    if (s_tx_busy != 0U)
    {
        __enable_irq();
        return;
    }

    chunk_len = RingBuffer_ReadLinear(&s_tx_ring, &chunk);
    if (chunk_len > 255U)
    {
        chunk_len = 255U;
    }

    if (chunk_len == 0U)
    {
        __enable_irq();
        return;
    }

    s_tx_busy = 1U;
    s_tx_active_len = (uint16_t)chunk_len;
    __enable_irq();

    if (HAL_UART_Transmit_DMA(&huart2, chunk, s_tx_active_len) != HAL_OK)
    {
        __disable_irq();
        s_tx_busy = 0U;
        s_tx_active_len = 0U;
        __enable_irq();
    }
}

static void send_packet(float angle_deg, uint8_t status, float distance_cm)
{
#if UART_REPORT_ENABLE
    uint8_t frame[5];
    frame[0] = PACKET_HEAD;
    frame[1] = (uint8_t)(clampf(angle_deg, 0.0f, 180.0f) + 0.5f);
    frame[2] = status;
    frame[3] = build_distance_byte(distance_cm, status);
    frame[4] = (uint8_t)((frame[0] + frame[1] + frame[2] + frame[3]) & 0xFFU);

    enqueue_frame(frame, sizeof(frame));
#else
    (void)angle_deg;
    (void)status;
    (void)distance_cm;
#endif
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == &huart2)
    {
        __disable_irq();
        RingBuffer_Advance(&s_tx_ring, s_tx_active_len);
        s_tx_busy = 0U;
        s_tx_active_len = 0U;
        __enable_irq();
        start_tx_dma();
    }
}
```

</details>

<details>
<summary>app/app.h</summary>

```c
#ifndef APP_H
#define APP_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialise the radar demo application.
 */
void App_Init(void);

/**
 * @brief Perform one radar update step.
 *
 * Call this from the main loop after App_Init() to keep the
 * servo, sensor, and display synchronised.
 */
void App_Update(void);

/**
 * @brief Convenience helper that calls App_Init() and then
 *        repeatedly executes App_Update() forever.
 */
void App_Run(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_H */
```

</details>

<details>
<summary>app/hcsr04_basic_port.c</summary>

```c
#include "driver_hcsr04_basic.h"

#include "driver_hcsr04_interface.h"

#include <stdint.h>

#if defined(__CC_ARM) || defined(__ARMCC_VERSION) || defined(__clang__) || defined(__GNUC__)
#define HCSR04_WEAK __attribute__((weak))
#else
#define HCSR04_WEAK
#endif

static hcsr04_handle_t s_hcsr04_handle;

HCSR04_WEAK uint8_t hcsr04_basic_init(void)
{
    uint8_t res;

    DRIVER_HCSR04_LINK_INIT(&s_hcsr04_handle, hcsr04_handle_t);
    DRIVER_HCSR04_LINK_TRIG_INIT(&s_hcsr04_handle, hcsr04_interface_trig_init);
    DRIVER_HCSR04_LINK_TRIG_DEINIT(&s_hcsr04_handle, hcsr04_interface_trig_deinit);
    DRIVER_HCSR04_LINK_TRIG_WRITE(&s_hcsr04_handle, hcsr04_interface_trig_write);
    DRIVER_HCSR04_LINK_ECHO_INIT(&s_hcsr04_handle, hcsr04_interface_echo_init);
    DRIVER_HCSR04_LINK_ECHO_DEINIT(&s_hcsr04_handle, hcsr04_interface_echo_deinit);
    DRIVER_HCSR04_LINK_ECHO_WRITE(&s_hcsr04_handle, hcsr04_interface_echo_read);
    DRIVER_HCSR04_LINK_TIMESTAMP_READ(&s_hcsr04_handle, hcsr04_interface_timestamp_read);
    DRIVER_HCSR04_LINK_DELAY_MS(&s_hcsr04_handle, hcsr04_interface_delay_ms);
    DRIVER_HCSR04_LINK_DELAY_US(&s_hcsr04_handle, hcsr04_interface_delay_us);
    DRIVER_HCSR04_LINK_DEBUG_PRINT(&s_hcsr04_handle, hcsr04_interface_debug_print);

    res = hcsr04_init(&s_hcsr04_handle);
    if (res != 0U)
    {
        hcsr04_interface_debug_print("hcsr04: init failed.\n");
        return 1U;
    }

    return 0U;
}

HCSR04_WEAK uint8_t hcsr04_basic_read(float *m)
{
    uint32_t time_us;

    if (m == NULL)
    {
        return 1U;
    }

    if (hcsr04_read(&s_hcsr04_handle, &time_us, m) != 0U)
    {
        return 1U;
    }

    return 0U;
}

HCSR04_WEAK uint8_t hcsr04_basic_deinit(void)
{
    if (hcsr04_deinit(&s_hcsr04_handle) != 0U)
    {
        return 1U;
    }

    return 0U;
}
```

</details>

<details>
<summary>app/ring_buffer.c</summary>

```c
#include "ring_buffer.h"

#include <string.h>

void RingBuffer_Init(RingBuffer *rb, uint8_t *storage, size_t capacity)
{
    rb->buffer = storage;
    rb->capacity = capacity;
    rb->head = 0U;
    rb->tail = 0U;
    rb->size = 0U;
}

size_t RingBuffer_Space(const RingBuffer *rb)
{
    if (rb->capacity < rb->size)
    {
        return 0U;
    }
    return rb->capacity - rb->size;
}

size_t RingBuffer_Size(const RingBuffer *rb)
{
    return rb->size;
}

size_t RingBuffer_Write(RingBuffer *rb, const uint8_t *data, size_t len)
{
    size_t written = 0U;

    while ((written < len) && (rb->size < rb->capacity))
    {
        size_t space_to_end = rb->capacity - rb->head;
        size_t chunk = len - written;

        if (chunk > space_to_end)
        {
            chunk = space_to_end;
        }

        size_t free_space = rb->capacity - rb->size;
        if (chunk > free_space)
        {
            chunk = free_space;
        }

        if (chunk == 0U)
        {
            break;
        }

        (void)memcpy(&rb->buffer[rb->head], &data[written], chunk);
        rb->head = (rb->head + chunk) % rb->capacity;
        rb->size += chunk;
        written += chunk;
    }

    return written;
}

size_t RingBuffer_ReadLinear(RingBuffer *rb, uint8_t **ptr)
{
    if (rb->size == 0U)
    {
        *ptr = NULL;
        return 0U;
    }

    *ptr = &rb->buffer[rb->tail];
    size_t contiguous = rb->capacity - rb->tail;

    if (contiguous > rb->size)
    {
        contiguous = rb->size;
    }

    return contiguous;
}

void RingBuffer_Advance(RingBuffer *rb, size_t len)
{
    if (len > rb->size)
    {
        len = rb->size;
    }

    rb->tail = (rb->tail + len) % rb->capacity;
    rb->size -= len;
}
```

</details>

<details>
<summary>app/ring_buffer.h</summary>

```c
#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stddef.h>
#include <stdint.h>

typedef struct
{
    uint8_t *buffer;
    size_t capacity;
    size_t head;
    size_t tail;
    size_t size;
} RingBuffer;

void RingBuffer_Init(RingBuffer *rb, uint8_t *storage, size_t capacity);
size_t RingBuffer_Space(const RingBuffer *rb);
size_t RingBuffer_Size(const RingBuffer *rb);
size_t RingBuffer_Write(RingBuffer *rb, const uint8_t *data, size_t len);
size_t RingBuffer_ReadLinear(RingBuffer *rb, uint8_t **ptr);
void RingBuffer_Advance(RingBuffer *rb, size_t len);

#endif /* RING_BUFFER_H */
```

</details>

<details>
<summary>bsp/hcsr04/driver_hcsr04.c</summary>

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
 * @file      driver_hcsr04.c
 * @brief     driver hcsr04 source file
 * @version   2.0.0
 * @author    Shifeng Li
 * @date      2021-04-15
 *
 * <h3>history</h3>
 * <table>
 * <tr><th>Date        <th>Version  <th>Author      <th>Description
 * <tr><td>2021/04/15  <td>2.0      <td>Shifeng Li  <td>format the code
 * <tr><td>2020/12/21  <td>1.0      <td>Shifeng Li  <td>first upload
 * </table>
 */

#include "driver_hcsr04.h"

/**
 * @brief chip information definition
 */
#define CHIP_NAME                 "JieShenna HCSR04"        /**< chip name */
#define MANUFACTURER_NAME         "JieShenna"               /**< manufacturer name */
#define SUPPLY_VOLTAGE_MIN        4.8f                      /**< chip min supply voltage */
#define SUPPLY_VOLTAGE_MAX        5.0f                      /**< chip max supply voltage */
#define MAX_CURRENT               15.0f                     /**< chip max current */
#define TEMPERATURE_MIN           -40.0f                    /**< chip min operating temperature */
#define TEMPERATURE_MAX           85.0f                     /**< chip max operating temperature */
#define DRIVER_VERSION            2000                      /**< driver version */

/**
 * @brief     initialize the chip
 * @param[in] *handle pointer to an hcsr04 handle structure
 * @return    status code
 *            - 0 success
 *            - 1 trig or echo init failed
 *            - 2 handle is NULL
 *            - 3 linked functions is NULL
 * @note      none
 */
uint8_t hcsr04_init(hcsr04_handle_t *handle)
{
    if (handle == NULL)                                                  /* check handle */
    {
        return 2;                                                        /* return error */
    }
    if (handle->debug_print == NULL)                                     /* check debug_print */
    {
        return 3;                                                        /* return error */
    }
    if (handle->trig_init == NULL)                                       /* check trig_init */
    {
        handle->debug_print("hcsr04: trig_init is null.\n");             /* trig_init is null */
       
        return 3;                                                        /* return error */
    }
    if (handle->trig_deinit == NULL)                                     /* check trig_deinit */
    {
        handle->debug_print("hcsr04: trig_deinit is null.\n");           /* trig_deinit is null */
       
        return 3;                                                        /* return error */
    }
    if (handle->trig_write == NULL)                                      /* check trig_write */
    {
        handle->debug_print("hcsr04: trig_write is null.\n");            /* trig_write is null */
       
        return 3;                                                        /* return error */
    }
    if (handle->echo_init == NULL)                                       /* check echo_init */
    {
        handle->debug_print("hcsr04: echo_init is null.\n");             /* echo_init is null */
       
        return 3;                                                        /* return error */
    }
    if (handle->echo_deinit == NULL)                                     /* check echo_deinit */
    {
        handle->debug_print("hcsr04: echo_deinit is null.\n");           /* echo_deinit is null */
       
        return 3;                                                        /* return error */
    }
    if (handle->echo_read == NULL)                                       /* check echo_read */
    {
        handle->debug_print("hcsr04: echo_read is null.\n");             /* echo_read is null */
       
        return 3;                                                        /* return error */
    }
    if (handle->delay_us == NULL)                                        /* check delay_us */
    {
        handle->debug_print("hcsr04: delay_us is null.\n");              /* delay_us is null */
       
        return 3;                                                        /* return error */
    }
    if (handle->delay_ms == NULL)                                        /* check delay_ms */
    {
        handle->debug_print("hcsr04: delay_ms is null.\n");              /* delay_ms is null */
       
        return 3;                                                        /* return error */
    }
    if (handle->timestamp_read == NULL)                                  /* check timestamp_read */
    {
        handle->debug_print("hcsr04: timestamp_read is null.\n");        /* timestamp_read is null */
       
        return 3;                                                        /* return error */
    }
    
    if (handle->trig_init() != 0)                                        /* initialize trig */
    {
        handle->debug_print("hcsr04: trig init failed.\n");              /* trig init failed */
       
        return 1;                                                        /* return error */
    }
    if (handle->echo_init() != 0)                                        /* initialize echo */
    {
        handle->debug_print("hcsr04: echo failed.\n");                   /* return error */
        (void)handle->trig_deinit();                                     /* deinit trig */
        
        return 1;                                                        /* return error */
    }
    handle->inited = 1;                                                  /* flag finish initialization */

    return 0;                                                            /* success return 0 */
}

/**
 * @brief     close the chip
 * @param[in] *handle pointer to an hcsr04 handle structure
 * @return    status code
 *            - 0 success
 *            - 1 trig or echo deinit failed
 *            - 2 handle is NULL
 *            - 3 handle is not initialized
 * @note      none
 */
uint8_t hcsr04_deinit(hcsr04_handle_t *handle)
{
    if (handle == NULL)                                              /* check handle */
    {
        return 2;                                                    /* return error */
    }
    if (handle->inited != 1)                                         /* check handle initialization */
    {
        return 3;                                                    /* return error */
    }
    
    if (handle->echo_deinit() != 0)                                  /* echo deinit */
    {
        handle->debug_print("hcsr04: echo deinit failed.\n");        /* echo deinit failed */
        
        return 1;                                                    /* return error */
    }   
    if (handle->trig_deinit() != 0)                                  /* trig deinit */
    {
        handle->debug_print("hcsr04: trig deinit failed.\n");        /* trig deinit failed */
        
        return 1;                                                    /* return error */
    } 
    handle->inited = 0;                                              /* flag close */
    
    return 0;                                                        /* success return 0 */
}

/**
 * @brief      read the distance
 * @param[in]  *handle pointer to an hcsr04 handle structure
 * @param[out] *time_us pointer to a us buffer
 * @param[out] *m pointer to a distance buffer
 * @return     status code
 *             - 0 success
 *             - 1 read failed
 *             - 2 handle is NULL
 *             - 3 handle is not initialized
 * @note       none
 */
uint8_t hcsr04_read(hcsr04_handle_t *handle, uint32_t *time_us, float *m)
{
    uint8_t res, value;
    hcsr04_time_t time_start;
    hcsr04_time_t time_stop;
    uint32_t timeout;
    uint8_t retry = HCSR04_READ_RETRY_TIMES;
    
    if (handle == NULL)                                                               /* check handle */
    {
        return 2;                                                                     /* return error */
    }
    if (handle->inited != 1)                                                          /* check handle initialization */
    {
        return 3;                                                                     /* return error */
    }
    
    while (1)                                                                         /* loop */
    {
        res = handle->trig_write(1);                                                  /* write trig 1 */
        if (res != 0)                                                                 /* check result */
        {
            handle->debug_print("hcsr04: trig write failed.\n");                      /* trig write failed */
           
            return 1;                                                                 /* return error */
        }
        handle->delay_us(20);                                                         /* delay 20 us */
        res = handle->trig_write(0);                                                  /* write trig 0 */
        if (res != 0)                                                                 /* check result */
        {
            handle->debug_print("hcsr04: trig write failed.\n");                      /* trig write failed */
           
            return 1;                                                                 /* return error */
        }
        value = 0;                                                                    /* reset value */
        timeout = 1000 * 5000;                                                        /* set timeout */
        while (value == 0)                                                            /* wait 5 s */
        {
            res = handle->echo_read((uint8_t *)&value);                               /* read echo data */
            if (res != 0)                                                             /* check result */
            {
                handle->debug_print("hcsr04: echo read failed.\n");                   /* echo read failed */
               
                return 1;                                                             /* return error */
            }
            handle->delay_us(1);                                                      /* delay 1 us */
            timeout--;                                                                /* timeout-- */
            if (timeout == 0)                                                         /* if timeout */
            {
                handle->debug_print("hcsr04: no response.\n");                        /* no response */
                
                return 1;                                                             /* return error */
            }
        }
        res = handle->timestamp_read((hcsr04_time_t *)&time_start);                   /* read timestamp */
        if (res != 0)                                                                 /* check result */
        {
            handle->debug_print("hcsr04: read timestamp failed.\n");                  /* read timestamp failed */
           
            return 1;                                                                 /* return error */
        }
        value = 1;                                                                    /* reset value */
        timeout = 1000 * 5000;                                                        /* wait 5 s */
        while (value != 0)                                                            /* check value */
        {
            res = handle->echo_read((uint8_t *)&value);                               /* read echo data */
            if (res != 0)                                                             /* check result */
            {
                handle->debug_print("hcsr04: echo read failed.\n");                   /* echo read failed */
               
                return 1;                                                             /* return error */
            }
            handle->delay_us(1);                                                      /* delay 1 us */
            timeout--;                                                                /* timeout-- */
            if (timeout == 0)                                                         /* if timeout */
            {
                handle->debug_print("hcsr04: no response.\n");                        /* no response */
               
                return 1;                                                             /* return error */
            }
        }
        res = handle->timestamp_read((hcsr04_time_t *)&time_stop);                    /* read timestamp */
        if (res != 0)                                                                 /* check result */
        {
            handle->debug_print("hcsr04: read timestamp failed.\n");                  /* read timestamp failed */
           
            return 1;                                                                 /* return error */
        }
        if (time_stop.millisecond < time_start.millisecond)                           /* check timestamp */
        {
            handle->debug_print("hcsr04: millisecond timestamp invalid.\n");          /* millisecond timestamp is invalid */
           
            return 1;                                                                 /* return error */
        }
        *time_us = (uint32_t)((int64_t)(((int64_t)time_stop.millisecond - 
                             (int64_t)time_start.millisecond) * 1000 + 
                             (int64_t)((int64_t)time_stop.microsecond - 
                             (int64_t)time_start.microsecond)));                      /* get time */
        if ((*time_us) > 150 * 1000)                                                  /* check time */
        {
            if (retry != 0)                                                           /* check remain retry times */
            {
                retry--;                                                              /* retry times-- */
                handle->delay_ms(150 + rand() % 100);                                 /* delay rand time */
            }
            else
            {
                handle->debug_print("hcsr04: no remain retry times.\n");              /* no remain retry times */
                
                return 1;                                                             /* return error */
            }
        }
        else
        {
            break;                                                                    /* successful */
        }
    }
    *m = 340.0f / 2.0f * (float)(*time_us) / 1000000.0f;                              /* calculate distance */

    return 0;                                                                         /* success return 0 */
}

/**
 * @brief      get chip's information
 * @param[out] *info pointer to an hcsr04 info structure
 * @return     status code
 *             - 0 success
 *             - 2 handle is NULL
 * @note       none
 */
uint8_t hcsr04_info(hcsr04_info_t *info)
{
    if (info == NULL)                                               /* check handle */
    {
        return 2;                                                   /* return error */
    }
    
    memset(info, 0, sizeof(hcsr04_info_t));                         /* initialize hcsr04 info structure */
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
<summary>bsp/hcsr04/driver_hcsr04.h</summary>

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
 * @file      driver_hcsr04.h
 * @brief     driver hcsr04 header file
 * @version   2.0.0
 * @author    Shifeng Li
 * @date      2021-04-15
 *
 * <h3>history</h3>
 * <table>
 * <tr><th>Date        <th>Version  <th>Author      <th>Description
 * <tr><td>2021/04/15  <td>2.0      <td>Shifeng Li  <td>format the code
 * <tr><td>2020/12/21  <td>1.0      <td>Shifeng Li  <td>first upload
 * </table>
 */

#ifndef DRIVER_HCSR04_H
#define DRIVER_HCSR04_H

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C"{
#endif

/**
 * @defgroup hcsr04_driver hcsr04 driver function
 * @brief    hcsr04 driver modules
 * @{
 */

/**
 * @addtogroup hcsr04_base_driver
 * @{
 */

/**
 * @brief hcsr04 read retry times definition
 */
#ifndef HCSR04_READ_RETRY_TIMES
    #define HCSR04_READ_RETRY_TIMES 3        /**< retry 3 times */
#endif

/**
 * @brief hcsr04 time structure definition
 */
typedef struct hcsr04_time_s
{
    uint64_t microsecond;        /**< microsecond */
    uint32_t millisecond;        /**< millisecond */
} hcsr04_time_t;

/**
 * @brief hcsr04 handle structure definition
 */
typedef struct hcsr04_handle_s
{
    uint8_t (*trig_init)(void);                            /**< point to a trig_init function address */
    uint8_t (*trig_deinit)(void);                          /**< point to a trig_deinit function address */
    uint8_t (*trig_write)(uint8_t value);                  /**< point to a trig_write function address */
    uint8_t (*echo_init)(void);                            /**< point to an echo_init function address */
    uint8_t (*echo_deinit)(void);                          /**< point to an echo_deinit function address */
    uint8_t (*echo_read)(uint8_t *value);                  /**< point to an echo_read function address */
    uint8_t (*timestamp_read)(hcsr04_time_t *time);        /**< point to a timestamp_read function address */
    void (*delay_us)(uint32_t ms);                         /**< point to a delay_us function address */
    void (*delay_ms)(uint32_t ms);                         /**< point to a delay_ms function address */
    void (*debug_print)(const char *const fmt, ...);       /**< point to a debug_print function address */
    uint8_t inited;                                        /**< inited flag */
} hcsr04_handle_t;

/**
 * @brief hcsr04 information structure definition
 */
typedef struct hcsr04_info_s
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
} hcsr04_info_t;

/**
 * @}
 */

/**
 * @defgroup hcsr04_link_driver hcsr04 link driver function
 * @brief    hcsr04 link driver modules
 * @ingroup  hcsr04_driver
 * @{
 */

/**
 * @brief     initialize hcsr04_handle_t structure
 * @param[in] HANDLE pointer to an hcsr04 handle structure
 * @param[in] STRUCTURE hcsr04_handle_t
 * @note      none
 */
#define DRIVER_HCSR04_LINK_INIT(HANDLE, STRUCTURE)      memset(HANDLE, 0, sizeof(STRUCTURE))

/**
 * @brief     link trig_init function
 * @param[in] HANDLE pointer to an hcsr04 handle structure
 * @param[in] FUC pointer to a trig_init function address
 * @note      none
 */
#define DRIVER_HCSR04_LINK_TRIG_INIT(HANDLE, FUC)      (HANDLE)->trig_init = FUC

/**
 * @brief     link trig_deinit function
 * @param[in] HANDLE pointer to an hcsr04 handle structure
 * @param[in] FUC pointer to a trig_deinit function address
 * @note      none
 */
#define DRIVER_HCSR04_LINK_TRIG_DEINIT(HANDLE, FUC)    (HANDLE)->trig_deinit = FUC

/**
 * @brief     link trig_write function
 * @param[in] HANDLE pointer to an hcsr04 handle structure
 * @param[in] FUC pointer to a trig_write function address
 * @note      none
 */
#define DRIVER_HCSR04_LINK_TRIG_WRITE(HANDLE, FUC)     (HANDLE)->trig_write = FUC

/**
 * @brief     link echo_init function
 * @param[in] HANDLE pointer to an hcsr04 handle structure
 * @param[in] FUC pointer to an echo_init function address
 * @note      none
 */
#define DRIVER_HCSR04_LINK_ECHO_INIT(HANDLE, FUC)      (HANDLE)->echo_init = FUC

/**
 * @brief     link echo_deinit function
 * @param[in] HANDLE pointer to an hcsr04 handle structure
 * @param[in] FUC pointer to an echo_deinit function address
 * @note      none
 */
#define DRIVER_HCSR04_LINK_ECHO_DEINIT(HANDLE, FUC)    (HANDLE)->echo_deinit = FUC

/**
 * @brief     link echo_read function
 * @param[in] HANDLE pointer to an hcsr04 handle structure
 * @param[in] FUC pointer to an echo_read function address
 * @note      none
 */
#define DRIVER_HCSR04_LINK_ECHO_WRITE(HANDLE, FUC)     (HANDLE)->echo_read = FUC

/**
 * @brief     link timestamp_read function
 * @param[in] HANDLE pointer to an hcsr04 handle structure
 * @param[in] FUC pointer to a timestamp_read function address
 * @note      none
 */
#define DRIVER_HCSR04_LINK_TIMESTAMP_READ(HANDLE, FUC) (HANDLE)->timestamp_read = FUC

/**
 * @brief     link delay_ms function
 * @param[in] HANDLE pointer to an hcsr04 handle structure
 * @param[in] FUC pointer to a delay_ms function address
 * @note      none
 */
#define DRIVER_HCSR04_LINK_DELAY_MS(HANDLE, FUC)       (HANDLE)->delay_ms = FUC

/**
 * @brief     link delay_us function
 * @param[in] HANDLE pointer to an hcsr04 handle structure
 * @param[in] FUC pointer to a delay_us function address
 * @note      none
 */
#define DRIVER_HCSR04_LINK_DELAY_US(HANDLE, FUC)       (HANDLE)->delay_us = FUC

/**
 * @brief     link debug_print function
 * @param[in] HANDLE pointer to an hcsr04 handle structure
 * @param[in] FUC pointer to a debug_print function address
 * @note      none
 */
#define DRIVER_HCSR04_LINK_DEBUG_PRINT(HANDLE, FUC)    (HANDLE)->debug_print = FUC

/**
 * @}
 */

/**
 * @defgroup hcsr04_base_driver hcsr04 base driver function
 * @brief    hcsr04 base driver modules
 * @ingroup  hcsr04_driver
 * @{
 */

/**
 * @brief      get chip's information
 * @param[out] *info pointer to an hcsr04 info structure
 * @return     status code
 *             - 0 success
 *             - 2 handle is NULL
 * @note       none
 */
uint8_t hcsr04_info(hcsr04_info_t *info);

/**
 * @brief     initialize the chip
 * @param[in] *handle pointer to an hcsr04 handle structure
 * @return    status code
 *            - 0 success
 *            - 1 trig or echo init failed
 *            - 2 handle is NULL
 *            - 3 linked functions is NULL
 * @note      none
 */
uint8_t hcsr04_init(hcsr04_handle_t *handle);

/**
 * @brief     close the chip
 * @param[in] *handle pointer to an hcsr04 handle structure
 * @return    status code
 *            - 0 success
 *            - 1 trig or echo deinit failed
 *            - 2 handle is NULL
 *            - 3 handle is not initialized
 * @note      none
 */
uint8_t hcsr04_deinit(hcsr04_handle_t *handle);

/**
 * @brief      read the distance
 * @param[in]  *handle pointer to an hcsr04 handle structure
 * @param[out] *time_us pointer to a us buffer
 * @param[out] *m pointer to a distance buffer
 * @return     status code
 *             - 0 success
 *             - 1 read failed
 *             - 2 handle is NULL
 *             - 3 handle is not initialized
 * @note       none
 */
uint8_t hcsr04_read(hcsr04_handle_t *handle, uint32_t *time_us, float *m);

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
<summary>bsp/hcsr04/driver_hcsr04_basic.c</summary>

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
 * @file      driver_hcsr04_basic.c
 * @brief     driver hcsr04 basic source file
 * @version   2.0.0
 * @author    Shifeng Li
 * @date      2021-04-15
 *
 * <h3>history</h3>
 * <table>
 * <tr><th>Date        <th>Version  <th>Author      <th>Description
 * <tr><td>2021/04/15  <td>2.0      <td>Shifeng Li  <td>format the code
 * <tr><td>2020/12/21  <td>1.0      <td>Shifeng Li  <td>first upload
 * </table>
 */

#include "driver_hcsr04_basic.h"

static hcsr04_handle_t gs_handle;        /**< hcsr04 handle */

/**
 * @brief  basic example init
 * @return status code
 *         - 0 success
 *         - 1 init failed
 * @note   none
 */
uint8_t hcsr04_basic_init(void)
{
    uint8_t res;
    
    /* link interface function */
    DRIVER_HCSR04_LINK_INIT(&gs_handle, hcsr04_handle_t);
    DRIVER_HCSR04_LINK_TRIG_INIT(&gs_handle, hcsr04_interface_trig_init);
    DRIVER_HCSR04_LINK_TRIG_DEINIT(&gs_handle, hcsr04_interface_trig_deinit);
    DRIVER_HCSR04_LINK_TRIG_WRITE(&gs_handle, hcsr04_interface_trig_write);
    DRIVER_HCSR04_LINK_ECHO_INIT(&gs_handle, hcsr04_interface_echo_init);
    DRIVER_HCSR04_LINK_ECHO_DEINIT(&gs_handle, hcsr04_interface_echo_deinit);
    DRIVER_HCSR04_LINK_ECHO_WRITE(&gs_handle, hcsr04_interface_echo_read);
    DRIVER_HCSR04_LINK_TIMESTAMP_READ(&gs_handle, hcsr04_interface_timestamp_read);
    DRIVER_HCSR04_LINK_DELAY_MS(&gs_handle, hcsr04_interface_delay_ms);
    DRIVER_HCSR04_LINK_DELAY_US(&gs_handle, hcsr04_interface_delay_us);
    DRIVER_HCSR04_LINK_DEBUG_PRINT(&gs_handle, hcsr04_interface_debug_print);
    
    /* hcsr04 init */
    res = hcsr04_init(&gs_handle);
    if (res != 0)
    {
        hcsr04_interface_debug_print("hcsr04: init failed.\n");
       
        return 1;
    }

    return 0;
}

/**
 * @brief      basic example read
 * @param[out] *m pointer to a distance buffer
 * @return     status code
 *             - 0 success
 *             - 1 read failed
 * @note       none
 */
uint8_t hcsr04_basic_read(float *m)
{
    uint32_t time_us;
    
    /* read distance */
    if (hcsr04_read(&gs_handle, (uint32_t *)&time_us, m) != 0)
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
uint8_t hcsr04_basic_deinit(void)
{
    /* close hcsr04 */
    if (hcsr04_deinit(&gs_handle) != 0)
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
<summary>bsp/hcsr04/driver_hcsr04_basic.h</summary>

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
 * @file      driver_hcsr04_basic.h
 * @brief     driver hcsr04 basic header file
 * @version   2.0.0
 * @author    Shifeng Li
 * @date      2021-04-15
 *
 * <h3>history</h3>
 * <table>
 * <tr><th>Date        <th>Version  <th>Author      <th>Description
 * <tr><td>2021/04/15  <td>2.0      <td>Shifeng Li  <td>format the code
 * <tr><td>2020/12/21  <td>1.0      <td>Shifeng Li  <td>first upload
 * </table>
 */

#ifndef DRIVER_HCSR04_BASIC_H
#define DRIVER_HCSR04_BASIC_H

#include "driver_hcsr04_interface.h"

#ifdef __cplusplus
extern "C"{
#endif

/**
 * @defgroup hcsr04_example_driver hcsr04 example driver function
 * @brief    hcsr04 example driver modules
 * @ingroup  hcsr04_driver
 * @{
 */

/**
 * @brief  basic example init
 * @return status code
 *         - 0 success
 *         - 1 init failed
 * @note   none
 */
uint8_t hcsr04_basic_init(void);

/**
 * @brief  basic example deinit
 * @return status code
 *         - 0 success
 *         - 1 deinit failed
 * @note   none
 */
uint8_t hcsr04_basic_deinit(void);

/**
 * @brief      basic example read
 * @param[out] *m pointer to a distance buffer
 * @return     status code
 *             - 0 success
 *             - 1 read failed
 * @note       none
 */
uint8_t hcsr04_basic_read(float *m);

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
<summary>bsp/hcsr04/driver_hcsr04_interface.c</summary>

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
 * @file      driver_hcsr04_interface_template.c
 * @brief     driver hcsr04 interface template source file
 * @version   2.0.0
 * @author    Shifeng Li
 * @date      2021-04-15
 *
 * <h3>history</h3>
 * <table>
 * <tr><th>Date        <th>Version  <th>Author      <th>Description
 * <tr><td>2021/04/15  <td>2.0      <td>Shifeng Li  <td>format the code
 * <tr><td>2020/12/21  <td>1.0      <td>Shifeng Li  <td>first upload
 * </table>
 */

#include "driver_hcsr04_interface.h"

#include "main.h"
#include "tim.h"

#include <stdarg.h>
#include <stdio.h>

/* TIM1 rolls over every 0x10000 ticks with 1 us resolution */
#define TIM1_AUTORELOAD_TICKS   (0x10000UL)

static volatile uint32_t s_tim1_overflow_count = 0;   /* number of completed TIM1 periods */
static uint8_t s_timer_started = 0;                   /* flag to avoid re-starting TIM1 */

/**
 * @brief  Start TIM1 in interrupt mode if it isn't running yet.
 * @return 0 on success, 1 on failure
 */
static uint8_t hcsr04_interface_start_timer_if_needed(void)
{
    if (s_timer_started != 0)
    {
        return 0;
    }

    if (htim1.Instance != TIM1)
    {
        hcsr04_interface_debug_print("hcsr04: TIM1 not initialized.\n");
        return 1;
    }

    /* make sure the IRQ is ready before the timer starts generating events */
    HAL_NVIC_SetPriority(TIM1_UP_TIM10_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(TIM1_UP_TIM10_IRQn);

    s_tim1_overflow_count = 0;
    __HAL_TIM_SET_COUNTER(&htim1, 0);
    if (HAL_TIM_Base_Start_IT(&htim1) != HAL_OK)
    {
        hcsr04_interface_debug_print("hcsr04: start TIM1 failed.\n");
        return 1;
    }

    s_timer_started = 1;

    return 0;
}

/**
 * @brief Get the current monotonic time in microseconds using TIM1.
 * @return 64-bit timestamp in microseconds
 */
static uint64_t hcsr04_interface_get_time_us(void)
{
    if (htim1.Instance != TIM1)
    {
        return 0ULL;
    }

    uint32_t overflow_before;
    uint32_t counter;

    do
    {
        overflow_before = s_tim1_overflow_count;
        counter = __HAL_TIM_GET_COUNTER(&htim1);
    } while (overflow_before != s_tim1_overflow_count);

    return ((uint64_t)overflow_before * TIM1_AUTORELOAD_TICKS) + counter;
}
/**
 * @brief  interface trig init
 * @return status code
 *         - 0 success
 *         - 1 trig init failed
 * @note   none
 */
uint8_t hcsr04_interface_trig_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOC_CLK_ENABLE();

    GPIO_InitStruct.Pin = TRIGGER_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(TRIGGER_GPIO_Port, &GPIO_InitStruct);

    HAL_GPIO_WritePin(TRIGGER_GPIO_Port, TRIGGER_Pin, GPIO_PIN_RESET);

    return 0;
}

/**
 * @brief  interface trig deinit
 * @return status code
 *         - 0 success
 *         - 1 trig deinit failed
 * @note   none
 */
uint8_t hcsr04_interface_trig_deinit(void)
{
    HAL_GPIO_DeInit(TRIGGER_GPIO_Port, TRIGGER_Pin);

    if (s_timer_started != 0)
    {
        if (HAL_TIM_Base_Stop_IT(&htim1) != HAL_OK)
        {
            return 1;
        }
        HAL_NVIC_DisableIRQ(TIM1_UP_TIM10_IRQn);
        s_timer_started = 0;
        s_tim1_overflow_count = 0;
    }

    return 0;
}

/**
 * @brief     interface trig write
 * @param[in] value written value
 * @return    status code
 *            - 0 success
 *            - 1 trig write failed
 * @note      none
 */
uint8_t hcsr04_interface_trig_write(uint8_t value)
{
    HAL_GPIO_WritePin(TRIGGER_GPIO_Port,
                      TRIGGER_Pin,
                      (value != 0U) ? GPIO_PIN_SET : GPIO_PIN_RESET);

    return 0;
}

/**
 * @brief  interface echo init
 * @return status code
 *         - 0 success
 *         - 1 echo init failed
 * @note   none
 */
uint8_t hcsr04_interface_echo_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOC_CLK_ENABLE();

    GPIO_InitStruct.Pin = ECHO_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(ECHO_GPIO_Port, &GPIO_InitStruct);

    return 0;
}

/**
 * @brief  interface echo deinit
 * @return status code
 *         - 0 success
 *         - 1 trig echo failed
 * @note   none
 */
uint8_t hcsr04_interface_echo_deinit(void)
{
    HAL_GPIO_DeInit(ECHO_GPIO_Port, ECHO_Pin);

    return 0;
}

/**
 * @brief      interface echo read
 * @param[out] *value pointer to a value buffer
 * @return     status code
 *             - 0 success
 *             - 1 echo read failed
 * @note       none
 */
uint8_t hcsr04_interface_echo_read(uint8_t *value)
{
    if (value == NULL)
    {
        return 1;
    }

    *value = (uint8_t)HAL_GPIO_ReadPin(ECHO_GPIO_Port, ECHO_Pin);

    return 0;
}

/**
 * @brief      interface timestamp read
 * @param[out] *t pointer to a time structure
 * @return     status code
 *             - 0 success
 *             - 1 timestamp read failed
 * @note       none
 */
uint8_t hcsr04_interface_timestamp_read(hcsr04_time_t *t)
{
    if (t == NULL)
    {
        return 1;
    }

    if (hcsr04_interface_start_timer_if_needed() != 0)
    {
        return 1;
    }

    uint64_t now_us = hcsr04_interface_get_time_us();

    t->millisecond = (uint32_t)(now_us / 1000ULL);
    t->microsecond = now_us % 1000ULL;

    return 0;
}

/**
 * @brief     interface delay us
 * @param[in] us time
 * @note      none
 */
void hcsr04_interface_delay_us(uint32_t us)
{
    if (us == 0U)
    {
        return;
    }

    if (hcsr04_interface_start_timer_if_needed() != 0)
    {
        return;
    }

    uint64_t start = hcsr04_interface_get_time_us();
    while ((hcsr04_interface_get_time_us() - start) < us)
    {
        /* busy wait to preserve microsecond precision */
    }

}

/**
 * @brief     interface delay ms
 * @param[in] ms time
 * @note      none
 */
void hcsr04_interface_delay_ms(uint32_t ms)
{
    HAL_Delay(ms);

}

/**
 * @brief  TIM period elapsed callback used to extend the 16-bit timer counter.
 * @param  htim pointer to the TIM handle
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM1)
    {
        s_tim1_overflow_count++;
    }
}

/**
 * @brief     interface print format data
 * @param[in] fmt format data
 * @note      none
 */

//void hcsr04_interface_debug_print(const char *const fmt, ...)
//{
//	 va_list args;
//	 va_start(args, fmt);
//   vprintf(fmt, args);
//   va_end(args);	
//}


void hcsr04_interface_debug_print(const char *const fmt, ...)
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
<summary>bsp/hcsr04/driver_hcsr04_interface.h</summary>

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
 * @file      driver_hcsr04_interface.h
 * @brief     driver hcsr04 interface header file
 * @version   2.0.0
 * @author    Shifeng Li
 * @date      2021-04-15
 *
 * <h3>history</h3>
 * <table>
 * <tr><th>Date        <th>Version  <th>Author      <th>Description
 * <tr><td>2021/04/15  <td>2.0      <td>Shifeng Li  <td>format the code
 * <tr><td>2020/12/21  <td>1.0      <td>Shifeng Li  <td>first upload
 * </table>
 */

#ifndef DRIVER_HCSR04_INTERFACE_H
#define DRIVER_HCSR04_INTERFACE_H

#include "driver_hcsr04.h"

#ifdef __cplusplus
extern "C"{
#endif

/**
 * @defgroup hcsr04_interface_driver hcsr04 interface driver function
 * @brief    hcsr04 interface driver modules
 * @ingroup  hcsr04_driver
 * @{
 */

/**
 * @brief  interface trig init
 * @return status code
 *         - 0 success
 *         - 1 trig init failed
 * @note   none
 */
uint8_t hcsr04_interface_trig_init(void);

/**
 * @brief  interface trig deinit
 * @return status code
 *         - 0 success
 *         - 1 trig deinit failed
 * @note   none
 */
uint8_t hcsr04_interface_trig_deinit(void);

/**
 * @brief     interface trig write
 * @param[in] value written value
 * @return    status code
 *            - 0 success
 *            - 1 trig write failed
 * @note      none
 */
uint8_t hcsr04_interface_trig_write(uint8_t value);

/**
 * @brief  interface echo init
 * @return status code
 *         - 0 success
 *         - 1 echo init failed
 * @note   none
 */
uint8_t hcsr04_interface_echo_init(void);

/**
 * @brief  interface echo deinit
 * @return status code
 *         - 0 success
 *         - 1 trig echo failed
 * @note   none
 */
uint8_t hcsr04_interface_echo_deinit(void);

/**
 * @brief      interface echo read
 * @param[out] *value pointer to a value buffer
 * @return     status code
 *             - 0 success
 *             - 1 echo read failed
 * @note       none
 */
uint8_t hcsr04_interface_echo_read(uint8_t *value);

/**
 * @brief      interface timestamp read
 * @param[out] *t pointer to a time structure
 * @return     status code
 *             - 0 success
 *             - 1 timestamp read failed
 * @note       none
 */
uint8_t hcsr04_interface_timestamp_read(hcsr04_time_t *t);

/**
 * @brief     interface delay us
 * @param[in] us time
 * @note      none
 */
void hcsr04_interface_delay_us(uint32_t us);

/**
 * @brief     interface delay ms
 * @param[in] ms time
 * @note      none
 */
void hcsr04_interface_delay_ms(uint32_t ms);

/**
 * @brief     interface print format data
 * @param[in] fmt format data
 * @note      none
 */
void hcsr04_interface_debug_print(const char *const fmt, ...);

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
<summary>bsp/hcsr04/driver_hcsr04_interface_template.c</summary>

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
 * @file      driver_hcsr04_interface_template.c
 * @brief     driver hcsr04 interface template source file
 * @version   2.0.0
 * @author    Shifeng Li
 * @date      2021-04-15
 *
 * <h3>history</h3>
 * <table>
 * <tr><th>Date        <th>Version  <th>Author      <th>Description
 * <tr><td>2021/04/15  <td>2.0      <td>Shifeng Li  <td>format the code
 * <tr><td>2020/12/21  <td>1.0      <td>Shifeng Li  <td>first upload
 * </table>
 */

#include "driver_hcsr04_interface.h"

/**
 * @brief  interface trig init
 * @return status code
 *         - 0 success
 *         - 1 trig init failed
 * @note   none
 */
uint8_t hcsr04_interface_trig_init(void)
{
    return 0;
}

/**
 * @brief  interface trig deinit
 * @return status code
 *         - 0 success
 *         - 1 trig deinit failed
 * @note   none
 */
uint8_t hcsr04_interface_trig_deinit(void)
{
    return 0;
}

/**
 * @brief     interface trig write
 * @param[in] value written value
 * @return    status code
 *            - 0 success
 *            - 1 trig write failed
 * @note      none
 */
uint8_t hcsr04_interface_trig_write(uint8_t value)
{
    return 0;
}

/**
 * @brief  interface echo init
 * @return status code
 *         - 0 success
 *         - 1 echo init failed
 * @note   none
 */
uint8_t hcsr04_interface_echo_init(void)
{
    return 0;
}

/**
 * @brief  interface echo deinit
 * @return status code
 *         - 0 success
 *         - 1 trig echo failed
 * @note   none
 */
uint8_t hcsr04_interface_echo_deinit(void)
{
    return 0;
}

/**
 * @brief      interface echo read
 * @param[out] *value pointer to a value buffer
 * @return     status code
 *             - 0 success
 *             - 1 echo read failed
 * @note       none
 */
uint8_t hcsr04_interface_echo_read(uint8_t *value)
{
    return 0;
}

/**
 * @brief      interface timestamp read
 * @param[out] *t pointer to a time structure
 * @return     status code
 *             - 0 success
 *             - 1 timestamp read failed
 * @note       none
 */
uint8_t hcsr04_interface_timestamp_read(hcsr04_time_t *t)
{
    return 0;
}

/**
 * @brief     interface delay us
 * @param[in] us time
 * @note      none
 */
void hcsr04_interface_delay_us(uint32_t us)
{

}

/**
 * @brief     interface delay ms
 * @param[in] ms time
 * @note      none
 */
void hcsr04_interface_delay_ms(uint32_t ms)
{

}

/**
 * @brief     interface print format data
 * @param[in] fmt format data
 * @note      none
 */
void hcsr04_interface_debug_print(const char *const fmt, ...)
{
    
}
```

</details>

<details>
<summary>bsp/hcsr04/driver_hcsr04_read_test.c</summary>

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
 * @file      driver_hcsr04_read_test.c
 * @brief     driver hcsr04 read test source file
 * @version   2.0.0
 * @author    Shifeng Li
 * @date      2021-04-15
 *
 * <h3>history</h3>
 * <table>
 * <tr><th>Date        <th>Version  <th>Author      <th>Description
 * <tr><td>2021/04/15  <td>2.0      <td>Shifeng Li  <td>format the code
 * <tr><td>2020/12/21  <td>1.0      <td>Shifeng Li  <td>first upload
 * </table>
 */

#include "driver_hcsr04_read_test.h"

static hcsr04_handle_t gs_handle;        /**< hcsr04 handle */

/**
 * @brief     read test
 * @param[in] times test times
 * @return    status code
 *            - 0 success
 *            - 1 test failed
 * @note      none
 */
uint8_t hcsr04_read_test(uint32_t times)
{
    uint8_t res;
    uint32_t i;
    hcsr04_info_t info;
    
    /* link interface function */
    DRIVER_HCSR04_LINK_INIT(&gs_handle, hcsr04_handle_t);
    DRIVER_HCSR04_LINK_TRIG_INIT(&gs_handle, hcsr04_interface_trig_init);
    DRIVER_HCSR04_LINK_TRIG_DEINIT(&gs_handle, hcsr04_interface_trig_deinit);
    DRIVER_HCSR04_LINK_TRIG_WRITE(&gs_handle, hcsr04_interface_trig_write);
    DRIVER_HCSR04_LINK_ECHO_INIT(&gs_handle, hcsr04_interface_echo_init);
    DRIVER_HCSR04_LINK_ECHO_DEINIT(&gs_handle, hcsr04_interface_echo_deinit);
    DRIVER_HCSR04_LINK_ECHO_WRITE(&gs_handle, hcsr04_interface_echo_read);
    DRIVER_HCSR04_LINK_TIMESTAMP_READ(&gs_handle, hcsr04_interface_timestamp_read);
    DRIVER_HCSR04_LINK_DELAY_MS(&gs_handle, hcsr04_interface_delay_ms);
    DRIVER_HCSR04_LINK_DELAY_US(&gs_handle, hcsr04_interface_delay_us);
    DRIVER_HCSR04_LINK_DEBUG_PRINT(&gs_handle, hcsr04_interface_debug_print);
    
    /* get info */
    res = hcsr04_info(&info);
    if (res != 0)
    {
        hcsr04_interface_debug_print("hcsr04: get info failed.\n");
       
        return 1;
    }
    else
    {
        /* print chip information */
        hcsr04_interface_debug_print("hcsr04: chip is %s.\n", info.chip_name);
        hcsr04_interface_debug_print("hcsr04: manufacturer is %s.\n", info.manufacturer_name);
        hcsr04_interface_debug_print("hcsr04: interface is %s.\n", info.interface);
        hcsr04_interface_debug_print("hcsr04: driver version is %d.%d.\n", info.driver_version / 1000, (info.driver_version % 1000) / 100);
        hcsr04_interface_debug_print("hcsr04: min supply voltage is %0.1fV.\n", info.supply_voltage_min_v);
        hcsr04_interface_debug_print("hcsr04: max supply voltage is %0.1fV.\n", info.supply_voltage_max_v);
        hcsr04_interface_debug_print("hcsr04: max current is %0.2fmA.\n", info.max_current_ma);
        hcsr04_interface_debug_print("hcsr04: max temperature is %0.1fC.\n", info.temperature_max);
        hcsr04_interface_debug_print("hcsr04: min temperature is %0.1fC.\n", info.temperature_min);
    }
    
    /* hscr04 init */
    res = hcsr04_init(&gs_handle);
    if (res != 0)
    {
        hcsr04_interface_debug_print("hcsr04: init failed.\n");
       
        return 1;
    }
    
    /* start read test */
    hcsr04_interface_debug_print("hcsr04: start read test.\n");
    for (i = 0; i < times; i++)
    {
        uint32_t time_us;
        float m;
        
        /* read distance */
        res = hcsr04_read(&gs_handle, (uint32_t *)&time_us, (float *)&m);
        if (res != 0)
        {
            hcsr04_interface_debug_print("hcsr04: read failed.\n");
            (void)hcsr04_deinit(&gs_handle);
            
            return 1;
        }
        m *= 100.0f;
        hcsr04_interface_debug_print("hcsr04: distance is %fcm.\n", m);
        hcsr04_interface_delay_ms(500);
    }
    
    /* finish read test */
    hcsr04_interface_debug_print("hcsr04: finish read test.\n");
    (void)hcsr04_deinit(&gs_handle);
    
    return 0;
}
```

</details>

<details>
<summary>bsp/hcsr04/driver_hcsr04_read_test.h</summary>

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
 * @file      driver_hcsr04_read_test.h
 * @brief     driver hcsr04 read test header file
 * @version   2.0.0
 * @author    Shifeng Li
 * @date      2021-04-15
 *
 * <h3>history</h3>
 * <table>
 * <tr><th>Date        <th>Version  <th>Author      <th>Description
 * <tr><td>2021/04/15  <td>2.0      <td>Shifeng Li  <td>format the code
 * <tr><td>2020/12/21  <td>1.0      <td>Shifeng Li  <td>first upload
 * </table>
 */

#ifndef DRIVER_HCSR04_READ_TEST_H
#define DRIVER_HCSR04_READ_TEST_H

#include <driver_hcsr04_interface.h>

#ifdef __cplusplus
extern "C"{
#endif

/**
 * @defgroup hcsr04_test_driver hcsr04 test driver function
 * @brief    hcsr04 test driver modules
 * @ingroup  hcsr04_driver
 * @{
 */

/**
 * @brief     read test
 * @param[in] times test times
 * @return    status code
 *            - 0 success
 *            - 1 test failed
 * @note      none
 */
uint8_t hcsr04_read_test(uint32_t times);

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
<summary>bsp/sg90/driver_sg90_test.c</summary>

```c
#include "driver_sg90_test.h"

#include "tim.h"
#include "usart.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

/**
 * TIM3 is configured for a 20 ms period with 1 µs resolution, so the CCR value
 * directly represents the pulse width in microseconds.
 */

static uint16_t clamp_pulse(uint16_t pulse_us)
{
    if (pulse_us < SG90_MIN_PULSE_US)
    {
        return SG90_MIN_PULSE_US;
    }
    if (pulse_us > SG90_MAX_PULSE_US)
    {
        return SG90_MAX_PULSE_US;
    }
    return pulse_us;
}

void SG90_Test_Init(void)
{
    /* Ensure PWM output is running before updating the duty cycle */
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);
    /* Move the horn to the mid position as a safe default */
    (void)SG90_Test_SetAngle(90.0f);
}

static char uart_log_buffer[2][32];
static uint8_t uart_active_buffer;

static void log_angle(float angle_deg)
{
    if (huart2.gState != HAL_UART_STATE_READY)
    {
        return;
    }

    uint8_t next_buffer = uart_active_buffer ^ 1U;
    int len = snprintf(uart_log_buffer[next_buffer], sizeof(uart_log_buffer[next_buffer]),
                       "SG90 angle: %.2f\r\n", angle_deg);
    if (len <= 0)
    {
        return;
    }

    if (HAL_UART_Transmit_DMA(&huart2,
                              (uint8_t *)uart_log_buffer[next_buffer],
                              (uint16_t)len) == HAL_OK)
    {
        uart_active_buffer = next_buffer;
    }
}

HAL_StatusTypeDef SG90_Test_SetPulse(uint16_t pulse_width_us)
{
    uint16_t clamped = clamp_pulse(pulse_width_us);

    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, clamped);

    return (clamped == pulse_width_us) ? HAL_OK : HAL_ERROR;
}

HAL_StatusTypeDef SG90_Test_SetAngle(float angle_deg)
{
    if (angle_deg != angle_deg)
    {
        return HAL_ERROR;
    }

    if (angle_deg < 0.0f)
    {
        angle_deg = 0.0f;
    }
    else if (angle_deg > 180.0f)
    {
        angle_deg = 180.0f;
    }

    float pulse = (float)SG90_MIN_PULSE_US +
                  (angle_deg / 180.0f) * (float)(SG90_MAX_PULSE_US - SG90_MIN_PULSE_US);

    return SG90_Test_SetPulse((uint16_t)(pulse + 0.5f));
}

void SG90_Test_Sweep(uint32_t dwell_ms)
{
    static const float positions[] = {0.0f, 45.0f, 90.0f, 135.0f, 180.0f};
    for (size_t i = 0; i < (sizeof(positions) / sizeof(positions[0])); ++i)
    {
        (void)SG90_Test_SetAngle(positions[i]);
        HAL_Delay(dwell_ms);
    }
}

void SG90_Test_Scan(uint32_t step_delay_ms)
{
    const uint32_t steps_per_degree = 4U; /* 0.25 degree increments */
    const uint32_t log_stride = steps_per_degree * 5U; /* log every 5 degrees */
    const uint32_t max_index = 180U * steps_per_degree;
    const float inv_steps = 1.0f / (float)steps_per_degree;
    uint32_t base_delay = step_delay_ms / steps_per_degree;
    uint32_t remainder = step_delay_ms % steps_per_degree;

    while (1)
    {
        /* Forward sweep 0 -> 180 */
        for (uint32_t idx = 0U; idx <= max_index; ++idx)
        {
            float angle = (float)idx * inv_steps;
            (void)SG90_Test_SetAngle(angle);
            if ((idx % log_stride) == 0U || idx == max_index)
            {
                log_angle(angle);
            }
            uint32_t delay_ms = base_delay + ((idx % steps_per_degree) < remainder ? 1U : 0U);
            HAL_Delay(delay_ms);
        }

        /* Reverse sweep 179.75 -> 0.25 to avoid pausing twice at the endpoints */
        for (uint32_t idx = max_index - 1U; idx > 0U; --idx)
        {
            float angle = (float)idx * inv_steps;
            (void)SG90_Test_SetAngle(angle);
            if ((idx % log_stride) == 0U)
            {
                log_angle(angle);
            }
            uint32_t delay_ms = base_delay + ((idx % steps_per_degree) < remainder ? 1U : 0U);
            HAL_Delay(delay_ms);
        }
    }
}
```

</details>

<details>
<summary>bsp/sg90/driver_sg90_test.h</summary>

```c
/**
 * @file driver_sg90_test.h
 * @brief Simple SG90 servo helper functions built on TIM3 CH4 (PC9).
 */

#ifndef DRIVER_SG90_TEST_H
#define DRIVER_SG90_TEST_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"

/**
 * SG90 pulse parameters in microseconds.
 * TIM3 is configured for 1 us ticks (84 MHz / 84 prescaler).
 */
#define SG90_MIN_PULSE_US   500U
#define SG90_MAX_PULSE_US  2500U
#define SG90_FRAME_PERIOD_US 20000U

/**
 * @brief Start TIM3 CH4 PWM output and park the servo at 90 degrees.
 */
void SG90_Test_Init(void);

/**
 * @brief Set the SG90 pulse width directly.
 * @param pulse_width_us Desired high time in microseconds, expected 500-2500.
 *        Values outside the range are clamped and return HAL_ERROR.
 */
HAL_StatusTypeDef SG90_Test_SetPulse(uint16_t pulse_width_us);

/**
 * @brief Position the SG90 by angle.
 * @param angle_deg Target angle in degrees, nominal range 0-180.
 *        Out-of-range values are saturated and report HAL_OK for the clamped setpoint.
 */
HAL_StatusTypeDef SG90_Test_SetAngle(float angle_deg);

/**
 * @brief Sweep through a set of demo angles.
 * @param dwell_ms Delay in milliseconds to hold each position; use >=100 for visible motion.
 */
void SG90_Test_Sweep(uint32_t dwell_ms);

/**
 * @brief Scan 0-180-0 degrees in fine (0.25°) steps while logging each angle over UART (DMA TX).
 *        This routine runs continuously until interrupted.
 *        Angle logs are throttled to once every ~5 degrees to keep motion smooth.
 * @param step_delay_ms Delay budget per degree; smaller values increase the sweep speed.
 *        Use at least 10 ms if the servo struggles to keep up.
 */
void SG90_Test_Scan(uint32_t step_delay_ms);

#ifdef __cplusplus
}
#endif

#endif /* DRIVER_SG90_TEST_H */
```

</details>

<!-- BSP_DRIVERS_END -->
