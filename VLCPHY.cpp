#include "VLCPHY.hpp"
#include <avr/io.h>
#include <avr/interrupt.h>
#include "LEDController.hpp"

/** Current instance of this PHY for handling of interrupts */
static VLCPHY* currentPHY = 0;

VLCPHY::VLCPHY() :
    PHY(),
    mTimestep(0),
    mSynchronizing(false),
    mSyncState(NoSync),
    mNumEdges(0),
    mSendBuffer(0),
    mNextSendBuffer(0),
    mSendStep(0),
    mHasData(false),
    mSendBitH(false),
    mSendBitL(false)
{
    currentPHY = this;

    // Initialize Timer 4: 24 kHz
    OCR4A = 660;
    TCCR4B = (1 << CS40) | (1 << WGM42);
    TIMSK4 = (1 << OCIE4A);

    // edge detector
    EIMSK = (1 << INT4);
    EICRB = (1 << ISC40);
}

/** ISR for time progress within the PHY, handled by sending code */
ISR(TIMER4_COMPA_vect) {
    currentPHY->doSend();
}

/** ISR getting called on every signal change from hardware edge "detector" */
ISR(INT4_vect, ISR_BLOCK) {
    bool signal = PINE & (1 << PINE4);
    currentPHY->onEdge(signal);
}

void VLCPHY::setPayload(const uint8_t* payload, uint16_t len) {
    Buffer *buf = !mNextSendBuffer ? &mSendBuffer1 :
                  (mNextSendBuffer == &mSendBuffer1 ? &mSendBuffer2 : &mSendBuffer1);

    for (uint8_t i = 0; i < len; ++i) {
        buf->data[i] = payload[i];
    }
    buf->len = len;
    buf->offset = 0;
    buf->bitOffset = 0;

    mNextSendBuffer = buf;
}

void VLCPHY::clearPayload() {
    mNextSendBuffer = 0;
}

void VLCPHY::sync(bool send) {
    mTimestep = 1;
    mNumEdges = 0;
    if(send) {
        mSendStep = 1;
    }
    TCNT4 = 0;
    LEDController::on(LEDController::TX);
    TIFR4 |= (1 << OCF4A);
}

void VLCPHY::resync() {
    mSyncState = NoSync;
    sync(false);
}

void VLCPHY::run() {
    while (mSampleBuffer.size()) {
        uint8_t sample = mSampleBuffer.pop();
        callBitHandler(sample & 2);
        callBitHandler(sample & 1);
    }
}

void VLCPHY::doSend() {
    ++mTimestep;

    switch (mSendStep) {
        case 0: // SyncUp
            LEDController::on(LEDController::TX);
            break;
        case 3: // Data1Down
            if(mHasData && !mSendBitH) {
                LEDController::off(LEDController::TX);
            }
            break;
        case 5: // Data1Up
            if(mHasData && !mSendBitH) {
                LEDController::on(LEDController::TX);
            }
            break;
        case 7: // Data2Down
            if(mHasData && mSendBitH) {
                LEDController::off(LEDController::TX);
            }
            break;
        case 9: // Data2Up
            if(mHasData && mSendBitH) {
                LEDController::on(LEDController::TX);
            }
            break;
        case 12: // SyncDown
            LEDController::off(LEDController::TX);
            break;
        case 15: // Data3Up
            if(mHasData && !mSendBitL) {
                LEDController::on(LEDController::TX);
            }
            break;
        case 17: // Data3Down
            if(mHasData && !mSendBitL) {
                LEDController::off(LEDController::TX);
            }
            break;
        case 19: // Data4Up
            if(mHasData && mSendBitL) {
                LEDController::on(LEDController::TX);
            }
            break;
        case 21: // Data4Down
            if(mHasData && mSendBitL) {
                LEDController::off(LEDController::TX);
            }
            break;
        default:
            break;
    }

    ++mSendStep;
    if(24 == mSendStep) {
        mSendStep = 0;
    }
    if(3 == mSendStep) {
        if(mNextSendBuffer != mSendBuffer) {
            mSendBuffer = mNextSendBuffer;
        } else if(mHasData) {
            mSendBuffer->bitOffset += 2;
            if(8 == mSendBuffer->bitOffset) {
                mSendBuffer->bitOffset = 0;
                ++mSendBuffer->offset;
                if(mSendBuffer->offset == mSendBuffer->len) {
                    mSendBuffer->offset = 0;
                }
            }
        }
        mHasData = mSendBuffer;
        if(mHasData) {
            uint8_t byte = mSendBuffer->data[mSendBuffer->offset];
            mSendBitH = byte & (128 >> mSendBuffer->bitOffset);
            mSendBitL = byte & (64 >> mSendBuffer->bitOffset);
        }
    }
}

void VLCPHY::onEdge(bool signal) {
    uint8_t now = mTimestep;

    // More (too?) strict variant commented
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
}

