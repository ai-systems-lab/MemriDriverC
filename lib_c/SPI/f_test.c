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

void send_spi_data(uint8_t *data, int len) {
    struct spi_ioc_transfer spi = {
        .tx_buf = (unsigned long)data,
        .rx_buf = 0,
        .len = len,
        .delay_usecs = 0, // задержка после отправки 
        .speed_hz = 0,    // Используем установленную скорость
        .bits_per_word = 8,
        .cs_change = 0    // Не изменять CS между передачами
    };

    // Активируем устройство
    digitalWrite(CS_PIN, LOW);
    usleep(10);

    // Выводим отправляемые данные
    printf("Отправка %d байт в режиме %d: [", len, current_spi_mode);
    for (int i = 0; i < len; i++) {
        printf("0x%02X%s", data[i], (i < len-1) ? ", " : "");
    }
    printf("]\n");

    // Отправляем данные
    if (ioctl(spi_fd, SPI_IOC_MESSAGE(1), &spi) < 0) {
        fprintf(stderr, "ERROR: Ошибка передачи SPI\n");
    }

    // Деактивируем устройство
    

    digitalWrite(GPIO_PIN, LOW);
    usleep(10);
    digitalWrite(GPIO_PIN, HIGH);
    //usleep(10);
}


/**
* Альтернативная реализация приема данных по SPI
* с пошаговым чтением и улучшенным управлением CS
* @param data - буфер для приема данных
* @param len - количество байт для чтения
*/
void receive_spi_data(uint8_t *data, int len) {
   if (spi_fd < 0) {
       fprintf(stderr, "SPI не инициализирован!\n");
       return;
   }

   
   
   printf("\n=== Начало чтения ===\n");
   
   for (int i = 0; i < len; i++) {
       uint8_t dummy_tx = 0xFF;  // Байт для отправки при чтении
       uint8_t rx_byte = 0;
       
       struct spi_ioc_transfer spi = {
           .tx_buf = (unsigned long)&dummy_tx,
           .rx_buf = (unsigned long)&rx_byte,
           .len = 1,
           .delay_usecs = 10,
           .speed_hz = 0,
           .bits_per_word = 8,
           .cs_change = 0  
       };

       // Выполняем передачу для одного байта
       int ret = ioctl(spi_fd, SPI_IOC_MESSAGE(1), &spi);
       
       if (ret < 0) {
           perror("Ошибка чтения байта");
           printf("Ошибка при чтении байта %d\n", i);
           data[i] = 0xFF;  // Заполняем мусором при ошибке
       } else {
           data[i] = rx_byte;
           printf("Байт %d: 0x%02X (DEC: %3d, BIN: ", i, rx_byte, rx_byte);
           
           // Вывод в бинарном виде
           for (int j = 7; j >= 0; j--) {
               printf("%d", (rx_byte >> j) & 0x01);
           }
           printf(")\n");
       }
       
       usleep(10);  // Пауза между байтами
   }
   
   // Деактивируем устройство
   digitalWrite(CS_PIN, HIGH);
   usleep(10);  // Пауза после деактивации
   
   printf("=== Прочитано %d байт ===\n\n", len);
}

void test_all_modes() {
    uint8_t tx_cmd = 0b00100000;  // Тестовая команда
    uint8_t rx_data[2] = {0};
    
    for (int mode = 0; mode < 4; mode++) {
        set_spi_mode(mode);
        printf("\n=== Тестирование режима %d (CPOL=%d, CPHA=%d) ===\n",
               mode, (mode >> 1) & 1, mode & 1);
        
        // Отправка команды
        send_spi_data(&tx_cmd, 1);
        usleep(1000);
        
        // Чтение ответа
        memset(rx_data, 0, sizeof(rx_data));
        receive_spi_data(rx_data, sizeof(rx_data));
        
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