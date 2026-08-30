
#include "zephyr/net/mqtt.h"
#include "haierctrl/drivers/heatpump.h"
#include "mqtt.h"
#include "zcbor_common.h"
#include "zcbor_encode.h"
#include "zephyr/kernel.h"
#include "zephyr/logging/log.h"
#include "zephyr/net/net_ip.h"
#include "zephyr/net/socket.h"
#include "zephyr/net/socket_poll.h"
#include "zephyr/sleep.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <sys/errno.h>
#include <zephyr/posix/arpa/inet.h>
#include "haierctrl.h"

LOG_MODULE_REGISTER(mqtt, CONFIG_LOG_DEFAULT_LEVEL);
static const struct device *heatpump = DEVICE_DT_GET(DT_ALIAS(heatpump));

#define CLIENT_ID "zephyr_heatpump"

static struct mqtt_client client;
static struct sockaddr_in broker_addr;
static uint8_t rx_buf[128];
static uint8_t tx_buf[128];
static bool connected = false;

#define CMD_COUNT 4

typedef void (*handler_t)(char *val);
struct cmd {
  char *name;
  handler_t handler;
};

static void cmd_ch(char *val) {
  char *err;
  float temp = strtof(val, &err);
  heatpump_set_ch_temp(heatpump, temp);
}

static void cmd_dhw(char *val) {
  char *err;
  float temp = strtof(val, &err);
  heatpump_set_dhw_temp(heatpump, temp);
}

static void cmd_state(char *val) {
  char *err;
  int mode = strtol(val, &err, 0);
  heatpump_set_state(heatpump, mode);
}

static void cmd_mode(char *val) {
  char *err;
  int mode = strtol(val, &err, 0);
  heatpump_set_mode(heatpump, mode);
}

static struct cmd cmd_list[CMD_COUNT] = {
  {"ch", cmd_ch}, 
  {"dhw", cmd_dhw}, 
  {"state", cmd_state}, 
  {"mode", cmd_mode}
};

static int subscribe_to_topic(struct mqtt_client *client, const char *topic) {
  struct mqtt_topic sub_topic = {
    .topic = {
      .utf8 = (uint8_t *)topic,
      .size = strlen(topic)
    },
    .qos = MQTT_QOS_0_AT_MOST_ONCE
  };

  const struct mqtt_subscription_list sub_list = {
    .list = &sub_topic,
    .list_count = 1U,
    .message_id = k_cycle_get_32()
  };

  int err = mqtt_subscribe(client, &sub_list);
  if(err)
    LOG_ERR("Failed to subscribe to %s, err: %d", topic, err);
  else
    LOG_INF("Subscribed to topic: %s", topic);

  return err;
}

static void dispatch_cmd(char *data) {
  char *ptr = data;
  while(*ptr != ' ') {
    ptr++;
  }
  *ptr = '\0';
  ptr++;
  for(int i = 0; i < CMD_COUNT; i++) {
    if(!strcmp(data, cmd_list[i].name)) {
      LOG_INF("Command dispatched: %s with value: %s", data, ptr);
      cmd_list[i].handler(ptr);
      k_sleep(K_SECONDS(1));
      struct status_packet packet;
      get_status(&packet);
      publish_status(&packet);
      return;
    }
  }
}

static void handler(struct mqtt_client *client, const struct mqtt_evt *evt) {
  switch (evt->type) {
  case MQTT_EVT_PUBLISH: {
    const struct mqtt_publish_param *p = &evt->param.publish;
    size_t len = p->message.payload.len;
    if (len >= sizeof(rx_buf)) {
      LOG_ERR("Incoming payload too large!");
      return;
    }
    int err = mqtt_read_publish_payload(client, rx_buf, len);
    if (err < 0) {
      LOG_ERR("Failed to read payload: %d", err);
      return;
    }
    LOG_INF("Received command on topic: %s", p->message.topic.topic.utf8);
    rx_buf[len] = '\0';
    dispatch_cmd(rx_buf);
    LOG_INF("Payload: %s", rx_buf);
    break;
  }
  case MQTT_EVT_CONNACK:
    if (evt->result == 0) {
      connected = true;
      LOG_INF("Successfully connected to MQTT Broker!");
      subscribe_to_topic(client, "heatpump/cmd");
    } else {
      LOG_ERR("MQTT Connect refused by broker: %d", evt->result);
    }
    break;

  case MQTT_EVT_SUBACK:
    LOG_INF("Subscription acknowledged by broker (Message ID: %d)", evt->param.suback.message_id);
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
  client.keepalive = 180;

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

  bool ok = zcbor_list_start_encode(state, 10);
  ok &= zcbor_float32_put(state, packet->ambient_temp); 
  ok &= zcbor_float32_put(state, packet->ch_temp); 
  ok &= zcbor_float32_put(state, packet->dhw_temp); 
  ok &= zcbor_float32_put(state, packet->ch_target_temp); 
  ok &= zcbor_float32_put(state, packet->dhw_target_temp); 
  ok &= zcbor_uint32_put(state, packet->opr_mode); 
  ok &= zcbor_uint32_put(state, packet->valve_state); 
  ok &= zcbor_uint32_put(state, packet->tank_state); 
  ok &= zcbor_uint32_put(state, packet->heater_state);
  ok &= zcbor_uint32_put(state, packet->driver_state);
  ok &= zcbor_list_end_encode(state, 10);

  if (!ok) {
      LOG_ERR("Failed to encode CBOR packet!");
      return;
  }

  size_t payload_len = (size_t)(state->payload - cbor_buf);
  publish_msg("heatpump/status", cbor_buf, payload_len);
}

void mqtt_thread(void *p1, void *p2, void *p3) {
  
  struct zsock_pollfd fds[1];

  while(1) {
    if(!connected) {
      LOG_INF("Attempting MQTT connection...");
      mqtt_init(BROKER_IP);
      k_sleep(K_SECONDS(5));
      continue;
    }
    
    fds[0].fd = client.transport.tcp.sock;
    fds[0].events = ZSOCK_POLLIN;
    
    int res = zsock_poll(fds, 1, 1000);
    if(res < 0) {
      LOG_ERR("Poll failed, err: %d", errno);
      connected = false;
      mqtt_abort(&client);
      continue;
    }

    if(res > 0 && (fds[0].revents & ZSOCK_POLLIN)) {
      int err = mqtt_input(&client);
      if(err < 0 && err != -EAGAIN && err != -EWOULDBLOCK) {
        LOG_ERR("mqtt input fail: %d", err);
        connected = false;
        mqtt_abort(&client);
        continue;
      }
    }

    int live_err = mqtt_live(&client);
    if(live_err < 0 && live_err != -EAGAIN) {
      LOG_ERR("mqtt_live error: %d", live_err);
      connected = false;
      mqtt_abort(&client);
    }
  }
}

K_THREAD_DEFINE(mqtt_sub, 4096, mqtt_thread, NULL, NULL, NULL, 7, 0, 30 * 1000);
