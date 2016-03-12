#include <avr/interrupt.h>

#ifdef TRAIN // We're a train
#include "TrainController.hpp"

int main() {
    TrainController train;

    sei();

    train.run();
}
#elif defined(CONTROLLER) // We're a station
#include "StationController.hpp"

int main() {
    StationController station;

    sei();

    station.run();
}
#elif defined(EVA_RECV) // We're the receiving end of an evaluation setup
#include "VLCPHY.hpp"
#include "VLCMAC.hpp"

int main() {
    VLCPHY phy;
    VLCMAC mac(phy);

    sei();

    while(true) {
        phy.run();
    }
}

#elif defined(EVA_SEND)  // We're the sending end of an evaluation setup
#include "VLCPHY.hpp"
#include "VLCMAC.hpp"
#include "Clock.hpp"
#include "UART.hpp"

static Clock clock;
static uint8_t bytesSent = 0;

static void writeStats(void*) {
    clock.execDelayed(1000, &writeStats, 0);

    UART::get() << bytesSent << '\n';
    bytesSent = 0;
}

int main() {
    VLCPHY phy;
    VLCMAC mac(phy);

    sei();

    clock.execDelayed(1000, &writeStats, 0);

    static uint8_t payload[31];
    while(mac.sendPayload(payload, sizeof(payload) / sizeof(uint8_t)) < 0xF0);
    while(true) {
        while(mac.sendPayload(payload, sizeof(payload) / sizeof(uint8_t)) < 0xF0) {
            bytesSent += sizeof(payload) / sizeof(uint8_t);
        }
        phy.run();
        clock.run();
    }
}

#endif // TRAIN
