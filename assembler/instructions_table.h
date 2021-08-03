#ifndef INSTRUCTIONS_TABLE_H /* header guards */
#define INSTRUCTIONS_TABLE_H

#include <stdio.h>

typedef struct instruction {
	struct instruction* next; /* next node in LL */
	char* name; /* instruction name */
	char type; /* R / J / I */
	char funct;
	char opcode;
} instruct;

/* a pointer to a linked list which included all the required instruction. */
extern instruct* instructions_table;

/* allocates an instruct node and assigns it with the given arguments - and returns it. */
instruct* alloc_instruction_node(char* name, char type, char funct, char opcode);

/* allocates and assigns the instructions table. */
void alloc_instructions_table(void);

#endif
