#include "relay.h"
#include "zephyr/device.h"
#include <sys/errno.h>
#include <zephyr/drivers/led.h>
#include <zephyr/devicetree.h>

#include "wifi.h"
#include "zephyr/kernel.h"
#include "zephyr/logging/log.h"
#include <haierctrl/drivers/heatpump.h>
#include <zephyr/shell/shell.h>
#include "temp.h"
#include "mqtt.h"
#include "zephyr/sleep.h"

LOG_MODULE_REGISTER(heatpump, CONFIG_LOG_DEFAULT_LEVEL);

static const struct device *heatpump = DEVICE_DT_GET(DT_ALIAS(heatpump));
static const struct led_dt_spec led_0 = LED_DT_SPEC_GET(DT_NODELABEL(status_led));

static void get_status(struct status_packet *packet) {
  float ti, to;
  heatpump_read_twi_two(heatpump, &ti, &to);
  struct heatpump_status stat;
  heatpump_read_status(heatpump, &stat);
  packet->ambient_temp = read_temp();
  packet->ch_temp = ti;
  packet->dhw_temp = heatpump_read_dhw_temp(heatpump);
  packet->ch_target_temp = heatpump_read_ch_temp(heatpump);
  packet->dhw_target_temp = heatpump_read_dhw_target_temp(heatpump);
  packet->opr_mode = heatpump_read_mode(heatpump);
  packet->tank_state = stat.has_tank;
  packet->heater_state = stat.heat_mode;
  packet->valve_state = heatpump_get_3way(heatpump);
}

int main() {
  relay_init();
  temp_init();
  wifi_init();
  wifi_connect(CONFIG_WIFI_CREDENTIALS_STATIC_SSID, CONFIG_WIFI_CREDENTIALS_STATIC_PASSWORD);
  wifi_wait_for_ip_addr();
  mqtt_init(BROKER_IP);
  while(1) {
    led_on_dt(&led_0);
    struct status_packet packet;
    get_status(&packet);
    publish_status(&packet);
    led_off_dt(&led_0);
    k_sleep(K_SECONDS(30));
  }
}


