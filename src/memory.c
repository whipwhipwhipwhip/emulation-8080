#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "memory.h"

//read file into memory
/*
void readFile(State8080* state, char* filename, uint32_t memoffset)
{    

    FILE *f= fopen(filename, "rb");    
    if (f==NULL)    
    {    
        printf("error: Couldn't open %s\n", filename);    
        exit(1);    
    }    

    //Get the file size and read it into a memory buffer  
    fseek(f, 0L, SEEK_END);    
    int fsize = ftell(f);    
    fseek(f, 0L, SEEK_SET);    

    //allocate memory to store the buffer
    unsigned char *buffer=malloc(fsize);    
    //read 1 element of size fsize to the array pointed to by buffer, from stream f
    fread(buffer, fsize, 1, f);   
    fclose(f);  

    uint8_t *buffer_mem = &state->memory[memoffset];
    memcpy(buffer_mem, buffer, fsize); 
    free(buffer);

}
*/

//return the byte at byte
uint16_t readMemoryAt(State8080* state, uint16_t byte)
{
    
    if (byte >= 0x4000)
    {
        printf("Attempt to read out of RAM at %x. Now exiting...\n", byte);
    }
    return state->memory[byte];
}

void writeMemoryAt(State8080* state, uint16_t address, uint8_t value)
{
    
    if (address < 0x2000)
    {
        printf("Attempt to write to ROM at %x. Now exiting...\n", address);
        exit(0);
    }
    if (address >=0x4000)
    {
        printf("Attempt to write out of RAM at %x. Warning.\n", address);
        exit(0);
    }
    state->memory[address] = value;
}

//do I even use this?
void * vram_location(State8080 * state)
{
    //what is this?
    return (void *) &state->memory[0x2400];
}