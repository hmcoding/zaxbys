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

void error_used_file(char *file_name) {
	printf("Error: %s: file with that name already exists in surrent directory\n", file_name);
}

void error_specify_file(char *command){
	printf("Error: %s: please specify a file name\n", command);
}     

void error_cd_file(char *filename) {
	printf("Error: %s: not a directory\n", filename);
}

void error_cd_not_here(char *directory) {
	printf("Error: %s: does not exist\n", directory);
}
