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

    virtual uint8_t sendPayload(const uint8_t* payload, uint8_t len);
    virtual void cancelPayload(uint8_t id);

protected:
    virtual void handleBit(bool bit);

private:
    enum ParseState {
        WAIT_BE,
        WAIT_EF,
        WAIT_LENGTH
    };

    RingBuffer<uint8_t, 63> mFrame;
    uint8_t mBitOffset;
    uint8_t mFrameOffset;
    ParseState mState;

    struct Payload {
        bool used;
        uint8_t len;
        uint8_t data[31];
    };
    Payload mPayloads[5];
    uint8_t mPayloadId;
    bool mAckId;
    bool mSendAck;
    bool mSendAckId;

    uint8_t frameByte(uint8_t offset);
    void shiftFrame();
    uint8_t frameSize();

    void handleFlags(uint8_t flags, bool doAck);
    void scheduleNext();
    void setPayload();
};


#endif //VLC_SCHROTTMAC_HPP
