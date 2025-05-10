Пошаговая инструкция
1. Установка зависимостей
bash

# Установка системных зависимостей (вне окружения)
sudo apt-get update
sudo apt-get install -y python3-venv python3-dev libgpiod-dev

2. Создание и активация окружения
bash

cd gpio_module
python3 -m venv .pkg_venv
source .pkg_venv/bin/activate

3. Установка инструментов сборки
bash

pip install wheel setuptools

4. Сборка и установка модуля
bash

python setup.py bdist_wheel
pip install dist/gpio_module-1.0-cp311-cp311-linux_aarch64.whl

5. Проверка установки
bash

# Проверьте, что модуль появился в окружении
ls .pkg_venv/lib/python3.11/site-packages/ | grep gpio
# Должно вывести: gpio.cpython-311-aarch64-linux-gnu.so

6. Запуск теста
bash

python test.py