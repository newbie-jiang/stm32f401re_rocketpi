#pragma once

// #include <zephyr/kernel.h>

////////////////////////////////////////////////////////////////////////////////
/// WIFI CONFIG
#define SSID "ZTE-45c476"
#define PSK "88888888"
#define BACKEND_IP "10.128.162.229"
#define BACKEND_PORT 8081
#define HTTP_SERVER_PORT 80

int run_main(void);
void wifi_init(void);
int wifi_connect(const char *ssid, const char *psk);
int wifi_disconnect(void);
int wifi_wait_for_ipv4(k_timeout_t timeout);
