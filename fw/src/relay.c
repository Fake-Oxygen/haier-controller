#include <zephyr/logging/log.h>
#include "zephyr/drivers/gpio.h"

#include "relay.h"

LOG_MODULE_REGISTER(relays, CONFIG_LOG_DEFAULT_LEVEL);

static struct gpio_dt_spec relays[RELAY_COUNT] = {
  GPIO_DT_SPEC_GET(DT_ALIAS(heating), gpios),
  GPIO_DT_SPEC_GET(DT_ALIAS(cooling), gpios),
  GPIO_DT_SPEC_GET(DT_ALIAS(freq_lim), gpios),
};

void relay_init() {
  for(int i = 0; i < RELAY_COUNT; i++) {
    if(!gpio_is_ready_dt(&relays[i])) {
      LOG_ERR("Relay device %s is not ready!", relays[i].port->name);
      continue;
    }

    int err = gpio_pin_configure_dt(&relays[i], GPIO_OUTPUT_INACTIVE);
    if(err < 0) {
      LOG_ERR("Failed to configure pin %d (err: %d)", relays[i].pin, err);
      continue;
    }
  }
}

void set_relay_state(enum relay_id relay, bool enable) {
  gpio_pin_set_dt(&relays[relay], enable);
}
