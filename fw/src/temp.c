#include "zephyr/device.h"
#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/sensor.h>

LOG_MODULE_REGISTER(temp, CONFIG_LOG_DEFAULT_LEVEL);

static const struct device *ds18b20 = DEVICE_DT_GET(DT_ALIAS(temp));

double read_temp() {
  struct sensor_value temp;
  int err = sensor_sample_fetch(ds18b20);
  if(err < 0) {
    LOG_ERR("Failed to fetch temperature from DS18B20 (err: %d)", err);
    return 0;
  }

  err = sensor_channel_get(ds18b20, SENSOR_CHAN_AMBIENT_TEMP, &temp);
  if(err != 0)
    LOG_ERR("Failed to read temperature. Err: %d", err);

  double val = sensor_value_to_double(&temp);
  LOG_DBG("DS18B20 Temperature: %.2f C", val);
  return val;
}

void temp_init() {
  if(!device_is_ready(ds18b20)) {
    LOG_ERR("DS18B20 is not ready!");
    return;
  }
}
