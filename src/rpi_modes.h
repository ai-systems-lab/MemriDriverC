#ifndef RPI_MODES_H
#define RPI_MODES_H

#include <stdint.h>
#include "r595hc.h"
#include "MVM_SPI.h"

typedef struct {
    SPI_send mvm_spi;
    RegControl595 reg;
} RPI_modes;

// Инициализация драйвера
void RPI_modes_init(RPI_modes *rpi);

// Установка SPI режимов
void set_mode_0(void);
void set_mode_1(void);

// Режимы работы
void mode_7(RPI_modes *rpi, uint16_t vDAC, uint16_t tms, uint16_t tus, uint8_t rev,
            uint16_t id, uint8_t wl, uint8_t bl, uint16_t *result, uint16_t *ret_id);

void mode_9(RPI_modes *rpi, uint16_t vDAC, uint16_t id, uint8_t wl, uint8_t bl,
            uint16_t *result, uint16_t *ret_id);

void mode_mvm(RPI_modes *rpi, uint16_t *vDAC_mas, uint16_t tms, uint16_t tus,
              uint16_t rtms, uint16_t rums, uint8_t wl, uint16_t id,
              uint16_t *result, uint16_t *ret_id);

// Быстрые режимы
void fast_mvm_ON(RPI_modes *rpi);
void fast_mvm_OFF(RPI_modes *rpi);

void fast_mvm(RPI_modes *rpi, uint16_t *vDAC_mas, uint16_t tms, uint16_t tus,
              uint16_t rtms, uint16_t rums, uint8_t wl, uint16_t id,
              uint16_t *result, uint16_t *ret_id);

#endif // RPI_MODES_H
