// rpi_modes.c
#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <math.h>
#include "r595hc.c"
#include "MVM_SPI.c"

typedef struct {
    SPI_send mvm_spi;
    RegControl595 reg;
} RPI_modes;

void RPI_modes_init(RPI_modes *rpi) {
    SPI_send_init(&rpi->mvm_spi);
    RegControl595_init(&rpi->reg);

    for (int i = 5; i <= 8; i++) {
        digitalWrite(i, LOW);
        mvm_dac_init(&rpi->mvm_spi);
        digitalWrite(i, HIGH);
    }
}

void mode_7(RPI_modes *rpi, uint16_t vDAC, uint16_t tms, uint16_t tus, uint8_t rev, 
            uint16_t id, uint8_t wl, uint8_t bl, uint16_t *result) {
    // Off MVM DACs
    for (int i = 5; i <= 8; i++) digitalWrite(i, LOW);
    mwm_dac_pd_on(&rpi->mvm_spi);
    for (int i = 5; i <= 8; i++) digitalWrite(i, HIGH);
    
    // LDAC
    digitalWrite(4, LOW);
    digitalWrite(4, HIGH);
    
    // Open all keys 714
    for (int i = 0; i < 4; i++) {
        bl_key_cs_L(&rpi->reg, i);
        reg_update(&rpi->reg);
        set_mode_1(&rpi->mvm_spi);
        key_set_MVM_off(&rpi->mvm_spi);
        bl_key_cs_H(&rpi->reg, i);
        reg_update(&rpi->reg);
    }
    
    // Set commutation WR for bl
    if (bl <= 15) {
        wr_spdt_comm_L(&rpi->reg);
        in_mux_EN_H(&rpi->reg, 0);
        in_mux_EN_L(&rpi->reg, 1);
    } else if (bl <= 31) {
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
    if (wl <= 7) {
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
    set_mode_0(&rpi->mvm_spi);
    wr_dac(&rpi->mvm_spi, vDAC);
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
    set_mode_0(&rpi->mvm_spi);
    wr_dac(&rpi->mvm_spi, 0);
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
    set_mode_0(&rpi->mvm_spi);
    wr_dac(&rpi->mvm_spi, 246);
    wr_dac_cs_H(&rpi->reg);
    reg_update(&rpi->reg);
    
    // Update WR DAC value with LDAC WR
    digitalWrite(12, LOW);
    digitalWrite(12, HIGH);
    
    // Read from ADC
    *result = 0;
    spdt_select_mode_for_ADC_wr();
    set_mode_1(&rpi->mvm_spi);
    digitalWrite(25, LOW);
    adc_read(&rpi->mvm_spi);
    digitalWrite(25, HIGH);
    
    for (int i = 0; i < 10; i++) {
        digitalWrite(25, LOW);
        *result += adc_read(&rpi->mvm_spi);
        digitalWrite(25, HIGH);
    }
    *result /= 10;
    
    // Set ZERO on WR DAC
    wr_dac_cs_L(&rpi->reg);
    reg_update(&rpi->reg);
    set_mode_0(&rpi->mvm_spi);
    wr_dac(&rpi->mvm_spi, 0);
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
        set_mode_1(&rpi->mvm_spi);
        key_set_MVM_off(&rpi->mvm_spi);
        bl_key_cs_H(&rpi->reg, i);
        reg_update(&rpi->reg);
    }
    
    // On MVM DACs
    for (int i = 5; i <= 8; i++) digitalWrite(i, LOW);
    mwm_dac_pd_off(&rpi->mvm_spi);
    for (int i = 5; i <= 8; i++) digitalWrite(i, HIGH);
    
    // LDAC
    digitalWrite(4, LOW);
    digitalWrite(4, HIGH);
}

int main() {
    wiringPiSetupGpio();
    
    RPI_modes rpi;
    RPI_modes_init(&rpi);
    
    uint16_t result;
    mode_7(&rpi, 245, 1, 0, 0, 123, 0, 0, &result);
    printf("Result: %d\n", result);
    
    return 0;
}