#include "MAC.hpp"

void MAC::bitCallback(bool bit, void* data) {
    reinterpret_cast<MAC*>(data)->handleBit(bit);
}
