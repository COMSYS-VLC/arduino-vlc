//
// Created by jan on 05.03.16.
//

#include "LEDController.hpp"

#include <avr/io.h>

const LEDController::LEDInfo LEDController::mInfo[6] = {
    {&PORTJ, _BV(PORTJ1)},
    {&PORTF, _BV(PORTF0)},
    {&PORTF, _BV(PORTF1)},
    {&PORTF, _BV(PORTF2)},
    {&PORTF, _BV(PORTF3)},
    {&PORTF, _BV(PORTF4)}
};

LEDController LEDController::mInit;

LEDController::LEDController() {
    DDRF |= _BV(DDF0) | _BV(DDF1) | _BV(DDF2) | _BV(DDF3) | _BV(DDF4);
    DDRJ |= _BV(DDJ1);
    PORTF |= _BV(PORTF0) | _BV(PORTF1) | _BV(PORTF2) | _BV(PORTF3) | _BV(PORTF4);
    PORTJ |= _BV(PORTJ1);
}