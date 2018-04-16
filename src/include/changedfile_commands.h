#ifndef COMMANDS_H__
#define COMMANDS_H__

int infoCmd();
int openCmd(char **progArgs);
int closeCmd(char **progArgs);
int createCmd(char **progArgs);
int rmCmd(char **progArgs);
int sizeCmd(char **progArgs);
int readCmd(char **progArgs);
int writeCmd(char **progArgs);
int exitCmd(void);


#endif
