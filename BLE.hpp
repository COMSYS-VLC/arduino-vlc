//
// Created by jan on 13.01.16.
//

#ifndef VLC_BLE_HPP
#define VLC_BLE_HPP

#include <stdint.h>
#include "RingBuffer.hpp"

class BLE {
public:
    BLE();

    void send(uint8_t byte);
    uint8_t receive();
    bool hasData() const;

    BLE& operator<<(uint8_t value);
    BLE& operator<<(uint16_t value);
    BLE& operator<<(uint32_t value);
    BLE& operator<<(int8_t value) { return value < 0 ? send('-'), *this << (uint8_t)(-value) : *this << (uint8_t)value; }
    BLE& operator<<(int16_t value) { return value < 0 ? send('-'), *this << (uint16_t)(-value) : *this << (uint16_t)value; }
    BLE& operator<<(int32_t value) { return value < 0 ? send('-'), *this << (uint32_t)(-value) : *this << (uint32_t)value; }
    BLE& operator<<(char c);
    BLE& operator<<(const char* str);

    BLE& operator>>(uint8_t &value);

    static BLE& get() { return mInstance; }

private:
    static BLE mInstance;
};


#endif //VLC_BLE_HPP
