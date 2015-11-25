#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "setup.h"
#include "tools.h"
#include "parse.h"

char *current_directory;
unsigned int cur_dir_capacity;

void setup(char *img_filename) {
	print_introduction(img_filename);
	current_directory = calloc(INIT_CUR_DIR_CAP, sizeof(char));
	strcat(current_directory, "/");
}

void print_prompt(void) {
	printf("%s]", current_directory);
}

void print_introduction(char *img_filename) {
	printf("Welcome\nImage, %s, is ready to view\n", img_filename);
}
