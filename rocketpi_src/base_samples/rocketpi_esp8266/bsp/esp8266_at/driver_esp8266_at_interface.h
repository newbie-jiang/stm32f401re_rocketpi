/**
 * @file driver_esp8266_at_interface.h
 * @brief Hardware adaptation layer for the ESP8266 AT driver.
 */
#ifndef DRIVER_ESP8266_AT_INTERFACE_H
#define DRIVER_ESP8266_AT_INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

#include "driver_esp8266_at.h"

esp8266_at_status_t esp8266_at_interface_hw_init(void);
esp8266_at_status_t esp8266_at_interface_hw_send(const uint8_t *data,
                                                 size_t length,
                                                 uint32_t timeout_ms);
uint32_t esp8266_at_interface_hw_get_tick(void);
void esp8266_at_interface_hw_restart_rx(void);
void esp8266_at_interface_flush_trace(void);

#ifdef __cplusplus
}
#endif

#endif /* DRIVER_ESP8266_AT_INTERFACE_H */
