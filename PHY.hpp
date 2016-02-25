//
// Created by jan on 13.01.16.
//

#ifndef VLC_PHY_HPP
#define VLC_PHY_HPP

#include <stdint.h>

class MAC;

class PHY {
public:
    PHY() : mMAC(0) {

    }

    virtual void setPayload(const uint8_t* payload, uint16_t len) = 0;

    virtual void run() = 0;

    void registerMAC(MAC* mac) {
        mMAC = mac;
    };

    MAC* mac() {
        return mMAC;
    }

private:
    MAC* mMAC;
};

#endif //VLC_PHY_HPP
