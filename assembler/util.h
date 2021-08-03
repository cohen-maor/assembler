#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>

/* returns 1 if  str is a valid label name and if doesn't exist already, 0 otherwise. */
int islabel(char* str);

/* returns 1 if str is .db instruction, 2 if .dh instruction, 4 if .dw instruction
	and 10 if .asciz instruction. If not any of the above, 0 is returned */
int isdataline(char* str);

/* puts a message to stdin that indicates need in memory  - and exits environment with -1 code */
void outofmemory();

/* returns 1 if str is .extern instruction, 2 if .entry instruction and 0 otherwise */
int extern_or_entry(char* str);

/* reads from the current cursor position of asmfile until the end of the current line */
void readline(FILE* asmfile);

/* reads argument from current cursor position of asmfile, which are expected to be separated by a comma.
	if there is only 1 arg, it'll be assigned to first, if only 2, then first, second will be assigned respectively,
		if only 3 then first, second, third are assigned respectively. Returns no. of args scanned, if there are more than 3,
			it returns 4. if there is 0 args, then 0. */
int readargs(FILE* asmfile, char* first, char* second, char* third, char isreadline);

/* returns 1 if given register no. is in range, 0 otherwise. assumes valid register name (starting with no.
	right after the $ sign) */
int isreginrange(char* reg);

/* reads one argument from given file pointer, and stores it in given char pointer.
	returns 1 on success of reading an argument, 0 otherwise. */
int readarg(FILE* asmfile, char* str);

/* returns 1 if str is a numerical string, 0 if not.
	+(digits...) or -(digits...) is the legal format. */
int isnum(char* str);

#endif
