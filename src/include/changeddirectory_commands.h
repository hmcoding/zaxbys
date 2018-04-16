#ifndef DIRECTORY_H__
#define DIRECTORY_H__

union dirEntry;

// commands for directories
int cdCmd(char **progArgs);
int lsCmd(char **progArgs);
int mkdirCmd(char **progArgs);
int rmdirCmd(char **progArgs);

// useful sub routines for directories
int displayDir(unsigned int dirClus);
int changeDirClus(union dirEntry *ptr);
int changeCurDir(union dirEntry *ptr);
unsigned int lookupNextToLastSlash();

#endif
