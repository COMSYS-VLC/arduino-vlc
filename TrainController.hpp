#ifndef VLC_TRAINCONTROLLER_HPP
#define VLC_TRAINCONTROLLER_HPP

#include "VLCMAC.hpp"
#include "VLCPHY.hpp"
#include "MotorController.hpp"
#include "LEDController.hpp"
#include "Clock.hpp"

/** Controller for defining the overall behavior of the train. */
class TrainController {
public:
    /** Constructor */
    TrainController();

    /** Starts the train run loop */
    void run() __attribute__((noreturn));

private:
    /** Possible states of a single LED */
    enum LightState {
        Off = 0,
        Blinking = 1,
        On = 2,
    };

    /** Updates the status message currently being sent to the station
     * controller via VLC. */
    void updateStatusMessage();

    /**
     * Updates the current state of the given LED.
     *
     * @param light LED whose state to change
     * @param state new state for the LED
     */
    void updateLight(LEDController::LED light, LightState state);

    /**
     * Callback for completely received payloads via VLC.
     */
    static void payloadCallback(const uint8_t* payload, uint8_t len,
                                void* data);

    /**
     * Callback for received acknowledgements via VLC.
     */
    static void ackCallback(uint8_t msgId, void* data);

    /**
     * Callback for the LED blinking clock timer.
     */
    static void blinkCallback(void* data);

    /** Timer abstraction for blinking */
    Clock mClock;
    /** PHY for station <-> train comm */
    VLCPHY mPhy;
    /** MAC for station <-> train comm */
    VLCMAC mMac;
    /** Controller for the train motor */
    MotorController mMotor;
    /** State of each head- and backlight */
    LightState mLights[4];
    /** Should the next blink interval have the LEDs on? */
    bool mBlinkState;

    /** Id of the currently being sent status message */
    uint8_t mStatusMsgId;

    /* Blinking interval in ms */
    static const uint8_t BLINK_DELAY = 250;
};

#endif //VLC_TRAINCONTROLLER_HPP
