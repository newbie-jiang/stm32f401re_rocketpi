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
<!-- BSP_DRIVERS_HASH:d005a26c686862d9a1c1bd464763356c1d1e2bb25e2d81c1aaae38d5d3c0958a -->
## 驱动以及测试代码

<details>
<summary>Core/Src/stm32f4xx_it.c</summary>

```c
/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f4xx_it.c
  * @brief   Interrupt Service Routines.
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
#include "stm32f4xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "tim.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

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
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern TIM_HandleTypeDef htim1;
extern DMA_HandleTypeDef hdma_usart2_rx;
extern DMA_HandleTypeDef hdma_usart2_tx;
extern UART_HandleTypeDef huart2;
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M4 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
   while (1)
  {
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */

  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
  * @brief This function handles Pre-fetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */

  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */

  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
  /* USER CODE BEGIN SVCall_IRQn 0 */

  /* USER CODE END SVCall_IRQn 0 */
  /* USER CODE BEGIN SVCall_IRQn 1 */

  /* USER CODE END SVCall_IRQn 1 */
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
  /* USER CODE BEGIN PendSV_IRQn 0 */

  /* USER CODE END PendSV_IRQn 0 */
  /* USER CODE BEGIN PendSV_IRQn 1 */

  /* USER CODE END PendSV_IRQn 1 */
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  /* USER CODE BEGIN SysTick_IRQn 1 */

  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32F4xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f4xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles DMA1 stream5 global interrupt.
  */
void DMA1_Stream5_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Stream5_IRQn 0 */

  /* USER CODE END DMA1_Stream5_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_usart2_rx);
  /* USER CODE BEGIN DMA1_Stream5_IRQn 1 */

  /* USER CODE END DMA1_Stream5_IRQn 1 */
}

/**
  * @brief This function handles DMA1 stream6 global interrupt.
  */
void DMA1_Stream6_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Stream6_IRQn 0 */

  /* USER CODE END DMA1_Stream6_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_usart2_tx);
  /* USER CODE BEGIN DMA1_Stream6_IRQn 1 */

  /* USER CODE END DMA1_Stream6_IRQn 1 */
}

/**
  * @brief This function handles TIM1 update interrupt and TIM10 global interrupt.
  */
void TIM1_UP_TIM10_IRQHandler(void)
{
  /* USER CODE BEGIN TIM1_UP_TIM10_IRQn 0 */

  /* USER CODE END TIM1_UP_TIM10_IRQn 0 */
  HAL_TIM_IRQHandler(&htim1);
  /* USER CODE BEGIN TIM1_UP_TIM10_IRQn 1 */

  /* USER CODE END TIM1_UP_TIM10_IRQn 1 */
}

/**
  * @brief This function handles USART2 global interrupt.
  */
void USART2_IRQHandler(void)
{
  /* USER CODE BEGIN USART2_IRQn 0 */

  /* USER CODE END USART2_IRQn 0 */
  HAL_UART_IRQHandler(&huart2);
  /* USER CODE BEGIN USART2_IRQn 1 */

  /* USER CODE END USART2_IRQn 1 */
}

/* USER CODE BEGIN 1 */


/* USER CODE END 1 */
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

<!-- BSP_DRIVERS_END -->
