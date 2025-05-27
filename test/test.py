from python.rpi_modes import RPI_modes as RPi_modess
from mvmdriver_wrapper import MVMDriver
import time

if __name__ == "__main__":
    driver = MVMDriver()

    pdriver = RPi_modess()

    vDAC_mas = [100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,]
    # print(len(vDAC_mas))
    # # Тест mode_7
    # result, id = driver.mode_7(0, 0, 0, 0, 123, 0,3)
    # print(f"mode_7 result: {result}, id: {id}")

    start = time.time()
    for i in range(1000):
        result, id = driver.mode_7(500, 0, 0, 0, 123, 0,0)
    end = time.time()
    print(f"time 7 C result: {end-start}")

    start = time.time()
    for i in range(1000):
        result, id = pdriver.mode_7(500, 0, 0, 0, 123, 0,0)
    end = time.time()
    print(f"time 7  Python result: {end-start}")

    start = time.time()
    for i in range(1000):
        result, id = driver.mode_9(200, 123, 0,0)
    end = time.time()
    print(f"time 9  C result: {end-start}")

    start = time.time()
    for i in range(1000):
        result, id = pdriver.mode_9(200, 123, 0,0)
    end = time.time()
    print(f"time 9  Python result: {end-start}")


   
    
    start = time.time()
    for i in range(1000):
        result, id = driver.mode_mvm(vDAC_mas, 1, 0, 0, 0, 0, 123)
    end = time.time()
    print(f"time mvm  C result: {end-start}")

    start = time.time()
    for i in range(1000):
        result, id = pdriver.mode_mvm(vDAC_mas, 1, 0, 0, 0, 0, 123)
    end = time.time()
    print(f"time mvm  Python result: {end-start}")

    print(f"time:{start-end}")
    # # Тест mode_mvm
    # vDAC_mas = [100] * 32  # Пример массива
    # result, id = driver.mode_mvm(vDAC_mas, 1, 0, 0, 0, 0, 123)
    # print(f"mode_mvm result: {result}, id: {id}")