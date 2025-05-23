// rpi_modes.c
#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include "r595hc.h"
#include "MVM_SPI.h"

typedef struct {
    SPI_send mvm_spi;
    RegControl595 reg;
} RPI_modes;

void RPI_modes_init(RPI_modes *rpi) {
    SPI_send_init(&rpi->mvm_spi);
    RegControl595_init(&rpi->reg);
    init_spi(SPI_BUS, SPI_CHANNEL,0, SPI_SPEED);

    for (int i = 5; i <= 8; i++) {
        digitalWrite(i, LOW);
        mvm_dac_init();
        digitalWrite(i, HIGH);
    }
}

void set_mode_0(void){
    set_spi_mode(0);
}

void set_mode_1(void){
    set_spi_mode(1);
}

void mode_7(RPI_modes *rpi, uint16_t vDAC, uint16_t tms, uint16_t tus, uint8_t rev, 
            uint16_t id, uint8_t wl, uint8_t bl, uint16_t *result, uint16_t *ret_id) {
    // Off MVM DACs
    for (int i = 5; i <= 8; i++) digitalWrite(i, LOW);
    mwm_dac_pd_on();
    for (int i = 5; i <= 8; i++) digitalWrite(i, HIGH);
    
    // LDAC
    digitalWrite(4, LOW);
    digitalWrite(4, HIGH);
    
    // Open all keys 714
    for (int i = 0; i < 4; i++) {
        bl_key_cs_L(&rpi->reg, i);
        reg_update(&rpi->reg);
        set_mode_1();
        key_set_MVM_off();
        bl_key_cs_H(&rpi->reg, i);
        reg_update(&rpi->reg);
    }
    
    // Set commutation WR for bl
    if (bl >= 0 && bl <= 15) {
        wr_spdt_comm_L(&rpi->reg);
        in_mux_EN_H(&rpi->reg, 0);
        in_mux_EN_L(&rpi->reg, 1);
    } else if (bl >= 16 && bl <= 31) {
        wr_spdt_comm_H(&rpi->reg);
        in_mux_EN_L(&rpi->reg, 0);
        in_mux_EN_H(&rpi->reg, 1);
        bl -= 16;
    }
    in_mux_set(&rpi->reg, bl);
    
    // Set mode to WR on wl mux spdt
    spdt_select_mode_L(&rpi->reg);
    reg_update(&rpi->reg);
    
    // Set commutation wl
    if (wl >= 0 && wl <= 7) {
        digitalWrite(17, (wl & 0b1) ? HIGH : LOW);
        digitalWrite(27, (wl >> 1 & 0b1) ? HIGH : LOW);
        digitalWrite(22, (wl >> 2 & 0b1) ? HIGH : LOW);
        digitalWrite(13, HIGH);
    } else {
        printf("Wrong WL!\n");
    }
    
    // Set signal direction
    if (rev) {
        wr_spdt_H(&rpi->reg, 0);
        wr_spdt_H(&rpi->reg, 1);
        wr_spdt_L(&rpi->reg, 2);
    } else {
        wr_spdt_L(&rpi->reg, 0);
        wr_spdt_L(&rpi->reg, 1);
        wr_spdt_L(&rpi->reg, 2);
        if (vDAC > 2457) vDAC = 2457;
    }
    reg_update(&rpi->reg);
    reg_update(&rpi->reg);
    
    // Set signal on WR DAC
    wr_dac_cs_L(&rpi->reg);
    reg_update(&rpi->reg);
    set_mode_0();
    wr_dac(vDAC);
    wr_dac_cs_H(&rpi->reg);
    reg_update(&rpi->reg);
    
    // Update WR DAC value with LDAC WR
    digitalWrite(12, LOW);
    digitalWrite(12, HIGH);
    
    // Wait
    usleep(tms * 1000 + tus);
    
    // Set ZERO on WR DAC
    wr_dac_cs_L(&rpi->reg);
    reg_update(&rpi->reg);
    set_mode_0();
    wr_dac(0);
    wr_dac_cs_H(&rpi->reg);
    reg_update(&rpi->reg);
    
    // Update WR DAC value with LDAC WR
    digitalWrite(12, LOW);
    digitalWrite(12, HIGH);
    
    // Set reading direction
    wr_spdt_L(&rpi->reg, 0);
    wr_spdt_L(&rpi->reg, 1);
    wr_spdt_H(&rpi->reg, 2);
    reg_update(&rpi->reg);
    
    // Set ZERO on WR DAC
    wr_dac_cs_L(&rpi->reg);
    reg_update(&rpi->reg);
    set_mode_0();
    wr_dac(246);
    wr_dac_cs_H(&rpi->reg);
    reg_update(&rpi->reg);
    
    // Update WR DAC value with LDAC WR
    digitalWrite(12, LOW);
    digitalWrite(12, HIGH);
    
    // Read from ADC
    *result = 0;
    spdt_select_mode_for_ADC_wr();
    set_mode_1();
    digitalWrite(25, LOW);
    adc_read();
    digitalWrite(25, HIGH);
    
    for (int i = 0; i < 10; i++) {
        digitalWrite(25, LOW);
        *result += adc_read();
        digitalWrite(25, HIGH);
    }
    *result /= 10;
    
    // Set ZERO on WR DAC
    wr_dac_cs_L(&rpi->reg);
    reg_update(&rpi->reg);
    set_mode_0();
    wr_dac(0);
    wr_dac_cs_H(&rpi->reg);
    reg_update(&rpi->reg);
    
    // Update WR DAC value with LDAC WR
    digitalWrite(12, LOW);
    digitalWrite(12, HIGH);
    
    // Set dir direction
    wr_spdt_L(&rpi->reg, 0);
    wr_spdt_L(&rpi->reg, 1);
    wr_spdt_L(&rpi->reg, 2);
    reg_update(&rpi->reg);
    
    // Disable commutation
    digitalWrite(13, LOW);
    digitalWrite(17, LOW);
    digitalWrite(27, LOW);
    digitalWrite(22, LOW);
    wr_spdt_comm_L(&rpi->reg);
    in_mux_EN_L(&rpi->reg, 0);
    in_mux_EN_L(&rpi->reg, 1);
    in_mux_set(&rpi->reg, 0);
    spdt_select_mode_L(&rpi->reg);
    reg_update(&rpi->reg);
    
    // Open all keys 714
    for (int i = 0; i < 4; i++) {
        bl_key_cs_L(&rpi->reg, i);
        reg_update(&rpi->reg);
        set_mode_1();
        key_set_MVM_off();
        bl_key_cs_H(&rpi->reg, i);
        reg_update(&rpi->reg);
    }
    
    // On MVM DACs
    for (int i = 5; i <= 8; i++) digitalWrite(i, LOW);
    mwm_dac_pd_off();
    for (int i = 5; i <= 8; i++) digitalWrite(i, HIGH);
    
    // LDAC
    digitalWrite(4, LOW);
    digitalWrite(4, HIGH);
    
    *ret_id = id;
}

void mode_9(RPI_modes *rpi, uint16_t vDAC, uint16_t id, uint8_t wl, uint8_t bl, 
            uint16_t *result, uint16_t *ret_id) {
    // Off MVM DACs
    for (int i = 5; i <= 8; i++) digitalWrite(i, LOW);
    mwm_dac_pd_on();
    for (int i = 5; i <= 8; i++) digitalWrite(i, HIGH);
    
    // LDAC
    digitalWrite(4, LOW);
    digitalWrite(4, HIGH);
    
    // Open all keys 714
    for (int i = 0; i < 4; i++) {
        bl_key_cs_L(&rpi->reg, i);
        reg_update(&rpi->reg);
        set_mode_1();
        key_set_MVM_off();
        bl_key_cs_H(&rpi->reg, i);
        reg_update(&rpi->reg);
    }
    
    // Set commutation WR for bl
    if (bl >= 0 && bl <= 15) {
        wr_spdt_comm_L(&rpi->reg);
        in_mux_EN_H(&rpi->reg, 0);
        in_mux_EN_L(&rpi->reg, 1);
    } else if (bl >= 16 && bl <= 31) {
        wr_spdt_comm_H(&rpi->reg);
        in_mux_EN_L(&rpi->reg, 0);
        in_mux_EN_H(&rpi->reg, 1);
        bl -= 16;
    }
    in_mux_set(&rpi->reg, bl);
    
    // Set mode to WR on wl mux spdt
    spdt_select_mode_L(&rpi->reg);
    reg_update(&rpi->reg);
    
    // Set commutation wl
    if (wl >= 0 && wl <= 7) {
        digitalWrite(17, (wl & 0b1) ? HIGH : LOW);
        digitalWrite(27, (wl >> 1 & 0b1) ? HIGH : LOW);
        digitalWrite(22, (wl >> 2 & 0b1) ? HIGH : LOW);
        digitalWrite(13, HIGH);
    } else {
        printf("Wrong WL!\n");
    }
    
    // Set reading direction
    wr_spdt_L(&rpi->reg, 0);
    wr_spdt_L(&rpi->reg, 1);
    wr_spdt_H(&rpi->reg, 2);
    reg_update(&rpi->reg);
    
    // Set ZERO on WR DAC
    if (vDAC > 246) vDAC = 246;
    wr_dac_cs_L(&rpi->reg);
    reg_update(&rpi->reg);
    set_mode_0();
    wr_dac(vDAC);
    wr_dac_cs_H(&rpi->reg);
    reg_update(&rpi->reg);
    
    // Update WR DAC value with LDAC WR
    digitalWrite(12, LOW);
    digitalWrite(12, HIGH);
    
    // Read from ADC
    spdt_select_mode_for_ADC_wr();
    set_mode_1();
    digitalWrite(25, LOW);
    adc_read();
    digitalWrite(25, HIGH);
    
    digitalWrite(25, LOW);
    *result = adc_read();
    digitalWrite(25, HIGH);
    
    // Set ZERO on WR DAC
    wr_dac_cs_L(&rpi->reg);
    reg_update(&rpi->reg);
    set_mode_0();
    wr_dac(0);
    wr_dac_cs_H(&rpi->reg);
    reg_update(&rpi->reg);
    
    // Update WR DAC value with LDAC WR
    digitalWrite(12, LOW);
    digitalWrite(12, HIGH);
    
    // Set dir direction
    wr_spdt_L(&rpi->reg, 0);
    wr_spdt_L(&rpi->reg, 1);
    wr_spdt_L(&rpi->reg, 2);
    reg_update(&rpi->reg);
    
    // Disable commutation
    digitalWrite(13, LOW);
    digitalWrite(17, LOW);
    digitalWrite(27, LOW);
    digitalWrite(22, LOW);
    wr_spdt_comm_L(&rpi->reg);
    in_mux_EN_L(&rpi->reg, 0);
    in_mux_EN_L(&rpi->reg, 1);
    in_mux_set(&rpi->reg, 0);
    spdt_select_mode_L(&rpi->reg);
    reg_update(&rpi->reg);
    
    // Open all keys 714
    for (int i = 0; i < 4; i++) {
        bl_key_cs_L(&rpi->reg, i);
        reg_update(&rpi->reg);
        set_mode_1();
        key_set_MVM_off();
        bl_key_cs_H(&rpi->reg, i);
        reg_update(&rpi->reg);
    }
    
    // On MVM DACs
    for (int i = 5; i <= 8; i++) digitalWrite(i, LOW);
    mwm_dac_pd_off();
    for (int i = 5; i <= 8; i++) digitalWrite(i, HIGH);
    
    // LDAC
    digitalWrite(4, LOW);
    digitalWrite(4, HIGH);
    
    *ret_id = id;
}

void mode_mvm(RPI_modes *rpi, uint16_t *vDAC_mas, uint16_t tms, uint16_t tus, 
              uint16_t rtms, uint16_t rums, uint8_t wl, uint16_t id, 
              uint16_t *result, uint16_t *ret_id) {
    // On MVM DACs
    for (int i = 5; i <= 8; i++) digitalWrite(i, LOW);
    set_mode_1();
    mwm_dac_pd_off();
    for (int i = 5; i <= 8; i++) digitalWrite(i, HIGH);
    
    // LDAC
    digitalWrite(4, LOW);
    digitalWrite(4, HIGH);
    
    // Close all keys 714 based on vDAC_mas mask
    uint8_t mask[32];
    for (int i = 0; i < 32; i++) {
        mask[i] = vDAC_mas[i] > 0 ? 1 : 0;
    }
    uint8_t mask_bytes[4] = {0};
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            mask_bytes[i] |= (mask[i * 8 + j] << j);
        }
    }

    // Открываем файл для записи (режим "a" — дописывать в конец)
    FILE *log_file = fopen("mvm_mask_debug.log", "a");
    if (log_file) {
        fprintf(log_file, "--- MVM Mask Debug ---\n");
        
        // Выводим маску для каждого блока
        for (int i = 0; i < 4; i++) {
            fprintf(log_file, "Block %d: 0x%02X (", i, mask_bytes[i]);
            
            // Выводим биты в двоичном виде (для наглядности)
            for (int j = 7; j >= 0; j--) {
                fprintf(log_file, "%d", (mask_bytes[i] >> j) & 1);
            }
            fprintf(log_file, ")\n");
        }
        fclose(log_file);
    }
    
    for (int i = 0; i < 4; i++) {
        bl_key_cs_L(&rpi->reg, i);
        reg_update(&rpi->reg);
        set_mode_1();
        key_set_MVM_on_mask(mask_bytes[i]);
        bl_key_cs_H(&rpi->reg, i);
        reg_update(&rpi->reg);
    }
    
    // Disable commutation
    digitalWrite(13, LOW);
    digitalWrite(17, LOW);
    digitalWrite(27, LOW);
    digitalWrite(22, LOW);
    wr_spdt_comm_L(&rpi->reg);
    in_mux_EN_L(&rpi->reg, 0);
    in_mux_EN_L(&rpi->reg, 1);
    in_mux_set(&rpi->reg, 0);
    spdt_select_mode_H(&rpi->reg);
    reg_update(&rpi->reg);
    
    // Set commutation wl
    if (wl >= 0 && wl <= 7) {
        digitalWrite(17, (wl & 0b1) ? HIGH : LOW);
        digitalWrite(27, (wl >> 1 & 0b1) ? HIGH : LOW);
        digitalWrite(22, (wl >> 2 & 0b1) ? HIGH : LOW);
        digitalWrite(13, HIGH);
    } else {
        printf("Wrong WL!\n");
    }
    
    // Set values to MVM DACs
    for (int i = 0; i < 4; i++) {
        set_mode_1();
        for (int j = 0; j < 8; j++) {
            digitalWrite(i + 5, LOW);
            mvm_dac(vDAC_mas[j + (8 * i)], j);
            digitalWrite(i + 5, HIGH);
        }
    }
    
    // LDAC
    digitalWrite(4, LOW);
    digitalWrite(4, HIGH);
    
    // Read from ADC
    spdt_select_mode_for_ADC_mvm();
    set_mode_1();
    digitalWrite(25, LOW);
    adc_read();
    digitalWrite(25, HIGH);
    
    digitalWrite(25, LOW);
    *result = adc_read();
    digitalWrite(25, HIGH);
    
    spdt_select_mode_for_ADC_wr();
    
    // Set ZERO to MVM DACs
    set_mode_1();
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            digitalWrite(i + 5, LOW);
            mvm_dac(0, j);
            digitalWrite(i + 5, HIGH);
        }
    }
    
    // LDAC
    digitalWrite(4, LOW);
    digitalWrite(4, HIGH);
    
    // Off MVM DACs
    for (int i = 5; i <= 8; i++) digitalWrite(i, LOW);
    set_mode_1();
    mwm_dac_pd_on();
    for (int i = 5; i <= 8; i++) digitalWrite(i, HIGH);
    
    // LDAC
    digitalWrite(4, LOW);
    digitalWrite(4, HIGH);
    
    // Disable commutation
    digitalWrite(13, LOW);
    digitalWrite(17, LOW);
    digitalWrite(27, LOW);
    digitalWrite(22, LOW);
    spdt_select_mode_H(&rpi->reg);
    reg_update(&rpi->reg);
    
    *ret_id = id;
}

void fast_mvm_ON(RPI_modes *rpi) {
    // On MVM DACs
    for (int i = 5; i <= 8; i++) digitalWrite(i, LOW);
    set_mode_1();
    mwm_dac_pd_off();
    for (int i = 5; i <= 8; i++) digitalWrite(i, HIGH);
    
    // LDAC
    digitalWrite(4, LOW);
    digitalWrite(4, HIGH);
    
   
    
    // Disable commutation
    digitalWrite(13, LOW);
    digitalWrite(17, LOW);
    digitalWrite(27, LOW);
    digitalWrite(22, LOW);
    wr_spdt_comm_L(&rpi->reg);
    in_mux_EN_L(&rpi->reg, 0);
    in_mux_EN_L(&rpi->reg, 1);
    in_mux_set(&rpi->reg, 0);
    spdt_select_mode_H(&rpi->reg);
    reg_update(&rpi->reg);
    
    spdt_select_mode_for_ADC_mvm();
}

void fast_mvm_OFF(RPI_modes *rpi) {
    // Off MVM DACs
    for (int i = 5; i <= 8; i++) digitalWrite(i, LOW);
    set_mode_1();
    mwm_dac_pd_on();
    for (int i = 5; i <= 8; i++) digitalWrite(i, HIGH);
    
    // LDAC
    digitalWrite(4, LOW);
    digitalWrite(4, HIGH);
    
    // Disable commutation
    digitalWrite(13, LOW);
    digitalWrite(17, LOW);
    digitalWrite(27, LOW);
    digitalWrite(22, LOW);
    spdt_select_mode_H(&rpi->reg);
    reg_update(&rpi->reg);
    
    spdt_select_mode_for_ADC_wr();
}

void fast_mvm(RPI_modes *rpi, uint16_t *vDAC_mas, uint16_t tms, uint16_t tus, 
              uint16_t rtms, uint16_t rums, uint8_t wl, uint16_t id, 
              uint16_t *result, uint16_t *ret_id) {

    uint8_t mask[32];
    for (int i = 0; i < 32; i++) {
        mask[i] = vDAC_mas[i] > 0 ? 1 : 0;
    }
    uint8_t mask_bytes[4] = {0};
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            mask_bytes[i] |= (mask[i * 8 + j] << j);
        }
    }
                     
     // Close all keys 714
     for (int i = 0; i < 4; i++) {
        bl_key_cs_L(&rpi->reg, i);
        reg_update(&rpi->reg);
        set_mode_1();
        key_set_MVM_on(mask_bytes[i]);
        bl_key_cs_H(&rpi->reg, i);
        reg_update(&rpi->reg);
    }
    
    
    // Set commutation wl
    if (wl >= 0 && wl <= 7) {
        digitalWrite(17, (wl & 0b1) ? HIGH : LOW);
        digitalWrite(27, (wl >> 1 & 0b1) ? HIGH : LOW);
        digitalWrite(22, (wl >> 2 & 0b1) ? HIGH : LOW);
        digitalWrite(13, HIGH);
    } else {
        printf("Wrong WL!\n");
    }
    
    // Set values to MVM DACs
    for (int i = 0; i < 4; i++) {
        set_mode_1();
        for (int j = 0; j < 8; j++) {
            digitalWrite(i + 5, LOW);
            mvm_dac(vDAC_mas[j + (8 * i)], j);
            digitalWrite(i + 5, HIGH);
        }
    }
    
    // LDAC
    digitalWrite(4, LOW);
    digitalWrite(4, HIGH);
    
    // Read from ADC
    set_mode_1();
    digitalWrite(25, LOW);
    adc_read();
    digitalWrite(25, HIGH);
    
    digitalWrite(25, LOW);
    *result = adc_read();
    digitalWrite(25, HIGH);
    
    // Set ZERO to MVM DACs
    set_mode_1();
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            digitalWrite(i + 5, LOW);
            mvm_dac(0, j);
            digitalWrite(i + 5, HIGH);
        }
    }
    
    // LDAC
    digitalWrite(4, LOW);
    digitalWrite(4, HIGH);
    
    *ret_id = id;
}

int main() {
    wiringPiSetupGpio();
    
    RPI_modes rpi;
    RPI_modes_init(&rpi);
    
    uint16_t result, ret_id;
    //0, 0, 0, 0, 123, 1,5
    mode_7(&rpi, 0, 0, 0, 0, 123, 1, 5, &result, &ret_id);
    printf("Result: %d, ID: %d\n", result, ret_id);
    
    return 0;
}
//gcc -shared -o libmvmdriver.so -fPIC rpi_modes.c MVM_SPI.c r595hc.c -lwiringPi