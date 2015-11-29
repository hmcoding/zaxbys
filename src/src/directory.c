#include <stdio.h>

#include "setup.h"
#include "directory.h"
#include "file_types.h"
#include "tools.h"
#include "shell_error.h"


struct fat32_info img_info;
unsigned int cur_dir_clus;


int my_cd(char **cmd_args) {
	return 0;
}

int my_ls(char **cmd_args) {
	int found;
	char *directory;
	union directory_entry file;
	unsigned int dir_clus;
	directory = cmd_args[1];
	if (directory == NULL) {
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
