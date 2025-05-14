/**
 * Программа для работы с SPI на Raspberry Pi с поддержкой всех 4 режимов
 * Подробные комментарии объясняют каждую часть кода
 */

// Подключаем необходимые библиотеки
#include <wiringPi.h>          // Для работы с GPIO (управление CS-пином)
#include <linux/spi/spidev.h>  // Для низкоуровневого управления SPI через ioctl
#include <fcntl.h>             // Для функции open()
#include <sys/ioctl.h>         // Для функции ioctl()
#include <stdio.h>             // Для функций ввода/вывода (printf)
#include <stdint.h>            // Для стандартных типов данных (uint8_t)
#include <unistd.h>            // Для usleep()
#include <string.h>            // Для memset()

// Определяем константы
#define SPI_DEVICE "/dev/spidev0.0"  // Путь к SPI-устройству (канал 0)
#define CS_PIN 17                    // GPIO17 для Chip Select (CS)
#define DEFAULT_SPI_SPEED 1000000    // Скорость по умолчанию (1 МГц)

// Глобальная переменная для файлового дескриптора SPI
int spi_fd;

/**
 * Функция инициализации SPI
 * @param mode - режим SPI (0-3)
 * @param speed - скорость SPI в Гц
 */
void init_spi(int mode, int speed) {
    // 1. Инициализация библиотеки wiringPi
    // wiringPiSetupGpio() использует нумерацию GPIO Broadcom
    if (wiringPiSetupGpio() == -1) {
        printf("[ОШИБКА] Не удалось инициализировать wiringPi\n");
        return;
    }

    // 2. Настройка CS-пина (Chip Select)
    // CS обычно активен в LOW, поэтому изначально устанавливаем HIGH
    pinMode(CS_PIN, OUTPUT);          // Настраиваем пин как выход
    digitalWrite(CS_PIN, HIGH);       // Устанавливаем HIGH (неактивное состояние)
    printf("CS-пин (GPIO%d) настроен как выход\n", CS_PIN);

    // 3. Открываем SPI-устройство
    // O_RDWR - открываем для чтения и записи
    spi_fd = open(SPI_DEVICE, O_RDWR);
    if (spi_fd < 0) {
        printf("[ОШИБКА] Не удалось открыть SPI устройство %s\n", SPI_DEVICE);
        return;
    }
    printf("SPI устройство %s успешно открыто\n", SPI_DEVICE);

    // 4. Устанавливаем режим SPI (CPOL и CPHA)
    // SPI_IOC_WR_MODE - команда для установки режима
    if (ioctl(spi_fd, SPI_IOC_WR_MODE, &mode) < 0) {
        printf("[ОШИБКА] Не удалось установить режим SPI\n");
        return;
    }
    printf("Режим SPI установлен: %d (CPOL=%d, CPHA=%d)\n", 
           mode, mode >> 1, mode & 0x01);

    // 5. Устанавливаем скорость передачи
    // SPI_IOC_WR_MAX_SPEED_HZ - команда для установки скорости
    if (ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) {
        printf("[ОШИБКА] Не удалось установить скорость SPI\n");
        return;
    }
    printf("Скорость SPI установлена: %d Гц (%.1f МГц)\n", 
           speed, speed / 1000000.0);

    // 6. (Опционально) Устанавливаем дополнительные параметры
    int bits_per_word = 8;  // Стандартный размер слова - 8 бит
    if (ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits_per_word) < 0) {
        printf("[ОШИБКА] Не удалось установить размер слова SPI\n");
        return;
    }
    printf("Размер слова SPI: %d бит\n", bits_per_word);

    // 7. (Опционально) Устанавливаем порядок битов (MSB first)
    int lsb_first = 0;  // 0 - MSB first (стандарт), 1 - LSB first
    if (ioctl(spi_fd, SPI_IOC_WR_LSB_FIRST, &lsb_first) < 0) {
        printf("[ОШИБКА] Не удалось установить порядок битов SPI\n");
        return;
    }
    printf("Порядок битов: %s\n", lsb_first ? "LSB first" : "MSB first");
}

/**
 * Функция для отправки данных по SPI
 * @param data - указатель на массив данных
 * @param len - количество байт для отправки
 */
void send_spi_data(uint8_t *data, int len) {
    // Структура для настройки параметров передачи SPI
    struct spi_ioc_transfer spi;
    
    // Обнуляем структуру перед использованием
    memset(&spi, 0, sizeof(spi));

    // Настраиваем параметры передачи:
    spi.tx_buf = (unsigned long)data;  // Буфер для передачи данных
    spi.rx_buf = (unsigned long)data;  // Буфер для приема данных (может быть NULL)
    spi.len = len;                     // Количество байт для передачи
    spi.delay_usecs = 10;              // Задержка после передачи (микросекунды)
    spi.speed_hz = 0;                  // 0 - использовать скорость по умолчанию
    spi.bits_per_word = 8;             // Размер слова (обычно 8 бит)
    spi.cs_change = 0;                 // Не изменять состояние CS после передачи

    // Активируем устройство - устанавливаем CS в LOW
    digitalWrite(CS_PIN, LOW);
    
    // Небольшая задержка для стабилизации сигнала
    usleep(10);

    // Выводим отправляемые данные для отладки
    printf("Отправка %d байт: ", len);
    for (int i = 0; i < len; i++) {
        printf("0x%02X ", data[i]);
    }
    printf("\n");

    // Отправляем данные через SPI
    // SPI_IOC_MESSAGE(1) - отправляем один пакет данных
    if (ioctl(spi_fd, SPI_IOC_MESSAGE(1), &spi) < 0) {
        printf("[ОШИБКА] Ошибка передачи данных по SPI\n");
    }

    // Деактивируем устройство - устанавливаем CS в HIGH
    digitalWrite(CS_PIN, HIGH);
    
    // Небольшая задержка между передачами
    usleep(10);
}

/**
 * Главная функция программы
 */
int main() {
    // Настройки SPI по умолчанию
    int spi_mode = 0;       // Режим SPI (0-3)
    int spi_speed = DEFAULT_SPI_SPEED;  // Скорость 1 МГц

    printf("\n=== Инициализация SPI ===\n");
    
    // Инициализируем SPI с выбранными параметрами
    init_spi(spi_mode, spi_speed);

    // Подготавливаем тестовые данные для отправки
    uint8_t test_data[2] = {0x12, 0x34};

    printf("\n=== Тестовая передача данных ===\n");
    
    // Отправляем данные
    send_spi_data(test_data, sizeof(test_data));

    printf("\n=== Завершение работы ===\n");
    
    // Закрываем SPI-устройство
    if (spi_fd >= 0) {
        close(spi_fd);
        printf("SPI устройство закрыто\n");
    }

    return 0;
}