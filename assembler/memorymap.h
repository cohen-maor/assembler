#ifndef MEMORYMAP_H
#define MEMORYMAP_H

extern char code[]; /* array to store code instructions */
extern char data[]; /* array to store data instructions */

extern int IC; /* code array indexer */
extern int DC; /* data array indexer */
extern int memoryoffset; /* same as memoffset macro */

extern const int MAX_LINE; /* max length of a line */

/* shifts the data array by n >= 0 elements right */
void shiftdatabuf(int n);

/* write object file of input file with the given name */
void writeobjfile(char* filename);

#endif
