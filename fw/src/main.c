#include "zephyr/device.h"
#include "zephyr/kernel.h"
#include <zephyr/drivers/led.h>
#include <zephyr/devicetree.h>

#include "wifi.h"
#include "zephyr/logging/log.h"
#include "zephyr/sys/printk.h"
#include <haierctrl/drivers/heatpump.h>
#include <zephyr/shell/shell.h>

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
    k_msleep(1500);
    // LOG_INF("3 Way: %d", heatpump_get_3way(heatpump)); // 1 - tank / both, 2 - central
    // LOG_INF("CH temp: %f", (double)heatpump_read_ch_temp(heatpump)); // target ch temp
    // LOG_INF("DHW temp: %f", (double)heatpump_read_dhw_temp(heatpump)); // current water temp
    struct heatpump_status stat;
    heatpump_read_status(heatpump, &stat);
    // LOG_INF("Status: %d, Tank: %d, Mode: %d", stat.is_on, stat.has_tank, stat.heat_mode); // eco / turbo / quiet
    float ti, to;
    heatpump_read_twi_two(heatpump, &ti, &to);
    LOG_INF("TI: %f, TO: %f", (double)ti, (double)to);
    // LOG_INF("Pump state: %d", heatpump_read_pump_state(heatpump)); // ?
  }
}

static int cmd_set_ch(const struct shell *sh, size_t argc, char **argv) {
  return heatpump_set_ch_temp(heatpump, 33);
}

SHELL_CMD_REGISTER(set_ch, NULL, "sets heater temperature", cmd_set_ch);
