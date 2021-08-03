#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "memorymap.h"
#include "symboltable.h"
#include "instructions_table.h"
#include "util.h"
#include "externinstruct.h"

static FILE* asmfile; /* the pointer to the file for processing */
static char* filename; /* the name of the file to open */
static char* instruction = NULL; /* the current instruction / label */
static char c; /* simple variable for holding characters */
static int error = 0; /* if 1, output files won't be created. if 0, then they might will. */
static int line = 0; /* current line */
static int ic; /* code buffer indexer */

void secondpass(char* fname)
{
	if (fname != NULL) /* if first call of this input file */
	{
		asmfile = fopen(fname, "r"); /* open the file to process using reading access privilege */
		if (asmfile == NULL) /* if couldn't open the file */
		{
			printf("Given permission to read the file is missing, or the file: %s, doesn't exist.\n", fname);
			return;
		}
		filename = fname; /* fname is a local variable, but will remain in the stack because of recursion */
	}
	if (instruction == NULL) /* if it's the first call of secondpass in this running */
	{
		instruction = malloc(MAX_LINE + 1);
		ic = memoryoffset; /* first instruction index is memoryoffset */
	}

	line++;

	while ((c = getc(asmfile)) == '\t' || c == ' '); /* skip whitespaces and tabs */
	if (c == '\n')
	{
		secondpass(NULL);
		return;
	}
	else if (c == EOF) /* if program has read all of the file */
	{
		if (!error) /* if no errors */
		{
			writeobjfile(filename); /* create and write to OBJECT file */

			write_entries(filename); /* create and write to ENTRIES file */

			write_externinstruct(filename); /* create and write to EXTERNALS file */
		}
		
		line = 0;
		error = 0;
		ic = memoryoffset; /* starting index of ic */

		return;
	}
	else /* this is a keyword to process */
		ungetc(c, asmfile);

	(void)fscanf(asmfile, "%s", instruction);
	if (*instruction == ';') /* if comment */
	{
		readline(asmfile);
		secondpass(NULL);
		return;
	}
	else if (instruction[strlen(instruction) - 1] == ':') /* if label */
		(void)fscanf(asmfile, "%s", instruction); /* ignore */
	if (isdataline(instruction) || extern_or_entry(instruction) == 1) /* if .extern / .db / .dh / .dw / .asciz */
	{
		readline(asmfile);
		secondpass(NULL);
		return;
	}
	else if (extern_or_entry(instruction) == 2) /* if .entry */
	{
		int symbolstatus; /* return of addsymbol */
		
		(void)fscanf(asmfile, "%s", instruction);
		symbolstatus = turn_on_entry_att(instruction); /* add entry attribute to the symbol with the name that instruction var holds */
		
		if (symbolstatus == 0) /* symbol isn't defined */
		{
			printf("%s, line: %d, no definition was found for symbol '%s'.\n", filename, line, instruction);
			error = 1;
		}
		else if (symbolstatus == 3) /* symbol is declared as external */
		{
			printf("%s, line: %d, symbol can't be declared as external and as an entry together in the same input file.\n", filename, line);
			error = 1;
		}
		readline(asmfile);
		secondpass(NULL); /* read next line */
		return;
	}
	else /* instruction line */
	{
		instruct* command; /* the instruction */
		command = instructions_table;
		while (strcmp(command->name, instruction) != 0) /* search for the command */
			command = command->next;

		/* command isn't null because instruction isn't unknown instruction because first pass has successfuly performed */

		if (command->type == 'J' && strcmp(command->name, "stop") != 0)
		{
			int32_t bininstruct; /* the binary instruction to add to code image */
			int32_t reset; /* uusuage explained below */

			(void)fscanf(asmfile, "%s", instruction);
			if (*instruction != '$') /* if it's label instruction */
			{
				symbol* label; /* the symbol in command */
				label = find_sym_by_name(instruction); /* instruction holds the symbol name */
				if (label == NULL) /* if symbol doesn't exist */
				{
					printf("%s, line: %d, no definition was found for symbol '%s'.\n", filename, line, instruction);
					readline(asmfile);
					error = 1;
					secondpass(NULL);
					return;
				}
				if (label->isextern == 1)
					build_externinstruct(ic, label->name); /* add to external instructions list for externals file */

				memcpy(&bininstruct, code + ic, 4); /* copy original isntruction (from first pass) */
				bininstruct |= label->value; /* code label value to instruction */
				reset = 0x1FFFFFF; /* = 0b00000001111111...1 */
				bininstruct &= reset; /* address field in 'J' instructions is 25 bit width. the and operator with reset, makes sure to turn off 31-25 bits.
									  Note: label->value is int32_t! */
				bininstruct |= command->opcode << 26;
				memcpy(code + ic, &bininstruct, 4); /* assign to code image */
				readline(asmfile); /* finish reading the line */
			}
		}
		else if (strcmp(command->name, "beq") == 0 || strcmp(command->name, "bne") == 0 || strcmp(command->name, "blt") == 0 || strcmp(command->name, "bgt") == 0)
		{
			char* first, * second, * third; /* first, second, third arguments */
			symbol* label; /* label of the instruction */
			int16_t distance; /* label's value minus the instruction index in code image */
			first = calloc(MAX_LINE + 1, 1);
			second = calloc(MAX_LINE + 1, 1);
			third = calloc(MAX_LINE + 1, 1);
			readargs(asmfile, first, second, third, 1); /* assign arguments to variables */

			label = find_sym_by_name(third);
			if (label == NULL)
				printf("%s, line: %d, no definition was found for symbol '%s'.\n", filename, line, third);
			else if (label->isextern == 1)
				printf("%s, line: %d, symbol '%s' is external and therefore can't be "
					"used with this instruction.\n", filename, line, label->name); /* external symbols CAN'T be used in these instructions */
			if (label == NULL || label->isextern == 1) /* either one of them is an error */
			{
				error = 1;
				secondpass(NULL);
				return;
			}

			distance = label->value - ic;
			if (distance < INT16_MIN || distance > INT16_MAX) /* if jump distance is too far */
			{
				printf("%s, line: %d, jump distance is out of int16 range (%d till %d).\n",
					filename, line, INT16_MIN, INT16_MAX);
				error = 1;
				secondpass(NULL);
				return;
			}
			memcpy(code + ic, &distance, 2); /* copy to immed field (0-15 bits) in 'I' instructions */
		}
		else
			readline(asmfile); /* finish reading the line - the instruction should be ignored */
		
		ic += 4; /* each instruction takes 4 bytes */
	}
	secondpass(NULL); /* advance to next line */
}