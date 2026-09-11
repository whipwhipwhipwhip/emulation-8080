#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <math.h>
#include "emulator.h"
#include "memory.h"


State8080 *state;

void doCPUTest()
{
    int success = 0;
    //Fix the first instruction to be JMP 0x100       

    state->pc = 0x100;
    // SP=0x100 only leaves 256 bytes of stack before it collides with the
    // loaded program at 0x100 and wraps around - enough for 8080PRE.COM but
    // not for the deeper call nesting in the 8080/8085 CPU exerciser, which
    // wraps SP past 0x0000 and corrupts a return address to 0, making the
    // harness think the test finished when it never really ran. There's no
    // real OS resident above the program, so 0x0000 (wrapping down through
    // the full 64KB) is safe and gives effectively unlimited headroom.
    state->sp = 0x0000;

    //Inject RET at 0x0005 to handle Call 5
    state->memory[5]=0xc9;  

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
    state->memory = calloc(16 * 0x1000, 1);
    
    readFile(state, argv[1], 0x100);

    doCPUTest();

}