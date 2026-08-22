#include "zephyr/net/wifi.h"
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/net/wifi_mgmt.h>

// Event callbacks
static struct net_mgmt_event_callback wifi_cb;
static struct net_mgmt_event_callback ipv4_cb;
static struct net_mgmt_event_callback scan_cb;

// Semaphores
static K_SEM_DEFINE(sem_wifi, 0, 1);
static K_SEM_DEFINE(sem_ipv4, 0, 1);
static K_SEM_DEFINE(sem_wifi_scan, 0, 1);

static uint8_t target_channel = WIFI_CHANNEL_ANY;
static char target_ssid[32];

static void on_wifi_scan_result(struct net_mgmt_event_callback *cb,
                                uint64_t mgmt_event,
                                struct net_if *iface)
{
    if (mgmt_event == NET_EVENT_WIFI_SCAN_RESULT) {
        const struct wifi_scan_result *entry = (const struct wifi_scan_result *)cb->info;
        if (strlen(target_ssid) > 0 && strncmp(entry->ssid, target_ssid, entry->ssid_length) == 0) {
            target_channel = entry->channel;
            printk("Found %s on Channel %d!\r\n", target_ssid, target_channel);
        }
    } else if (mgmt_event == NET_EVENT_WIFI_SCAN_DONE) {
        k_sem_give(&sem_wifi_scan);
    }
}

static void on_wifi_connection_event(struct net_mgmt_event_callback *cb,
                                     uint64_t mgmt_event,
                                     struct net_if *iface)
{
    const struct wifi_status *status = (const struct wifi_status *)cb->info;

    if (mgmt_event == NET_EVENT_WIFI_CONNECT_RESULT) {
        if (status->status) {
            printk("Error (%d): Connection request failed\r\n", status->status);
        } else {
            printk("Connected!\r\n");
            k_sem_give(&sem_wifi);
        }
    } else if (mgmt_event == NET_EVENT_WIFI_DISCONNECT_RESULT) {
        if (status->status) {
            printk("Error (%d): Disconnection request failed\r\n", status->status);
        } else {
            printk("Disconnected\r\n");
            k_sem_take(&sem_wifi, K_NO_WAIT);
        }
    }
}

static void on_ipv4_obtained(struct net_mgmt_event_callback *cb,
                             uint64_t mgmt_event,
                             struct net_if *iface)
{
    if (mgmt_event == NET_EVENT_IPV4_ADDR_ADD) {
        k_sem_give(&sem_ipv4);
    }
}

void wifi_init(void)
{
    // MUST listen to both RESULT and DONE for scan completion
    net_mgmt_init_event_callback(&scan_cb, on_wifi_scan_result, 
                                  NET_EVENT_WIFI_SCAN_RESULT | NET_EVENT_WIFI_SCAN_DONE);
    net_mgmt_init_event_callback(&wifi_cb, on_wifi_connection_event,
                                  NET_EVENT_WIFI_CONNECT_RESULT | NET_EVENT_WIFI_DISCONNECT_RESULT);
    net_mgmt_init_event_callback(&ipv4_cb, on_ipv4_obtained, NET_EVENT_IPV4_ADDR_ADD);

    net_mgmt_add_event_callback(&scan_cb);
    net_mgmt_add_event_callback(&wifi_cb);
    net_mgmt_add_event_callback(&ipv4_cb);
}

int wifi_connect(char *ssid, char *psk)
{
    int ret;
    struct net_if *iface = net_if_get_default();
    
    // CRITICAL FIX 1: Zero out struct completely
    struct wifi_connect_req_params params = {0};

    // Store SSID for scan matching
    strncpy(target_ssid, ssid, sizeof(target_ssid) - 1);
    target_channel = WIFI_CHANNEL_ANY;

    printk("Scanning for APs...\r\n");
    ret = net_mgmt(NET_REQUEST_WIFI_SCAN, iface, NULL, 0);
    if (ret < 0) {
        printk("Failed to request Wi-Fi scan: %d\r\n", ret);
        return ret;
    }

    // Wait for scan done event
    k_sem_take(&sem_wifi_scan, K_FOREVER);
    
    // CRITICAL FIX 2: Give driver time to exit SCANNING state
    k_msleep(200); 

    printk("Scan finished. Connecting...\r\n");

    params.ssid = (const uint8_t *)ssid;
    params.ssid_length = strlen(ssid);
    params.psk = (const uint8_t *)psk;
    params.psk_length = strlen(psk);
    params.security = WIFI_SECURITY_TYPE_PSK;
    params.band = WIFI_FREQ_BAND_2_4_GHZ;
    
    // CRITICAL FIX 3: Use detected channel from scan (or fallback to ANY)
    params.channel = target_channel; 
    params.mfp = WIFI_MFP_DISABLE;
    params.timeout = SYS_FOREVER_MS;

    ret = net_mgmt(NET_REQUEST_WIFI_CONNECT, iface, &params, sizeof(params));
    if (ret != 0) {
        printk("Connection request rejected by driver: %d\r\n", ret);
        return ret;
    }

    k_sem_take(&sem_wifi, K_FOREVER);
    return 0;
}

void wifi_wait_for_ip_addr(void)
{
    struct wifi_iface_status status;
    struct net_if *iface = net_if_get_default();
    char ip_addr[NET_IPV4_ADDR_LEN];
    char gw_addr[NET_IPV4_ADDR_LEN];

    k_sem_take(&sem_ipv4, K_FOREVER);

    if (net_mgmt(NET_REQUEST_WIFI_IFACE_STATUS, iface, &status, sizeof(struct wifi_iface_status))) {
        printk("Error: WiFi status request failed\r\n");
    }

    memset(ip_addr, 0, sizeof(ip_addr));
    if (net_addr_ntop(AF_INET, &iface->config.ip.ipv4->unicast[0].ipv4.address.in_addr, ip_addr, sizeof(ip_addr)) == NULL) {
        printk("Error: Could not convert IP address to string\r\n");
    }

    memset(gw_addr, 0, sizeof(gw_addr));
    if (net_addr_ntop(AF_INET, &iface->config.ip.ipv4->gw, gw_addr, sizeof(gw_addr)) == NULL) {
        printk("Error: Could not convert gateway address to string\r\n");
    }

    printk("WiFi status:\r\n");
    if (status.state >= WIFI_STATE_ASSOCIATED) {
        printk("  SSID: %-32s\r\n", status.ssid);
        printk("  Band: %s\r\n", wifi_band_txt(status.band));
        printk("  Channel: %d\r\n", status.channel);
        printk("  Security: %s\r\n", wifi_security_txt(status.security));
        printk("  IP address: %s\r\n", ip_addr);
        printk("  Gateway: %s\r\n", gw_addr);
    }
}

int wifi_disconnect(void)
{
    struct net_if *iface = net_if_get_default();
    return net_mgmt(NET_REQUEST_WIFI_DISCONNECT, iface, NULL, 0);
}
