#include <stdio.h>
#include <stdlib.h>


#include "setup.h"
#include "parse.h"
#include "execute.h"
#include "clean.h"
#include "tools.h"
#include "file_types.h"


int endianVar;
FILE *fatImage;
struct fatData imageData;
unsigned int dataSec;
unsigned int rootSec;
char *thisDir;
unsigned int thisDirSec;



int main(int argc, char *argv[]) {
	char *progLine;
	char **progArgs;
	int program;

	if (argc < 2) {
		printf("USAGE: %s <FAT32 img>\n", argv[0]);
		return 0;
	}

	if (!setup(argv[1], argv[0])) {
		return 0;
	}
	program = 1;
	while (program) {

		displayPrompt();
		if((progLine = readIn()) == NULL) {
			fprintf(stderr, "Bad read, try again\n");
			continue;
		}
		progArgs = parse(progLine);
		program = userCmd(progArgs);
		loopClean(progLine, progArgs);
	}
	globClean();

	return 0;
}
