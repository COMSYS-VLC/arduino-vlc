//
// Created by ney on 02.03.2016.
//

#ifndef VLC_STATIONCONTROLLER_HPP
#define VLC_STATIONCONTROLLER_HPP


#include "SchrottMAC.hpp"
#include "SchrottPHY.hpp"
#include "TurnoutController.hpp"

class StationController {
public:
    StationController();

    void run() __attribute__((noreturn));

private:

    static void payloadCallback(const uint8_t* payload, uint8_t len, void* data);
    static void ackCallback(uint8_t msgId, void* data);

    SchrottPHY mPhy;
    SchrottMAC mMac;
    TurnoutController mTurnout;

    void sendVelocity(uint8_t vel);
    uint8_t mVelocityMsgId;
};


#endif //VLC_STATIONCONTROLLER_HPP
