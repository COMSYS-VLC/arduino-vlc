//
// Created by jan on 13.01.16.
//

#include "SchrottMAC.hpp"
#include "UART.hpp"
#include <util/crc16.h>

SchrottMAC::SchrottMAC(PHY& phy) :
        MAC(phy), mBitOffset(7), mFrameOffset(0), mState(WAIT_BE)
{
    mFrame << 0;

    phy.registerMAC(this);
}

void SchrottMAC::sendPayload(const uint8_t* payload, uint8_t len) {
    uint8_t frame[2 + 1 + len + 2];
    uint8_t *cur = frame;
    *cur++ = 0xBE;
    *cur++ = 0xEF;
    *cur++ = len;
    for(int i = 0; i < len; ++i) {
        *cur++ = payload[i];
    }

    uint16_t crc = 0xFFFF;
    for(int i = 2; i < (3 + len); ++i) {
        crc = _crc_ccitt_update(crc, frame[i]);
    }

    *cur++ = (crc >> 8) & 0xFF;
    *cur = crc & 0xFF;

    UART::get() << "Frame: ";
    for(int i = 0; i < 5 + len; ++i) {
        UART::get() << frame[i] << ' ';
    }
    UART::get() << '\n';

    phy().sendPayload(frame, 5 + len);
    //uint8_t data[] = { 0xE7, 0x39, 0xCE, 0x73, 0x9C, 0xE7, 0x39, 0xCE, 0x73, 0x9C };
    //uint8_t data[] = { 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8 };
    //phy().sendPayload(data, sizeof(data));
}

void SchrottMAC::handleBit(bool bit) {
    if (bit) {
        mFrame.at(mFrame.size() - 1) |= (1 << mBitOffset);
    } else {
        mFrame.at(mFrame.size() - 1) &= ~(1 << mBitOffset);
    }

    if (0 == mBitOffset) {
        mFrame << 0;
        mBitOffset = 7;
    } else {
        --mBitOffset;
    }

    uint16_t fSize;
    bool needMore = false;

    while((fSize = frameSize()) && !needMore) {
        switch(mState) {
        case WAIT_BE:
            if (0xBE != frameByte(0)) {
                shiftFrame();
                break;
            } else {
                mState = WAIT_EF;
                UART::get() << "Frame started\n";
            }

        case WAIT_EF:
            if (1 < fSize) {
                if (0xEF != frameByte(1)) {
                    shiftFrame();
                    mState = WAIT_BE;
                    break;
                } else {
                    mState = WAIT_LENGTH;
                    UART::get() << "BEEF complete\n";
                }
            } else {
                needMore = true;
                break;
            }

        case WAIT_LENGTH:
            if (2 < fSize) {
                uint8_t len = frameByte(2);
                if (len + 5 <= fSize) {
                    UART::get() << "Packet complete: " << fSize << " bytes \n";
                    uint16_t crc = 0xFFFF;
                    for (int i = 2; i < (len + 3); ++i) {
                        crc = _crc_ccitt_update(crc, frameByte(i));
                    }
                    uint16_t crcPacket = ((uint16_t) frameByte(fSize - 2)) << 8;
                    crcPacket |= frameByte(fSize - 1);
                    if (crc != crcPacket) {
                        UART::get() << "Packet dropped: CRC " << crcPacket << " vs. " << crc << '\n';
                        shiftFrame();
                    } else {
                        for (uint8_t i = 0; i < len; ++i) {
                            UART::get() << (char)frameByte(i + 3);
                        }
                        UART::get() << '\n';
                        for (uint16_t i = 0; i < len + 5; ++i) {
                            mFrame.pop();
                        }
                    }
                    mState = WAIT_BE;
                } else {
                    needMore = true;
                    break;
                }
            } else {
                needMore = true;
                break;
            }
        }
    }

}

uint8_t SchrottMAC::frameByte(uint16_t offset) {
    uint8_t byte = mFrame.at(offset);
    if(0 != mFrameOffset) {
        byte <<= mFrameOffset;
        byte |= mFrame.at(offset + 1) >> (8 - mFrameOffset);
    }
    return byte;
}

void SchrottMAC::shiftFrame() {
    ++mFrameOffset;
    if(8 == mFrameOffset) {
        mFrame.pop();
        mFrameOffset = 0;
    }
}

uint16_t SchrottMAC::frameSize() {
    uint16_t size = mFrame.size() - 1;
    if(mFrameOffset > (7 - mBitOffset)) {
        --size;
    }
    return size;
}
