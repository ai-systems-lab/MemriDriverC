/**
 * Полная реализация SPI для Raspberry Pi
 * С поддержкой всех 4 режимов и выбора канала через SPI_CHANNEL
 * Добавлена функция смены режима SPI без переинициализации
 */

 #include <wiringPi.h>
 #include <linux/spi/spidev.h>
 #include <fcntl.h>
 #include <sys/ioctl.h>
 #include <stdio.h>
 #include <stdint.h>
 #include <unistd.h>
 #include <string.h>
 
 // ========== КОНФИГУРАЦИЯ ==========
 #define SPI_BUS         0       // 0 (SPI0) или 1 (SPI1)
 #define SPI_CHANNEL     0       // 0 или 1 (канал SPI)
 #define CS_PIN         17       // GPIO для Chip Select
 #define GPIO_PIN       22       // ldac 
 #define SPI_SPEED   1000000     // Скорость по умолчанию (1 МГц)
 #define SPI_MODE        0       // Режим SPI по умолчанию (0-3)
 
 // ========== ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ ==========
 int spi_fd;  // Файловый дескриптор SPI
 int current_spi_mode; // Текущий режим SPI
 uint8_t current_bit_order;
 
 /**
  * Функция изменения режима SPI без переинициализации
  * @param mode - новый режим SPI (0-3)
  */
 void set_spi_mode(uint8_t mode) {
     if (spi_fd < 0) {
         fprintf(stderr, "SPI не инициализирован!\n");
         return;
     }
 
     // Убедимся, что CS неактивен перед сменой режима
     digitalWrite(CS_PIN, HIGH);
     usleep(10); // Короткая пауза
 
     if (ioctl(spi_fd, SPI_IOC_WR_MODE, &mode) < 0) {
         perror("Ошибка смены режима SPI");
         return;
     }
 
     // Проверяем, что режим установился
     uint8_t read_mode;
     if (ioctl(spi_fd, SPI_IOC_RD_MODE, &read_mode) < 0) {
         perror("Ошибка чтения режима SPI");
         return;
     }
 
     if (read_mode != mode) {
         fprintf(stderr, "Режим не изменился! Текущий: %d\n", read_mode);
     } else {
         current_spi_mode = mode;
         printf("Режим SPI изменен на %d (CPOL=%d, CPHA=%d)\n",
                mode, (mode >> 1) & 0x01, mode & 0x01);
     }
 }
 
 /**
  * Инициализация SPI
  * @param bus     - номер шины SPI (0 или 1)
  * @param channel - номер канала (0 или 1)
  * @param mode    - режим SPI (0-3)
  * @param speed   - скорость в Гц
  */
 void init_spi(int bus, int channel, int mode, int speed) {
     // 1. Формируем путь к устройству SPI
     char spi_device[20];
     snprintf(spi_device, sizeof(spi_device), "/dev/spidev%d.%d", bus, channel);
 
     // 2. Инициализация GPIO
     if (wiringPiSetupGpio() == -1) {
         fprintf(stderr, "ERROR: Не удалось инициализировать wiringPi\n");
         return;
     }
 
     // 3. Настройка CS-пина
     pinMode(CS_PIN, OUTPUT);
     digitalWrite(CS_PIN, HIGH);  // Деактивируем CS
     //pinMode(10, OUTPUT);
     //pinMode
     pinMode(GPIO_PIN, OUTPUT);
     digitalWrite(GPIO_PIN, HIGH);
     printf("Настроен CS на GPIO%d\n", CS_PIN);
 
     // 4. Открываем устройство SPI
     spi_fd = open(spi_device, O_RDWR);
     if (spi_fd < 0) {
         fprintf(stderr, "ERROR: Не удалось открыть %s\n", spi_device);
         return;
     }
     printf("Открыто SPI устройство: %s\n", spi_device);
 
     // 5. Устанавливаем режим SPI
     if (ioctl(spi_fd, SPI_IOC_WR_MODE, &mode) < 0) {
         fprintf(stderr, "ERROR: Не удалось установить режим SPI\n");
         return;
     }
     current_spi_mode = mode;
     printf("Режим SPI: %d (CPOL=%d, CPHA=%d)\n", 
            mode, (mode >> 1) & 0x01, mode & 0x01);
 
     // 6. Устанавливаем скорость
     if (ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) {
         fprintf(stderr, "ERROR: Не удалось установить скорость SPI\n");
         return;
     }
     printf("Скорость SPI: %d Hz (%.1f MHz)\n", speed, speed/1000000.0);
 
     // 7. Дополнительные настройки
     uint8_t bits = 8;  // 8 бит на слово
     if (ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0) {
         fprintf(stderr, "WARNING: Не удалось установить размер слова\n");
     }
 }

 /**
 * Устанавливает порядок бит (LSB first / MSB first)
 * @param lsb_first - 1 для LSB first, 0 для MSB first
 */
void set_spi_bit_order(uint8_t lsb_first) {
    if (spi_fd < 0) {
        fprintf(stderr, "SPI не инициализирован!\n");
        return;
    }

    if (ioctl(spi_fd, SPI_IOC_WR_LSB_FIRST, &lsb_first) < 0) {
        perror("Ошибка установки порядка бит");
        return;
    }

    // Проверка
    uint8_t check;
    if (ioctl(spi_fd, SPI_IOC_RD_LSB_FIRST, &check) < 0) {
        perror("Ошибка чтения порядка бит");
        return;
    }

    if (check != lsb_first) {
        fprintf(stderr, "Порядок бит не изменился! Текущий: %s\n",
                check ? "LSB first" : "MSB first");
    } else {
        current_bit_order = check;
        printf("Порядок бит установлен: %s\n",
               check ? "LSB first" : "MSB first");
    }
}
 
 /**
  * Отправка данных по SPI
  * @param data - указатель на данные
  * @param len  - количество байт
  */
 void send_spi_data(uint8_t *data, int len) {
     struct spi_ioc_transfer spi = {
         .tx_buf = (unsigned long)data,
         .rx_buf = (unsigned long)data,
         .len = len,
         .delay_usecs = 0, // задержка после отправки 
         .speed_hz = 0,    // Используем установленную скорость
         .bits_per_word = 8,
         .cs_change = 0    // Не изменять CS между передачами
     };
 
     // Активируем устройство
     digitalWrite(CS_PIN, LOW);
 
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
     digitalWrite(CS_PIN, HIGH);

     digitalWrite(GPIO_PIN, LOW);
     usleep(10);
     digitalWrite(GPIO_PIN, HIGH);
     //usleep(10);
 }

 /**
 * Прием данных по SPI
 * @param data - указатель на буфер для приема данных
 * @param len  - количество байт для приема
 */
void receive_spi_data(uint8_t *data, int len) {
    struct spi_ioc_transfer spi = {
        .tx_buf = 0,            // Не отправляем данные
        .rx_buf = (unsigned long)data,
        .len = len,
        .delay_usecs = 0,
        .speed_hz = 0,          // Используем установленную скорость
        .bits_per_word = 8,
        .cs_change = 0          // Не изменять CS между передачами
    };

    // Активируем устройство
    digitalWrite(CS_PIN, LOW);

    // Принимаем данные
    if (ioctl(spi_fd, SPI_IOC_MESSAGE(1), &spi) < 0) {
        fprintf(stderr, "ERROR: Ошибка приема SPI\n");
    }

    // Деактивируем устройство
    digitalWrite(CS_PIN, HIGH);

    // Выводим принятые данные
    printf("Принято %d байт в режиме %d: [", len, current_spi_mode);
    for (int i = 0; i < len; i++) {
        printf("0x%02X%s", data[i], (i < len-1) ? ", " : "");
    }
    printf("]\n");
}
 
 /**
  * Закрытие SPI
  */
 void close_spi() {
     if (spi_fd >= 0) {
         close(spi_fd);
         printf("SPI устройство закрыто\n");
     }
 }
 
 /**
  * Главная функция
  */
 int main() {
      printf("\n===== Инициализация SPI =====\n");
      init_spi(SPI_BUS, SPI_CHANNEL, SPI_MODE, SPI_SPEED);
      set_spi_bit_order(1); // 0 - msb -1 - lsb
      // Тестовые данные
      uint8_t test_data[] = {0b00000000, 0b00111110};
      uint8_t test_data2[] = {0x56, 0x78};
      //100 0000000 00 0000000 00 
      printf("\n===== Тестовая передача =====\n");
      send_spi_data(test_data, sizeof(test_data));
 
    //   printf("\n===== Смена режима на 2 =====\n");
    //   set_spi_mode(2);  // Меняем режим без переинициализации
      
    //   printf("\n===== Тестовая передача в новом режиме =====\n");
    //   send_spi_data(test_data2, sizeof(test_data2));
 
    //   printf("\n===== Возврат в режим 0 =====\n");
    //   set_spi_mode(0);
      
    //   printf("\n===== Тестовая передача =====\n");
    //   send_spi_data(test_data, sizeof(test_data));
 
    //   printf("\n===== Завершение работы =====\n");
    //   close_spi();
 
      return 0;
 }

 //===== Инициализация SPI =====
// Настроен CS на GPIO17
// Открыто SPI устройство: /dev/spidev0.0
// Режим SPI: 0 (CPOL=0, CPHA=0)
// Скорость SPI: 1000000 Hz (1.0 MHz)
// Ошибка установки порядка бит: Invalid argument

// ===== Тестовая передача =====
// Отправка 2 байт в режиме 0: [0x00, 0x3E]