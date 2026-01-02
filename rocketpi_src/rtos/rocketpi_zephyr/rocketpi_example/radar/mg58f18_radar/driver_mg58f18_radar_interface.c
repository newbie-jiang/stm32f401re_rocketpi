/**
 * @file driver_mg58f18_radar_interface.c
 * @brief Zephyr 平台下的 UART/GPIO 适配层，实现 USART1(PA9/PA10) 收发。
 */

#include "driver_mg58f18_radar_interface.h"

#include <errno.h>
#include <string.h>

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(mg58f18_radar_if, LOG_LEVEL_INF);

#if DT_NODE_HAS_STATUS(DT_ALIAS(radaruart), okay)
#define RADAR_UART_NODE DT_ALIAS(radaruart)
#elif DT_NODE_HAS_STATUS(DT_NODELABEL(usart1), okay)
#define RADAR_UART_NODE DT_NODELABEL(usart1)
#else
#error "MG58F18 radar requires either alias 'radaruart' or an enabled usart1 node"
#endif

#define RADAR_UART_DEFAULT_BAUDRATE DT_PROP_OR(RADAR_UART_NODE, current_speed, 9600)

#if DT_NODE_EXISTS(DT_PATH(zephyr_user))
#define RADAR_USER_NODE DT_PATH(zephyr_user)
#endif

#if defined(RADAR_USER_NODE) && DT_NODE_HAS_PROP(RADAR_USER_NODE, radar_out_gpios)
#define RADAR_HAS_IO 1
static const struct gpio_dt_spec s_radar_io = GPIO_DT_SPEC_GET(RADAR_USER_NODE, radar_out_gpios);
#else
#define RADAR_HAS_IO 0
#endif

#define RADAR_RX_IDLE_TIMEOUT_US      5000U /* Flush short frames after a brief idle, similar to HAL ReceiveToIdle */
#define RADAR_RX_BUFFER_COUNT        2U
#define RADAR_RX_THREAD_STACK_SIZE 512
#define RADAR_RX_THREAD_PRIORITY    5

typedef struct
{
	const struct device *uart;
	struct k_sem         tx_lock;
	struct k_sem         tx_done;
	bool                 tx_async_enabled;
	bool                 tx_async_warning_printed;
	bool                 rx_async_warning_printed;
	bool                 rx_async_enabled;
	volatile bool        initialised;
	volatile bool        rx_active;
	volatile bool        rx_restart_pending;
	uint8_t              rx_buffers[RADAR_RX_BUFFER_COUNT][MG58F18_RADAR_RX_BUFFER_SIZE];
	uint8_t              rx_next_buffer;
#if RADAR_HAS_IO
	struct gpio_dt_spec io_pin;
	bool                io_ready;
#endif
} mg58f18_radar_hal_t;

static mg58f18_radar_hal_t s_hal;
static void mg58f18_radar_uart_rx_work_handler(struct k_work *work);
K_WORK_DELAYABLE_DEFINE(s_radar_rx_work, mg58f18_radar_uart_rx_work_handler);
static K_THREAD_STACK_DEFINE(s_radar_rx_stack, RADAR_RX_THREAD_STACK_SIZE);
static struct k_thread s_radar_rx_thread;
static bool s_radar_rx_thread_started;

static void mg58f18_radar_uart_tx_poll(const uint8_t *data, size_t length)
{
	for (size_t i = 0U; i < length; ++i) {
		uart_poll_out(s_hal.uart, data[i]);
	}
}

static bool mg58f18_radar_uart_async_error_is_fatal(int err)
{
	return (err == -ENOSYS) || (err == -ENOTSUP) || (err == -ENODEV);
}

static void mg58f18_radar_uart_poll_thread(void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	while (1) {
		if (!s_hal.initialised || s_hal.uart == NULL || s_hal.rx_async_enabled) {
			k_msleep(5);
			continue;
		}

		uint8_t byte;
		if (uart_poll_in(s_hal.uart, &byte) == 0) {
			mg58f18_radar_receive_bytes(&byte, 1U);
		} else {
			k_usleep(100);
		}
	}
}

static void mg58f18_radar_uart_start_poll_thread(void)
{
	if (s_radar_rx_thread_started) {
		return;
	}

	k_thread_create(&s_radar_rx_thread,
			s_radar_rx_stack,
			K_THREAD_STACK_SIZEOF(s_radar_rx_stack),
			mg58f18_radar_uart_poll_thread,
			NULL,
			NULL,
			NULL,
			RADAR_RX_THREAD_PRIORITY,
			0,
			K_NO_WAIT);
#if defined(CONFIG_THREAD_NAME)
	k_thread_name_set(&s_radar_rx_thread, "radar_uart_poll");
#endif
	s_radar_rx_thread_started = true;
}

static void mg58f18_radar_uart_switch_to_polling_rx(int err)
{
	if (!s_hal.rx_async_enabled) {
		return;
	}

	s_hal.rx_async_enabled      = false;
	s_hal.rx_active             = false;
	s_hal.rx_restart_pending    = false;
	(void)k_work_cancel_delayable(&s_radar_rx_work);

	/* Ensure the polling RX thread is running once async RX is unavailable. */
	mg58f18_radar_uart_start_poll_thread();

	/* Only emit a debug note once to keep logs clean on platforms without async UART. */
	if (!s_hal.rx_async_warning_printed) {
		LOG_DBG("UART async RX unavailable (err=%d), switching to polling mode", err);
		s_hal.rx_async_warning_printed = true;
	}
}

static void mg58f18_radar_uart_schedule_rx_start(k_timeout_t delay)
{
	if (!s_hal.rx_async_enabled) {
		return;
	}

	(void)k_work_schedule(&s_radar_rx_work, delay);
}

static void mg58f18_radar_uart_provide_next_buffer(const struct device *dev)
{
	const uint8_t index = s_hal.rx_next_buffer & 0x01U;
	s_hal.rx_next_buffer ^= 0x01U;
	(void)uart_rx_buf_rsp(dev, s_hal.rx_buffers[index], MG58F18_RADAR_RX_BUFFER_SIZE);
}

static void mg58f18_radar_uart_handle_rx_ready(const struct uart_event *evt)
{
	const uint8_t *data = &evt->data.rx.buf[evt->data.rx.offset];
	const size_t len    = evt->data.rx.len;
	if (len > 0U) {
		mg58f18_radar_receive_bytes(data, len);
	}
}

static void mg58f18_radar_uart_event_handler(const struct device *dev,
					     struct uart_event *evt,
					     void *user_data)
{
	ARG_UNUSED(user_data);

	switch (evt->type) {
	case UART_RX_RDY:
		if (s_hal.rx_async_enabled) {
			mg58f18_radar_uart_handle_rx_ready(evt);
		}
		break;
	case UART_RX_BUF_REQUEST:
		if (s_hal.rx_async_enabled) {
			mg58f18_radar_uart_provide_next_buffer(dev);
		}
		break;
	case UART_RX_BUF_RELEASED:
		break;
	case UART_RX_DISABLED:
		if (s_hal.rx_async_enabled) {
			s_hal.rx_active = false;
			if (s_hal.rx_restart_pending) {
				mg58f18_radar_uart_schedule_rx_start(K_NO_WAIT);
			}
		}
		break;
	case UART_RX_STOPPED:
		if (s_hal.rx_async_enabled) {
			s_hal.rx_active = false;
			s_hal.rx_restart_pending = true;
			mg58f18_radar_uart_schedule_rx_start(K_MSEC(1));
		}
		break;
	case UART_TX_DONE:
	case UART_TX_ABORTED:
		k_sem_give(&s_hal.tx_done);
		break;
	default:
		break;
	}
}

static void mg58f18_radar_uart_rx_work_handler(struct k_work *work)
{
	ARG_UNUSED(work);

	if (!s_hal.initialised || s_hal.uart == NULL || s_hal.rx_active || !s_hal.rx_async_enabled) {
		s_hal.rx_restart_pending = false;
		return;
	}

	s_hal.rx_next_buffer = 1U;
	const int err = uart_rx_enable(s_hal.uart,
				       s_hal.rx_buffers[0],
				       MG58F18_RADAR_RX_BUFFER_SIZE,
				       RADAR_RX_IDLE_TIMEOUT_US);
	if (err == 0) {
		s_hal.rx_active = true;
		s_hal.rx_restart_pending = false;
	} else if (mg58f18_radar_uart_async_error_is_fatal(err)) {
		mg58f18_radar_uart_switch_to_polling_rx(err);
	} else {
		/* Retry shortly if enabling failed (e.g. busy). */
		s_hal.rx_restart_pending = true;
		mg58f18_radar_uart_schedule_rx_start(K_MSEC(5));
	}
}

mg58f18_radar_status_t mg58f18_radar_interface_hw_init(void)
{
	memset(&s_hal, 0, sizeof(s_hal));
	(void)k_work_cancel_delayable(&s_radar_rx_work);

	s_hal.uart = DEVICE_DT_GET(RADAR_UART_NODE);
	if (!device_is_ready(s_hal.uart)) {
		return MG58F18_RADAR_STATUS_HAL_ERROR;
	}

	struct uart_config cfg = {
		.baudrate  = RADAR_UART_DEFAULT_BAUDRATE,
		.parity    = UART_CFG_PARITY_NONE,
		.stop_bits = UART_CFG_STOP_BITS_1,
		.data_bits = UART_CFG_DATA_BITS_8,
		.flow_ctrl = UART_CFG_FLOW_CTRL_NONE,
	};

	const int cfg_err = uart_configure(s_hal.uart, &cfg);
	if (cfg_err != 0) {
		return MG58F18_RADAR_STATUS_HAL_ERROR;
	}

	k_sem_init(&s_hal.tx_lock, 1, 1);
	k_sem_init(&s_hal.tx_done, 0, 1);
	s_hal.tx_async_enabled = true;
	s_hal.tx_async_warning_printed = false;
	s_hal.rx_async_enabled = true;
	s_hal.rx_async_warning_printed = false;

	const int cb_err = uart_callback_set(s_hal.uart, mg58f18_radar_uart_event_handler, NULL);
	if (cb_err != 0) {
		if (mg58f18_radar_uart_async_error_is_fatal(cb_err)) {
			mg58f18_radar_uart_switch_to_polling_rx(cb_err);
			s_hal.tx_async_enabled = false;
		} else {
			return MG58F18_RADAR_STATUS_HAL_ERROR;
		}
	}

#if RADAR_HAS_IO
	s_hal.io_pin = s_radar_io;
	if (device_is_ready(s_hal.io_pin.port)) {
		if (gpio_pin_configure_dt(&s_hal.io_pin, GPIO_INPUT) == 0) {
			s_hal.io_ready = true;
		}
	}
#endif

	s_hal.initialised = true;
	if (s_hal.rx_async_enabled) {
		s_hal.rx_restart_pending = true;
		mg58f18_radar_uart_schedule_rx_start(K_NO_WAIT);
	} else {
		mg58f18_radar_uart_start_poll_thread();
	}

	return MG58F18_RADAR_STATUS_OK;
}

mg58f18_radar_status_t mg58f18_radar_interface_hw_send(const uint8_t *data,
						       size_t length,
						       uint32_t timeout_ms)
{
	mg58f18_radar_status_t status = MG58F18_RADAR_STATUS_OK;

	if (!s_hal.initialised || s_hal.uart == NULL) {
		return MG58F18_RADAR_STATUS_NOT_INITIALISED;
	}

	if (data == NULL || length == 0U || length > MG58F18_RADAR_FRAME_SIZE) {
		return MG58F18_RADAR_STATUS_INVALID_ARGUMENT;
	}

	const k_timeout_t lock_timeout = (timeout_ms == 0U) ? K_NO_WAIT : K_MSEC(timeout_ms);

	if (k_sem_take(&s_hal.tx_lock, lock_timeout) != 0) {
		return MG58F18_RADAR_STATUS_BUSY;
	}

	if (!s_hal.tx_async_enabled) {
		mg58f18_radar_uart_tx_poll(data, length);
	} else {
		const k_timeout_t tx_timeout = (timeout_ms == 0U) ? K_FOREVER : K_MSEC(timeout_ms);

		while (k_sem_take(&s_hal.tx_done, K_NO_WAIT) == 0) {
			/* Drain stale completions. */
		}

		const int err = uart_tx(s_hal.uart, data, length, SYS_FOREVER_US);
		if (err == 0) {
			if (k_sem_take(&s_hal.tx_done, tx_timeout) != 0) {
				(void)uart_tx_abort(s_hal.uart);
				status = MG58F18_RADAR_STATUS_TIMEOUT;
			}
		} else {
			s_hal.tx_async_enabled = false;
			if (!s_hal.tx_async_warning_printed) {
				LOG_DBG("UART async TX unavailable (err=%d), switching to polling mode", err);
				s_hal.tx_async_warning_printed = true;
			}
			mg58f18_radar_uart_tx_poll(data, length);
		}
	}

	k_sem_give(&s_hal.tx_lock);
	return status;
}

uint32_t mg58f18_radar_interface_hw_get_tick(void)
{
	return (uint32_t)k_uptime_get_32();
}

void mg58f18_radar_interface_hw_restart_rx(void)
{
	if (!s_hal.initialised || s_hal.uart == NULL || !s_hal.rx_async_enabled) {
		return;
	}

	s_hal.rx_restart_pending = true;
	if (s_hal.rx_active) {
		(void)uart_rx_disable(s_hal.uart);
	} else {
		mg58f18_radar_uart_schedule_rx_start(K_NO_WAIT);
	}
}

bool mg58f18_radar_interface_hw_read_io(void)
{
#if RADAR_HAS_IO
	if (!s_hal.initialised || !s_hal.io_ready) {
		return false;
	}

	const int level = gpio_pin_get_dt(&s_hal.io_pin);
	return (level > 0);
#else
	/* 未在 DTS 中声明 radar_out_gpios 时默认返回低电平。 */
	return false;
#endif
}
