//
// Created by ney on 02.03.2016.
//

#include "TrainController.hpp"
#include "LEDController.hpp"

TrainController::TrainController() :
    mMac(mPhy),
    mStatusMsgId(0xFF)
{
    mMac.setPayloadHandler(&payloadCallback, this);
    mMac.setAckHandler(&ackCallback, this);
}

void TrainController::run() {
    mMotor.setVelocity(0x40);
    mMotor.forward();

    LEDController::on(LEDController::FrontLeft);
    LEDController::on(LEDController::FrontRight);
    LEDController::on(LEDController::RearLeft);
    LEDController::on(LEDController::RearRight);

    updateStatusMessage();

    while(true) {
        mPhy.run();
    }
}

void TrainController::payloadCallback(const uint8_t *payload, uint8_t len, void* data) {
    TrainController* tc = reinterpret_cast<TrainController*>(data);

    if(1 < len) {
        switch(payload[0] >> 4) {
            case 1:
                tc->mMotor.setVelocity(payload[1]);
                if(payload[0] & 0x01) {
                    tc->mMotor.backward();
                } else {
                    tc->mMotor.forward();
                }
                tc->updateStatusMessage();
                break;
            default:
                break;
        }
    }
}

void TrainController::updateStatusMessage() {
    mMac.cancelPayload(mStatusMsgId);

    uint8_t data[2];
    data[0] = (TRAIN_ID << 4);
    if(!mMotor.isForward()) {
        data[0] |= 1;
    }

    data[1] = mMotor.velocity();

    mStatusMsgId = mMac.sendPayload(data, 2);
}

void TrainController::ackCallback(uint8_t msgId, void *data) {
    TrainController* tc = reinterpret_cast<TrainController*>(data);

    if(msgId == tc->mStatusMsgId) {
        tc->updateStatusMessage();
    }
}
