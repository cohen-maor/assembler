#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "util.h"

#define maxinput 20000 /* max input size (for each data/code array) */
#define memoffset 100 /* memory offset for code array */

char code[maxinput] = ""; /* initializes all array elements to '\0' (convenience) */
char data[maxinput] = "";

int IC = memoffset;
int DC = 0;
int memoryoffset = memoffset;

const int MAX_LINE = 80; /* maximum length of a line */

void shiftdatabuf(int n)
{
	int i;
	for (i = DC - 1; i > -1; i--)
	{
		data[i + n] = data[i]; /* the shift */
		data[i] = '\0'; /* isn't necessary - only for convenience */
	}
}

void writeobjfile(char* filename)
{
	char* fnameobj; /* full file name of object file */
	FILE* fobj; /* the pointer to object file */
	char* format; /* the current format string to append object file */
	int ic; /* code image indexer */
	int dc; /* data image indexer */

	fnameobj = (char*)calloc(strlen(filename) + 1, 1);
	if (fnameobj == NULL)
		outofmemory();
	strcpy(fnameobj, filename); /* copy filename to fnameobj */

	/* set file extension from .as to .ob */
	fnameobj[strlen(fnameobj) - 2] = 'o';
	fnameobj[strlen(fnameobj) - 1] = 'b';

	fobj = fopen(fnameobj, "w"); /* create the object file with writing permission */
	if (fobj == NULL)
	{
		printf("There was an error with creating object file for %s input file. Check OS permissions.", filename);
		exit(-1);
	}
	format = (char*)calloc(MAX_LINE, 1);
	if (format == NULL)
		outofmemory();
	sprintf(format, "     %d %d\n", IC - memoryoffset, DC); /* IC - memoryoffset is code image length, and DC is data image length */
	fputs(format, fobj);

	/* write code image */
	for (ic = memoryoffset; ic < IC; ic += 4) /* += 4 because each instruction is 4 bytes long */
	{
		sprintf(format, "%04d %02X %02X %02X %02X\n", ic, code[ic] & 0xff,
			code[ic + 1] & 0xff, code[ic + 2] & 0xff, code[ic + 3] & 0xff); /* & 0xff to handle signed elements */
		fputs(format, fobj);
	}

	/* write data image */
	for (dc = IC; dc <= IC + DC - 4; dc += 4) /* dc = IC because of data buffer shift */
	{
		sprintf(format, "%04d %02X %02X %02X %02X\n", dc, data[dc] & 0xff,
			data[dc + 1] & 0xff, data[dc + 2] & 0xff, data[dc + 3] & 0xff);
		fputs(format, fobj);
	}

	*format = '\0'; /* reset format string */
	if (dc == IC + DC - 3) /* if 3 elements left to write */
		sprintf(format, "%04d %02X %02X %02X", dc, data[dc] & 0xff,
			data[dc + 1] & 0xff, data[dc + 2] & 0xff);
	else if (dc == IC + DC - 2) /* if 2 elements left to write */
	{
		sprintf(format, "%04d %02X %02X", dc, data[dc] & 0xff,
			data[dc + 1] & 0xff);
	}
	else if (dc == IC + DC - 1) /* if 1 element left to write */
	{
		sprintf(format, "%04d %02X", dc, data[dc] & 0xff);
	}
	fputs(format, fobj);
	fclose(fobj);
}