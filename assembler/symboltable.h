#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include <stdlib.h>
#include <stdint.h>

typedef struct symbol_struct {
	struct symbol_struct* next; /* the next node (symbol) in the LL */
	char* name; /* label name */
	int32_t value; /* the value of the symbol */
	char iscode; /* code attribute */
	char isdata; /* data att. */
	char isentry; /* .entry att. */
	char isextern; /* .extern att. */
} symbol;

/* a pointer to a LL of all the symbols defined in the current file */
extern symbol* symboltable;

/* builds a new symbol with given attribute or assigns attributes to an existing symbol.
if 0 is returned then there was a redefinition attempt to the symbol with the given name.
if 2 - no action was performed since an external symbol can't be defined in the same input file.
if 3 - no action was performed since an external symbol can't be declared as entry in the same file.
if 1 - success. attributes were assigned / a new symbol was created.
*/
int addsymbol(char* name, int value, char iscode, char isdata, char isentry, char isextern);

/* add num argument to every symbol with data attribute */
void add_num_to_every_data_sym(int num);

/* delete all symbols from table and reset the attributes */
void reset_symbols_table();

/* turns on the entry attribute of the symbol with the given name.
	returns 1 if symbol was found, 0 if not. */
int turn_on_entry_att(char* name);

/* returns symbol* to the symbol with the given name.
	returns NULL if no such a symbol exists. */
symbol* find_sym_by_name(char* name);

/* returns the no. of symbols associated with entry attribute. */
int entries_quantity();

/* write the symbols with entry attribute to an .ent file with the given 'filename' (without 'filename' extension).
	if there aren't any at all, no file is created. */
void write_entries(char* filename);

#endif
