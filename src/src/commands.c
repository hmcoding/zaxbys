#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

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
		} else if (file_mode_to_byte(cmd_args[2]) == 0x0) {
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
	char *file_name;
	file_name = cmd_args[1];
	if (file_name == NULL) {
		error_specify_file("create");                
	}else{
		create_file(file_name,0);
	}
return 1;
}


int create_file(char *file_name, int directory){
	unsigned int clus,i,period_found = 0,file_found;
	union directory_entry file;
	char eoc[] = {0x0F,0xFF,0xFF,0xF8};
	file_found = find_file(file_name, cur_dir_clus, &file);
		if (!file_found){
			find_open_directory_entry(cur_dir_clus,&file);
			for (i=0;i<11;++i){
				if (file_name[i] == '\0'){
					break;
				}else if(file_name[i] == '.'){
					while(i < 8)
						file.sf.name[i++] = ' ';
					i = 7;
					period_found = 1;
				}else if((i > 7) && (period_found == 0)){
				}else{
					file.sf.name[i] = toupper(file_name[i]);
				}
			}
			if (directory)
				 file.sf.attr = 0x10;
			else
				 file.sf.attr = 0x00;
			file.sf.crt_time = file.sf.wrt_time = get_time();
			file.sf.crt_date = file.sf.wrt_date = file.sf.last_acc_date = get_date();
			clus = find_open_cluster(); 
			write_chars(eoc,get_fat_cluster_position(clus,0),4);
			write_chars(eoc,get_fat_cluster_position(clus,1),4);
			file.sf.first_clus_hi = get_hi(clus);
			file.sf.first_clus_lo = get_lo(clus);

		
		}
		else {
			error_used_file(file_name);
			return 0;
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
	union directory_entry file;
	if (cmd_args[1] == NULL) {
		error_specify_file(cmd_args[0]);
	} else {
		found = find_file(cmd_args[1], cur_dir_clus, &file);
		if (!found) {
			error_open_no_file(cmd_args[1]);
		} else {
			printf("%u\n", get_size(&file));
		}
	}
	return 0;
}

int my_read(char **cmd_args) {
	int found;
	union directory_entry file;
	char *filename;
	unsigned int file_clus, position, size, file_size;
	struct node *file_ptr;
	if (cmd_args[1] == NULL || cmd_args[2] == NULL || cmd_args[3] == NULL) {
		error_specify_file_pos_size(cmd_args[0]);
	} else {
		filename = cmd_args[1];
		position = strtoul(cmd_args[2], NULL, 10);
		size = strtoul(cmd_args[3], NULL, 10);
		found = find_file(filename, cur_dir_clus, &file);
		if (!found) {
			error_open_no_file(filename);
		} else if ((file.sf.attr & ATTR_DIRECTORY) == ATTR_DIRECTORY) {
			error_close_directory(filename);
		} else {
			file_clus = get_file_cluster(&file);
			file_ptr = opened_files->find(opened_files, file_clus);
			file_size = get_size(&file);
			if (file_ptr == NULL) {
				error_not_open(filename);
			} else if (!check_read(file_ptr)) {
				error_not_readable(filename);
			} else if (position + size > file_size) {
				error_beyond_EOF(position, size, file_size);
			} else {
				read_file(&file, position, size);
				printf("\n");
			}
		}
	}
	return 0;
}

int my_write(char **cmd_args) {
	return 0;
}

int print_help(void) {
	printf("list of commands\n");
	printf("open <FILE_NAME> <MODE>\n");
	printf("close <FILE_NAME>\n");
	printf("create <FILE_NAME>\n");
	printf("rm <FILE_NAME>\n");
	printf("size <FILE_NAME>\n");
	printf("cd <DIR_NAME>\n");
	printf("ls <DIR_NAME>\n");
	printf("mkdir <DIR_NAME>\n");
	printf("rmdir <DIR_NAME>\n");
	printf("read <FILE_NAME> <POSITION> <NUM_BYTES>\n");
	printf("write <FILE_NAME> <POSITION> <NUM_BYTES> <STRING>\n");
	printf("help\n");
	return 0;
}

int my_exit(void) {
	printf("exiting fat32 utility\n");
	return 0;
}
