#include <bcm2835.h>
#include <stdio.h>

int main() {
    if (!bcm2835_init()) {
        printf("Ошибка инициализации BCM2835!\n");
        return 1;
    }

    bcm2835_spi_begin();
    bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
    bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);
    bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_256); // ~1 МГц

    uint8_t tx_data[] = {0x55, 0xAA};
    uint8_t rx_data[2] = {0};
    
    bcm2835_spi_transfernb((char*)tx_data, (char*)rx_data, sizeof(tx_data));
    
    printf("Отправлено: 0x%02X 0x%02X\n", tx_data[0], tx_data[1]);
    printf("Принято:    0x%02X 0x%02X\n", rx_data[0], rx_data[1]);

    bcm2835_spi_end();
    bcm2835_close();
    return 0;
}//gcc -o spi_test spi_test.c -lbcm2835