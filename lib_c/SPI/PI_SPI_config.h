/* PI_SPI_config.h
  created 20140905 by Yann Guidon whygee@f-cpu.org

  Includes PI_SPI.c, or an eventual wrapper and some eventual configuration values
*/

#define PI_GPIO_ERR
#include "PI_SPI.c"
#define SPICTL_OR   (SPI0_CS_CHIPSEL0)   // SEL=0 when transfer active

