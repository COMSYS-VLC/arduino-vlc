//
// Created by jan on 13.01.16.
//

#include "SchrottPHY.hpp"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include "MAC.hpp"
#include "UART.hpp"

#define SET_BIT(x, y) x |= _BV(y)
#define CLEAR_BIT(x, y) x &= ~_BV(y)
#define TOGGLE_BIT(x, y) x ^= _BV(y)
#define APPLY_BIT(x, y, z) (z) ? (SET_BIT(x, y)) : (CLEAR_BIT(x, y))

#define PHYDDR_OUT DDRB
#define PHYPORT_OUT PORTB
#define PHYPIN_OUT PB5
#define PHYPIN_DBG PB6

#define PHYDDR_IN DDRF
#define PHYPORT_IN PORTF
#define PHYPIN_IN PF0

static SchrottPHY* currentPHY = 0;

SchrottPHY::SchrottPHY() :
    PHY(),
    mTimestep(0),
    mSynchronizing(false),
    mSyncState(NoSync),
    mNumEdges(0),
    mSendBuffer(0),
    mSendStep(0),
    mHasData(false),
    mSendBitH(false),
    mSendBitL(false),
    mPause(0)
{
    currentPHY = this;

    // configure pins
    SET_BIT(PHYDDR_OUT, PHYPIN_OUT);
    CLEAR_BIT(PHYPORT_OUT, PHYPIN_OUT);
    SET_BIT(PHYDDR_OUT, PHYPIN_DBG);
    CLEAR_BIT(PHYPORT_OUT, PHYPIN_DBG);

    CLEAR_BIT(PHYDDR_IN, PHYPIN_IN);
    CLEAR_BIT(PHYPORT_IN, PHYPIN_IN);

    SET_BIT(PHYPORT_OUT, PHYPIN_OUT);

    // Initialize Timer 4: 24 kHz
    OCR4A = 1110;
    TCCR4B = (1 << CS40) | (1 << WGM42);
    TIMSK4 = (1 << OCIE4A);

    // edge detector
    EIMSK = (1 << INT4);
    EICRB = (1 << ISC40);
}

void SchrottPHY::setPayload(const uint8_t* payload, uint16_t len) {
    Buffer *buf = !mSendBuffer ? &mSendBuffer1 : (mSendBuffer == &mSendBuffer1 ? &mSendBuffer2 : &mSendBuffer1);

    for (uint8_t i = 0; i < len; ++i) {
        buf->data[i] = payload[i];
    }
    buf->len = len;
    buf->offset = 0;
    buf->bitOffset = 0;

    mSendBuffer = buf;
}

ISR(TIMER4_COMPA_vect) {
    currentPHY->doSend();
}

ISR(INT4_vect, ISR_BLOCK) {
    bool signal = PINE & (1 << PINE4);
    currentPHY->onEdge(signal);
    //TOGGLE_BIT(PHYPORT_OUT, PHYPIN_DBG);
}

void SchrottPHY::sync(bool send) {
    mTimestep = 1;
    mNumEdges = 0;
    if(send) {
        mSendStep = 1;
        TOGGLE_BIT(PHYPORT_OUT, PHYPIN_DBG);
    }
    TCNT4 = 0;
    SET_BIT(PHYPORT_OUT, PHYPIN_OUT);
    TIFR4 |= (1 << OCF4A);
}

void SchrottPHY::resync() {
    mSyncState = NoSync;
    sync(false);
}

void SchrottPHY::run() {
    while (mSampleBuffer.size()) {
        UART::get() << '\n' << mSampleBuffer.size() << '\n';
        uint8_t sample = mSampleBuffer.pop();
        if (mac()) {
            mac()->handleBit(sample & 2);
            mac()->handleBit(sample & 1);
        }
    }
}

void SchrottPHY::doSend() {
    //TOGGLE_BIT(PHYPORT_OUT, PHYPIN_DBG);

    ++mTimestep;

    switch (mSendStep) {
        case 0: // SyncUp
            SET_BIT(PHYPORT_OUT, PHYPIN_OUT);
            break;
        case 3: // Data1Down
            if(mHasData && !mSendBitH) {
                CLEAR_BIT(PHYPORT_OUT, PHYPIN_OUT);
            }
            break;
        case 5: // Data1Up
            if(mHasData && !mSendBitH) {
                SET_BIT(PHYPORT_OUT, PHYPIN_OUT);
            }
            break;
        case 7: // Data2Down
            if(mHasData && mSendBitH) {
                CLEAR_BIT(PHYPORT_OUT, PHYPIN_OUT);
            }
            break;
        case 9: // Data2Up
            if(mHasData && mSendBitH) {
                SET_BIT(PHYPORT_OUT, PHYPIN_OUT);
            }
            break;
        case 12: // SyncDown
            CLEAR_BIT(PHYPORT_OUT, PHYPIN_OUT);
            break;
        case 15: // Data3Up
            if(mHasData && !mSendBitL) {
                SET_BIT(PHYPORT_OUT, PHYPIN_OUT);
            }
            break;
        case 17: // Data3Down
            if(mHasData && !mSendBitL) {
                CLEAR_BIT(PHYPORT_OUT, PHYPIN_OUT);
            }
            break;
        case 19: // Data4Up
            if(mHasData && mSendBitL) {
                SET_BIT(PHYPORT_OUT, PHYPIN_OUT);
            }
            break;
        case 21: // Data4Down
            if(mHasData && mSendBitL) {
                CLEAR_BIT(PHYPORT_OUT, PHYPIN_OUT);
            }
            break;
        default:
            break;
    }

    mSendStep = (mSendStep + 1) % 24;
    if(3 == mSendStep) {
        if(mHasData) {
            mSendBuffer->bitOffset += 2;
            if(8 == mSendBuffer->bitOffset) {
                mSendBuffer->bitOffset = 0;
                mSendBuffer->offset = (mSendBuffer->offset + 1) % mSendBuffer->len;
            }
        }
        if(mPause < 20) {
            mHasData = mSendBuffer;
            ++mPause;
        } else {
            if(mPause > 25) {
                mPause = 0;
            } else {
                ++mPause;
            }
            mHasData = false;
        }
        if(mHasData) {
            uint8_t byte = mSendBuffer->data[mSendBuffer->offset];
            static bool toggle = false;
            if(toggle) {
                mSendBitH = true;//byte & (128 >> mSendBuffer->bitOffset);
                mSendBitL = false;//byte & (64 >> mSendBuffer->bitOffset);
            } else {
                mSendBitH = false;//byte & (128 >> mSendBuffer->bitOffset);
                mSendBitL = true;//byte & (64 >> mSendBuffer->bitOffset);
            }
            toggle = !toggle;
        }
    }
}

void SchrottPHY::onEdge(bool signal) {
    uint8_t now = mTimestep;

    if (mSynchronizing || FullSync == mSyncState) {
        // Synchronized or synchronizing
        if (mNumEdges < 10) {
            mEdgeTimes[mNumEdges] = now;
            ++mNumEdges;
        }

        if (signal && 22 < now) {
            // Likely to be a SyncUp...
            if (2 == mNumEdges) { // SyncDown + SyncUp (idle frame)
                // No data edges in the first half, synchronize with this edge...

                sync(true);
                mSynchronizing = false;
                mSyncState = FullSync;
            } else if (3 < mNumEdges) { // Data{1,2}Down, Data{1,2}Up, SyncDown, Data{3,4}Up, ... (any data frame)
                // must be SyncUp, validate
                if (6 == mNumEdges) { // Data{1,2}Down, Data{1,2}Up, SyncDown, Data{3,4}Up, Data{3,4}Down, SyncUp
                    // Determine half-time
                    uint8_t half = mEdgeTimes[2];
                    uint8_t quarter = half / 2;
                    uint8_t lateQuarter = (now - half) / 2 + half;
                    if (mEdgeTimes[0] < quarter) {
                        // Looks like Data1Down, check for Data1Up
                        //if (mEdgeTimes[1] < quarter) {
                            // yep, Data1! Check Data3...
                            if (mEdgeTimes[3] < lateQuarter/* && mEdgeTimes[4] < lateQuarter*/) {
                                // OK
                                sync(true);
                                mSampleBuffer << 0;
                                mSynchronizing = false;
                                mSyncState = FullSync;
                            } else {
                                // NO SYNC!
                                //resync();
                                sync(true);
                                mSampleBuffer << 1;
                                mSynchronizing = false;
                                mSyncState = FullSync;
                            }
                        /*} else {
                            // NO SYNC!
                            resync();
                        }*/
                    } else {
                        // Looks like Data2Down, Data2Up has guaranteed happened before SyncDown, so...just take it
                        // check for Data4
                        if (mEdgeTimes[3] < lateQuarter/* && mEdgeTimes[4] >= lateQuarter*/) {
                            // OK
                            sync(true);
                            mSampleBuffer << 2;
                            mSynchronizing = false;
                            mSyncState = FullSync;
                        } else {
                            // NO SYNC!
                            //resync();
                            sync(true);
                            mSampleBuffer << 3;
                            mSynchronizing = false;
                            mSyncState = FullSync;
                        }
                    }
                } else {
                    // NO SYNC!
                    resync();
                }
            }
        } else if (now > 25) {
            // NO SYNC!
            if (signal) {
                resync();
                mSynchronizing = true;
            } else {
                mSyncState = NoSync;
                mSynchronizing = false;
            }
        }
    } else {
        // Not yet synchronized or synchronizing
        if (signal) {
            // synchronize with this edge
            sync(false);
            mSynchronizing = true;
        }
    }

    //APPLY_BIT(PHYPORT_OUT, PHYPIN_DBG, mSyncState == FullSync);
}

