#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

int main(){
    wiringPiSetupPinType(WPI_PIN_BCM);
    int i = 0;
    pinMode(17, OUTPUT);
    digitalWrite(17, HIGH);
    usleep(1000);
    digitalWrite(17, LOW);
return 0;
}