import gpio
gpio.init()
line = gpio.setup(17)
while True:
    gpio.set(line, 1)
    gpio.set(line, 0)