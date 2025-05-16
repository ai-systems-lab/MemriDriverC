/**
 * Улучшенная реализация SPI для Raspberry Pi
 * С поддержкой полнодуплексной передачи, улучшенным логированием и обработкой ошибок
 */

 #include <wiringPi.h>
 #include <linux/spi/spidev.h>
 #include <fcntl.h>
 #include <sys/ioctl.h>
 #include <stdio.h>
 #include <stdint.h>
 #include <unistd.h>
 #include <string.h>
 #include <errno.h>
 
 // ========== КОНФИГУРАЦИЯ ==========
 #define SPI_BUS         0       // 0 (SPI0) или 1 (SPI1)
 #define SPI_CHANNEL     0       // 0 или 1 (канал SPI)
 #define CS_PIN         17       // GPIO для Chip Select
 #define GPIO_PIN       22       // LDAC 
 #define SPI_SPEED   1000000     // Скорость по умолчанию (1 МГц)
 #define SPI_MODE        0       // Режим SPI по умолчанию (0-3)
 #define SPI_DELAY_US    10      // Задержка между операциями (мкс)
 
 // ========== ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ ==========
 int spi_fd = -1;                // Файловый дескриптор SPI
 int current_spi_mode;           // Текущий режим SPI
 uint8_t current_bit_order;      // Текущий порядок бит
 uint32_t current_speed;         // Текущая скорость SPI
 
 /**
  * Вывод отладочной информации о SPI
  */
 void print_spi_debug_info() {
     if (spi_fd < 0) {
         printf("SPI не инициализирован\n");
         return;
     }
     
     printf("\n--- Текущие настройки SPI ---\n");
     printf("Файловый дескриптор: %d\n", spi_fd);
     printf("Режим: %d (CPOL=%d, CPHA=%d)\n", 
            current_spi_mode, (current_spi_mode >> 1) & 0x01, current_spi_mode & 0x01);
     printf("Порядок бит: %s\n", current_bit_order ? "LSB first" : "MSB first");
     printf("Скорость: %d Hz (%.1f MHz)\n", current_speed, current_speed/1000000.0);
     
     uint8_t bits;
     if (ioctl(spi_fd, SPI_IOC_RD_BITS_PER_WORD, &bits) == 0) {
         printf("Бит на слово: %d\n", bits);
     }
 }
 
 /**
  * Установка режима SPI
  * @param mode - новый режим SPI (0-3)
  * @return 0 при успехе, -1 при ошибке
  */
 int set_spi_mode(uint8_t mode) {
     if (spi_fd < 0) {
         fprintf(stderr, "SPI не инициализирован!\n");
         return -1;
     }
 
     digitalWrite(CS_PIN, HIGH);
     usleep(SPI_DELAY_US);
 
     if (ioctl(spi_fd, SPI_IOC_WR_MODE, &mode) < 0) {
         perror("Ошибка смены режима SPI");
         return -1;
     }
 
     uint8_t read_mode;
     if (ioctl(spi_fd, SPI_IOC_RD_MODE, &read_mode) < 0) {
         perror("Ошибка чтения режима SPI");
         return -1;
     }
 
     if (read_mode != mode) {
         fprintf(stderr, "Режим не изменился! Текущий: %d\n", read_mode);
         return -1;
     }
 
     current_spi_mode = mode;
     printf("Режим SPI изменен на %d (CPOL=%d, CPHA=%d)\n",
            mode, (mode >> 1) & 0x01, mode & 0x01);
     return 0;
 }
 
 /**
  * Инициализация SPI
  * @param bus     - номер шины SPI (0 или 1)
  * @param channel - номер канала (0 или 1)
  * @param mode    - режим SPI (0-3)
  * @param speed   - скорость в Гц
  * @return 0 при успехе, -1 при ошибке
  */
 int init_spi(int bus, int channel, int mode, int speed) {
     char spi_device[20];
     snprintf(spi_device, sizeof(spi_device), "/dev/spidev%d.%d", bus, channel);
 
     if (wiringPiSetupGpio() == -1) {
         fprintf(stderr, "ERROR: Не удалось инициализировать wiringPi\n");
         return -1;
     }
 
     pinMode(CS_PIN, OUTPUT);
     digitalWrite(CS_PIN, HIGH);
     pinMode(GPIO_PIN, OUTPUT);
     digitalWrite(GPIO_PIN, HIGH);
 
     spi_fd = open(spi_device, O_RDWR);
     if (spi_fd < 0) {
         fprintf(stderr, "ERROR: Не удалось открыть %s: %s\n", spi_device, strerror(errno));
         return -1;
     }
 
     if (set_spi_mode(mode) {
         close(spi_fd);
         spi_fd = -1;
         return -1;
     }
 
     if (ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) {
         fprintf(stderr, "ERROR: Не удалось установить скорость SPI\n");
         close(spi_fd);
         spi_fd = -1;
         return -1;
     }
     current_speed = speed;
 
     uint8_t bits = 8;
     if (ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0) {
         fprintf(stderr, "WARNING: Не удалось установить размер слова\n");
     }
 
     print_spi_debug_info();
     return 0;
 }
 
 /**
  * Установка порядка бит
  * @param lsb_first - 1 для LSB first, 0 для MSB first
  * @return 0 при успехе, -1 при ошибке
  */
 int set_spi_bit_order(uint8_t lsb_first) {
     if (spi_fd < 0) {
         fprintf(stderr, "SPI не инициализирован!\n");
         return -1;
     }
 
     if (ioctl(spi_fd, SPI_IOC_WR_LSB_FIRST, &lsb_first) < 0) {
         perror("Ошибка установки порядка бит");
         return -1;
     }
 
     uint8_t check;
     if (ioctl(spi_fd, SPI_IOC_RD_LSB_FIRST, &check) < 0) {
         perror("Ошибка чтения порядка бит");
         return -1;
     }
 
     if (check != lsb_first) {
         fprintf(stderr, "Порядок бит не изменился! Текущий: %s\n",
                 check ? "LSB first" : "MSB first");
         return -1;
     }
 
     current_bit_order = check;
     printf("Порядок бит установлен: %s\n",
            check ? "LSB first" : "MSB first");
     return 0;
 }
 
 /**
  * Полнодуплексная передача данных по SPI
  * @param tx_data - данные для отправки
  * @param rx_data - буфер для приема
  * @param len     - количество байт
  * @return количество переданных байт или -1 при ошибке
  */
 int spi_transfer(uint8_t *tx_data, uint8_t *rx_data, int len) {
     if (spi_fd < 0) {
         fprintf(stderr, "SPI не инициализирован!\n");
         return -1;
     }
 
     struct spi_ioc_transfer spi = {
<<<<<<< HEAD
         .tx_buf = (unsigned long)tx_data,
         .rx_buf = (unsigned long)rx_data,
=======
         .tx_buf = (unsigned long)data,
         .rx_buf = (unsigned long)data,
>>>>>>> parent of 7527843 (fix sent data)
         .len = len,
         .delay_usecs = SPI_DELAY_US,
         .speed_hz = current_speed,
         .bits_per_word = 8,
         .cs_change = 0
     };
 
     digitalWrite(CS_PIN, LOW);
 
     printf("Отправка %d байт: [", len);
     for (int i = 0; i < len; i++) {
         printf("0x%02X%s", tx_data[i], (i < len-1) ? ", " : "");
     }
     printf("]\n");
 
     int ret = ioctl(spi_fd, SPI_IOC_MESSAGE(1), &spi);
     if (ret < 0) {
         perror("Ошибка передачи SPI");
         digitalWrite(CS_PIN, HIGH);
         return -1;
     }
 
     digitalWrite(CS_PIN, HIGH);
     digitalWrite(GPIO_PIN, LOW);
     usleep(SPI_DELAY_US);
     digitalWrite(GPIO_PIN, HIGH);
 
     printf("Принято %d байт: [", len);
     for (int i = 0; i < len; i++) {
         printf("0x%02X%s", rx_data[i], (i < len-1) ? ", " : "");
     }
     printf("]\n");
 
     return len;
 }
 
 /**
  * Закрытие SPI
  */
 void close_spi() {
     if (spi_fd >= 0) {
         close(spi_fd);
         spi_fd = -1;
         printf("SPI устройство закрыто\n");
     }
 }
 
 int main() {
     printf("\n===== Инициализация SPI =====\n");
     if (init_spi(SPI_BUS, SPI_CHANNEL, SPI_MODE, SPI_SPEED)) {
         fprintf(stderr, "Ошибка инициализации SPI!\n");
         return 1;
     }
 
     if (set_spi_bit_order(0)) {
         fprintf(stderr, "Ошибка установки порядка бит!\n");
         close_spi();
         return 1;
     }
 
     uint8_t tx_data[] = {0xAA, 0x55, 0x01};  // Тестовые данные для отправки
     uint8_t rx_data[sizeof(tx_data)] = {0};   // Буфер для приема
     // 0b00100000
     printf("\n===== Тестовая передача =====\n");
     if (spi_transfer(tx_data, rx_data, sizeof(tx_data)) < 0) {
         fprintf(stderr, "Ошибка передачи данных!\n");
     }
 
     close_spi();
     return 0;
 }