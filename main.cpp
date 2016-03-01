#include <avr/io.h>
#include <util/atomic.h>
#include <util/delay.h>
#include "SchrottPHY.hpp"
#include "SchrottMAC.hpp"
#include "MotorController.hpp"
#include "UART.hpp"

int main() {
    cli();
    SchrottPHY phy;
    SchrottMAC mac(phy);
    MotorController motor;

    //sei();

    //mac.sendPayload((const uint8_t*)"Hallo Welt!", 11);

    if(mac.sendPayload((const uint8_t*)"Hey", 3) == 0xFF) {
        UART::get() << "Send error\n";
    }

    motor.forward();

    while(true) {
        phy.run();
    }
}
