#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <limits.h>

#include "instructions_table.h"
#include "util.h"
#include "symboltable.h"
#include "memorymap.h"
#include "externinstruct.h"

static char* instruction = NULL; /* the current instruction / label */
static int labelflag; /* if it's 1 then there is a label definition in the current line, 0 if not. */
static int currentline = 0; /* current line counter. incremented immediately in the start of the function. */
static char* labelname = NULL; /* the label's (if exists) name */
static char ch; /* simple variable for holding characters */
static int iserror = 0; /* if 1 then secondpass() isn't called, if 0 then second pass will be processed. */
static FILE* asmfile; /* the pointer to the file for processing */
static char* filename; /* the name of the file to open */

extern void secondpass(char* fname);

/* the first pass function, implemented using recursion */
void firstpass(char* fname)
{
	if (fname != NULL) /* if first call of this input file */
	{
		asmfile = fopen(fname, "r"); /* open the file to process using reading access privilege */
		if (asmfile == NULL) /* if couldn't open the file */
		{
			printf("Incorrect file name / no permission to read the file: %s\n", fname);
			return;
		}
		filename = fname; /* fname is a local variable, but will remain in the stack because of recursion */
	}
	if (instruction == NULL)
	{
		instruction = (char*)calloc(MAX_LINE + 1, sizeof(char)); /* + 1 for '\0' character */
		alloc_instructions_table(); /* initialize the instructions table */
	}
	
	currentline++;
	labelflag = 0; /* by default - no label definition yet */
	labelname = NULL; /* memory will be allocated for labelname if label definition will be found */

	while ((ch = getc(asmfile)) == '\t' || ch == ' '); /* skip spaces and tabs in the beggining of the line */
	if (ch == '\n') /* if empty line */
	{
		firstpass(NULL); /* read next line */
		return;
	}
	else if (ch == EOF) /* if the program finished to read the file */
	{
		if (!iserror) /* if there weren't any errors */
		{
			add_num_to_every_data_sym(IC); /* IC (ICF) will be added to each symbol's value with data attribute */

			shiftdatabuf(IC); /* right shift data buffer by ICF */

			secondpass(filename); /* start second pass */
		}

		currentline = 0; /* reset line counting */
		iserror = 0; /* reset error flag */

		reset_symbols_table(); /* remove symbols for possible next file */
		reset_externinstruct(); /* remove external instructions list */
		
		/* reset image indexers */
		IC = memoryoffset;
		DC = 0;

		return;
	}
	else /* ch is a character of code to process - unget it */
		ungetc(ch, asmfile);

	(void)fscanf(asmfile, "%s", instruction); /* read first word of line */

	if (instruction[0] == ';') /* if comment line */
	{
		readline(asmfile); /* ignore the line by reading it */

		firstpass(NULL);
		return;
	}
	else if (instruction[strlen(instruction) - 1] == ':') /* if label definition */
	{
		if ((labelflag = islabel(instruction))) /* if label name is legal */
		{
			labelname = calloc(MAX_LINE, sizeof(char)); /* sizeof(char) is always 1 */
			if (labelname == NULL)
				outofmemory(); /* msg to user and exits environment with -1 code */

			strncpy(labelname, instruction, strlen(instruction) - 1); /* instruction holds the label name - copy it */
			(void)fscanf(asmfile, "%s", instruction); /* advance to next keyword */
		}
		else
		{
			instruction[strlen(instruction) - 1] = '\0'; /* remove ':' character */
			printf("%s, line: %d, illegal label name: %s.\n", filename, currentline, instruction);
			iserror = 1; /* turn on error flag */
			readline(asmfile); /* finish reading this line */
			firstpass(NULL); /* keep looking for errors */
			return;
		}
	}
	if (isdataline(instruction)) /* if .db or .dh or .dw or .asciz instruction */
	{
		int datainstructype; /* data allocation instruction by no. */
		datainstructype = isdataline(instruction); /* datainstructype will also define no. of bytes to allocate */
		if (labelflag) /* if there is a label definition */
		{
			int symbolstatus; /* the return value of addsymbol method */

			if (labelname == NULL) /* out of memory error */
				outofmemory(); /* msg to user */

			symbolstatus = addsymbol(labelname, DC, 0, 1, 0, 0); /* add DC as value and data attribute to the symbol and store status */
			if (symbolstatus == 0) /* if redefinition of symbol */
			{
				printf("%s, line: %d, '%s' symbol is already defined.\n", filename, currentline, labelname);
				iserror = 1;
				readline(asmfile);
				firstpass(NULL);
				return;
			}
			else if (symbolstatus == 2) /* if symbol is already defined as external */
			{
				printf("%s, line: %d, symbol can't be defined and declared as external together in the same input file.\n", filename, currentline);
				iserror = 1;
				readline(asmfile);
				firstpass(NULL);
				return;
			}
		}
		if (strcmp(instruction, ".asciz") == 0) /* .asciz instruction - store characters */
		{
			char* c; /* character pointer to process string */

			(void)fscanf(asmfile, "%s", instruction);
			if (*instruction != '"') /* if incorrect type of operand given */
			{
				printf("%s, line: %d, .asciz: incorrect operand type (expected a string with quotes surrounding it).\n", filename, currentline);
				iserror = 1;
				readline(asmfile);
				firstpass(NULL);
				return;
			}
			c = instruction + 1; /* skips the first '"' character */
			while (*c != '"') /* while c didn't reach the end of the string */
			{
				data[DC++] = *c++; /* add the character to data buffer, and advance DC, c pointers by 1 */
				if (c == instruction + (strlen(instruction) - 1)) /* if c reached the last character of this word */
				{
					if (*c == '"') /* if c reached end of string */
						break;
					data[DC++] = *c; /* assign last character of the word */
					data[DC++] = ' '; /* assign whitespace */
					(void)fscanf(asmfile, "%s", instruction); /* read next word */
					c = instruction; /* point c to the first character of the new word */
				}
			}
			data[DC++] = '\0'; /* null terminating the string */
			readline(asmfile);
		}
		else
		{
			int64_t num; /* current number in list */
			int64_t atoll(char*); /* string to int64 */
			char* arg; /* current argument in list */
			int numcounter; /* quantity of numbers in list */

			num = 0;
			numcounter = 0;

			arg = (char*)calloc(MAX_LINE, 1);
			if (arg == NULL)
				outofmemory();

			while (readarg(asmfile, arg) == 1)
			{
				if (!isnum(arg)) /* if arg isn't a number (in string) */
				{
					printf("%s, line: %d, %s instruction takes whole numbers only (illegal argument type).\n", filename, currentline, instruction);
					iserror = 1;
					readline(asmfile);
					firstpass(NULL);
					return;
				}
				num = atoll(arg); /* arg has been checked to be a number - safe assignment */
				if (datainstructype == 1) /* .db */
				{
					char numinsize; /* the number that got scanned, in the correct data type */

					if (num < CHAR_MIN || num > CHAR_MAX)
					{
						printf("%s, line: %d, assignment is out of range; .db range: %d till %d.\n",
							filename, currentline, CHAR_MIN, CHAR_MAX);
						iserror = 1;
						readline(asmfile);
						firstpass(NULL);
						return;
					}
					numinsize = (char)num; /* after range checking, this is a safe narrowing type casting */
					data[DC++] = numinsize;
				}
				else if (datainstructype == 2) /* .dh */
				{
					int16_t numinsize; /* int16_t is signed */

					if (num < INT16_MIN || num > INT16_MAX)
					{
						printf("%s, line: %d, assignment is out of range; .dh range: %d till %d.\n",
							filename, currentline, INT16_MIN, INT16_MAX);
						iserror = 1;
						readline(asmfile);
						firstpass(NULL);
						return;
					}
					numinsize = (int16_t)num;
					memcpy(data + DC, &numinsize, datainstructype); /* copies 2 bytes starting from numinsize address, to (starting from) data + DC */
					DC += 2; /* += 2 since 2 bytes were assigned to data buffer */
				}
				else if (datainstructype == 4) /* .dw */
				{
					int32_t numinsize; /* int32_t is signed */
					
					if (num < INT32_MIN || num > INT32_MAX)
					{
						printf("%s, line: %d, assignment is out of range; .dw range: %d till %d.\n",
							filename, currentline, INT32_MIN, INT32_MAX);
						iserror = 1;
						readline(asmfile);
						firstpass(NULL);
						return;
					}
					numinsize = (int32_t)num;
					memcpy(data + DC, &numinsize, datainstructype);
					DC += 4;
				}
				numcounter++;
				memset(arg, '\0', strlen(arg)); /* reset arg string for next number */
			}
			if (numcounter == 0) /* if the instruction wasn't given any numbers */
			{
				printf("%s, line: %d, empty %s instruction.\n", filename, currentline, instruction);
				iserror = 1;
			}
		}
		firstpass(NULL);
		return;
	}
	else if (extern_or_entry(instruction)) /* if .extern or .entry instruction */
	{
		int ext_or_ent; /* indicates if it's .extern instruction or .entry instruction */
		ext_or_ent = extern_or_entry(instruction); /* 1 means .extern, 2 means .entry */
		if (ext_or_ent == 2) /* if .entry */
		{
			/* will be handled in second pass */
			if (labelflag)
				printf("%s, line: %d, WARNING: ignored label definition in .entry line.\n", filename, currentline);
			readline(asmfile);
			firstpass(NULL);
			return;
		}
		/* .extern */
		(void)fscanf(asmfile, "%s", instruction);
		if (!islabel(instruction)) /* if illegal label name */
		{
			iserror = 1;
			printf("%s, line: %d, illegal label name: %s.\n", filename, currentline, instruction);
		}
		else
		{
			int symbolstatus; /* the return value of addsymbol method */

			if (labelflag)
				printf("%s, line: %d, WARNING: ignored label definition in .extern line.\n", filename, currentline);
			
			symbolstatus = addsymbol(instruction, 0, 0, 0, 0, 1); /* add the symbol (or add .extern att. if already exists) */
			if (symbolstatus == 2) /* if symbol is defined in this input file  */
			{
				printf("%s, line: %d, symbol can't be defined and declared as external together in the same input file.\n", filename, currentline);
				iserror = 1;
			}
			else if (symbolstatus == 3) /* if symbol has already .entry attribute */
			{
				printf("%s, line: %d, symbol can't be declared as external and as an entry together in the same input file.\n", filename, currentline);
				iserror = 1;
			}
		}
		readline(asmfile);
		firstpass(NULL);
		return;
	}
	else /* instruction line */
	{
		char *first, *second, *third;/* first, second, third argument in list (if exist) */
		int argscounter, paramsnumber; /* argscounter - actual no. of parameters given */
		instruct* command; /* the instruction */
		if (labelflag) /* if there is a flag definition */
		{
			int symbolstatus;
			symbolstatus = addsymbol(labelname, IC, 1, 0, 0, 0); /* add IC as value and code attribute to the symbol and store status */
			if (symbolstatus == 0) /* if redefinition of symbol */
			{
				printf("%s, line: %d, '%s' symbol is already defined.\n", filename, currentline, labelname);
				iserror = 1;
				readline(asmfile);
				firstpass(NULL);
				return;
			}
			else if (symbolstatus == 2) /* if symbol is already defined as external */
			{
				printf("%s, line: %d, symbol can't be defined and declared as external together in the same input file.\n", filename, currentline);
				iserror = 1;
				readline(asmfile);
				firstpass(NULL);
				return;
			}
		}
		command = instructions_table; /* set command to first command */
		if (command == NULL) /* ensure initialization of instructions table */
		{
			printf("Instructions table isn't intialized.\n");
			exit(-1); /* exists environment with code -1 */
		}
		while ((command != NULL) && (strcmp(command->name, instruction) != 0)) /* search for the instruction in the instructions table */
			command = command->next;
		if (command == NULL) /* if command wasn't found */
		{
			printf("%s, line: %d, unknown %s instruction.\n", filename, currentline, instruction);
			iserror = 1;
			readline(asmfile);
			firstpass(NULL);
			return;
		}
		if (command->type == 'J') /* 'J' type of instruction */
		{
			if (strcmp(command->name, "stop") == 0)
				paramsnumber = 0; /* stop instruction doesn't take any arguments */
			else
				paramsnumber = 1; /* rather than 'stop', 'J' instructions take only 1 argument */
		}
		else if (command->type == 'R')
		{
			if (strcmp(command->name, "move") == 0 || strcmp(command->name, "mvhi") == 0 || strcmp(command->name, "mvlo") == 0)
				paramsnumber = 2; /* these commands take 2 arguments */
			else
				paramsnumber = 3; /* otherwise, 'R' commands take 3 args */
		}
		else
			paramsnumber = 3; /* 'I' commands always take 3 arguments */

		first = calloc(MAX_LINE, 1);
		second = calloc(MAX_LINE, 1);
		third = calloc(MAX_LINE, 1);
		if (first == NULL || second == NULL || third == NULL)
			outofmemory();

		argscounter = readargs(asmfile, first, second, third, 1); /* readargs also finishes reading the current line */
		if (argscounter > paramsnumber) /* if extraneous argument */
		{
			printf("%s, line: %d, extraneous operand.\n", filename, currentline);
			iserror = 1;
			firstpass(NULL);
			return;
		}
		else if (argscounter < paramsnumber) /* if an argument is missing */
		{
			printf("%s, line: %d, missing operand.\n", filename, currentline);
			iserror = 1;
			firstpass(NULL);
			return;
		}
		else /* no. of arguments matched no. of parameters */
		{
			int32_t binaryinstruct; /* int32_t and not long because int32_t is promised to be 4 bytes in size */
			binaryinstruct = 0;
			if (command->type == 'R')
			{
				if (strcmp(command->name, "move") == 0 || strcmp(command->name, "mvhi") == 0 || strcmp(command->name, "mvlo") == 0)
				{
					if (*first++ != '$' || *second++ != '$') /* if first or second arguments aren't registers */
					{
						printf("%s, line: %d, illegal register name.\n", filename, currentline);
						iserror = 1;
						firstpass(NULL);
						return;
					}
					if (!isreginrange(first) || !isreginrange(second)) /* if one of the registers no. is out of range */
					{
						printf("%s, line: %d, register no. out of range (only 0-31 is legal).\n", filename, currentline);
						iserror = 1;
						firstpass(NULL);
						return;
					}
					/* the instruction building - for better understanding, have a look at the project instructions */
					binaryinstruct |= (command->funct) << 6;
					binaryinstruct |= atoi(second) << 11; /* atoi: string to int */
					binaryinstruct |= atoi(first) << 21; 
					binaryinstruct |= (command->opcode) << 26;
				}
				else
				{
					/* now there are 3 registers to process */
					if (*first++ != '$' || *second++ != '$' || *third++ != '$')
					{
						printf("%s, line: %d, illegal register name.\n", filename, currentline);
						iserror = 1;
						firstpass(NULL);
						return;
					}
					/* if any of the three is out of range */
					if (!isreginrange(first) || !isreginrange(second) || !isreginrange(third))
					{
						printf("%s, line: %d, register no. out of range (only 0-31 is legal).\n", filename, currentline);
						iserror = 1;
						firstpass(NULL);
						return;
					}
					/* build the instruction */
					binaryinstruct |= (command->funct) << 6;
					binaryinstruct |= atoi(third) << 11;
					binaryinstruct |= atoi(second) << 16;
					binaryinstruct |= atoi(first) << 21;
					binaryinstruct |= (command->opcode) << 26;
				}
			}
			else if (command->type == 'I')
			{
				if (strcmp(command->name, "beq") == 0 || strcmp(command->name, "bne") == 0 ||
					strcmp(command->name, "blt") == 0 || strcmp(command->name, "bgt") == 0)
				{
					/* these instructions require a different treatment than other 'I' instructions */
					if (*first++ != '$' || *second++ != '$')
					{
						printf("%s, line: %d, illegal register name.\n", filename, currentline);
						iserror = 1;
						firstpass(NULL);
						return;
					}
					if (!isreginrange(first) || !isreginrange(second))
					{
						printf("%s, line: %d, register no. out of range (only 0-31 is legal).\n", filename, currentline);
						iserror = 1;
						firstpass(NULL);
						return;
					}
					/* fill registers - label will be handled in second pass */
					binaryinstruct |= atoi(second) << 16;
					binaryinstruct |= atoi(first) << 21;
				}
				else
				{
					if (*first++ != '$' || *third++ != '$')
					{
						printf("%s, line: %d, illegal register name.\n", filename, currentline);
						iserror = 1;
						firstpass(NULL);
						return;
					}
					if (!isreginrange(first) ||  !isreginrange(third))
					{
						printf("%s, line: %d, register no. out of range (only 0-31 is legal).\n", filename, currentline);
						iserror = 1;
						firstpass(NULL);
						return;
					}
					if (atoi(second) < INT16_MIN || atoi(second) > INT16_MAX) /* second argument is a constant numerical value */
					{
						printf("%s, line: %d, immed argument is out of int16 range.\n", filename, currentline);
						iserror = 1;
						firstpass(NULL);
						return;
					}
					binaryinstruct |= atoi(second);
					binaryinstruct &= 0xffff; /* for machines with int data type in size 32 bits */
					/* registers */
					binaryinstruct |= atoi(third) << 16;
					binaryinstruct |= atoi(first) << 21;
				}
				binaryinstruct |= (command->opcode) << 26;
			}
			else /* command->type is 'J'*/
			{
				if (strcmp(command->name, "jmp") == 0)
				{
					if (*first++ == '$') /* if first argument is register and not label */
					{
						if (!isreginrange(first))
						{
							printf("%s, line: %d, register no. out of range (only 0-31 is legal).\n", filename, currentline);
							iserror = 1;
							firstpass(NULL); /* keep searching for errors */
							return;
						}
						binaryinstruct |= atoi(first);
						binaryinstruct |= 1 << 25; /* turn on reg flag */
					}
				}
				binaryinstruct |= command->opcode << 26; /* if needed, the rest of instruction's building will be handled in second pass */
			}
			memcpy(code + IC, &binaryinstruct, 4); /* copy the instruction to code image */

			IC += 4; /* += 4 because every instruction takes 4 bytes of memory in code image */
			firstpass(NULL);
			return;
		}
	}
}
