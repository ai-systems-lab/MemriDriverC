#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <gpiod.h>
#include <errno.h>   // Added for errno
#include <string.h>  // Added for string functions

#define CHIP_PATH "/dev/gpiochip0" // Default chip path

static int running = 1;
static struct gpiod_chip *global_chip = NULL; // Global chip pointer

// Function to handle SIGINT (Ctrl+C)
void handle_sigint(int signum) {
    running = 0;
    printf("Caught signal %d, exiting...\n", signum);
}

// Function to initialize the GPIO chip
int gpio_init() {
    global_chip = gpiod_chip_open(CHIP_PATH);
    if (!global_chip) {
        perror("Ошибка открытия чипа: gpiod_chip_open");
        fprintf(stderr, "errno: %d, %s\n", errno, strerror(errno));
        return -1;
    }
    return 0;
}

// Function to deinitialize the GPIO chip
void gpio_deinit() {
    if (global_chip) {
        gpiod_chip_close(global_chip);
        global_chip = NULL; // Reset global chip pointer
    }
}

// Function to request and configure a GPIO line as output
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

    ret = gpiod_line_request_output(line, "led_toggle", 0); // Initial state 0 (off)
    if (ret < 0) {
        perror("Ошибка запроса линии на вывод: gpiod_line_request_output");
        fprintf(stderr, "errno: %d, %s\n", errno, strerror(errno));
        gpiod_line_release(line); // Release the line if setup fails
        return NULL;
    }

    return line;
}

// Function to set the state (value) of a GPIO line
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

// Function to release a GPIO line
void gpio_line_release(struct gpiod_line *line) {
    if (line) {
        gpiod_line_release(line);
    }
}

// Function to blink an LED on a specific GPIO pin
int blink_led(int gpio_pin, int blink_duration_seconds) {
    struct gpiod_line *line = gpio_line_setup(gpio_pin);
    if (!line) {
        return -1;
    }

    printf("Мигаем светодиодом на GPIO %d. Нажмите Ctrl+C для выхода.\n", gpio_pin);

    while (running) {
        if (gpio_line_set(line, 1) < 0) { // Turn on LED
            break;
        }
        sleep(blink_duration_seconds);

        if (gpio_line_set(line, 0) < 0) { // Turn off LED
            break;
        }
        sleep(blink_duration_seconds);
    }

    gpio_line_release(line);
    return 0;
}

int setup(int gpio_pin){
    struct gpiod_line *line = gpio_line_setup(gpio_pin);
    if (!line) {
        return -1;
    }

}

int main(void) {
    int led_gpio = 17; // Example GPIO pin
    int blink_duration = 0.5; //seconds

    // Register signal handler for graceful exit
    signal(SIGINT, handle_sigint);

    // Initialize the GPIO chip
    if (gpio_init() < 0) {
        return EXIT_FAILURE;
    }

    // Blink the LED
    blink_led(led_gpio, blink_duration);

    // Deinitialize the GPIO chip
    gpio_deinit();

    printf("Завершение работы.\n");
    return 0;
}