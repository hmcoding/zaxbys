#ifndef DIRECTORY_H__
#define DIRECTORY_H__

union dirEntry;

// commands for directories
int cdCmd(char **cmd_args);
int lsCmd(char **cmd_args);
int mkdirCmd(char **cmd_args);
int rmdirCmd(char **cmd_args);

// useful sub routines for directories
int displayDir(unsigned int directory_clus);
int changeDirClus(union dirEntry *ptr);
int changeCurDir(union dirEntry *ptr);
unsigned int find_penultimate_slash();

#endif
