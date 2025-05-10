from setuptools import setup, Extension

module = Extension(
    'gpio',  # Имя модуля для импорта (import gpio)
    sources=['gpio_module.c'],
    libraries=['gpiod'],
    extra_compile_args=["-O2"]  # Оптимизация
)

setup(
    name='gpio_module',
    version='1.0',
    description='GPIO C-модуль для Raspberry Pi',
    ext_modules=[module],
)