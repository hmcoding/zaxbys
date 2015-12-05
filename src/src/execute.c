#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>

#include "execute.h"
#include "file_commands.h"
#include "directory_commands.h"

/* execute_cmd
 *
 */
int execute_cmd(char **cmd_args) {
	if (cmd_args[0] == NULL) {
		// do nothing
	} else if (strcmp(cmd_args[0], "open") == 0) {
		my_open(cmd_args);
	} else if (strcmp(cmd_args[0], "close") == 0) {
		my_close(cmd_args);
	} else if (strcmp(cmd_args[0], "create") == 0) {
		my_create(cmd_args);
	} else if (strcmp(cmd_args[0], "rm") == 0) {
		my_rm(cmd_args);
	} else if (strcmp(cmd_args[0], "size") == 0) {
		my_size(cmd_args);
	} else if (strcmp(cmd_args[0], "cd") == 0) {
		my_cd(cmd_args);
	} else if (strcmp(cmd_args[0], "ls") == 0) {
		my_ls(cmd_args);
	} else if (strcmp(cmd_args[0], "mkdir") == 0) {
		my_mkdir(cmd_args);
	} else if (strcmp(cmd_args[0], "rmdir") == 0) {
		my_rmdir(cmd_args);
	} else if (strcmp(cmd_args[0], "read") == 0) {
		my_read(cmd_args);
	} else if (strcmp(cmd_args[0], "write") == 0) {
		my_write(cmd_args);
	} else if (strcmp(cmd_args[0], "exit") == 0) {
		 return my_exit();
	} else if (strcmp(cmd_args[0], "help") == 0 || strcmp(cmd_args[0], "h") == 0) {
		print_help();
	} else {
		printf("command: %s: Command is not recognized\n", cmd_args[0]);
		print_help();
	}
	return 1;
}
