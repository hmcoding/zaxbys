#include <stdio.h>
#include <stdlib.h>

#include "setup.h"
#include "parse.h"
#include "execute.h"
#include "clean.h"


int main(int argc, char *argv[]) {
	char *cmd_line;
	char **cmd_args;
	int status;

	if (argc < 2) {
		printf("USAGE: %s <FAT32 img>", argv[0]);
		return 0;
	}

	setup(argv[2]);
	status = 1;
	while (status) {
		print_prompt();
		if((cmd_line = read_input()) == NULL) {
			fprintf(stderr, "Bad read, try again\n");
			continue;
		}
		cmd_args = parse(cmd_line);
		status = execute_cmd(cmd_args);
		clean_up_loop(cmd_line, cmd_args);
	}
	clean_up_globals();

	return 0;
}
