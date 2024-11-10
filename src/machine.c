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
            return a;
        }
        default:
            return a;
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
        // shift data
        case 4:
            sim->shift0 = sim->shift1;
            sim->shift1 = value;
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
    //perform "PUSH PC"    
    //Push(state, (state->pc & 0xFF00) >> 8, (state->pc & 0xff));    

    //Set the PC to the low memory vector.    
    //This is identical to an "RST interrupt_num" instruction.    
    state->pc = 8 * interrupt_num; 

    // set interrupts to null until they're reenabled
    //state->int_enable = 0;  

    printf("Just executed interrupt %x\n", interrupt_num);
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
    //uint8_t *buffer = &state->memory[offset];
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
    // set to 1Mhz because there are interrupts at the midpoint and end of frame
    static int clockSpeed = 1e6;
    static int FPS = 60;
    uint32_t lastTime = 1000 * SDL_GetTicks();
    sim->lastTimer = lastTime;
    sim->whichInterrupt = 0;
    unsigned char *op;

    while (quit == 0)
    {
        while(SDL_PollEvent(&event) != 0) {
		    if(event.type == SDL_QUIT) {
			    quit = 1;
			    return;
		    }
        }	

        runFrame();
        DrawGraphics(sim);

    }
}

void doEmulation_Old()
{
    int quit = 0;
    // set to 1Mhz because there are interrupts at the midpoint and end of frame
    static int clockSpeed = 1e6;
    static int FPS = 60;
    uint32_t lastTime = 1000 * SDL_GetTicks();
    sim->lastTimer = lastTime;
    sim->whichInterrupt = 0;
    int inst = 0;
    unsigned char *op;

    while (quit == 0)
    {
        while(SDL_PollEvent(&event) != 0) {
		    if(event.type == SDL_QUIT) {
			    quit = 1;
			    break;
		    }
	    }	

        for(int i = 0; i < 2; i ++)
        {
            sim->whichInterrupt = i+1;
            int cycles = 0;
            inst = 0;
            // change back to clockSpeed / FPS
            while (cycles < clockSpeed / FPS)
            {

                op = &sim->state->memory[sim->state->pc];
                if (*op == 0xdb) //machine specific handling for IN
                {
                    sim->state->a = InPort(op[1]);
                    sim->state->pc += 2;
                    cycles+=3;
                    inst +=1;
                }
                else if (*op == 0xd3) //machine specific handling for OUT
                {
                    OutPort(op[1], sim->state->a);
                    sim->state->pc += 2;
                    cycles+=3;
                    inst+=1;
                }
                else
                    cycles += Emulate8080p(sim->state);
                    inst+=1;
            }
            getchar();
            generate_interrupt(sim->state, sim->whichInterrupt);
            DrawGraphics(sim);
            getchar();
        }

    }
}

void doCPUTest()
{
    int success = 0;
     //Fix the first instruction to be JMP 0x100    
    state->memory[0]=0xc3;    
    state->memory[1]=0;    
    state->memory[2]=0x01;    

    //Fix the stack pointer from 0x6ad to 0x7ad    
    // this 0x06 byte 112 in the code, which is    
    // byte 112 + 0x100 = 368 in memory    
    state->memory[368] = 0x7;    

    //Skip DAA test    
    state->memory[0x59c] = 0xc3; //JMP    
    state->memory[0x59d] = 0xc2;    
    state->memory[0x59e] = 0x05;    

    int cycleBlock = 100;
    int iter = 0;

    while (1)
    {
        int cycles = 0;
        while (cycles < cycleBlock)
        {
            unsigned char *op = &state->memory[state->pc];
            if(*op == 0x0)
            {
                printf("Reached end, or failed. Exiting...\n");
                exit(0);
            }
            cycles += Emulate8080p(state);
        }
        //getchar();
        iter++;
    }
}


int main(int argc, char * argv[])
{
    // Make emulator file, load into RAM
    sim = (SpaceInvadersMachine *) calloc(1, sizeof(SpaceInvadersMachine));
    sim->state = (State8080*) calloc(1, sizeof(State8080));
    sim->state->memory = malloc(16 * 0x1000);
    sim->state->int_enable = 1;

    initialise_graphics(sim);

    //state = (State8080*) calloc(1, sizeof(State8080));
    //state->memory = malloc(16 * 0x1000);
    
    readFile(argv[1], 0);
    readFile(argv[2], 0x800);
    readFile(argv[3], 0x1000);
    readFile(argv[4], 0x1800);

    doEmulation();
    //doCPUTest();

}