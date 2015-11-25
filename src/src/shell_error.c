#include <stdio.h>
#include <string.h>
#include <errno.h>


/* shell_perror
 *
 * A simple function which prints the error message for certain commands. It
 * takes the command and argument string (which can be customized by the calling
 * function) and outputs the total error message.
 */
void shell_perror(char *cmd, int errnum) {
	fprintf(stderr, "baash: %s: %s\n", cmd, strerror(errnum));
}
