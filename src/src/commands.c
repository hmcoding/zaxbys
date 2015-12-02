#include <stdio.h>
#include <stdlib.h>

#include "setup.h"
#include "file_types.h"
#include "commands.h"
#include "tools.h"
#include "shell_error.h"

int my_open(char **cmd_args) {
	int found;
	char *file_name;
   union directory_entry file;
   file_name = cmd_args[1];
   if (file_name == NULL) {
		error_specify_file("open");
		return 0;
   } else {
      found = find_file(cmd_args[1], cur_dir_clus, &file);
      if (found && ((file.sf.attr & ATTR_DIRECTORY) != ATTR_DIRECTORY)) {
			file.sf.last_acc_date = get_date(&file);
	
   	} else {
      	error_open_no_file(cmd_args[1]);
      	return 0;
		}
	}
	 return 1;
}

int my_close(char **cmd_args) {
        int found;
        char *file_name;
        union directory_entry file;
        file_name = cmd_args[1];
        if (file_name == NULL) {
		error_specify_file("close");                
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
		file.sf.crt_time = file.sf.wrt_time =  get_time(&file);
		file.sf.crt_date = file.sf.wrt_date =  file.sf.last_acc_date = get_date(&file);
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
