#ifndef COMMANDS_H__
#define COMMANDS_H__

int my_info();
int my_open(char **cmd_args);
int my_close(char **cmd_args);
int my_create(char **cmd_args);
int my_rm(char **cmd_args);
int my_size(char **cmd_args);
int my_read(char **cmd_args);
int my_write(char **cmd_args);
int print_help(void);
int my_exit(void);


#endif
