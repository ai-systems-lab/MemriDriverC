from setuptools import setup, Extension

module = Extension(
    'gpio',
    sources=['gpio_module.c'],
    libraries=['gpiod'],  # Указываем линковку с libgpiod
)

setup(
    name='gpio_с', # имя пакета
    version='0.1.0', # версия
    description='GPIO модуль для Raspberry Pi на С', # описание
    ext_modules=[module],
)