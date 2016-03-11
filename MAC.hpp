#ifndef VLC_MAC_HPP
#define VLC_MAC_HPP

#include "PHY.hpp"

/** Abstract base class for all kinds of MAC layers. */
class MAC {
public:
    /** Callback signature for received payloads */
    typedef void (*payloadHandlerCallback)(const uint8_t*, uint8_t, void*);
    /** Callback signature for received acknowledgements */
    typedef void (*ackHandlerCallback)(uint8_t, void*);

    /** Constructor */
    MAC(PHY& phy) : mPHY(0), mPayloadCallback(0), mPayloadCallbackData(0),
                    mAckCallback(0), mAckCallbackData(0) { usePHY(phy); }

    /**
     * Enqueue the given payload for sending. Returns an id for the message
     * which can be used for later cancellation or acknowledgement handling.
     *
     * @param payload data to send
     * @param len     byte length of given data
     * @returns id of enqueued payload, 0xFE on full buffer, 0xFF on error
     */
    virtual uint8_t sendPayload(const uint8_t* payload, uint8_t len) = 0;

    /**
     * Cancels the sending and waiting for ack of the payload for the given id.
     *
     * @param id id of the payload to cancel
     */
    virtual void cancelPayload(uint8_t id) = 0;

    /**
     * Make this MAC use the given PHY for its communication.
     *
     * @param phy PHY to use
     */
    void usePHY(PHY& phy) {
        mPHY = &phy;
        mPHY->setBitHandler(bitCallback, this);
    }

    /**
     * Sets the handler to call for complete received payloads.
     *
     * @param callback handler callback
     * @param data     user data for the callback
     */
    void setPayloadHandler(payloadHandlerCallback callback, void* data) {
        mPayloadCallback = callback;
        mPayloadCallbackData = data;
    }

    /**
     * Sets the handler to call for received acknowledgments/completed
     * transfers.
     *
     * @param callback handler callback
     * @param data     user data for the callback
     */
    void setAckHandler(ackHandlerCallback callback, void* data) {
        mAckCallback = callback;
        mAckCallbackData = data;
    }

    /**
     * Returns the currently used PHY.
     *
     * @returns currently used PHY
     */
    PHY& phy() const { return *mPHY; }

protected:
    /**
     * Handles a single bit received from the PHY.
     *
     * @param bit received bit
     */
    virtual void handleBit(bool bit) = 0;

    /**
     * Calls the currently set payload handler if there is one for the given
     * payload.
     *
     * @param payload received and decoded payload
     * @param len     byte length of the payload
     */
    void callPayloadHandler(const uint8_t* payload, uint8_t len) const {
        if(mPayloadCallback) {
            mPayloadCallback(payload, len, mPayloadCallbackData);
        }
    }

    /**
     * Calls the currently set acknowledgement handler if there is one for the
     * given payload id.
     *
     * @param payloadId id of the acknowledged payload
     */
    void callAckHandler(uint8_t payloadId) const {
        if(mAckCallback) {
            mAckCallback(payloadId, mAckCallbackData);
        }
    }

private:
    /** Callback method for the current PHY */
    static void bitCallback(bool bit, void* data);

    /** Current PHY */
    PHY* mPHY;
    /** Current payload callback */
    payloadHandlerCallback mPayloadCallback;
    /** User data for the current payload callback */
    void* mPayloadCallbackData;
    /** Current acknowledgement callback */
    ackHandlerCallback mAckCallback;
    /** User data for the current acknowledgement callback */
    void* mAckCallbackData;
};

#endif //VLC_MAC_HPP
