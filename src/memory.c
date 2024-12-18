#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "memory.h"
#include "emulator.h"

#define SpaceInvadersROM 0

void dumpState(State8080* state)
{
    	printf("\n");
		printf("%c", state->cc.z ? 'z' : '.');
		printf("%c", state->cc.s ? 's' : '.');
		printf("%c", state->cc.p ? 'p' : '.');
		printf("%c", state->cc.cy ? 'c' : '.');
		printf("%c  ", state->cc.ac ? 'a' : '.');
		printf("A $%02x B $%02x C $%02x D $%02x E $%02x H $%02x L $%02x SP %04x, INC %04x CYCLES %04x\n", state->a, state->b, state->c,
           	state->d, state->e, state->h, state->l, state->sp, 0,0);
}

//return the byte at byte
uint16_t readMemoryAt(State8080* state, uint16_t byte)
{
    #if SpaceInvadersROM
    if (byte >= 0x4000)
    {
        printf("Attempt to read out of RAM at %x. Moving to mirror byte.\n", byte);
        byte -= 0x2000;
    }
    #endif
    return state->memory[byte];
}

void writeMemoryAt(State8080* state, uint16_t address, uint8_t value)
{
    #if SpaceInvadersROM
    if (address < 0x2000)
    {
        printf("Attempt to write to ROM at %x. Now dumping state and exiting...\n", address);
        //dumpState(state);
        exit(0);
    }
    if (address >=0x4000)
    {
        printf("Attempt to write out of RAM at %x. Warning.\n", address);
        exit(0);
    }
    #endif
    state->memory[address] = value;
}

void readFile(State8080* state, char* filename, uint32_t offset)
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
	
	uint8_t *buffer = &state->memory[offset];
	fread(buffer, fsize, 1, f);
	fclose(f);
}

//do I even use this?
void * vram_location(State8080 * state)
{
    //what is this?
    return (void *) &state->memory[0x2400];
}