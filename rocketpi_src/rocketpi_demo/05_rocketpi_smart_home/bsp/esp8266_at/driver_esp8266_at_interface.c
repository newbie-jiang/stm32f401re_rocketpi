/**
 * @file driver_esp8266_at_interface.c
 * @brief STM32F4 UART/DMA implementation for the ESP8266 AT driver.
 */

#include "driver_esp8266_at_interface.h"

#include <stdbool.h>
#include <string.h>

#include "usart.h"

#define ESP8266_AT_INTERFACE_TX_BUFFER_SIZE 1024U
#define ESP8266_AT_INTERFACE_RX_BUFFER_SIZE 1024U

typedef struct
{
    UART_HandleTypeDef *uart;
    volatile bool       initialised;
    volatile bool       tx_busy;
    uint8_t             tx_buffer[ESP8266_AT_INTERFACE_TX_BUFFER_SIZE];
    uint8_t             rx_buffer[ESP8266_AT_INTERFACE_RX_BUFFER_SIZE];
} esp8266_at_hal_t;

static esp8266_at_hal_t s_hal = {0};

typedef struct
{
    volatile bool pending;
    uint16_t      length;
    uint8_t       buffer[ESP8266_AT_INTERFACE_RX_BUFFER_SIZE];
} esp8266_at_trace_t;

static esp8266_at_trace_t s_trace = {0};

static void esp8266_at_interface_trace_store(const uint8_t *data, uint16_t size)
{
    if (size > ESP8266_AT_INTERFACE_RX_BUFFER_SIZE)
    {
        size = ESP8266_AT_INTERFACE_RX_BUFFER_SIZE;
    }

    memcpy(s_trace.buffer, data, size);
    s_trace.length  = size;
    s_trace.pending = true;
}

static bool esp8266_at_interface_start_rx(void)
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

    if (status == HAL_BUSY)
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

esp8266_at_status_t esp8266_at_interface_hw_init(void)
{
    memset(&s_hal, 0, sizeof(s_hal));
    s_hal.uart = &huart6;

    if (s_hal.uart == NULL)
    {
        return ESP8266_AT_STATUS_HAL_ERROR;
    }

    s_hal.initialised = true;
    if (!esp8266_at_interface_start_rx())
    {
        s_hal.initialised = false;
        memset(&s_hal, 0, sizeof(s_hal));
        return ESP8266_AT_STATUS_HAL_ERROR;
    }

    return ESP8266_AT_STATUS_OK;
}

esp8266_at_status_t esp8266_at_interface_hw_send(const uint8_t *data,
                                                 size_t length,
                                                 uint32_t timeout_ms)
{
    if (!s_hal.initialised || s_hal.uart == NULL || data == NULL || length == 0U)
    {
        return ESP8266_AT_STATUS_NOT_INITIALISED;
    }

    if (length > sizeof(s_hal.tx_buffer))
    {
        return ESP8266_AT_STATUS_INVALID_ARGUMENT;
    }

    memcpy(s_hal.tx_buffer, data, length);

    s_hal.tx_busy = true;
    const HAL_StatusTypeDef status =
        HAL_UART_Transmit_DMA(s_hal.uart, s_hal.tx_buffer, (uint16_t)length);
    if (status == HAL_BUSY)
    {
        s_hal.tx_busy = false;
        return ESP8266_AT_STATUS_BUSY;
    }
    if (status != HAL_OK)
    {
        s_hal.tx_busy = false;
        return ESP8266_AT_STATUS_HAL_ERROR;
    }

    const uint32_t timeout = (timeout_ms != 0U) ? timeout_ms : ESP8266_AT_DEFAULT_TIMEOUT_MS;
    const uint32_t start   = HAL_GetTick();
    while (s_hal.tx_busy)
    {
        if ((HAL_GetTick() - start) > timeout)
        {
            HAL_UART_AbortTransmit(s_hal.uart);
            s_hal.tx_busy = false;
            return ESP8266_AT_STATUS_TIMEOUT;
        }
    }

    return ESP8266_AT_STATUS_OK;
}

uint32_t esp8266_at_interface_hw_get_tick(void)
{
    return HAL_GetTick();
}

void esp8266_at_interface_hw_restart_rx(void)
{
    (void)esp8266_at_interface_start_rx();
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
    esp8266_at_interface_start_rx();
}

void esp8266_at_interface_flush_trace(void)
{
    if (!s_trace.pending)
    {
        return;
    }

    __disable_irq();
    s_trace.pending = false;
    __enable_irq();
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t size)
{
    if (!s_hal.initialised || huart != s_hal.uart || size == 0U)
    {
        return;
    }

    esp8266_at_interface_trace_store(s_hal.rx_buffer, size);
    esp8266_at_receive_bytes(s_hal.rx_buffer, size);
    (void)esp8266_at_interface_start_rx();
}
