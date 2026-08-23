#ifndef HAIER_CMD_H
#define HAIER_CMD_H

#include "haierctrl/drivers/heatpump.h"
#include <stdint.h>
#include <stddef.h>

struct arch_errors {
  uint8_t err1, err2, err3;
};

struct compressor_info {
  float fset;
  float fact;
  float current;
  float voltage;
  float temp;
};

struct fans_info {
  uint16_t rpm[2];
};

struct pd_ps_info {
  float pd_set;
  float pd_act;
  float ps_set;
  float ps_act;
};

struct td_ts_info {
  float td;
  float ts;
};

struct ti_to_info {
  float ti;
  float to;
};

struct tsat_info {
  float tsat_target;
  float tsat_act;
};

struct heatpump_status {
  bool is_on;
  bool has_tank;
  enum heatpump_mode heat_mode;
};

enum heatpump_threeway get_3way_state(const uint16_t *r141_regs, size_t count);
int get_arch_errors(const uint16_t *r241_regs, size_t count, struct arch_errors *err);
int get_ch_temp(const uint16_t *r101_regs, size_t count, float *temp);
int get_comp_info(const uint16_t *r241_regs, size_t count, struct compressor_info *comp);
int get_dhw_compensation(const uint16_t *r101_regs, size_t count, float *comp);
int get_dhw_temp(const uint16_t *r141_regs, size_t count, float *temp);
int get_dhw_target_temp(const uint16_t *r101_regs, size_t count, float *temp);
int get_eev_level(const uint16_t *r241_regs, size_t count, uint8_t *eev);
int get_error(const uint16_t *r141_regs, size_t count, uint8_t *err);
int get_fans_rpm(const uint16_t *r241_regs, size_t count, struct fans_info *fans);
int get_firmware_version(const uint16_t *r241_regs, size_t count, float *ver);
enum heatpump_state get_heater_state(const uint16_t *r141_regs, size_t count);
int get_last_error(const uint16_t *r241_regs, size_t count, uint8_t *err);
enum heatpump_mode get_mode(const uint16_t *r201_regs, size_t count);
int get_pd_ps_info(const uint16_t *r241_regs, size_t count, struct pd_ps_info *pdps);
enum heatpump_state get_pump_state(const uint16_t *r141_regs, size_t count);
int get_state(const uint16_t *r101_regs, size_t count, struct heatpump_status *stat);
int get_tao(const uint16_t *r241_regs, size_t count, float *tao_out);
int get_tdef(const uint16_t *r241_regs, size_t count, float *tdef_out);
int get_td_ts_info(const uint16_t *r241_regs, size_t count, struct td_ts_info *tdts);
int get_temp_compensation(const uint16_t *r101_regs, size_t count, float *comp_out);
int get_thi_tho_info(const uint16_t *r141_regs, size_t count, struct ti_to_info *thitho);
int get_tsat_pd_info(const uint16_t *r241_regs, size_t count, struct tsat_info *tsat);
int get_tsat_ps_info(const uint16_t *r241_regs, size_t count, struct tsat_info *tsat);
int get_twi_two_info(const uint16_t *r141_regs, size_t count, struct ti_to_info *twitwo);
int set_ch_temp(const uint16_t *current_regs, size_t count, float new_temp, uint16_t *out_regs);
int set_dhw_compensation(const uint16_t *current_regs, size_t count, float new_comp, uint16_t *out_regs);
int set_dhw_temp(const uint16_t *current_regs, size_t count, float new_temp, uint16_t *out_regs);
int set_heater(const uint16_t *current_regs, size_t count, bool enable, uint16_t *out_regs);
int set_mode(enum heatpump_mode mode, uint16_t *reg_out);
int set_pump(const uint16_t *current_regs, size_t count, bool enable, uint16_t *out_regs);
int set_state(const uint16_t *current_regs, size_t count, enum heatpump_state target_state, uint16_t *out_regs);
int set_temp_compensation(const uint16_t *current_regs, size_t count, float new_comp, uint16_t *out_regs);

#endif // !HAIER_CMD_H
