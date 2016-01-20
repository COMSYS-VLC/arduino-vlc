//
// Created by jan on 13.01.16.
//

#include "UART.hpp"

#include <avr/io.h>

UART UART::mInstance;

UART::UART() {
    uint16_t br = F_CPU / 16 / 9600 - 1;
    UBRR0H = br >> 8;
    UBRR0L = br;

    UCSR0B = (1 << TXEN0) | (1 << RXEN0);
    UCSR0C = (1 << USBS0) | (3 << UCSZ00);
}

void UART::send(uint8_t byte) {
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = byte;
}

UART& UART::operator<<(uint8_t value) {
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
        send(*cur--);
    }
    return *this;
}

UART& UART::operator<<(uint16_t value) {
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
        send(*cur--);
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
        send(*cur--);
    }
    return *this;
}

UART& UART::operator<<(char c) {
    send(c);
    return *this;
}

UART& UART::operator<<(const char* str) {
    while(*str) {
        send(*str++);
    }
}
