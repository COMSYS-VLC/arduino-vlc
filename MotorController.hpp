//
// Created by oggy on 26.02.16.
//

#ifndef VLC_MOTORCONTROLLER_HPP
#define VLC_MOTORCONTROLLER_HPP

#include <stdint.h>

class MotorController {
public:
    MotorController();

    void forward();
    void backward();
    void stop();

    uint8_t velocity() const { return mVelocity; }
    void setVelocity(uint8_t velocity);

    bool isForward() const;

private:
    uint8_t mVelocity;
};

#endif //VLC_MOTORCONTROLLER_HPP
