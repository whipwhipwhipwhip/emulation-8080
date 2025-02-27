#ifndef EMULATOR_H
#define EMULATOR_H
#include<stdint.h>
#include "memory.h"

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
	uint8_t     inst;
	uint16_t	sp;
	uint16_t    lastSp;
	uint16_t	pc;
	uint16_t    port1;
	uint16_t    port2;
	uint16_t    prevWrite3;
	uint16_t    prevWrite5;
	uint8_t		*memory;
	struct		ConditionCodes cc;
	uint8_t		int_enable;
	uint8_t     success;
} State8080;

int Emulate8080p(State8080* state);
void writeMemoryAt(State8080* state, uint16_t address, uint8_t value);
uint16_t readMemoryAt(State8080* state, uint16_t byte);

#endif
