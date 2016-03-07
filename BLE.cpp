//
// Created by jan on 13.01.16.
//

#include "BLE.hpp"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define SET_BIT(x, y) x |= _BV(y)
#define CLEAR_BIT(x, y) x &= ~_BV(y)
#define TOGGLE_BIT(x, y) x ^= _BV(y)

static BLE* uart;
static RingBuffer<uint8_t, 127> recvBuffer;

ISR(USART2_RX_vect) {
    uint8_t data = UDR2;
    recvBuffer << data;
}

BLE BLE::mInstance;

BLE::BLE() {
    uart = this;

    uint16_t br = F_CPU / 16 / 9600 - 1;

    // USART 2 (BLEBee)
    UBRR2H = br >> 8;
    UBRR2L = br;

    UCSR2B = (1 << TXEN2) | (1 << RXEN2) | (1 << RXCIE2);
    UCSR2C = (1 << USBS2) | (3 << UCSZ20);
}

void BLE::send(uint8_t byte) {
    while (!(UCSR2A & (1 << UDRE2)));
    UDR2 = byte;
}

uint8_t BLE::receive() {
    while(!(UCSR2A & (1 << RXC2)));
    return UDR2;
}

bool BLE::hasData() const {
    return !recvBuffer.empty();
}

BLE& BLE::operator<<(uint8_t value) {
    if(value == 0) {
        send('0');
        return *this;
    }
    char buf[3];
    char *cur = buf;
    while(value > 0) {
        *cur++ = '0' + (value % 10);
        value /= 10;
    }
    --cur;
    while(cur >= buf) {
        send(static_cast<uint8_t>(*cur--));
    }
    return *this;
}

BLE& BLE::operator<<(uint16_t value) {
    if(value == 0) {
        send('0');
        return *this;
    }
    char buf[5];
    char *cur = buf;
    while(value > 0) {
        *cur++ = '0' + (value % 10);
        value /= 10;
    }
    --cur;
    while(cur >= buf) {
        send(static_cast<uint8_t>(*cur--));
    }
    return *this;
}

BLE& BLE::operator<<(uint32_t value) {
    if(value == 0) {
        send('0');
        return *this;
    }
    char buf[10];
    char *cur = buf;
    while(value > 0) {
        *cur++ = '0' + (value % 10);
        value /= 10;
    }
    --cur;
    while(cur >= buf) {
        send(static_cast<uint8_t>(*cur--));
    }
    return *this;
}

BLE& BLE::operator<<(char c) {
    send(static_cast<uint8_t>(c));
    return *this;
}

BLE& BLE::operator<<(const char* str) {
    while(*str) {
        send(static_cast<uint8_t>(*str++));
    }
}

BLE& BLE::operator>>(uint8_t &value) {
    while(recvBuffer.empty());
    recvBuffer >> value;

    return *this;
}