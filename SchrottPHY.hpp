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
    RingBuffer<uint8_t, 512> mSampleBuffer;

    // Sending
    RingBuffer<uint8_t, 128> mFrameBuffer;
    uint8_t mSendStep;
    uint8_t mSendBitOffset;
    bool mHasData;
    bool mSendBitH;
    bool mSendBitL;

    void resync();
    void sync(bool send);
};


#endif //VLC_SCHROTTPHY_HPP
