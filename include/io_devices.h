#ifndef IODEVICES_H
#define IODEVICES_H
#include<stdint.h>
#include "emulator.h"

void processKeyPress(int key, State8080* state);
void processKeyRelease(int key, State8080* state);



#endif