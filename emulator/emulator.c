#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

/* Condition codes:
 * z = Zero bit: is 1 if result equal to zero
 * s = Sign bit: is 1 if MSB of math instruction is set
 * p = Parity bit: 1 when answer has even parity (in number of bits set)
 * cy = Carry bit: 1 when instruction results in carry out of 7 bit
 * ac = Auxiliary Carry: Not used in space invaders - when interaction results in carry out from 3 bit
 * pad = ???
 */

typedef struct ConditionCodes {
	// Colon means use only that number of bits
	// uint8_t unsigned 8 bit integer
	uint8_t		z:1;
	uint8_t		s:1;
	uint8_t		p:1;
	uint8_t		cy:1;
	uint8_t		ac:1;
	uint8_t		pad:3;
} ConditionCodes;

typedef struct State8080 {
	uint8_t		a;
	uint8_t		b;
	uint8_t		c;
	uint8_t		d;
	uint8_t		e;
	uint8_t		h;
	uint8_t		l;
	uint16_t	sp;
	uint16_t	pc;
	uint8_t		*memory;
	struct		ConditionCodes cc;
	uint8_t		int_enable;
} State8080;

void UnimplementedInstruction(State8080* state)
{
	//pc will have advanced one, undo
	state->pc--;
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
		if (x & 0x1) p++;
		x = x >> 1;
	} 
	return (0 = (p & 0x1));
}

void arithmeticFlags(State8080 *state, uint16_t answer, uint16_t s2)
{
	uint16_t s1 = (uint16_t) state->a
	state->cc.z = (answer&0xff) == 0;
	state->cc.s = (answer&0x80) > 0;
	state->cc.cy = (answer > 0xff);
	state->cc.p = parity(answer&0xff, 8);
	// To test auxiliary carry, see if sum of individual bits equals result bit
	uint8_t summands = (s1 & 0x08) + (s2 & 0x08)
	uint8_t res_bit = (answer & 0x08)
	state->cc.ac = (summands == 16) | (summands == 8 & res_bit == 0)
}

uint16_t memoryFromHL(State8080 *state)
{
	answer = (state->h << 8) | state->l
	return answer
}

int Emulate8080p(State8080* state)
{
	unsigned char *opcode = &state->memory[state->pc];

	/* Instruction descriptions:
	 * NOP: Do Nothing
	 * LXI P: operate on pair P, most significant to first register,
	 * least significant to second register
	 */

	state->pc += 1;

	switch(*opcode)
	{
		case 0x00: // NOP 
			   break; 
		case 0x01: // LXI B
			   {
			   state->c = opcode[1];
			   state->b = opcode[2];
			   }
			   break;
		case 0x03: // INX B
			   {
			   state->b = state->b + 1;
			   state->c = state->c + 1;
			   }
			   break;
		case 0x04: // INR B
			   {
			   uint16_t answer = (uint16_t) state->b + 1;
			   pv_cy = state->cc.cy;
			   arithmeticFlags(state, answer, 0x01);
			   state->cc.cy = pv_cy;
			   state->b = (uint8_t) answer;
			   }
			   break;
		case 0x05: // DCR B
			   {
			   uint16_t s2 = ~(0x01) + 1;
			   uint16_t answer = (uint16_t) state->b + s2;
			   pv_cy = state->cc.cy;
			   arithmeticFlags(state, answer, s2);
			   state->cc.cy = pv_cy;
			   state->b = (uint8_t) answer;
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
		case 0x0b: // DCX B
			   {
			   state->b = state->b - 1;
			   state->c = state->c - 1;
			   }
			   break;
		case 0x0c: // INR C
			   {
			   uint16_t answer = (uint16_t) state->c + 1;
			   pv_cy = state->cc.cy;
			   arithmeticFlags(state, answer, 0x01);
			   state->cc.cy = pv_cy;
			   state->c = (uint8_t) answer;
			   }
			   break;
		case 0x0d: // DCR C
			   {
			   uint16_t s2 = ~(0x01) + 1;
			   uint16_t answer = (uint16_t) state->c + s2;
			   pv_cy = state->cc.cy;
			   arithmeticFlags(state, answer, s2);
			   state->cc.cy = pv_cy;
			   state->c = (uint8_t) answer;
			   }
			   break;
		case 0x13: // INX D
			   {
			   state->d = state->d + 1;
			   state->e = state->e + 1;
			   }
			   break;
		case 0x14: // INR D
			   {
			   uint16_t answer = (uint16_t) state->d + 1;
			   pv_cy = state->cc.cy;
			   arithmeticFlags(state, answer, 0x01);
			   state->cc.cy = pv_cy;
			   state->d = (uint8_t) answer;
			   }
			   break;
		case 0x15: // DCR D
			   {
			   uint16_t s2 = ~(0x01) + 1;
			   uint16_t answer = (uint16_t) state->d + s2;
			   pv_cy = state->cc.cy;
			   arithmeticFlags(state, answer, s2);
			   state->cc.cy = pv_cy;
			   state->d = (uint8_t) answer;
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
		case 0x1b: // DCX D
			   {
			   state->d = state->d - 1;
			   state->e = state->e - 1;
			   }
			   break;
		case 0x1c: // INR E
			   {
			   uint16_t answer = (uint16_t) state->e + 1;
			   pv_cy = state->cc.cy;
			   arithmeticFlags(state, answer, 0x01);
			   state->cc.cy = pv_cy;
			   state->e = (uint8_t) answer;
			   }
			   break;
		case 0x1d: // DCR E
			   {
			   uint16_t s2 = ~(0x01) + 1;
			   uint16_t answer = (uint16_t) state->e + s2;
			   pv_cy = state->cc.cy;
			   arithmeticFlags(state, answer, s2);
			   state->cc.cy = pv_cy;
			   state->e = (uint8_t) answer;
			   }
			   break;
		case 0x23: // INX H
			   {
			   state->h = state->h + 1;
			   state->l = state->l + 1;
			   }
			   break;
		case 0x24: // INR H
			   {
			   uint16_t answer = (uint16_t) state->h + 1;
			   pv_cy = state->cc.cy;
			   arithmeticFlags(state, answer, 0x01);
			   state->cc.cy = pv_cy;
			   state->h = (uint8_t) answer;
			   }
			   break;
		case 0x25: // DCR H
			   {
			   uint16_t s2 = ~(0x01) + 1;
			   uint16_t answer = (uint16_t) state->h + s2;
			   pv_cy = state->cc.cy;
			   arithmeticFlags(state, answer, s2);
			   state->cc.cy = pv_cy;
			   state->h = (uint8_t) answer;
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
			   pv_cy = state->cc.cy;
			   arithmeticFlags(state, answer, 0x01);
			   state->cc.cy = pv_cy;
			   state->l = (uint8_t) answer;
			   }
			   break;
		case 0x2d: // DCR L
			   {
			   uint16_t s2 = ~(0x01) + 1;
			   uint16_t answer = (uint16_t) state->l + s2;
			   pv_cy = state->cc.cy;
			   arithmeticFlags(state, answer, s2);
			   state->cc.cy = pv_cy;
			   state->l = (uint8_t) answer;
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
			   pv_cy = state->cc.cy;
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
			   pv_cy = state->cc.cy;
			   arithmeticFlags(state, answer, s2);
			   state->cc.cy = pv_cy;
			   state->h = (answer & 0xff00) >> 8;
			   state->l = answer & 0xff;
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
		case 0x3b: // DCX SP
			   {
			   state->sp = state->sp - 1;
			   }
			   break;
		case 0x3c: // INR A
			   {
			   uint16_t answer = (uint16_t) state->a + 1;
			   pv_cy = state->cc.cy;
			   arithmeticFlags(state, answer, 0x01);
			   state->cc.cy = pv_cy;
			   state->a = (uint8_t) answer;
			   }
			   break;
		case 0x3d: // DCR A
			   {
			   uint16_t s2 = ~(0x01) + 1;
			   uint16_t answer = (uint16_t) state->a + s2;
			   pv_cy = state->cc.cy;
			   arithmeticFlags(state, answer, s2);
			   state->cc.cy = pv_cy;
			   state->a = (uint8_t) answer;
			   }
			   break;
		case 0x80: // ADD B
			   // We store it in higher precision to capture carry out, then shrink
			   // to needed size
			   {
			   uint16_t s2 = (uint_16t) state->b;
			   uint16_t answer = (uint16_t) state->a + s2;
			   arithmeticFlags(state, answer, s2);
			   state->a = answer & 0xff;
			   }
			   break;
		case 0x81: // ADD C
			   {
			   uint16_t s2 = (uint_16t) state->c;
			   uint16_t answer = (uint16_t) state->a + s2;
			   arithmeticFlags(state, answer, s2);
			   state->a = answer & 0xff;
			   }
			   break;
		case 0x82: // ADD D
			   {
			   uint16_t s2 = (uint_16t) state->d;
			   uint16_t answer = (uint16_t) state->a + s2;
			   arithmeticFlags(state, answer, s2);
			   state->a = answer & 0xff;
			   }
			   break;
		case 0x83: // ADD E
			   {
			   uint16_t s2 = (uint_16t) state->e;
			   uint16_t answer = (uint16_t) state->a + s2;
			   arithmeticFlags(state, answer, s2);
			   state->a = answer & 0xff;
			   }
			   break;
		case 0x84: // ADD H
			   {
			   uint16_t s2 = (uint_16t) state->h;
			   uint16_t answer = (uint16_t) state->a + s2;
			   arithmeticFlags(state, s2);
			   state->a = answer & 0xff;
			   }
			   break;
		case 0x85: // ADD L
			   {
			   uint16_t s2 = (uint_16t) state->l;
			   uint16_t answer = (uint16_t) state->a + s2;
			   arithmeticFlags(state, answer, s2);
			   state->a = answer & 0xff;
			   }
			   break;
		case 0x86: // ADD M
			   {
			   uint16_t s2 = memoryFromHL(state);
			   uint16_t answer = (uint16_t) state->a + s2
			   arithmeticFlags(state, answer, s2);
			   state->a = answer & 0xff;
			   }
			   break;
		case 0x87: // ADD A
			   {
			   uint16_t s2 = (uint_16t) state->a;
			   uint16_t answer = (uint16_t) state->a + s2
			   arithmeticFlags(state, answer, s2);
			   state->a = answer & 0xff;
			   }
			   break;
		case 0x88: // ADC B
			   {
			   uint16_t s2 = (uint_16t) state->b + state->cc.cy;
			   uint16_t answer = (uint16_t) state->a + s2
			   arithmeticFlags(state, answer, s2);
			   state->a = answer & 0xff;
			   }
			   break;
		case 0x89: // ADC C
			   {
			   uint16_t s2 = (uint_16t) state->c + state->cc.cy;
			   uint16_t answer = (uint16_t) state->a + s2;
			   arithmeticFlags(state, answer, s2);
			   state->a = answer & 0xff;
			   }
			   break;
		case 0x8a: // ADC D
			   {
			   uint16_t s2 = (uint_16t) state->d + state->cc.cy;
			   uint16_t answer = (uint16_t) state->a + s2;
			   arithmeticFlags(state, answer, s2);
			   state->a = answer & 0xff;
			   }
			   break;
		case 0x8b: // ADC E
			   {
			   uint16_t s2 = (uint_16t) state->e + state->cc.cy;
			   uint16_t answer = (uint16_t) state->a + s2;
			   arithmeticFlags(state, answer, s2);
			   state->a = answer & 0xff;
			   }
			   break;
		case 0x8c: // ADC H
			   {
			   uint16_t s2 = (uint_16t) state->h + state->cc.cy;
			   uint16_t answer = (uint16_t) state->a + s2;
			   arithmeticFlags(state, answer, s2);
			   state->a = answer & 0xff;
			   }
			   break;
		case 0x8d: // ADC L
			   {
			   uint16_t s2 = (uint_16t) state->l + state->cc.cy;
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
		case 0xcc: // CNZ
			   {
			   if (state->cc.z == 1)
            		   	uint16_t ret = state->pc+2;    
            		   	state->memory[state->sp-1] = (ret >> 8) & 0xff;    
            		   	state->memory[state->sp-2] = (ret & 0xff);    
            		   	state->sp = state->sp - 2;    
            		   	state->pc = (opcode[2] << 8) | opcode[1];    
			   else
				state->pc += 2
            		   }   
		case 0xc6: // ADI
			   {
			   uint16_t s2 = (uint16_t) opcode[1];
			   uint16_t answer = (uint16_t) state->a + s2;
			   arithmeticFlags(state, answer, s2);
			   state->a = answer & 0xff;
			   state->pc++;
			   }
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
            		   	uint16_t ret = state->pc+2;    
            		   	state->memory[state->sp-1] = (ret >> 8) & 0xff;    
            		   	state->memory[state->sp-2] = (ret & 0xff);    
            		   	state->sp = state->sp - 2;    
            		   	state->pc = (opcode[2] << 8) | opcode[1];    
			   else
				state->pc += 2
            		   }   
		case 0xcd: // CALL
			   {    
            		   uint16_t ret = state->pc+2;    
            		   state->memory[state->sp-1] = (ret >> 8) & 0xff;    
            		   state->memory[state->sp-2] = (ret & 0xff);    
            		   state->sp = state->sp - 2;    
            		   state->pc = (opcode[2] << 8) | opcode[1];    
            		   }    
		case 0xce: // ACI
			   {
			   uint16_t s2 = (uint16_t) opcode[1] + state->cc.cy;
			   uint16_t answer = (uint16_t) state->a + s2;
			   arithmeticFlags(state, answer, s2);
			   state->a = answer & 0xff;
			   state->pc++;
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
		case 0xd4: // CNC
			   {
			   if (state->cc.cy == 0)
            		   	uint16_t ret = state->pc+2;    
            		   	state->memory[state->sp-1] = (ret >> 8) & 0xff;    
            		   	state->memory[state->sp-2] = (ret & 0xff);    
            		   	state->sp = state->sp - 2;    
            		   	state->pc = (opcode[2] << 8) | opcode[1];    
			   else
				state->pc += 2
            		   }   
		case 0xd6: // SUI
			   {
			   uint16_t s2 = ~((uint16_t) opcode[1]) + 1;
			   uint16_t answer = (uint16_t) state->a + s2;
			   arithmeticFlags(state, answer, s2);
			   state->a = answer & 0xff;
			   state->pc++;
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
		case 0xdc: // CC
			   {
			   if (state->cc.cy == 1)
            		   	uint16_t ret = state->pc+2;    
            		   	state->memory[state->sp-1] = (ret >> 8) & 0xff;    
            		   	state->memory[state->sp-2] = (ret & 0xff);    
            		   	state->sp = state->sp - 2;    
            		   	state->pc = (opcode[2] << 8) | opcode[1];    
			   else
				state->pc += 2
            		   }    
		case 0xde: // SBI
			   {
			   uint16_t s2 = ~((uint16_t) opcode[1] + state->cc.cy) + 1
			   uint16_t answer = (uint16_t) state->a + s2;
			   arithmeticFlags(state, answer, s2);
			   state->a = answer & 0xff;
			   state->pc++;
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
		case 0xcc: // CPO
			   {
			   if (state->cc.p == 0)
            		   	uint16_t ret = state->pc+2;    
            		   	state->memory[state->sp-1] = (ret >> 8) & 0xff;    
            		   	state->memory[state->sp-2] = (ret & 0xff);    
            		   	state->sp = state->sp - 2;    
            		   	state->pc = (opcode[2] << 8) | opcode[1];    
			   else
				state->pc += 2
            		   }   
		case 0xea: // JPE
			   {
			   if (state->cc.p == 1)
				state->pc = opcode[2] << 8 | opcode[1];
			   else
				state->pc += 2;
			   }
			   break;
		case 0xec: // CPE
			   {
			   if (state->cc.p == 1)
            		   	uint16_t ret = state->pc+2;    
            		   	state->memory[state->sp-1] = (ret >> 8) & 0xff;    
            		   	state->memory[state->sp-2] = (ret & 0xff);    
            		   	state->sp = state->sp - 2;    
            		   	state->pc = (opcode[2] << 8) | opcode[1];    
			   else
				state->pc += 2
            		   }   
		case 0xf2: // JP
			   {
			   if (state->cc.s == 0)
				state->pc = opcode[2] << 8 | opcode[1];
			   else
				state->pc += 2;
			   }
			   break;
		case 0xf4: // CP
			   {
			   if (state->cc.s == 0)
            		   	uint16_t ret = state->pc+2;    
            		   	state->memory[state->sp-1] = (ret >> 8) & 0xff;    
            		   	state->memory[state->sp-2] = (ret & 0xff);    
            		   	state->sp = state->sp - 2;    
            		   	state->pc = (opcode[2] << 8) | opcode[1];    
			   else
				state->pc += 2
            		   }   
		case 0xfa: // JM
			   {
			   if (state->cc.s == 1)
				state->pc = opcode[2] << 8 | opcode[1];
			   else
				state->pc += 2;
			   }
			   break;
		case 0xfc: // CM
			   {
			   if (state->cc.s == 1)
            		   	uint16_t ret = state->pc+2;    
            		   	state->memory[state->sp-1] = (ret >> 8) & 0xff;    
            		   	state->memory[state->sp-2] = (ret & 0xff);    
            		   	state->sp = state->sp - 2;    
            		   	state->pc = (opcode[2] << 8) | opcode[1];    
			   else
				state->pc += 2
            		   }   
	}
