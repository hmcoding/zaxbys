#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include "tools.h"

/* get_env_value
 *
 * This takes in a function of a certain format and an initial size value and 
 * returns a c-string. The point of this function is to return an environment
 * variable using one of the standard functions; however, the dilemma before was
 * that the size of the buffer is unknown. So this function loops and calls the 
 * function passed into it everytime the ERANGE error is returned. The buf is
 * realloc'ed and the passed function is tried again.
 *
 * The c-string which is returned should be freed.
 */
char *get_env_value(int (*env_getter)(char *, size_t), size_t init_len) {
	size_t size = init_len;
	char *buf = malloc(size);
	while ((*env_getter)(buf, size) != -1 && errno == ERANGE) {
		size *= 2;
		buf = realloc(buf, size);
	}
	return buf;
}

/* new_getcwd
 *
 * takes in a c-string and size value and returns an int. A 0 is returned on
 * success and -1 is returned for an error. This function is required since
 * getcwd is the only one out of getcwd, gethostname, getlogin_r which doesn't
 * return an int (the function type "int func(char*,size_t)" is required for 
 * get_env_value).
 */
int new_getcwd(char *buf, size_t len) {
	if (getcwd(buf, len) == NULL) {
		return -1;
	} else {
		return 0;
	}
}

/* full_getcwd
 *
 * Returns the current working directory. The pointer returned should be freed.
 */
char *full_getcwd(size_t init_len) {
	char *buf;
	env_getter getter = &new_getcwd;
	buf = get_env_value(getter, init_len);
	return buf;
}

/* full_gethostname
 *
 * Returns the hostname of the comp. The pointer returned should be freed.
 */
char *full_gethostname(size_t init_len) {
	char *buf;
	env_getter getter = &gethostname;
	buf = get_env_value(getter, init_len);
	return buf;
}

/* full_getlogin_r
 *
 * Returns the current username. The pointer returned should be freed.
 */
char *full_getlogin_r(size_t init_len) {
	char *buf;
	env_getter getter = &getlogin_r;
	buf = get_env_value(getter, init_len);
	return buf;
}

//check the validity of the input
int checkInput(char **cmd_args)
{
int arg;					//argument value
int numArgs=0; 		//number of arguments

while (cmd_args[numArgs] != '\0'){
++numArgs;
}

for (arg=0;arg<numArgs;++arg)
{
if ((cmd_args[arg][0]) == '|')
	if ((arg == 0) || (arg == (numArgs-1)))
		return 1;
if ((cmd_args[arg][0]) == '&')
	if ((arg < (numArgs-1)) && (arg > 0))
		return 1;
if ((cmd_args[arg][0]) == '>')
	if ((arg == 0) || (arg == (numArgs-1)))
		return 1;
if ((cmd_args[arg][0]) == '<')
	if ((arg == 0) || (arg == (numArgs-1)))
		return 1;
}


return 0;
}


