#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include "instructions_table.h"

void outofmemory()
{
	printf("Out of memory. Consider closing open programs.");
	exit(-1); /* -1 to indicate an invalid process */
}

int islabel(char* str)
{
	if (isalpha(str[0]) && strlen(str) <= 31) /* if so, this is a valid label-NAME */
	{
		char* labelname; /* the name of the label */
		instruct* current; /* current symbol */

		labelname = (char*)calloc(32, 1);
		if (labelname != NULL)
			strncpy(labelname, str, strlen(str) - 1); /* copy the label name from str to the local variable labelname */
		else
			outofmemory();

		current = instructions_table;
		while (current != NULL)
		{
			if (strcmp(current->name, labelname) == 0) /* if symbol already exists in the table */
				return 0;
			current = current->next; /* advance pointer to the next node */
		}
		return 1; /* the symbol has a valid name and doesn't already exist */
	}
	return 0; /* the symbol has invalid label name */
}

int isdataline(char* str)
{
	if (strcmp(str, ".db") == 0)
		return 1; /* 1 because .db defines 1 byte */

	if (strcmp(str, ".dh") == 0)
		return 2; /* 1 because .dh defines 2 bytes */

	if (strcmp(str, ".dw") == 0)
		return 4; /* 1 because .db defines 4 bytes */

	if (strcmp(str, ".asciz") == 0)
		return 10; /* it could be almost any other number */

	return 0; /* not a data line */
}

int extern_or_entry(char* str)
{
	if (strcmp(".extern", str) == 0)
		return 1;
	else if (strcmp(".entry", str) == 0)
		return 2;
	return 0; /* neither .extern nor .entry */
}

void readline(FILE* asmfile)
{
	char c;
	while (((c = getc(asmfile)) != '\n') && (c != EOF)); /* read until EOF or new line character */
}

int readargs(FILE* asmfile, char* first, char* second, char* third, char isreadline)
{
	char c;
	char* pointers[3]; /* array to hold pointers to first, second, third arguments */
	int k; /* loop interator */
	int i; /* pointers indexer */

	pointers[0] = first;
	pointers[1] = second;
	pointers[2] = third;
	i = 0;

	for (k = 0; k < 4; k++, i = 0)
	{
		while (((c = getc(asmfile)) == ' ') || (c == '\t') || (c == ','));
		if ((c == EOF) || (c == '\n'))
			return k; /* finished reading the list. return the no. of arguments assigned */
		else if (k == 3)
		{
			/* there are more than 3 arguments. finish reading the line using readline(), if needed, and return 4
				(there are at least 4 arguments) */
			if (isreadline)
				readline(asmfile);
			return 4;
		}
		pointers[k][i++] = c; /* assign the first letter of this argument */
		while (((c = getc(asmfile)) != ' ') && (c != '\t') && (c != ',') && (c != EOF) && (c != '\n')) /* finish assigning the arg */
			pointers[k][i++] = c;
		if ((c == EOF) || (c == '\n'))
			return k + 1; /* now k + 1 is the no. of arguments scanned - since we scanned a new argument */
	}

	return -1; /* can't be reached  - just to suppress compiler warning */
}

int isreginrange(char* reg)
{
	return atoi(reg) <= 31 && atoi(reg) >= 0; /* llegal range: 0-31 */
}

int readarg(FILE* asmfile, char* str)
{
	char c;
	int i; /* str indexer */

	i = 0;

	while (((c = getc(asmfile)) == ' ') || (c == '\t') || (c == ',')); /* skip whitespaces, tabs and commas*/
	if ((c == EOF) || (c == '\n')) /* if no argument to scan */
		return 0;
	str[i++] = c; /* assign the first letter of the argument */
	while (((c = getc(asmfile)) != ' ') && (c != '\t') && (c != ',') && (c != EOF) && (c != '\n')) /* finish assigning the arg */
		str[i++] = c;
	if ((c == EOF) || (c == '\n'))
		ungetc(c, asmfile); /* unget c for next call */
	
	return 1; /* successfully scanned one argument */
}

int isnum(char* str)
{
	int i;
	for (i = 0; i < strlen(str); i++)
	{
		if (i == 0) /* first character can be a number, a minus sign, or plus sign */
		{
			if (!isdigit(str[i]) && str[i] != '-' && str[i] != '+')
				return 0;
		}
		else if (!isdigit(str[i])) /* else if current character isn't a digit */
			return 0;
	}
	return 1; /* the given argument is a number */
}