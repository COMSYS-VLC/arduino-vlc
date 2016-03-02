//
// Created by jan on 13.01.16.
//

#ifndef VLC_MAC_HPP
#define VLC_MAC_HPP

#include "PHY.hpp"

class MAC {
public:
    typedef void (*payloadHandlerCallback)(const uint8_t*, uint8_t, void*);
    typedef void (*ackHandlerCallback)(uint8_t, void*);

    MAC(PHY& phy) : mPHY(0), mPayloadCallback(0), mPayloadCallbackData(0),
                    mAckCallback(0), mAckCallbackData(0) { usePHY(phy); }

    virtual uint8_t sendPayload(const uint8_t* payload, uint8_t len) = 0;
    virtual void cancelPayload(uint8_t id) = 0;

    void usePHY(PHY& phy) {
        mPHY = &phy;
        mPHY->setBitHandler(bitCallback, this);
    }

    void setPayloadHandler(payloadHandlerCallback callback, void* data) {
        mPayloadCallback = callback;
        mPayloadCallbackData = data;
    }

    void setAckHandler(ackHandlerCallback callback, void* data) {
        mAckCallback = callback;
        mAckCallbackData = data;
    }

    PHY& phy() const { return *mPHY; }

protected:
    virtual void handleBit(bool bit) = 0;

    void callPayloadHandler(const uint8_t* payload, uint8_t len) const {
        if(mPayloadCallback) {
            mPayloadCallback(payload, len, mPayloadCallbackData);
        }
    }

    void callAckHandler(uint8_t payloadId) const {
        if(mAckCallback) {
            mAckCallback(payloadId, mAckCallbackData);
        }
    }

private:
    static void bitCallback(bool bit, void* data);

    PHY* mPHY;
    payloadHandlerCallback mPayloadCallback;
    void* mPayloadCallbackData;
    ackHandlerCallback mAckCallback;
    void* mAckCallbackData;
};

#endif //VLC_MAC_HPP
