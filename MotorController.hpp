#ifndef VLC_MOTORCONTROLLER_HPP
#define VLC_MOTORCONTROLLER_HPP

#include <stdint.h>

/**
 * Controller class for easy controlling of the trains motor
 */
class MotorController {
public:
    /** Constructor */
    MotorController();

    /** Switch the driving direction to forward */
    void forward();

    /** Switch the driving direction to backward */
    void backward();

    /** Stop the motor */
    void stop();

    /**
     * Getter for current velocity setting
     *
     * @returns current velocity
     */
    uint8_t velocity() const { return mVelocity; }

    /**
     * Sets the current target velocity on a range from 0-255
     *
     * @param velocity new target velocity
     */
    void setVelocity(uint8_t velocity);

    /**
     * Indicates whether the motor is currently driving forward
     *
     * @returns true if driving forward, false otherwise
     */
    bool isForward() const;

private:
    /** cached current target velocity */
    uint8_t mVelocity;
};

#endif //VLC_MOTORCONTROLLER_HPP
