#ifndef EXTERNINSTRUCT_H /* header guards */
#define EXTERNINSTRUCT_H

#include <stdio.h>
#include <stdint.h>

typedef struct extern_instruct {
	struct extern_instruct* next; /* the next node (symbol) in the LL */
	int32_t ic;
	char* externsym;
} externinstruct;

/* builds an external instruction node with given ic, externsym values. advances current to the builded node. */
void build_externinstruct(int32_t ic, char* externsym);

/* writes the externals file (with the given name with extern .ext). if there aren't any externals symbols at all, the file isn't created. */
void write_externinstruct(char* filename);

/* free all external instructions nodes */
void reset_externinstruct(void);

#endif
