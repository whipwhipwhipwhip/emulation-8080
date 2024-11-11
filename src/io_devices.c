#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <SDL2/SDL_main.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include <time.h>
#include <math.h>
#include "io_devices.h"

void processKeyPress(int key, State8080* state)
{
    switch(key)
    {
        case SDLK_LEFT:
            state->port1 |= 1 << 5;
            break;
        case SDLK_RIGHT:
            state->port1 |= 1 << 6;
            break;
        case SDLK_SPACE:
            state->port1 |= 1 << 4;
            break;
        case SDLK_c:
            state->port1 |= 1 << 0;
            break;
        case SDLK_t:
            state->port2 |= 1 << 2;
            break;
        case SDLK_RETURN:
            state->port1 |= 1 << 2;
            break;
    }

}

void processKeyRelease(int key, State8080* state)
{
    switch(key)
    {
        case SDLK_LEFT:
            state->port1 &= ~(1 << 5);
            break;
        case SDLK_RIGHT:
            state->port1 &= ~(1 << 6);
            break;
        case SDLK_SPACE:
            state->port1 &= ~(1 << 4);
            break;
        case SDLK_c:
            state->port1 &= ~(1 << 0);
            break;
        case SDLK_t:
            state->port2 &= ~(1 << 2);
            break;
        case SDLK_RETURN:
            state->port1 &= ~(1 << 2);
            break;
    }

}

