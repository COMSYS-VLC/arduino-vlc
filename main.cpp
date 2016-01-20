#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "SchrottPHY.hpp"
#include "SchrottMAC.hpp"

int main() {
    SchrottPHY phy;
    SchrottMAC mac(phy);

    sei();

    mac.sendPayload((const uint8_t *) "A", 1);
    mac.sendPayload((const uint8_t *) "A", 1);
    mac.sendPayload((const uint8_t *) "A", 1);
    while (1) {
        phy.run();
        //_delay_ms(5000);
    }
}//190 239 1 65 186 237