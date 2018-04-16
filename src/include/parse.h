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
char **parse(char *cmd_line);
char *parseWhite(char *cmd_line);
char **parseArgs(char *cmd_line);

// helper functions to the main components of parsing
char *remLeadWhite(char *cmd_line);
char *remMidWhite(char *cmd_line);
char *addMidWhite(char *cmd_line);
char *remTrailWhite(char *cmd_line);

// some other utility functions
int checkLastQuote(char *ptr);
size_t trackArgs(char *cmd_line);
int trackCmdSize(char *cmd_line);
void ridCmdLine(char *cmd_line);
void ridCmdArgs(char **cmd_args);

#endif
