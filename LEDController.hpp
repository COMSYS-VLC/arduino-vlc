//
// Created by jan on 05.03.16.
//

#ifndef VLC_LEDCONTROLLER_HPP
#define VLC_LEDCONTROLLER_HPP

#include <stdint.h>

class LEDController {
public:
    enum LED {
        TX = 0,
        FrontLeft,
        FrontRight,
        RearLeft,
        RearRight,
        Debug
    };

    static void on(LED led) {
        *(mInfo[led].port) &= ~mInfo[led].bitValue;
    }

    static void off(LED led) {
        *(mInfo[led].port) |= mInfo[led].bitValue;
    }

    static void set(LED led, bool val) {
        val ? on(led) : off(led);
    }

    static void toggle(LED led) {
        *(mInfo[led].port) ^= mInfo[led].bitValue;
    }

private:
    /* Not meant to be instantiated by users */
    LEDController();

    struct LEDInfo {
        volatile uint8_t* port;
        uint8_t bitValue;
    };

    static const LEDInfo mInfo[6];

    static LEDController mInit;
};

#endif //VLC_LEDCONTROLLER_HPP
