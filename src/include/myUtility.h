#ifndef MYUTILITY_H__
#define MYUTILITY_H__

union dirEntry;

void loopClean(char *progLine, char **progArgs);
void globClean(void);
void error(int type, char* print);
int displayDir(unsigned int dirClus);
int changeDirClus(union dirEntry *ptr);
int changeCurDir(union dirEntry *ptr);
unsigned int lookupNextToLastSlash();
int userCmd(char **progArgs);
int exitCmd(void);
int infoCmd();
int lsCmd(char **progArgs);
int cdCmd(char **progArgs);
int sizeCmd(char **progArgs);
int createCmd(char **progArgs);
int mkdirCmd(char **progArgs);
int rmCmd(char **progArgs);
int rmdirCmd(char **progArgs);
int openCmd(char **progArgs);
int closeCmd(char **progArgs);
int readCmd(char **progArgs);
int writeCmd(char **progArgs);


#endif
