#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "dissasembler.h"
#include "emulator.h"

/* Condition codes:
 * z = Zero bit: is 1 if result equal to zero
 * s = Sign bit: is 1 if MSB of math instruction is set
 * p = Parity bit: 1 when answer has even parity (in number of bits set)
 * cy = Carry bit: 1 when instruction results in carry out of 7 bit
 * ac = Auxiliary Carry: Not used in space invaders - when interaction results in carry out from 3 bit
 * pad = ???
 */

void UnimplementedInstruction(State8080* state)
{
	//pc will have advanced one, undo
	state->pc--;
	printf("Instruction not implemented, exiting...\n");
	Disassemble8080p(state->memory, state->pc);
	printf("\n");
	exit(1);
}

int parity(int x, int size)
{
	int i;
	int p = 0;
	x = (x & ((1<<size)-1));
	for (i = 0; i<size; i++)
	{
		if (x & 0x01) p++;
		x = x >> 1;
	} 
	return (0 == (p & 0x01));
}

void arithmeticFlags(State8080 *state, uint16_t answer, uint16_t s2)
{
	uint16_t s1 = (uint16_t) state->a;
	state->cc.z = (answer&0xff) == 0;
	state->cc.s = (answer&0x80) > 0;
	state->cc.cy = (answer > 0xff);
	state->cc.p = parity(answer&0xff, 8);
	// To test auxiliary carry, see if sum of individual bits equals result bit
	uint8_t summands = (s1 & 0x08) + (s2 & 0x08);
	uint8_t res_bit = (answer & 0x08);
	state->cc.ac = (summands == 16) | (summands == 8 & res_bit == 0);
}

void logicFlags(State8080 *state)
{
	state->cc.cy = 0;
	state->cc.z = state->a == 0;
	state->cc.s = (state->a&0x80) > 0;
	state->cc.p = parity(state->a, 8);
}

uint8_t memoryFromHL(State8080 *state)
{
	uint16_t answer = (state->h << 8) | state->l;
	return state->memory[answer];
}

# define PRINTOPS 1

int Emulate8080p(State8080* state)
{
	unsigned char *opcode = &state->memory[state->pc];

	/* Instruction descriptions:
	 * NOP: Do Nothing
	 * LXI P: operate on pair P, most significant to first register,
	 * least significant to second register
	 */

	#if PRINTOPS
			printf("%04x\n", opcode);
    		//Disassemble8080p(state->memory, state->pc);
	#endif
	uint16_t prev_pc = state->pc;
	state->pc += 1;

	switch(*opcode)
	{
		case 0x00: // NOP 
			   break; 
		case 0x01: // LXI B
			   {
			   state->c = opcode[1];
			   state->b = opcode[2];
			   state->pc += 2;
			   }
			   break;
		case 0x03: // INX B
			   {
			   state->c = state->c + 1;
			   if (state->c == 0)
				state->b++;
			   }
			   break;
		case 0x04: // INR B
			   {
			   uint16_t answer = (uint16_t) state->b + 1;
			   uint8_t pv_cy = state->cc.cy;
			   arithmeticFlags(state, answer, 0x01);
			   state->cc.cy = pv_cy;
			   state->b = (uint8_t) answer;
			   }
			   break;
		case 0x05: // DCR B
			   {
			   uint16_t s2 = ~(0x01) + 1;
			   uint16_t answer = (uint16_t) state->b + s2;
			   uint8_t pv_cy = state->cc.cy;
			   arithmeticFlags(state, answer, s2);
			   state->cc.cy = pv_cy;
			   state->b = (uint8_t) answer;
			   }
			   break;
		case 0x06: // MVI B
			   {
			   state->b = opcode[1];
			   state->pc += 1;
			   }
			   break;
		case 0x07: // RLC
			   {
			   state->a = (state->a << 1) | (state->a & 0xf0) >> 7;
			   state->cc.cy = state->a & 1;
			   }
			   break;
		case 0x09: // DAD B
			   {
			   uint32_t s1 = state->h << 8 | state->l;
			   uint32_t s2 = state->b << 8 | state->c;
			   uint32_t answer = s1 + s2;
			   state->cc.cy = answer > 0xffff;
			   state->h = (answer & 0xff00) >> 8;
			   state->l = (answer & 0xff);
			   }
			   break;
		case 0x0a: // LDAX B
			   {
			   uint16_t offset = state->b << 8 | state->c;
			   state->a = state->memory[offset];
			   }
			   break;
		case 0x0b: // DCX B
			   {
			   state->b = state->b - 1;
			   state->c = state->c - 1;
			   }
			   break;
		case 0x0c: // INR C
			   {
			   uint16_t answer = (uint16_t) state->c + 1;
			   uint8_t pv_cy = state->cc.cy;
			   arithmeticFlags(state, answer, 0x01);
			   state->cc.cy = pv_cy;
			   state->c = (uint8_t) answer;
			   }
			   break;
		case 0x0d: // DCR C
			   {
			   uint16_t s2 = ~(0x01) + 1;
			   uint16_t answer = (uint16_t) state->c + s2;
			   uint8_t pv_cy = state->cc.cy;
			   arithmeticFlags(state, answer, s2);
			   state->cc.cy = pv_cy;
			   state->c = (uint8_t) answer;
			   }
			   break;
		case 0x0e: // MVI C
			   {
			   state->c = opcode[1];
			   state->pc += 1;
			   }
			   break;
		case 0x0f: // RRC
			   {
			   state->cc.cy = state->a & 1;
			   state->a = (state->a >> 1) | (state->a & 1) << 7;
			   }
			   break;
		case 0x11: // LXI D
			   {
			   state->e = opcode[1];
			   state->d = opcode[2];
			   state->pc += 2;
			   }
			   break;
		case 0x13: // INX D
			   {
			   state->e++;
			   if (state->e == 0)
				state->d++;
			   }
			   break;
		case 0x14: // INR D
			   {
			   uint16_t answer = (uint16_t) state->d + 1;
			   uint8_t pv_cy = state->cc.cy;
			   arithmeticFlags(state, answer, 0x01);
			   state->cc.cy = pv_cy;
			   state->d = (uint8_t) answer;
			   }
			   break;
		case 0x15: // DCR D
			   {
			   uint16_t s2 = ~(0x01) + 1;
			   uint16_t answer = (uint16_t) state->d + s2;
			   uint8_t pv_cy = state->cc.cy;
			   arithmeticFlags(state, answer, s2);
			   state->cc.cy = pv_cy;
			   state->d = (uint8_t) answer;
			   }
			   break;
		case 0x16: // MVI D
			   {
			   state->d = opcode[1];
			   state->pc += 1;
			   }
			   break;
		case 0x19: // DAD D
			   {
			   uint32_t s1 = state->h << 8 | state->l;
			   uint32_t s2 = state->d << 8 | state->e;
			   uint32_t answer = s1 + s2;
			   state->cc.cy = answer > 0xffff;
			   state->h = (answer & 0xff00) >> 8;
			   state->l = (answer & 0xff);
			   }
			   break;
		case 0x1a: // LDAX D
			   {
			   uint16_t offset = state->d << 8 | state->e;
			   state->a = state->memory[offset];
			   }
			   break;
		case 0x1b: // DCX D
			   {
			   state->d = state->d - 1;
			   state->e = state->e - 1;
			   }
			   break;
		case 0x1c: // INR E
			   {
			   uint16_t answer = (uint16_t) state->e + 1;
			   uint8_t pv_cy = state->cc.cy;
			   arithmeticFlags(state, answer, 0x01);
			   state->cc.cy = pv_cy;
			   state->e = (uint8_t) answer;
			   }
			   break;
		case 0x1d: // DCR E
			   {
			   uint16_t s2 = ~(0x01) + 1;
			   uint16_t answer = (uint16_t) state->e + s2;
			   uint8_t pv_cy = state->cc.cy;
			   arithmeticFlags(state, answer, s2);
			   state->cc.cy = pv_cy;
			   state->e = (uint8_t) answer;
			   }
			   break;
		case 0x1e: // MVI E
			   {
			   state->e = opcode[1];
			   state->pc += 1;
			   }
			   break;
		case 0x21: // LXI H
			   {
			   state->l = opcode[1];
			   state->h = opcode[2];
			   state->pc += 2;
			   }
			   break;
		case 0x23: // INX H
			   {
			   state->l++;
			   if (state->l == 0)
				state->h++;
			   }
			   break;
		case 0x24: // INR H
			   {
			   uint16_t answer = (uint16_t) state->h + 1;
			   uint8_t pv_cy = state->cc.cy;
			   arithmeticFlags(state, answer, 0x01);
			   state->cc.cy = pv_cy;
			   state->h = (uint8_t) answer;
			   }
			   break;
		case 0x25: // DCR H
			   {
			   uint16_t s2 = ~(0x01) + 1;
			   uint16_t answer = (uint16_t) state->h + s2;
			   uint8_t pv_cy = state->cc.cy;
			   arithmeticFlags(state, answer, s2);
			   state->cc.cy = pv_cy;
			   state->h = (uint8_t) answer;
			   }
			   break;
		case 0x26: // MVI H
			   {
			   state->h = opcode[1];
			   state->pc += 1;
			   }
			   break;
		case 0x29: // DAD H
			   {
			   uint32_t s1 = state->h << 8 | state->l;
			   uint32_t s2 = state->h << 8 | state->l;
			   uint32_t answer = s1 + s2;
			   state->cc.cy = answer > 0xffff;
			   state->h = (answer & 0xff00) >> 8;
			   state->l = (answer & 0xff);
			   }
			   break;
		case 0x2b: // DCX H
			   {
			   state->h = state->h - 1;
			   state->l = state->l - 1;
			   }
			   break;
		case 0x2c: // INR L
			   {
			   uint16_t answer = (uint16_t) state->l + 1;
			   uint8_t pv_cy = state->cc.cy;
			   arithmeticFlags(state, answer, 0x01);
			   state->cc.cy = pv_cy;
			   state->l = (uint8_t) answer;
			   }
			   break;
		case 0x2d: // DCR L
			   {
			   uint16_t s2 = ~(0x01) + 1;
			   uint16_t answer = (uint16_t) state->l + s2;
			   uint8_t pv_cy = state->cc.cy;
			   arithmeticFlags(state, answer, s2);
			   state->cc.cy = pv_cy;
			   state->l = (uint8_t) answer;
			   }
			   break;
		case 0x2e: // MVI L
			   {
			   state->l = opcode[1];
			   state->pc += 1;
			   }
			   break;
		case 0x2f: // CMA
			   {
			   state->a = ~state->a;
		           }
			   break;	
		case 0x31: // LXI SP
			   {
			   state->sp = (opcode[2] << 8) | (opcode[1] & 0x08);
			   state->pc += 2;
			   }
			   break;
		case 0x32: // STA
			   {
			   uint16_t memloc = (opcode[2] << 8) | (opcode[1]);
			   state->memory[memloc] = state->a;
			   state->pc += 2;
			   }
			   break;
		case 0x33: // INX SP
			   {
			   state->sp = state->sp + 1;
			   }
			   break;
		case 0x34: // INR M
			   {
			   uint16_t answer = memoryFromHL(state) + 1;
			   uint8_t pv_cy = state->cc.cy;
			   arithmeticFlags(state, answer, 0x01);
			   state->cc.cy = pv_cy;
			   state->h = (answer & 0xff00) >> 8;
			   state->l = answer & 0xff;
			   }
			   break;
		case 0x35: // DCR M
			   {
			   uint16_t s2 = ~(0x01) + 1;
			   uint16_t answer = memoryFromHL(state) + s2;
			   uint8_t pv_cy = state->cc.cy;
			   arithmeticFlags(state, answer, s2);
			   state->cc.cy = pv_cy;
			   state->h = (answer & 0xff00) >> 8;
			   state->l = answer & 0xff;
			   }
			   break;
		case 0x36: // MVI M
			   {
			   uint16_t mem_set = (state->h << 8) | state->l;
			   state->memory[mem_set] = opcode[1];
			   state->pc += 1;
			   }
			   break;
		case 0x39: // DAD SP
			   {
			   uint32_t s1 = state->h << 8 | state->l;
			   uint32_t s2 = state->sp;
			   uint32_t answer = s1 + s2;
			   state->cc.cy = answer > 0xffff;
			   state->h = (answer & 0xff00) >> 8;
			   state->l = (answer & 0xff);
			   }
			   break;
		case 0x3a: // LDA
			   {
			   uint16_t memloc = (opcode[2] << 8) | (opcode[1]);
			   state->a = state->memory[memloc];
			   state->pc += 2;
			   }
			   break;
		case 0x3b: // DCX SP
			   {
			   state->sp = state->sp - 1;
			   }
			   break;
		case 0x3c: // INR A
			   {
			   uint16_t answer = (uint16_t) state->a + 1;
			   uint8_t pv_cy = state->cc.cy;
			   arithmeticFlags(state, answer, 0x01);
			   state->cc.cy = pv_cy;
			   state->a = (uint8_t) answer;
			   }
			   break;
		case 0x3d: // DCR A
			   {
			   uint16_t s2 = ~(0x01) + 1;
			   uint16_t answer = (uint16_t) state->a + s2;
			   uint8_t pv_cy = state->cc.cy;
			   arithmeticFlags(state, answer, s2);
			   state->cc.cy = pv_cy;
			   state->a = (uint8_t) answer;
			   }
			   break;
		case 0x3e: // MVI A
			   {
			   state->a = opcode[1];
			   state->pc += 1;
			   }
			   break;
		case 0x40: // MOV B,B
			   {
			   state->b = state->b;
			   }
			   break;
		case 0x41: // MOV B,C
			   {
			   state->b = state->c;
			   }
			   break;
		case 0x42: // MOV B,D
			   {
			   state->b = state->d;
			   }
			   break;
		case 0x43: // MOV B,E
			   {
			   state->b = state->e;
			   }
			   break;
		case 0x44: // MOV B,H
			   {
			   state->b = state->h;
			   }
			   break;
		case 0x45: // MOV B,L
			   {
			   state->b = state->l;
			   }
			   break;
		case 0x46: // MOV B,M
			   {
			   state->b = memoryFromHL(state);
			   }
			   break;
		case 0x47: // MOV B,A
			   {
			   state->b = state->a;
			   }
			   break;
		case 0x48: // MOV C,B
			   {
			   state->c = state->b;
			   }
			   break;
		case 0x49: // MOV C,C
			   {
			   state->c = state->c;
			   }
			   break;
		case 0x4a: // MOV C,D
			   {
			   state->c = state->d;
			   }
			   break;
		case 0x4b: // MOV C,E
			   {
			   state->c = state->e;
			   }
			   break;
		case 0x4c: // MOV C,H
			   {
			   state->c = state->h;
			   }
			   break;
		case 0x4d: // MOV C,L
			   {
			   state->c = state->l;
			   }
			   break;
		case 0x4e: // MOV C,M
			   {
			   state->c = memoryFromHL(state);
			   }
			   break;
		case 0x4f: // MOV C,A
			   {
			   state->c = state->a;
			   }
			   break;
		case 0x50: // MOV D,B
			   {
			   state->d = state->b;
			   }
			   break;
		case 0x51: // MOV D,C
			   {
			   state->d = state->c;
			   }
			   break;
		case 0x52: // MOV D,D
			   {
			   state->d = state->d;
			   }
			   break;
		case 0x53: // MOV D,E
			   {
			   state->d = state->e;
			   }
			   break;
		case 0x54: // MOV D,H
			   {
			   state->d = state->h;
			   }
			   break;
		case 0x55: // MOV D,L
			   {
			   state->d = state->l;
			   }
			   break;
		case 0x56: // MOV D,M
			   {
			   state->d = memoryFromHL(state);
			   }
			   break;
		case 0x57: // MOV D,A
			   {
			   state->d = state->a;
			   }
			   break;
		case 0x58: // MOV E,B
			   {
			   state->e = state->b;
			   }
			   break;
		case 0x59: // MOV E,C
			   {
			   state->e = state->c;
			   }
			   break;
		case 0x5a: // MOV E,D
			   {
			   state->e = state->d;
			   }
			   break;
		case 0x5b: // MOV E,E
			   {
			   state->e = state->e;
			   }
			   break;
		case 0x5c: // MOV E,H
			   {
			   state->e = state->h;
			   }
			   break;
		case 0x5d: // MOV E,L
			   {
			   state->e = state->l;
			   }
			   break;
		case 0x5e: // MOV E,M
			   {
			   state->e = memoryFromHL(state);
			   }
			   break;
		case 0x5f: // MOV E,A
			   {
			   state->e = state->a;
			   }
			   break;
		case 0x60: // MOV H,B
			   {
			   state->h = state->b;
			   }
			   break;
		case 0x61: // MOV H,C
			   {
			   state->h = state->c;
			   }
			   break;
		case 0x62: // MOV H,D
			   {
			   state->h = state->d;
			   }
			   break;
		case 0x63: // MOV H,E
			   {
			   state->h = state->e;
			   }
			   break;
		case 0x64: // MOV H,H
			   {
			   state->h = state->h;
			   }
			   break;
		case 0x65: // MOV H,L
			   {
			   state->h = state->l;
			   }
			   break;
		case 0x66: // MOV H,M
			   {
			   state->h = memoryFromHL(state);
			   }
			   break;
		case 0x67: // MOV H,A
			   {
			   state->h = state->a;
			   }
			   break;
		case 0x68: // MOV L,B
			   {
			   state->l = state->b;
			   }
			   break;
		case 0x69: // MOV L,C
			   {
			   state->l = state->c;
			   }
			   break;
		case 0x6a: // MOV L,D
			   {
			   state->l = state->d;
			   }
			   break;
		case 0x6b: // MOV L,E
			   {
			   state->l = state->e;
			   }
			   break;
		case 0x6c: // MOV L,H
			   {
			   state->l = state->h;
			   }
			   break;
		case 0x6d: // MOV L,L
			   {
			   state->l = state->l;
			   }
			   break;
		case 0x6e: // MOV L,M
			   {
			   state->l = memoryFromHL(state);
			   }
			   break;
		case 0x6f: // MOV L,A
			   {
			   state->l = state->a;
			   }
			   break;
		case 0x70: // MOV M,B
			   {
			   uint16_t memloc = (state->h << 8) | state->l;
			   state->memory[memloc] = state->b;
			   }
			   break;
		case 0x71: // MOV M,C
			   {
			   uint16_t memloc = (state->h << 8) | state->l;
			   state->memory[memloc] = state->c;
			   }
			   break;
		case 0x72: // MOV M,D
			   {
			   uint16_t memloc = (state->h << 8) | state->l;
			   state->memory[memloc] = state->d;
			   }
			   break;
		case 0x73: // MOV M,E
			   {
			   uint16_t memloc = (state->h << 8) | state->l;
			   state->memory[memloc] = state->e;
			   }
			   break;
		case 0x74: // MOV M,H
			   {
			   uint16_t memloc = (state->h << 8) | state->l;
			   state->memory[memloc] = state->h;
			   }
			   break;
		case 0x75: // MOV M,L
			   {
			   uint16_t memloc = (state->h << 8) | state->l;
			   state->memory[memloc] = state->l;
			   }
			   break;
		case 0x77: // MOV M,A
			   {
			   uint16_t memloc = (state->h << 8) | state->l;
			   state->memory[memloc] = state->a;
			   }
			   break;
		case 0x78: // MOV A,B
			   {
			   state->a = state->b;
			   }
			   break;
		case 0x79: // MOV A,C
			   {
			   state->a = state->c;
			   }
			   break;
		case 0x7a: // MOV A,D
			   {
			   state->a = state->d;
			   }
			   break;
		case 0x7b: // MOV A,E
			   {
			   state->a = state->e;
			   }
			   break;
		case 0x7c: // MOV A,H
			   {
			   state->a = state->h;
			   }
			   break;
		case 0x7d: // MOV A,L
			   {
			   state->a = state->l;
			   }
			   break;
		case 0x7e: // MOV A,M
			   {
			   state->a = memoryFromHL(state);
			   }
			   break;
		case 0x7f: // MOV A,A
			   {
			   state->a = state->a;
			   }
			   break;
		case 0x80: // ADD B
			   // We store it in higher precision to capture carry out, then shrink
			   // to needed size
			   {
			   uint16_t s2 = (uint16_t) state->b;
			   uint16_t answer = (uint16_t) state->a + s2;
			   arithmeticFlags(state, answer, s2);
			   state->a = answer & 0xff;
			   }
			   break;
		case 0x81: // ADD C
			   {
			   uint16_t s2 = (uint16_t) state->c;
			   uint16_t answer = (uint16_t) state->a + s2;
			   arithmeticFlags(state, answer, s2);
			   state->a = answer & 0xff;
			   }
			   break;
		case 0x82: // ADD D
			   {
			   uint16_t s2 = (uint16_t) state->d;
			   uint16_t answer = (uint16_t) state->a + s2;
			   arithmeticFlags(state, answer, s2);
			   state->a = answer & 0xff;
			   }
			   break;
		case 0x83: // ADD E
			   {
			   uint16_t s2 = (uint16_t) state->e;
			   uint16_t answer = (uint16_t) state->a + s2;
			   arithmeticFlags(state, answer, s2);
			   state->a = answer & 0xff;
			   }
			   break;
		case 0x84: // ADD H
			   {
			   uint16_t s2 = (uint16_t) state->h;
			   uint16_t answer = (uint16_t) state->a + s2;
			   arithmeticFlags(state, answer, s2);
			   state->a = answer & 0xff;
			   }
			   break;
		case 0x85: // ADD L
			   {
			   uint16_t s2 = (uint16_t) state->l;
			   uint16_t answer = (uint16_t) state->a + s2;
			   arithmeticFlags(state, answer, s2);
			   state->a = answer & 0xff;
			   }
			   break;
		case 0x86: // ADD M
			   {
			   uint16_t s2 = memoryFromHL(state);
			   uint16_t answer = (uint16_t) state->a + s2;
			   arithmeticFlags(state, answer, s2);
			   state->a = answer & 0xff;
			   }
			   break;
		case 0x87: // ADD A
			   {
			   uint16_t s2 = (uint16_t) state->a;
			   uint16_t answer = (uint16_t) state->a + s2;
			   arithmeticFlags(state, answer, s2);
			   state->a = answer & 0xff;
			   }
			   break;
		case 0x88: // ADC B
			   {
			   uint16_t s2 = (uint16_t) state->b + state->cc.cy;
			   uint16_t answer = (uint16_t) state->a + s2;
			   arithmeticFlags(state, answer, s2);
			   state->a = answer & 0xff;
			   }
			   break;
		case 0x89: // ADC C
			   {
			   uint16_t s2 = (uint16_t) state->c + state->cc.cy;
			   uint16_t answer = (uint16_t) state->a + s2;
			   arithmeticFlags(state, answer, s2);
			   state->a = answer & 0xff;
			   }
			   break;
		case 0x8a: // ADC D
			   {
			   uint16_t s2 = (uint16_t) state->d + state->cc.cy;
			   uint16_t answer = (uint16_t) state->a + s2;
			   arithmeticFlags(state, answer, s2);
			   state->a = answer & 0xff;
			   }
			   break;
		case 0x8b: // ADC E
			   {
			   uint16_t s2 = (uint16_t) state->e + state->cc.cy;
			   uint16_t answer = (uint16_t) state->a + s2;
			   arithmeticFlags(state, answer, s2);
			   state->a = answer & 0xff;
			   }
			   break;
		case 0x8c: // ADC H
			   {
			   uint16_t s2 = (uint16_t) state->h + state->cc.cy;
			   uint16_t answer = (uint16_t) state->a + s2;
			   arithmeticFlags(state, answer, s2);
			   state->a = answer & 0xff;
			   }
			   break;
		case 0x8d: // ADC L
			   {
			   uint16_t s2 = (uint16_t) state->l + state->cc.cy;
			   uint16_t answer = (uint16_t) state->a + s2;
			   arithmeticFlags(state, answer, s2);
			   state->a = answer & 0xff;
			   }
			   break;
		case 0x8e: // ADC M
			   {
			   uint16_t s2 = memoryFromHL(state) + state->cc.cy;
			   uint16_t answer = (uint16_t) state->a + s2;
			   arithmeticFlags(state, answer, s2);
			   state->a = answer & 0xff;
			   }
			   break;
		case 0x8f: // ADC A
			   {
			   uint16_t s2 = (uint16_t) state->a + state->cc.cy;
			   uint16_t answer = (uint16_t) state->a + s2;
			   arithmeticFlags(state, answer, s2);
			   state->a = answer & 0xff;
			   }
			   break;
		case 0x90: // SUB B
			   {
			   uint16_t s2 = ~((uint16_t) state->b) + 1;
			   uint16_t answer = (uint16_t) state->a + s2;
			   arithmeticFlags(state, answer, s2);
			   state->a = answer & 0xff;
			   }
			   break;
		case 0x91: // SUB C
			   {
			   uint16_t s2 = ~((uint16_t) state->c) + 1;
			   uint16_t answer = (uint16_t) state->a + s2;
			   arithmeticFlags(state, answer, s2);
			   state->a = answer & 0xff;
			   }
			   break;
		case 0x92: // SUB D
			   {
			   uint16_t s2 = ~((uint16_t) state->d) + 1;
			   uint16_t answer = (uint16_t) state->a + s2;
			   arithmeticFlags(state, answer, s2);
			   state->a = answer & 0xff;
			   }
			   break;
		case 0x93: // SUB E
			   {
			   uint16_t s2 = ~((uint16_t) state->e) + 1;
			   uint16_t answer = (uint16_t) state->a + s2;
			   arithmeticFlags(state, answer, s2);
			   state->a = answer & 0xff;
			   }
			   break;
		case 0x94: // SUB H
			   {
			   uint16_t s2 = ~((uint16_t) state->h) + 1;
			   uint16_t answer = (uint16_t) state->a + s2;
			   arithmeticFlags(state, answer, s2);
			   state->a = answer & 0xff;
			   }
			   break;
		case 0x95: // SUB L
			   {
			   uint16_t s2 = ~((uint16_t) state->l) + 1;
			   uint16_t answer = (uint16_t) state->a + s2;
			   arithmeticFlags(state, answer, s2);
			   state->a = answer & 0xff;
			   }
			   break;
		case 0x96: // SUB M
			   {
			   uint16_t s2 = ~memoryFromHL(state) + 1;
			   uint16_t answer = (uint16_t) state->a + s2;
			   arithmeticFlags(state, answer, s2);
			   state->a = answer & 0xff;
			   }
			   break;
		case 0x97: // SUB A
			   {
			   uint16_t s2 = ~((uint16_t) state->a) + 1;
			   uint16_t answer = (uint16_t) state->a + s2;
			   arithmeticFlags(state, answer, s2);
			   state->a = answer & 0xff;
			   }
			   break;
		case 0x98: // SBB B
			   {
			   uint16_t s2 = ~((uint16_t) state->b + state->cc.cy) + 1;
			   uint16_t answer = (uint16_t) state->a + s2;
			   arithmeticFlags(state, answer, s2);
			   state->a = answer & 0xff;
			   }
			   break;
		case 0x99: // SBB C
			   {
			   uint16_t s2 = ~((uint16_t) state->c + state->cc.cy) + 1;
			   uint16_t answer = (uint16_t) state->a + s2;
			   arithmeticFlags(state, answer, s2);
			   state->a = answer & 0xff;
			   }
			   break;
		case 0x9a: // SBB D
			   {
			   uint16_t s2 = ~((uint16_t) state->d + state->cc.cy) + 1;
			   uint16_t answer = (uint16_t) state->a + s2;
			   arithmeticFlags(state, answer, s2);
			   state->a = answer & 0xff;
			   }
			   break;
		case 0x9b: // SBB E
			   {
			   uint16_t s2 = ~((uint16_t) state->e + state->cc.cy) + 1;
			   uint16_t answer = (uint16_t) state->a + s2;
			   arithmeticFlags(state, answer, s2);
			   state->a = answer & 0xff;
			   }
			   break;
		case 0x9c: // SBB H
			   {
			   uint16_t s2 = ~((uint16_t) state->h + state->cc.cy) + 1;
			   uint16_t answer = (uint16_t) state->a + s2;
			   arithmeticFlags(state, answer, s2);
			   state->a = answer & 0xff;
			   }
			   break;
		case 0x9d: // SBB L
			   {
			   uint16_t s2 = ~((uint16_t) state->l + state->cc.cy) + 1;
			   uint16_t answer = (uint16_t) state->a + s2;
			   arithmeticFlags(state, answer, s2);
			   state->a = answer & 0xff;
			   }
			   break;
		case 0x9e: // SBB M
			   {
			   uint16_t s2 = ~(memoryFromHL(state) + state->cc.cy) + 1;
			   uint16_t answer = (uint16_t) state->a + s2;
			   arithmeticFlags(state, answer, s2);
			   state->a = answer & 0xff;
			   }
			   break;
		case 0x9f: // SBB A
			   {
			   uint16_t s2 = ~((uint16_t) state->a + state->cc.cy) + 1;
			   uint16_t answer = (uint16_t) state->a + s2;
			   arithmeticFlags(state, answer, s2);
			   state->a = answer & 0xff;
			   }
			   break;
		case 0xa0: // ANA B
			   {
			   state->a = state->a & state->b;
			   logicFlags(state);
			   }
			   break;
		case 0xa1: // ANA C
			   {
			   state->a = state->a & state->c;
			   logicFlags(state);
			   }
			   break;
		case 0xa2: // ANA D
			   {
			   state->a = state->a & state->d;
			   logicFlags(state);
			   }
			   break;
		case 0xa3: // ANA E
			   {
			   state->a = state->a & state->e;
			   logicFlags(state);
			   }
			   break;
		case 0xa4: // ANA H
			   {
			   state->a = state->a & state->h;
			   logicFlags(state);
			   }
			   break;
		case 0xa5: // ANA L
			   {
			   state->a = state->a & state->l;
			   logicFlags(state);
			   }
			   break;
		case 0xa6: // ANA M
			   {
			   state->a = state->a & memoryFromHL(state);
			   logicFlags(state);
			   }
			   break;
		case 0xa7: // ANA A
			   {
			   state->a = state->a & state->a;
			   logicFlags(state);
			   }
			   break;
		case 0xa8: // XRA B
			   {
			   state->a = state->a ^ state->b;
			   logicFlags(state);
			   }
			   break;
		case 0xa9: // XRA C
			   {
			   state->a = state->a ^ state->c;
			   logicFlags(state);
			   }
			   break;
		case 0xaa: // XRA D
			   {
			   state->a = state->a ^ state->d;
			   logicFlags(state);
			   }
			   break;
		case 0xab: // XRA E
			   {
			   state->a = state->a ^ state->e;
			   logicFlags(state);
			   }
			   break;
		case 0xac: // XRA H
			   {
			   state->a = state->a ^ state->h;
			   logicFlags(state);
			   }
			   break;
		case 0xad: // XRA L
			   {
			   state->a = state->a ^ state->l;
			   logicFlags(state);
			   }
			   break;
		case 0xae: // XRA M
			   {
			   state->a = state->a ^ memoryFromHL(state);
			   logicFlags(state);
			   }
			   break;
		case 0xaf: // XRA A
			   {
			   state->a = state->a ^ state->a;
			   logicFlags(state);
			   }
			   break;
		case 0xb0: // ORA B
			   {
			   state->a = state->a | state->b;
			   logicFlags(state);
			   }
			   break;
		case 0xb1: // ORA C
			   {
			   state->a = state->a | state->c;
			   logicFlags(state);
			   }
			   break;
		case 0xb2: // ORA D
			   {
			   state->a = state->a | state->d;
			   logicFlags(state);
			   }
			   break;
		case 0xb3: // ORA E
			   {
			   state->a = state->a | state->e;
			   logicFlags(state);
			   }
			   break;
		case 0xb4: // ORA H
			   {
			   state->a = state->a | state->h;
			   logicFlags(state);
			   }
			   break;
		case 0xb5: // ORA L
			   {
			   state->a = state->a | state->l;
			   logicFlags(state);
			   }
			   break;
		case 0xb6: // ORA M
			   {
			   state->a = state->a | memoryFromHL(state);
			   logicFlags(state);
			   }
			   break;
		case 0xb7: // ORA A
			   {
			   state->a = state->a | state->a;
			   logicFlags(state);
			   }
			   break;
		case 0xb8: // CMP B
			   {
			   uint16_t s2 = ~((uint16_t) state->b) + 1;
			   uint16_t answer = state->a + s2;
			   arithmeticFlags(state, answer, s2);
			   }
			   break;
		case 0xb9: // CMP C
			   {
			   uint16_t s2 = ~((uint16_t) state->c) + 1;
			   uint16_t answer = state->a + s2;
			   arithmeticFlags(state, answer, s2);
			   }
			   break;
		case 0xba: // CMP D
			   {
			   uint16_t s2 = ~((uint16_t) state->d) + 1;
			   uint16_t answer = state->a + s2;
			   arithmeticFlags(state, answer, s2);
			   }
			   break;
		case 0xbb: // CMP E
			   {
			   uint16_t s2 = ~((uint16_t) state->e) + 1;
			   uint16_t answer = state->a + s2;
			   arithmeticFlags(state, answer, s2);
			   }
			   break;
		case 0xbc: // CMP H
			   {
			   uint16_t s2 = ~((uint16_t) state->h) + 1;
			   uint16_t answer = state->a + s2;
			   arithmeticFlags(state, answer, s2);
			   }
			   break;
		case 0xbd: // CMP L
			   {
			   uint16_t s2 = ~((uint16_t) state->l) + 1;
			   uint16_t answer = state->a + s2;
			   arithmeticFlags(state, answer, s2);
			   }
			   break;
		case 0xbe: // CMP M
			   {
			   uint16_t s2 = ~((uint16_t) memoryFromHL(state)) + 1;
			   uint16_t answer = state->a + s2;
			   arithmeticFlags(state, answer, s2);
			   }
			   break;
		case 0xbf: // CMP A
			   {
			   uint16_t s2 = ~((uint16_t) state->a) + 1;
			   uint16_t answer = state->a + s2;
			   arithmeticFlags(state, answer, s2);
			   }
			   break;
		case 0xc0: // RNZ
			   {
			   if (state->cc.z == 0)
			   	state->pc = state->memory[state->sp] | state->memory[state->sp+1] << 8;
			   	state->sp += 2;
			   }
			   break;
		case 0xc1: //POP B
			   {
			   state->c = state->memory[state->sp];
			   state->b = state->memory[state->sp+1];
			   state->sp += 2;
			   }
			   break;
		case 0xc2: // JNZ
			   {
			   if (state->cc.z == 0)
				state->pc = opcode[2] << 8 | opcode[1];
			   else
				state->pc += 2;
			   }
			   break;
		case 0xc3: // JMP
			   {
			   state->pc = opcode[2] << 8 | opcode[1];
			   }
			   break;
		case 0xc4: // CNZ
			   {
			   if (state->cc.z == 1)
			   {
            		   	uint16_t ret = state->pc+2;    
            		   	state->memory[state->sp-1] = (ret >> 8) & 0xff;    
            		   	state->memory[state->sp-2] = (ret & 0xff);    
            		   	state->sp = state->sp - 2;    
            		   	state->pc = (opcode[2] << 8) | opcode[1];    
			   }
			   else
		           {
				state->pc += 2;
			   }
            		   }   
			   break;
		case 0xc5: // PUSH B
			   {
				state->memory[state->sp-1] = state->b;
				state->memory[state->sp-2] = state->c;
				state->sp -= 2;
			   }
			   break;
		case 0xc6: // ADI
			   {
			   uint16_t s2 = (uint16_t) opcode[1];
			   uint16_t answer = (uint16_t) state->a + s2;
			   arithmeticFlags(state, answer, s2);
			   state->a = answer & 0xff;
			   state->pc++;
			   }
			   break;
		case 0xc7: // RST 0
			   {
			   uint16_t push = state->pc+2;
			   state->memory[state->sp-1] = push >> 8 & 0xff;
			   state->memory[state->sp-2] = push & 0xff;
			   state->sp -= 2;
			   state->pc = 0x0000;
			   }
			   break;
		case 0xc8: // RZ
			   {
			   if (state->cc.z == 1)
			   	state->pc = state->memory[state->sp] | state->memory[state->sp+1] << 8;
			   	state->sp += 2;
			   }
			   break;
		case 0xc9: // RET
			   state->pc = state->memory[state->sp] | state->memory[state->sp+1] << 8;
			   state->sp += 2; 
			   break;
		case 0xca: // JZ
			   {
			   if (state->cc.z == 1)
				state->pc = opcode[2] << 8 | opcode[1];
			   else
				state->pc += 2;
			   }
			   break;
		case 0xcc: // CZ
			   {
			   if (state->cc.z == 0)
			   {
            		   	uint16_t ret = state->pc+2;    
            		   	state->memory[state->sp-1] = (ret >> 8) & 0xff;    
            		   	state->memory[state->sp-2] = (ret & 0xff);    
            		   	state->sp = state->sp - 2;    
            		   	state->pc = (opcode[2] << 8) | opcode[1];    
			   }
			   else
		           {
				state->pc += 2;
			   }
            		   }   
			   break;
		case 0xcd: // CALL
			   {    
            		   uint16_t ret = state->pc+2;    
            		   state->memory[state->sp-1] = (ret >> 8) & 0xff;    
            		   state->memory[state->sp-2] = (ret & 0xff);    
            		   state->sp = state->sp - 2;    
            		   state->pc = (opcode[2] << 8) | opcode[1];    
            		   }    
			   break;
		case 0xce: // ACI
			   {
			   uint16_t s2 = (uint16_t) opcode[1] + state->cc.cy;
			   uint16_t answer = (uint16_t) state->a + s2;
			   arithmeticFlags(state, answer, s2);
			   state->a = answer & 0xff;
			   state->pc++;
			   }
			   break;
		case 0xcf: // RST 1
			   {
			   uint16_t push = state->pc+2;
			   state->memory[state->sp-1] = push >> 8 & 0xff;
			   state->memory[state->sp-2] = push & 0xff;
			   state->sp -= 2;
			   state->pc = 0x0008;
			   }
			   break;
		case 0xd0: // RNC
			   {
			   if (state->cc.cy == 0)
			   	state->pc = state->memory[state->sp] | state->memory[state->sp+1] << 8;
			   	state->sp += 2;
			   }
			   break;
		case 0xd1: //POP D
			   {
			   state->e = state->memory[state->sp];
			   state->d = state->memory[state->sp+1];
			   state->sp += 2;
			   }
			   break;
		case 0xd2: // JNC
			   {
			   if (state->cc.cy == 0)
				state->pc = opcode[2] << 8 | opcode[1];
			   else
				state->pc += 2;
			   }
			   break;
		case 0xd3: // OUT
			   state->pc++;
			   break;
		case 0xd4: // CNC
			   {
			   if (state->cc.cy == 0)
			   {
            		   	uint16_t ret = state->pc+2;    
            		   	state->memory[state->sp-1] = (ret >> 8) & 0xff;    
            		   	state->memory[state->sp-2] = (ret & 0xff);    
            		   	state->sp = state->sp - 2;    
            		   	state->pc = (opcode[2] << 8) | opcode[1];    
			   }
			   else
		           {
				state->pc += 2;
			   }
            		   }   
			   break;
		case 0xd5: // PUSH D
			   {
				state->memory[state->sp-1] = state->d;
				state->memory[state->sp-2] = state->e;
				state->sp -= 2;
			   }
			   break;
		case 0xd6: // SUI
			   {
			   uint16_t s2 = ~((uint16_t) opcode[1]) + 1;
			   uint16_t answer = (uint16_t) state->a + s2;
			   arithmeticFlags(state, answer, s2);
			   state->a = answer & 0xff;
			   state->pc++;
			   }
			   break;
		case 0xd7: // RST 2
			   {
			   uint16_t push = state->pc+2;
			   state->memory[state->sp-1] = push >> 8 & 0xff;
			   state->memory[state->sp-2] = push & 0xff;
			   state->sp -= 2;
			   state->pc = 0x0010;
			   }
			   break;
		case 0xd8: // RC
			   {
			   if (state->cc.cy == 1)
			   	state->pc = state->memory[state->sp] | state->memory[state->sp+1] << 8;
			   	state->sp += 2;
			   }
			   break;
		case 0xda: // JC
			   {
			   if (state->cc.cy == 1)
				state->pc = opcode[2] << 8 | opcode[1];
			   else
				state->pc += 2;
			   }
			   break;
		case 0xdb: // IN
			   state->pc++;
			   break;
		case 0xdc: // CC
			   {
			   if (state->cc.cy == 1)
			   {
            		   	uint16_t ret = state->pc+2;    
            		   	state->memory[state->sp-1] = (ret >> 8) & 0xff;    
            		   	state->memory[state->sp-2] = (ret & 0xff);    
            		   	state->sp = state->sp - 2;    
            		   	state->pc = (opcode[2] << 8) | opcode[1];    
			   }
			   else
		           {
				state->pc += 2;
			   }
            		   }   
			   break;
		case 0xde: // SBI
			   {
			   uint16_t s2 = ~((uint16_t) opcode[1] + state->cc.cy) + 1;
			   uint16_t answer = (uint16_t) state->a + s2;
			   arithmeticFlags(state, answer, s2);
			   state->a = answer & 0xff;
			   state->pc++;
			   }
			   break;
		case 0xdf: // RST 3
			   {
			   uint16_t push = state->pc+2;
			   state->memory[state->sp-1] = push >> 8 & 0xff;
			   state->memory[state->sp-2] = push & 0xff;
			   state->sp -= 2;
			   state->pc = 0x0018;
			   }
			   break;
		case 0xe0: // RPO
			   {
			   if (state->cc.p == 0)
			   	state->pc = state->memory[state->sp] | state->memory[state->sp+1] << 8;
			   	state->sp += 2;
			   }
			   break;
		case 0xe1: // POP H
			   {
			   state->l = state->memory[state->sp];
			   state->h = state->memory[state->sp+1];
			   state->sp += 2;
			   }
			   break;
		case 0xe2: // JPO
			   {
			   if (state->cc.p == 1)
				state->pc = opcode[2] << 8 | opcode[1];
			   else
				state->pc += 2;
			   }
			   break;
		case 0xe4: // CPO
			   {
			   if (state->cc.p == 0)
			   {
            		   	uint16_t ret = state->pc+2;    
            		   	state->memory[state->sp-1] = (ret >> 8) & 0xff;    
            		   	state->memory[state->sp-2] = (ret & 0xff);    
            		   	state->sp = state->sp - 2;    
            		   	state->pc = (opcode[2] << 8) | opcode[1];    
			   }
			   else
		           {
				state->pc += 2;
			   }
            		   }   
			   break;
		case 0xe5: // PUSH H
			   {
				state->memory[state->sp-1] = state->h;
				state->memory[state->sp-2] = state->l;
				state->sp -= 2;
			   }
			   break;
		case 0xe6: // ANI
			   {
				state->a = state->a & opcode[1];
				state->pc++;
				logicFlags(state);
			   }
			   break;
		case 0xe7: // RST 4
			   {
			   uint16_t push = state->pc+2;
			   state->memory[state->sp-1] = push >> 8 & 0xff;
			   state->memory[state->sp-2] = push & 0xff;
			   state->sp -= 2;
			   state->pc = 0x0020;
			   }
			   break;
		case 0xe8: // RPE
			   {
			   if (state->cc.p == 1)
			   	state->pc = state->memory[state->sp] | state->memory[state->sp+1] << 8;
			   	state->sp += 2;
			   }
			   break;
		case 0xe9: // PCHL
			   {
				state->pc = memoryFromHL(state);
			   }
			   break;
		case 0xea: // JPE
			   {
			   if (state->cc.p == 1)
				state->pc = opcode[2] << 8 | opcode[1];
			   else
				state->pc += 2;
			   }
			   break;
		case 0xeb: // XCHG
			   {
				uint8_t tmp_1 = state->h;
				uint8_t tmp_2 = state->l;
				state->h = state->d;
				state->l = state->e;
				state->d = tmp_1;
				state->e = tmp_2;
			   }
		 	   break;
		case 0xec: // CPE
			   {
			   if (state->cc.p == 1)
			   {
            		   	uint16_t ret = state->pc+2;    
            		   	state->memory[state->sp-1] = (ret >> 8) & 0xff;    
            		   	state->memory[state->sp-2] = (ret & 0xff);    
            		   	state->sp = state->sp - 2;    
            		   	state->pc = (opcode[2] << 8) | opcode[1];    
			   }
			   else
		           {
				state->pc += 2;
			   }
            		   }   
			   break;
		case 0xee: // XRI
			   {
				state->a = state->a ^ opcode[1];
				state->pc++;
				logicFlags(state);
			   }
			   break;
		case 0xef: // RST 5
			   {
			   uint16_t push = state->pc+2;
			   state->memory[state->sp-1] = push >> 8 & 0xff;
			   state->memory[state->sp-2] = push & 0xff;
			   state->sp -= 2;
			   state->pc = 0x0028;
			   }
			   break;
		case 0xf0: // RP
			   {
			   if (state->cc.s == 0)
			   	state->pc = state->memory[state->sp] | state->memory[state->sp+1] << 8;
			   	state->sp += 2;
			   }
			   break;
		case 0xf1: //POP PSW
			   {
			   state->a = state->memory[state->sp+1];
			   uint8_t psw = state->memory[state->sp];
			   state->cc.z  = (0x01 == (psw & 0x01));
			   state->cc.s  = (0x02 == (psw & 0x02));
			   state->cc.p  = (0x04 == (psw & 0x04));
			   state->cc.cy = (0x05 == (psw & 0x08));
			   state->cc.ac = (0x10 == (psw & 0x10));
			   state->sp += 2;
			   }
			   break;
		case 0xf2: // JP
			   {
			   if (state->cc.s == 0)
				state->pc = opcode[2] << 8 | opcode[1];
			   else
				state->pc += 2;
			   }
			   break;
		case 0xf3: // DI
			   {
				state->int_enable = 0;
			   }
			   break;
		case 0xf4: // CP
			   {
			   if (state->cc.s == 0)
			   {
            		   	uint16_t ret = state->pc+2;    
            		   	state->memory[state->sp-1] = (ret >> 8) & 0xff;    
            		   	state->memory[state->sp-2] = (ret & 0xff);    
            		   	state->sp = state->sp - 2;    
            		   	state->pc = (opcode[2] << 8) | opcode[1];    
			   }
			   else
		           {
				state->pc += 2;
			   }
            		   }   
			   break;
		case 0xf5: // PUSH PSW
			   {
				state->memory[state->sp-1] = state->a;    
            			uint8_t psw = (state->cc.z |    
                            			state->cc.s << 1 |    
                            			state->cc.p << 2 |    
                            			state->cc.cy << 3 |    
                            	state->cc.ac << 4 ); 
				state->memory[state->sp-2] = psw;
				state->sp -= 2;
			   }
			   break;
		case 0xf6: // ORI
			   {
				state->a = state->a | opcode[1];
				state->pc++;
				logicFlags(state);
			   }
			   break;
		case 0xf7: // RST 6
			   {
			   uint16_t push = state->pc+2;
			   state->memory[state->sp-1] = push >> 8 & 0xff;
			   state->memory[state->sp-2] = push & 0xff;
			   state->sp -= 2;
			   state->pc = 0x0030;
			   }
			   break;
		case 0xf8: // RM
			   {
			   if (state->cc.s == 1)
			   	state->pc = state->memory[state->sp] | state->memory[state->sp+1] << 8;
			   	state->sp += 2;
			   }
			   break;
		case 0xfa: // JM
			   {
			   if (state->cc.s == 1)
				state->pc = opcode[2] << 8 | opcode[1];
			   else
				state->pc += 2;
			   }
			   break;
		case 0xfb: // EI
			   {
				state->int_enable = 1;
			   }
			   break;
		case 0xfc: // CM
			   {
			   if (state->cc.s == 1)
			   {
            		   	uint16_t ret = state->pc+2;    
            		   	state->memory[state->sp-1] = (ret >> 8) & 0xff;    
            		   	state->memory[state->sp-2] = (ret & 0xff);    
            		   	state->sp = state->sp - 2;    
            		   	state->pc = (opcode[2] << 8) | opcode[1];    
			   }
			   else
		           {
				state->pc += 2;
			   }
            		   }   
			   break;
		case 0xfe: // CPI 
			   {
				state->pc++;
				uint16_t s2 = ~((uint16_t) opcode[1]) + 1;
			   	uint16_t answer = (uint16_t) state->a + s2;
			   	arithmeticFlags(state, answer, s2);
			   }
			   break;
		case 0xff: // RST 7
			   {
			   uint16_t push = state->pc+2;
			   state->memory[state->sp-1] = push >> 8 & 0xff;
			   state->memory[state->sp-2] = push & 0xff;
			   state->sp -= 2;
			   state->pc = 0x0038;
			   }
			   break;
		default: UnimplementedInstruction(state); break;
	}
	#if PRINTOPS
		printf("\t");
		printf("%c", state->cc.z ? 'z' : '.');
		printf("%c", state->cc.s ? 's' : '.');
		printf("%c", state->cc.p ? 'p' : '.');
		printf("%c", state->cc.cy ? 'c' : '.');
		printf("%c  ", state->cc.ac ? 'a' : '.');
		printf("A $%02x B $%02x C $%02x D $%02x E $%02x H $%02x L $%02x SP %04x, INC %04x\n", state->a, state->b, state->c,
           	state->d, state->e, state->h, state->l, state->sp, state->pc - prev_pc);
	#endif
	return 1;
}

void ReadFileIntoMemoryAt(State8080* state, char* filename, uint32_t offset)
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

State8080* Init8080(void)
{
	State8080* state = calloc(1,sizeof(State8080));
	state->memory = malloc(0x10000);  //16K
	return state;
}
