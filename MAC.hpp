//
// Created by jan on 13.01.16.
//

#ifndef VLC_MAC_HPP
#define VLC_MAC_HPP

#include "PHY.hpp"

class MAC {
public:
    MAC(PHY& phy) : mPhy(phy) {}

    virtual void sendPayload(const uint8_t* payload, uint8_t len) = 0;
    virtual void handleBit(bool bit) = 0;

    PHY& phy() const { return mPhy; }

private:
    PHY& mPhy;
};

#endif //VLC_MAC_HPP
