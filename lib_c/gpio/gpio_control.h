#ifndef GPIO_CONTROL_H
#define GPIO_CONTROL_H

#include <gpiod.h>

// Функция для обработки сигнала SIGINT (Ctrl+C)
void handle_sigint(int signum);

// Инициализация GPIO чипа
int gpio_init(void);

// Деинициализация GPIO чипа
void gpio_deinit(void);

// Настройка GPIO линии как выхода
struct gpiod_line *gpio_line_setup(int gpio_pin);

// Установка состояния GPIO линии
int gpio_line_set(struct gpiod_line *line, int value);

// Освобождение GPIO линии
void gpio_line_release(struct gpiod_line *line);

// Мигание светодиодом
int blink_led(int gpio_pin, int blink_duration_seconds);

#endif // GPIO_CONTROL_H