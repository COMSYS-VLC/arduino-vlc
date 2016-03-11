#include "VLCMAC.hpp"
#include "LEDController.hpp"
#include <util/crc16.h>

VLCMAC::VLCMAC(PHY& phy) :
        MAC(phy), mBitOffset(7), mFrameOffset(0), mState(WAIT_BE),
        mPayloadId(0), mAckId(false), mSendAck(false), mSendAckId(false)
{
    mFrame << 0;

    for(uint8_t i = 0; i < 5; ++i) {
        mPayloads[i].used = false;
    }
}

uint8_t VLCMAC::sendPayload(const uint8_t* payload, uint8_t len) {
    if(0 == len || 31 < len) {
        return 0xFE;
    }

    // Find free payload slot
    uint8_t id = (mPayloadId + 1) % 5;
    for(uint8_t i = 0; i < 5; ++i) {
        if(!mPayloads[id].used) {
            break;
        }
        id = (id + 1) % 5;
    }
    if(mPayloads[id].used) {
        return 0xFF;
    }


    mPayloads[id].len = len;
    for(uint8_t i = 0; i < len; ++i) {
        mPayloads[id].data[i] = payload[i];
    }

    if(!mPayloads[mPayloadId].used) {
        mPayloads[id].used = true;
        scheduleNext();
    } else {
        mPayloads[id].used = true;
    }

    return id;
}

void VLCMAC::cancelPayload(uint8_t id) {
    if(5 < id) {
        return;
    }
    mPayloads[id].used = false;
    if(mPayloadId == id) {
        scheduleNext();
    }
}

void VLCMAC::handleBit(bool bit) {
    if (bit) {
        mFrame.at(mFrame.size() - 1) |= (1 << mBitOffset);
    }

    if (0 == mBitOffset) {
        mFrame << 0;
        mBitOffset = 7;
    } else {
        --mBitOffset;
    }

    uint8_t fSize;
    bool needMore = false;

    while((fSize = frameSize()) && !needMore) {
        switch(mState) {
        case WAIT_BE:
            if (0xBE != frameByte(0)) {
                shiftFrame();
                break;
            } else {
                mState = WAIT_EF;
            }

        case WAIT_EF:
            if (1 < fSize) {
                if (0xEF != frameByte(1)) {
                    shiftFrame();
                    mState = WAIT_BE;
                    break;
                } else {
                    mState = WAIT_LENGTH;
                }
            } else {
                needMore = true;
                break;
            }

        case WAIT_LENGTH:
            if (3 < fSize) {
                // Validate header checksum to ensure length correctness
                uint8_t crcPacket = frameByte(3);
                uint8_t crc = 0x00;
                for (uint8_t i = 0; i < 3; ++i) {
                    crc = _crc8_ccitt_update(crc, frameByte(i));
                }
                if(crc != crcPacket) {
                    shiftFrame();
                    mState = WAIT_BE;
                } else {
                    uint8_t len = frameByte(2) & 0x1F;
                    if(0 == len || len + 5 <= fSize) {
                        handleFlags(frameByte(2), 0 != len);
                        if(0 != len) {
                            // Validate (Header checksum + Payload) checksum
                            crc = _crc8_ccitt_update(0x00, frameByte(3));
                            uint8_t payload[len];
                            for (uint8_t i = 4, j = 0; i < (len + 4); ++i, ++j) {
                                payload[j] = frameByte(i);
                                crc = _crc8_ccitt_update(crc, payload[j]);
                            }
                            crcPacket = frameByte(len + 4);
                            if (crc != crcPacket) {
                                shiftFrame();
                            } else {
                                callPayloadHandler(payload, len);
                                for (uint8_t i = 0; i < len + 5; ++i) {
                                    mFrame.pop();
                                }
                            }
                        } else {
                            for (uint8_t i = 0; i < 4; ++i) {
                                mFrame.pop();
                            }
                        }
                        mState = WAIT_BE;
                    } else {
                        needMore = true;
                    }
                }
            } else {
                needMore = true;
            }
            break;
        }
    }
}

uint8_t VLCMAC::frameByte(uint8_t offset) {
    uint8_t byte = mFrame.at(offset);
    if(0 != mFrameOffset) {
        byte <<= mFrameOffset;
        byte |= mFrame.at(offset + 1) >> (8 - mFrameOffset);
    }
    return byte;
}

void VLCMAC::shiftFrame() {
    ++mFrameOffset;
    if(8 == mFrameOffset) {
        mFrame.pop();
        mFrameOffset = 0;
    }
}

uint8_t VLCMAC::frameSize() {
    uint8_t size = mFrame.size() - 1;
    if(mFrameOffset > (7 - mBitOffset)) {
        --size;
    }
    return size;
}

void VLCMAC::handleFlags(uint8_t flags, bool doAck) {
    bool update = false;

    if(mSendAck != doAck) {
        update = true;
        mSendAck = doAck;
    }

    bool sackId = flags & (1 << 7);
    if(mSendAck && mSendAckId != sackId) {
        update = true;
        mSendAckId = sackId;
    }

    if(flags & (1 << 6)) { // acking
        bool ackId = flags & (1 << 5);
        if(mAckId == ackId && mPayloads[mPayloadId].used) {
            LEDController::toggle(LEDController::Debug);
            mPayloads[mPayloadId].used = false;
            uint8_t oldId = mPayloadId;
            scheduleNext(); // schedule next before calling client code
            callAckHandler(oldId);
            update = false;
        }
    }

    if(update) {
        setPayload();
    }
}

void VLCMAC::scheduleNext() {
    // Find used payload slot
    uint8_t id = (mPayloadId + 1) % 5;
    for(uint8_t i = 0; i < 5; ++i) {
        if(mPayloads[id].used) {
            break;
        }
        id = (id + 1) % 5;
    }
    if(mPayloads[id].used) {
        mAckId = !mAckId;
    }
    mPayloadId = id;
    setPayload();
}

void VLCMAC::setPayload() {
    if(!mSendAck && !mPayloads[mPayloadId].used) {
        phy().clearPayload();
        return;
    }

    uint8_t len = mPayloads[mPayloadId].used ? mPayloads[mPayloadId].len : 0;
    uint8_t frame[2 + 2 + len + 1];
    uint8_t *cur = frame;

    // Magic number
    *cur++ = 0xBE;
    *cur++ = 0xEF;

    // Header fields
    uint8_t flaglen = len;
    if(mSendAck) {
        flaglen |= (1 << 6);
        if(mSendAckId) {
            flaglen |= (1 << 5);
        }
    }
    if(mAckId && 0 < len) {
        flaglen |= (1 << 7);
    }
    *cur++ = flaglen;

    // Header checksum
    uint8_t crc = 0x00;
    for(uint8_t i = 0; i < 3; ++i) {
        crc = _crc8_ccitt_update(crc, frame[i]);
    }
    *cur++ = crc;

    if(0 < len) {
        // Payload
        for (uint8_t i = 0; i < len; ++i) {
            *cur++ = mPayloads[mPayloadId].data[i];
        }

        // (Header checksum + Payload) checksum
        crc = 0x00;
        for (uint8_t i = 3; i < (4 + len); ++i) {
            crc = _crc8_ccitt_update(crc, frame[i]);
        }
        *cur = crc;

        phy().setPayload(frame, 5 + len);
    } else {
        phy().setPayload(frame, 4);
    }
}
