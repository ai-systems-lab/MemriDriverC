import spidev
import time

# Настройка SPI
spi = spidev.SpiDev()
spi.open(0, 0)  # SPI0, канал 0
spi.max_speed_hz = 1000000  # 1 MHz
spi.mode = 0b00  # Режим 0
spi.lsbfirst = False  # MSB first

# GPIO для CS (необязательно, но можно использовать)
CS_PIN = 17
try:
    import RPi.GPIO as GPIO
    GPIO.setmode(GPIO.BCM)
    GPIO.setup(CS_PIN, GPIO.OUT)
    use_gpio = True
except:
    use_gpio = False
    print("GPIO не доступен, используем аппаратный CS")

def send_receive(data, receive_len=2):
    if use_gpio:
        GPIO.output(CS_PIN, GPIO.LOW)
        time.sleep(0.0001)  # 100 мкс
    
    # Отправка и прием
    response = spi.xfer2([data] + [0xFF]*receive_len)
    
    if use_gpio:
        time.sleep(0.0001)
        GPIO.output(CS_PIN, GPIO.HIGH)
    
    # Первый байт - отправленный, остальные - принятые
    return response[1:] if len(response) > 1 else []

# Тест
try:
    print("Тест SPI:")
    print("Отправляем 0x20...")
    received = send_receive(0x20)
    
    if received:
        print(f"Получено {len(received)} байт:")
        for i, byte in enumerate(received):
            print(f"Байт {i}: 0x{byte:02X} (DEC: {byte}, BIN: {bin(byte)})")
    else:
        print("Нет полученных данных!")
        
except Exception as e:
    print(f"Ошибка: {str(e)}")
finally:
    spi.close()
    if use_gpio:
        GPIO.cleanup()