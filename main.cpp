#include <avr/io.h>
#include <util/atomic.h>
#include <util/delay.h>
#include "SchrottPHY.hpp"
#include "SchrottMAC.hpp"



int main() {
    SchrottPHY phy;
    SchrottMAC mac(phy);

    sei();

    //mac.sendPayload((const uint8_t*)"Hallo Welt!", 11);

    mac.sendPayload((const uint8_t*)"Hey", 3);

    DDRE = (1 << DDE3) | (1 << DDE5);
    DDRG = (1 << DDG5);

    OCR3A = 0xFF;
    //OCR3C = 0xA0;
    TCCR3A = (1 << COM3C1) | (1 << WGM30) | (1 << WGM31);
    TCCR3B = (1 << WGM32) | (1 << WGM33) | (1 << CS32) | (1 << CS30);

    //PORTE = (1 << PORTE5); // Pin 3 (Enable)
    //PORTG = (1 << PORTG5); // Pin 4 (Forward)
    //PORTE = (1 << PORTE3); // Pin 5 (Backward)

    /*
    while(true) {
        PORTG = (1 << PORTG5); // Pin 4 (Forward)
        PORTE = 0;
        for(uint8_t i = 0; i < 0xA0; ++i) {
            OCR3C = i;
            _delay_ms(50);
        }
        PORTE = (1 << PORTE3); // Pin 5 (Backward)
        PORTG = 0;
        for(uint8_t i = 0xA0; i >= 0x00; --i) {
            OCR3C = i;
            _delay_ms(50);
        }
    }
    */

    while(true) {
        phy.run();
    }
}
