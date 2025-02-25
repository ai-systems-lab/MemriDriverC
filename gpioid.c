#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <gpiod.h>

#define LED_GPIO 17  // Измените номер GPIO при необходимости

static int running = 1;

void handle_sigint(int signum) {
    running = 0;
}

int main(void) {
    struct gpiod_chip *chip;
    struct gpiod_line *line;
    int ret;

    // Обработка сигнала Ctrl+C для корректного завершения программы
    signal(SIGINT, handle_sigint);

    // Открываем чип по указанному пути
    chip = gpiod_chip_open("/dev/gpiomem4");
    if (!chip) {
        perror("Ошибка открытия чипа");
        exit(EXIT_FAILURE);
    }

    // Получаем линию GPIO, к которой подключен светодиод
    line = gpiod_chip_get_line(chip, LED_GPIO);
    if (!line) {
        perror("Ошибка получения линии");
        gpiod_chip_close(chip);
        exit(EXIT_FAILURE);
    }

    // Запрашиваем линию на вывод с начальным уровнем 0 (светодиод выключен)
    ret = gpiod_line_request_output(line, "led_toggle", 0);
    if (ret < 0) {
        perror("Ошибка запроса линии на вывод");
        gpiod_chip_close(chip);
        exit(EXIT_FAILURE);
    }

    printf("Мигаем светодиодом на GPIO %d. Нажмите Ctrl+C для выхода.\n", LED_GPIO);

    // Бесконечный цикл мигания (включение/выключение с задержкой 1 секунда)
    while (running) {
        // Включаем светодиод (устанавливаем значение 1)
        ret = gpiod_line_set_value(line, 1);
        if (ret < 0) {
            perror("Ошибка установки значения линии");
            break;
        }
        sleep(1);

        // Выключаем светодиод (устанавливаем значение 0)
        ret = gpiod_line_set_value(line, 0);
        if (ret < 0) {
            perror("Ошибка установки значения линии");
            break;
        }
        sleep(1);
    }

    // Освобождаем ресурсы
    gpiod_line_release(line);
    gpiod_chip_close(chip);

    printf("Завершение работы.\n");
    return 0;
}


//
//gcc -Wall -o led led.c -lgpiod
//
//
//
//
