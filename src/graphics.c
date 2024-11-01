#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL_main.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include "graphics.h"
#include "memory.h"

SDL_Graphics *sim_graphics;
SDL_Color WHITE = {255, 255, 255};
SDL_Color RED = {255, 0, 0};
SDL_Color GREEN = {0, 255, 0};

void initialise_graphics(SpaceInvadersMachine* sim)
{
    //SpaceInvadersMachine m_sim = sim;
    // Graphics
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    SDL_CreateWindowAndRenderer(224, 256, 0, &sim_graphics->window, &sim_graphics->renderer);

    //SetupAudio();
}

SDL_Color calculateOverlay(uint8_t hor, uint8_t ver)
{
    // Default Space invaders game overlay
    if (ver >= 256 - 32) { return WHITE; }
    if (ver >= (256 - 32 - 32)) { return RED; }
    if (ver >= (256 - 32 - 32 - 120)) { return WHITE; }
    if (ver >= (256 - 32 - 32 - 120 - 56)) { return GREEN; }
    // Last horizontal region divided in 3 parts
    if (hor <= 16) { return WHITE; }
    if (hor <= (16 + 118)) { return GREEN; }
    return WHITE;
}

void DrawGraphics(SpaceInvadersMachine* sim)
{

    SDL_Color pixelColor;
    SDL_SetRenderDrawColor(sim_graphics->renderer, 0, 0, 0, 0);
    SDL_RenderClear(sim_graphics->renderer);
    // VRAM at memory locations 0x2400 - 0x3fff.
    // Cycle through
    uint16_t vram_start = 0x2400;
    for(uint16_t vert = 0; vert < 224; vert++){
        for(uint16_t hor = 0; hor < 256; hor++){
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
                SDL_SetRenderDrawColor(sim_graphics->renderer, pixelColor.r, pixelColor.g, pixelColor.b, 255);
            }
            else {
                SDL_SetRenderDrawColor(sim_graphics->renderer, 0, 0, 0, 0);
            }

        // Rotate coordinates counter clockwise
        SDL_RenderDrawPoint(sim_graphics->renderer, vert, 256-hor-1);
        }
    }
    SDL_RenderPresent(sim_graphics->renderer);
}


