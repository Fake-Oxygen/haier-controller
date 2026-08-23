
#include "zephyr/device.h"
#include "zephyr/drivers/gpio.h"
#include "zephyr/drivers/uart.h"
#include "zephyr/kernel.h"
#include "zephyr/modbus/modbus.h"
#include "zephyr/sleep.h"
#include "zephyr/sys/clock.h"
#include "zephyr/sys/time_units.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/errno.h>
#include <zephyr/devicetree.h>
#include <zephyr/logging/log.h>
#include <zephyr/logging/log_core.h>

#include <haierctrl/drivers/heatpump.h>

#define DT_DRV_COMPAT haier_heatpump
LOG_MODULE_REGISTER(heatpump_driver, CONFIG_LOG_DEFAULT_LEVEL);

K_MUTEX_DEFINE(data_mutex);

static int write_pump(const struct device *dev) {
  const struct heatpump_config *cfg = dev->config;
  const struct heatpump_data *data = dev->data;
  if(data->modbus_client_iface < 0)
    return -EINVAL;

  uint16_t val = 120;
  gpio_pin_set_dt(&cfg->enable_gpio, 1);
  k_msleep(1000);
  int err = modbus_write_holding_regs(data->modbus_client_iface, cfg->slave_addr, 100, &val, 1);
  k_msleep(1000);
  gpio_pin_set_dt(&cfg->enable_gpio, 0);
  return err;
}

static bool uart_read_bytes(const struct device *dev, uint8_t *buf, size_t len, k_timeout_t timeout) {
  size_t read_count = 0;
  int64_t start_time = k_uptime_get();

  while(read_count < len) {
    uint8_t c;
    if(uart_poll_in(dev, &c) == 0) 
      buf[read_count++] = c;
    else 
      k_msleep(1);

    if(K_TIMEOUT_EQ(timeout, K_NO_WAIT) || (k_uptime_get() - start_time > k_ticks_to_ms_floor64(timeout.ticks)))
      break;
  }
  return (read_count == len);
}

static void sniffer(void *p1, void *p2, void *p3) {
  const struct heatpump_config *cfg = p1;
  if(!device_is_ready(cfg->uart)) {
    LOG_ERR("UART device not ready!");
    return;
  }

  LOG_INF("Modbus sniffer started...");

  while(1) {
    uint8_t addr;
    if(!uart_read_bytes(cfg->uart, &addr, 1, K_FOREVER))
      continue;

    if(addr != cfg->slave_addr)
      continue;

    uint8_t header[2];

    if(!uart_read_bytes(cfg->uart, header, 2, K_MSEC(50)))
      continue;

    if(header[0] != 0x03)
      continue;
    
    uint8_t payload[64];
    if(header[1] > sizeof(payload))
      continue;

    if(uart_read_bytes(cfg->uart, payload, header[1], K_MSEC(100))) {
      k_mutex_lock(&data_mutex, K_FOREVER);
      LOG_HEXDUMP_INF(header, 2, "Header");
      LOG_HEXDUMP_INF(payload, header[1], "Payload");
      k_mutex_unlock(&data_mutex);
    }
    
  }
}


static const struct heatpump_driver_api heatpump_api = {
  .get_state = NULL,
};

static int heatpump_init(const struct device *dev) {
  const struct heatpump_config *cfg = dev->config;
  struct heatpump_data *data = dev->data;

  data->modbus_client_iface = modbus_iface_get_by_name(cfg->modbus_iface_name);
  if(data->modbus_client_iface < 0) {
    LOG_ERR("Failed to find Modbus interface: %s", cfg->modbus_iface_name);
    return -ENODEV;
  }

  struct modbus_iface_param param = {
    .mode = MODBUS_MODE_RTU,
    .rx_timeout = 500000,
    .serial = {
      .baud = cfg->baud_rate,
      .parity = UART_CFG_PARITY_EVEN,
      .stop_bits = UART_CFG_STOP_BITS_1,
    }
  };

  int ret = modbus_init_client(data->modbus_client_iface, param);
  if(ret != 0) {
    LOG_ERR("Failed to init Modbus client for %s (err: %d)", dev->name, ret);
    return ret;
  }

  if (!gpio_is_ready_dt(&cfg->enable_gpio)) {
      LOG_ERR("GPIO device %s is not ready!", cfg->enable_gpio.port->name);
      return -ENODEV;
  }

  ret = gpio_pin_configure_dt(&cfg->enable_gpio, GPIO_OUTPUT_INACTIVE);
  if (ret < 0) {
      LOG_ERR("Failed to configure enable GPIO (err: %d)", ret);
      return ret;
  }

  write_pump(dev);
  LOG_INF("Heatpump driver initialized on bus %s", cfg->modbus_iface_name);
  return 0;
}

#define HEATPUMP_DEFINE(inst) \
  static const struct heatpump_config heatpump_cfg_##inst = {  \
    .uart = DEVICE_DT_GET(DT_NODELABEL(uart1))/*DT_PARENT(DT_PARENT(DT_DRV_INST(inst))))*/, \
    .modbus_iface_name = DT_NODE_FULL_NAME(DT_INST_PARENT(inst)), \
    .slave_addr = DT_INST_PROP(inst, slave_addr),  \
    .baud_rate = DT_PROP_OR(DT_INST_BUS(inst), current_speed, 9600), \
    .enable_gpio = GPIO_DT_SPEC_INST_GET(inst, enable_gpios), \
  };  \
  K_THREAD_DEFINE(sniffer_tid_##inst, 2048, sniffer, &heatpump_cfg_##inst, NULL, NULL, 5, 0, 0); \
  static struct heatpump_data heatpump_data_##inst; \
  DEVICE_DT_INST_DEFINE(inst, heatpump_init, NULL, &heatpump_data_##inst, &heatpump_cfg_##inst, POST_KERNEL, 90, &heatpump_api);

DT_INST_FOREACH_STATUS_OKAY(HEATPUMP_DEFINE);

