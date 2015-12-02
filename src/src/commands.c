#include <stdio.h>
#include <stdlib.h>

#include "setup.h"
#include "file_types.h"
#include "commands.h"
#include "tools.h"
#include "shell_error.h"

struct list *opened_files;

int my_open(char **cmd_args) {
	int found;
	union directory_entry file;
	unsigned int file_clus;
	struct node *file_ptr;
	if (cmd_args[1] == NULL || cmd_args[2] == NULL) {
		error_specify_file_and_mode(cmd_args[0]);
	} else {
		found = find_file(cmd_args[1], cur_dir_clus, &file);
		if (!found) {
			error_open_no_file(cmd_args[1]);
		} else if ((file.sf.attr & ATTR_DIRECTORY) == ATTR_DIRECTORY) {
			error_open_directory(cmd_args[1]);
		} else if (file_mode_to_byte(cmd_args[2])) {
			error_open_bad_param(cmd_args[2]);
		} else {
			file_clus = get_file_cluster(&file);
			file_ptr = opened_files->find(opened_files, file_clus);
			if (file_ptr != NULL) {
				error_open_already(cmd_args[1]);
			} else {
				file.sf.last_acc_date = get_date();
				opened_files->add(opened_files, get_file_cluster(&file), cmd_args[2]);
			}
		}
	}
	return 0;
}

int my_close(char **cmd_args) {
	int found;
	union directory_entry file;
	unsigned int file_clus;
	struct node *file_ptr;
	if (cmd_args[1] == NULL) {
		error_specify_file(cmd_args[0]);
	} else {
		found = find_file(cmd_args[1], cur_dir_clus, &file);
		if (!found) {
			error_open_no_file(cmd_args[1]);
		} else if ((file.sf.attr & ATTR_DIRECTORY) == ATTR_DIRECTORY) {
			error_close_directory(cmd_args[1]);
		} else {
			file_clus = get_file_cluster(&file);
			file_ptr = opened_files->find(opened_files, file_clus);
			if (file_ptr == NULL) {
				error_not_open(cmd_args[1]);
			} else {
				opened_files->remove(opened_files, file_clus);
			}
		}
	}
	return 0;
}

int my_create(char **cmd_args) {
	int found;
	char *file_name;
	union directory_entry file;
	file_name = cmd_args[1];
	if (file_name == NULL) {
		error_specify_file("create");                
	}
	else { 
		found = find_file(file_name, cur_dir_clus, &file);
		if (!(found)){
			file.sf.crt_time = file.sf.wrt_time =  get_time();
			file.sf.crt_date = file.sf.wrt_date =  file.sf.last_acc_date = get_date();
		}
		else {
			error_used_file(file_name);
			return 0;
		}
	}
	return 0;
}

int my_rm(char **cmd_args) {
	int found;
	char *file_name;
	union directory_entry file;
	file_name = cmd_args[1];
	if (file_name == NULL) {
		error_specify_file("rm");                
	} else {
		found = find_file(cmd_args[1], cur_dir_clus, &file);
		if (found && ((file.sf.attr & ATTR_DIRECTORY) != ATTR_DIRECTORY)) {

		}
		else {
			error_open_no_file(cmd_args[1]);
			return 0;
		}
	}
	return 0;
}

int my_size(char **cmd_args) {
	int found;
	char *file_name;
	union directory_entry file;
	file_name = cmd_args[1];
	if (file_name == NULL) {
		error_specify_file("size");                
	} else {
		found = find_file(cmd_args[1], cur_dir_clus, &file);
		if (found && ((file.sf.attr & ATTR_DIRECTORY) != ATTR_DIRECTORY)) {

		}
		else {
			error_open_no_file(cmd_args[1]);
			return 0;
		}
	}
	return 0;
}

int my_read(char **cmd_args) {
	return 0;
}

int my_write(char **cmd_args) {
	return 0;
}

int print_help(void) {
	return 0;
}

int my_exit(void) {
	printf("exiting fat32 utility\n");
	return 0;
}
