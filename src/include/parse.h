#ifndef PARSER_H__
#define PARSER_H__


#define INPUT_BUFFER_SIZE 1024


char *readIn();


char **parseFunc(char *progLine);
char *parseFuncWhite(char *progLine);
char **parseFuncArgs(char *progLine);


char *remLeadWhite(char *progLine);
char *remMidWhite(char *progLine);
char *addMidWhite(char *progLine);
char *remTrailWhite(char *progLine);


int checkLastQuote(char *ptr);
size_t trackArgs(char *progLine);
int trackCmdSize(char *progLine);
/*
void ridCmdLine(char *progLine);
void ridCmdArgs(char **progArgs);
*/


#endif
