#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <SDL2/SDL_main.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include <time.h>
#include <math.h>
#include "emulator.h"
#include "machine.h"
#include "graphics.h"
#include "io_devices.h"

#define TITLE "Space Invaders"
#define HEIGHT 256
#define WIDTH 224

SDL_Surface *surf;
int resizef;
SDL_Window *win;
SDL_Surface *winsurf;
SpaceInvadersMachine *sim;
SDL_Event event;
State8080 *state;

uint8_t InPort(uint8_t port_bit)
{
    uint8_t a = 0;
    switch(port_bit)
    {
        case 0:
            return 1;
        case 1:
            return sim->state->port1;
        case 2:
            return sim->state->port2;
        case 3:
        {
            uint16_t v = (sim->shift1<<8) | sim->shift0;    
            a = ((v >> (8-sim->shift_offset)) & 0xff); 
            return a;
        }
        default:
            printf("\nError, In ports should only be 0,1,2,3, and we tried %x\n", port_bit);
            exit(1);
    }
}

void OutPort(uint8_t port_bit, uint8_t value)
{
    switch(port_bit)
    {
        // shift amount
        case 2:
            sim->shift_offset = value & 0x7;
            break;
        // Discrete Sounds
        case 3:
            if ((sim->state->prevWrite3 != value))
            {
                playSound(value, 0);
                sim->state->prevWrite3 = value;
            }
            break;
        // shift data
        case 4:
            sim->shift0 = sim->shift1;
            sim->shift1 = value;
            break;
         // Discrete Sounds
        case 5:
            if (sim->state->prevWrite5 != value)
            {
                playSound(value, 4);
                sim->state->prevWrite5 = value;
            }
            break;
    }
  
}

void generate_interrupt(State8080* state, int interrupt_num)
{
    uint16_t push = state->pc;
	state->memory[state->sp-1] = (push >> 8) & 0xff;
	state->memory[state->sp-2] = push & 0xff;
	state->sp -= 2;
    state->lastSp = state->sp;  

    //Set the PC to the low memory vector.      
    state->pc = 8 * interrupt_num; 

    // set interrupts to null until they're reenabled
    state->int_enable = 0;  
}

void readFile(char* filename, uint32_t offset)
{
	FILE *f= fopen(filename, "rb");
	if (f==NULL)
	{
		printf("error: Couldn't open %s\n", filename);
		exit(1);
	}
	fseek(f, 0L, SEEK_END);
	int fsize = ftell(f);
	fseek(f, 0L, SEEK_SET);
	
	uint8_t *buffer = &sim->state->memory[offset];
	fread(buffer, fsize, 1, f);
	fclose(f);
}

void runFrame()
{
    //time in microseconds
    double time_now = 1000 *SDL_GetTicks();
    static int clockSpeed = 1e6;
    static int FPS = 60;
    int cycles_to_run = floor(clockSpeed / FPS);
    int cycles_performed = 0;
    unsigned char* op;

    // we start by setting the next interrupt time if necessary
    if(sim->lastTimer == 0.0)
    {
        printf("Time not properly Initialised - exiting...\n");
        exit(0);
    }

    sim->nextInterrupt = time_now + cycles_to_run;
    sim->whichInterrupt = 1;

    // now run a full frame of cycles
    for(int i = 0; i < 2; i ++)
    {
        cycles_performed = 0;
        // in here, a half frame that will culminate in an interrupt
        while(cycles_performed < cycles_to_run)
        {
            double time_elapsed = 1000 * SDL_GetTicks() - sim->lastTimer;
            int cycles_needed = 2 * time_elapsed;
            int batch_cycles = 0;

            while(batch_cycles < cycles_needed)
            {
                op = &sim->state->memory[sim->state->pc];
                if (*op == 0xdb) //machine specific handling for IN
                {
                    sim->state->a = InPort(op[1]);
                    sim->state->pc += 2;
                    batch_cycles+=3;
                }
                else if (*op == 0xd3) //machine specific handling for OUT
                {
                    OutPort(op[1], sim->state->a);
                    sim->state->pc += 2;
                    batch_cycles+=3;
                }
                else
                    batch_cycles += Emulate8080p(sim->state);
            }
            cycles_performed += batch_cycles;
            sim->lastTimer = 1000 * SDL_GetTicks();
        }
        // Now done, perform the interrupt and repeat
        if(sim->state->int_enable)
        {
            generate_interrupt(sim->state, sim->whichInterrupt);
            sim->whichInterrupt = (sim->whichInterrupt == 1)? 2: 1;
            time_now = 1000 * SDL_GetTicks();
            sim->nextInterrupt = time_now + 8000;
        }
    }

}

void doEmulation()
{
    int quit = 0;
    uint32_t lastTime = 1000 * SDL_GetTicks();
    sim->lastTimer = lastTime;
    sim->whichInterrupt = 0;

    while (quit == 0)
    {
        while(SDL_PollEvent(&event) != 0) {
		    if(event.type == SDL_QUIT) {
			    quit = 1;
			    return;
		    }
            if(event.type == SDL_KEYDOWN) {
                int key = event.key.keysym.sym;
                processKeyPress(key, sim->state);
            }

            if(event.type == SDL_KEYUP) {
                int key = event.key.keysym.sym;
                processKeyRelease(key, sim->state);
            }
        }	

        runFrame();
        DrawGraphics(sim);

    }
}

int main(int argc, char * argv[])
{
    // Make emulator file, load into RAM
    sim = (SpaceInvadersMachine *) calloc(1, sizeof(SpaceInvadersMachine));
    sim->state = (State8080*) calloc(1, sizeof(State8080));
    sim->state->memory = malloc(16 * 0x1000);
    sim->state->int_enable = 1;

    //sim->audio_player = (AudioPlayer *) calloc(1, sizeof(AudioPlayer));
    //sim->audio_player->sounds = malloc(9 * sizeof(Mix_Chunk));

    initialise_graphics(sim);
    setupAudio(sim);
    
    readFile(argv[1], 0);
    readFile(argv[2], 0x800);
    readFile(argv[3], 0x1000);
    readFile(argv[4], 0x1800);

    doEmulation();

}