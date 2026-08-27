
#include "zephyr/device.h"
#include "zephyr/devicetree.h"
#include "zephyr/logging/log.h"
#include <stdint.h>
#include <zephyr/drivers/w1.h>

LOG_MODULE_REGISTER(rom, CONFIG_LOG_DEFAULT_LEVEL);

static const struct device *ds2401 = DEVICE_DT_GET(DT_NODELABEL(w1_bus_id));

void read_id(struct w1_rom *rom) {
  if(!device_is_ready(ds2401)) {
    LOG_ERR("One wire device not ready!");
    return;
  }
  
  int err = w1_reset_bus(ds2401);
  if(err < 0) {
    LOG_ERR("One wire devices not detected!");
    return;
  }

  w1_write_byte(ds2401, W1_CMD_READ_ROM);
  w1_read_block(ds2401, (uint8_t *)rom, sizeof(struct w1_rom));

  LOG_INF("1-Wire device family: 0x%02X, Serial: %02X%02X%02X%02X%02X%02X",
      rom->family, rom->serial[0], rom->serial[1], rom->serial[2], rom->serial[3],
      rom->serial[4], rom->serial[5]);
}
