#ifndef HAIERCTRL_MQTT_H
#define HAIERCTRL_MQTT_H

#include "zephyr/net/mqtt.h"
#include <stddef.h>
#include <stdint.h>

#define BROKER_IP "192.168.10.184"

struct status_packet {
  float ambient_temp;
  float ch_temp;
  float dhw_temp;
  float ch_target_temp;
  float dhw_target_temp;
  uint8_t opr_mode;
  uint8_t valve_state;
  uint8_t tank_state;
  uint8_t heater_state;
  uint8_t driver_state;
};

void mqtt_init(const char *broker_ip);
void publish_status(struct status_packet *packet);

#endif // !HAIERCTRL_MQTT_H
