#ifndef COMMANDS_H__
#define COMMANDS_H__

int infoCmd();
int openCmd(char **cmd_args);
int closeCmd(char **cmd_args);
int createCmd(char **cmd_args);
int rmCmd(char **cmd_args);
int sizeCmd(char **cmd_args);
int readCmd(char **cmd_args);
int writeCmd(char **cmd_args);
int exitCmd(void);


#endif
