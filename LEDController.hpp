#ifndef VLC_LEDCONTROLLER_HPP
#define VLC_LEDCONTROLLER_HPP

#include <stdint.h>

/**
 * Controller class for all LEDs on the shield.
 */
class LEDController {
public:
    /** Enumeration of all known LEDs */
    enum LED {
        TX = 0,
        FrontLeft,
        FrontRight,
        RearLeft,
        RearRight,
        Debug
    };

    /**
     * Turn the given LED on.
     *
     * @param led LED to turn on
     */
    static void on(LED led) {
        *(mInfo[led].port) &= ~mInfo[led].bitValue;
    }

    /**
     * Turn the given LED off.
     *
     * @param led LED to turn off
     */
    static void off(LED led) {
        *(mInfo[led].port) |= mInfo[led].bitValue;
    }

    /**
     * Set the given LED to the given state.
     *
     * @param led LED to set
     * @param val value to set the LEDs state to
     */
    static void set(LED led, bool val) {
        val ? on(led) : off(led);
    }

    /**
     * Toggles the state of the given LED.
     *
     * @param led LED to toggle
     */
    static void toggle(LED led) {
        *(mInfo[led].port) ^= mInfo[led].bitValue;
    }

private:
    /** Private constructor: not meant to be instantiated by users */
    LEDController();

    /**
     * Struct with information on how to access each LED on the
     * controller.
     */
    struct LEDInfo {
        /** I/O address of the port the LED is located at */
        volatile uint8_t* port;
        /** Byte value of the pin the LED is located at in the port */
        uint8_t bitValue;
    };

    /** Access information for all known LEDs */
    static const LEDInfo mInfo[6];

    /**
     * Private and static instance to initialize the LEDs before the public
     * static methods can be called.
     */
    static LEDController mInit;
};

#endif //VLC_LEDCONTROLLER_HPP
