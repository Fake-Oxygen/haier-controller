
#include "zephyr/net/mqtt.h"
#include "haierctrl/drivers/heatpump.h"
#include "mqtt.h"
#include "zcbor_common.h"
#include "zcbor_encode.h"
#include "zephyr/kernel.h"
#include "zephyr/logging/log.h"
#include "zephyr/net/net_ip.h"
#include <stddef.h>
#include <stdint.h>
#include <sys/errno.h>
#include <zephyr/posix/arpa/inet.h>

LOG_MODULE_REGISTER(mqtt, CONFIG_LOG_DEFAULT_LEVEL);

#define CLIENT_ID "zephyr_heatpump"

static struct mqtt_client client;
static struct sockaddr_in broker_addr;
static uint8_t rx_buf[128];
static uint8_t tx_buf[128];
static bool connected = false;

static void handler(struct mqtt_client *client, const struct mqtt_evt *evt) {
  switch (evt->type) {
  case MQTT_EVT_CONNACK:
    if (evt->result == 0) {
      connected = true;
      LOG_INF("Successfully connected to MQTT Broker!");
    } else {
      LOG_ERR("MQTT Connect refused by broker: %d", evt->result);
    }
    break;

  case MQTT_EVT_DISCONNECT:
    connected = false;
    LOG_WRN("MQTT Disconnected: %d", evt->result);
    break;

  default:
    break;
  }
}

void mqtt_init(const char *broker_ip) {
  mqtt_client_init(&client);

  broker_addr.sin_family = AF_INET;
  broker_addr.sin_port = htons(1883);
  inet_pton(AF_INET, broker_ip, &broker_addr.sin_addr);

  client.broker = &broker_addr;
  client.evt_cb = handler;
  client.client_id.utf8 = (uint8_t *)CLIENT_ID;
  client.client_id.size = strlen(CLIENT_ID);
  client.protocol_version = MQTT_VERSION_3_1_1;
  client.rx_buf = rx_buf;
  client.rx_buf_size = sizeof(rx_buf);
  client.tx_buf = tx_buf;
  client.tx_buf_size = sizeof(tx_buf);

  int err = mqtt_connect(&client);
  if (err) {
      LOG_ERR("mqtt_connect failed: %d", err);
      return;
  }

  int timeout = 50; 
  while (!connected && timeout-- > 0) {
      mqtt_input(&client);
      k_sleep(K_MSEC(100));
  }

  if (!connected) {
      LOG_ERR("Timed out waiting for MQTT_EVT_CONNACK");
  }
}

static int publish_msg(const char *topic, const uint8_t *payload, size_t len) {
  if (!connected) {
      LOG_WRN("Cannot publish: MQTT socket is not connected!");
      mqtt_init(BROKER_IP);
      return -ENOTSOCK;
  }
  struct mqtt_publish_param param;

  param.message.topic.qos = MQTT_QOS_0_AT_MOST_ONCE;
  param.message.topic.topic.utf8 = (uint8_t *)topic;
  param.message.topic.topic.size = strlen(topic);
  param.message.payload.data = (uint8_t *)payload;
  param.message.payload.len = len;
  param.message_id = k_cycle_get_32();
  param.dup_flag = 0U;
  param.retain_flag = 0U;

  int err = mqtt_publish(&client, &param);
  if (err)
      LOG_ERR("MQTT publish error: %d", err);
  else
      LOG_INF("MQTT message published successfully");
  return err;
}

void publish_status(struct status_packet *packet) {
  uint8_t cbor_buf[64];
  zcbor_state_t state[4];

  zcbor_new_encode_state(state, 4, cbor_buf, sizeof(cbor_buf), 0);

  bool ok = zcbor_list_start_encode(state, 9);
  ok &= zcbor_float32_put(state, packet->ambient_temp); 
  ok &= zcbor_float32_put(state, packet->ch_temp); 
  ok &= zcbor_float32_put(state, packet->dhw_temp); 
  ok &= zcbor_float32_put(state, packet->ch_target_temp); 
  ok &= zcbor_float32_put(state, packet->dhw_target_temp); 
  ok &= zcbor_uint32_put(state, packet->opr_mode); 
  ok &= zcbor_uint32_put(state, packet->valve_state); 
  ok &= zcbor_uint32_put(state, packet->tank_state); 
  ok &= zcbor_uint32_put(state, packet->heater_state);
  ok &= zcbor_list_end_encode(state, 9);

  if (!ok) {
      LOG_ERR("Failed to encode CBOR packet!");
      return;
  }

  size_t payload_len = (size_t)(state->payload - cbor_buf);
  publish_msg("heatpump/status", cbor_buf, payload_len);
}
