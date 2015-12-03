#ifndef SHELL_ERROR_H__
#define SHELL_ERROR_H__

//parsing errors
void error_dangling_quote();

// set of open/close errors
void error_open_already(char *filename);
void error_open_no_file(char *filename);
void error_open_directory(char *directory);
void error_open_bad_param(char *param);
void error_close_directory(char *directory);
void error_not_open(char *filename);

// create errors
void error_used_file(char *file_name);
void error_specify_file_and_mode(char *command);
void error_specify_file(char *command);

// set of cd errors
void error_cd_file(char *filename);
void error_cd_not_here(char *directory);

// set of read/write errors
void error_specify_file_pos_size(char *command);
void error_not_readable(char *filename);
void error_beyond_EOF(unsigned int position, unsigned int size, unsigned int file_size);

#endif
