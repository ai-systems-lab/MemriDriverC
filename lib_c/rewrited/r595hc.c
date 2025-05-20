// r595hc.c
#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#define SHCP_PIN 23
#define STCP_PIN 24
#define DS_PIN 16
#define WR_LDAC_PIN 12
#define ADC_CS_PIN 25
#define ADC_SPDT_PIN 26

#define MVM_DAC_CS_0 5
#define MVM_DAC_CS_1 6
#define MVM_DAC_CS_2 7
#define MVM_DAC_CS_3 8
#define MVM_LDAC_PIN 4

#define WL_MUX_EN 13
#define WL_MUX_0 17
#define WL_MUX_1 27
#define WL_MUX_2 22

#define BUZZER_PIN 21
#define LED_PIN 19

typedef struct {
    uint8_t main_bytes[2];
} RegControl595;

void RegControl595_init(RegControl595 *reg) {
    reg->main_bytes[0] = 0xf0;
    reg->main_bytes[1] = 0x20;

    wiringPiSetupGpio();

    pinMode(SHCP_PIN, OUTPUT);
    pinMode(STCP_PIN, OUTPUT);
    pinMode(DS_PIN, OUTPUT);
    pinMode(WR_LDAC_PIN, OUTPUT);
    pinMode(ADC_CS_PIN, OUTPUT);
    pinMode(ADC_SPDT_PIN, OUTPUT);

    pinMode(MVM_DAC_CS_0, OUTPUT);
    pinMode(MVM_DAC_CS_1, OUTPUT);
    pinMode(MVM_DAC_CS_2, OUTPUT);
    pinMode(MVM_DAC_CS_3, OUTPUT);
    pinMode(MVM_LDAC_PIN, OUTPUT);

    pinMode(WL_MUX_EN, OUTPUT);
    pinMode(WL_MUX_0, OUTPUT);
    pinMode(WL_MUX_1, OUTPUT);
    pinMode(WL_MUX_2, OUTPUT);

    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(LED_PIN, OUTPUT);

    digitalWrite(SHCP_PIN, LOW);
    digitalWrite(STCP_PIN, HIGH);
    digitalWrite(DS_PIN, LOW);
    digitalWrite(WR_LDAC_PIN, HIGH);
    digitalWrite(ADC_CS_PIN, HIGH);
    digitalWrite(ADC_SPDT_PIN, LOW);

    digitalWrite(MVM_DAC_CS_0, HIGH);
    digitalWrite(MVM_DAC_CS_1, HIGH);
    digitalWrite(MVM_DAC_CS_2, HIGH);
    digitalWrite(MVM_DAC_CS_3, HIGH);
    digitalWrite(MVM_LDAC_PIN, HIGH);

    digitalWrite(WL_MUX_EN, LOW);
    digitalWrite(WL_MUX_0, LOW);
    digitalWrite(WL_MUX_1, LOW);
    digitalWrite(WL_MUX_2, LOW);

    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_PIN, LOW);

    // Toggle ADC_CS
    digitalWrite(ADC_CS_PIN, HIGH);
    digitalWrite(ADC_CS_PIN, LOW);
    digitalWrite(ADC_CS_PIN, HIGH);
}

void __transfer595(RegControl595 *reg, uint8_t x) {
    x &= 0xff;
    uint8_t ii = 1;
    for (int i = 0; i < 8; i++) {
        digitalWrite(DS_PIN, (ii & x) ? HIGH : LOW);
        ii <<= 1;
        digitalWrite(SHCP_PIN, HIGH);
        digitalWrite(SHCP_PIN, LOW);
    }
    digitalWrite(DS_PIN, LOW);
}

void reg_update(RegControl595 *reg) {
    digitalWrite(STCP_PIN, LOW);
    __transfer595(reg, reg->main_bytes[0]);
    __transfer595(reg, reg->main_bytes[1]);
    digitalWrite(STCP_PIN, HIGH);
}

void bl_key_cs_H(RegControl595 *reg, uint8_t n) {
    reg->main_bytes[0] |= (1 << (4 + n)) & 0xff;
}
void bl_key_cs_L(RegControl595 *reg, uint8_t n) {
    reg->main_bytes[0] &= (~(1 << (4 + n))) & 0xff;
}

void in_mux_set(RegControl595 *reg, uint8_t n) {
    reg->main_bytes[0] &= 0xf0;
    reg->main_bytes[0] |= n & 0xf;
}

void wr_spdt_H(RegControl595 *reg, uint8_t n) {
    reg->main_bytes[1] |= (1 << (2 + n)) & 0xff;
}
void wr_spdt_L(RegControl595 *reg, uint8_t n) {
    reg->main_bytes[1] &= (~(1 << (2 + n))) & 0xff;
}

void in_mux_EN_H(RegControl595 *reg, uint8_t n) {
    reg->main_bytes[1] |= (1 << n) & 0xff;
}
void in_mux_EN_L(RegControl595 *reg, uint8_t n) {
    reg->main_bytes[1] &= (~(1 << n)) & 0xff;
}

void wr_dac_cs_H(RegControl595 *reg) {
    reg->main_bytes[1] |= (1 << 5) & 0xff;
}
void wr_dac_cs_L(RegControl595 *reg) {
    reg->main_bytes[1] &= (~(1 << 5)) & 0xff;
}

void wr_spdt_comm_H(RegControl595 *reg) {
    reg->main_bytes[1] |= (1 << 6) & 0xff;
}
void wr_spdt_comm_L(RegControl595 *reg) {
    reg->main_bytes[1] &= (~(1 << 6)) & 0xff;
}

void spdt_select_mode_H(RegControl595 *reg) {
    reg->main_bytes[1] |= (1 << 7) & 0xff;
}
void spdt_select_mode_L(RegControl595 *reg) {
    reg->main_bytes[1] &= (~(1 << 7)) & 0xff;
}

void spdt_select_mode_for_ADC_wr() {
    digitalWrite(ADC_SPDT_PIN, LOW);
}
void spdt_select_mode_for_ADC_mvm() {
    digitalWrite(ADC_SPDT_PIN, HIGH);
}

void BEEPBEEP(double timeS) {
    digitalWrite(BUZZER_PIN, HIGH);
    usleep(timeS * 1000000);
    digitalWrite(BUZZER_PIN, LOW);
}

void BLINKBLINK(double timeS) {
    digitalWrite(LED_PIN, HIGH);
    usleep(timeS * 1000000);
    digitalWrite(LED_PIN, LOW);
}