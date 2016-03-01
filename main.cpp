#include <avr/io.h>
#include <util/atomic.h>
#include <util/delay.h>
#include "SchrottPHY.hpp"
#include "SchrottMAC.hpp"
#include "MotorController.hpp"
#include "UART.hpp"
#include "TurnoutController.hpp"

int main() {
    SchrottPHY phy;
    SchrottMAC mac(phy);
#ifdef TRAIN
    MotorController motor;
#else
    TurnoutController turnout;
#endif

    sei();

    //mac.sendPayload((const uint8_t*)"Hallo Welt!", 11);

    if(mac.sendPayload((const uint8_t*)"Hey", 3) == 0xFF) {
        UART::get() << "Send error\n";
    }

#ifdef TRAIN
    motor.setVelocity(0x40);
    motor.forward();
#else

#endif

    while(true) {
        phy.run();
    }
}
