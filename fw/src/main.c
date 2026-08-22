#include "zephyr/kernel.h"
#include <zephyr/drivers/led.h>
#include <zephyr/devicetree.h>

#include "wifi.h"

static const struct led_dt_spec led_0 = LED_DT_SPEC_GET(DT_NODELABEL(status_led));

int main() {
  wifi_init();
  wifi_connect(CONFIG_WIFI_CREDENTIALS_STATIC_SSID, CONFIG_WIFI_CREDENTIALS_STATIC_PASSWORD);
  wifi_wait_for_ip_addr();
  while(1) {
    led_on_dt(&led_0);
    k_msleep(500);
    led_off_dt(&led_0);
    k_msleep(500);

  }
}
