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

    sim->audio_player = (AudioPlayer *) calloc(1, sizeof(AudioPlayer));
    sim->audio_player->sounds = malloc(9 * sizeof(Mix_Chunk));

    Mix_AllocateChannels(4);

    for(int i = 0; i < NUM_SOUNDS; i++)
    {
        sim->audio_player->soundLocations[i] = _wavFileNames[i];
        sim->audio_player->sounds[i] = Mix_LoadWAV(sim->audio_player->soundLocations[i]);
        if (sim->audio_player->sounds[i] == NULL) {
           printf("Failed to load sound: %s\n", Mix_GetError());
        }
    }
}

void playSound(AudioPlayer * player, uint8_t soundIndex, int offset)
{
    int trueIndex = ((int) soundIndex) + offset;
    for (int bit = 0; bit != 5; ++bit) {
        if (soundIndex & (0b1 << bit))
        {
            Mix_PlayChannel(-1, player->sounds[bit+offset], 0);
        }
    }
    
}

