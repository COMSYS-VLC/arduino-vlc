//
// Created by jan on 13.01.16.
//

#ifndef VLC_SCHROTTMAC_HPP
#define VLC_SCHROTTMAC_HPP

#include "MAC.hpp"
#include "RingBuffer.hpp"

class SchrottMAC : public MAC {
public:
    SchrottMAC(PHY& phy);

    virtual void sendPayload(const uint8_t* payload, uint8_t len);
    virtual void handleBit(bool bit);

private:
    enum ParseState {
        WAIT_BE,
        WAIT_EF,
        WAIT_LENGTH
    };

    RingBuffer<uint8_t, 260> mFrame;
    uint8_t mBitOffset;
    uint8_t mFrameOffset;
    ParseState mState;

    uint8_t frameByte(uint8_t offset);
    void shiftFrame();
    uint16_t frameSize();
};


#endif //VLC_SCHROTTMAC_HPP
