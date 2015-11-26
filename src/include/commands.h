#ifndef COMMANDS_H__
#define COMMANDS_H__

int my_open(char **cmd_args);
int my_close(char **cmd_args);
int my_create(char **cmd_args);
int my_rm(char **cmd_args);
int my_size(char **cmd_args);
int my_cd(char **cmd_args);
int my_ls(char **cmd_args);
int my_mkdir(char **cmd_args);
int my_rmdir(char **cmd_args);
int my_read(char **cmd_args);
int my_write(char **cmd_args);
int print_help();

/*int my_open_stub(cmd_args);
int my_close_stub(cmd_args);
int my_create_stub(cmd_args);
int my_rm_stub(cmd_args);
int my_size_stub(cmd_args);
int my_cd_stub(cmd_args);
int my_ls_stub(cmd_args);
int my_mkdir_stub(cmd_args);
int my_rmdir_stub(cmd_args);
int my_read_stub(cmd_args);
int my_write_stub(cmd_args);
int print_help_stub();*/

#endif
