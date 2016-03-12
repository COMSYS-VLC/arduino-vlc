#include "Clock.hpp"
#include <avr/io.h>
#include <avr/interrupt.h>

/** centiseconds since the last main thread handling */
static volatile uint8_t timePassed = 0;

/** ISR for increasing the passed time */
ISR(TIMER5_COMPA_vect) {
    ++timePassed;
}

Clock::Clock() {
    for(uint8_t i = 0; SLOTS > i; ++i) {
        mEntries[i].used = false;
    }

    // Initialize Timer 5: 100Hz
    OCR5A = 97;
    TCCR5B = (1 << CS50) | (1 << CS52) | (1 << WGM52);
    TIMSK5 = (1 << OCIE5A);
}

void Clock::execDelayed(uint16_t delayms, Clock::TimerCallback callback, void *data) {
    // find unused slot
    uint8_t i = 0;
    for(; SLOTS > i; ++i) {
        if(!mEntries[i].used) {
            break;
        }
    }
    if(SLOTS <= i) {
        return;
    }

    mEntries[i].callback = callback;
    mEntries[i].data = data;
    mEntries[i].time = delayms;
    mEntries[i].used = true;
}

void Clock::run() {
    uint16_t passed = timePassed;
    timePassed = 0;
    passed *= 10;

    for(uint8_t i = 0; SLOTS > i; ++i) {
        if(mEntries[i].used) {
            if(mEntries[i].time <= passed) {
                mEntries[i].used = false;
                mEntries[i].callback(mEntries[i].data);
            } else {
                mEntries[i].time -= passed;
            }
        }
    }
}
