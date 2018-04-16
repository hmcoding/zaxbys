#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <time.h>



#include "clean.h"
#include "parse.h"
#include "setup.h"
#include "file_types.h"
#include "execute.h"
#include "file_commands.h"
#include "directory_commands.h"
#include "tools.h"
#include "shell_error.h"

// VARIABLES FOR ALL FILES

FILE *fat32_img;
struct list *opened_files;
struct fat32_info img_info;
unsigned int cur_dir_clus;
char *current_directory;
unsigned int current_directory_capacity;
int endianness;
unsigned int first_data_sec;
unsigned int cur_dir_sec;
unsigned int first_root_sec;
unsigned int count_of_clusters;



// END VARIABLES


// CLEAN



void loopClean(char *cmd_line, char **cmd_args) {
	ridCmdLine(cmd_line);
	ridCmdArgs(cmd_args);
}

void globClean(void) {
	delList(opened_files);
	free(current_directory);
	fclose(fat32_img);
}
// END CLEAN

// DIRECTORY




int cdCmd(char **cmd_args) {
	int check;
	union dirEntry file;
	if (cmd_args[1] != NULL) {
		check = lookupFile(cmd_args[1], cur_dir_clus, &file, NULL, NULL);
		if (!check) {
			error_cd_not_here(cmd_args[1]);
		} else if((file.sf.attr & ATTR_DIRECTORY) != ATTR_DIRECTORY) {
			error_cd_file(cmd_args[1]);
		} else { // it's a directory
			changeDirClus(&file);
			changeCurDir(&file);
		}
	}
	return 0;
}

/* prints the entries in the given directory in cmd_args[1]
 */
int lsCmd(char **cmd_args) {
	int check;
	union dirEntry file;
	unsigned int dir_clus;
	if (cmd_args[1] == NULL) {
		dir_clus = cur_dir_clus;
	} else {
		check = lookupFile(cmd_args[1], cur_dir_clus, &file, NULL, NULL);
		if (!check) {
			error_cd_not_here(cmd_args[1]);
			return 0;
		} else if ((file.sf.attr & ATTR_DIRECTORY) != ATTR_DIRECTORY) {
			error_cd_file(cmd_args[1]);
			return 0;
		} else {
			dir_clus = retFileClus(&file);
		}
	}
	displayDir(dir_clus);
	printf("\n");
	return 0;
}

int mkdirCmd(char **cmd_args) {
	int check;
	union dirEntry file;
	unsigned int dir_clus, offset;
	if (cmd_args[1] == NULL) {
		error_specify_file(cmd_args[0]);
	} else {
		check = lookupFile(cmd_args[1], cur_dir_clus, &file, &dir_clus, &offset);
		if (check) {
			error_file_or_directory_exists(cmd_args[1]);
		} else {
			makeDir(cmd_args[1], cur_dir_clus);
		}
	}
	return 0;
}

int rmdirCmd(char **cmd_args) {
	int check;
	char *file_name;
	union dirEntry file;
	unsigned int dir_clus, offset;
	file_name = cmd_args[1];
	if (file_name == NULL) {
		error_specify_directory(cmd_args[0]);                
	} else {
		check = lookupFile(cmd_args[1], cur_dir_clus, &file, &dir_clus, &offset);
		if (!check) {
			error_open_no_file(cmd_args[1]);
		} else if ((file.sf.attr & ATTR_DIRECTORY) != ATTR_DIRECTORY) {
			error_bad_directory(cmd_args[1]);
		} else if (!emptyDir(&file)) {
			error_not_empty(cmd_args[1]);
		} else {
			fileDel(&file, dir_clus, offset);
		}
	}
	return 0;
}

int displayDir(unsigned int directory_clus) {
	unsigned int current_clus, i, limit, done;
	union dirEntry file;
	char filename[12];
	current_clus = directory_clus;
	limit = img_info.bytes_per_sec*img_info.sec_per_clus/32;
	done = 0;
	do {
		for (i = 0; i < limit; ++i) {
			retDirEntry(&file, current_clus, i);
			if (file.raw_bytes[0] == 0x00) {
				done = 1;
				break;
			} else if (file.raw_bytes[0] == 0xE5) {
				continue;
			} else if ((file.lf.attr & ATTR_LONG_NAME_MASK) == ATTR_LONG_NAME) {
				continue;
			} else if ((file.sf.attr & (ATTR_DIRECTORY | ATTR_VOLUME_ID)) == 0x00) {
				shortLow(filename, file.sf.name);
				printf("%s\t", filename);
			} else if ((file.sf.attr & (ATTR_DIRECTORY | ATTR_VOLUME_ID)) == ATTR_DIRECTORY) {
				shortLow(filename, file.sf.name);
				printf("%s/\t", filename);
			} else if ((file.sf.attr & (ATTR_DIRECTORY | ATTR_VOLUME_ID)) == ATTR_VOLUME_ID) {
				continue;
			} else {
				// bad file
			}
		}
		current_clus = retFatNextClus(current_clus);
	} while (!chainEnd(current_clus) && !done);
	return 1;
}

int changeDirClus(union dirEntry *ptr) {
	unsigned int file_clus;
	if ((ptr->sf.attr & ATTR_DIRECTORY) == ATTR_DIRECTORY) {
		file_clus = retFileClus(ptr);
		if (file_clus == 0) {
			cur_dir_clus = img_info.root_clus;
		} else {
			cur_dir_clus = file_clus;
		}
	}
	return 1;
}

int changeCurDir(union dirEntry *ptr) {
	char filename[12];
	unsigned int pen_slash;
	if ((ptr->sf.attr & ATTR_DIRECTORY) == ATTR_DIRECTORY) {
		shortLow(filename, ptr->sf.name);
		if (retFileClus(ptr) == 0) {
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

// END DIRECTORY

// EXECUTE

/* userCmd
 *
 */
int userCmd(char **cmd_args) {
	if (cmd_args[0] == NULL) {
		// do nothing
	} else if (strcmp(cmd_args[0], "open") == 0) {
		openCmd(cmd_args);
	} else if (strcmp(cmd_args[0], "close") == 0) {
		closeCmd(cmd_args);
	} else if (strcmp(cmd_args[0], "create") == 0) {
		createCmd(cmd_args);
	} else if (strcmp(cmd_args[0], "rm") == 0) {
		rmCmd(cmd_args);
	} else if (strcmp(cmd_args[0], "size") == 0) {
		sizeCmd(cmd_args);
	} else if (strcmp(cmd_args[0], "cd") == 0) {
		cdCmd(cmd_args);
	} else if (strcmp(cmd_args[0], "ls") == 0) {
		lsCmd(cmd_args);
	} else if (strcmp(cmd_args[0], "mkdir") == 0) {
		mkdirCmd(cmd_args);
	} else if (strcmp(cmd_args[0], "rmdir") == 0) {
		rmdirCmd(cmd_args);
	} else if (strcmp(cmd_args[0], "read") == 0) {
		readCmd(cmd_args);
	} else if (strcmp(cmd_args[0], "write") == 0) {
		writeCmd(cmd_args);
	} else if (strcmp(cmd_args[0], "exit") == 0) {
		 return exitCmd();
	} else if (strcmp(cmd_args[0], "info") == 0) {
		infoCmd();
	}  else {
		printf("command: %s: Command is not recognized\n", cmd_args[0]);
	}
	return 1;
}

// END EXECUTE



// FILE COMMANDS




int infoCmd(){
	printf("Bytes per Sector: %d\n", img_info.bytes_per_sec);
	printf("Sectors per Cluster: %d\n", img_info.sec_per_clus);
	printf("Total Sectors: %d\n",img_info.tot_sec32);
	printf("Sectors per FAT: %d\n", img_info.fat_sz32);
	printf("Number of FATs: %d\n", img_info.num_fat);

	return 0;
}



int openCmd(char **cmd_args) {
	int check;
	union dirEntry file;
	unsigned int file_clus, dir_clus, offset;
	struct node *file_ptr;
	if (cmd_args[1] == NULL || cmd_args[2] == NULL) {
		error_specify_file_and_mode(cmd_args[0]);
	} else {
		check = lookupFile(cmd_args[1], cur_dir_clus, &file, &dir_clus, &offset);
		if (!check) {
			error_open_no_file(cmd_args[1]);
		} else if ((file.sf.attr & ATTR_DIRECTORY) == ATTR_DIRECTORY) {
			error_open_directory(cmd_args[1]);
		} else if (toByte(cmd_args[2]) == 0x0) {
			error_open_bad_param(cmd_args[2]);
		} else {
			file_clus = retFileClus(&file);
			file_ptr = opened_files->find(opened_files, file_clus);
			if (file_ptr != NULL) {
				error_open_already(cmd_args[1]);
			} else {
				file.sf.last_acc_date = retDate();
				setDirEntry(&file, dir_clus, offset);
				opened_files->add(opened_files, retFileClus(&file), cmd_args[2]);
			}
		}
	}
	return 0;
}

int closeCmd(char **cmd_args) {
	int check;
	union dirEntry file;
	unsigned int file_clus;
	struct node *file_ptr;
	if (cmd_args[1] == NULL) {
		error_specify_file(cmd_args[0]);
	} else {
		check = lookupFile(cmd_args[1], cur_dir_clus, &file, NULL, NULL);
		if (!check) {
			error_open_no_file(cmd_args[1]);
		} else if ((file.sf.attr & ATTR_DIRECTORY) == ATTR_DIRECTORY) {
			error_close_directory(cmd_args[1]);
		} else {
			file_clus = retFileClus(&file);
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

int createCmd(char **cmd_args) {
	int check;
	union dirEntry file;
	unsigned int dir_clus, offset;
	if (cmd_args[1] == NULL) {
		error_specify_file(cmd_args[0]);
	} else {
		check = lookupFile(cmd_args[1], cur_dir_clus, &file, &dir_clus, &offset);
		if (check) {
			error_file_or_directory_exists(cmd_args[1]);
		} else {
			makeFile(cmd_args[1], cur_dir_clus);
		}
	}
	return 0;
}

int rmCmd(char **cmd_args) {
	int check;
	char *file_name;
	union dirEntry file;
	unsigned int file_clus, dir_clus, offset;
	struct node *file_ptr;
	file_name = cmd_args[1];
	if (file_name == NULL) {
		error_specify_file(cmd_args[0]);                
	} else {
		check = lookupFile(cmd_args[1], cur_dir_clus, &file, &dir_clus, &offset);
		if (!check) {
			error_open_no_file(cmd_args[1]);
		} else if ((file.sf.attr & ATTR_DIRECTORY) == ATTR_DIRECTORY) {
			error_remove_directory(cmd_args[1]);
		} else {
			file_clus = retFileClus(&file);
			file_ptr = opened_files->find(opened_files, file_clus);
			if (file_ptr != NULL) {
				opened_files->remove(opened_files, file_clus);
			}
			fileDel(&file, dir_clus, offset);
		}
	}
	return 0;
}

int sizeCmd(char **cmd_args) {
	int check;
	union dirEntry file;
	if (cmd_args[1] == NULL) {
		error_specify_file(cmd_args[0]);
	} else {
		check = lookupFile(cmd_args[1], cur_dir_clus, &file, NULL, NULL);
		if (!check) {
			error_open_no_file(cmd_args[1]);
		} else {
			printf("%u\n", retSize(&file));
		}
	}
	return 0;
}

int readCmd(char **cmd_args) {
	int check;
	union dirEntry file;
	char *filename;
	unsigned int file_clus, position, size, file_size, dir_clus, offset;
	struct node *file_ptr;
	if (cmd_args[1] == NULL || cmd_args[2] == NULL || cmd_args[3] == NULL) {
		error_specify_file_pos_size(cmd_args[0]);
	} else {
		filename = cmd_args[1];
		position = strtoul(cmd_args[2], NULL, 10);
		size = strtoul(cmd_args[3], NULL, 10);
		check = lookupFile(filename, cur_dir_clus, &file, &dir_clus, &offset);
		if (!check) {
			error_open_no_file(filename);
		} else if ((file.sf.attr & ATTR_DIRECTORY) == ATTR_DIRECTORY) {
			error_close_directory(filename);
		} else {
			file_clus = retFileClus(&file);
			file_ptr = opened_files->find(opened_files, file_clus);
			file_size = retSize(&file);
			if (file_ptr == NULL) {
				error_not_open(filename);
			} else if (!readCheck(file_ptr)) {
				error_not_readable(filename);
			} else if (position + size > file_size) {
				error_beyond_EOF(position, size, file_size);
			} else {
				file.sf.last_acc_date = retTime();
				setDirEntry(&file, dir_clus, offset);
				fileR(&file, position, size);
				printf("\n");
			}
		}
	}
	return 0;
}

int writeCmd(char **cmd_args) {
	int check;
	union dirEntry file;
	char *filename, *str;
	unsigned int file_clus, position, size, dir_clus, offset, str_length, given_size;
	struct node *file_ptr;
	if (cmd_args[1] == NULL || cmd_args[2] == NULL || cmd_args[3] == NULL || cmd_args[4] == NULL) {
		error_specify_file_pos_size_str(cmd_args[0]);
	} else {
		filename = cmd_args[1];
		str = cmd_args[4];
		position = strtoul(cmd_args[2], NULL, 10);
		str_length = strlen(str);
		given_size = strtoul(cmd_args[3], NULL, 10);
		if (str_length > given_size) {
			size = given_size;
		} else {
			size = str_length;
		}
		check = lookupFile(filename, cur_dir_clus, &file, &dir_clus, &offset);
		if (!check) {
			error_open_no_file(filename);
		} else if ((file.sf.attr & ATTR_DIRECTORY) == ATTR_DIRECTORY) {
			error_close_directory(filename);
		} else {
			file_clus = retFileClus(&file);
			file_ptr = opened_files->find(opened_files, file_clus);
			if (file_ptr == NULL) {
				error_not_open(filename);
			} else if (!writeCheck(file_ptr)) {
				error_not_writeable(filename);
			} else if (position > UINT_MAX - size) {
				error_too_large(position, size);
			} else {
				fileW(&file, position, size, str);
				file.sf.last_acc_date = retTime();
				setDirEntry(&file, dir_clus, offset);
			}
		}
	}
	return 0;
}



int exitCmd(void) {
	printf("exiting fat32 utility\n");
	return 0;
}



// END FILE COMMANDS



// FILE TYPES




int retDirEntry(union dirEntry *ptr, unsigned int directory_clus, unsigned int entry_num) {
	unsigned int first_dir_clus, entry_first_byte_offset;
	first_dir_clus = retSecClus(directory_clus);
	if ((entry_first_byte_offset = 32*entry_num) >= img_info.bytes_per_sec*img_info.sec_per_clus) {
		// bad offset
		return 0;
	}
	rChar(ptr, entry_first_byte_offset + first_dir_clus*img_info.bytes_per_sec, sizeof(union dirEntry));
	return 1;
}

int setDirEntry(union dirEntry *ptr, unsigned int directory_clus, unsigned int entry_num) {
	unsigned int first_dir_clus, entry_first_byte_offset;
	first_dir_clus = retSecClus(directory_clus);
	if ((entry_first_byte_offset = 32*entry_num) >= img_info.bytes_per_sec*img_info.sec_per_clus) {
		// bad offset
		return 0;
	}
	wChar(ptr, entry_first_byte_offset + first_dir_clus*img_info.bytes_per_sec, sizeof(union dirEntry));
	return 1;
}

int retNextDirEntry(union dirEntry *ptr, unsigned int directory_clus, unsigned int entry_num) {
	unsigned int first_dir_clus, entry_first_byte_offset, next_num;
	next_num = entry_num + 1;
	if ((entry_first_byte_offset = 32*next_num) >= img_info.bytes_per_sec*img_info.sec_per_clus) {
		first_dir_clus = retSecClus(retFatNextClus(directory_clus));
		entry_first_byte_offset -= img_info.bytes_per_sec*img_info.sec_per_clus;
	} else {
		first_dir_clus = retSecClus(directory_clus);
	}
	rChar(ptr, entry_first_byte_offset + first_dir_clus*img_info.bytes_per_sec, sizeof(union dirEntry));
	return 1;
}


// implementation of list and open file table
struct list *makeList(void) {
	struct list *m_list;
	m_list = calloc(1, sizeof(struct list));
	m_list->clear = &clearList;
	m_list->add = &addList;
	m_list->remove = &remList;
	m_list->find = &lookList;
	m_list->get_head = &headList;
	m_list->empty = &emptyList;
	m_list->head = NULL;
	m_list->size = 0;
	return m_list;
}

// function for free'ing all the memory
void delList(struct list *old_list) {
	old_list->clear(old_list);
	free(old_list);
}

// frees each entry in the list
void clearList(struct list *m_list) {
	struct node *ptr, *next_ptr;
	ptr = m_list->head;
	while (ptr != NULL) {
		next_ptr = ptr->next;
		free(ptr);
		ptr = next_ptr;
	}
	m_list->size = 0;
}

// adds the entry with the given file cluster and read/write mode
// will only add with a valid file mode
int addList(struct list *m_list, unsigned int file_clus, char *mode) {
	struct node *new_node;
	unsigned char flags;
	flags = toByte(mode);
	if ((m_list->find(m_list, file_clus) == NULL) && (flags != 0x0)) {
		new_node = calloc(1, sizeof(struct node));
		new_node->fst_file_clus = file_clus;
		new_node->flags = flags;
		new_node->next = m_list->head;
		m_list->head = new_node;
		++(m_list->size);
		return 1;
	}
	return 0;
}

// removes the given file cluster from the list
int remList(struct list *m_list, unsigned int file_clus) {
	struct node *ptr, *prev;
	ptr = m_list->head;
	while (ptr != NULL) {
		if (ptr->fst_file_clus == file_clus) {
			break;
		}
		prev = ptr;
		ptr = ptr->next;
	}
	if (ptr != NULL) {
		if (ptr == m_list->head) {
			m_list->head = ptr->next;
		} else {
			prev->next = ptr->next;
		}
		--(m_list->size);
		free(ptr);
		return 1;
	}
	return 0;
}

// returns the node pointer to the given file cluster or NULL if it's not there
struct node *lookList(struct list *m_list, unsigned int file_clus) {
	struct node *ptr;
	ptr = m_list->head;
	while (ptr != NULL) {
		if (ptr->fst_file_clus == file_clus) {
			break;
		}
		ptr = ptr->next;
	}
	return ptr;
}

// returns the head of the list (for stack-like behavior)
struct node *headList(struct list *m_list) {
	return m_list->head;
}

int emptyList(struct list *m_list) {
	return !m_list->size;
}

unsigned char toByte(char *mode) {
	if (strcmp(mode, "r") == 0 ) {
		return OPEN_READ;
	} else if (strcmp(mode, "w") == 0) {
		return OPEN_WRITE;
	} else if ((strcmp(mode, "rw") == 0) | (strcmp(mode, "wr") == 0)) {
		return OPEN_READ | OPEN_WRITE;
	} else {
		return OPEN_BAD;
	}
}

int readCheck(struct node *file) {
	if ((file->flags & OPEN_READ) == OPEN_READ) {
		return 1;
	} else {
		return 0;
	}
}

int writeCheck(struct node *file) {
	if ((file->flags & OPEN_WRITE) == OPEN_WRITE) {
		return 1;
	} else {
		return 0;
	}
}

unsigned int retSize(union dirEntry *file) {
	unsigned int size;
	size = file->sf.file_size;
	if (endianness) {
		size = switch32(size);
	}
	return size;
}

int fileR(union dirEntry *file, unsigned int position, unsigned int size) {
	unsigned int offset, bytes_left, cur_clus, bytes_per_clus, byte_position, nmemb;
	char *buffer;
	bytes_per_clus = img_info.bytes_per_sec*img_info.sec_per_clus;
	buffer = malloc(sizeof(char)*(bytes_per_clus));
	offset = position;
	bytes_left = size;
	cur_clus = retFileClus(file);
	while (offset > bytes_per_clus) {
		cur_clus = retFatNextClus(cur_clus);
		if (chainEnd(cur_clus)) {
			free(buffer);
			return 0;
		}
		offset -= bytes_per_clus;
	}
	byte_position = img_info.bytes_per_sec*retSecClus(cur_clus) + offset;
	if (bytes_left > bytes_per_clus - offset) {
		nmemb = bytes_per_clus - offset;
	} else {
		nmemb = bytes_left;
	}
	rChar(buffer, byte_position, nmemb);
	fwrite(buffer, sizeof(char), nmemb, stdout);
	bytes_left -= nmemb;
	while (bytes_left > 0) {
		cur_clus = retFatNextClus(cur_clus);
		if (chainEnd(cur_clus)) {
			free(buffer);
			return 0;
		}
		byte_position = img_info.bytes_per_sec*retSecClus(cur_clus);
		if (bytes_left < bytes_per_clus) {
			nmemb = bytes_left;
		} else {
			nmemb = bytes_per_clus;
		}
		rChar(buffer, byte_position, nmemb);
		fwrite(buffer, sizeof(char), nmemb, stdout);
		bytes_left -= nmemb;
	}
	return 1;
}

int fileW(union dirEntry *file, unsigned int position, unsigned int size, char *str) {
	unsigned int offset, bytes_left, cur_clus, bytes_per_clus, byte_position, nmemb, start;
	bytes_per_clus = img_info.bytes_per_sec*img_info.sec_per_clus;
	offset = position;
	bytes_left = size;
	start = 0;
	cur_clus = retFileClus(file);
	while (offset > bytes_per_clus) {
		cur_clus = retFatNextClus(cur_clus);
		if (chainEnd(cur_clus)) {
			expClus(cur_clus);
		}
		offset -= bytes_per_clus;
	}
	byte_position = img_info.bytes_per_sec*retSecClus(cur_clus) + offset;
	if (bytes_left > bytes_per_clus - offset) {
		nmemb = bytes_per_clus - offset;
	} else {
		nmemb = bytes_left;
	}
	wChar(&str[start], byte_position, nmemb);
	bytes_left -= nmemb;
	start += nmemb;
	while (bytes_left > 0) {
		cur_clus = retFatNextClus(cur_clus);
		if (chainEnd(cur_clus)) {
			expClus(cur_clus);
		}
		byte_position = img_info.bytes_per_sec*retSecClus(cur_clus);
		if (bytes_left < bytes_per_clus) {
			nmemb = bytes_left;
		} else {
			nmemb = bytes_per_clus;
		}
		wChar(&str[start], byte_position, nmemb);
		bytes_left -= nmemb;
		start += nmemb;
	}
	if (position + size > file->sf.file_size) {
		file->sf.file_size = position + size;
	}
	return 1;
}

int fileDel(union dirEntry *file_ptr, unsigned int directory_clus, unsigned int entry_num) {
	union dirEntry next_file;
	struct list *clusters;
	struct node *clus_node;
	unsigned int file_clus;
	clusters = makeList();
	file_clus = retFileClus(file_ptr);
	do {
		clusters->add(clusters, file_clus, "r");
		file_clus = retFatNextClus(file_clus);
	} while (!chainEnd(file_clus));
	
	while (!clusters->empty(clusters)) {
		clus_node = clusters->get_head(clusters);
		clustDel(clus_node->fst_file_clus);
		clusters->remove(clusters, clus_node->fst_file_clus);
	}

	retNextDirEntry(&next_file, directory_clus, entry_num);
	if (next_file.raw_bytes[0] == 0x00) {
		file_ptr->raw_bytes[0] = 0x00;
	} else {
		file_ptr->raw_bytes[0] = 0xE5;
	}
	delList(clusters);
	setDirEntry(file_ptr, directory_clus, entry_num);
	return 1;
}

int clustDel(unsigned int file_clus) {
	unsigned int true_value;
	true_value = retFatNextClus_true(file_clus);
	true_value &= 0xF0000000;
	changeFats(file_clus, true_value);
	return 1;
}

int makeDirEntry(char *file_name, unsigned int directory_clus, union dirEntry *file, unsigned int *clus_ptr, unsigned int *offset_ptr, int find_new_clus){
	unsigned int clus;
	char short_name[11];
	lookupOpenDirEntry(directory_clus, file, clus_ptr, offset_ptr);
	fileShort(file_name, short_name);
	strncpy(file->sf.name, short_name, 11);
	file->sf.crt_time = file->sf.wrt_time = retTime();
	file->sf.crt_date = file->sf.wrt_date = file->sf.last_acc_date = retDate();
	file->sf.file_size = 0;
	if (find_new_clus) {
		clus = lookupOpenClus();
		file->sf.first_clus_hi = retHiVal(clus);
		file->sf.first_clus_lo = retLoVal(clus);
		changeFats(clus, END_OF_CHAIN);
	}
	return 0;
}

int makeFile(char *file_name, unsigned int directory_clus) {
	unsigned int clus, offset;
	union dirEntry file;
	makeDirEntry(file_name, directory_clus, &file, &clus, &offset, 1);
	file.sf.attr = 0x00;
	setDirEntry(&file, clus, offset);
	return 1;
}

int makeDir(char *dir_name, unsigned int directory_clus) {
	unsigned int clus, offset, d_clus, d_offset, dd_clus, dd_offset, new_dir_clus;
	union dirEntry file, d_file, dd_file;
	makeDirEntry(dir_name, directory_clus, &file, &clus, &offset, 1);
	file.sf.attr = 0x10;
	setDirEntry(&file, clus, offset);
	new_dir_clus = retFileClus(&file);
	makeDirEntry(".", new_dir_clus, &d_file, &d_clus, &d_offset, 0);
	d_file.sf.first_clus_hi = retHiVal(new_dir_clus);
	d_file.sf.first_clus_lo = retLoVal(new_dir_clus);
	d_file.sf.attr = 0x10;
	setDirEntry(&d_file, d_clus, d_offset);
	makeDirEntry("..", new_dir_clus, &dd_file, &dd_clus, &dd_offset, 0);
	if (directory_clus == img_info.root_clus) {
		dd_file.sf.first_clus_hi = 0x0000;
		dd_file.sf.first_clus_lo = 0x0000;
	} else {
		dd_file.sf.first_clus_hi = retHiVal(directory_clus);
		dd_file.sf.first_clus_lo = retLoVal(directory_clus);
	}
	dd_file.sf.attr = 0x10;
	setDirEntry(&dd_file, dd_clus, dd_offset);
	return 1;
}



// END FILE TYPES

// PARSE

/* readIn
 *
 * Dynamically allocates memory for a c-string, reads stdin and places the input
 * into the c-string. Returns the c-string.
 */
char *readIn() {
	char *cmd_line = calloc(INPUT_BUFFER_SIZE, sizeof(char));
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
	line = parseWhite(line);
	args = parseArgs(line);
	return args;
}

/* parseWhite
 *
 * Takes a c-string, cmd_line, and removes unnecessary whitespace and adds
 * whitespace where needed.
 *
 * special characters: |, <, >, &, $, ~
 * don't do this for: ., /
 * whitespace: <space>, \t, \v, \f, \r (\n is used to delimit commands)
 */
char *parseWhite(char *cmd_line) {
	remLeadWhite(cmd_line);
	remMidWhite(cmd_line);
	//addMidWhite(cmd_line);
	remTrailWhite(cmd_line);
	return cmd_line;
}

/* parseArgs
 *
 * Takes a c-string, cmd_line, and splits it into tokens delimited by space. An
 * array of strings is returned. The size of the array of strings is governed by
 * the constant PARAM_LIMIT. The array and it's elements use allocated memory
 * and should be freed later. The last element is followed by a NULL pointer to
 * mark the end of the array.
 */
char **parseArgs(char *cmd_line) {
	size_t arg_amount;
	char *tmp_line;
	int i = 0;
	int offset, position;
	char **cmd_args;
	arg_amount = 0;
	arg_amount = trackArgs(cmd_line);
	// arg_amount + 1 for the null value at the end
	cmd_args = calloc(arg_amount + 1, sizeof(char *));
	tmp_line = strndup(cmd_line, strlen(cmd_line) + 1);
	position = 0;
	offset = strcspn(&tmp_line[position], " ");
	while (offset != 0) {
		cmd_args[i] = (char *)malloc(offset + 1);
		strncpy(cmd_args[i], &tmp_line[position], offset);
		cmd_args[i++][offset] = '\0';
		if (tmp_line[position + offset] == '\0') {
			break;
		}
		if (tmp_line[position + offset + 1] == '\"') {
			position += offset + 2;
			offset = strcspn(&tmp_line[position], "\"");
			cmd_args[i] = (char *)malloc(offset + 1);
			strncpy(cmd_args[i], &tmp_line[position], offset);
			cmd_args[i++][offset] = '\0';
			++offset;
		}
		position += offset + 1;
		if (tmp_line[position - 1] == '\0') {
			break;
		}
		offset = strcspn(&tmp_line[position], " ");
	}
	/*token = strtok_r(tmp_line, sep, &save_ptr);
	while (token != NULL) {
		cmd_args[i] = (char *) malloc(strlen(token) + 1);
		strcpy(cmd_args[i++], token);
		token = strtok_r(NULL, sep, &save_ptr);
	}*/
	cmd_args[i] = NULL;
	free(tmp_line);
	return cmd_args;
}

/* trackArgs
 *
 * This take a c-string and counts the number of arguments which corresponds to
 * the number of spaces plus one.
 */
size_t trackArgs(char *cmd_line) {
	int i, in_quotes;
	size_t count;
	in_quotes = 0;
	count = 0;
	for (i = 0; cmd_line[i] != '\0'; ++i) {
		if (cmd_line[i] == ' ' && !in_quotes) {
			++count;
		} else if (cmd_line[i] == '\"' && !in_quotes) {
			++count;
			in_quotes = 1;
		} else if (cmd_line[i] == '\"' && in_quotes) {
			in_quotes = 0;
		}
	}
	return count + 1;
}

/* remLeadWhite
 *
 * Takes the input c-string and loops through string. The leading whitespace is
 * ignored while the rest of the string is shifted to the front.
 */
char *remLeadWhite(char *cmd_line){
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

/* remMidWhite
 *
 * Takes c-string and shifts the characters so that there is at most one space
 * character between each token separated by whitespace. E.g.
 * 
 * "file1   \t \v    file2" ==> "file1 file2"
 * 
 * A null byte is added to the end of the string -- doesn't worry about the 
 * whitespace at the end.
 */
char *remMidWhite(char *cmd_line){
	// copy holds place where char is modified, ptr holds place where checking
	char *copy = cmd_line, *ptr = cmd_line;
	int in_quotes = 0;
	int ws_count = 0;
	while (*ptr != 0) {
		if (*ptr == '"'){
			*copy = *ptr;
			copy++;
			if (in_quotes == 0){
				if (checkLastQuote(ptr)){
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

int checkLastQuote(char* ptr){
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

/* addMidWhite
 *
 * Takes c-string and adds space character between tokens which may not have any
 * whitespace initially. Special characters include <, >, |, &. Also in
 * consideration is >>.
 */
char *addMidWhite(char *cmd_line){
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
	if (!trackCmdSize(copy)) {
		// error stuff goes here
	}
	strcpy(cmd_line, copy);
	return cmd_line;
}

/* remTrailWhite
 *
 * Takes c-string and adds the a null byte after the last non-whitespace char.
 */
char *remTrailWhite(char *cmd_line){
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

/* trackCmdSize
 *
 * Makes sure the given string is shorter than the maximum allowed size of the
 * command string. Returns 1 if the string is good and 0 if the string is bad
 */
int trackCmdSize(char *cmd_line) {
	int i;
	for (i = 0; i < INPUT_BUFFER_SIZE; ++i) {
		if (cmd_line[i] == '\0') {
			return 1;
		}
	}
	return 0;
}

/* ridCmdLine
 * 
 * Takes a c-string and frees the memory allocated for that
 */
void ridCmdLine(char *cmd_line) {
	free(cmd_line);
}

/* ridCmdArgs
 *
 * Takes array of c-strings and frees each element and then frees the memory of
 * the array
 */
void ridCmdArgs(char **cmd_args) {
	int i;
	for (i = 0; cmd_args[i] != NULL; ++i) {
		free(cmd_args[i]);
		cmd_args[i] = NULL;
	}
	free(cmd_args);
}



// END PARSE

// SETUP

// global variables



/* setup function which initializes all the global variables and opens the image
 * in read and write mode.
 */
int setup(char *img_filename, char *exe_name) {
	if ((fat32_img = fopen(img_filename, "r+")) == NULL) {
		perror(NULL);
		return 0;
	}
	endianness = check_endian();
	getFatInfo();
	setRootDir();
	setOpened();
	displayIntro(img_filename, exe_name);

	return 1;
}

/* extracts the vital information for traversing the fat32 image from the boot
 * sector
 */
int getFatInfo(void) {
	readUnSh(&img_info.bytes_per_sec, 11);
	readUnCh(&img_info.sec_per_clus, 13);
	readUnSh(&img_info.rsvd_sec_cnt, 14);
	readUnCh(&img_info.num_fat, 16);
	readUnSh(&img_info.root_ent_cnt, 17);
	readUnInt(&img_info.tot_sec32, 32);
	readUnInt(&img_info.fat_sz32, 36);
	readUnSh(&img_info.ext_flags, 40);
	readUnInt(&img_info.root_clus, 44);
	return 0;
}

/* sets the first root directory sector, current directory sector and names, and
 * the first data sector
 */
int setRootDir(void) {
	unsigned int data_sec;
	data_sec = img_info.tot_sec32 - (img_info.rsvd_sec_cnt + (img_info.num_fat*img_info.fat_sz32));
	count_of_clusters = data_sec/img_info.sec_per_clus;
	first_data_sec = img_info.rsvd_sec_cnt + (img_info.num_fat*img_info.fat_sz32);
	first_root_sec = retSecClus(img_info.root_clus);
	cur_dir_clus = img_info.root_clus;
	cur_dir_sec = first_root_sec;
	current_directory_capacity = INIT_CUR_DIR_CAP;
	current_directory = calloc(current_directory_capacity, sizeof(char));
	strcat(current_directory, "/");
	return 1;
}

int setOpened(void) {
	opened_files = makeList();
	return 1;
}

/* prints the prompt */
void displayPrompt(void) {
	printf("%s] ", current_directory);
}

/* prints the intro message */
void displayIntro(char *img_filename, char *exe_name) {
	printf("Welcome to the %s shell utility\n", exe_name);
	printf("Image, %s, is ready to view\n", img_filename);
	printf("For a list of commands, type \"help\" or \"h\"\n");
}


// END SETUP

// ERROR

//parsing errors
void error_dangling_quote(){
	printf("Error: closing quotation missing!\n");
}


// open/close errors
void error_open_already(char *filename) {
	printf("Error: %s: file already open!\n", filename);
}

void error_open_no_file(char *filename) {
	printf("Error: %s: file does not exist\n", filename);
}

void error_open_directory(char *directory) {
	printf("Error: %s: cannot open directory\n", directory);
}

void error_open_bad_param(char *param) {
	printf("Error: %s: incorrect parameter\n", param);
}

void error_close_directory(char *directory) {
	printf("Error: %s: cannot close directory\n", directory);
}

void error_not_open(char *filename) {
	printf("Error: %s: file not open\n", filename);
}


// create errors
void error_used_file(char *file_name) {
	printf("Error: %s: file already exists\n", file_name);
}

void error_specify_file_and_mode(char *command){
	printf("Error: %s: please specify a file name and mode\n", command);
} 

void error_specify_file(char *command){
	printf("Error: %s: please specify a file name\n", command);
} 

void error_specify_directory(char *command){
	printf("Error: %s: please specify a directory name\n", command);
} 

void error_no_more_space(){
	printf("Error: No more space available on the image!\n");
}


// ls, cd errors
void error_cd_file(char *filename) {
	printf("Error: %s: not a directory\n", filename);
}

void error_cd_not_here(char *directory) {
	printf("Error: %s: does not exist\n", directory);
}


// read/write errors
void error_specify_file_pos_size(char *command) {
	printf("Error: %s: please specify a file name, position, and size\n", command);
}

void error_specify_file_pos_size_str(char *command) {
	printf("Error: %s: please specify a file name, position, size, and string\n", command);
}

void error_not_readable(char *filename) {
	printf("Error: %s: this file is not open in read mode\n", filename);
}

void error_beyond_EOF(unsigned int position, unsigned int size, unsigned int file_size) {
	printf("Error: %u + %u > %u: attempt to read beyond EOF\n", position, size, file_size);
}

void error_not_writeable(char *filename) {
	printf("Error: %s: File is not open for writing\n", filename);
}

void error_too_large(unsigned int position, unsigned int size) {
	printf("Error: %u + %u > %u: attempt to make file too large\n", position, size, UINT_MAX);
}


// other
void error_bad_directory(char *filename) {
	printf("Error: %s: not a directory\n", filename);
}

void error_not_empty(char *directory) {
	printf("Error: %s: directory not empty\n", directory);
}

void error_file_or_directory_exists(char *filename) {
	printf("Error: %s: file or directory exists already\n", filename);
}

void error_remove_directory(char *directory) {
	printf("Error: %s: not a file\n", directory);
}



// END ERROR

// TOOLS

// global variables used in this file





// reads nmemb amount of characters from fat32_img file at pos
int rChar(void *ptr, long pos, size_t nmemb) {
	long offset = pos - ftell(fat32_img);
	if (fseek(fat32_img, offset, SEEK_CUR) == -1) {
		perror(NULL);
		return 0;
	}
	if (fread(ptr, sizeof(char), nmemb, fat32_img) != nmemb) {
		perror(NULL);
		return 0;
	}
	return 1;
}

// writes nmemb amount of characters from fat32_img file at pos
int wChar(void *ptr, long pos, size_t nmemb) {
	long offset = pos - ftell(fat32_img);
	if (fseek(fat32_img, offset, SEEK_CUR) == -1) {
		perror(NULL);
		return 0;
	}
	if (fwrite(ptr, sizeof(char), nmemb, fat32_img) != nmemb) {
		perror(NULL);
		return 0;
	}
	return 1;
}

// reads an unsigned int from fat32_img at pos
unsigned int *readUnInt(unsigned int *ptr, long pos) {
	long offset = pos - ftell(fat32_img);
	if (fseek(fat32_img, offset, SEEK_CUR) == -1) {
		perror(NULL);
		return NULL;
	}
	if (fread(ptr, sizeof(unsigned int), 1, fat32_img) != 1) {
		perror(NULL);
		return NULL;
	}
	if (endianness) {
		*ptr = switch32(*ptr);
	}
	return ptr;
}

// writes an unsigned int to fat32_img at pos
unsigned int *writeUnInt(unsigned int *ptr, long pos) {
	long offset;
	unsigned int copy;
	offset = pos - ftell(fat32_img);
	if (endianness) {
		copy = switch32(*ptr);
	} else {
		copy = *ptr;
	}
	if (fseek(fat32_img, offset, SEEK_CUR) == -1) {
		perror(NULL);
		return NULL;
	}
	if (fwrite(&copy, sizeof(unsigned int), 1, fat32_img) != 1) {
		perror(NULL);
		return NULL;
	}
	return ptr;
}

// reads an unsigned short from fat32_img at pos
unsigned short *readUnSh(unsigned short *ptr, long pos) {
	long offset = pos - ftell(fat32_img);
	if (fseek(fat32_img, offset, SEEK_CUR) == -1) {
		perror(NULL);
		return NULL;
	}
	if (fread(ptr, sizeof(unsigned short), 1, fat32_img) != 1) {
		perror(NULL);
		return NULL;
	}
	if (endianness) {
		*ptr = switch16(*ptr);
	}
	return ptr;
}

// writes an unsigned short to fat32_img at pos
unsigned short *writeUnSh(unsigned short *ptr, long pos) {
	long offset;
	unsigned short copy;
	offset = pos - ftell(fat32_img);
	if (endianness) {
		copy = switch16(*ptr);
	} else {
		copy = *ptr;
	}
	if (fseek(fat32_img, offset, SEEK_CUR) == -1) {
		perror(NULL);
		return NULL;
	}
	if (fwrite(&copy, sizeof(unsigned short), 1, fat32_img) != 1) {
		perror(NULL);
		return NULL;
	}
	return ptr;
}

// reads an unsigned char from fat32_img at pos
unsigned char *readUnCh(unsigned char *ptr, long pos) {
	long offset = pos - ftell(fat32_img);
	if (fseek(fat32_img, offset, SEEK_CUR) == -1) {
		perror(NULL);
		return NULL;
	}
	if (fread(ptr, sizeof(char), 1, fat32_img) != 1) {
		perror(NULL);
		return NULL;
	}
	return ptr;
}

// writes an unsigned char to fat32_img at pos
unsigned int switch32(unsigned int val) {
	return ((val>>24)&0xff) | ((val<<8)&0xff0000) | ((val>>8)&0xff00) | ((val<<24)&0xff000000);
}

unsigned short switch16(unsigned short val) {
	return (val<<8) | (val>>8);
}

int check_endian(void) {
	int num = 1;
	if(*(char *)&num == 1) {
		return 0; // little endian
	} else {
		return 1; // big endian
	}
}

// follows the formula from the FAT32 specifications
unsigned int retSecClus(unsigned int clus) {
	return (clus - 2)*img_info.sec_per_clus + first_data_sec;
}

/* returns the absolute position of the given cluster in the given FAT in terms
 * of bytes
 */
unsigned long retFatClusPos(unsigned int clus, unsigned int fat) {
	unsigned int i, fat_start_sec;
	i = fat < img_info.num_fat ? fat : 0;
	fat_start_sec = img_info.rsvd_sec_cnt + i*img_info.fat_sz32;
	return fat_start_sec*img_info.bytes_per_sec + 4*clus;
}

/* returns the next cluster in the chain
 */
unsigned int retFatNextClus(unsigned int clus) {
	unsigned long position;
	unsigned int next_clus;
	position = retFatClusPos(clus, 0);
	readUnInt(&next_clus, position);
	return next_clus & NEXT_CLUS_MASK;
}

// same as above but doesn't apply the NEXT_CLUS_MASK to result
unsigned int retFatNextClus_true(unsigned int clus) {
	unsigned long position;
	unsigned int next_clus;
	position = retFatClusPos(clus, 0);
	readUnInt(&next_clus, position);
	return next_clus;
}

// checks if the value is one of the end of chain values
int chainEnd(unsigned int clus) {
	if ((clus & END_OF_CHAIN) == END_OF_CHAIN) {
		return 1;
	} else {
		return 0;
	}
}

// changes the values in all file allocation tables at file_clus to "value"
int changeFats(unsigned int file_clus, unsigned int value) {
	unsigned long position;
	int i;
	for (i = 0; i < img_info.num_fat; ++i) {
		position = retFatClusPos(file_clus, i);
		writeUnInt(&value, position);
	}
	return 1;
}

// takes a short name and stores transformed name to filename
int shortLow(char filename[12], char short_name[11]) {
	int i, j;
	for (i = 0, j = 0; i < 11; ++i) {
		if (short_name[i] != ' ') {
			if (i == 8) {
				filename[j++] = '.';
			}
			filename[j++] = tolower(short_name[i]);
		}
	}
	for (j = j; j < 12; ++j) {
		filename[j] = '\0';
	}
	return 1;
}

// takes filename and stores "short name transformed" string in short_name
int fileShort(char filename[12], char short_name[11]) {
	int i, j;
	if (strcmp(filename, ".") == 0) {
		strcpy(short_name, ".          ");
		return 1;
	} else if (strcmp(filename, "..") == 0) {
		strcpy(short_name, "..         ");
		return 1;
	}
	i = 0, j = 0;
	while (i < 8 && filename[j] != '.' && filename[j] !='\0') {
		short_name[i++] = toupper(filename[j++]);
	}
	for (i = i; i < 8; i++) {
		short_name[i] = ' ';
	}
	if (filename[j++] == '.') {
		while (i < 11 && filename != '\0') {
			short_name[i++] = toupper(filename[j++]);
		}
	}
	for (i = i; i < 11; ++i) {
		short_name[i] = ' ';
	}
	return 1;
}

// finds the file in the given directory entry with name "filename". returns 1
// if a file is found and 0 if no file is found
int lookupFile(char *filename, unsigned int directory_clus, union dirEntry *ptr, unsigned int *clus_ptr, unsigned int *offset_ptr) {
	unsigned int current_clus, i, limit, done;
	union dirEntry file;
	char short_name[11];
	fileShort(filename, short_name);
	current_clus = directory_clus;
	limit = img_info.bytes_per_sec*img_info.sec_per_clus/32;
	done = 0;
	do {
		for (i = 0; i < limit; ++i) {
			retDirEntry(&file, current_clus, i);
			if (file.raw_bytes[0] == 0x00) {
				done = 1;
				break;
			} else if (file.raw_bytes[0] == 0xE5) {
				continue;
			} else if ((file.lf.attr & ATTR_LONG_NAME_MASK) == ATTR_LONG_NAME) {
				continue;
			} else if ((file.sf.attr & (ATTR_DIRECTORY | ATTR_VOLUME_ID)) == ATTR_VOLUME_ID) {
				continue;
			} else if ((strncmp(file.sf.name, short_name, 11) == 0)) {
				*ptr = file;
				if (clus_ptr != NULL) {
					*clus_ptr = current_clus;
				}
				if (offset_ptr != NULL) {
					*offset_ptr = i;
				}
				return 1;
			}
		}
		current_clus = retFatNextClus(current_clus);
	} while (!chainEnd(current_clus) && !done);
	return 0;
}

// gets the first file cluster in the given directory entry
unsigned int retFileClus(union dirEntry *ptr) {
	unsigned int file_clus;
	unsigned short hi, lo;
	hi = ptr->sf.first_clus_hi;
	lo = ptr->sf.first_clus_lo;
	if (endianness) {
		hi = switch16(hi);
		lo = switch16(lo);
	}
	file_clus = (hi << 16) | (lo & 0xFFFF);
	return file_clus;
}

// checks if the directory is empty
int emptyDir(union dirEntry *dir) {
	unsigned int current_clus, i, j, limit, done;
	union dirEntry file;
	current_clus = retFileClus(dir);
	limit = img_info.bytes_per_sec*img_info.sec_per_clus/32;
	done = 0;
	j = 0;
	do {
		for (i = 0; i < limit; ++i, ++j) {
			retDirEntry(&file, current_clus, i);
			if (j == 0 || j == 1) {
				continue;
			} else if ((file.lf.attr & ATTR_LONG_NAME_MASK) == ATTR_LONG_NAME) {
				continue;
			} else if (file.raw_bytes[0] == 0xE5) {
				continue;
			} else if (file.raw_bytes[0] == 0x00) {
				return 1;
			} else {
				return 0;
			}
		}
		current_clus = retFatNextClus(current_clus);
	} while (!chainEnd(current_clus) && !done);
	return 1;
}

// returns the hi value of the given cluster
unsigned short retHiVal(unsigned int clus) {
	unsigned short hi;
	hi = (unsigned short)clus >> 16;
	if (endianness) {
		hi = switch16(hi);
	}
	return hi;
}

// returns the lo value of the given cluster
unsigned short retLoVal(unsigned int clus) {
	unsigned short lo;
	lo = (unsigned short)clus & 0xFFFF;
	if (endianness) {
		lo = switch16(lo);
	}
	return lo;
}

// returns the time in the FAT32 format
unsigned short retTime(void){
	time_t rawtime;
	struct tm * cur_time;
	unsigned short t;

	time(&rawtime);
	cur_time = localtime(&rawtime);
	t = ((cur_time->tm_hour*0x0800)+(cur_time->tm_min*0x0020)+(cur_time->tm_sec/2));
	if (endianness) {
		t = switch16(t);
	}
	return t;
}

// returns the date in the FAT32 format
unsigned short retDate(void){
	time_t rawtime;
	struct tm * cur_time;
	unsigned short d;

	time(&rawtime);
	cur_time = localtime(&rawtime);
	d = (((cur_time->tm_year-80)*0x0200)+((cur_time->tm_mon+1)*0x0020)+(cur_time->tm_mday));
	if (endianness) {
		d = switch16(d);
	}
	return d;
}

// searches for the next open cluster in the FAT
unsigned int lookupOpenClus() {
	unsigned int value;	//cluster data
	unsigned int found = 1;	//free space found
	unsigned int i;			//counter
	for (i = 2; i < count_of_clusters; ++i) {
		value = retFatNextClus(i);
		if (value == 0){
			found = 0;
			break;
		}
	}
	if (found == 1){
		error_no_more_space();
		return 0;
	}
	return i;
}

// finds the next available directory entry
unsigned int lookupOpenDirEntry(unsigned int directory_clus, union dirEntry *ptr, unsigned int *clus_ptr, unsigned int *offset_ptr) {
	unsigned int current_clus, i, limit, found;
	union dirEntry file;
	current_clus = directory_clus;
	limit = img_info.bytes_per_sec*img_info.sec_per_clus/32;
	found = 0;
	do {
		for (i = 0; i < limit; ++i) {
			retDirEntry(&file, current_clus, i);
			if (file.raw_bytes[0] == 0x00) {
				setEntryNull(current_clus, i);
				*ptr = file;
				found = 1;
				break;
			} else if (file.raw_bytes[0] == 0xE5) {
				*ptr = file;
				found = 1;
				break;
			} else if ((file.lf.attr & ATTR_LONG_NAME_MASK) == ATTR_LONG_NAME) {
				continue;
			} else if ((file.sf.attr & (ATTR_DIRECTORY | ATTR_VOLUME_ID)) == ATTR_VOLUME_ID) {
				continue;
			}
		}
		if (found) {
			if (clus_ptr != NULL) {
				*clus_ptr = current_clus;
			}
			if (offset_ptr != NULL) {
				*offset_ptr = i;
			}
			return 1;
		}
		current_clus = retFatNextClus(current_clus);
	} while (!chainEnd(current_clus));
	return 0;
}

// sets the next directory entry to 0x00. Expands the directory if necessary
int setEntryNull(unsigned int directory_clus, unsigned int entry_num) {
	union dirEntry file;
	unsigned int first_dir_clus, entry_first_byte_offset, next_num;
	next_num = entry_num + 1;
	if ((entry_first_byte_offset = 32*next_num) >= img_info.bytes_per_sec*img_info.sec_per_clus) {
		if (chainEnd(retFatNextClus(directory_clus))) {
			expClus(directory_clus);
		}
		first_dir_clus = retSecClus(retFatNextClus(directory_clus));
		entry_first_byte_offset -= img_info.bytes_per_sec*img_info.sec_per_clus;
	} else {
		first_dir_clus = retSecClus(directory_clus);
	}
	rChar(&file, entry_first_byte_offset + first_dir_clus*img_info.bytes_per_sec, sizeof(union dirEntry));
	file.raw_bytes[0] = 0x00;
	wChar(&file, entry_first_byte_offset + first_dir_clus*img_info.bytes_per_sec, sizeof(union dirEntry));
	return 1;
}

// function for making room in the file
int expClus(unsigned int old_clus) {
	unsigned int new_clus;
	new_clus = lookupOpenClus();
	if (new_clus != 0) {
		changeFats(old_clus, new_clus);
		changeFats(new_clus, END_OF_CHAIN);
	}
	return 1;
}


// END TOOLS
