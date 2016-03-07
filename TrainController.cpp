//
// Created by ney on 02.03.2016.
//

#include "TrainController.hpp"

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

    updateLight(LEDController::FrontLeft, Blinking);
    updateLight(LEDController::FrontRight, Off);
    updateLight(LEDController::RearLeft, Off);
    updateLight(LEDController::RearRight, Blinking);

    updateStatusMessage();

    mClock.execDelayed(BLINK_DELAY, &blinkCallback, this);

    while(true) {
        mPhy.run();
        mClock.run();
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
            case 2:
                tc->updateLight(LEDController::FrontLeft, static_cast<LightState>(payload[1] >> 6));
                tc->updateLight(LEDController::FrontRight, static_cast<LightState>((payload[1] >> 4) & 0x03));
                tc->updateLight(LEDController::RearLeft, static_cast<LightState>((payload[1] >> 2) & 0x03));
                tc->updateLight(LEDController::RearRight, static_cast<LightState>(payload[1] & 0x03));
                tc->updateStatusMessage();
            default:
                break;
        }
    }
}

void TrainController::updateLight(LEDController::LED light, LightState state) {
    switch(state) {
        case On:
            LEDController::on(light);
            break;
        case Off:
            LEDController::off(light);
            break;
        case Blinking:
            // nothing to do
            break;
    }
    mLights[light - LEDController::FrontLeft] = state;
}

void TrainController::updateStatusMessage() {
    mMac.cancelPayload(mStatusMsgId);

    uint8_t data[3];
    data[0] = (TRAIN_ID << 4);
    if(!mMotor.isForward()) {
        data[0] |= 1;
    }

    data[1] = mMotor.velocity();
    data[2] = 0;

    for(uint8_t i = 0; i < 4; ++i) {
        data[2] |= static_cast<uint8_t>(mLights[i]) << (6 - i * 2);
    }

    mStatusMsgId = mMac.sendPayload(data, 3);
}

void TrainController::ackCallback(uint8_t msgId, void *data) {
    TrainController* tc = reinterpret_cast<TrainController*>(data);

    if(msgId == tc->mStatusMsgId) {
        tc->updateStatusMessage();
    }
}

void TrainController::blinkCallback(void *data) {
    TrainController* tc = reinterpret_cast<TrainController*>(data);

    for(uint8_t i = 0; i < 4; ++i) {
        if(Blinking == tc->mLights[i]) {
            if(tc->mBlinkState) {
                LEDController::on(LEDController::FrontLeft + i);
            } else {
                LEDController::off(LEDController::FrontLeft + i);
            }
        }
    }
    tc->mBlinkState = !tc->mBlinkState;

    tc->mClock.execDelayed(tc->BLINK_DELAY, &blinkCallback, data);
}
