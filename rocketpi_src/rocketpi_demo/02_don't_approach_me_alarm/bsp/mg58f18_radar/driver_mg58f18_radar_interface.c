/**
 * @file driver_mg58f18_radar_interface.c
 * @brief STM32F4 上的 UART/DMA 适配实现。
 */

#include "driver_mg58f18_radar_interface.h"

#include <stdbool.h>
#include <string.h>

#include "usart.h"
#include "gpio.h"

typedef struct
{
    UART_HandleTypeDef *uart;                         /**< 使用的串口句柄（默认 USART1） */
    volatile bool       initialised;                  /**< 是否已完成初始化 */
    volatile bool       tx_busy;                      /**< DMA 发送忙标志 */
    uint8_t             tx_buffer[MG58F18_RADAR_FRAME_SIZE]; /**< 发送缓冲，长度等于一帧 */
    uint8_t             rx_buffer[MG58F18_RADAR_RX_BUFFER_SIZE]; /**< 接收缓冲，供空闲中断使用 */
} mg58f18_radar_hal_t;

static mg58f18_radar_hal_t s_hal = {0};

/**
 * @brief 启动 DMA 空闲接收，必要时复位 HAL 状态。
 */
static bool mg58f18_radar_interface_start_rx(void)
{
    if (!s_hal.initialised || s_hal.uart == NULL)
    {
        return false;
    }

    HAL_StatusTypeDef status =
        HAL_UARTEx_ReceiveToIdle_DMA(s_hal.uart, s_hal.rx_buffer, sizeof(s_hal.rx_buffer));

    if (status == HAL_OK)
    {
        if (s_hal.uart->hdmarx != NULL)
        {
            __HAL_DMA_DISABLE_IT(s_hal.uart->hdmarx, DMA_IT_HT);
        }
        return true;
    }
    else if (status == HAL_BUSY)
    {
        HAL_UART_AbortReceive(s_hal.uart);
        status = HAL_UARTEx_ReceiveToIdle_DMA(s_hal.uart, s_hal.rx_buffer, sizeof(s_hal.rx_buffer));
        if (status == HAL_OK && s_hal.uart->hdmarx != NULL)
        {
            __HAL_DMA_DISABLE_IT(s_hal.uart->hdmarx, DMA_IT_HT);
        }
        return (status == HAL_OK);
    }

    return false;
}

mg58f18_radar_status_t mg58f18_radar_interface_hw_init(void)
{
    memset(&s_hal, 0, sizeof(s_hal));
    s_hal.uart = &huart1;

    if (s_hal.uart == NULL)
    {
        return MG58F18_RADAR_STATUS_HAL_ERROR;
    }

    s_hal.initialised = true;
    if (!mg58f18_radar_interface_start_rx())
    {
        s_hal.initialised = false;
        memset(&s_hal, 0, sizeof(s_hal));
        return MG58F18_RADAR_STATUS_HAL_ERROR;
    }
    return MG58F18_RADAR_STATUS_OK;
}

mg58f18_radar_status_t mg58f18_radar_interface_hw_send(const uint8_t *data,
                                                       size_t length,
                                                       uint32_t timeout_ms)
{
    if (!s_hal.initialised || s_hal.uart == NULL || data == NULL || length == 0U)
    {
        return MG58F18_RADAR_STATUS_NOT_INITIALISED;
    }

    if (length > sizeof(s_hal.tx_buffer))
    {
        return MG58F18_RADAR_STATUS_INVALID_ARGUMENT;
    }

    memcpy(s_hal.tx_buffer, data, length);

    s_hal.tx_busy = true;
    const HAL_StatusTypeDef hal_status =
        HAL_UART_Transmit_DMA(s_hal.uart, s_hal.tx_buffer, (uint16_t)length);
    if (hal_status == HAL_BUSY)
    {
        s_hal.tx_busy = false;
        return MG58F18_RADAR_STATUS_BUSY;
    }
    if (hal_status != HAL_OK)
    {
        s_hal.tx_busy = false;
        return MG58F18_RADAR_STATUS_HAL_ERROR;
    }

    const uint32_t start = HAL_GetTick();
    while (s_hal.tx_busy)
    {
        if ((HAL_GetTick() - start) > timeout_ms)
        {
            HAL_UART_AbortTransmit(s_hal.uart);
            s_hal.tx_busy = false;
            return MG58F18_RADAR_STATUS_TIMEOUT;
        }
    }

    return MG58F18_RADAR_STATUS_OK;
}

uint32_t mg58f18_radar_interface_hw_get_tick(void)
{
    return HAL_GetTick();
}

void mg58f18_radar_interface_hw_restart_rx(void)
{
    (void)mg58f18_radar_interface_start_rx();
}

/**
 * @brief 读取雷达 OUT 引脚逻辑电平。
 */
bool mg58f18_radar_interface_hw_read_io(void)
{
    const GPIO_PinState state = HAL_GPIO_ReadPin(RADAR_IO_GPIO_Port, RADAR_IO_Pin);
    return (state == GPIO_PIN_SET);
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (s_hal.initialised && huart == s_hal.uart)
    {
        s_hal.tx_busy = false;
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (!s_hal.initialised || huart != s_hal.uart)
    {
        return;
    }

    s_hal.tx_busy = false;
    mg58f18_radar_interface_start_rx();
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t size)
{
    if (!s_hal.initialised || huart != s_hal.uart || size == 0U)
    {
        return;
    }

    mg58f18_radar_receive_bytes(s_hal.rx_buffer, size);
    (void)mg58f18_radar_interface_start_rx();
}
