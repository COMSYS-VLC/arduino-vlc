#ifndef VLC_BLE_HPP
#define VLC_BLE_HPP

#include <stdint.h>
#include "RingBuffer.hpp"

/**
 * Class to abstract communication with the BLEBee mounted on the shield via
 * UART2.
 *
 * As it uses an interrupt there may only ever be one instance at a time.
 */
class BLE {
public:
    /** Callback signature for incoming data notification */
    typedef void (*BLECallback)(uint8_t*, uint8_t, void*);

    /** Constructor */
    BLE();

    /**
     * Method to invoke in the main program/thread to deal with decoding and
     * notification.
     * Usually to be called in the run loop.
     */
    void run();

    /**
     * Sends the given data via BLE.
     *
     * @param data data to send
     * @param len  length of data
     */
    void send(uint8_t* data, uint8_t len);

    /**
     * Registers the given callback for notification of new incoming data.
     *
     * @param callback callback to invoke
     * @param data     user defined data to pass to the callback
     */
    void registerCallback(BLECallback callback, void* data);

private:
    /**
     * Sends the given byte via BLE without any framing.
     *
     * @param data byte to send
     */
    void send(uint8_t data);

    /** Callback to invoke on incoming data */
    BLECallback mCallback;
    /** Data to pass when invoking the callback */
    void* mCallbackData;
};


#endif //VLC_BLE_HPP
