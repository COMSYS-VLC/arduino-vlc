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

    virtual void setPayload(const uint8_t* payload, uint16_t len);
    virtual void clearPayload();

    virtual void run();

    // Internal interrupt methods
    void doSend();
    void onEdge(bool signal);

private:
    enum SyncState : uint8_t {
        NoSync = 0,
        FullSync
    };

    // Synchronisation / Reading
    volatile uint8_t mTimestep;
    bool mSynchronizing;
    SyncState mSyncState;
    uint8_t mNumEdges;
    uint8_t mEdgeTimes[10];
    RingBuffer<uint8_t, 127> mSampleBuffer;

    // Sending
    struct Buffer {
        uint8_t data[40];
        uint8_t len;
        uint8_t offset;
        uint8_t bitOffset;
    };
    Buffer mSendBuffer1;
    Buffer mSendBuffer2;
    Buffer *mSendBuffer;
    Buffer *mNextSendBuffer;
    uint8_t mSendStep;
    bool mHasData;
    bool mSendBitH;
    bool mSendBitL;
    uint8_t mPause;

    void resync();
    void sync(bool send);
};


#endif //VLC_SCHROTTPHY_HPP
