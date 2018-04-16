#include <stdio.h>
#include <stdlib.h>

#include "setup.h"
#include "parse.h"
#include "execute.h"
#include "clean.h"
#include "tools.h"
#include "file_types.h"

int endianness;
FILE *fat32_img;
struct fat32_info img_info;
unsigned int first_data_sec;
unsigned int first_root_sec;
char *current_directory;
unsigned int cur_dir_sec;



int main(int argc, char *argv[]) {
	char *cmd_line;
	char **cmd_args;
	int status;

	if (argc < 2) {
		printf("USAGE: %s <FAT32 img>\n", argv[0]);
		return 0;
	}

	if (!setup(argv[1], argv[0])) {
		return 0;
	}
	status = 1;
	while (status) {

		displayPrompt();
		if((cmd_line = readIn()) == NULL) {
			fprintf(stderr, "Bad read, try again\n");
			continue;
		}
		cmd_args = parse(cmd_line);
		status = userCmd(cmd_args);
		loopClean(cmd_line, cmd_args);
	}
	globClean();

	return 0;
}
