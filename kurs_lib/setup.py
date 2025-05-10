from setuptools import setup, Extension

module = Extension(
    'gpio',
    sources=['gpio_module.c'],
    libraries=['gpiod'],  # Указываем линковку с libgpiod
)

setup(
    name='gpio',
    version='1.0',
    description='GPIO модуль для Raspberry Pi',
    ext_modules=[module],
)