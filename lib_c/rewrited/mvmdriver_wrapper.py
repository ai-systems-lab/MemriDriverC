from ctypes import *
import os

# Загружаем разделяемую библиотеку
lib_path = os.path.join(os.path.dirname(__file__), 'libmvmdriver.so')
mvmdriver = CDLL(lib_path)

# Определяем структуры для соответствия C-структурам
class RegControl595(Structure):
    _fields_ = [("main_bytes", c_ubyte * 2)]

class SPI_send(Structure):
    _fields_ = [("spi_fd", c_int),
                ("current_spi_mode", c_ubyte)]

class RPI_modes(Structure):
    _fields_ = [("mvm_spi", SPI_send),
                ("reg", RegControl595)]

# Инициализация функций
mvmdriver.RPI_modes_init.argtypes = [POINTER(RPI_modes)]
mvmdriver.RPI_modes_init.restype = None

mvmdriver.mode_7.argtypes = [POINTER(RPI_modes), c_uint16, c_uint16, c_uint16, 
                            c_uint8, c_uint16, c_uint8, c_uint8, POINTER(c_uint16)]
mvmdriver.mode_7.restype = None

# Класс-обертка для удобной работы из Python
class MVMDriver:
    def __init__(self):
        self.rpi = RPI_modes()
        mvmdriver.RPI_modes_init(byref(self.rpi))
    
    def mode_7(self, vDAC, tms, tus, rev, id, wl, bl):
        result = c_uint16(0)
        mvmdriver.mode_7(byref(self.rpi), vDAC, tms, tus, rev, id, wl, bl, byref(result))
        return result.value, id