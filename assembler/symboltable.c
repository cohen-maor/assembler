#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "symboltable.h"
#include "memorymap.h"

symbol* symboltable = NULL;
static symbol* current = NULL; /* current node in LL - inner use */
static int symbolscount = 0; /* for inner use - to keep track of the symbols quantity */

int addsymbol(char* name, int value, char iscode, char isdata, char isentry, char isextern)
{
	symbol* curr; /* current symbol */
	int i; /* no. of symbols scanned */
	curr = symboltable;
	i = 0;

	while (i < symbolscount) /* while there are more symbols to iterate over */
	{
		if ((strcmp(curr->name, name) == 0)) /* current symbol and given name argument are the same */
		{
			if (isdata || iscode) /* if it's a label definition */
			{
				if (curr->isdata || curr->iscode) /* if it's a redefinition */
					return 0;
				if (curr->isextern) /* if it's a definition to an external symbol */
					return 2;
				
				/* legal label. assign attributes */
				curr->iscode = iscode;
				curr->isdata = isdata;
				curr->value = value;
			}
			else if (isextern) /* if it's a .extern instruction */
			{
				if (curr->isdata || curr->iscode) /* if it's a .extern instruction to a defined symbol in current file */
					return 2;
				if (curr->isentry) /* if .extern and .entry attributes */
					return 3;
				curr->isentry = 1;
			}
			return 1; /* success */
		}
		curr = curr->next;
		i++;
	}
	if (symboltable == NULL)
	{
		/* this is the FIRST symbol to add */
		symboltable = malloc(sizeof(symbol));
		if (symboltable == NULL)
		{
			outofmemory();
			exit(-1);
		}
		symboltable->name = calloc(32, 1);
		if (symboltable->name == NULL)
		{
			outofmemory();
			exit(-1);
		}
		strcpy(symboltable->name, name);
		symboltable->value = value;
		symboltable->iscode = iscode;
		symboltable->isdata = isdata;
		symboltable->isentry = isentry;
		symboltable->isextern = isextern;
		current = symboltable;
		symbolscount++;

		return 1;
	}
	else
	{
		/* the symbol wasn't found and therefore should be added -
			there are different implemnetations whether it's the first symbol or not the first. */
		current->next = malloc(sizeof(symbol));
		if (current->next == NULL)
		{
			outofmemory();
			exit(-1);
		}
		current->next->name = calloc(32, 1);
		if (current->next->name == NULL)
		{
			outofmemory();
			exit(-1);
		}
		strcpy(current->next->name, name);
		current->next->value = value;
		current->next->iscode = iscode;
		current->next->isdata = isdata;
		current->next->isentry = isentry;
		current->next->isextern = isextern;
		current = current->next;
		symbolscount++;

		return 1; /* success */
	}
}

void add_num_to_every_data_sym(int num)
{
	int i; /* no. of symbols scanned */
	symbol* curr; /* current symbol */
	curr = symboltable; /* set to head */
	for (i = 0; i < symbolscount; i++)
	{
		if (curr->isdata) /* the symbol has the data attribute */
			curr->value += num;
		curr = curr->next; /* advance to next symbol */
	}
}

void reset_symbols_table()
{
	symbol* curr; /* current symbol */
	symbol* p; /* the next symbol after curr */
	int i; /* no. of symbols scanned */

	p = symboltable; /* initialize to head */
	for (i = 0; i < symbolscount; i++)
	{
		curr = p;
		p = p->next;
		free(curr);
	}
	current = NULL;
	symbolscount = 0;
	symboltable = 0;
}

int turn_on_entry_att(char* name)
{
	int i; /* no. of symbols scanned */
	symbol* curr; /* current sym */
	curr = symboltable; /* set to the first node */
	
	for (i = 0; i < symbolscount; i++)
	{
		if (strcmp(curr->name, name) == 0) /* if current symbol's name is the given name */
		{
			if (curr->isextern) /* if symbol has .extern attribute */
				return 3;
			curr->isentry = 1; /* turn on entry attribute of this symbol */
			return 1; /* success */
		}
		curr = curr->next; /* advance to next symbol */
	}

	return 0; /* failed, symbol wasn't found */
}

symbol* find_sym_by_name(char* name)
{
	int i;
	symbol* curr; /* current symbol */
	curr = symboltable;

	for (i = 0; i < symbolscount; i++)
	{
		if (strcmp(curr->name, name) == 0) /* if current symbol name is as the name given */
			return curr; /* found the symbol, return it */
		curr = curr->next;
	}
	return NULL; /* symbol wasn't found */
}

int entries_quantity()
{
	symbol* curr;
	int i;
	int quantity; /* no. of symbols with entry attribute */

	curr = symboltable;
	quantity = 0;
	i = 0;
	while (i < symbolscount)
	{
		if (curr->isentry) /* if current symbol has entry attribute */
			quantity++;
		curr = curr->next;
		i++;
	}

	return quantity;
}

void write_entries(char* filename)
{
	char* fnameent; /* the entries file name */
	FILE* fent; /* pointer to entries file */
	int i; /* symbols counter */
	int entries; /* no of entries exist */
	symbol* curr; /* current symbol */
	char* format; /* format string */

	if (entries_quantity() == 0)
		return; /* no file needs to be created. */
	fnameent = calloc(strlen(filename) + 5, 1); /* fnameent is longer than filename by 1. +5 for safety */
	if (fnameent == NULL)
		outofmemory();
	strcpy(fnameent, filename);

	/* change from .as extension to .ent */
	fnameent[strlen(fnameent) - 2] = 'e';
	fnameent[strlen(fnameent) - 1] = 'n';
	*(fnameent + strlen(fnameent)) = 't'; /* fnameent + strlen(fnameent) is allocated */

	fent = fopen(fnameent, "w"); /* create entries file with writing permission */
	curr = symboltable; /* set to head */
	i = 0;
	
	format = (char*)calloc(MAX_LINE, 1);
	if ((entries = entries_quantity()) > 0)
	{
		while (i < symbolscount) /* while there are more symbols to process */
		{
			if (curr->isentry) /* if symbol has entry attribute */
			{
				if (entries > 1) /* if it's not the last one */
					sprintf(format, "%s %04d\n", curr->name, curr->value);
				else /* last one */
					sprintf(format, "%s %04d", curr->name, curr->value);
				
				fputs(format, fent); /* write to entries file */
				entries--;
			}
			curr = curr->next; /* advance to next symbol */
			i++;
		}
	}
	fclose(fent);
}
