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
    mAdcCount(0),
    mAdcAvg(0),
    mLastSignal(false),
    mEdgeDetected(false),
    mSyncState(NoSync),
    mNextEdge(SyncUp),
    mPeriodStep(0),
    mIsData(false),
    mDataValue(false),
    mSendStep(0),
    mSendBitOffset(0)
{
    currentPHY = this;

    // configure pins
    SET_BIT(PHYDDR_OUT, PHYPIN_OUT);
    CLEAR_BIT(PHYPORT_OUT, PHYPIN_OUT);
    SET_BIT(PHYDDR_OUT, PHYPIN_DBG);
    CLEAR_BIT(PHYPORT_OUT, PHYPIN_DBG);

    CLEAR_BIT(PHYDDR_IN, PHYPIN_IN);
    CLEAR_BIT(PHYPORT_IN, PHYPIN_IN);

    // Initialize Timer 0: 10 ms
    OCR0A = 77;
    TCCR0A = (1 << WGM01);
    TCCR0B = (1 << CS02) | (1 << CS00);
    TIMSK0 = (1 << OCIE0A);

    // Initialize Timer 2: 2.5 ms
    OCR2A = 77;
    TCCR2A = (1 << WGM21);
    TCCR2B = (1 << CS22) | (1 << CS21);
    TIMSK2 = (1 << OCIE2A);

    // Disable input buffer on pin.
    SET_BIT(DIDR0, ADC0D);
    // Start ADC.
    ADMUX |= (1 << REFS0);
    ADCSRA |= (7 << ADPS0) | (1 << ADIE) | (1 << ADATE) | (1 << ADEN) | (1 << ADSC);
}

void SchrottPHY::sendPayload(const uint8_t* payload, uint16_t len) {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        for (uint8_t i = 0; i < len; ++i) {
            mFrameBuffer << payload[i];
        }
    }
}

ISR(TIMER0_COMPA_vect) {
        currentPHY->doSend();
}

ISR(TIMER2_COMPA_vect) {
        currentPHY->synchronize();
}

ISR(ADC_vect) {
    currentPHY->detectEdge();
}

void SchrottPHY::resync() {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        mPeriodStep = 0;
        mEdgeDetected = true;
        mSyncState = NoSync;
        mNextEdge = SyncUp;
        TCNT2 = 0;
    }
}

void SchrottPHY::resetSend() {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        mSendStep = 1;
        mSendBitOffset = 0;
        TCNT0 = 0;
        SET_BIT(PHYPORT_OUT, PHYPIN_OUT);
    }
}

void SchrottPHY::synchronize() {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        switch (mPeriodStep) {
            case 3:
                mNextEdge = NoEdge;
                if (!mEdgeDetected) {
                    clearSync();
                } else {
                    mEdgeDetected = false;
                }
                break;
            case 6:
                if (mSyncState == FullSync) {
                    mEdgeDetected = false;
                    mNextEdge = DataDown;
                }
                break;
            case 10:
                if (mSyncState == FullSync) {
                    if (mEdgeDetected) {
                        mEdgeDetected = false;
                        mNextEdge = DataUp;
                    } else {
                        mIsData = false;
                        mNextEdge = NoEdge;
                    }
                }
                break;
            case 14:
                if (mSyncState == FullSync && mNextEdge == DataUp) {
                    if (mEdgeDetected) {
                        mIsData = true;
                        mNextEdge = NoEdge;
                        mDataValue = true;
                    } else {
                        clearSync();
                    }
                }
                mEdgeDetected = false;
                break;
            case 18:
                mEdgeDetected = false;
                mNextEdge = SyncDown;
                break;
            case 23:
                mNextEdge = NoEdge;
                if (!mEdgeDetected) {
                    clearSync();
                } else {
                    mEdgeDetected = false;
                }
                break;
            case 26:
                if (mSyncState == FullSync && !mIsData) {
                    mEdgeDetected = false;
                    mNextEdge = DataUp;
                }
                break;
            case 30:
                if (mSyncState == FullSync) {
                    if (mEdgeDetected) {
                        mNextEdge = DataDown;
                    } else {
                        mNextEdge = NoEdge;
                    }
                }
                mEdgeDetected = false;
                break;
            case 34:
                if (mSyncState == FullSync && mNextEdge == DataDown) {
                    if (mEdgeDetected) {
                        mIsData = true;
                        mNextEdge = NoEdge;
                        mDataValue = false;
                    } else {
                        clearSync();
                    }
                }
                mEdgeDetected = false;
                break;
            case 38:
                mEdgeDetected = false;
                mNextEdge = SyncUp;
                break;
            default:
                break;
        }

        mPeriodStep = (mPeriodStep + 1) % 40;

        if(0 == mPeriodStep && mSyncState == FullSync && mIsData) {
            //UART::get() << mDataValue << "\n";
            mSampleBuffer << mDataValue;
        }
    }
}

void SchrottPHY::run() {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        while(mSampleBuffer.size()) {
            bool sample = mSampleBuffer.pop();
            NONATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
                if(mac()) {
                    mac()->handleBit(sample);
                }
            }
        }
    }
}

void SchrottPHY::doSend() {
    switch (mSendStep) {
        case 0:
            SET_BIT(PHYPORT_OUT, PHYPIN_OUT);
            break;
        case 2:
            if(!mFrameBuffer.empty() && mFrameBuffer.at(0) & (128 >> mSendBitOffset)) {
                CLEAR_BIT(PHYPORT_OUT, PHYPIN_OUT);
            }
            break;
        case 3:
            if(!mFrameBuffer.empty() && mFrameBuffer.at(0) & (128 >> mSendBitOffset)) {
                SET_BIT(PHYPORT_OUT, PHYPIN_OUT);
            }
            break;
        case 5:
            CLEAR_BIT(PHYPORT_OUT, PHYPIN_OUT);
            break;
        case 7:
            if(!mFrameBuffer.empty() && !(mFrameBuffer.at(0) & (128 >> mSendBitOffset))) {
                SET_BIT(PHYPORT_OUT, PHYPIN_OUT);
            }
            break;
        case 8:
            if(!mFrameBuffer.empty() && !(mFrameBuffer.at(0) & (128 >> mSendBitOffset))) {
                CLEAR_BIT(PHYPORT_OUT, PHYPIN_OUT);
            }
            break;
        default:
            break;
    }

    mSendStep = (mSendStep + 1) % 10;
    if(0 == mSendStep && !mFrameBuffer.empty()) {
        if(8 == ++mSendBitOffset) {
            mSendBitOffset = 0;
            mFrameBuffer.pop();
        }
    }
}

void SchrottPHY::clearSync() {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        mSyncState = NoSync;
        mNextEdge = SyncUp;
        mEdgeDetected = false;
    }
}

void SchrottPHY::detectEdge() {
#define AVERAGE_SAMPLES (512)

    uint16_t value = ADCL;
    value |= ((uint16_t) ADCH) << 8;

    if (mAdcCount < AVERAGE_SAMPLES)
    {
        ++mAdcCount;
        mAdcSum -= mAdcAvg;
        mAdcSum += value;
        mAdcAvg = mAdcSum / mAdcCount;
    }
    else
    {
        mAdcSum -= mAdcAvg;
        mAdcSum += value;
        mAdcAvg = mAdcSum / AVERAGE_SAMPLES;
    }

    int16_t diff = (int16_t) mAdcAvg - value;

    bool signal = diff > 40;

    // Wenn flanke, dann...
    if (signal != mLastSignal)
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            switch (mNextEdge) {
                case SyncDown:
                    if (signal) {
                        resync();
                    } else {
                        if (mSyncState != FullSync) {
                            mSyncState = HalfSync;
                        }
                        mEdgeDetected = true;
                    }
                    break;
                case SyncUp:
                    if (!signal) {
                        clearSync();
                    } else {
                        mEdgeDetected = true;
                        if (mSyncState == HalfSync) {
                            mSyncState = FullSync;
                            resetSend();
                        } else if (mSyncState == NoSync) {
                            resync();
                        }
                    }
                    break;
                case DataUp:
                    if (signal) {
                        mEdgeDetected = true;
                    }
                    break;
                case DataDown:
                    if (!signal) {
                        mEdgeDetected = true;
                    }
                    break;
                case NoEdge:
                    if (mSyncState == FullSync) {
                        if (signal) {
                            resync();
                        } else {
                            clearSync();
                        }
                    }
                    break;
            }
        }
    }

    APPLY_BIT(PHYPORT_OUT, PHYPIN_DBG, mSyncState == FullSync);

    mLastSignal = signal;
}
