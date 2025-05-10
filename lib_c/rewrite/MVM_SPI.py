import spidev

class SPI_send:
    # HAS SPI MODE 0: WR DAC, 
    # HAS SPI MODE 1: MVM DACs,  714 KEYs, ADC

    def __init__(self) -> None:
        self.spi = spidev.SpiDev()
        self.spi.open(0,0)
        self.spi.max_speed_hz = 15000000
        # self.spi.no_cs = True
        self.spi.mode = 0
    
    def set_mode_0(self) -> None:
        self.spi.mode = 0
    def set_mode_1(self) -> None:
        self.spi.mode = 1


    def mvm_dac_init(self) -> None:
        self.spi.writebytes([0x80, 0xD])
    def mvm_dac(self, bitvalue, channel) -> None:
        values = [0x00, 0x00]
        values[0] = channel << 4 | bitvalue >> 8
        values[1] = bitvalue & 0xff
        self.spi.writebytes(values)

    def wr_dac(self, bitvalue = 0) -> None:
        values = [0x00, 0x00]
        values[0] = 0b00110000 | bitvalue >> 8
        values[1] = bitvalue & 0xff
        self.spi.writebytes(values)
    
    def adc_read(self) -> int:

        res = 0
        res = self.spi.readbytes(2)
        self.spi.writebytes([0x00, 0x00])
        #print(bin(res[0]),bin(res[1]))
        return((res[0] << 6) | res[1]>>2)
    
    def key_set_MVM_on_mask(self, mask) -> None:
        # print(mask)
        self.spi.writebytes([mask])

    def key_set_MVM_on(self, mask) -> None:
        self.spi.writebytes([mask])


    def key_set_MVM_off(self) -> None:
        values = [0x00]
        self.spi.writebytes(values)
    
    def mwm_dac_pd_on(self) -> None:
        values = [0b11000000, 0xff]
        self.spi.writebytes(values)
    def mwm_dac_pd_off(self) -> None:
        values = [0b11000000, 0x00]
        self.spi.writebytes(values)
        




# TEST        
if __name__ == "__main__":
    test = SPI_send()
    test.mvm_dac(675, 3)
    test.mvm_dac(0, 0)