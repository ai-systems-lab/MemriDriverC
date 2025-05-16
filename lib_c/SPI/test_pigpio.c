#include <pigpio.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

// Конфигурация
#define SPI_BUS      0      // 0 (SPI0) или 1 (SPI1)
#define SPI_CHANNEL  0      // Канал SPI (0 или 1)
#define CS_PIN       17     // GPIO для ручного управления CS
#define SPI_SPEED    1000000 // Скорость SPI (1 МГц)
#define SPI_MODE     0      // Режим SPI (0-3)

int spi_handle;

void init_spi() {
    if (gpioInitialise() < 0) {
        fprintf(stderr, "Ошибка инициализации pigpio\n");
        exit(1);
    }

    // Настройка CS вручную
    gpioSetMode(CS_PIN, PI_OUTPUT);
    gpioWrite(CS_PIN, 1);
    printf("Настроен CS на GPIO%d\n", CS_PIN);

    // Открываем SPI
    spi_handle = spiOpen(SPI_CHANNEL, SPI_SPEED, SPI_MODE);
    if (spi_handle < 0) {
        fprintf(stderr, "Ошибка открытия SPI: %s\n", spiGetErrorString(spi_handle));
        gpioTerminate();
        exit(1);
    }

    printf("SPI инициализирован. Дескриптор: %d\n", spi_handle);
}

void spi_transfer(uint8_t *tx_data, uint8_t *rx_data, int len) {
    gpioWrite(CS_PIN, 0);
    usleep(10);

    int ret = spiXfer(spi_handle, (char*)tx_data, (char*)rx_data, len);
    
    gpioWrite(CS_PIN, 1);

    if (ret != len) {
        fprintf(stderr, "Ошибка передачи SPI: %s\n", spiGetErrorString(ret));
        return;
    }

    printf("Передано %d байт. Принято: [", len);
    for (int i = 0; i < len; i++) {
        printf("0x%02X%s", rx_data[i], (i < len-1) ? ", " : "");
    }
    printf("]\n");
}

void test_loopback() {
    uint8_t tx_data[] = {0x55, 0xAA, 0x01, 0x80, 0xFF, 0x00};
    uint8_t rx_data[sizeof(tx_data)] = {0};

    printf("\n=== Loopback тест (MOSI-MISO замкнуты) ===\n");
    printf("Отправка: [");
    for (size_t i = 0; i < sizeof(tx_data); i++) {
        printf("0x%02X%s", tx_data[i], (i < sizeof(tx_data)-1) ? ", " : "");
    }
    printf("]\n");

    spi_transfer(tx_data, rx_data, sizeof(tx_data));

    // Проверка результатов
    int match = 1;
    for (size_t i = 0; i < sizeof(tx_data); i++) {
        if (tx_data[i] != rx_data[i]) {
            match = 0;
            break;
        }
    }

    if (match) {
        printf("Тест пройден: данные совпадают!\n");
    } else {
        printf("Ошибка: принятые данные не совпадают с отправленными!\n");
    }
}

int main() {
    printf("===== Тест SPI с библиотекой pigpio =====\n");
    
    init_spi();
    test_loopback();
    
    spiClose(spi_handle);
    gpioTerminate();
    
    return 0;
}
// 