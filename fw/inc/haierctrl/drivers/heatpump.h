#ifndef HAIERCTRL_INCLUDE_DRIVERS_HEATPUMP_H
#define HAIERCTRL_INCLUDE_DRIVERS_HEATPUMP_H

#include "zephyr/device.h"
#include "zephyr/drivers/gpio.h"
#include "zephyr/sys/__assert.h"
#include "zephyr/toolchain.h"
#include <stdint.h>
#include <zephyr/kernel.h>

struct heatpump_config {
  const struct device *uart;
  const char *modbus_iface_name;
  uint8_t slave_addr;
  uint32_t baud_rate;
  struct gpio_dt_spec enable_gpio;
};

struct regs {
  uint16_t R101[6];
  uint16_t R141[16];
  uint16_t R201[1];
  uint16_t R241[22];
};

struct heatpump_data {
  int modbus_client_iface;
  struct regs registers;
};

enum heatpump_state {
  HEATPUMP_STATE_ON = 0,
  HEATPUMP_STATE_OFF,
  HEATPUMP_STATE_COOLING,
  HEATPUMP_STATE_HEATING,
  HEATPUMP_STATE_DHW,
  HEATPUMP_STATE_COOL_DHW,
  HEATPUMP_STATE_HEAT_DHW,
  HEATPUMP_STATE_INVALID,
};

enum heatpump_mode {
  HEATPUMP_MODE_ECO = 0,
  HEATPUMP_MODE_QUIET,
  HEATPUMP_MODE_TURBO,
  HEATPUMP_MODE_INVALID,

  HEATPUMP_MODE_NONE,
  HEATPUMP_MODE_COOL,
  HEATPUMP_MODE_HEAT,
};

enum heatpump_threeway {
  THREEWAY_OFF = 0,
  THREEWAY_DHW,
  THREEWAY_CH,
  THREEWAY_ANTIFREEZE,
  THREEWAY_DEFROST,
  THREEWAY_INVALID
};

struct heatpump_status {
  bool is_on;
  bool has_tank;
  enum heatpump_mode heat_mode;
};

typedef enum heatpump_threeway (*heatpump_get_3way_t)(const struct device *dev);
typedef enum heatpump_state (*heatpump_get_state_t)(const struct device *dev);
typedef enum heatpump_mode (*heatpump_get_mode_t)(const struct device *dev);
typedef float (*read_float_t)(const struct device *dev);
typedef int (*read_float2_t)(const struct device *dev, float *ti, float *to);
typedef int (*read_hp_status_t)(const struct device *dev, struct heatpump_status *stat);
typedef int (*set_float_t)(const struct device *dev, float val);

__subsystem struct heatpump_driver_api {
  heatpump_get_3way_t get_3way;
  read_float_t read_ch_temp;
  read_float_t read_dhw_temp;
  heatpump_get_state_t read_heater_state;
  heatpump_get_mode_t read_heatpump_mode;
  heatpump_get_state_t read_pump_state;
  read_float2_t read_twi_two;
  read_hp_status_t read_status;
  set_float_t set_ch_temp;
};

static inline enum heatpump_threeway heatpump_get_3way(const struct device *dev) {
  __ASSERT_NO_MSG(dev != NULL);
  return DEVICE_API_GET(heatpump, dev)->get_3way(dev);
}

static inline float heatpump_read_ch_temp(const struct device *dev) {
  __ASSERT_NO_MSG(dev != NULL);
  return DEVICE_API_GET(heatpump, dev)->read_ch_temp(dev);
}

static inline float heatpump_read_dhw_temp(const struct device *dev) {
  __ASSERT_NO_MSG(dev != NULL);
  return DEVICE_API_GET(heatpump, dev)->read_dhw_temp(dev);
}

static inline enum heatpump_state heatpump_read_heater_state(const struct device *dev) {
  __ASSERT_NO_MSG(dev != NULL);
  return DEVICE_API_GET(heatpump, dev)->read_heater_state(dev);
}

static inline enum heatpump_mode heatpump_read_mode(const struct device *dev) {
  __ASSERT_NO_MSG(dev != NULL);
  return DEVICE_API_GET(heatpump, dev)->read_heatpump_mode(dev);
}

static inline enum heatpump_state heatpump_read_pump_state(const struct device *dev) {
  __ASSERT_NO_MSG(dev != NULL);
  return DEVICE_API_GET(heatpump, dev)->read_pump_state(dev);
}

static inline int heatpump_read_twi_two(const struct device *dev, float *ti, float *to) {
  __ASSERT_NO_MSG(dev != NULL);
  return DEVICE_API_GET(heatpump, dev)->read_twi_two(dev, ti, to);
}

static inline int heatpump_read_status(const struct device *dev, struct heatpump_status *status) {
  __ASSERT_NO_MSG(dev != NULL);
  return DEVICE_API_GET(heatpump, dev)->read_status(dev, status);
}

static inline int heatpump_set_ch_temp(const struct device *dev, float temp) {
  __ASSERT_NO_MSG(dev != NULL);
  return DEVICE_API_GET(heatpump, dev)->set_ch_temp(dev, temp);
}
#endif 
