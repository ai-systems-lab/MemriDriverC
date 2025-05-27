#include "src/r595hc.h"
#include "src/MVM_SPI.h"
#include "src/rpi_modes.h"

int main() {
     wiringPiSetupGpio();
    
     RPI_modes rpi;
     RPI_modes_init(&rpi);
    
     uint16_t result, ret_id;
for (int i = 0; i < 10; i++) {  // Пример: 10 итераций
    mode_7(&rpi, 350, 0, 0, 0, 123, 0, 0, &result, &ret_id);
    printf("Iteration %d: Result: %d, ID: %d\n", i, result, ret_id);
    
}    
     return 0;
 }

