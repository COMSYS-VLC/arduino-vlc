#ifndef VLC_VLCPHY_HPP
#define VLC_VLCPHY_HPP

#include "PHY.hpp"
#include "RingBuffer.hpp"

class MAC;

/** Visual Light PHY */
class VLCPHY : public PHY {
public:
    /** Constructor */
    VLCPHY();

    virtual void setPayload(const uint8_t* payload, uint16_t len);
    virtual void clearPayload();

    virtual void run();

    // Internal interrupt methods
    /**
     * Handles the properly timed generation of the output signal.
     */
    void doSend();
    /**
     * Stores and analyzes every seen edge.
     */
    void onEdge(bool signal);

private:
    /** Synchronization states */
    enum SyncState : uint8_t {
        NoSync = 0,
        FullSync
    };

    // Synchronization / Reading
    /** Current time in PHY context for receiving (period split in 24 steps) */
    volatile uint8_t mTimestep;
    /** Are we synchronizing right now? */
    bool mSynchronizing;
    /** Current sychronization state */
    SyncState mSyncState;
    /** Number of seen edges in this period */
    uint8_t mNumEdges;
    /** Memory of time each edge was seen */
    uint8_t mEdgeTimes[10];
    /** Buffer of recognized two-bit pairs */
    RingBuffer<uint8_t, 127> mSampleBuffer;

    /**
     * Reset everything for a new synchronization attempt starting at the time
     * of this functions call.
     */
    void resync();

    /**
     * Align receiving and if wanted also the sending clock to a period starting
     * at the time of this functions call.
     *
     * @param send align sending as well
     */
    void sync(bool send);

    // Sending
    /** Structure for output buffers with sending state */
    struct Buffer {
        /** Data to transmit */
        uint8_t data[40];
        /** Byte length of data */
        uint8_t len;
        /** Current byte offset for sending */
        uint8_t offset;
        /** Current bit offset for sending */
        uint8_t bitOffset;
    };
    /** Buffer 1 of the double-buffered sending buffer */
    Buffer mSendBuffer1;
    /** Buffer 2 of the double-buffered sending buffer */
    Buffer mSendBuffer2;
    /** Which sending buffer to use in this period */
    Buffer *mSendBuffer;
    /** Which sending buffer to use in the next period, or null to idle */
    Buffer *mNextSendBuffer;
    /** Current time in PHY context for sending (period split in 24 steps) */
    uint8_t mSendStep;
    /** Cached info if the PHY is idling or not */
    bool mHasData;
    /** Cached info if the high bit for the current period is set */
    bool mSendBitH;
    /** Cached info if the low bit for the current period is set */
    bool mSendBitL;
};


#endif //VLC_VLCPHY_HPP
