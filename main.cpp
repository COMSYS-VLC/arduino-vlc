#include <avr/io.h>
#include <util/atomic.h>
#include "SchrottPHY.hpp"
#include "SchrottMAC.hpp"



int main() {

    SchrottPHY phy;
    SchrottMAC mac(phy);

    sei();

    mac.sendPayload((const uint8_t*)"Hallo Welt!", 11);
    mac.sendPayload((const uint8_t*)"Hallo Welt!", 11);
    mac.sendPayload((const uint8_t*)"Hallo Welt!", 11);
    mac.sendPayload((const uint8_t*)"Hallo Welt!", 11);
    mac.sendPayload((const uint8_t*)"Hallo Welt!", 11);
    mac.sendPayload((const uint8_t*)"Hallo Welt!", 11);
    mac.sendPayload((const uint8_t*)"Hallo Welt!", 11);
    mac.sendPayload((const uint8_t*)"Hallo Welt!", 11);

    /*mac.sendPayload((const uint8_t*)"Hey", 3);
    mac.sendPayload((const uint8_t*)"Hey", 3);
    mac.sendPayload((const uint8_t*)"Hey", 3);
    mac.sendPayload((const uint8_t*)"Hey", 3);*/

    while(true) {
        phy.run();
    }
}
