#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <math.h>
#include "emulator.h"


State8080 *state;

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
	
	uint8_t *buffer = &state->memory[offset];
	fread(buffer, fsize, 1, f);
	fclose(f);
}

void doCPUTest()
{
    int success = 0;
    //Fix the first instruction to be JMP 0x100    
    //state->memory[0]=0xc3;    
    //state->memory[1]=0;    
    //state->memory[2]=0x01;    

    state->pc = 0x100;
    state->sp = 0x100;

    //Inject RET at 0x0005 to handle Call 5
    state->memory[5]=0xc9;

    //Fix the stack pointer from 0x6ad to 0x7ad    
    // this 0x06 byte 112 in the code, which is    
    // byte 112 + 0x100 = 368 in memory    
    //state->memory[368] = 0x7;    

    //Skip DAA test    
    //state->memory[0x59c] = 0xc3; //JMP    
    //state->memory[0x59d] = 0xc2;    
    //state->memory[0x59e] = 0x05;    

    int cycleBlock = 100;
    int iter = 0;
    while (1)
    {
        for(int i = 0; i < 1000000; i++)
        {
            unsigned char *op = &state->memory[state->pc];
            int pc = state->pc;

            if (pc == 0x0005) {
                if (state->c == 9) {
                    int i;
                    uint16_t offset = (state->d<<8) | (state->e);  
                    for (i = offset; state->memory[i] != '$'; i += 1)
                        putchar(state->memory[i]);
                    success = 1;
                }
                if (state->c == 2) putchar((char)state->e);
            }

            Emulate8080p(state);
            if(state->pc == 0)
            {
                printf("\nReached end, or failed?? Success is %x, jumped to 0 from %04X\n", state->success, pc);
                if (state->success)
                    exit(1);
                return;
            }
        }
        //getchar();
    }
}


int main(int argc, char * argv[])
{
    // Make emulator file, load into RAM
    state = (State8080*) calloc(1, sizeof(State8080));
    state->memory = malloc(16 * 0x1000);
    
    readFile(argv[1], 0x100);

    doCPUTest();

}