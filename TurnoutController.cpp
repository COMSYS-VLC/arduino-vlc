//
// Created by jan on 01.03.16.
//

#include <avr/interrupt.h>
#include "TurnoutController.hpp"


#define SET_BIT(x, y)   x |=  _BV(y)
#define CLEAR_BIT(x, y) x &= ~_BV(y)

static TurnoutController* turnout = 0;

ISR(TIMER3_COMPB_vect, ISR_NOBLOCK) {
    turnout->isr();
}

// Pin 3/E5: Enable
// Pin 4/G5: Backward / close
// Pin 5/E3: Forward / open

TurnoutController::TurnoutController() :
        mState(true),
        mDelayStep(0)
{
    turnout = this;

    // Setup pins for output
    DDRE = (1 << DDE3) | (1 << DDE5);
    DDRG = (1 << DDG5);

    // Setup timer 3 for PWM on the enable pin
    OCR3A = 0xFF; // one full period
    OCR3B = 0xFF;
    OCR3C = 0xAA; // high time
    TCCR3A = (1 << COM3C1) | (1 << WGM30) | (1 << WGM31);
    TCCR3B = (1 << WGM32) | (1 << WGM33) | (1 << CS32) | (1 << CS30);

    stop();
}

void TurnoutController::isr() {
    ++mDelayStep;

    if(100 == mDelayStep) {
        stop();
        stopTimer();
        mDelayStep = 0;
    }
}

void TurnoutController::startTimer() {
    TIMSK3 = (1 << OCIE3B);
}

void TurnoutController::stopTimer() {
    TIMSK3 = 0x00;
    TIFR3 = (1 << OCF3B);
}

void TurnoutController::open() {
    forward();
    startTimer();
}

void TurnoutController::close() {
    backward();
    startTimer();
}

void TurnoutController::forward() {
    CLEAR_BIT(PORTG, PIN5);
    SET_BIT(PORTE, PIN3);
}

void TurnoutController::backward() {
    CLEAR_BIT(PORTE, PIN3);
    SET_BIT(PORTG, PIN5);
}

void TurnoutController::stop() {
    CLEAR_BIT(PORTE, PIN3);
    CLEAR_BIT(PORTG, PIN5);
}
