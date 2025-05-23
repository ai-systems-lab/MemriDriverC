#include <wiringPi.h>
#include <linux/spi/spidev.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "MVM_SPI.h"

// Глобальные переменные
static int spi_fd;
static int current_spi_mode;

void SPI_send_init(SPI_send *spi) {
    spi->spi_fd = -1;
}

// Инициализация SPI
void init_spi(int bus, int channel, int mode, int speed) {
    char spi_device[20];
    snprintf(spi_device, sizeof(spi_device), "/dev/spidev%d.%d", bus, channel);

    if (wiringPiSetupPinType(WPI_PIN_BCM) == -1) {
        fprintf(stderr, "ERROR: Не удалось инициализировать wiringPi\n");
        exit(1);
    }

    pinMode(CS_PIN, OUTPUT);
    digitalWrite(CS_PIN, HIGH);

    spi_fd = open(spi_device, O_RDWR);
    if (spi_fd < 0) {
        fprintf(stderr, "ERROR: Не удалось открыть %s\n", spi_device);
        exit(1);
    }

    if (ioctl(spi_fd, SPI_IOC_WR_MODE, &mode) < 0) {
        fprintf(stderr, "ERROR: Не удалось установить режим SPI\n");
        exit(1);
    }
    current_spi_mode = mode;

    if (ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) {
        fprintf(stderr, "ERROR: Не удалось установить скорость SPI\n");
        exit(1);
    }

    uint8_t bits = 8;
    if (ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0) {
        fprintf(stderr, "WARNING: Не удалось установить размер слова\n");
    }
}

// Установка режима SPI
void set_spi_mode(uint8_t mode) {
    if (spi_fd < 0) {
        fprintf(stderr, "SPI не инициализирован!\n");
        return;
    }


    if (ioctl(spi_fd, SPI_IOC_WR_MODE, &mode) < 0) {
        perror("Ошибка смены режима SPI");
        return;
    }

    uint8_t read_mode;
    if (ioctl(spi_fd, SPI_IOC_RD_MODE, &read_mode) < 0) {
        perror("Ошибка чтения режима SPI");
        return;
    }

    if (read_mode != mode) {
        fprintf(stderr, "Режим не изменился! Текущий: %d\n", read_mode);
    } else {
        current_spi_mode = mode;
    }
}

// Отправка данных по SPI
void spi_writebytes(uint8_t *data, int len) {
    struct spi_ioc_transfer spi = {
        .tx_buf = (unsigned long)data,
        .rx_buf = 0,
        .len = len,
        .delay_usecs = 0,
        .speed_hz = 0,
        .bits_per_word = 8,
        .cs_change = 0
    };

    digitalWrite(CS_PIN, LOW);
    //usleep(10);


    if (ioctl(spi_fd, SPI_IOC_MESSAGE(1), &spi) < 0) {
        fprintf(stderr, "ERROR: Ошибка передачи SPI\n");
    }

    // digitalWrite(CS_PIN, HIGH);
    // usleep(10);
}

// Чтение данных по SPI
void spi_readbytes(uint8_t *data, int len) {
    uint8_t *tx_buf = malloc(len);
    uint8_t *rx_buf = malloc(len);
    
    if (!tx_buf || !rx_buf) {
        perror("Ошибка выделения памяти");
        free(tx_buf);
        free(rx_buf);
        return;
    }

    memset(tx_buf, 0xFF, len);
    memset(rx_buf, 0x00, len);

    struct spi_ioc_transfer spi = {
        .tx_buf = (uintptr_t)tx_buf,
        .rx_buf = (uintptr_t)rx_buf,
        .len = len,
        .delay_usecs = 10,
        .speed_hz = 0,
        .bits_per_word = 8,
        .cs_change = 0
    };

    // digitalWrite(CS_PIN, LOW);
    // usleep(10);

    int ret = ioctl(spi_fd, SPI_IOC_MESSAGE(1), &spi);
    if (ret < 0) {
        perror("Ошибка SPI чтения");
    } else if (ret != len) {
    } else {
        memcpy(data, rx_buf, len);
    }

    digitalWrite(CS_PIN, HIGH);
    free(tx_buf);
    free(rx_buf);
}

// Функции из Python класса SPI_send

// Инициализация MVM DAC
void mvm_dac_init() {
    uint8_t data[] = {0x80, 0x0D};
    spi_writebytes(data, sizeof(data));
}

// Установка значения MVM DAC
void mvm_dac(uint16_t bitvalue, uint8_t channel) {
    uint8_t values[2];
    values[0] = (channel << 4) | (bitvalue >> 8);
    values[1] = bitvalue & 0xFF;
    spi_writebytes(values, sizeof(values));
}

// Установка значения WR DAC
void wr_dac(uint16_t bitvalue) {
    uint8_t values[2];
    values[0] = 0b00110000 | (bitvalue >> 8);
    values[1] = bitvalue & 0xFF;
    spi_writebytes(values, sizeof(values));
}

// Чтение ADC
uint16_t adc_read() {
    uint8_t res[2] = {0};
    spi_readbytes(res, sizeof(res));
    
    // Отправка нулей после чтения, как в Python коде
    uint8_t zeros[] = {0x00, 0x00};
    spi_writebytes(zeros, sizeof(zeros));
    
    return ((res[0] << 6) | (res[1] >> 2));
}

void key_set_MVM_on_mask(uint8_t mask) {
    spi_writebytes(&mask, 1);
}
// Включение ключей MVM
void key_set_MVM_on() {
    uint8_t value = 0xff;
    spi_writebytes(&value, 1);
}

// Выключение ключей MVM
void key_set_MVM_off() {
    uint8_t value = 0x00;
    spi_writebytes(&value, 1);
}

// Включение power down для MVM DAC
void mwm_dac_pd_on() {
    uint8_t values[] = {0b11000000, 0xFF};
    spi_writebytes(values, sizeof(values));
}

// Выключение power down для MVM DAC
void mwm_dac_pd_off() {
    uint8_t values[] = {0b11000000, 0x00};
    spi_writebytes(values, sizeof(values));
}

// Закрытие SPI
void close_spi() {
    if (spi_fd >= 0) {
        close(spi_fd);
    }
}

// Тестовая функция (аналог if __name__ == "__main__")
// int main() {
//     printf("\n===== Инициализация SPI =====\n");
//     init_spi(SPI_BUS, SPI_CHANNEL, 0, SPI_SPEED);
    
//     // Тест из Python: test.mvm_dac(675, 3); test.mvm_dac(0, 0);
//     printf("\n===== Тест MVM DAC =====\n");
//     mvm_dac(675, 3);
//     mvm_dac(0, 0);
    
//     close_spi();
//     return 0;
// }