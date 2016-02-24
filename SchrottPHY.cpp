//
// Created by jan on 13.01.16.
//

#include "SchrottPHY.hpp"
#include <avr/io.h>
#include <util/delay.h>
#include <util/atomic.h>
#include <avr/interrupt.h>
#include <signal.h>
#include "UART.hpp"
#include "MAC.hpp"

#define SET_BIT(x, y) x |= _BV(y)
#define CLEAR_BIT(x, y) x &= ~_BV(y)
#define TOGGLE_BIT(x, y) x ^= _BV(y)
#define APPLY_BIT(x, y, z) (z) ? (SET_BIT(x, y)) : (CLEAR_BIT(x, y))

#define ABS(x) ((x) >= 0 ? (x) : -(x))

#define SIGNAL_PERIOD (1000) //µs
#define CLOCK_PERIOD (41) // µs
#define SIGNAL_LENGTH (SIGNAL_PERIOD / CLOCK_PERIOD)

#define TIME_SYNC_UP (SIGNAL_LENGTH)
#define TIME_DATA1 (SIGNAL_LENGTH / 6)
#define TIME_DATA2 (SIGNAL_LENGTH / 3)
#define TIME_SYNC_DOWN (SIGNAL_LENGTH / 2)
#define TIME_DATA3 (2 * SIGNAL_LENGTH / 3)
#define TIME_DATA4 (5 * SIGNAL_LENGTH / 6)

#define TIME_OFFSET (SIGNAL_LENGTH / 24)
#define TIME_DATA_BEFORE(x) ((x) - TIME_OFFSET)
#define TIME_DATA_AFTER(x) ((x) + TIME_OFFSET)

#define TIME_DATA1_DOWN TIME_DATA_BEFORE(TIME_DATA1)
#define TIME_DATA1_UP TIME_DATA_AFTER(TIME_DATA1)
#define TIME_DATA2_DOWN TIME_DATA_BEFORE(TIME_DATA2)
#define TIME_DATA2_UP TIME_DATA_AFTER(TIME_DATA2)
#define TIME_DATA3_UP TIME_DATA_BEFORE(TIME_DATA3)
#define TIME_DATA3_DOWN TIME_DATA_AFTER(TIME_DATA3)
#define TIME_DATA4_UP TIME_DATA_BEFORE(TIME_DATA4)
#define TIME_DATA4_DOWN TIME_DATA_AFTER(TIME_DATA4)

#define PHYDDR_OUT DDRB
#define PHYPORT_OUT PORTB
#define PHYPIN_OUT PB5
#define PHYPIN_DBG PB6

#define PHYDDR_IN DDRF
#define PHYPORT_IN PORTF
#define PHYPIN_IN PF0

static SchrottPHY* currentPHY = 0;

static SchrottPHY::EdgeType EdgeLUT[] = { SchrottPHY::NoEdge,
                                          SchrottPHY::Data1Down, SchrottPHY::Data1Down,
                                          SchrottPHY::Data1Up, SchrottPHY::Data1Up,
                                          SchrottPHY::Data2Down, SchrottPHY::Data2Down,
                                          SchrottPHY::Data2Up, SchrottPHY::Data2Up,
                                          SchrottPHY::SyncDown, SchrottPHY::SyncDown, SchrottPHY::SyncDown, SchrottPHY::SyncDown,
                                          SchrottPHY::Data3Up, SchrottPHY::Data3Up,
                                          SchrottPHY::Data3Down, SchrottPHY::Data3Down,
                                          SchrottPHY::Data4Up, SchrottPHY::Data4Up,
                                          SchrottPHY::Data4Down, SchrottPHY::Data4Down,
                                          SchrottPHY::SyncUp, SchrottPHY::SyncUp, SchrottPHY::SyncUp, SchrottPHY::SyncUp };

SchrottPHY::SchrottPHY() :
    PHY(),
    mTimestep(0),
    mSyncState(NoSync),
    mSendStep(0),
    mSendBitOffset(0),
    mHasData(false),
    mSendBitH(false),
    mSendBitL(false),
    mSynchronizing(false),
    mHalfSyncSeen(false),
    mNumEdges(0)
{
    currentPHY = this;

    /*
    for(uint8_t i = TIME_DATA1_DOWN - TIME_OFFSET; i <= TIME_DATA1_DOWN + TIME_OFFSET; ++i) {
        EdgeLUT[i] = Data1Down;
    }
    for(uint8_t i = TIME_DATA1_UP - TIME_OFFSET; i <= TIME_DATA1_UP + TIME_OFFSET; ++i) {
        EdgeLUT[i] = Data1Up;
    }
    for(uint8_t i = TIME_DATA2_DOWN - TIME_OFFSET; i <= TIME_DATA2_DOWN + TIME_OFFSET; ++i) {
        EdgeLUT[i] = Data2Down;
    }
    for(uint8_t i = TIME_DATA2_UP - TIME_OFFSET; i <= TIME_DATA2_UP + TIME_OFFSET; ++i) {
        EdgeLUT[i] = Data2Up;
    }
    for(uint8_t i = TIME_SYNC_DOWN - TIME_OFFSET; i <= TIME_SYNC_DOWN + TIME_OFFSET; ++i) {
        EdgeLUT[i] = SyncDown;
    }
    for(uint8_t i = TIME_DATA3_UP - TIME_OFFSET; i <= TIME_DATA3_UP + TIME_OFFSET; ++i) {
        EdgeLUT[i] = Data3Up;
    }
    for(uint8_t i = TIME_DATA3_DOWN - TIME_OFFSET; i <= TIME_DATA3_DOWN + TIME_OFFSET; ++i) {
        EdgeLUT[i] = Data3Down;
    }
    for(uint8_t i = TIME_DATA4_UP - TIME_OFFSET; i <= TIME_DATA4_UP + TIME_OFFSET; ++i) {
        EdgeLUT[i] = Data4Up;
    }
    for(uint8_t i = TIME_DATA4_DOWN - TIME_OFFSET; i <= TIME_DATA4_DOWN + TIME_OFFSET; ++i) {
        EdgeLUT[i] = Data4Down;
    }
    for(uint8_t i = TIME_SYNC_UP - TIME_OFFSET; i <= TIME_SYNC_UP + TIME_OFFSET; ++i) {
        EdgeLUT[i] = SyncUp;
    }*/

    for(uint8_t i = 0; i < sizeof(EdgeLUT); ++i) {
        UART::get() << EdgeLUT[i] << ' ';
    }

    // configure pins
    SET_BIT(PHYDDR_OUT, PHYPIN_OUT);
    CLEAR_BIT(PHYPORT_OUT, PHYPIN_OUT);
    SET_BIT(PHYDDR_OUT, PHYPIN_DBG);
    CLEAR_BIT(PHYPORT_OUT, PHYPIN_DBG);

    CLEAR_BIT(PHYDDR_IN, PHYPIN_IN);
    CLEAR_BIT(PHYPORT_IN, PHYPIN_IN);

    SET_BIT(PHYPORT_OUT, PHYPIN_OUT);

    // Initialize Timer 1: 50 kHz
    OCR1A = 36;
    //TCCR1B = (1 << CS11) | (1 << WGM12);
    TIMSK1 = (1 << OCIE1A);

    // Initialize Timer 4: 24 kHz
    OCR4A = 666;
    TCCR4B = (1 << CS40) | (1 << WGM42);
    TIMSK4 = (1 << OCIE4A);

    // edge detector
    EIMSK = (1 << INT4);
    EICRB = (1 << ISC40);
}

void SchrottPHY::sendPayload(const uint8_t* payload, uint16_t len) {
    for (uint8_t i = 0; i < len; ++i) {
        mFrameBuffer << payload[i];
    }
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
    }
    TCNT4 = 0;
    SET_BIT(PHYPORT_OUT, PHYPIN_OUT);
    TIFR4 |= (1 << OCF4A);
}

void SchrottPHY::resync() {
    mSyncState = NoSync;
    sync(false);

    /*mSyncState = NoSync;
    mSynchronizing = false;
    mHalfSyncSeen = true;
    mDataSeen[0][0] = mDataSeen[0][1] =
    mDataSeen[1][0] = mDataSeen[1][1] =
    mDataSeen[2][0] = mDataSeen[2][1] =
    mDataSeen[3][0] = mDataSeen[3][1] = false;*/

    //TCNT2 = 0;
    //TCCR2B = (1 << CS22) | (1 << CS21);
    //TCCR2B = (1 << CS22);
    //TCCR2B = (1 << CS21) | (1 << CS20);
    //TIFR2 |= (1 << OCF2A);
}

void SchrottPHY::resetSend() {
    mSendStep = 0;
    mSendBitOffset = 0;
    TCNT0 = 0;
    //SET_BIT(PHYPORT_OUT, PHYPIN_OUT);
}

void SchrottPHY::run() {
    while(mSampleBuffer.size()) {
        uint8_t sample = mSampleBuffer.pop();
        if(mac()) {
            mac()->handleBit(sample & 2);
            mac()->handleBit(sample & 1);
        }
    }
}

void SchrottPHY::doSend() {
    //TOGGLE_BIT(PHYPORT_OUT, PHYPIN_DBG);

    ++currentPHY->mTimestep;
    //APPLY_BIT(PHYPORT_OUT, PHYPIN_DBG, currentPHY->mTimestep % 2);

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
        if(mHasData && 8 == (mSendBitOffset += 2)) {
            mSendBitOffset = 0;
            mFrameBuffer.pop();
        }
        mHasData = !mFrameBuffer.empty();
        if(mHasData) {
            uint8_t byte = mFrameBuffer.at(0);
            mSendBitH = byte & (128 >> mSendBitOffset);
            mSendBitL = byte & (64 >> mSendBitOffset);
        }
    }
}

void SchrottPHY::clearSync() {
    mSyncState = NoSync;
    TCCR2B = 0;
    TIFR2 = (1 << OCF2A);
    TCNT2 = 0;
}

bool SchrottPHY::decodeFrame() {
    bool result = true;
    if(mDataSeen[0][0] != mDataSeen[0][1] ||
            mDataSeen[1][0] != mDataSeen[1][1] ||
            mDataSeen[2][0] != mDataSeen[2][1] ||
            mDataSeen[3][0] != mDataSeen[3][1] ||
            mDataSeen[0][0] != mDataSeen[2][0] ||
            mDataSeen[1][0] != mDataSeen[3][0]) {
        result = false;
        goto reset;
    }

    if(mDataSeen[0][0]) {
        if(mDataSeen[0][0] == mDataSeen[1][0]) {
            result = false;
            goto reset;
        }
        mSampleBuffer << true;
    } else if(mDataSeen[1][0]) {
        if(mDataSeen[0][0] == mDataSeen[1][0]) {
            result = false;
            goto reset;
        }
        mSampleBuffer << false;
    }

reset:
    mDataSeen[0][0] = mDataSeen[0][1] =
    mDataSeen[1][0] = mDataSeen[1][1] =
    mDataSeen[2][0] = mDataSeen[2][1] =
    mDataSeen[3][0] = mDataSeen[3][1] = false;
    return result;
}

void SchrottPHY::onEdge(bool signal) {
    uint8_t now = mTimestep;

    if (mSynchronizing || FullSync == mSyncState) {
        // Synchronized or synchronizing
        if (mNumEdges < 10) {
            edgeTimes[mNumEdges] = now;
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
                    uint8_t half = edgeTimes[2];
                    uint8_t quarter = half / 2;
                    uint8_t lateQuarter = (now - half) / 2 + half;
                    if (edgeTimes[0] < quarter) {
                        // Looks like Data1Down, check for Data1Up
                        //if (edgeTimes[1] < quarter) {
                            // yep, Data1! Check Data3...
                            if (edgeTimes[3] < lateQuarter/* && edgeTimes[4] < lateQuarter*/) {
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
                        if (edgeTimes[3] < lateQuarter/* && edgeTimes[4] >= lateQuarter*/) {
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
        } else if (now > 30) {
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

    APPLY_BIT(PHYPORT_OUT, PHYPIN_DBG, mSyncState == FullSync);

    /*
    //APPLY_BIT(PHYPORT_OUT, PHYPIN_DBG, signal);
    if(signal) {
        if(NoSync == mSyncState) {
            SET_BIT(PHYPORT_OUT, PHYPIN_OUT);
            TCNT4 = 0;
            TIFR4 |= (1 << OCF4A);
            mSendStep = 1;
            mTimestep = 0;
            mSyncState = HalfSync;
        } else if(NoSync != mSyncState) {
            if(mTimestep <= 7) { // Data1Up
                mDataSeen[0][1] = true;
            } else if(mTimestep <= 11) { // Data2Up
                mDataSeen[1][1] = true;
            } else if(mTimestep <= 17) { // Data3Up
                mDataSeen[2][0] = true;
            } else if(mTimestep <= 21) { // Data4Up
                mDataSeen[3][0] = true;
            }
        }
    } else {
        if(NoSync != mSyncState) {
            if(mTimestep <= 5) { // Data1Down
                mDataSeen[0][0] = true;
            } else if(mTimestep <= 9) { // Data2Down
                mDataSeen[1][0] = true;
            } else if(mTimestep <= 15) { // SyncDown
                mSyncState = FullSync;
            } else if(mTimestep <= 19) { // Data3Down
                mDataSeen[2][1] = true;
            } else if(mTimestep <= 23) { // Data4Down
                mDataSeen[3][1] = true;
            }
        }
    }*/

    /*EdgeType type;

    if(NoSync == mSyncState && !mSynchronizing) {
        if(!signal) {
            return;
        }
        type = SyncUp;
    } else {
        if(mTimestep < sizeof(EdgeLUT)) {
            type = EdgeLUT[mTimestep];
        } else {
            type = NoEdge;
        }
        if(NoEdge == type) {
            resync();
        }
    }


    if(signal) {
        switch (type) {
            case SyncUp:
                if(NoSync == mSyncState) {
                    mSynchronizing = true;
                } else {
                    if(decodeFrame()) {
                        mSyncState = FullSync;
                        mSynchronizing = false;
                        TOGGLE_BIT(PHYPORT_OUT, PHYPIN_DBG);
                    } else {
                        mSyncState = NoSync;
                        mSynchronizing = true;
                        //TOGGLE_BIT(PHYPORT_OUT, PHYPIN_DBG);
                    }
                }
                SET_BIT(PHYPORT_OUT, PHYPIN_OUT);
                TCNT4 = 0;
                TIFR4 |= (1 << OCF4A);
                mSendStep = 1;
                mHalfSyncSeen = false;
                mTimestep = 0;
                break;
            case Data1Up:
                if(!mDataSeen[0][0]) {
                    resync();
                    onEdge(signal);
                    return;
                }
                mDataSeen[0][1] = true;
                break;
            case Data2Up:
                if(!mDataSeen[1][0]) {
                    resync();
                    onEdge(signal);
                    return;
                }
                mDataSeen[1][1] = true;
                break;
            case Data3Up:
                mDataSeen[2][0] = true;
                break;
            case Data4Up:
                mDataSeen[3][0] = true;
                break;
            default:
                resync();
                onEdge(signal);
                break;
        }
    } else {
        switch (type) {
            case Data1Down:
                mDataSeen[0][0] = true;
                break;
            case Data2Down:
                mDataSeen[1][0] = true;
                break;
            case SyncDown:
                if(NoSync == mSyncState) {
                    mSyncState = HalfSync;
                }
                mHalfSyncSeen = true;
                break;
            case Data3Down:
                if(!mDataSeen[2][0]) {
                    resync();
                    return;
                }
                mDataSeen[2][1] = true;
                break;
            case Data4Down:
                if(!mDataSeen[3][0]) {
                    resync();
                    return;
                }
                mDataSeen[3][1] = true;
                break;
            default:
                resync();
                break;
        }
    }*/

    /*
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
                    } else {
                        if(0 != mSendStep) {
                            if(!mFrameBuffer.empty() && 8 == ++mSendBitOffset) {
                                mSendBitOffset = 0;
                                mFrameBuffer.pop();
                            }
                            mSendStep = 0;
                        }

                        //mSendStep = 0;
                        SET_BIT(PHYPORT_OUT, PHYPIN_OUT);
                        TCNT0 = 0;
                        TIFR0 |= (1 << OCF0A);

                        if(37 <= mPeriodStep && mIsData) {
                            mSampleBuffer << mDataValue;
                        }

                        mPeriodStep = 0;
                        TCNT2 = 0;
                        TIFR2 |= (1 << OCF2A);
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
    */
    //APPLY_BIT(PHYPORT_OUT, PHYPIN_DBG, mSyncState == FullSync);
}

