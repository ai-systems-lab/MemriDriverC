#include "gpio_control.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

#define CHIP_PATH "/dev/gpiochip0"

static int running = 1;
static struct gpiod_chip *global_chip = NULL;

void handle_sigint(int signum) {
    running = 0;
    printf("Caught signal %d, exiting...\n", signum);
}

int gpio_init() {
    global_chip = gpiod_chip_open(CHIP_PATH);
    if (!global_chip) {
        perror("Ошибка открытия чипа: gpiod_chip_open");
        fprintf(stderr, "errno: %d, %s\n", errno, strerror(errno));
        return -1;
    }
    return 0;
}

void gpio_deinit() {
    if (global_chip) {
        gpiod_chip_close(global_chip);
        global_chip = NULL;
    }
}

struct gpiod_line *gpio_line_setup(int gpio_pin) {
    struct gpiod_line *line;
    int ret;

    if (!global_chip) {
        fprintf(stderr, "Ошибка: Чип GPIO не инициализирован.\n");
        return NULL;
    }

    line = gpiod_chip_get_line(global_chip, gpio_pin);
    if (!line) {
        perror("Ошибка получения линии: gpiod_chip_get_line");
        fprintf(stderr, "errno: %d, %s\n", errno, strerror(errno));
        return NULL;
    }

    ret = gpiod_line_request_output(line, "led_toggle", 0);
    if (ret < 0) {
        perror("Ошибка запроса линии на вывод: gpiod_line_request_output");
        fprintf(stderr, "errno: %d, %s\n", errno, strerror(errno));
        gpiod_line_release(line);
        return NULL;
    }

    return line;
}

int gpio_line_set(struct gpiod_line *line, int value) {
    if (!line) {
        fprintf(stderr, "Ошибка: Линия GPIO не настроена.\n");
        return -1;
    }

    int ret = gpiod_line_set_value(line, value);
    if (ret < 0) {
        perror("Ошибка установки значения линии: gpiod_line_set_value");
        fprintf(stderr, "errno: %d, %s\n", errno, strerror(errno));
        return -1;
    }
    return 0;
}

void gpio_line_release(struct gpiod_line *line) {
    if (line) {
        gpiod_line_release(line);
    }
}

int blink_led(int gpio_pin, int blink_duration_seconds) {
    struct gpiod_line *line = gpio_line_setup(gpio_pin);
    if (!line) {
        return -1;
    }

    printf("Мигаем светодиодом на GPIO %d. Нажмите Ctrl+C для выхода.\n", gpio_pin);

    while (running) {
        if (gpio_line_set(line, 1) < 0) {
            break;
        }
        sleep(blink_duration_seconds);

        if (gpio_line_set(line, 0) < 0) {
            break;
        }
        sleep(blink_duration_seconds);
    }

    gpio_line_release(line);
    return 0;
}