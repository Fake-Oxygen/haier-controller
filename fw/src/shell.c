#include "relay.h"
#include <sys/errno.h>
#include <zephyr/shell/shell.h>
#include <stdlib.h>
#include <haierctrl/drivers/heatpump.h>
#include "relay.h"

static const struct device *heatpump = DEVICE_DT_GET(DT_ALIAS(heatpump));

static int cmd_set_ch(const struct shell *sh, size_t argc, char **argv) {
  if(argc != 2)
    return -EINVAL;
  char *err;
  float temp = strtof(argv[1], &err);
  return heatpump_set_ch_temp(heatpump, temp);
}

static int cmd_set_dhw(const struct shell *sh, size_t argc, char **argv) {
  if(argc != 2)
    return -EINVAL;
  char *err;
  float temp = strtof(argv[1], &err);
  return heatpump_set_dhw_temp(heatpump, temp);
}

static int cmd_set_mode(const struct shell *sh, size_t argc, char **argv) {
  if(argc != 2)
    return -EINVAL;
  char *err;
  int mode = strtol(argv[1], &err, 0);
  return heatpump_set_mode(heatpump, mode);
}

static int cmd_set_state(const struct shell *sh, size_t argc, char **argv) {
  if(argc != 2)
    return -EINVAL;
  char *err;
  int mode = strtol(argv[1], &err, 0);
  return heatpump_set_state(heatpump, mode);
}

static int cmd_get_ch(const struct shell *sh, size_t argc, char **argv) {
  float ti, to;
  int err = heatpump_read_twi_two(heatpump, &ti, &to);
  if(err < 0) {
    shell_print(sh, "ERR=%d", err);
    return err;
  }
  shell_print(sh, "VAL=%.1f", (double)ti);
  return 0;
}

static int cmd_get_dhw(const struct shell *sh, size_t argc, char **argv) {
  shell_print(sh, "VAL=%.1f", (double)(heatpump_read_dhw_temp(heatpump)));
  return 0;
}

static int cmd_get_mode(const struct shell *sh, size_t argc, char **argv) {
  shell_print(sh, "VAL=%d", heatpump_read_mode(heatpump));
  return 0;
}

static int cmd_get_tank(const struct shell *sh, size_t argc, char **argv) {
  struct heatpump_status stat;
  int err = heatpump_read_status(heatpump, &stat);
  if(err < 0) {
    shell_print(sh, "ERR=%d", err);
    return err;
  }
  shell_print(sh, "VAL=%d", stat.has_tank);
  return 0;
}

static int cmd_get_heat(const struct shell *sh, size_t argc, char **argv) {
  struct heatpump_status stat;
  int err = heatpump_read_status(heatpump, &stat);
  if(err < 0) {
    shell_print(sh, "ERR=%d", err);
    return err;
  }
  shell_print(sh, "VAL=%d", stat.heat_mode);
  return 0;
}

static int cmd_get_3way(const struct shell *sh, size_t argc, char **argv) {
  shell_print(sh, "VAL=%d", heatpump_get_3way(heatpump));
  return 0;
}

static int cmd_get_ch_target(const struct shell *sh, size_t argc, char **argv) {
  shell_print(sh, "VAL=%.1f", (double)heatpump_read_ch_temp(heatpump));
  return 0;
}

static int cmd_get_dhw_target(const struct shell *sh, size_t argc, char **argv) {
  shell_print(sh, "VAL=%.1f", (double)heatpump_read_dhw_target_temp(heatpump));
  return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(sub_heatpump_set,
  SHELL_CMD(ch, NULL, "sets heater temperature", cmd_set_ch),
  SHELL_CMD(dhw, NULL, "sets tank temperature", cmd_set_dhw),
  SHELL_CMD(mode, NULL, "sets heatpump mode", cmd_set_mode),
  SHELL_CMD(state, NULL, "sets heatpump state", cmd_set_state),
  SHELL_SUBCMD_SET_END
);

SHELL_STATIC_SUBCMD_SET_CREATE(sub_heatpump_get,
  SHELL_CMD(ch, NULL, "gets heater temperature", cmd_get_ch),
  SHELL_CMD(dhw, NULL, "gets tank temperature", cmd_get_dhw),
  SHELL_CMD(mode, NULL, "gets heatpump mode", cmd_get_mode),
  SHELL_CMD(tank, NULL, "gets heatpump tank status", cmd_get_tank),
  SHELL_CMD(heat, NULL, "gets heatpump heat / cool state", cmd_get_heat),
  SHELL_CMD(threeway, NULL, "gets heatpump threeway valve state", cmd_get_3way),
  SHELL_CMD(ch_target, NULL, "gets heatpump heating temp target", cmd_get_ch_target),
  SHELL_CMD(dhw_target, NULL, "gets heatpump water temp target", cmd_get_dhw_target),
  SHELL_SUBCMD_SET_END
);

SHELL_STATIC_SUBCMD_SET_CREATE(sub_heatpump,
    SHELL_CMD(get, &sub_heatpump_get, "Read heatpump parameters", NULL),
    SHELL_CMD(set, &sub_heatpump_set, "Write heatpump parameters", NULL),
    SHELL_SUBCMD_SET_END
);

SHELL_CMD_REGISTER(hp, &sub_heatpump, "Heatpump control commands", NULL);

static int cmd_set_relay(const struct shell *sh, size_t argc, char **argv) {
  if(argc != 3)
    return -EINVAL;
  
  char *err;
  uint8_t relay = strtol(argv[1], &err, 0);
  int enable = strtol(argv[2], &err, 0);
  if(relay >= RELAY_COUNT)
    return -EINVAL;
  set_relay_state(relay, enable);
  return 0;
}

SHELL_CMD_REGISTER(set_relay, NULL, "Sets relay state", cmd_set_relay);
