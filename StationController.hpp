//
// Created by ney on 02.03.2016.
//

#ifndef VLC_STATIONCONTROLLER_HPP
#define VLC_STATIONCONTROLLER_HPP


#include "SchrottMAC.hpp"
#include "SchrottPHY.hpp"
#include "TurnoutController.hpp"
#include "BLE.hpp"

class StationController {
public:
    StationController();

    void run() __attribute__((noreturn));

private:

    static void payloadCallback(const uint8_t* payload, uint8_t len, void* data);
    static void ackCallback(uint8_t msgId, void* data);
    static void BLECallback(uint8_t* payload, uint8_t len, void* data);

    SchrottPHY mPhy;
    SchrottMAC mMac;
    TurnoutController mTurnout;
    BLE mBLE;

    void sendVelocity();
    uint8_t mVelocityMsgId;
    uint8_t mVelocity;
    bool mForward;

    bool mTurnoutStraight;

    void sendLEDstates();
    uint8_t mLEDstatesMsgId;
    uint8_t mLEDs[4];

    void sendBLEVelocity();
    void sendBLELEDStates();
    void sendBLETurnoutState();
};


#endif //VLC_STATIONCONTROLLER_HPP
