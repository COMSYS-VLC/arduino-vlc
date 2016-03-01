//
// Created by jan on 01.03.16.
//

#ifndef VLC_TURNOUTCONTROLLER_HPP
#define VLC_TURNOUTCONTROLLER_HPP

#include <stdint.h>

class TurnoutController {
public:
    TurnoutController();

    void isr();
    void stop();
    void open();
    void close();

private:
    bool mState; // true: open (straight), false: closed (diverging)
    uint8_t mDelayStep;

    void forward();
    void backward();
    void startTimer();
    void stopTimer();
};

#endif //VLC_TURNOUTCONTROLLER_HPP
