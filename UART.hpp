//
// Created by jan on 13.01.16.
//

#ifndef VLC_UART_HPP
#define VLC_UART_HPP

#include <stdint.h>

class UART {
public:
    UART();

    void send(uint8_t byte);

    UART& operator<<(uint8_t value);
    UART& operator<<(uint16_t value);
    UART& operator<<(uint32_t value);
    UART& operator<<(int8_t value) { return value < 0 ? send('-'), *this << (uint8_t)(-value) : *this << (uint8_t)value; }
    UART& operator<<(int16_t value) { return value < 0 ? send('-'), *this << (uint16_t)(-value) : *this << (uint16_t)value; }
    UART& operator<<(int32_t value) { return value < 0 ? send('-'), *this << (uint32_t)(-value) : *this << (uint32_t)value; }
    UART& operator<<(char c);
    UART& operator<<(const char* str);

    static UART& get() { return mInstance; }

private:
    static UART mInstance;
};


#endif //VLC_UART_HPP
