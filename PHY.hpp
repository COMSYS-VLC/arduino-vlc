#ifndef VLC_PHY_HPP
#define VLC_PHY_HPP

#include <stdint.h>

/**
 * Abstract base class for all kinds of PHYs/bit layers.
 */
class PHY {
public:
    /** Callback signature for decoded bit values */
    typedef void (*bitHandlerCallback)(bool, void*);

    /** Constructor */
    PHY() : mCallback(0), mCallbackData(0) {

    }

    /**
     * Sets the current payload (bit sequence) which is to be transferred
     * through the PHY.
     *
     * @param payload bit sequence to transfer
     * @param len     byte length of the payload
     */
    virtual void setPayload(const uint8_t* payload, uint16_t len) = 0;

    /**
     * Clears the currently transferred payload if there is one. This
     * effectively sets the PHY into an idle state.
     */
    virtual void clearPayload() = 0;

    /**
     * Lets the PHY handle notification and probably heavy computations on the
     * main thread.
     * Usually called in the run loop.
     */
    virtual void run() = 0;

    /**
     * Sets the callback to call for each decoded bit.
     *
     * @param callback callback to call
     * @param data     user data to provide to the callback
     */
    void setBitHandler(bitHandlerCallback callback, void* data) {
        mCallback = callback;
        mCallbackData = data;
    }

protected:
    /**
     * Calls the currently set bit handler if there is one for the given bit.
     *
     * @param bit decoded bit to handle
     */
    void callBitHandler(bool bit) {
        if(mCallback) {
            mCallback(bit, mCallbackData);
        }
    }

private:
    /** registered callback to use */
    bitHandlerCallback mCallback;
    /** user data for the current callback */
    void* mCallbackData;
};

#endif //VLC_PHY_HPP
