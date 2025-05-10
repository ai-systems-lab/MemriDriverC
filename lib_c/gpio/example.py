import ctypes
import time

# Загрузка библиотеки
lib = ctypes.CDLL('./libreg_control_595.so')

# Определение типов возвращаемых значений и аргументов
lib.reg_control_595_init.restype = ctypes.c_void_p
lib.reg_control_595_deinit.argtypes = [ctypes.c_void_p]
lib.reg_update.argtypes = [ctypes.c_void_p]
lib.bl_key_cs_H.argtypes = [ctypes.c_void_p, ctypes.c_int]
lib.bl_key_cs_L.argtypes = [ctypes.c_void_p, ctypes.c_int]
lib.in_mux_set.argtypes = [ctypes.c_void_p, ctypes.c_int]
lib.wr_spdt_H.argtypes = [ctypes.c_void_p, ctypes.c_int]
lib.wr_spdt_L.argtypes = [ctypes.c_void_p, ctypes.c_int]
lib.in_mux_EN_H.argtypes = [ctypes.c_void_p, ctypes.c_int]
lib.in_mux_EN_L.argtypes = [ctypes.c_void_p, ctypes.c_int]
lib.wr_dac_cs_H.argtypes = [ctypes.c_void_p]
lib.wr_dac_cs_L.argtypes = [ctypes.c_void_p]
lib.wr_spdt_comm_H.argtypes = [ctypes.c_void_p]
lib.wr_spdt_comm_L.argtypes = [ctypes.c_void_p]
lib.spdt_select_mode_H.argtypes = [ctypes.c_void_p]
lib.spdt_select_mode_L.argtypes = [ctypes.c_void_p]
lib.spdt_select_mode_for_ADC_wr.argtypes = [ctypes.c_void_p]
lib.spdt_select_mode_for_ADC_mvm.argtypes = [ctypes.c_void_p]
lib.beep_beep.argtypes = [ctypes.c_void_p, ctypes.c_float]
lib.blink_blink.argtypes = [ctypes.c_void_p, ctypes.c_float]

# Инициализация
ctrl = lib.reg_control_595_init()
if not ctrl:
    print("Ошибка инициализации")
    exit(1)

try:
    # Пример использования
    lib.in_mux_set(ctrl, 2)
    lib.reg_update(ctrl)
    lib.blink_blink(ctrl, 0.5)
    lib.beep_beep(ctrl, 0.5)
finally:
    # Очистка
    lib.reg_control_595_deinit(ctrl)