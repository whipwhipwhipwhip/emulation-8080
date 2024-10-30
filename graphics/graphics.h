#ifndef GRAPHICS_H
#define GRAPHICS_H
//#include <SDL2/SDL.h>
#include "machine.h"


typedef struct SpaceInvadersMachine;
class SDL_Graphics {
    public:
        void Initialise();
        void DrawGraphics(SpaceInvadersMachine& sim);
};

#endif