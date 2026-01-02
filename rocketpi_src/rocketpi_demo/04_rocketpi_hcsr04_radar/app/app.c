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
