#include <avr/interrupt.h>

#ifdef TRAIN
#include "TrainController.hpp"

int main() {
    TrainController train;

    sei();

    train.run();
}
#else
#include "StationController.hpp"

int main() {
    StationController station;

    sei();

    station.run();
}

#endif // TRAIN