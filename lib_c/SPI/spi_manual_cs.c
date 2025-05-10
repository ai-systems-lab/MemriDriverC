#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#define SPI_CHANNEL 0       // SPI0 на Raspberry Pi
#define SPI_SPEED   1000000 // 1 МГц
#define CS_PIN      17      // GPIO17 для CS

void init_spi() {
    // Инициализация wiringPi
    if (wiringPiSetupGpio() == -1) {
        printf("Ошибка инициализации wiringPi\n");
        return;
    }

    // Настройка пина CS как выход
    pinMode(CS_PIN, OUTPUT);
    digitalWrite(CS_PIN, HIGH); // CS высокий (неактивен)

    // Инициализация SPI
    if (wiringPiSPISetup(SPI_CHANNEL, SPI_SPEED) < 0) {
        printf("Ошибка настройки SPI\n");
        return;
    }
}

void send_spi_data(uint8_t *data, int len) {
    // Активация CS (низкий уровень)
    digitalWrite(CS_PIN, LOW);
    usleep(10); // Небольшая задержка для стабилизации

    // Отправка данных
    wiringPiSPIDataRW(SPI_CHANNEL, data, len);

    // Деактивация CS (высокий уровень)
    digitalWrite(CS_PIN, HIGH);
}

int main() {
    // Инициализация
    init_spi();

    // Данные для отправки (2 байта)
    uint8_t data[2] = {0x12, 0x34};

    // Отправка данных
    printf("Отправка данных: 0x%02X 0x%02X\n", data[0], data[1]);
    send_spi_data(data, 2);

    return 0;
}