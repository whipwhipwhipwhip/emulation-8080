#ifndef GRAPHICS_H
#define GRAPHICS_H
#include <SDL2/SDL.h>
#include "machine.h"


typedef struct SpaceInvadersMachine;
typedef struct SDL_Graphics {
	// Colon means use only that number of bits
	// uint8_t unsigned 8 bit integer
	SDL_Window * window;
    SDL_Renderer * renderer;
} SDL_Graphics;

#endif