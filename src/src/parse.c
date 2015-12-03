#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "shell_error.h"
#include "parse.h"


/* read_input
 *
 * Dynamically allocates memory for a c-string, reads stdin and places the input
 * into the c-string. Returns the c-string.
 */
char *read_input() {
	char *cmd_line = malloc(INPUT_BUFFER_SIZE);
	if (fgets(cmd_line, INPUT_BUFFER_SIZE, stdin)) {
		return cmd_line;
	} else {
		//Do something about this error, NULL on return
		return NULL;
	}
}

/* parse
 * 
 * The grand parsing function which wraps around many other smaller functions
 * necessary for parsing the commands given in the terminal.
 */
char **parse(char *line) {
	char **args;
	line = parse_whitespace(line);
	args = parse_arguments(line);
	return args;
}

/* parse_whitespace
 *
 * Takes a c-string, cmd_line, and removes unnecessary whitespace and adds
 * whitespace where needed.
 *
 * special characters: |, <, >, &, $, ~
 * don't do this for: ., /
 * whitespace: <space>, \t, \v, \f, \r (\n is used to delimit commands)
 */
char *parse_whitespace(char *cmd_line) {
	remove_leading_whitespace(cmd_line);
	remove_middle_whitespace(cmd_line);
	add_middle_whitespace(cmd_line);
	remove_trailing_whitespace(cmd_line);
	return cmd_line;
}

/* parse_arguments
 *
 * Takes a c-string, cmd_line, and splits it into tokens delimited by space. An
 * array of strings is returned. The size of the array of strings is governed by
 * the constant PARAM_LIMIT. The array and it's elements use allocated memory
 * and should be freed later. The last element is followed by a NULL pointer to
 * mark the end of the array.
 */
char **parse_arguments(char *cmd_line) {
	int arg_amount = count_args(cmd_line);
	// arg_amount + 1 for the null value at the end
	char **cmd_args = (char **) calloc((size_t)arg_amount + 1, sizeof(char *));
	char *token, *tmp_line, *save_ptr;
	char sep[] = " ";
	tmp_line = strdup(cmd_line);
	int i = 0;
	token = strtok_r(tmp_line, sep, &save_ptr);
	while (token != NULL) {
		cmd_args[i] = (char *) malloc(strlen(token) + 1);
		strcpy(cmd_args[i++], token);
		token = strtok_r(NULL, sep, &save_ptr);
	}
	cmd_args[i] = NULL;
	free(tmp_line);
	return cmd_args;
}

/* count_args
 *
 * This take a c-string and counts the number of arguments which corresponds to
 * the number of spaces plus one.
 */
int count_args(char *cmd_line) {
	int i, count;
	for (i = 0; cmd_line[i] != '\0'; ++i) {
		if (cmd_line[i] == ' ') {
			++count;
		}
	}
	return count + 1;
}

/* remove_leading_whitespace
 *
 * Takes the input c-string and loops through string. The leading whitespace is
 * ignored while the rest of the string is shifted to the front.
 */
char *remove_leading_whitespace(char *cmd_line){
	char *copy = cmd_line, *ptr = cmd_line;
	short reached_nonspace = 0;
	while (*ptr != 0) {
		if (!isspace(*ptr)){
			reached_nonspace = 1;
		}
		if (reached_nonspace) {
			*copy = *ptr;
			copy++;
		}
		ptr++;
	}
	*copy = '\0';
	return cmd_line;
}

/* remove_middle_whitespace
 *
 * Takes c-string and shifts the characters so that there is at most one space
 * character between each token separated by whitespace. E.g.
 * 
 * "file1   \t \v    file2" ==> "file1 file2"
 * 
 * A null byte is added to the end of the string -- doesn't worry about the 
 * whitespace at the end.
 */
char *remove_middle_whitespace(char *cmd_line){
	// copy holds place where char is modified, ptr holds place where checking
	char *copy = cmd_line, *ptr = cmd_line;
	int in_quotes = 0;
	int ws_count = 0;
	while (*ptr != 0) {
		if (*ptr == '"'){
			if (in_quotes == 0){
				if (check_end_quote(ptr)){
			         	error_dangling_quote();
					cmd_line[0] = 0;
					break;	
				}
				in_quotes = 1;
			} else {
			in_quotes = 0;
			} 
			
		} else if (in_quotes){
			*copy = *ptr;
			copy++;
			ws_count = 0;
		} else if (isspace(*ptr)) {
			if (ws_count == 0) {
				*copy = ' ';
				copy++;
			}
			ws_count++;
		} else {
			*copy = *ptr;
			copy++;
			ws_count = 0;
		}
		ptr++;
	}
	*copy = '\0';
	return cmd_line;
}


int check_end_quote(char* ptr){
	char* init_ptr = ptr++;
	while (*ptr != 0){
		if (*ptr == '"'){
		ptr = init_ptr;
		return 0;
		}
	++ptr;
	}
	ptr = init_ptr;
	return 1;
}



/* add_middle_whitespace
 *
 * Takes c-string and adds space character between tokens which may not have any
 * whitespace initially. Special characters include <, >, |, &. Also in
 * consideration is >>.
 */
char *add_middle_whitespace(char *cmd_line){
	char copy[INPUT_BUFFER_SIZE];
	char *ptr = cmd_line;
	int i = 0;
	while (*ptr != 0) {
		switch (*ptr) {
			case '|':
			case '<':
			case '>':
			case '&':
				if (i > 0 && copy[i - 1] != ' ') {
					copy[i++] = ' ';
				}
				copy[i++] = *ptr;
				if (*(ptr + 1) != ' ') {
					copy[i++] = ' ';
				} else if ((ptr + 1) != 0) {
					copy[i++] = ' ';
					++ptr;
				}
				++ptr;
				break;
			default:
				copy[i++] = *ptr;
				++ptr;
		}
	}
	copy[i] = '\0';
	if (!check_cmd_size(copy)) {
		// error stuff goes here
	}
	strcpy(cmd_line, copy);
	return cmd_line;
}

/* remove_trailing_whitespace
 *
 * Takes c-string and adds the a null byte after the last non-whitespace char.
 */
char *remove_trailing_whitespace(char *cmd_line){
	char *last = cmd_line, *ptr = cmd_line;
	while (*ptr != 0) {
		if (!isspace(*ptr)) {
			last = ptr;
		}
		ptr++;
	}
	*(++last) = '\0';
	return cmd_line;
}

/* check_cmd_size
 *
 * Makes sure the given string is shorter than the maximum allowed size of the
 * command string. Returns 1 if the string is good and 0 if the string is bad
 */
int check_cmd_size(char *cmd_line) {
	int i;
	for (i = 0; i < INPUT_BUFFER_SIZE; ++i) {
		if (cmd_line[i] == '\0') {
			return 1;
		}
	}
	return 0;
}

/* destroy_cmd_line
 * 
 * Takes a c-string and frees the memory allocated for that
 */
void destroy_cmd_line(char *cmd_line) {
	free(cmd_line);
}

/* destroy_cmd_args
 *
 * Takes array of c-strings and frees each element and then frees the memory of
 * the array
 */
void destroy_cmd_args(char **cmd_args) {
	int i;
	for (i = 0; cmd_args[i] != NULL; ++i) {
		free(cmd_args[i]);
		cmd_args[i] = NULL;
	}
	free(cmd_args);
}
