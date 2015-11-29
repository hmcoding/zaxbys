#ifndef DIRECTORY_H__
#define DIRECTORY_H__

// commands for directories
int my_cd(char **cmd_args);
int my_ls(char **cmd_args);
int my_mkdir(char **cmd_args);
int my_rmdir(char **cmd_args);


int print_directory(unsigned int directory_clus);

#endif
