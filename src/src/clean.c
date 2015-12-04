#include <stdio.h>
#include <stdlib.h>

#include "clean.h"
#include "parse.h"
#include "setup.h"
#include "file_types.h"

FILE *fat32_img;
struct list *opened_files;


/* clean_up
 *
 * frees up the memory at *cmd_line and *cmd_args
 */
void clean_up_loop(char *cmd_line, char **cmd_args) {
	destroy_cmd_line(cmd_line);
	destroy_cmd_args(cmd_args);
}

void clean_up_globals(void) {
	delete_list(opened_files);
	free(current_directory);
	fclose(fat32_img);
}
