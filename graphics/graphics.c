#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL_main.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include "graphics.h"

void Platform_SDL::Initialize(SpaceInvadersMachine* sim)
{
    m_sim = sim;
    // Graphics
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    SDL_CreateWindowAndRenderer(224, 256, 0, &window, &renderer);

    //SetupAudio();
}

void SDL_Graphics::DrawGraphics(SpaceInvadersMachine& sim)
{
    // VRAM at memory locations 0x2400 - 0x3fff.
    // Cycle through
    uint16_t vram_start = 0x2400;
    for(uint16_t vert = 0; vert < 224; vert++){
        for(uint16_t hor = 0; hor < 256; hor++){
            uint16_t row_start_byte = 0x20 * vert;
            uint16_t row_entry_byte = h >> 3;
            uint16_t cur_byte = vram_start + row_start_byte + row_entry_byte;

            uint8_t cur_bit = h % 8;

            //bool pixelOn = readMemoryFrom (current_byte), check if bit cur_bit is 1.
            // If so, draw the pixel in accordane to overlay. Otherwise, don't.
        }
    }
}


