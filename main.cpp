#include <avr/interrupt.h>

#ifdef TRAIN
#include "TrainController.hpp"

int main() {
    TrainController train;

    sei();

    train.run();
}
#else
#include "SchrottPHY.hpp"
#include "SchrottMAC.hpp"
#include "TurnoutController.hpp"

int main() {
    SchrottPHY phy;
    SchrottMAC mac(phy);
    TurnoutController turnout;

    sei();

    while(true) {
        phy.run();
    }
}

#endif // TRAIN