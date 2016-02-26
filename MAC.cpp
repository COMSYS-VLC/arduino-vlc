//
// Created by oggy on 26.02.16.
//
#include "MAC.hpp"

void MAC::bitCallback(bool bit, void* data) {
    reinterpret_cast<MAC*>(data)->handleBit(bit);
}
