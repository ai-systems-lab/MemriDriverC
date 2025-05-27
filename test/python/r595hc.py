import RPi.GPIO as gpio
from time import sleep

class RegControl595:
    

    # BL_KEY_3-0    IN_MUX_3-0
    # SPDT_SELECT_MODE  WR_SPDT_COMM    WR_DAC_CS   WR_SPDT_2-0 IN_MUX_EN_2-1



    def __init__(self) -> None:

        self.main_bytes = [0xf0, 0x20]
        gpio.setmode(gpio.BCM)
        gpio.setup(23,gpio.OUT, initial=gpio.LOW)      # SHCP  | SCLK_R        
        gpio.setup(24,gpio.OUT, initial=gpio.HIGH)     # STCP  | CS_R
        gpio.setup(16,gpio.OUT, initial=gpio.LOW)      # DS    | MOSI_R
        gpio.setup(12,gpio.OUT, initial=gpio.HIGH)     # WR_LDAC
        gpio.setup(25,gpio.OUT, initial=gpio.HIGH)     # ADC_CS
        gpio.setup(26,gpio.OUT, initial=gpio.LOW)      # ADC_SPDT

        gpio.setup(5,gpio.OUT, initial=gpio.HIGH)      # MVM_DAC_CS_0
        gpio.setup(6,gpio.OUT, initial=gpio.HIGH)      # MVM_DAC_CS_1
        gpio.setup(7,gpio.OUT, initial=gpio.HIGH)      # MVM_DAC_CS_2
        gpio.setup(8,gpio.OUT, initial=gpio.HIGH)      # MVM_DAC_CS_3
        gpio.setup(4,gpio.OUT, initial=gpio.HIGH)      # MVM_LDAC

        gpio.setup(13,gpio.OUT, initial=gpio.LOW)      # WL_MUX_EN
        gpio.setup(17,gpio.OUT, initial=gpio.LOW)      # WL_MUX_0
        gpio.setup(27,gpio.OUT, initial=gpio.LOW)      # WL_MUX_1
        gpio.setup(22,gpio.OUT, initial=gpio.LOW)      # WL_MUX_2

        gpio.setup(21,gpio.OUT, initial=gpio.LOW)      # BUZZER
        gpio.setup(19,gpio.OUT, initial=gpio.LOW)      # LED
        
        gpio.output(25,gpio.HIGH)
        gpio.output(25,gpio.LOW)
        gpio.output(25,gpio.HIGH)

        self.reg_update()


    def __transfer595(self, x):
        x = x & 0xff
        ii = 1
        for i in range(8):
            if ii & x:
                gpio.output(16,gpio.HIGH)
            else:
                gpio.output(16,gpio.LOW)
            ii = ii << 1
            gpio.output(23,gpio.HIGH)
            gpio.output(23,gpio.LOW)
        gpio.output(16,gpio.LOW)

    def reg_update(self):
        gpio.output(24,gpio.LOW)
        self.__transfer595(self.main_bytes[0])
        self.__transfer595(self.main_bytes[1])
        gpio.output(24,gpio.HIGH)

    def bl_key_cs_H(self, n = 0) -> None:
        self.main_bytes[0] |= (1 << (4+n)) & 0xff
    def bl_key_cs_L(self, n = 0) -> None:
        self.main_bytes[0] &= (~(1 << (4+n))) & 0xff

    def in_mux_set(self, n = 0) -> None:
        self.main_bytes[0] &= 0xf0
        self.main_bytes[0] |= n & 0xf

    
    def wr_spdt_H(self, n = 0) -> None:
        self.main_bytes[1] |= (1 << (2+n)) & 0xff
    def wr_spdt_L(self, n = 0) -> None:
        self.main_bytes[1] &= (~(1 << (2+n))) & 0xff

    def in_mux_EN_H(self, n = 0) -> None:
        self.main_bytes[1] |= (1 << n) & 0xff
    def in_mux_EN_L(self, n = 0) -> None:
        self.main_bytes[1] &= (~(1 << n)) & 0xff

    def wr_dac_cs_H(self) -> None:
        self.main_bytes[1] |= (1 << 5) & 0xff
    def wr_dac_cs_L(self) -> None:
        self.main_bytes[1] &= (~(1 << 5)) & 0xff

    def wr_spdt_comm_H(self) -> None:
        self.main_bytes[1] |= (1 << 6) & 0xff
    def wr_spdt_comm_L(self) -> None:
        self.main_bytes[1] &= (~(1 << 6)) & 0xff
    
    def spdt_select_mode_H(self) -> None:
        self.main_bytes[1] |= (1 << 7) & 0xff
    def spdt_select_mode_L(self) -> None:
        self.main_bytes[1] &= (~(1 << 7)) & 0xff


    def spdt_select_mode_for_ADC_wr(self):
        gpio.output(26,gpio.LOW)
    def spdt_select_mode_for_ADC_mvm(self):
        gpio.output(26,gpio.HIGH)    
        
    def BEEPBEEP(self, timeS) -> None:
        gpio.output(21, gpio.HIGH)
        sleep(timeS)
        gpio.output(21, gpio.LOW)  # BEEEEEEEEP

    def BLINKBLINK(self, timeS) -> None:
        gpio.output(19, gpio.HIGH)
        sleep(timeS)
        gpio.output(19, gpio.LOW)  # BLINK


#test 
if __name__ == "__main__":
    a = RegControl595()
    a.reg_update()
    gpio.cleanup()