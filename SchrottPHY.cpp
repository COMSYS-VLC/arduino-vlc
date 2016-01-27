//
// Created by jan on 13.01.16.
//

#include "SchrottPHY.hpp"
#include <avr/io.h>
#include <util/delay.h>
#include <util/atomic.h>
#include <avr/interrupt.h>
#include "UART.hpp"
#include "MAC.hpp"

#define SET_BIT(x, y) x |= _BV(y)
#define CLEAR_BIT(x, y) x &= ~_BV(y)
#define TOGGLE_BIT(x, y) x ^= _BV(y)
#define APPLY_BIT(x, y, z) (z) ? (SET_BIT(x, y)) : (CLEAR_BIT(x, y))

#define PHYDDR_OUT DDRB
#define PHYPORT_OUT PORTB
#define PHYPIN_OUT PB7
#define PHYPIN_DBG PB6

#define PHYDDR_IN DDRF
#define PHYPORT_IN PORTF
#define PHYPIN_IN PF0

static SchrottPHY* currentPHY = 0;

SchrottPHY::SchrottPHY() :
    PHY(),
    mAdcSum(0),
    mNextIsFirst(true),
    mFirstSample(false),
    mLastSignal(false),
    mDataEdge(true),
    mSendNextIsFirst(true),
    mSendBitOffset(0)
{
    currentPHY = this;

    SET_BIT(PHYDDR_OUT, PHYPIN_OUT);
    CLEAR_BIT(PHYPORT_OUT, PHYPIN_OUT);
    SET_BIT(PHYDDR_OUT, PHYPIN_DBG);
    CLEAR_BIT(PHYPORT_OUT, PHYPIN_DBG);

    CLEAR_BIT(PHYDDR_IN, PHYPIN_IN);
    CLEAR_BIT(PHYPORT_IN, PHYPIN_IN);

    // Disable input buffer on pin.
    SET_BIT(DIDR0, ADC0D);
    // Start ADC.
    ADMUX |= (1 << REFS0);
    ADCSRA |= (7 << ADPS0) | /*(1 << ADIE) | (1 << ADATE) | */ (1 << ADEN); //| (1 << ADSC);

    // Initialize Timer
    OCR0A = 255;
    TCCR0A = (1 << WGM01);
    TCCR0B = (1 << CS02) | (1 << CS00);
    TIMSK0 = (1 << OCIE0A);
}

void SchrottPHY::sendPayload(const uint8_t* payload, uint16_t len) {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        for (uint8_t i = 0; i < len; ++i) {
            mFrameBuffer << payload[i];
        }
    }
}

ISR(TIMER0_COMPA_vect) {
    static uint8_t delaySync = 1;
    static uint8_t delaySend = 1;
    if(0 == --delaySync) {
        delaySync = currentPHY->synchronize() * 5;
    }
    if(0 == --delaySend) {
        TOGGLE_BIT(PHYPORT_OUT, PHYPIN_DBG);
        delaySend = currentPHY->doSend() * 5;
    }
}

void SchrottPHY::decodeManchester(bool signal) {
    if(mDataEdge) {
        if(signal != mLastSignal) {
            if(mac()) {
                //UART::get() << signal << '\n';
                mac()->handleBit(signal);
            }
            mDataEdge = false;
        } else {
            if(mac()) {
                UART::get() << 'x';
                mac()->handleBit(mLastSignal);
            }
        }
    } else {
        mDataEdge = true;
    }

    mLastSignal = signal;
}

uint8_t SchrottPHY::synchronize() {
    if(mNextIsFirst) {
        mNextIsFirst = false;
        mFirstSample = sense();
        return 8;
    } else {
        mNextIsFirst = true;
        bool sample2 = sense();

        if(mFirstSample != sample2) {
            UART::get() << 'y';
            return 3;
        } else {
            ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
                mSampleBuffer << mFirstSample;
            }
            return 2;
        }
    }
}

bool SchrottPHY::sense() {
    ADCSRA |= (1 << ADSC);
    while(!(ADCSRA & (1 << ADIF))) {
        // sampling...
    }

    uint16_t value = ADCL;
    value |= ((uint16_t) ADCH) << 8;

    if(mAdcBuffer.size() == mAdcBuffer.capacity()) {
        mAdcSum -= mAdcBuffer.at(0);
    }
    mAdcBuffer << value;
    mAdcSum += value;

    uint16_t avg = mAdcSum / mAdcBuffer.size();
    int16_t diff = (int16_t)avg - value;

    if (diff > 0 && (uint16_t)diff > (40)) {
        //SET_BIT(PHYPORT_OUT, PHYPIN_OUT);
        return true;
        //SET_BIT(PHYPORT_OUT, PHYPIN_OUT);
    } else {
        //CLEAR_BIT(PHYPORT_OUT, PHYPIN_OUT);
        return false;
        //CLEAR_BIT(PHYPORT_OUT, PHYPIN_OUT);
    }

    //UART::get() << mAdcBuffer.size() << ' ' << avg << ' ' << diff << '\n';
}

void SchrottPHY::run() {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        while(mSampleBuffer.size()) {
            bool sample = mSampleBuffer.pop();
            NONATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
                decodeManchester(sample);
            }
        }
    }
}

uint8_t SchrottPHY::doSend() {
    uint8_t byte = 0;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        if(mFrameBuffer.empty()) {
            return 10;
        }
        byte = mFrameBuffer.at(0);
    }

    if(mSendNextIsFirst) {
        if(byte & _BV(mSendBitOffset)) {
            CLEAR_BIT(PHYPORT_OUT, PHYPIN_OUT);
        } else {
            SET_BIT(PHYPORT_OUT, PHYPIN_OUT);
        }
        mSendNextIsFirst = false;
    } else {
        if(byte & _BV(mSendBitOffset)) {
            SET_BIT(PHYPORT_OUT, PHYPIN_OUT);
        } else {
            CLEAR_BIT(PHYPORT_OUT, PHYPIN_OUT);
        }
        if(8 == ++mSendBitOffset) {
            mSendBitOffset = 0;
            ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
                mFrameBuffer.pop();

                if(mFrameBuffer.empty()) {
                    CLEAR_BIT(PHYPORT_OUT, PHYPIN_OUT);
                }
            }
        }
        mSendNextIsFirst = true;
    }
    return 10;
}
