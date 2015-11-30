#ifndef DIRECTORY_H__
#define DIRECTORY_H__

union directory_entry;

// commands for directories
int my_cd(char **cmd_args);
int my_ls(char **cmd_args);
int my_mkdir(char **cmd_args);
int my_rmdir(char **cmd_args);

// useful sub routines for directories
int print_directory(unsigned int directory_clus);
int change_directory_cluster(union directory_entry *ptr);
int change_current_directory(union directory_entry *ptr);
unsigned int find_penultimate_slash();

#endif
