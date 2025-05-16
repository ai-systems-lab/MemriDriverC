#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

int main() {
    int fd;
    uint8_t tx_data[] = {0x55, 0xAA};
    uint8_t rx_data[2] = {0};

    // Открываем устройство SPI
    fd = open("/dev/spidev0.0", O_RDWR);
    if (fd < 0) {
        perror("Ошибка открытия SPI устройства");
        return 1;
    }

    // Настраиваем режим SPI (Mode 0)
    uint8_t mode = SPI_MODE_0;
    ioctl(fd, SPI_IOC_WR_MODE, &mode);

    // Настраиваем скорость (1 МГц)
    uint32_t speed = 1000000;
    ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);

    // Структура для передачи данных
    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)tx_data,
        .rx_buf = (unsigned long)rx_data,
        .len = sizeof(tx_data),
        .delay_usecs = 0,
        .speed_hz = speed,
        .bits_per_word = 8,
    };

    // Отправляем данные
    if (ioctl(fd, SPI_IOC_MESSAGE(1), &tr) < 0) {
        perror("Ошибка передачи SPI");
        close(fd);
        return 1;
    }

    printf("Отправлено: 0x%02X 0x%02X\n", tx_data[0], tx_data[1]);
    printf("Принято:    0x%02X 0x%02X\n", rx_data[0], rx_data[1]);

    close(fd);
    return 0;
}// gcc spi_test.c -o spi_test
//sudo ./spi_test