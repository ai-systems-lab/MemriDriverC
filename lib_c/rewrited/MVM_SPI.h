#ifndef SPI_DRIVER_H
#define SPI_DRIVER_H

#include <stdint.h>

// Конфигурация
#define SPI_BUS         0
#define SPI_CHANNEL     0
#define CS_PIN         17
#define SPI_SPEED   15000000

typedef struct {
    int spi_fd;
} SPI_send;

// Прототипы функций SPI
void SPI_send_init(SPI_send *spi);
void init_spi(int bus, int channel, int mode, int speed);
void set_spi_mode(uint8_t mode);
void spi_writebytes(uint8_t *data, int len);
void spi_readbytes(uint8_t *data, int len);
void close_spi(void);

// Функции DAC/ADC/ключей
void mvm_dac_init(void);
void mvm_dac(uint16_t bitvalue, uint8_t channel);
void wr_dac(uint16_t bitvalue);
uint16_t adc_read(void);
void key_set_MVM_on_mask(uint8_t mask);
void key_set_MVM_on();
void key_set_MVM_off(void);
void mwm_dac_pd_on(void);
void mwm_dac_pd_off(void);


#endif // SPI_DRIVER_H