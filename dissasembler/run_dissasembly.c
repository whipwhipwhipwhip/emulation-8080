#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "dissasembler.h"

int main (int argc, char**argv)    
   {
    // argv[1] is file name (argv[0] is program name) 
    // pointer to file   
    FILE *f= fopen(argv[1], "rb");    
    if (f==NULL)    
    {    
        printf("error: Couldn't open %s\n", argv[1]);    
        exit(1);    
    }    

    //Get the file size and read it into a memory buffer  
    //1. Move pointer to end give position
    fseek(f, 0L, SEEK_END);    
    int fsize = ftell(f);    
    //2. Move back to start
    fseek(f, 0L, SEEK_SET);    

    //allocate memory to store the buffer
    unsigned char *buffer=malloc(fsize);    

    //read 1 element of size fsize to the array pointed to by buffer, from stream f
    fread(buffer, fsize, 1, f);    
    fclose(f);    

    //start at 0, go to file size, increment by how many bytes used
    int pc = 0;    

    while (pc < fsize)    
    {    
        pc += Disassemble8080p(buffer, pc);    
    }    
    return 0;    
}