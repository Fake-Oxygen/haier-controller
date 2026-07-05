#include "zephyr/kernel.h"
#include <zephyr/drivers/led.h>
#include <zephyr/devicetree.h>

static const struct led_dt_spec led_0 = LED_DT_SPEC_GET(DT_NODELABEL(status_led));

int main() {
  while (1) {
    led_on_dt(&led_0);
    k_msleep(1000);
    led_off_dt(&led_0);
    k_msleep(1000);
  }
}
