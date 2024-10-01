#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

/* Condition codes:
 * z = Zero bit: is 1 if result equal to zero
 * s = Sign bit: is 1 if MSB of math instruction is set
 * p = Parity bit: 1 when answer has even parity (in number of bits set)
 * cy = Carry bit: 1 when instruction results in carry into high order bit
 * ac = Auxiliary Carry: Not used in space invaders, for now ignore
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
	//pc will have advancced one, undo
	state->pc--;
	Disassemble8080p(state->memory, state->pc);
	printf("\n");
	exit(1);
}

int parity(int x, int size)
{
	return 1;
}
int Emulate8080p(State8080* state)
{
	unsigned char *opcode = &state->memory[state->pc];

	/* Instruction descriptions:
	 * NOP: Do Nothing
	 * LXI P: operate on pair P, most significant to first register,
	 * least significant to second register
	 */

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
		case 0x80: // ADD B
			   // We store it in higher precision to capture carry out, then shrink
			   // to needed size
			   {
			   uint16_t answer = (uint16_t) state->a + (uint_16t) state-> b;
			   state->cc.z = (answer&0xff) == 0;
			   state->cc.s = (answer&0x80) > 0;
			   state->cc.cy = (answer > 0xff);
			   state->cc.p = parity(answer&0xff, 8);
			   state->a = answer & 0xff;
			   }
			   break;
		case 0x81: // ADD C
			   {
			   uint16_t answer = (uint16_t) state->a + (uint_16t) state-> c;
			   state->cc.z = (answer&0xff) == 0;
			   state->cc.s = (answer&0x80) > 0;
			   state->cc.cy = (answer > 0xff);
			   state->cc.p = parity(answer&0xff, 8);
			   state->a = answer & 0xff;
			   }
			   break;
	}
	state->pc += 1;
