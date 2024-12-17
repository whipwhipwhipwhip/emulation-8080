#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <SDL2/SDL_main.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include <time.h>
#include <math.h>
#include "machine.h"
#include "io_devices.h"

AudioPlayer sim_audio;
static Mix_Chunk* s_pChunk[9] = {};

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
        case SDLK_p:
            state->port1 |= 1 << 1;
            break;
        case SDLK_s:
            state->port2 |= 1 << 4;
            break;
        case SDLK_a:
            state->port2 |= 1 << 5;
            break;
        case SDLK_d:
            state->port2 |= 1 << 6;
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
        case SDLK_p:
            state->port1 &= ~(1 << 1);
            break;
        case SDLK_s:
            state->port2 &= ~(1 << 4);
            break;
        case SDLK_a:
            state->port2 &= ~(1 << 5);
            break;
        case SDLK_d:
            state->port2 &= ~(1 << 6);
            break;
    }

}

void setupAudio(SpaceInvadersMachine* sim)
{

    if(Mix_OpenAudio(44100, AUDIO_S16SYS, 2, 512) != 0)
    {
        printf("Error on initialising audio system. Exiting...\n");
    }

    sim_audio.sounds = malloc(9 * sizeof(Mix_Chunk));

    Mix_AllocateChannels(4);

    for(int i = 0; i < NUM_SOUNDS; i++)
    {
        sim_audio.soundLocations[i] = _wavFileNames[i];
        sim_audio.sounds[i] = Mix_LoadWAV(sim_audio.soundLocations[i]);
        if (sim_audio.sounds[i] == NULL) {
           printf("Failed to load sound: %s\n", Mix_GetError());
        }
    }
}

void playSound(uint8_t soundIndex, int offset)
{
    int trueIndex = ((int) soundIndex) + offset;
    printf("%d %d %d", trueIndex, soundIndex, offset);
    printf("\n");
    for (int bit = 0; bit != 5; ++bit) {
        if (soundIndex & (0b1 << bit))
        {
            printf("%d %d", bit, offset);
            printf("\n");
            Mix_PlayChannel(-1, sim_audio.sounds[bit+offset], 0);
        }
    }
    
}

