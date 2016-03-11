//
// Created by jan on 13.01.16.
//

#ifndef VLC_BLE_HPP
#define VLC_BLE_HPP

#include <stdint.h>
#include "RingBuffer.hpp"

class BLE {
public:
    typedef void (*BLECallback)(uint8_t*, uint8_t, void*);

    BLE();

    void run();
    void send(uint8_t* data, uint8_t len);

    BLE& operator>>(uint8_t &value);

    void registerCallback(BLECallback callback, void* data);

private:
    void send(uint8_t data);

    BLECallback mCallback;
    void* mCallbackData;
};


#endif //VLC_BLE_HPP
