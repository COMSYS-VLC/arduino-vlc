//
// Created by jan on 13.01.16.
//

#include "UART.hpp"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define SET_BIT(x, y) x |= _BV(y)
#define CLEAR_BIT(x, y) x &= ~_BV(y)
#define TOGGLE_BIT(x, y) x ^= _BV(y)

static UART* uart;
static RingBuffer<uint8_t, 127> recvBuffer;

ISR(USART0_RX_vect) {
    uint8_t data = UDR0;
    recvBuffer << data;
}

UART UART::mInstance;

UART::UART() {
    uart = this;

    uint16_t br = F_CPU / 16 / 9600 - 1;
    UBRR0H = br >> 8;
    UBRR0L = br;

    UCSR0B = (1 << TXEN0) | (1 << RXEN0) | (1 << RXCIE0);
    UCSR0C = (1 << USBS0) | (3 << UCSZ00);
}

void UART::send(uint8_t byte) {
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = byte;
}

uint8_t UART::receive() {
    while(!(UCSR0A & (1 << RXC0)));
    return UDR0;
}

UART& UART::operator<<(uint8_t value) {
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

UART& UART::operator<<(uint16_t value) {
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

UART& UART::operator<<(uint32_t value) {
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

UART& UART::operator<<(char c) {
    send(static_cast<uint8_t>(c));
    return *this;
}

UART& UART::operator<<(const char* str) {
    while(*str) {
        send(static_cast<uint8_t>(*str++));
    }
}

UART& UART::operator>>(uint8_t &value) {
    while(recvBuffer.empty());
    recvBuffer >> value;

    return *this;
}