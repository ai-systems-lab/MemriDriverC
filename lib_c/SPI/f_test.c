#include <wiringPi.h>
#include <linux/spi/spidev.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define SPI_BUS         0
#define SPI_CHANNEL     0
#define CS_PIN         17
#define GPIO_PIN       22
#define SPI_SPEED   1000000

int spi_fd;
int current_spi_mode;

void set_spi_mode(uint8_t mode) {
    if (spi_fd < 0) return;
    
    digitalWrite(CS_PIN, HIGH);
    usleep(10);
    
    if (ioctl(spi_fd, SPI_IOC_WR_MODE, &mode) < 0) {
        perror("Ошибка смены режима");
        return;
    }
    current_spi_mode = mode;
}

void init_spi(int bus, int channel, int mode, int speed) {
    char spi_device[20];
    snprintf(spi_device, sizeof(spi_device), "/dev/spidev%d.%d", bus, channel);

    if (wiringPiSetupGpio() == -1) {
        fprintf(stderr, "Ошибка инициализации GPIO\n");
        return;
    }

    pinMode(CS_PIN, OUTPUT);
    digitalWrite(CS_PIN, HIGH);
    pinMode(GPIO_PIN, OUTPUT);
    digitalWrite(GPIO_PIN, HIGH);

    spi_fd = open(spi_device, O_RDWR);
    if (spi_fd < 0) {
        perror("Ошибка открытия SPI");
        return;
    }

    set_spi_mode(mode);
    
    if (ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) {
        perror("Ошибка установки скорости");
    }
}

void spi_transfer(uint8_t *tx, uint8_t *rx, int len) {
    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)tx,
        .rx_buf = (unsigned long)rx,
        .len = len,
        .delay_usecs = 20,
        .speed_hz = 0,
        .bits_per_word = 8,
        .cs_change = 0
    };

    digitalWrite(CS_PIN, LOW);
    usleep(10);
    
    if (ioctl(spi_fd, SPI_IOC_MESSAGE(1), &tr) < 0) {
        perror("Ошибка передачи");
    }
    
    digitalWrite(CS_PIN, HIGH);
    usleep(10);
}

void test_all_modes() {
    uint8_t tx_cmd = 0b00100000;  // Тестовая команда
    uint8_t rx_data[4] = {0};
    
    for (int mode = 0; mode < 4; mode++) {
        set_spi_mode(mode);
        printf("\n=== Тестирование режима %d (CPOL=%d, CPHA=%d) ===\n",
               mode, (mode >> 1) & 1, mode & 1);
        
        // Отправка команды
        spi_transfer(&tx_cmd, rx_data, 1);
        usleep(1000);
        
        // Чтение ответа
        memset(rx_data, 0, sizeof(rx_data));
        spi_transfer(NULL, rx_data, sizeof(rx_data));
        
        printf("Ответ: ");
        for (int i = 0; i < sizeof(rx_data); i++) {
            printf("0x%02X ", rx_data[i]);
        }
        printf("\n");
        
        usleep(50000);  // Пауза между режимами
    }
}

int main() {
    printf("===== Всесторонняя диагностика SPI =====\n");
    init_spi(SPI_BUS, SPI_CHANNEL, 0, SPI_SPEED);
    
    printf("\nТестирование всех режимов SPI:\n");
    test_all_modes();
    
    close(spi_fd);
    return 0;
}