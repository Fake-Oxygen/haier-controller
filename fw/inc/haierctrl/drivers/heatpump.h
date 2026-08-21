#ifndef HAIERCTRL_INCLUDE_DRIVERS_HEATPUMP_H
#define HAIERCTRL_INCLUDE_DRIVERS_HEATPUMP_H

#include "zephyr/device.h"
#include "zephyr/sys/__assert.h"
#include "zephyr/toolchain.h"
#include <stdint.h>
#include <zephyr/kernel.h>

enum heatpump_state {
  HEATPUMP_STATE_ON = 0,
  HEATPUMP_STATE_OFF
};

typedef enum heatpump_state (*heatpump_get_state_t)(const struct device *dev);

__subsystem struct heatpump_driver_api {
  heatpump_get_state_t get_state;
};

__syscall enum heatpump_state heatpump_get_state(const struct device *dev);

static inline int z_impl_heatpump_get_state(const struct device *dev) {
  __ASSERT_NO_MSG(dev != NULL);
  return DEVICE_API_GET(heatpump, dev)->get_state(dev);
}

#endif 
