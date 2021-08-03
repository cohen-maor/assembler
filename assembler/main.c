#include <stdio.h>

int main(int argc, char* argv[])
{
	extern int firstpass(char* filename);
	
	while (argc-- > 1) /* until there are no more files to process. */
		firstpass(argv[argc]); /* secondpass() will be called (if there weren't any errors) within firstpass() */

	return 1;
}