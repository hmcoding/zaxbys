#ifndef PARSER_H__
#define PARSER_H__


#define INPUT_BUFFER_SIZE 1024

// function for reading from stdin
char *read_input();

/* parse and its collection of functions utilized
 *
 * parse_whitespace -- remove and add whitespace where needed
 * parse_arguments -- split the string into an array of arguments
 * expand_variables -- find variable like arguments and replace them with value
 * resolve_paths -- find paths and fully expand to absolute paths
 */
char **parse(char *cmd_line);
char *parse_whitespace(char *cmd_line);
char **parse_arguments(char *cmd_line);

// helper functions to the main components of parsing
char *remove_leading_whitespace(char *cmd_line);
char *remove_middle_whitespace(char *cmd_line);
char *add_middle_whitespace(char *cmd_line);
char *remove_trailing_whitespace(char *cmd_line);

// some other utility functions
int check_end_quote(char *ptr);
size_t count_args(char *cmd_line);
int check_cmd_size(char *cmd_line);
void destroy_cmd_line(char *cmd_line);
void destroy_cmd_args(char **cmd_args);

#endif
