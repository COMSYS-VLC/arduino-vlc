#include <avr/io.h>
#include <util/atomic.h>
#include <util/delay.h>
#include "SchrottPHY.hpp"
#include "SchrottMAC.hpp"
#include "MotorController.hpp"
#include "UART.hpp"
#include "TurnoutController.hpp"

#define SET_BIT(x, y) x |= _BV(y)
#define CLEAR_BIT(x, y) x &= ~_BV(y)
#define TOGGLE_BIT(x, y) x ^= _BV(y)

#ifdef TRAIN
    void trainHandler(const uint8_t* payload, uint8_t len, void* callbackData);

    MotorController motor;
#elif CONTROLLER
void controllerHandler(const uint8_t* payload, uint8_t len, void* callbackData);

    TurnoutController turnout;
#endif

int main() {
    SchrottPHY phy;
    SchrottMAC mac(phy);

    sei();

#ifdef TRAIN
    mac.setPayloadHandler(trainHandler, 0x00);

    motor.setVelocity(0x40);
    motor.forward();
#elif CONTROLLER
    mac.setPayloadHandler(controllerHandler, 0x00);

    uint8_t data = 0x00;
    mac.sendPayload((const uint8_t *) &data, 1);
#endif

    while(true) {
        phy.run();
    }
}

#ifdef TRAIN
    void trainHandler(const uint8_t* payload, uint8_t len, void* callbackData) {
        if(1 == len) {
            uint8_t velocity = *payload;
            if(0 == velocity) {
                motor.backward();
                motor.setVelocity(0xFF);
                _delay_ms(10);
                motor.setVelocity(velocity);
                motor.forward();
            }
        }
    };
#elif CONTROLLER
    void controllerHandler(const uint8_t* payload, uint8_t len, void* callbackData) {

    };
#endif