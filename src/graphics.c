#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL_main.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_render.h>
#include "graphics.h"
#include "machine.h"
#include "memory.h"

#define TITLE "Space Invaders"
#define HEIGHT 256
#define WIDTH 224

SDL_Color WHITE = {255, 255, 255};
SDL_Color RED = {255, 0, 0};
SDL_Color GREEN = {0, 255, 0};

void initialise_graphics(SpaceInvadersMachine* sim)
{

    // Graphics
    sim->graphics = (SDL_Graphics *) calloc(1, sizeof(SDL_Graphics));
    
    // Attempt to initialize graphics and timer system.
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) != 0)
    {
        printf("error initializing SDL: %s\n", SDL_GetError());
        exit(0);
    }

    SDL_CreateWindowAndRenderer(WIDTH, HEIGHT, 0, &sim->graphics->window, &sim->graphics->renderer);
}

SDL_Color calculateOverlay(uint8_t hor, uint8_t ver)
{
    // Default Space invaders game overlay
    if (ver >= HEIGHT - 32) { return WHITE; }
    if (ver >= (HEIGHT - 32 - 32)) { return RED; }
    if (ver >= (HEIGHT - 32 - 32 - 120)) { return WHITE; }
    if (ver >= (HEIGHT - 32 - 32 - 120 - 56)) { return GREEN; }
    // Last horizontal region divided in 3 parts
    if (hor <= 16) { return WHITE; }
    if (hor <= (16 + 118)) { return GREEN; }
    return WHITE;
}

void DrawGraphics(SpaceInvadersMachine* sim)
{

    SDL_Color pixelColor;
    SDL_SetRenderDrawColor(sim->graphics->renderer, 0, 0, 0, 0);
    SDL_RenderClear(sim->graphics->renderer);
    // VRAM at memory locations 0x2400 - 0x3fff.
    // Cycle through
    uint16_t vram_start = 0x2400;
    for(uint16_t vert = 0; vert < WIDTH; vert++){
        for(uint16_t hor = 0; hor < HEIGHT; hor++){
            uint16_t row_start_byte = 0x20 * vert;
            uint16_t row_entry_byte = hor >> 3;
            uint16_t cur_byte = vram_start + row_start_byte + row_entry_byte;

            uint8_t cur_bit = hor % 8;

            uint16_t cur_mem = readMemoryAt(sim->state, cur_byte);
            bool pixelOn = (cur_mem & (1 << cur_bit)) != 0;
            //check if bit cur_bit is 1.
            // If so, draw the pixel in accordance to overlay. Otherwise, don't.
            if(pixelOn)
            {
                SDL_Color pixelColor = calculateOverlay(vert, hor);
                SDL_SetRenderDrawColor(sim->graphics->renderer, pixelColor.r, pixelColor.g, pixelColor.b, 255);
            }
            else {
                SDL_SetRenderDrawColor(sim->graphics->renderer, 0, 0, 0, 0);
            }

        // Rotate coordinates counter clockwise
        SDL_RenderDrawPoint(sim->graphics->renderer, vert, HEIGHT-hor-1);
        }
    }
    SDL_RenderPresent(sim->graphics->renderer);
}


