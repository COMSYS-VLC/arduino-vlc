//
// Created by ney on 02.03.2016.
//

#include "StationController.hpp"
#include "UART.hpp"

StationController::StationController() :
    mMac(mPhy),
    mVelocityMsgId(0xFF)
{
    mMac.setPayloadHandler(&payloadCallback, this);
    mMac.setAckHandler(&ackCallback, this);
}

void StationController::run() {
    mTurnout.open();

    while(true) {
        mPhy.run();

        if(UART::get().hasData()) {
            uint8_t data;
            UART::get() >> data;

            sendVelocity(data);
        }
    }
}

void StationController::sendVelocity(uint8_t vel) {
    mMac.cancelPayload(mVelocityMsgId);

    uint8_t payload[2];
    payload[0] = 0x10;
    payload[1] = vel;

    mVelocityMsgId = mMac.sendPayload(payload, 2);
}

void StationController::payloadCallback(const uint8_t *payload, uint8_t len, void* data) {
    StationController* sc = reinterpret_cast<StationController*>(data);

    if(1 < len) {
        // TODO
    }
}

void StationController::ackCallback(uint8_t msgId, void *data) {
    StationController* sc = reinterpret_cast<StationController*>(data);
}
