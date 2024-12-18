#ifndef MEMORY_H
#define MEMORY_H
#include<stdint.h>
#include "emulator.h"

typedef struct State8080 State8080;
void readFile(State8080* state, char* filename, uint32_t offset);


#endif