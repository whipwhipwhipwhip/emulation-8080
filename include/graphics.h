#ifndef GRAPHICS_H
#define GRAPHICS_H
#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>


typedef struct SpaceInvadersMachine SpaceInvadersMachine;
typedef struct SDL_Graphics {
	SDL_Window * window;
    SDL_Renderer * renderer;
} SDL_Graphics;


void initialise_graphics(SpaceInvadersMachine* sim);
void DrawGraphics(SpaceInvadersMachine* sim);

#endif