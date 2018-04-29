#ifndef MYPARSER_H__
#define MYPARSER_H__


#define INPUT_BUFFER_SIZE 1024


char *readIn();
char **parseFunc(char *progLine);
char *parseFuncWhite(char *progLine);
char *addMidWhite(char *progLine);
char *remLeadWhite(char *progLine);
char *remMidWhite(char *progLine);
char *remTrailWhite(char *progLine);
char **parseFuncArgs(char *progLine);
size_t trackArgs(char *progLine);
int trackCmdSize(char *progLine);
int checkLastQuote(char *ptr);




#endif
