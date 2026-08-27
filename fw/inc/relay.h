#ifndef HAIERCTRL_RELAY_H
#define HAIERCTRL_RELAY_H

#include <stdbool.h>

enum relay_id {
  RELAY_HEATING = 0,
  RELAY_COOLING,
  RELAY_FREQUENCY_LIMIT,

  RELAY_COUNT
};

void relay_init();
void set_relay_state(enum relay_id relay, bool enable);

#endif // !RELAY_H
