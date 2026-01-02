#include <errno.h>
#include <stdlib.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/net_config.h>
#include <zephyr/net/net_event.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/net_ip.h>
#include <zephyr/net/wifi_mgmt.h>

#include "config.h"

static struct net_mgmt_event_callback wifi_cb;
static struct net_mgmt_event_callback ipv4_cb;

static K_SEM_DEFINE(wifi_connected, 0, 1);
static K_SEM_DEFINE(ipv4_address_obtained, 0, 1);

static void handle_wifi_connect_result(struct net_mgmt_event_callback *cb)
{
    const struct wifi_status *status = (const struct wifi_status *)cb->info;

    if (status->status)
    {
        printk("Connection request failed status (%i)\n", status->status);
    }
    else
    {
        printk("Connected\n");
        k_sem_give(&wifi_connected);
    }
}

static void handle_wifi_disconnect_result(struct net_mgmt_event_callback *cb)
{
    const struct wifi_status *status = (const struct wifi_status *)cb->info;

    if (status->status)
    {
        printk("Disconnection request (%d)\n", status->status);
    }
    else
    {
        printk("Disconnected\n");
        k_sem_take(&wifi_connected, K_NO_WAIT);
    }
}

static void handle_ipv4_result(struct net_if *iface)
{
    ARG_UNUSED(iface);
    k_sem_give(&ipv4_address_obtained);
    printk("IP Address obtained...\n");
}

static void wifi_mgmt_event_handler(struct net_mgmt_event_callback *cb, uint64_t mgmt_event, struct net_if *iface)
{
    switch (mgmt_event)
    {
    case NET_EVENT_WIFI_CONNECT_RESULT:
        handle_wifi_connect_result(cb);
        break;

    case NET_EVENT_WIFI_DISCONNECT_RESULT:
        handle_wifi_disconnect_result(cb);
        break;

    case NET_EVENT_IPV4_ADDR_ADD:
        handle_ipv4_result(iface);
        break;

    default:
        break;
    }
}

void wifi_init(void)
{
    printk("WiFi initializing...\n");
    net_mgmt_init_event_callback(&wifi_cb, wifi_mgmt_event_handler,
                                 NET_EVENT_WIFI_CONNECT_RESULT | NET_EVENT_WIFI_DISCONNECT_RESULT);
    net_mgmt_add_event_callback(&wifi_cb);

    net_mgmt_init_event_callback(&ipv4_cb, wifi_mgmt_event_handler, NET_EVENT_IPV4_ADDR_ADD);
    net_mgmt_add_event_callback(&ipv4_cb);
    printk("WiFi initialized...\n");
}

int wifi_connect(const char *ssid, const char *psk)
{
    struct net_if *iface = net_if_get_default();
    struct wifi_connect_req_params wifi_params = {0};

    wifi_params.ssid = (uint8_t *)ssid;
    wifi_params.psk = (uint8_t *)psk;
    wifi_params.ssid_length = strlen(ssid);
    wifi_params.psk_length = strlen(psk);
    wifi_params.channel = 0;
    wifi_params.security = WIFI_SECURITY_TYPE_PSK;
    wifi_params.band = WIFI_FREQ_BAND_2_4_GHZ;
    wifi_params.mfp = WIFI_MFP_OPTIONAL;
    wifi_params.timeout = 20;

    k_sem_reset(&wifi_connected);
    k_sem_reset(&ipv4_address_obtained);

    printk("Connecting to SSID: %s\n", wifi_params.ssid);
    if (net_mgmt(NET_REQUEST_WIFI_CONNECT, iface, &wifi_params, sizeof(struct wifi_connect_req_params)))
    {
        printk("WiFi Connection Request Failed\n");
        return -EIO;
    }

    k_sem_take(&wifi_connected, K_FOREVER);
    printk("WiFi link ready\n");

    return 0;
}

int wifi_disconnect(void)
{
    struct net_if *iface = net_if_get_default();

    if (net_mgmt(NET_REQUEST_WIFI_DISCONNECT, iface, NULL, 0))
    {
        printk("WiFi Disconnection Request Failed\n");
        return -EIO;
    }

    return 0;
}

int wifi_wait_for_ipv4(k_timeout_t timeout)
{
    if (k_sem_take(&ipv4_address_obtained, timeout))
    {
        printk("Waiting for IPv4 address timed out\n");
        return -ETIMEDOUT;
    }

    return 0;
}

static void log_ipv4_details(void)
{
    struct net_if *iface = net_if_get_default();

    if (iface == NULL)
    {
        printk("No default network interface\n");
        return;
    }

    struct in_addr *addr = net_if_ipv4_get_global_addr(iface, NET_ADDR_PREFERRED);
    if (addr == NULL)
    {
        printk("IPv4 address not available yet\n");
        return;
    }

    char hr_addr[NET_IPV4_ADDR_LEN];
    net_addr_ntop(AF_INET, addr, hr_addr, sizeof(hr_addr));
    printk("IPv4 address acquired: %s\n", hr_addr);
}

int run_main(void)
{
    int ret;

    wifi_init();
    wifi_connect(SSID, PSK);

    ret = net_config_init_app(NULL, "Initializing network");
    if (ret < 0)
    {
        printk("Network configuration failed (%d)\n", ret);
        return ret;
    }

    ret = wifi_wait_for_ipv4(K_FOREVER);
    if (ret < 0)
    {
        return ret;
    }

    log_ipv4_details();

    printk("Network ready, sleeping forever...");
    k_sleep(K_FOREVER);

    return 0;
}

int main(void)
{
    return run_main();
}
