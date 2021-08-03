#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memorymap.h"
#include "externinstruct.h"
#include "util.h"

externinstruct* externinstruct_head = NULL;
externinstruct* externinstruct_current = NULL;
static int externinstruct_quantity = 0;

void build_externinstruct(int32_t ic, char* externsym)
{
	if (externinstruct_head == NULL) /* if it's the first node to initialize */
	{
		externinstruct_head = (externinstruct*)malloc(sizeof(externinstruct));
		if (externinstruct_head == NULL) /* if no memory left */
			outofmemory();

		externinstruct_head->ic = ic;

		externinstruct_head->externsym = (char*)calloc(MAX_LINE + 1, 1);
		if (externinstruct_head->externsym == NULL)
			outofmemory();
		strcpy(externinstruct_head->externsym, externsym); /* copy externsym string to externinstruct_head->externsym */

		externinstruct_current = externinstruct_head;

		externinstruct_quantity++;
	}
	else
	{
		externinstruct_current->next = (externinstruct*)malloc(sizeof(externinstruct));
		if (externinstruct_current->next == NULL)
			outofmemory();

		externinstruct_current->next->ic = ic;

		externinstruct_current->next->externsym = (char*)calloc(MAX_LINE + 1, 1);
		if (externinstruct_current->next->externsym == NULL)
			outofmemory();
		strcpy(externinstruct_current->next->externsym, externsym);

		externinstruct_current = externinstruct_current->next; /* advance current to the allocated node */

		externinstruct_quantity++;
	}
}

void write_externinstruct(char* filename)
{
	char* fnameext; /* file name of externals file */
	FILE* fext; /* pointer to external file */
	externinstruct* curr; /* current external instruction node */
	int i; /* no. of external instructions node scanned */
	char* format; /* format string */
	
	if (externinstruct_quantity == 0)
		return; /* no need to create file */
	fnameext = calloc(strlen(filename) + 5, 1); /* external file name is longer by 1 from filename, +5 for safety */
	if (fnameext == NULL)
		outofmemory();

	strcpy(fnameext, filename); /* copy filename string to fnameext */

	/* change extension from .as to .ext */
	fnameext[strlen(fnameext) - 2] = 'e';
	fnameext[strlen(fnameext) - 1] = 'x';
	*(fnameext + strlen(fnameext)) = 't'; /* fnameext + strlen(fnameext) is allocated */

	fext = fopen(fnameext, "w"); /* create file */
	curr = externinstruct_head; /* set to head */
	i = 0;
	format = (char*)calloc(MAX_LINE, 1);
	if (format == NULL)
		outofmemory();

	while (i < externinstruct_quantity)
	{
		if (i + 1 == externinstruct_quantity) /* if this is the last external instruction node */
			sprintf(format, "%s %04d", curr->externsym, curr->ic);
		else /* need a new line character */
			sprintf(format, "%s %04d\n", curr->externsym, curr->ic);
		fputs(format, fext);

		curr = curr->next;
		i++;
	}
	fclose(fext);
}

void reset_externinstruct(void)
{
	externinstruct* curr; /* current external instruction node */
	externinstruct* p; /* the next external instruction after curr */
	int i; /* no. of external instructions scanned */

	p = externinstruct_head; /* initialize to head */
	for (i = 0; i < externinstruct_quantity; i++)
	{
		curr = p;
		p = p->next;
		free(curr);
	}
	/* reset attributes */
	externinstruct_current = NULL;
	externinstruct_quantity = 0;
	externinstruct_head = NULL;
}