from ctypes import *
import os
import sys
from rpi_modes import RPI_modes as RPi_modess
import time
# Загружаем разделяемую библиотеку
lib_path = os.path.join(os.path.dirname(__file__), 'libmvmdriver.so')
mvmdriver = CDLL(lib_path)

sys.stderr = sys.stdout

# Определяем структуры
class RegControl595(Structure):
    _fields_ = [("main_bytes", c_ubyte * 2)]

class SPI_send(Structure):
    _fields_ = [("spi_fd", c_int),
                ("current_spi_mode", c_ubyte)]

class RPI_modes(Structure):
    _fields_ = [("mvm_spi", SPI_send),
                ("reg", RegControl595)]

# Привязки функций
mvmdriver.RPI_modes_init.argtypes = [POINTER(RPI_modes)]
mvmdriver.RPI_modes_init.restype = None

mvmdriver.mode_7.argtypes = [POINTER(RPI_modes), c_uint16, c_uint16, c_uint16, 
                            c_uint8, c_uint16, c_uint8, c_uint8, POINTER(c_uint16)]
mvmdriver.mode_7.restype = None

mvmdriver.mode_9.argtypes = [POINTER(RPI_modes), c_uint16, c_uint16, c_uint8, c_uint8, 
                            POINTER(c_uint16)]
mvmdriver.mode_9.restype = None

mvmdriver.mode_mvm.argtypes = [POINTER(RPI_modes), POINTER(c_uint16), c_uint16, c_uint16, 
                              c_uint16, c_uint16, c_uint8, c_uint16, POINTER(c_uint16)]
mvmdriver.mode_mvm.restype = None

mvmdriver.fast_mvm_ON.argtypes = [POINTER(RPI_modes)]
mvmdriver.fast_mvm_ON.restype = None

mvmdriver.fast_mvm_OFF.argtypes = [POINTER(RPI_modes)]
mvmdriver.fast_mvm_OFF.restype = None

mvmdriver.fast_mvm.argtypes = [POINTER(RPI_modes), POINTER(c_uint16), c_uint16, c_uint16, 
                              c_uint16, c_uint16, c_uint8, c_uint16, POINTER(c_uint16)]
mvmdriver.fast_mvm.restype = None

# Класс-обёртка
class MVMDriver:
    def __init__(self):
        self.rpi = RPI_modes()
        mvmdriver.RPI_modes_init(byref(self.rpi))
    
    def mode_7(self, vDAC, tms, tus, rev, id, wl, bl):
        result = c_uint16(0)
        ret_id = c_uint16(0)
        mvmdriver.mode_7(byref(self.rpi), vDAC, tms, tus, rev, id, wl, bl, 
                        byref(result), byref(ret_id))
        return result.value, ret_id.value
    
    def mode_9(self, vDAC, id, wl, bl):
        result = c_uint16(0)
        mvmdriver.mode_9(byref(self.rpi), vDAC, id, wl, bl, byref(result))
        return result.value, id
    
    def mode_mvm(self, vDAC_mas, tms, tus, rtms, rums, wl, id):
        # Преобразуем Python-список в C-массив
        if len(vDAC_mas) != 32:
            raise ValueError("vDAC_mas должен содержать ровно 32 элемента")
        vDAC_mas_array = (c_uint16 * 32)(*vDAC_mas)
        result = c_uint16(0)
        mvmdriver.mode_mvm(byref(self.rpi), vDAC_mas_array, tms, tus, rtms, rums, 
                          wl, id, byref(result))
        return result.value, id
    
    def fast_mvm_ON(self):
        mvmdriver.fast_mvm_ON(byref(self.rpi))
    
    def fast_mvm_OFF(self):
        mvmdriver.fast_mvm_OFF(byref(self.rpi))
    
    def fast_mvm(self, vDAC_mas, tms, tus, rtms, rums, wl, id):
        # Преобразуем Python-список в C-массив
        if len(vDAC_mas) != 32:
            raise ValueError("vDAC_mas должен содержать ровно 32 элемента")
        vDAC_mas_array = (c_uint16 * 32)(*vDAC_mas)
        result = c_uint16(0)
        mvmdriver.fast_mvm(byref(self.rpi), vDAC_mas_array, tms, tus, rtms, rums, 
                          wl, id, byref(result))
        return result.value, id

if __name__ == "__main__":
    driver = MVMDriver()

    pdriver = RPi_modess()

    # Тест mode_7
    #result, id = driver.mode_7(0, 0, 0, 0, 123, 1,5)
    #print(f"mode_7 result: {result}, id: {id}")

    mas = [0,100,100,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]
    start = time.time()
    for i in range(10000):
        result, id = driver.mode_mvm(mas, 0,0,0,0,0,123)
    end = time.time()
    print(f"time C result: {end-start}")

    start = time.time()
    for i in range(10000):
        result, id = pdriver.mode_mvm(mas, 0,0,0,0,0,123)
    end = time.time()
    print(f"time Python result: {end-start}")
    # print(f"time:{start-end}")
    # # Тест mode_mvm
    # vDAC_mas = [100] * 32  # Пример массива
    # result, id = driver.mode_mvm(vDAC_mas, 1, 0, 0, 0, 0, 123)
    # print(f"mode_mvm result: {result}, id: {id}")