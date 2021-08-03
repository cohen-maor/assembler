#include <stdlib.h>
#include <string.h>

#include "instructions_table.h"
#include "util.h"

instruct* instructions_table = NULL;

instruct* alloc_instruction_node(char* name, char type, char funct, char opcode)
{
	instruct* instruction; /* pointer to instruction node to allocate */

	instruction = (instruct*)malloc(sizeof(instruct));
	if (instruction == NULL)
		outofmemory();

	instruction->name = (char*)malloc(10); /* instruction length can't exceed 10 */
	if (instruction->name == NULL)
		outofmemory();
	strcpy(instruction->name, name);

	instruction->type = type; /* 'R' or 'J' or 'I' */

	instruction->funct = funct;
	
	instruction->opcode = opcode;

	return instruction;
}

void alloc_instructions_table(void)
{
	instruct* instructs_table, *current;
	
	/* no need for null checking, since it's allocated once per program running */
	current = instructs_table = alloc_instruction_node("add", 'R', 1, 0);
	
	current = current->next = alloc_instruction_node("sub", 'R', 2, 0);

	current = current->next = alloc_instruction_node("and", 'R', 3, 0);

	current = current->next = alloc_instruction_node("or", 'R', 4, 0);

	current = current->next = alloc_instruction_node("nor", 'R', 5, 0);

	current = current->next = alloc_instruction_node("move", 'R', 1, 1);

	current = current->next = alloc_instruction_node("mvhi", 'R', 2, 1);

	current = current->next = alloc_instruction_node("mvlo", 'R', 3, 1);

	/* 0 funct means no funct code */
	current = current->next = alloc_instruction_node("addi", 'I', 0, 10);

	current = current->next = alloc_instruction_node("subi", 'I', 0, 11);
	
	current = current->next = alloc_instruction_node("andi", 'I', 0, 12);

	current = current->next = alloc_instruction_node("ori", 'I', 0, 13);
	
	current = current->next = alloc_instruction_node("nori", 'I', 0, 14);

	current = current->next = alloc_instruction_node("bne", 'I', 0, 15);

	current = current->next = alloc_instruction_node("beq", 'I', 0, 16);

	current = current->next = alloc_instruction_node("blt", 'I', 0, 17);

	current = current->next = alloc_instruction_node("bgt", 'I', 0, 18);

	current = current->next = alloc_instruction_node("lb", 'I', 0, 19);

	current = current->next = alloc_instruction_node("sb", 'I', 0, 20);

	current = current->next = alloc_instruction_node("lw", 'I', 0, 21);

	current = current->next = alloc_instruction_node("sw", 'I', 0, 22);

	current = current->next = alloc_instruction_node("lh", 'I', 0, 23);

	current = current->next = alloc_instruction_node("sh", 'I', 0, 24);

	current = current->next = alloc_instruction_node("jmp", 'J', 0, 30);

	current = current->next = alloc_instruction_node("la", 'J', 0, 31);

	current = current->next = alloc_instruction_node("call", 'J', 0, 32);

	current = current->next = alloc_instruction_node("stop", 'J', 0, 63);

	current->next = NULL;

	instructions_table = instructs_table; /* assign to the global variable */
}
