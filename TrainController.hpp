//
// Created by ney on 02.03.2016.
//

#ifndef VLC_TRAINCONTROLLER_HPP
#define VLC_TRAINCONTROLLER_HPP


#include "SchrottMAC.hpp"
#include "SchrottPHY.hpp"
#include "MotorController.hpp"

class TrainController {
public:
    TrainController();

    void run() __attribute__((noreturn));

private:
    void updateStatusMessage();

    static void payloadCallback(const uint8_t* payload, uint8_t len, void* data);
    static void ackCallback(uint8_t msgId, void* data);

    SchrottPHY mPhy;
    SchrottMAC mMac;
    MotorController mMotor;

    uint8_t mStatusMsgId;
};


#endif //VLC_TRAINCONTROLLER_HPP
