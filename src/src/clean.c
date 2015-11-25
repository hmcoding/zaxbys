#include "clean.h"
#include "parse.h"


/* clean_up
 *
 * frees up the memory at *cmd_line and *cmd_args
 */
void clean_up_loop(char *cmd_line, char **cmd_args) {
	destroy_cmd_line(cmd_line);
	destroy_cmd_args(cmd_args);
}

void clean_up_globals(void) {
}
