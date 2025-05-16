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

// Конфигурация
#define SPI_BUS         0
#define SPI_CHANNEL     0
#define CS_PIN         17
#define SPI_SPEED   1000000
#define SPI_MODE        0

int spi_fd = -1;

// Инициализация SPI (аналогична предыдущему примеру)
int init_spi(int bus, int channel, int mode, int speed) {
    char spi_device[20];
    snprintf(spi_device, sizeof(spi_device), "/dev/spidev%d.%d", bus, channel);

    if (wiringPiSetupGpio() == -1) {
        fprintf(stderr, "Ошибка инициализации wiringPi\n");
        return -1;
    }

    pinMode(CS_PIN, OUTPUT);
    digitalWrite(CS_PIN, HIGH);

    spi_fd = open(spi_device, O_RDWR);
    if (spi_fd < 0) {
        fprintf(stderr, "Ошибка открытия %s: %s\n", spi_device, strerror(errno));
        return -1;
    }

    if (ioctl(spi_fd, SPI_IOC_WR_MODE, &mode) < 0) {
        fprintf(stderr, "Ошибка установки режима SPI: %s\n", strerror(errno));
        close(spi_fd);
        return -1;
    }

    if (ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) {
        fprintf(stderr, "Ошибка установки скорости SPI: %s\n", strerror(errno));
        close(spi_fd);
        return -1;
    }

    printf("SPI инициализировано: %s, режим %d, скорость %d Гц\n", spi_device, mode, speed);
    return 0;
}

// Отправка команды
int send_spi_command(uint8_t *cmd, int len) {
    struct spi_ioc_transfer spi = {
        .tx_buf = (uintptr_t)cmd,
        .rx_buf = 0,
        .len = len,
        .delay_usecs = 0,
        .speed_hz = 0,
        .bits_per_word = 8,
        .cs_change = 0
    };

    digitalWrite(CS_PIN, LOW);
    usleep(10);

    if (ioctl(spi_fd, SPI_IOC_MESSAGE(1), &spi) < 0) {
        fprintf(stderr, "Ошибка отправки команды: %s\n", strerror(errno));
        digitalWrite(CS_PIN, HIGH);
        return -1;
    }

    digitalWrite(CS_PIN, HIGH);
    usleep(10);
    return 0;
}

// Чтение данных после команды
int read_spi_data_after_command(uint8_t *cmd, int cmd_len, uint8_t *data, int data_len) {
    if (spi_fd < 0) {
        fprintf(stderr, "SPI не инициализирован\n");
        return -1;
    }

    // Отправляем команду
    if (send_spi_command(cmd, cmd_len) < 0) {
        return -1;
    }

    // Читаем данные
    uint8_t dummy_tx[data_len];
    memset(dummy_tx, 0xFF, data_len);

    struct spi_ioc_transfer spi = {
        .tx_buf = (uintptr_t)dummy_tx,
        .rx_buf = (uintptr_t)data,
        .len = data_len,
        .delay_usecs = 0,
        .speed_hz = 0,
        .bits_per_word = 8,
        .cs_change = 0
    };

    digitalWrite(CS_PIN, LOW);
    usleep(10);

    if (ioctl(spi_fd, SPI_IOC_MESSAGE(1), &spi) < 0) {
        fprintf(stderr, "Ошибка чтения SPI: %s\n", strerror(errno));
        digitalWrite(CS_PIN, HIGH);
        return -1;
    }

    digitalWrite(CS_PIN, HIGH);
    usleep(10);

    printf("Прочитано %d байт: ", data_len);
    for (int i = 0; i < data_len; i++) {
        printf("0x%02X ", data[i]);
    }
    printf("\n");

    return data_len;
}

// Закрытие SPI
void close_spi() {
    if (spi_fd >= 0) {
        close(spi_fd);
        spi_fd = -1;
        printf("SPI закрыто\n");
    }
}

int main() {
    if (init_spi(SPI_BUS, SPI_CHANNEL, SPI_MODE, SPI_SPEED) < 0) {
        return 1;
    }

    // Команда для устройства (пример)
    uint8_t command[] = {0b00100000}; // Команда чтения (зависит от устройства)
    uint8_t data[2] = {0};

    // Чтение после команды
    if (read_spi_data_after_command(command, sizeof(command), data, sizeof(data)) < 0) {
        close_spi();
        return 1;
    }

    close_spi();
    return 0;
}