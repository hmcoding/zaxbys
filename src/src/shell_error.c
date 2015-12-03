#include <stdio.h>

#include "shell_error.h"
//parsing errors
void error_dangling_quote(){
	printf("Error: closing quotation missing!\n");
}


// open/close errors
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

void error_close_directory(char *directory) {
	printf("Error: %s: cannot close directory\n", directory);
}

void error_not_open(char *filename) {
	printf("Error: %s: file not open\n", filename);
}


// create errors
void error_used_file(char *file_name) {
	printf("Error: %s: file already exists\n", file_name);
}

void error_specify_file_and_mode(char *command){
	printf("Error: %s: please specify a file name and mode\n", command);
} 

void error_specify_file(char *command){
	printf("Error: %s: please specify a file name\n", command);
} 


// ls, cd errors
void error_cd_file(char *filename) {
	printf("Error: %s: not a directory\n", filename);
}

void error_cd_not_here(char *directory) {
	printf("Error: %s: does not exist\n", directory);
}
