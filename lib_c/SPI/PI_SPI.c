/* PI_SPI.c
 Yann Guidon whygee@f-cpu.org
 based on code from G.J. van Loo 15-Jan-2012
 20130508 : init
 20140904
 20140907 : stderr
 20161109 : HaD release

 Don't include this file directly.
 Include PI_SPI_config.h instead.
*/

#ifndef PI_SPI
#define PI_SPI

#include "PI_GPIO.c"

volatile unsigned *PI_spi0 = NULL;

#define SPI0_BASE  (0x20204000) /* SPI0 controller */

// From Gert's example :
#define SPI0_CNTLSTAT *(PI_spi0 + 0)
#define SPI0_FIFO     *(PI_spi0 + 1)
#define SPI0_CLKSPEED *(PI_spi0 + 2)

// SPI0_CNTLSTAT register bits
#define SPI0_CS_CS2ACTHIGH   0x00800000 // CS2 active high
#define SPI0_CS_CS1ACTHIGH   0x00400000 // CS1 active high
#define SPI0_CS_CS0ACTHIGH   0x00200000 // CS0 active high
#define SPI0_CS_RXFIFOFULL   0x00100000 // Receive FIFO full
#define SPI0_CS_RXFIFO3_4    0x00080000 // Receive FIFO 3/4 full
#define SPI0_CS_TXFIFOSPCE   0x00040000 // Transmit FIFO has space
#define SPI0_CS_RXFIFODATA   0x00020000 // Receive FIFO has data
#define SPI0_CS_DONE         0x00010000 // SPI transfer done. WRT to CLR!
#define SPI0_CS_MOSI_INPUT   0x00001000 // MOSI is input, read from MOSI (BI-dir mode)
#define SPI0_CS_DEASRT_CS    0x00000800 // De-assert CS at end
#define SPI0_CS_RX_IRQ       0x00000400 // Receive irq enable
#define SPI0_CS_DONE_IRQ     0x00000200 // irq when done
#define SPI0_CS_DMA_ENABLE   0x00000100 // Run in DMA mode
#define SPI0_CS_ACTIVATE     0x00000080 // Activate: be high before starting
#define SPI0_CS_CS_POLARIT   0x00000040 // Chip selects active high
#define SPI0_CS_CLRTXFIFO    0x00000020 // Clear TX FIFO    (auto clear bit)
#define SPI0_CS_CLRRXFIFO    0x00000010 // Clear RX FIFO    (auto clear bit)
#define SPI0_CS_CLRFIFOS     0x00000030 // Clear BOTH FIFOs (auto clear bit)
#define SPI0_CS_CLK_IDLHI    0x00000008 // Clock pin is high when idle
#define SPI0_CS_CLKTRANS     0x00000004 // 0=first clock in middle of data bit
                                        // 1=first clock at begin of data bit
#define SPI0_CS_CHIPSEL0     0x00000000 // Use chip select 0
#define SPI0_CS_CHIPSEL1     0x00000001 // Use chip select 1
#define SPI0_CS_CHIPSEL2     0x00000002 // Use chip select 2
#define SPI0_CS_CHIPSELN     0x00000003 // No chip select (e.g. use GPIO pin)

#define SPI0_CS_CLRALL      (SPI0_CS_CLRFIFOS|SPI0_CS_DONE)

// Which pins are used ?
#define PI_PIN_SCLK (1<<11)
#define PI_PIN_MOSI (1<<10)
#define PI_PIN_MISO (1<<9)
#define PI_PIN_SCE0 (1<<8)
#define PI_PIN_SCE1 (1<<7)

// Backup of the expected controller state
int SPI_CTL_normal;

#ifndef GPIO_NO_ATEXIT
// parachute :
void parachute_SPI() {
  fputs("\nfermeture SPI", stderr);
  SPI0_CNTLSTAT = SPI_CTL_normal
                | SPI0_CS_CLRFIFOS
                | SPI0_CS_DONE;
}
#endif

void PI_setup_SPI(int pins, int speed, int spi_ctl) {

  // only enable the pins that you actually use:
  int pin_nr=7, pin_mask=1<<7;
  while(pins) {
    if(pins & pin_mask){
      PI_GPIO_config(pin_nr,BCM_GPIO_ALT0);
      pins -= pin_mask;
    }
    pin_mask+=pin_mask;
    pin_nr++;
  }

  // requests access to the SPI controller
  PI_spi0 = PI_IOmmap(SPI0_BASE);

  // speed = 250MHz/desired speed
  SPI0_CLKSPEED = speed;
  SPI0_CNTLSTAT = SPI0_CS_CLRFIFOS
                | SPI0_CS_DONE;

  SPI0_CNTLSTAT
    = SPI_CTL_normal
    = spi_ctl;
#ifndef GPIO_NO_ATEXIT
  atexit(parachute_SPI);
#endif
}

int SPI_status;
char SPI_dummy;

void SPI_wait_done(){
  // wait for SPI to be done and empty the FIFO
  do {
    SPI_status = SPI0_CNTLSTAT;
    if (SPI_status & SPI0_CS_RXFIFODATA)
      SPI_dummy = SPI0_FIFO; // Flush the read FIFO
  } while (( SPI_status & SPI0_CS_DONE) == 0);
}

void SPI_wait_send(char dat){
  // wait for SPI to be ready then send another byte
  do {
    SPI_status = SPI0_CNTLSTAT;
    if (SPI_status & SPI0_CS_RXFIFODATA)
      SPI_dummy = SPI0_FIFO; // Flush the read FIFO
  } while ((SPI_status & SPI0_CS_TXFIFOSPCE ) == 0);
  SPI0_FIFO = dat; // send the byte
}

// Send and receive a byte at the same time
unsigned int SPI_transmit_byte(unsigned int u) {
  SPI0_FIFO = u;  // send the byte
  // Wait until transmission is done:
  do {
    SPI_status = SPI0_CNTLSTAT;
  } while (( SPI_status & SPI0_CS_DONE) == 0);
  return SPI0_FIFO;  // read the data FIFO
}

#endif
