#ifndef TOOLS_H__
#define TOOLS_H__


#define INIT_BUFFER_SIZE 256


/* typedef for a function pointer for functions which grab some environment 
 * variable and puts it into the c-string. The size parameter is the size the
 * function is allowed to copy into the buffer.
 */
typedef int (*env_getter)(char *, size_t);

char *get_env_value(env_getter getter, size_t init_len);

// new getcwd which follows format of function pointer described above
int new_getcwd(char *buf, size_t len);

//check the input for invalid data, return zero if good
int checkInput(char **cmd_args);


// new set of functions for grabbing cwd, hostname, and login which are bigger
// than the initial size of the buffer
char *full_getcwd(size_t init_len);
char *full_gethostname(size_t init_len);
char *full_getlogin_r(size_t init_len);


#endif
