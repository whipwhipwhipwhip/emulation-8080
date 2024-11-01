#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <SDL2/SDL_main.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include "emulator.h"
#include "machine.h"

#define TITLE "Space Invaders"
#define HEIGHT 256
#define WIDTH 224

SDL_Surface *surf;
int resizef;
SDL_Window *win;
SDL_Surface *winsurf;
SpaceInvadersMachine *sim;

uint8_t InPort(uint8_t port_bit)
{
    unsigned char a;
    switch(port_bit)
    {
        //Attract mode, no coin in, P2 start? why not 0? test - experiment with these when working
        case 0:
            return 1;
        //Attract mode, no player starts etc
        case 1:
            return 0;
        case 3:
        {
            uint16_t v = (sim->shift1<<8) | sim->shift0;    
            a = ((v >> (8-sim->shift_offset)) & 0xff); 
        }
    }
}

void OutPort(uint8_t port_bit)
{
    switch(port_bit)
    {
        // shift amount
        case 2:
            sim->shift_offset = port_bit & 0x7;
            break;
        // shift data
        case 4:
            sim->shift0 = sim->shift1;
            sim->shift1 = sim->shift1;
            break;
    }
    
}

void init_display() {
    // Init SDL
    if (SDL_Init(SDL_INIT_VIDEO)) {
        printf("%s\n", SDL_GetError());
        exit(1);
    }

    // Create a window
    win = SDL_CreateWindow(
            TITLE,
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            2*WIDTH, 2*HEIGHT,
            SDL_WINDOW_RESIZABLE
            );
    if (!win) {
        puts("Failed to create window");
        exit(1);
    }

    // Get surface
    winsurf = SDL_GetWindowSurface(win);
    if (!winsurf) {
        puts("Failed to get surface");
        exit(1);
    }

    // Handle resize events
    //SDL_AddEventWatch(HandleResize, NULL);

    // Create backbuffer surface
    surf = SDL_CreateRGBSurface(0, WIDTH, HEIGHT, 32, 0, 0, 0, 0);
}

void generate_interrupt(State8080* state, int interrupt_num)
{
    //perform "PUSH PC"    
    Push(state, (state->pc & 0xFF00) >> 8, (state->pc & 0xff));    

    //Set the PC to the low memory vector.    
    //This is identical to an "RST interrupt_num" instruction.    
    state->pc = 8 * interrupt_num; 
}

void run_cpu(long cycles)
{
    
    unsigned char *op;
    for (int i = 0; i < cycles; i++)
    {
        op = &sim->state->memory[sim->state->pc];
        if (*op == 0xdb) //machine specific handling for IN
        {
            sim->state->a = InPort(op[1]);
            sim->state->pc += 2;
            cycles+=3;
        }
        else if (*op == 0xd3) //machine specific handling for OUT
        {
            OutPort(op[1]);
            sim->state->pc += 2;
            cycles+=3;
        }
        else
            cycles += Emulate8080Op(sim->state);
    }
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

void doEmulation()
{
    int quit = 0;
    // set to 1Mhz because there are interrupts at the midpoint and end of frame
    static int clockSpeed = 1e6;
    static int FPS = 60;
    uint32_t lastTime = SDL_GetTicks();
    sim->lastTimer = lastTime;
    sim->whichInterrupt = 0;

    while (quit == 0)
    {
        for(int i = 0; i < 2; i ++)
        {
            sim->whichInterrupt = i+1;
            int cycles = 0;
            while (cycles < clockSpeed/FPS)
            {
                cycles = cycles + Emulate8080p(sim->state);
            }
            generate_interrupt(sim->state, sim->whichInterrupt);
        }

    }
}


int main(int argc, char * argv[])
{
    // Make emulator file, load into RAM
    sim->state = calloc(sizeof(State8080), 1);
    sim->state->memory = malloc(16 * 0x1000);

    init_display();

    readFile(argv[1], 0);
    readFile(argv[2], 0x800);
    readFile(argv[3], 0x1000);
    readFile(argv[4], 0x1800);

    doEmulation();

}