#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <SDL2/SDL_main.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include <time.h>
#include <math.h>
#include "emulator.h"
#include "machine.h"
#include "graphics.h"
#include "io_devices.h"

SpaceInvadersMachine *sim;

int main(int argc, char * argv[])
{

    sim = (SpaceInvadersMachine *) calloc(1, sizeof(SpaceInvadersMachine));
    sim->state = (State8080*) calloc(1, sizeof(State8080));
    sim->state->memory = malloc(16 * 0x1000);
    sim->state->int_enable = 1;

    initialise_graphics(sim);
    setupAudio(sim);
    
    readFile(sim->state, argv[1], 0);
    readFile(sim->state, argv[2], 0x800);
    readFile(sim->state, argv[3], 0x1000);
    readFile(sim->state, argv[4], 0x1800);

    doEmulation(sim);

}