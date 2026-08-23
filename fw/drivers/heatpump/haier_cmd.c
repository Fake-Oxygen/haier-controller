#include "haier_cmd.h"
#include "rom/uart.h"
#include <haierctrl/drivers/heatpump.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/errno.h>

enum heatpump_threeway get_3way_state(const uint16_t *r141_regs, size_t count) {
  if(count != 16)
    return THREEWAY_INVALID;

  if(r141_regs[0] & BIT(9))
    return THREEWAY_DHW;

  if(r141_regs[0] & BIT(10))
    return THREEWAY_CH;

  if(r141_regs[0] & BIT(11))
    return THREEWAY_ANTIFREEZE;

  if(r141_regs[0] & BIT(13))
    return THREEWAY_DEFROST;

  return THREEWAY_OFF;
}

int get_arch_errors(const uint16_t *r241_regs, size_t count, struct arch_errors *err) {
  if(count != 22 || err == NULL)
    return -EINVAL;

  err->err1 = (uint8_t)(r241_regs[19] >> 8);
  err->err2 = (uint8_t)(r241_regs[19] & 0xFF);
  err->err3 = (uint8_t)(r241_regs[20] >> 8);

  return 0;
}

int get_ch_temp(const uint16_t *r101_regs, size_t count, float *temp) {
  if(count != 6 || temp == NULL)
    return -EINVAL;

  *temp = (float)((uint8_t)(r101_regs[1] >> 8)) / 2.0f;
  return 0;
}

int get_comp_info(const uint16_t *r241_regs, size_t count, struct compressor_info *comp) {
  if(count != 22 || comp == NULL)
    return -EINVAL;

  comp->fact = (uint8_t)(r241_regs[2] & 0xFF);
  comp->fset = (uint8_t)(r241_regs[3] >> 8);
  comp->current = (float)((uint8_t)(r241_regs[15] >> 8)) / 5.0f;
  comp->voltage = (float)((uint8_t)(r241_regs[15] & 0xFF)) * 4.0f;
  comp->temp = (float)((uint8_t)(r241_regs[14] & 0xFF)) / 10.0f;
  return 0;
}

int get_dhw_compensation(const uint16_t *r101_regs, size_t count, float *comp) {
  if(count != 6 || comp == NULL)
    return -EINVAL;

  *comp = (float)(((int)(r101_regs[5] & 0xFF) - 30)) / 2.0f;
  return 0;
} 

int get_dhw_temp(const uint16_t *r141_regs, size_t count, float *temp) {
  if(count != 16 || temp == NULL)
    return -EINVAL;

  *temp = (float)((int16_t)(r141_regs[13])) / 10.0f;
  return 0;
} 

int get_dhw_target_temp(const uint16_t *r101_regs, size_t count, float *temp) {
  if(count != 6 || temp == NULL)
    return -EINVAL;

  *temp = (float)((int8_t)(r101_regs[5] >> 8)) / 2.0f;
  return 0;
}

int get_eev_level(const uint16_t *r241_regs, size_t count, uint8_t *eev) {
  if(count != 22 || eev == NULL)
    return -EINVAL;
  
  *eev = (uint8_t)(r241_regs[5] >> 8);
  return 0;
}

int get_error(const uint16_t *r141_regs, size_t count, uint8_t *err) {
  if(count != 16 || err == NULL)
    return -EINVAL;
  
  *err = (uint8_t)(r141_regs[0] & 0xFF);
  return 0;
}

int get_fans_rpm(const uint16_t *r241_regs, size_t count, struct fans_info *fans) {
  if(count != 22 || fans == NULL)
    return -EINVAL;
  
  fans->rpm[0] = (uint16_t)(r241_regs[3] & 0xFF) * 5;
  fans->rpm[1] = (uint16_t)(r241_regs[4] >> 8) * 5;
  return 0;
}

int get_firmware_version(const uint16_t *r241_regs, size_t count, float *ver) {
  if(count != 22 || ver == NULL)
    return -EINVAL;

  *ver = (float)((uint8_t)(r241_regs[18] >> 8)) / 10.0f;
  return 0;
}

enum heatpump_state get_heater_state(const uint16_t *r141_regs, size_t count) {
  if(count != 16)
    return HEATPUMP_STATE_INVALID;

  if(r141_regs[3] & BIT(8))
    return HEATPUMP_STATE_ON;

  return HEATPUMP_STATE_OFF;
}

int get_last_error(const uint16_t *r241_regs, size_t count, uint8_t *err) {
  if(count != 22 || err == NULL)
    return -EINVAL;
  
  *err = (uint8_t)(r241_regs[0] & 0xFF);
  return 0;
}

enum heatpump_mode get_mode(const uint16_t *r201_regs, size_t count) {
  if(count != 1)
    return HEATPUMP_MODE_INVALID;

  if(r201_regs[0] == 0)
    return HEATPUMP_MODE_ECO;
  if(r201_regs[0] == 1)
    return HEATPUMP_MODE_QUIET;
  if(r201_regs[0] == 2)
    return HEATPUMP_MODE_TURBO;

  return HEATPUMP_MODE_INVALID;
}

int get_pd_ps_info(const uint16_t *r241_regs, size_t count, struct pd_ps_info *pdps) {
  if(count != 22 || pdps == NULL)
    return -EINVAL;
  
  pdps->pd_set = (float)((uint8_t)(r241_regs[5] & 0xFF)) * 0.2f;
  pdps->pd_act = (float)((uint8_t)(r241_regs[6] >> 8)) * 0.2f;
  pdps->ps_set = (float)((uint8_t)(r241_regs[8] >> 8)) * 0.2f;
  pdps->ps_act = (float)((uint8_t)(r241_regs[8] & 0xFF)) * 0.2f;
  return 0;
}

enum heatpump_state get_pump_state(const uint16_t *r141_regs, size_t count) {
  if(count != 16)
    return HEATPUMP_STATE_INVALID;

  if(r141_regs[3] & BIT(9))
    return HEATPUMP_STATE_ON;

  return HEATPUMP_STATE_OFF;
}

int get_state(const uint16_t *r101_regs, size_t count, struct heatpump_status *stat) {
  if (count != 6 || stat == NULL)
    return -EINVAL;

  uint16_t val = r101_regs[0];

  stat->is_on = (val & BIT(0)) != 0;

  stat->has_tank = (val & BIT(7)) != 0;

  bool is_cool = (val & BIT(1)) != 0;
  bool is_heat = (val & BIT(2)) != 0;

  if (is_cool && is_heat)
    stat->heat_mode = HEATPUMP_MODE_NONE;
  else if (is_cool)
    stat->heat_mode = HEATPUMP_MODE_COOL;
  else if (is_heat)
    stat->heat_mode = HEATPUMP_MODE_HEAT;
  else
    stat->heat_mode = HEATPUMP_MODE_NONE;

  return 0;
}

int get_tao(const uint16_t *r241_regs, size_t count, float *tao_out) {
  if (count != 22 || tao_out == NULL)
    return -EINVAL;

  uint16_t raw_12bit = r241_regs[12] & 0x0FFF;

  int16_t tao_raw;
  if (raw_12bit > 2047) 
    tao_raw = (int16_t)(raw_12bit - 4096);
  else
    tao_raw = (int16_t)raw_12bit;

  *tao_out = (float)tao_raw / 10.0f;

  return 0;
}

int get_tdef(const uint16_t *r241_regs, size_t count, float *tdef_out) {
  if (count != 22 || tdef_out == NULL)
    return -EINVAL;

  uint16_t msb = (r241_regs[12] >> 12) & 0x0F;
  uint16_t lsb = (r241_regs[13] >> 8) & 0xFF;
  uint16_t raw_12bit = (msb << 8) | lsb;

  int16_t tdef_raw;
  if (raw_12bit > 2047)
    tdef_raw = (int16_t)(raw_12bit - 4096);
  else
    tdef_raw = (int16_t)raw_12bit;

  *tdef_out = (float)tdef_raw / 10.0f;

  return 0;
}

int get_td_ts_info(const uint16_t *r241_regs, size_t count, struct td_ts_info *tdts) {
  if (count != 22 || tdts == NULL)
    return -EINVAL;

  uint8_t reg10_low = (uint8_t)(r241_regs[10] & 0xFF);
  uint8_t reg11_high = (uint8_t)(r241_regs[11] >> 8);
  uint8_t reg11_low = (uint8_t)(r241_regs[11] & 0xFF);

  uint16_t td_raw_12 = ((reg10_low & 0x0F) << 8) | reg11_high;
  int16_t td_signed = (td_raw_12 > 2047) ? (int16_t)(td_raw_12 - 4096) : (int16_t)td_raw_12;
  tdts->td = (float)td_signed / 10.0f;

  uint16_t ts_raw_12 = (((reg10_low >> 4) & 0x0F) << 8) | reg11_low;
  int16_t ts_signed = (ts_raw_12 > 2047) ? (int16_t)(ts_raw_12 - 4096) : (int16_t)ts_raw_12;
  tdts->ts = (float)ts_signed / 10.0f;

  return 0;
}

int get_temp_compensation(const uint16_t *r101_regs, size_t count, float *comp_out) {
  if (count != 6 || comp_out == NULL)
    return -EINVAL;

  int raw_byte = (int)(r101_regs[1] & 0xFF);
  *comp_out = (raw_byte - 30) / 2.0f;
  return 0;
}

int get_thi_tho_info(const uint16_t *r141_regs, size_t count, struct ti_to_info *thitho) {
  if (count != 16 || thitho == NULL)
    return -EINVAL;

  uint8_t reg7_high = (uint8_t)(r141_regs[7] >> 8);
  uint8_t reg7_low = (uint8_t)(r141_regs[7] & 0xFF);
  uint8_t reg8_high = (uint8_t)(r141_regs[8] >> 8);

  uint16_t thi_raw_12 = ((reg7_high & 0x0F) << 8) | reg7_low;
  int16_t thi_signed = (thi_raw_12 > 2047) ? (int16_t)(thi_raw_12 - 4096) : (int16_t)thi_raw_12;
  thitho->ti = (float)thi_signed / 10.0f;

  uint16_t tho_raw_12 = (((reg7_high >> 4) & 0x0F) << 8) | reg8_high;
  int16_t tho_signed = (tho_raw_12 > 2047) ? (int16_t)(tho_raw_12 - 4096) : (int16_t)tho_raw_12;
  thitho->to = (float)tho_signed / 10.0f;

  return 0;
}

int get_tsat_pd_info(const uint16_t *r241_regs, size_t count, struct tsat_info *tsat)
{
  if (count != 22 || tsat == NULL)
    return -EINVAL;

  uint8_t reg6_low = (uint8_t)(r241_regs[6] & 0xFF);
  uint8_t reg7_high = (uint8_t)(r241_regs[7] >> 8);
  uint8_t reg7_low = (uint8_t)(r241_regs[7] & 0xFF);

  uint16_t b4f_raw_12 = ((reg6_low & 0x0F) << 8) | reg7_high;
  int16_t b4f_signed = (b4f_raw_12 > 2047) ? (int16_t)(b4f_raw_12 - 4096) : (int16_t)b4f_raw_12;
  tsat->tsat_target = (float)b4f_signed / 10.0f;

  uint16_t b50_raw_12 = (((reg6_low >> 4) & 0x0F) << 8) | reg7_low;
  int16_t b50_signed = (b50_raw_12 > 2047) ? (int16_t)(b50_raw_12 - 4096) : (int16_t)b50_raw_12;
  tsat->tsat_act = (float)b50_signed / 10.0f;

  return 0;
}

int get_tsat_ps_info(const uint16_t *r241_regs, size_t count, struct tsat_info *tsat)
{
  if (count != 22 || tsat == NULL)
    return -EINVAL;

  uint8_t reg9_high = (uint8_t)(r241_regs[9] >> 8);
  uint8_t reg9_low = (uint8_t)(r241_regs[9] & 0xFF);
  uint8_t reg10_high = (uint8_t)(r241_regs[10] >> 8);

  uint16_t b53_raw_12 = ((reg9_high & 0x0F) << 8) | reg9_low;
  int16_t b53_signed = (b53_raw_12 > 2047) ? (int16_t)(b53_raw_12 - 4096) : (int16_t)b53_raw_12;
  tsat->tsat_target = (float)b53_signed / 10.0f;

  uint16_t b54_raw_12 = (((reg9_high >> 4) & 0x0F) << 8) | reg10_high;
  int16_t b54_signed = (b54_raw_12 > 2047) ? (int16_t)(b54_raw_12 - 4096) : (int16_t)b54_raw_12;
  tsat->tsat_act = (float)b54_signed / 10.0f;

  return 0;
}

int get_twi_two_info(const uint16_t *r141_regs, size_t count, struct ti_to_info *twitwo)
{
  if (count != 16 || twitwo == NULL)
    return -EINVAL;

  uint8_t reg5_low = (uint8_t)(r141_regs[5] & 0xFF);
  uint8_t reg6_high = (uint8_t)(r141_regs[6] >> 8);
  uint8_t reg6_low = (uint8_t)(r141_regs[6] & 0xFF);

  uint16_t twi_raw_12 = ((reg5_low & 0x0F) << 8) | reg6_high;
  twitwo->ti = (float)twi_raw_12 / 10.0f;

  uint16_t two_raw_12 = (((reg5_low >> 4) & 0x0F) << 8) | reg6_low;
  twitwo->to = (float)two_raw_12 / 10.0f;

  return 0;
}

int set_ch_temp(const uint16_t *current_regs, size_t count, float new_temp, uint16_t *out_regs)
{
  if (count != 6 || current_regs == NULL || out_regs == NULL)
    return -EINVAL;

  out_regs[0] = 0x0400 | (current_regs[0] & 0xFF);

  uint16_t scaled_temp = (uint16_t)(new_temp * 2.0f);
  out_regs[1] = (scaled_temp << 8) | (current_regs[1] & 0xFF);
  out_regs[2] = current_regs[2];
  out_regs[3] = current_regs[3] & 0x0F;
  out_regs[4] = current_regs[4] & 0xFF;
  out_regs[5] = current_regs[5];

  return 0;
}

int set_dhw_compensation(const uint16_t *current_regs, size_t count, float new_comp, uint16_t *out_regs)
{
  if (count != 6 || current_regs == NULL || out_regs == NULL)
    return -EINVAL;

  out_regs[0] = 0x0800 | (current_regs[0] & 0xFF);
  out_regs[1] = current_regs[1];
  out_regs[2] = current_regs[2];
  out_regs[3] = current_regs[3] & 0x0F;
  out_regs[4] = current_regs[4] & 0xFF;

  uint16_t curr_temp_high = current_regs[5] & 0xFF00;
  uint8_t compensation = (uint8_t)(30.0f + (new_comp * 2.0f));
  out_regs[5] = curr_temp_high | compensation;

  return 0;
}

int set_dhw_temp(const uint16_t *current_regs, size_t count, float new_temp, uint16_t *out_regs)
{
  if (count != 6 || current_regs == NULL || out_regs == NULL)
    return -EINVAL;

  out_regs[0] = current_regs[0] & 0xFF;
  out_regs[1] = current_regs[1];
  out_regs[2] = current_regs[2];
  out_regs[3] = current_regs[3] & 0x0F;
  out_regs[4] = 0x0200 | (current_regs[4] & 0xFF);
  uint16_t scaled_temp = (uint16_t)(new_temp * 2.0f);
  out_regs[5] = (scaled_temp << 8) | (current_regs[5] & 0xFF);

  return 0;
}

int set_heater(const uint16_t *current_regs, size_t count, bool enable, uint16_t *out_regs)
{
  if (count != 6 || current_regs == NULL || out_regs == NULL)
    return -EINVAL;

  uint8_t heater_bit = enable ? (1 << 3) : 0; 
  uint8_t pump_state = (uint8_t)(current_regs[0] & 0xFF);
  out_regs[0] = 0x1000 | (uint16_t)heater_bit | pump_state;
  out_regs[1] = current_regs[1];
  out_regs[2] = current_regs[2];
  out_regs[3] = current_regs[3] & 0x0F;
  out_regs[4] = current_regs[4] & 0xFF;
  out_regs[5] = current_regs[5];

  return 0;
}

int set_mode(enum heatpump_mode mode, uint16_t *reg_out)
{
  if (reg_out == NULL)
    return -EINVAL;

  switch (mode) {
  case HEATPUMP_MODE_ECO:
  case HEATPUMP_MODE_QUIET:
  case HEATPUMP_MODE_TURBO:
    *reg_out = 0x0100 | (uint16_t)mode;
    return 0;
  default:
    return -EINVAL;
  }
}

int set_pump(const uint16_t *current_regs, size_t count, bool enable, uint16_t *out_regs)
{
  if (count != 6 || current_regs == NULL || out_regs == NULL)
    return -EINVAL;

  uint8_t cpump_bit = enable ? (1 << 5) : 0; /* Bit 5 = 32 */
  uint8_t pump_state = (uint8_t)(current_regs[0] & 0xFF);
  out_regs[0] = 0x2000 | (uint16_t)cpump_bit | pump_state;
  out_regs[1] = current_regs[1];
  out_regs[2] = current_regs[2];
  out_regs[3] = current_regs[3] & 0x0F;
  out_regs[4] = current_regs[4] & 0xFF;
  out_regs[5] = current_regs[5];

  return 0;
}

int set_state(const uint16_t *current_regs, size_t count, enum heatpump_state target_state, uint16_t *out_regs)
{
  if (count != 6 || current_regs == NULL || out_regs == NULL)
    return -EINVAL;

  uint16_t cmd_flag;
  uint8_t state_byte;

  switch (target_state) {
  case HEATPUMP_STATE_OFF:
    cmd_flag = 0x0100;
    state_byte = (uint8_t)((current_regs[0] & 0xFF) - 1);
    break;

  case HEATPUMP_STATE_ON:
    cmd_flag = 0x0100;
    state_byte = (uint8_t)((current_regs[0] & 0xFF) + 1);
    break;

  case HEATPUMP_STATE_COOLING:
    cmd_flag = 0x8600;
    state_byte = 3;
    break;

  case HEATPUMP_STATE_HEATING:
    cmd_flag = 0x8600;
    state_byte = 5;
    break;

  case HEATPUMP_STATE_DHW:
    cmd_flag = 0x8600;
    state_byte = 135;
    break;

  case HEATPUMP_STATE_COOL_DHW:
    cmd_flag = 0x8600;
    state_byte = 131;
    break;

  case HEATPUMP_STATE_HEAT_DHW:
    cmd_flag = 0x8600;
    state_byte = 133;
    break;

  default:
    return -EINVAL;
  }

  out_regs[0] = cmd_flag | (uint16_t)state_byte;
  out_regs[1] = current_regs[1];
  out_regs[2] = current_regs[2];
  out_regs[3] = current_regs[3] & 0x0F;
  out_regs[4] = current_regs[4] & 0xFF;
  out_regs[5] = current_regs[5];

  return 0;
}

int set_temp_compensation(const uint16_t *current_regs, size_t count, float new_comp, uint16_t *out_regs)
{
  if (count != 6 || current_regs == NULL || out_regs == NULL)
    return -EINVAL;

  out_regs[0] = 0x0800 | (current_regs[0] & 0xFF);
  uint16_t curr_temp_high = current_regs[1] & 0xFF00;
  uint8_t compensation = (uint8_t)(30.0f + (new_comp * 2.0f));
  out_regs[1] = curr_temp_high | compensation;
  out_regs[2] = current_regs[2];
  out_regs[3] = current_regs[3] & 0x0F;
  out_regs[4] = current_regs[4] & 0xFF;
  out_regs[5] = current_regs[5];

  return 0;
}
