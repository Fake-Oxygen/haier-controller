#include "zephyr/device.h"
#include "zephyr/kernel.h"
#include <zephyr/drivers/led.h>
#include <zephyr/devicetree.h>

#include "wifi.h"
#include "zephyr/logging/log.h"
#include "zephyr/sys/printk.h"
#include <haierctrl/drivers/heatpump.h>

LOG_MODULE_REGISTER(heatpump, CONFIG_LOG_DEFAULT_LEVEL);

static const struct led_dt_spec led_0 = LED_DT_SPEC_GET(DT_NODELABEL(status_led));
static const struct device *heatpump = DEVICE_DT_GET(DT_ALIAS(heatpump));

int main() {
  wifi_init();
  wifi_connect(CONFIG_WIFI_CREDENTIALS_STATIC_SSID, CONFIG_WIFI_CREDENTIALS_STATIC_PASSWORD);
  wifi_wait_for_ip_addr();
  while(1) {
    led_on_dt(&led_0);
    k_msleep(500);
    led_off_dt(&led_0);
    k_msleep(500);
    LOG_INF("3 Way: %d", heatpump_get_3way(heatpump));
    LOG_INF("CH temp: %f", (double)heatpump_read_ch_temp(heatpump));
    LOG_INF("DHW temp: %f", (double)heatpump_read_dhw_temp(heatpump));
    LOG_INF("Heater state: %d", heatpump_read_heater_state(heatpump));
    LOG_INF("Mode: %d", heatpump_read_mode(heatpump));
    LOG_INF("Pump state: %d", heatpump_read_pump_state(heatpump));
  }
}
