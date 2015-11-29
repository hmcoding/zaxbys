#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "setup.h"
#include "directory.h"
#include "file_types.h"
#include "tools.h"
#include "shell_error.h"


struct fat32_info img_info;
unsigned int cur_dir_clus;
char *current_directory;
unsigned int current_directory_capacity;


int my_cd(char **cmd_args) {
	int found;
	union directory_entry file;
	if (cmd_args[1] != NULL) {
		found = find_file(cmd_args[1], cur_dir_clus, &file);
		if (!found) {
			error_cd_not_here(cmd_args[1]);
		} else if((file.sf.attr & ATTR_DIRECTORY) != ATTR_DIRECTORY) {
			error_cd_file(cmd_args[1]);
		} else { // it's a directory
			change_directory_cluster(&file);
			change_current_directory(&file);
		}
	}
	return 0;
}

/* prints the entries in the given directory in cmd_args[1]
 */
int my_ls(char **cmd_args) {
	int found;
	union directory_entry file;
	unsigned int dir_clus;
	if (cmd_args[1] == NULL) {
		dir_clus = cur_dir_clus;
	} else {
		found = find_file(cmd_args[1], cur_dir_clus, &file);
		if (found && (file.sf.attr & ATTR_DIRECTORY) == ATTR_DIRECTORY) {
			dir_clus = get_file_cluster(&file);
		} else {
			error_bad_directory(cmd_args[1]);
			return 0;
		}
	}
	print_directory(dir_clus);
	printf("\n");
	return 0;
}

int my_mkdir(char **cmd_args) {
	return 0;
}

int my_rmdir(char **cmd_args) {
	return 0;
}

int print_directory(unsigned int directory_clus) {
	unsigned int current_clus, i, limit, done;
	union directory_entry file;
	char filename[12];
	current_clus = directory_clus;
	limit = img_info.bytes_per_sec*img_info.sec_per_clus/32;
	done = 0;
	do {
		for (i = 0; i < limit; ++i) {
			get_directory_entry(&file, current_clus, i);
			if (file.raw_bytes[0] == 0x00) {
				done = 1;
				break;
			} else if (file.raw_bytes[0] == 0xE5) {
				continue;
			} else if ((file.lf.attr & ATTR_LONG_NAME_MASK) == ATTR_LONG_NAME) {
				continue;
			} else if ((file.sf.attr & (ATTR_DIRECTORY | ATTR_VOLUME_ID)) == 0x00) {
				short_to_lowercase(filename, file.sf.name);
				printf("%s\t", filename);
			} else if ((file.sf.attr & (ATTR_DIRECTORY | ATTR_VOLUME_ID)) == ATTR_DIRECTORY) {
				short_to_lowercase(filename, file.sf.name);
				printf("%s/\t", filename);
			} else if ((file.sf.attr & (ATTR_DIRECTORY | ATTR_VOLUME_ID)) == ATTR_VOLUME_ID) {
				continue;
			} else {
				// bad file
			}
		}
		current_clus = get_next_cluster_in_fat(current_clus);
	} while (!end_of_chain(current_clus) && !done);
	return 1;
}

int change_directory_cluster(union directory_entry *ptr) {
	unsigned int file_clus;
	if ((ptr->sf.attr & ATTR_DIRECTORY) == ATTR_DIRECTORY) {
		file_clus = get_file_cluster(ptr);
		if (file_clus == 0) {
			cur_dir_clus = img_info.root_clus;
		} else {
			cur_dir_clus = file_clus;
		}
	}
	return 1;
}

int change_current_directory(union directory_entry *ptr) {
	char filename[12];
	unsigned int pen_slash;
	if ((ptr->sf.attr & ATTR_DIRECTORY) == ATTR_DIRECTORY) {
		short_to_lowercase(filename, ptr->sf.name);
		if (get_file_cluster(ptr) == 0) {
			strcpy(current_directory, "/");
		} else if (strcmp(filename, ".") == 0) {
			// do nothing
		} else if (strcmp(filename, "..") == 0) {
			pen_slash = find_penultimate_slash();
			current_directory[pen_slash + 1] = '\0';
		} else {
			if (strlen(current_directory) + strlen(filename) + 1 > current_directory_capacity) {
				current_directory_capacity *= 2;
				current_directory = realloc(current_directory, current_directory_capacity);
			}
			strcat(current_directory, filename);
			strcat(current_directory, "/");
		}
	}
	return 1;
}

unsigned int find_penultimate_slash() {
	unsigned int ultimate, penultimate, i;
	ultimate = 0, penultimate = 0;
	for (i = 1; current_directory[i] != '\0'; ++i) {
		if (current_directory[i] == '/') {
			penultimate = ultimate;
			ultimate = i;
		}
	}
	return penultimate;
}
