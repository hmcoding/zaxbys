#include <stdio.h>

#include "clean.h"
#include "parse.h"
#include "setup.h"

FILE *fat32_img;


/* clean_up
 *
 * frees up the memory at *cmd_line and *cmd_args
 */
void clean_up_loop(char *cmd_line, char **cmd_args) {
	destroy_cmd_line(cmd_line);
	destroy_cmd_args(cmd_args);
}

void clean_up_globals(void) {
	fclose(fat32_img);
}
