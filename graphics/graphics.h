#include <SDL2/SDL.h>

class SpaceInvadersMachine;

class SDL_Graphics{
    public:
        void Initialise();
        void DrawGraphics(SpaceInvadersMachine& sim);
    
    public:
        SpaceInvadersMachine* sim;
};