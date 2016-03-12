#ifndef VLC_TURNOUTCONTROLLER_HPP
#define VLC_TURNOUTCONTROLLER_HPP

#include <stdint.h>

/** Controller for the turnout motor */
class TurnoutController {
public:
    /** Constructor */
    TurnoutController();

    /** Called from ISR to stop the motor after a short while */
    void isr();

    /** Opens the turnout, letting the train take the straight direction */
    void open();

    /** Closes the turnout, letting the train take the diverging direction */
    void close();

private:
    /** Cached state of the turnout */
    bool mState; // true: open (straight), false: closed (diverging)

    /** Counter for stopping delay */
    uint8_t mDelayStep;

    /** Makes the motor turn forward */
    void forward();
    /** Makes the motor turn backward */
    void backward();
    /** Makes the motor stop */
    void stop();
    /** Starts the stopping delay timer */
    void startTimer();
    /** Stops the stopping delay timer */
    void stopTimer();
};

#endif //VLC_TURNOUTCONTROLLER_HPP
