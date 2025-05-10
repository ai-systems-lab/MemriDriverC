import gpio as gpio
import time
print("Testing GPIO module...")
gpio.init()

line = gpio.setup(17)
print(f"GPIO 17 line: {line}")
while True:        
    gpio.set(line, 1)
    time.sleep(2)
    gpio.set(line, 0)
