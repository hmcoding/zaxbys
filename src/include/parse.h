#ifndef PARSER_H__
#define PARSER_H__


#define INPUT_BUFFER_SIZE 1024

// function for reading from stdin
char *readIn();

/* parse and its collection of functions utilized
 *
 * parseWhite -- remove and add whitespace where needed
 * parseArgs -- split the string into an array of arguments
 * expand_variables -- find variable like arguments and replace them with value
 * resolve_paths -- find paths and fully expand to absolute paths
 */
char **parse(char *progLine);
char *parseWhite(char *progLine);
char **parseArgs(char *progLine);

// helper functions to the main components of parsing
char *remLeadWhite(char *progLine);
char *remMidWhite(char *progLine);
char *addMidWhite(char *progLine);
char *remTrailWhite(char *progLine);

// some other utility functions
int checkLastQuote(char *ptr);
size_t trackArgs(char *progLine);
int trackCmdSize(char *progLine);
void ridCmdLine(char *progLine);
void ridCmdArgs(char **progArgs);

#endif
