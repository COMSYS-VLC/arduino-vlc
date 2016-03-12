#ifndef VLC_CLOCK_HPP
#define VLC_CLOCK_HPP

#include <stdint.h>

/**
 * Class for providing dynamic timer functionality to the main program
 * flow/thread.
 *
 * As it uses an interrupt there may only ever be one instance at a time.
 */
class Clock {
public:
    /** Callback signature for timer expiry */
    typedef void(*TimerCallback)(void*);

    /** Constructor */
    Clock();

    /**
     * Method to invoke in the main program/thread to deal with notification of
     * expired timers.
     * Usually to be called in the run loop.
     */
    void run();

    /**
     * Schedules the given callback for delayed execution within the main
     * thread.
     *
     * @param delayms milliseconds until timer expiry and callback invocation
     * @param callback callback to be invoked delayed
     * @param data user defined data to be provided to the callback
     */
    void execDelayed(uint16_t delayms, TimerCallback callback, void* data);

private:
    /**
     * Internal storage structure for a single execution
     */
    struct TimerEntry {
        /** is this storage slot currently being used? */
        bool used;
        /** milliseconds remaining until timeout */
        uint16_t time;
        /** callback to invoke on timeout */
        TimerCallback callback;
        /** user data */
        void* data;
    };

    /** number of available slots */
    static const uint8_t SLOTS = 5;

    /** array of all available execution slots */
    TimerEntry mEntries[SLOTS];
};

#endif //VLC_CLOCK_HPP
