#ifndef MACHINE_H
#define MACHINE_H
#include<stdint.h>
#include "emulator.h"


typedef struct SpaceInvadersMachine {
	// Colon means use only that number of bits
	// uint8_t unsigned 8 bit integer
	State8080 *state;

    double lastTimer;
    double nextInterrupt;
    int whichInterrupt;

    uint8_t shift0; //LSB of Space Invaders external shift
    uint8_t shift1; //MSB
    uint8_t shift_offset; //offset for external shift hardware
} SpaceInvadersMachine;

#endif
