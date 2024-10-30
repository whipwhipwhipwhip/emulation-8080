#ifndef MEMORY_H
#define MEMORY_H
#include<stdint.h>
#include "emulator.h"

uint16_t readMemoryAt(State8080* state, uint16_t byte);


#endif