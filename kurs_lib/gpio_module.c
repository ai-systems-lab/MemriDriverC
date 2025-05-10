#include <Python.h>
#include <gpiod.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

#define CHIP_PATH "/dev/gpiochip0"
static struct gpiod_chip *global_chip = NULL;

// === Оригинальные функции из вашего кода ===
int gpio_init() {
    printf("Trying to open GPIO chip...\n");
    global_chip = gpiod_chip_open(CHIP_PATH);
    if (!global_chip) {
        fprintf(stderr, "Error opening chip: %s\n", strerror(errno));
        return -1;
    }
    printf("Successfully opened GPIO chip\n");
    return 0;
}

void gpio_deinit() {
    if (global_chip) {
        gpiod_chip_close(global_chip);
        global_chip = NULL;
    }
}

struct gpiod_line *gpio_line_setup(int gpio_pin) {
    if (!global_chip) {
        fprintf(stderr, "GPIO chip not initialized!\n");
        return NULL;
    }

    struct gpiod_line *line = gpiod_chip_get_line(global_chip, gpio_pin);
    if (!line) {
        fprintf(stderr, "Failed to get line %d: %s\n", gpio_pin, strerror(errno));
        return NULL;
    }

    // Запрашиваем линию как выход с начальным значением 0
    if (gpiod_line_request_output(line, "python_gpio", 0) < 0) {
        fprintf(stderr, "Failed to request line %d: %s\n", gpio_pin, strerror(errno));
        gpiod_line_release(line);
        return NULL;
    }

    printf("Line %d setup successfully: %p\n", gpio_pin, line);
    return line;
}

int gpio_line_set(struct gpiod_line *line, int value) {
    if (!line) {
        fprintf(stderr, "Line is NULL in gpio_line_set!\n");
        return -1;
    }

    printf("Attempting to set line %p to %d...\n", line, value);
    int ret = gpiod_line_set_value(line, value);
    if (ret < 0) {
        fprintf(stderr, "Failed to set line: %s\n", strerror(errno));
    } else {
        printf("Line set successfully\n");
    }
    return ret;
}

void gpio_line_release(struct gpiod_line *line) {
    if (line) gpiod_line_release(line);
}

// === Python-обёртки ===
static PyObject *py_gpio_init(PyObject *self, PyObject *args) {
    if (gpio_init() < 0) {
        PyErr_SetString(PyExc_RuntimeError, "Не удалось инициализировать GPIO");
        return NULL;
    }
    Py_RETURN_NONE;
}

static PyObject *py_gpio_deinit(PyObject *self, PyObject *args) {
    gpio_deinit();
    Py_RETURN_NONE;
}

static PyObject *py_gpio_setup(PyObject *self, PyObject *args) {
    int gpio_pin;
    if (!PyArg_ParseTuple(args, "i", &gpio_pin)) {
        return NULL;
    }

    struct gpiod_line *line = gpio_line_setup(gpio_pin);
    if (!line) {
        PyErr_SetString(PyExc_RuntimeError, "Не удалось настроить GPIO");
        return NULL;
    }

    // Возвращаем указатель на линию как Python-объект (в виде числа)
    return PyLong_FromVoidPtr(line);
}

static PyObject *py_gpio_set(PyObject *self, PyObject *args) {
    void *line_ptr;
    int value;
    if (!PyArg_ParseTuple(args, "pi", &line_ptr, &value)) {
        return NULL;
    }

    struct gpiod_line *line = (struct gpiod_line *)line_ptr;
    if (gpio_line_set(line, value) < 0) {
        PyErr_SetString(PyExc_RuntimeError, "Не удалось установить значение GPIO");
        return NULL;
    }

    Py_RETURN_NONE;
}

static PyObject *py_gpio_release(PyObject *self, PyObject *args) {
    void *line_ptr;
    if (!PyArg_ParseTuple(args, "p", &line_ptr)) {
        return NULL;
    }

    struct gpiod_line *line = (struct gpiod_line *)line_ptr;
    gpio_line_release(line);
    Py_RETURN_NONE;
}

// === Определение методов модуля ===
static PyMethodDef GpioMethods[] = {
    {"init", py_gpio_init, METH_NOARGS, "Инициализировать GPIO"},
    {"deinit", py_gpio_deinit, METH_NOARGS, "Деинициализировать GPIO"},
    {"setup", py_gpio_setup, METH_VARARGS, "Настроить GPIO (возвращает handle)"},
    {"set", py_gpio_set, METH_VARARGS, "Установить значение GPIO (0/1)"},
    {"release", py_gpio_release, METH_VARARGS, "Освободить GPIO"},
    {NULL, NULL, 0, NULL} // Маркер конца
};

// === Определение модуля ===
static struct PyModuleDef gpio_module = {
    PyModuleDef_HEAD_INIT,
    "gpio",      // Имя модуля
    NULL,        // Документация
    -1,          // Размер состояния модуля (-1 = глобальное состояние)
    GpioMethods  // Таблица методов
};

// === Инициализация модуля ===
PyMODINIT_FUNC PyInit_gpio(void) {
    return PyModule_Create(&gpio_module);
}