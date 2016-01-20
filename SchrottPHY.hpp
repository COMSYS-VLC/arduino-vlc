//
// Created by jan on 13.01.16.
//

#ifndef VLC_SCHROTTPHY_HPP
#define VLC_SCHROTTPHY_HPP

#include "PHY.hpp"
#include "RingBuffer.hpp"

class MAC;

class SchrottPHY : public PHY {
public:
    SchrottPHY();

    virtual void sendPayload(const uint8_t* payload, uint16_t len);

    virtual void run();

    // Internal interrupt methods
    uint8_t synchronize();
    uint8_t doSend();

private:
    // Sensing
    RingBuffer<uint16_t, 32> mAdcBuffer;
    uint32_t mAdcSum;

    bool sense();

    // Synchronisation
    bool mNextIsFirst;
    bool mFirstSample;
    RingBuffer<bool, 1024> mSampleBuffer;

    // Manchester Decoding
    bool mLastSignal;
    bool mDataEdge;

    void decodeManchester(bool signal);

    // Sending
    RingBuffer<uint8_t, 512> mFrameBuffer;
    bool mSendNextIsFirst;
    uint8_t mSendBitOffset;
};


#endif //VLC_SCHROTTPHY_HPP
