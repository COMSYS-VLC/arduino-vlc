//
// Created by ney on 02.03.2016.
//

#include <avr/io.h>
#include "StationController.hpp"
#include "LEDController.hpp"

StationController::StationController() :
    mMac(mPhy),
    mVelocityMsgId(0xFF),
    mTurnoutStraight(true),
    mVelocity(0xFF),
    mForward(true),
    mLEDstatesMsgId(0xFF)
{
    for(uint8_t i = 0; i < 4; ++i) {
        mLEDs[i] = 0;
    }

    mMac.setPayloadHandler(&payloadCallback, this);
    mMac.setAckHandler(&ackCallback, this);
    mBLE.registerCallback(&BLECallback, this);
}

void StationController::run() {
    mTurnout.open();
    sendBLETurnoutState();

    LEDController::on(LEDController::FrontLeft);
    LEDController::on(LEDController::FrontRight);
    LEDController::on(LEDController::RearLeft);
    LEDController::on(LEDController::RearRight);

    while(true) {
        mPhy.run();
        mBLE.run();
    }
}

void StationController::sendVelocity() {
    mMac.cancelPayload(mVelocityMsgId);

    uint8_t payload[2];
    payload[0] = 0x10;
    payload[1] = mVelocity;
    if(!mForward) {
        payload[0] |= 1;
    }

    mVelocityMsgId = mMac.sendPayload(payload, 2);

    sendBLEVelocity();
}

void StationController::sendLEDstates() {
    mMac.cancelPayload(mLEDstatesMsgId);

    uint8_t payload[2];
    payload[0] = 0x20;
    payload[1] = 0;

    for(uint8_t i = 0; i < 4; ++i) {
        payload[1] |= (mLEDs[i] << (6 - i * 2));
    }

    mLEDstatesMsgId = mMac.sendPayload(payload, 2);

    sendBLELEDStates();
}

void StationController::payloadCallback(const uint8_t *payload, uint8_t len, void* data) {
    StationController* sc = reinterpret_cast<StationController*>(data);

    if(2 < len) {
        if(sc->mVelocityMsgId == 0xFF) {
            sc->mForward = !(payload[0] & 0x01);
            sc->mVelocity = payload[1];
            sc->sendBLEVelocity();
        }

        if(sc->mLEDstatesMsgId == 0xFF) {
            for (uint8_t i = 0; i < 4; ++i) {
                sc->mLEDs[i] = (payload[2] >> (6 - i * 2)) & 0x03;
            }
            sc->sendBLELEDStates();
        }
    }
}

void StationController::ackCallback(uint8_t msgId, void *data) {
    StationController* sc = reinterpret_cast<StationController*>(data);

    if(msgId == sc->mVelocityMsgId) {
        sc->mVelocityMsgId = 0xFF;
    } else if (msgId == sc->mLEDstatesMsgId) {
        sc->mLEDstatesMsgId = 0xFF;
    }
}

void StationController::BLECallback(uint8_t* payload, uint8_t len, void* data) {
    StationController* sc = reinterpret_cast<StationController*>(data);
    switch(payload[0]) {
        case 0:
            if(payload[1]) {
                sc->mTurnout.close();
                sc->mTurnoutStraight = false;
            } else {
                sc->mTurnout.open();
                sc->mTurnoutStraight =  true;
            }
            break;

        case 1:
            sc->mVelocity = (payload[1] & 0x3F) << 2;
            sc->mForward = !(payload[1] & 0x80);
            sc->sendVelocity();
            break;

        case 2:
        case 3:
        case 4:
        case 5:
            sc->mLEDs[payload[0] - 2] = payload[1];
            sc->sendLEDstates();
            break;

        case 254:
            sc->sendBLEVelocity();
            sc->sendBLELEDStates();
            sc->sendBLETurnoutState();
            break;
    }
}

void StationController::sendBLEVelocity() {
    uint8_t payload[2];

    payload[0] = 1;
    payload[1] = mVelocity >> 2;
    if(!mForward) {
        payload[1] |= (0x80);
    }

    mBLE.send(payload, 2);
}

void StationController::sendBLELEDStates() {
    for(uint8_t i = 0; i < 4; ++i) {
        uint8_t payload[2];

        payload[0] = i + 2;
        payload[1] = mLEDs[i];

        mBLE.send(payload, 2);
    }
}

void StationController::sendBLETurnoutState() {
    uint8_t payload[2];

    payload[0] = 0;
    payload[1] = !mTurnoutStraight;

    mBLE.send(payload, 2);
}
