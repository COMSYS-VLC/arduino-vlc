//
// Created by ney on 07.03.2016.
//

#ifndef VLC_CLOCK_HPP
#define VLC_CLOCK_HPP


#include <stdint.h>

class Clock {
public:
    typedef void(*TimerCallback)(void*);

    Clock();

    void run();
    void execDelayed(uint16_t delayms, TimerCallback callback, void* data);

private:
    struct TimerEntry {
        bool used;
        uint16_t time;
        TimerCallback callback;
        void* data;
    };

    static const uint8_t SLOTS = 5;
    TimerEntry mEntries[SLOTS];
};


#endif //VLC_CLOCK_HPP
