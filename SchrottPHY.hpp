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
    void synchronize();
    void doSend();
    void detectEdge();
private:
    enum SyncState : uint8_t {
        NoSync = 0,
        HalfSync,
        FullSync
    };

    enum EdgeType : uint8_t {
        NoEdge = 0,
        SyncUp,
        SyncDown,
        DataUp,
        DataDown
    };

    // Sensing / Edge detection
    uint32_t mAdcSum;
    uint32_t mAdcCount;
    uint16_t mAdcAvg;
    bool mLastSignal;
    bool mEdgeDetected;

    // Synchronisation / Reading
    SyncState mSyncState;
    EdgeType mNextEdge;
    uint8_t mPeriodStep;
    bool mIsData;
    bool mDataValue;
    RingBuffer<bool, 1024> mSampleBuffer;

    // Sending
    RingBuffer<uint8_t, 128> mFrameBuffer;
    uint8_t mSendStep;
    uint8_t mSendBitOffset;

    void resync();
    void resetSend();
    void clearSync();
};


#endif //VLC_SCHROTTPHY_HPP
