//
// Created by jan on 13.01.16.
//

#include "BLE.hpp"
#include "LEDController.hpp"
#include "UART.hpp"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/crc16.h>

static RingBuffer<uint8_t, 127> recvBuffer;

ISR(USART2_RX_vect) {
    uint8_t data = UDR2;
    recvBuffer << data;

    UART::get() << data << ' ';
}

BLE::BLE() : mCallback(0) {
    uint16_t br = F_CPU / 16 / 9600 - 1;

    // USART 2 (BLEBee)
    UBRR2H = br >> 8;
    UBRR2L = br;

    UCSR2B = (1 << TXEN2) | (1 << RXEN2) | (1 << RXCIE2);
    UCSR2C = (1 << USBS2) | (3 << UCSZ20);
}

void BLE::run() {
    while(!recvBuffer.empty()) {
        while(!recvBuffer.empty()) {
            uint8_t sync = recvBuffer.at(0);
            if (0xFF != sync) {
                recvBuffer.pop();
            } else {
                break;
            }
        }

        if(3 < recvBuffer.size()) {
            uint8_t crcPacket = recvBuffer.at(3);
            uint8_t crc = 0x00;
            for (uint8_t i = 0; i < 3; ++i) {
                crc = _crc8_ccitt_update(crc, recvBuffer.at(i));
                //UART::get() << i << " " << crc << " \n";
            }
            if (crc != crcPacket) {
                UART::get() << "CRC mismatch: " << crc << " vs " << crcPacket << '\n';
                recvBuffer.pop();
            } else {
                uint8_t payload[2];
                payload[0] = recvBuffer.at(1);
                payload[1] = recvBuffer.at(2);

                if (mCallback) {
                    mCallback(payload, 2, mCallbackData);
                }

                recvBuffer.pop();
                recvBuffer.pop();
                recvBuffer.pop();
                recvBuffer.pop();
            }
        } else {
            break;
        }
    }
}

void BLE::send(uint8_t* data, uint8_t len) {
    send(0xFF);

    uint8_t crc = _crc8_ccitt_update(0, 0xFF);
    for(uint8_t i = 0; i < len; ++i) {
        send(data[i]);
        crc = _crc8_ccitt_update(crc, data[i]);
    }

    send(crc);
}

void BLE::send(uint8_t byte) {
    while (!(UCSR2A & (1 << UDRE2)));
    UDR2 = byte;
}

BLE& BLE::operator>>(uint8_t &value) {
    while(recvBuffer.empty());
    recvBuffer >> value;

    return *this;
}

void BLE::registerCallback(BLE::BLECallback callback, void *data) {
    mCallback = callback;
    mCallbackData = data;
}
