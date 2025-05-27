// r595hc.h
#ifndef R595HC_H
#define R595HC_H

#include <stdint.h>

typedef struct {
    uint8_t main_bytes[2];
} RegControl595;

void RegControl595_init(RegControl595 *reg);
void reg_update(RegControl595 *reg);
void bl_key_cs_H(RegControl595 *reg, uint8_t n);
void bl_key_cs_L(RegControl595 *reg, uint8_t n);
void in_mux_set(RegControl595 *reg, uint8_t n);
void wr_spdt_H(RegControl595 *reg, uint8_t n);
void wr_spdt_L(RegControl595 *reg, uint8_t n);
void in_mux_EN_H(RegControl595 *reg, uint8_t n);
void in_mux_EN_L(RegControl595 *reg, uint8_t n);
void wr_dac_cs_H(RegControl595 *reg);
void wr_dac_cs_L(RegControl595 *reg);
void wr_spdt_comm_H(RegControl595 *reg);
void wr_spdt_comm_L(RegControl595 *reg);
void spdt_select_mode_H(RegControl595 *reg);
void spdt_select_mode_L(RegControl595 *reg);
void spdt_select_mode_for_ADC_wr(void);
void spdt_select_mode_for_ADC_mvm(void);
void BEEPBEEP(double timeS);
void BLINKBLINK(double timeS);

#endif // R595HC_H