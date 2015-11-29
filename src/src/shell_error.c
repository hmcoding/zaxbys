#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "shell_error.h"

void error_bad_directory(char *directory) {
	printf("Error: %s: directory does not exist\n", directory);
}

void error_open_already(char *filename) {
	printf("Error: %s: file already open!\n", filename);
}

void error_open_no_file(char *filename) {
	printf("Error: %s: file does not exist\n", filename);
}

void error_open_directory(char *directory) {
	printf("Error: %s: cannot open directory\n", directory);
}

void error_open_bad_param(char *param) {
	printf("Error: %s: incorrect parameter\n", param);
}
