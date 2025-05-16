#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include <wiringPi.h>

#define CS_PIN 17  // Номер GPIO (BCM)
#define MAX_BUF 256

int spi_fd = -1;
uint32_t spi_speed = 1000000;
uint8_t spi_mode = 0;

void cs_low() {
    digitalWrite(CS_PIN, LOW);
    usleep(10);
}

void cs_high() {
    usleep(10);
    digitalWrite(CS_PIN, HIGH);
}

int init_spi(const char *dev_path, uint8_t mode, uint32_t speed) {
    if (wiringPiSetupGpio() == -1) {
        fprintf(stderr, "Ошибка инициализации GPIO\n");
        return -1;
    }
    pinMode(CS_PIN, OUTPUT);
    cs_high();

    spi_fd = open(dev_path, O_RDWR);
    if (spi_fd < 0) {
        perror("Не удалось открыть SPI");
        return -1;
    }

    if (ioctl(spi_fd, SPI_IOC_WR_MODE, &mode) < 0) {
        perror("Не удалось установить режим SPI");
        return -1;
    }

    if (ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) {
        perror("Не удалось установить скорость SPI");
        return -1;
    }

    spi_mode = mode;
    spi_speed = speed;

    printf("SPI открыт (%s) в режиме %d, %u Гц\n", dev_path, mode, speed);
    return 0;
}

void hex_dump(const char *label, const uint8_t *data, int len) {
    printf("%s [%d байт]: ", label, len);
    for (int i = 0; i < len; i++)
        printf("0x%02X ", data[i]);
    printf("\n");
}

void spi_transfer(uint8_t *tx, uint8_t *rx, int len) {
    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)tx,
        .rx_buf = (unsigned long)rx,
        .len = len,
        .speed_hz = spi_speed,
        .delay_usecs = 0,
        .bits_per_word = 8,
    };

    cs_low();
    int ret = ioctl(spi_fd, SPI_IOC_MESSAGE(1), &tr);
    cs_high();

    if (ret < 1) {
        perror("Ошибка передачи SPI");
    }
}

int main() {
    char dev_path[32] = "/dev/spidev0.0";
    printf("Универсальный SPI-отладчик\n");

    printf("Устройство SPI [%s]: ", dev_path);
    fgets(dev_path, sizeof(dev_path), stdin);
    if (dev_path[0] == '\n') strcpy(dev_path, "/dev/spidev0.0");
    else dev_path[strcspn(dev_path, "\n")] = 0;

    printf("Режим SPI [0–3] (по умолчанию 0): ");
    char buf[8];
    fgets(buf, sizeof(buf), stdin);
    uint8_t mode = atoi(buf);

    printf("Скорость (Гц, по умолчанию 1000000): ");
    fgets(buf, sizeof(buf), stdin);
    uint32_t speed = atoi(buf);
    if (speed == 0) speed = 1000000;

    if (init_spi(dev_path, mode, speed) != 0) return 1;

    while (1) {
        printf("\nВведи HEX-байты через пробел (например: `A0 00 12`) или `exit`:\n> ");
        char input[1024];
        fgets(input, sizeof(input), stdin);
        if (strncmp(input, "exit", 4) == 0) break;

        uint8_t tx_buf[MAX_BUF] = {0};
        uint8_t rx_buf[MAX_BUF] = {0};
        int tx_len = 0;

        char *token = strtok(input, " ");
        while (token != NULL && tx_len < MAX_BUF) {
            unsigned int val;
            if (sscanf(token, "%x", &val) == 1) {
                tx_buf[tx_len++] = (uint8_t)val;
            }
            token = strtok(NULL, " ");
        }

        if (tx_len == 0) {
            printf("Ничего не отправлено.\n");
            continue;
        }

        spi_transfer(tx_buf, rx_buf, tx_len);

        hex_dump("TX", tx_buf, tx_len);
        hex_dump("RX", rx_buf, tx_len);
    }

    close(spi_fd);
    return 0;
}
// gcc -o spi_debugger spi_debugger.c -lwiringPi
