from setuptools import setup, Extension, find_packages
import os

# Проверяем, установлен ли libgpiod
def check_libgpiod():
    try:
        from subprocess import check_output
        check_output(['pkg-config', '--exists', 'libgpiod'])
        return True
    except:
        return False

if not check_libgpiod():
    raise RuntimeError("libgpiod not found. Please install it first: 'sudo apt-get install libgpiod-dev'")

module = Extension(
    'gpio_c._gpio',  # Имя модуля (будет доступно как gpio_c._gpio)
    sources=['gpio_c/gpio_module.c'],
    libraries=['gpiod'],
    define_macros=[('PY_SSIZE_T_CLEAN', None)],
)

setup(
    name='gpio_c',
    version='1.0.0',
    description='GPIO модуль для Raspberry Pi на С',
    long_description=open('README.md').read(),
    long_description_content_type='text/markdown',
    author='Kirill Kolokolov',
    author_email='kirillkolokolov@bk.ru',
    url='https://github.com/KiriusXd/kurs_neuro_C',
    packages=find_packages(),
    ext_modules=[module],
    python_requires='>=3.6',
    classifiers=[
        'Development Status :: 4 - Beta',
        'Intended Audience :: Developers',
        'License :: OSI Approved :: MIT License',
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: 3.6',
        'Programming Language :: Python :: 3.7',
        'Programming Language :: Python :: 3.8',
        'Programming Language :: Python :: 3.9',
        'Operating System :: POSIX :: Linux',
    ],
    zip_safe=False,
)