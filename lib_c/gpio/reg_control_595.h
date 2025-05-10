#ifndef REG_CONTROL_595_H
#define REG_CONTROL_595_H

typedef struct RegControl595 RegControl595;

// Инициализация
RegControl595* reg_control_595_init(void);

// Деинициализация
void reg_control_595_deinit(RegControl595 *ctrl);

// Обновление регистров
void reg_update(RegControl595 *ctrl);

// Управление BL_KEY_CS
void bl_key_cs_H(RegControl595 *ctrl, int n);
void bl_key_cs_L(RegControl595 *ctrl, int n);

// Установка IN_MUX
void in_mux_set(RegControl595 *ctrl, int n);

// Управление WR_SPDT
void wr_spdt_H(RegControl595 *ctrl, int n);
void wr_spdt_L(RegControl595 *ctrl, int n);

// Управление IN_MUX_EN
void in_mux_EN_H(RegControl595 *ctrl, int n);
void in_mux_EN_L(RegControl595 *ctrl, int n);

// Управление WR_DAC_CS
void wr_dac_cs_H(RegControl595 *ctrl);
void wr_dac_cs_L(RegControl595 *ctrl);

// Управление WR_SPDT_COMM
void wr_spdt_comm_H(RegControl595 *ctrl);
void wr_spdt_comm_L(RegControl595 *ctrl);

// Управление SPDT_SELECT_MODE
void spdt_select_mode_H(RegControl595 *ctrl);
void spdt_select_mode_L(RegControl595 *ctrl);

// Управление ADC_SPDT
void spdt_select_mode_for_ADC_wr(RegControl595 *ctrl);
void spdt_select_mode_for_ADC_mvm(RegControl595 *ctrl);

// Управление buzzer и LED
void beep_beep(RegControl595 *ctrl, float timeS);
void blink_blink(RegControl595 *ctrl, float timeS);

#endif // REG_CONTROL_595_H