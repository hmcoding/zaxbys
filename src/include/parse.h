#ifndef PARSER_H__
#define PARSER_H__


#define INPUT_BUFFER_SIZE 1024


char *readIn();


char **parse(char *progLine);
char *parseWhite(char *progLine);
char **parseArgs(char *progLine);


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
