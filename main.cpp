#include <avr/io.h>
#include <util/atomic.h>
#include <util/delay.h>
#include "SchrottPHY.hpp"
#include "SchrottMAC.hpp"
#include "MotorController.hpp"

int main() {
    SchrottPHY phy;
    SchrottMAC mac(phy);
    MotorController motor;

    sei();

    //mac.sendPayload((const uint8_t*)"Hallo Welt!", 11);

    mac.sendPayload((const uint8_t*)"Hey", 3);

    motor.forward();

    while(true) {
        phy.run();
    }
}
