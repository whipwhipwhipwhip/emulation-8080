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

SDL_Event event;

uint8_t InPort(SpaceInvadersMachine *sim, uint8_t port_bit)
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

void OutPort(SpaceInvadersMachine * sim, uint8_t port_bit)
{
    uint8_t value = sim->state->a;
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
                playSound(sim->audio_player, value, 0);
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
                playSound(sim->audio_player, value, 4);
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

void runFrame(SpaceInvadersMachine *sim)
{
    //time in microseconds
    static int clockSpeed = 1e6;
    static int FPS = 60;
    int cycles_to_run = floor(clockSpeed / FPS);
    int milisPerFrame = ceil(1000 / FPS);
    int cycles_performed = 0;
    unsigned char* op;

    // we start by setting the next interrupt time if necessary
    if(sim->lastTimer == 0.0)
    {
        printf("Time not properly Initialised - exiting...\n");
        exit(0);
    }

    
    sim->whichInterrupt = 1;
    unsigned long long time_now;

    // now run a full frame of cycles
    for(int i = 0; i < 2; i ++)
    {
        time_now = SDL_GetTicks64();
        cycles_performed = 0;
        // in here, a half frame that will culminate in an interrupt
        while(cycles_performed < cycles_to_run)
        {
            op = &sim->state->memory[sim->state->pc];
            if (*op == 0xdb) //machine specific handling for IN
            {
                sim->state->a = InPort(sim, op[1]);
                sim->state->pc += 2;
                cycles_performed+=3;
            }
            else if (*op == 0xd3) //machine specific handling for OUT
            {
                OutPort(sim, op[1]);
                sim->state->pc += 2;
                cycles_performed+=3;
            }
            else
                cycles_performed += Emulate8080p(sim->state);
        }
        // Now done, perform the interrupt
        if(sim->state->int_enable)
        {
            generate_interrupt(sim->state, sim->whichInterrupt);
            sim->whichInterrupt = (sim->whichInterrupt == 1)? 2: 1;
        }

        // Sleep until next 0.5 frame
        SDL_Delay(fmax(0LL, milisPerFrame - (SDL_GetTicks64() - time_now)));
    }

}

void doEmulation(SpaceInvadersMachine *sim)
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

        runFrame(sim);
        DrawGraphics(sim);

    }
}

/*
int main(int argc, char * argv[])
{

    sim = (SpaceInvadersMachine *) calloc(1, sizeof(SpaceInvadersMachine));
    sim->state = (State8080*) calloc(1, sizeof(State8080));
    sim->state->memory = malloc(16 * 0x1000);
    sim->state->int_enable = 1;

    initialise_graphics(sim);
    setupAudio(sim);
    
    readFile(sim->state, argv[1], 0);
    readFile(sim->state, argv[2], 0x800);
    readFile(sim->state, argv[3], 0x1000);
    readFile(sim->state, argv[4], 0x1800);

    doEmulation();

}
*/