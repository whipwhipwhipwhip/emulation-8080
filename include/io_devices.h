#ifndef IODEVICES_H
#define IODEVICES_H
#include<stdint.h>
#include<SDL2/SDL.h>
#include<SDL2/SDL_mixer.h>
#include "emulator.h"

void processKeyPress(int key, State8080* state);
void processKeyRelease(int key, State8080* state);
void playSound(uint8_t soundIndex, int offset);
void setupAudio();

static int NUM_SOUNDS = 9;

const static char* _wavFileNames[] =
{
    "../../sounds/ufo_highpitch.wav",
    "../../sounds/invaderkilled.wav",
    "../../sounds/explosion.wav",
    "../../sounds/shoot.wav",
    "../../sounds/fastinvader1.wav",
    "../../sounds/fastinvader2.wav",
    "../../sounds/fastinvader3.wav",
    "../../sounds/fastinvader4.wav",
    "../../sounds/ufo_lowpitch.wav"
};

typedef struct AudioPlayer {
	const char* soundLocations[9];
    Mix_Chunk** sounds;
} AudioPlayer;

#endif