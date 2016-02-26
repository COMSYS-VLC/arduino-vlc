//
// Created by jan on 13.01.16.
//

#ifndef VLC_PHY_HPP
#define VLC_PHY_HPP

#include <stdint.h>

class PHY {
public:
    typedef void (*bitHandlerCallback)(bool, void*);

    PHY() : mCallback(0), mCallbackData(0) {

    }

    virtual void setPayload(const uint8_t* payload, uint16_t len) = 0;
    virtual void clearPayload() = 0;

    virtual void run() = 0;

    void setBitHandler(bitHandlerCallback callback, void* data) {
        mCallback = callback;
        mCallbackData = data;
    }

protected:
    void callBitHandler(bool bit) {
        if(mCallback) {
            mCallback(bit, mCallbackData);
        }
    }

private:
    bitHandlerCallback mCallback;
    void* mCallbackData;
};

#endif //VLC_PHY_HPP
