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
    void onEdge(bool signal);

    // neu
    volatile uint8_t mTimestep;
    enum EdgeType : uint8_t {
        NoEdge = 0,
        SyncUp,
        Data1Down,
        Data1Up,
        Data2Down,
        Data2Up,
        SyncDown,
        Data3Up,
        Data3Down,
        Data4Up,
        Data4Down
    };
private:
    enum SyncState : uint8_t {
        NoSync = 0,
        HalfSync,
        FullSync
    };

    // Synchronisation / Reading
    SyncState mSyncState;
    RingBuffer<uint8_t, 512> mSampleBuffer;

    // Sending
    RingBuffer<uint8_t, 128> mFrameBuffer;
    uint8_t mSendStep;
    uint8_t mSendBitOffset;
    bool mHasData;
    bool mSendBitH;
    bool mSendBitL;

    // neu
    bool mSynchronizing;
    bool mDataSeen[4][2];
    bool mHalfSyncSeen;

    volatile uint8_t mNumEdges;
    uint8_t edgeTimes[10];

    void resync();
    void resetSend();
    void clearSync();

    bool decodeFrame();

    void sync(bool send);
};


#endif //VLC_SCHROTTPHY_HPP
