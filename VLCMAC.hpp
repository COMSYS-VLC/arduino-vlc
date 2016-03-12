#ifndef VLC_VLCMAC_HPP
#define VLC_VLCMAC_HPP

#include "MAC.hpp"
#include "RingBuffer.hpp"

/** Visual Light MAC */
class VLCMAC : public MAC {
public:
    /**
     * Constructor.
     *
     * @param phy PHY to use
     */
    VLCMAC(PHY& phy);

    virtual uint8_t sendPayload(const uint8_t* payload, uint8_t len);
    virtual void cancelPayload(uint8_t id);

protected:
    virtual void handleBit(bool bit);

private:
    /** Internal states during bit buffer parsing */
    enum ParseState {
        WAIT_BE,
        WAIT_EF,
        WAIT_LENGTH
    };

    /** Buffer of received bits */
    RingBuffer<uint8_t, 63> mFrame;
    /** Current writing bit offset in the bit buffer */
    uint8_t mBitOffset;
    /** Current reading bit offset in the bit buffer */
    uint8_t mFrameOffset;
    /** State of bit buffer parsing */
    ParseState mState;

    /** Structure holding a queued payload for sending */
    struct Payload {
        /** Is this slot used? */
        bool used;
        /** Byte length of the payload */
        uint8_t len;
        /** Data of the payload */
        uint8_t data[31];
    };
    /** Payload queue */
    Payload mPayloads[5];
    /** Id/Index of currently sending payload */
    uint8_t mPayloadId;
    /** Ack id to use for the currently sending payload */
    bool mAckId;
    /** Do we need to acknowledge a received payload? */
    bool mSendAck;
    /** Ack id we need to acknowledge */
    bool mSendAckId;

    /**
     * Extracts 8 consecutive bits from the bit buffer with given byte offset.
     *
     * @param offset byte offset into the bit buffer
     * @returns extracted byte
     */
    uint8_t frameByte(uint8_t offset);

    /**
     * Shifts the bit buffer by one bit, dropping the oldest one.
     */
    void shiftFrame();

    /**
     * Calculates the size of the bit buffer in byte considering bit offsets.
     *
     * @returns effective size of the bit buffer
     */
    uint8_t frameSize();

    /**
     * Handles flags received in a validated header.
     *
     * @param flags flags of received header
     * @param doAck received packet needs to be ack'd
     */
    void handleFlags(uint8_t flags, bool doAck);

    /**
     * Fetches the next payload from the queue and tells the PHY.
     */
    void scheduleNext();

    /**
     * Wraps the current payload in MAC frame or sets a flags-only (len = 0)
     * frame and provides it to the PHY for immediate transmission.
     */
    void setPayload();
};

#endif //VLC_VLCMAC_HPP
