#ifndef COMMANDS_H__
#define COMMANDS_H__

int my_open(cmd_args);
int my_close(cmd_args);
int my_create(cmd_args);
int my_rm(cmd_args);
int my_size(cmd_args);
int my_cd(cmd_args);
int my_ls(cmd_args);
int my_mkdir(cmd_args);
int my_rmdir(cmd_args);
int my_read(cmd_args);
int my_write(cmd_args);
int print_help();

int my_open_stub(cmd_args);
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
int print_help_stub();

#endif
