from setuptools import setup, Extension

module = Extension(
    'gpio',
    sources=['gpio_module.c'],
    libraries=['gpiod'],  # Указываем линковку с libgpiod
)

setup(
    name='gpio_с',
    version='0.1',
    description='GPIO модуль для Raspberry Pi на С',
    ext_modules=[module],
)