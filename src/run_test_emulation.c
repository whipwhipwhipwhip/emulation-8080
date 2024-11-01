
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "dissasembler.h"
#include "emulator.h"

int main (int argc, char**argv)
{
	int done = 0;
	int steps;
	int vblankcycles = 0;
	State8080* state = Init8080();
	
	ReadFileIntoMemoryAt(state, "invaders.h", 0);
	ReadFileIntoMemoryAt(state, "invaders.g", 0x800);
	ReadFileIntoMemoryAt(state, "invaders.f", 0x1000);
	ReadFileIntoMemoryAt(state, "invaders.e", 0x1800);
	
	while (done == 0)
	{
		printf("How many steps to perform: ");
		scanf("%d", &steps);
		for(int i = 0; i < steps; i++)
		{
			done = Emulate8080p(state);
			if (done != 0) break;
		}
	}
	return 0;
}