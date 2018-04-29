#include <stdio.h>
#include <stdlib.h>


#include "myUtility.h"
#include "myHelpers.h"
#include "myParser.h"
#include "myStarter.h"
#include "myFiles.h"

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
		printf("Usage: %s <FAT32 img>\n", argv[0]);
		return 0;
	}

	if (!toStart(argv[1], argv[0])) {
		return 0;
	}
	program = 1;
	while (program) {

		displayPrompt();
		if((progLine = readIn()) == NULL) {
			fprintf(stderr, "Bad read, please try again\n");
			continue;
		}
		progArgs = parseFunc(progLine);
		program = userCmd(progArgs);
		loopClean(progLine, progArgs);
	}
	globClean();

	return 0;
}
