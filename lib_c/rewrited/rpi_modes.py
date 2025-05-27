import MVM_SPI as MVM_SPI
import r595hc as r595hc
import spidev
import RPi.GPIO as gpio
import time
import numpy as np

class RPI_modes():   


    def __init__(self) -> None:
        self.mvm_spi = MVM_SPI.SPI_send()
        self.reg = r595hc.RegControl595()

        for i in range(5,9):
            gpio.output(i, gpio.LOW)
            self.mvm_spi.mvm_dac_init()
            gpio.output(i, gpio.HIGH)
        

    def mode_7(self, vDAC, tms, tus, rev, id, wl, bl):
                                            # off mvm dacs 
        gpio.output(5, gpio.LOW)
        gpio.output(6, gpio.LOW)
        gpio.output(7, gpio.LOW)
        gpio.output(8, gpio.LOW)                                                
        self.mvm_spi.mwm_dac_pd_on()
        gpio.output(5, gpio.HIGH)
        gpio.output(6, gpio.HIGH)
        gpio.output(7, gpio.HIGH)
        gpio.output(8, gpio.HIGH)
                                                    # LDAC
        gpio.output(4, gpio.LOW)
        gpio.output(4, gpio.HIGH)
                                            # open all keys 714
        for i in range(4):
            self.reg.bl_key_cs_L(i)         # i key cs ON
            self.reg.reg_update()           # update reg
            self.mvm_spi.set_mode_1()
            self.mvm_spi.key_set_MVM_off()   # send 0xff to i key
            self.reg.bl_key_cs_H(i)         # i key cs OFF
            self.reg.reg_update()           # update reg
        
                                            # set commutation WR for bl
        if bl >= 0 and bl <= 15:
            self.reg.wr_spdt_comm_L()       # set wr spdt to 1st in mux
            self.reg.in_mux_EN_H(0)         # set EN on 1st in mux 
            self.reg.in_mux_EN_L(1)         # off EN on 2nd in mux
        elif bl >= 16 and bl <= 31:
            self.reg.wr_spdt_comm_H()       # set wr spdt to 1st in mux
            self.reg.in_mux_EN_L(0)         # set EN on 1st in mux 
            self.reg.in_mux_EN_H(1)         # off EN on 2nd in mux
            bl -= 16
        self.reg.in_mux_set(bl)
                                            # set mode to WR on wl mux spdt
        self.reg.spdt_select_mode_L()
        self.reg.reg_update()
                                            
        
                                            # set commutation wl
        if wl >= 0 and wl <= 7:
            if (wl & 0b1):
                gpio.output(17, gpio.HIGH)  
            else:
                gpio.output(17, gpio.LOW)
            if (wl >> 1 & 0b1):
                gpio.output(27, gpio.HIGH)
            else:
                gpio.output(27, gpio.LOW)
            if (wl >> 2 & 0b1):
                gpio.output(22, gpio.HIGH)
            else:
                gpio.output(22, gpio.LOW)

            gpio.output(13, gpio.HIGH)      # set wl EN 
        else:
            print("Wrong WL!")

                                            # set signal direction
        if rev:
            self.reg.wr_spdt_H(0)
            self.reg.wr_spdt_H(1)
            self.reg.wr_spdt_L(2)
        else:
            self.reg.wr_spdt_L(0)
            self.reg.wr_spdt_L(1)
            self.reg.wr_spdt_L(2)
            if vDAC > 2457:
                vDAC = 2457
        self.reg.reg_update()
        self.reg.reg_update()
                                            # set signal on WR DAC
        self.reg.wr_dac_cs_L()
        self.reg.reg_update()
        self.mvm_spi.set_mode_0()
        self.mvm_spi.wr_dac(vDAC)
        self.reg.wr_dac_cs_H()
        self.reg.reg_update()
                                            # update wr dac value with LDAC WR
        gpio.output(12, gpio.LOW)
        gpio.output(12, gpio.HIGH)


        

        time.sleep(tms/1000)
        time.sleep(tus/1000000)


                                            # set ZERO on WR DAC
        self.reg.wr_dac_cs_L()
        self.reg.reg_update()
        self.mvm_spi.set_mode_0()
        self.mvm_spi.wr_dac(0)
        self.reg.wr_dac_cs_H()
        self.reg.reg_update()
                                            # update wr dac value with LDAC WR
        gpio.output(12, gpio.LOW)
        gpio.output(12, gpio.HIGH)
                                            # set reading direction
        self.reg.wr_spdt_L(0)
        self.reg.wr_spdt_L(1)
        self.reg.wr_spdt_H(2)
        self.reg.reg_update()
                                            # set ZERO on WR DAC
        self.reg.wr_dac_cs_L()
        self.reg.reg_update()
        self.mvm_spi.set_mode_0()
        self.mvm_spi.wr_dac(246)
        self.reg.wr_dac_cs_H()
        self.reg.reg_update()
                                            # update wr dac value with LDAC WR
        gpio.output(12, gpio.LOW)
        gpio.output(12, gpio.HIGH)

                                            # read from ADC
        result = 0
        self.reg.spdt_select_mode_for_ADC_wr()
        self.mvm_spi.set_mode_1()
        gpio.output(25, gpio.LOW)
        _ = self.mvm_spi.adc_read()
        gpio.output(25, gpio.HIGH)
        for i in range(10):

            gpio.output(25, gpio.LOW)
            result += self.mvm_spi.adc_read()
            gpio.output(25, gpio.HIGH)
        result /= 10
                                            # set ZERO on WR DAC
        self.reg.wr_dac_cs_L()
        self.reg.reg_update()
        self.mvm_spi.set_mode_0()
        self.mvm_spi.wr_dac(0)
        self.reg.wr_dac_cs_H()
        self.reg.reg_update()
                                            # update wr dac value with LDAC WR
        gpio.output(12, gpio.LOW)
        gpio.output(12, gpio.HIGH)
                                                    # set dir direction
        self.reg.wr_spdt_L(0)
        self.reg.wr_spdt_L(1)
        self.reg.wr_spdt_L(2)
        self.reg.reg_update()
                                            # disable commutation
        gpio.output(13, gpio.LOW)
        gpio.output(17, gpio.LOW)
        gpio.output(27, gpio.LOW)
        gpio.output(22, gpio.LOW)
        self.reg.wr_spdt_comm_L()       
        self.reg.in_mux_EN_L(0)   
        self.reg.in_mux_EN_L(1)     
        self.reg.in_mux_set(0)
        self.reg.spdt_select_mode_L()
        self.reg.reg_update()
                                            # open all keys 714
        for i in range(4):
            self.reg.bl_key_cs_L(i)         # i key cs ON
            self.reg.reg_update()           # update reg
            self.mvm_spi.set_mode_1()
            self.mvm_spi.key_set_MVM_off()   # send 0x00 to i key
            self.reg.bl_key_cs_H(i)         # i key cs OFF
            self.reg.reg_update()           # update reg
        
                                                    # on mvm dacs 
        gpio.output(5, gpio.LOW)
        gpio.output(6, gpio.LOW)
        gpio.output(7, gpio.LOW)
        gpio.output(8, gpio.LOW)                                                
        self.mvm_spi.mwm_dac_pd_off()
        gpio.output(5, gpio.HIGH)
        gpio.output(6, gpio.HIGH)
        gpio.output(7, gpio.HIGH)
        gpio.output(8, gpio.HIGH)
                                                    # LDAC
        gpio.output(4, gpio.LOW)
        gpio.output(4, gpio.HIGH)

        #self.reg.BLINKBLINK(0.1)
        
        return result, id

    def mode_9(self, vDAC, id, wl, bl):
                                            # off mvm dacs 
        gpio.output(5, gpio.LOW)
        gpio.output(6, gpio.LOW)
        gpio.output(7, gpio.LOW)
        gpio.output(8, gpio.LOW)                                                
        self.mvm_spi.mwm_dac_pd_on()
        gpio.output(5, gpio.HIGH)
        gpio.output(6, gpio.HIGH)
        gpio.output(7, gpio.HIGH)
        gpio.output(8, gpio.HIGH)
                                                    # LDAC
        gpio.output(4, gpio.LOW)
        gpio.output(4, gpio.HIGH)
                                            # open all keys 714
        for i in range(4):
            self.reg.bl_key_cs_L(i)         # i key cs ON
            self.reg.reg_update()           # update reg
            self.mvm_spi.set_mode_1()
            self.mvm_spi.key_set_MVM_off()   # send 0xff to i key
            self.reg.bl_key_cs_H(i)         # i key cs OFF
            self.reg.reg_update()           # update reg
        
                                            # set commutation WR for bl
        if bl >= 0 and bl <= 15:
            self.reg.wr_spdt_comm_L()       # set wr spdt to 1st in mux
            self.reg.in_mux_EN_H(0)         # set EN on 1st in mux 
            self.reg.in_mux_EN_L(1)         # off EN on 2nd in mux
        elif bl >= 16 and bl <= 31:
            self.reg.wr_spdt_comm_H()       # set wr spdt to 1st in mux
            self.reg.in_mux_EN_L(0)         # set EN on 1st in mux 
            self.reg.in_mux_EN_H(1)         # off EN on 2nd in mux
            bl -= 16
        self.reg.in_mux_set(bl)
                                            # set mode to WR on wl mux spdt
        self.reg.spdt_select_mode_L()
        self.reg.reg_update()
                                            
        
                                            # set commutation wl
        if wl >= 0 and wl <= 7:
            if (wl & 0b1):
                gpio.output(17, gpio.HIGH)  
            else:
                gpio.output(17, gpio.LOW)
            if (wl >> 1 & 0b1):
                gpio.output(27, gpio.HIGH)
            else:
                gpio.output(27, gpio.LOW)
            if (wl >> 2 & 0b1):
                gpio.output(22, gpio.HIGH)
            else:
                gpio.output(22, gpio.LOW)

            gpio.output(13, gpio.HIGH)      # set wl EN 
        else:
            print("Wrong WL!")
                                            # set reading direction
        self.reg.wr_spdt_L(0)
        self.reg.wr_spdt_L(1)
        self.reg.wr_spdt_H(2)
        self.reg.reg_update()
                                            # set ZERO on WR DAC
        if vDAC > 246: vDAC = 246
        self.reg.wr_dac_cs_L()
        self.reg.reg_update()
        self.mvm_spi.set_mode_0()
        self.mvm_spi.wr_dac(vDAC)
        self.reg.wr_dac_cs_H()
        self.reg.reg_update()
                                            # update wr dac value with LDAC WR
        gpio.output(12, gpio.LOW)
        gpio.output(12, gpio.HIGH)
                                            # read from ADC
        self.reg.spdt_select_mode_for_ADC_wr()

        self.mvm_spi.set_mode_1()
        
        gpio.output(25, gpio.LOW)
        _ = self.mvm_spi.adc_read()
        gpio.output(25, gpio.HIGH)

        gpio.output(25, gpio.LOW)
        result = self.mvm_spi.adc_read()
        gpio.output(25, gpio.HIGH)
                                            # set ZERO on WR DAC
        self.reg.wr_dac_cs_L()
        self.reg.reg_update()
        self.mvm_spi.set_mode_0()
        self.mvm_spi.wr_dac(0)
        self.reg.wr_dac_cs_H()
        self.reg.reg_update()
                                            # update wr dac value with LDAC WR
        gpio.output(12, gpio.LOW)
        gpio.output(12, gpio.HIGH)
                                                    # set dir direction
        self.reg.wr_spdt_L(0)
        self.reg.wr_spdt_L(1)
        self.reg.wr_spdt_L(2)
        self.reg.reg_update()
                                            # disable commutation
        gpio.output(13, gpio.LOW)
        gpio.output(17, gpio.LOW)
        gpio.output(27, gpio.LOW)
        gpio.output(22, gpio.LOW)
        self.reg.wr_spdt_comm_L()       
        self.reg.in_mux_EN_L(0)   
        self.reg.in_mux_EN_L(1)     
        self.reg.in_mux_set(0)
        self.reg.spdt_select_mode_L()
        self.reg.reg_update()
                                            # open all keys 714
        for i in range(4):
            self.reg.bl_key_cs_L(i)         # i key cs ON
            self.reg.reg_update()           # update reg
            self.mvm_spi.set_mode_1()
            self.mvm_spi.key_set_MVM_off()   # send 0x00 to i key
            self.reg.bl_key_cs_H(i)         # i key cs OFF
            self.reg.reg_update()           # update reg
        
                                                    # on mvm dacs 
        gpio.output(5, gpio.LOW)
        gpio.output(6, gpio.LOW)
        gpio.output(7, gpio.LOW)
        gpio.output(8, gpio.LOW)                                                
        self.mvm_spi.mwm_dac_pd_off()
        gpio.output(5, gpio.HIGH)
        gpio.output(6, gpio.HIGH)
        gpio.output(7, gpio.HIGH)
        gpio.output(8, gpio.HIGH)
                                                    # LDAC
        gpio.output(4, gpio.LOW)
        gpio.output(4, gpio.HIGH)
        
        #self.reg.BLINKBLINK(0.1)
        return result, id


    def mode_mvm(self, vDAC_mas, tms, tus, rtms, rums, wl, id):
                                                    # on mvm dacs 
        gpio.output(5, gpio.LOW)
        gpio.output(6, gpio.LOW)
        gpio.output(7, gpio.LOW)
        gpio.output(8, gpio.LOW)
        self.mvm_spi.set_mode_1()                                                
        self.mvm_spi.mwm_dac_pd_off()
        gpio.output(5, gpio.HIGH)
        gpio.output(6, gpio.HIGH)
        gpio.output(7, gpio.HIGH)
        gpio.output(8, gpio.HIGH)
                                                    # LDAC
        gpio.output(4, gpio.LOW)
        gpio.output(4, gpio.HIGH)
                                                        # close all keys 714
        
        
        mask = list(map( lambda x: int(x), np.where(np.array(vDAC_mas) > 0, 1, 0)))  # create byte mask
        # print(mask)
        mask_bytes = [
            sum(mask[i * 8 + j] << (j) for j in range(8))
            for i in range(4)
        ]
           
                
        for i in range(4):
            self.reg.bl_key_cs_L(i)           # i key cs OFF
            self.reg.reg_update()             # update reg
            self.mvm_spi.set_mode_1()
            #print(type(mask_bytes[i]))
            self.mvm_spi.key_set_MVM_on_mask(mask_bytes[i])    #s send mask to i key 
            self.reg.bl_key_cs_H(i)           # i key cs OFF
            self.reg.reg_update()             # update reg
                                                    # disable commutation
        gpio.output(13, gpio.LOW)
        gpio.output(17, gpio.LOW)
        gpio.output(27, gpio.LOW)
        gpio.output(22, gpio.LOW)
        self.reg.wr_spdt_comm_L()       
        self.reg.in_mux_EN_L(0)   
        self.reg.in_mux_EN_L(1)     
        self.reg.in_mux_set(0)
        self.reg.spdt_select_mode_H()
        self.reg.reg_update()
                                            # set commutation wl
        if wl >= 0 and wl <= 7:
            if (wl & 0b1):
                gpio.output(17, gpio.HIGH)  
            else:
                gpio.output(17, gpio.LOW)
            if (wl >> 1 & 0b1):
                gpio.output(27, gpio.HIGH)
            else:
                gpio.output(27, gpio.LOW)
            if (wl >> 2 & 0b1):
                gpio.output(22, gpio.HIGH)
            else:
                gpio.output(22, gpio.LOW)

            gpio.output(13, gpio.HIGH)      # set wl EN 
        else:
            print("Wrong WL!")

                                            # set values to MVM DACs
        for i in range(0,4):
            self.mvm_spi.set_mode_1()
            
            for j in range(0,8):
                gpio.output(i+5, gpio.LOW)
                self.mvm_spi.mvm_dac(vDAC_mas[j+(8*i)], j)
                gpio.output(i+5, gpio.HIGH)
                                            # LDAC
        gpio.output(4, gpio.LOW)
        gpio.output(4, gpio.HIGH)
                                                    # read from ADC
        self.reg.spdt_select_mode_for_ADC_mvm()
        
        self.mvm_spi.set_mode_1()
        gpio.output(25, gpio.LOW)
        _ = self.mvm_spi.adc_read()
        gpio.output(25, gpio.HIGH)
        
        gpio.output(25, gpio.LOW)
        result = self.mvm_spi.adc_read()
        gpio.output(25, gpio.HIGH)
        self.reg.spdt_select_mode_for_ADC_wr()
                                                    # set ZERO to MVM DACs
        self.mvm_spi.set_mode_1()
        for i in range(0,4):
            
            
            for j in range(0,8):
                gpio.output(i+5, gpio.LOW)
                self.mvm_spi.mvm_dac(0, j)
                gpio.output(i+5, gpio.HIGH)
                                            # LDAC
        gpio.output(4, gpio.LOW)
        gpio.output(4, gpio.HIGH)
                                                            # off mvm dacs 
        gpio.output(5, gpio.LOW)
        gpio.output(6, gpio.LOW)
        gpio.output(7, gpio.LOW)
        gpio.output(8, gpio.LOW)  
        self.mvm_spi.set_mode_1()                                              
        self.mvm_spi.mwm_dac_pd_on()
        gpio.output(5, gpio.HIGH)
        gpio.output(6, gpio.HIGH)
        gpio.output(7, gpio.HIGH)
        gpio.output(8, gpio.HIGH)
                                                    # LDAC
        gpio.output(4, gpio.LOW)
        gpio.output(4, gpio.HIGH)
                                                    # disable commutation
        gpio.output(13, gpio.LOW)
        gpio.output(17, gpio.LOW)
        gpio.output(27, gpio.LOW)
        gpio.output(22, gpio.LOW)
        self.reg.spdt_select_mode_H()
        self.reg.reg_update()
        return result, id
        
    
    def fast_mvm_ON(self):
                                                # on mvm dacs 
        gpio.output(5, gpio.LOW)
        gpio.output(6, gpio.LOW)
        gpio.output(7, gpio.LOW)
        gpio.output(8, gpio.LOW)
        self.mvm_spi.set_mode_1()                                                
        self.mvm_spi.mwm_dac_pd_off()
        gpio.output(5, gpio.HIGH)
        gpio.output(6, gpio.HIGH)
        gpio.output(7, gpio.HIGH)
        gpio.output(8, gpio.HIGH)
                                                    # LDAC
        gpio.output(4, gpio.LOW)
        gpio.output(4, gpio.HIGH)
                                                    # close all keys 714
        for i in range(4):
            self.reg.bl_key_cs_L(i)           # i key cs OFF
            self.reg.reg_update()             # update reg
            self.mvm_spi.set_mode_1()
            self.mvm_spi.key_set_MVM_on()    # send 0xFF to i key
            self.reg.bl_key_cs_H(i)           # i key cs OFF
            self.reg.reg_update()             # update reg
                                                    # disable commutation
        gpio.output(13, gpio.LOW)
        gpio.output(17, gpio.LOW)
        gpio.output(27, gpio.LOW)
        gpio.output(22, gpio.LOW)
        self.reg.wr_spdt_comm_L()       
        self.reg.in_mux_EN_L(0)   
        self.reg.in_mux_EN_L(1)     
        self.reg.in_mux_set(0)
        self.reg.spdt_select_mode_H()
        self.reg.reg_update()
        self.reg.spdt_select_mode_for_ADC_mvm()
    
    
    def fast_mvm_OFF(self):
        gpio.output(5, gpio.LOW)
        gpio.output(6, gpio.LOW)
        gpio.output(7, gpio.LOW)
        gpio.output(8, gpio.LOW)  
        self.mvm_spi.set_mode_1()                                              
        self.mvm_spi.mwm_dac_pd_on()
        gpio.output(5, gpio.HIGH)
        gpio.output(6, gpio.HIGH)
        gpio.output(7, gpio.HIGH)
        gpio.output(8, gpio.HIGH)
                                                    # LDAC
        gpio.output(4, gpio.LOW)
        gpio.output(4, gpio.HIGH)
                                                    # disable commutation
        gpio.output(13, gpio.LOW)
        gpio.output(17, gpio.LOW)
        gpio.output(27, gpio.LOW)
        gpio.output(22, gpio.LOW)
        self.reg.spdt_select_mode_H()
        self.reg.reg_update()
        self.reg.spdt_select_mode_for_ADC_wr()


    def fast_mvm(self, vDAC_mas:list, tms, tus, rtms, rums, wl, id):
                    # set commutation wl
        if wl >= 0 and wl <= 7:
            if (wl & 0b1):
                gpio.output(17, gpio.HIGH)  
            else:
                gpio.output(17, gpio.LOW)
            if (wl >> 1 & 0b1):
                gpio.output(27, gpio.HIGH)
            else:
                gpio.output(27, gpio.LOW)
            if (wl >> 2 & 0b1):
                gpio.output(22, gpio.HIGH)
            else:
                gpio.output(22, gpio.LOW)

            gpio.output(13, gpio.HIGH)      # set wl EN 
        else:
            print("Wrong WL!")

                                            # set values to MVM DACs
        for i in range(0,4):
            self.mvm_spi.set_mode_1()
            
            for j in range(0,8):
                gpio.output(i+5, gpio.LOW)
                self.mvm_spi.mvm_dac(vDAC_mas[j+(8*i)], j)
                gpio.output(i+5, gpio.HIGH)
                                            # LDAC
        gpio.output(4, gpio.LOW)
        gpio.output(4, gpio.HIGH)
                                                    # read from ADC
        
        self.mvm_spi.set_mode_1()
        gpio.output(25, gpio.LOW)
        _ = self.mvm_spi.adc_read()
        gpio.output(25, gpio.HIGH)

        gpio.output(25, gpio.LOW)
        result = self.mvm_spi.adc_read()
        gpio.output(25, gpio.HIGH)
                                                    # set ZERO to MVM DACs
        self.mvm_spi.set_mode_1()
        for i in range(0,4):
            
            
            for j in range(0,8):
                gpio.output(i+5, gpio.LOW)
                self.mvm_spi.mvm_dac(0, j)
                gpio.output(i+5, gpio.HIGH)
                                            # LDAC
        gpio.output(4, gpio.LOW)
        gpio.output(4, gpio.HIGH)



        return result, id
        






if __name__ == "__main__":
    test = RPI_modes()
    print(test.mode_7(245, 1, 0, 0, 123, 0, 0))