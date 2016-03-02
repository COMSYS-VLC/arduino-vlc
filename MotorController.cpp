//
// Created by oggy on 26.02.16.
//

#include "MotorController.hpp"
#include <avr/io.h>

#define SET_BIT(x, y)   x |=  _BV(y)
#define CLEAR_BIT(x, y) x &= ~_BV(y)
#define GET_BIT(x, y)   static_cast<bool>(x & _BV(y))

// Pin 3/E5: Enable
// Pin 4/G5: Backward
// Pin 5/E3: Forward

MotorController::MotorController() :
    mVelocity(0xA0)
{
    // Setup pins for output
    DDRE = (1 << DDE3) | (1 << DDE5);
    DDRG = (1 << DDG5);

    // Setup timer 3 for PWM on the enable pin
    OCR3A = 0xFF; // one full period
    OCR3C = mVelocity; // high time
    TCCR3A = (1 << COM3C1) | (1 << WGM30) | (1 << WGM31);
    TCCR3B = (1 << WGM32) | (1 << WGM33) | (1 << CS32) | (1 << CS30);

    stop();
}

void MotorController::forward() {
    CLEAR_BIT(PORTG, PIN5);
    SET_BIT(PORTE, PIN3);
}

void MotorController::backward() {
    CLEAR_BIT(PORTE, PIN3);
    SET_BIT(PORTG, PIN5);
}

void MotorController::stop() {
    CLEAR_BIT(PORTE, PIN3);
    CLEAR_BIT(PORTG, PIN5);
}

void MotorController::setVelocity(uint8_t velocity) {
    mVelocity = velocity;
    OCR3C = velocity;
}

bool MotorController::isForward() const {
    return GET_BIT(PORTE, PIN3);
}
