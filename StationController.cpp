//
// Created by ney on 02.03.2016.
//

#include <avr/io.h>
#include <util/delay.h>
#include "StationController.hpp"
#include "LEDController.hpp"
#include "BLE.hpp"

#define SET_BIT(x, y) x |= _BV(y)
#define CLEAR_BIT(x, y) x &= ~_BV(y)

StationController::StationController() :
    mMac(mPhy),
    mVelocityMsgId(0xFF)
{
    mMac.setPayloadHandler(&payloadCallback, this);
    mMac.setAckHandler(&ackCallback, this);
}

void StationController::run() {
    mTurnout.open();

    LEDController::on(LEDController::FrontLeft);
    LEDController::on(LEDController::FrontRight);
    LEDController::on(LEDController::RearLeft);
    LEDController::on(LEDController::RearRight);

    while(true) {
        mPhy.run();

        if(BLE::get().hasData()) {
            uint8_t data;
            BLE::get() >> data;

            sendVelocity(data);
            while(data--) {
                LEDController::on(LEDController::Debug);
                _delay_ms(50);
                LEDController::off(LEDController::Debug);
                _delay_ms(500);
            }
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
