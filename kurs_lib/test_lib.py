import gpio
print("Initializing GPIO...")
gpio.init()

print("Setting up GPIO 17...")
line = gpio.setup(17)

print("Setting GPIO to 1...")
gpio.set(line, 1)

input("Press Enter to exit...")

print("Cleaning up...")
gpio.set(line, 0)
gpio.deinit()