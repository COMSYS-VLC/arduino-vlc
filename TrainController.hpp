//
// Created by ney on 02.03.2016.
//

#ifndef VLC_TRAINCONTROLLER_HPP
#define VLC_TRAINCONTROLLER_HPP


#include "SchrottMAC.hpp"
#include "SchrottPHY.hpp"
#include "MotorController.hpp"
#include "LEDController.hpp"
#include "Clock.hpp"

class TrainController {
public:
    TrainController();

    void run() __attribute__((noreturn));

private:
    enum LightState {
        Off = 0,
        On = 1,
        Blinking = 2
    };

    void updateStatusMessage();
    void updateLight(LEDController::LED light, LightState state);

    static void payloadCallback(const uint8_t* payload, uint8_t len, void* data);
    static void ackCallback(uint8_t msgId, void* data);
    static void blinkCallback(void* data);

    Clock mClock;
    SchrottPHY mPhy;
    SchrottMAC mMac;
    MotorController mMotor;
    LightState mLights[4];
    bool mBlinkState;

    uint8_t mStatusMsgId;

    static const uint8_t BLINK_DELAY = 250;
};


#endif //VLC_TRAINCONTROLLER_HPP
