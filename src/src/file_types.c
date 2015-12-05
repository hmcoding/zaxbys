#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tools.h"
#include "file_types.h"
#include "setup.h"


struct fat32_info img_info;

// grabs the 32 bytes for the file at (directory_clus, entry_num) and place it
// where ptr points to
int get_directory_entry(union directory_entry *ptr, unsigned int directory_clus, unsigned int entry_num) {
	unsigned int first_dir_clus, entry_first_byte_offset;
	first_dir_clus = get_first_sector_of_cluster(directory_clus);
	if ((entry_first_byte_offset = 32*entry_num) >= img_info.bytes_per_sec*img_info.sec_per_clus) {
		// bad offset
		return 0;
	}
	read_chars(ptr, entry_first_byte_offset + first_dir_clus*img_info.bytes_per_sec, sizeof(union directory_entry));
	return 1;
}

int set_directory_entry(union directory_entry *ptr, unsigned int directory_clus, unsigned int entry_num) {
	unsigned int first_dir_clus, entry_first_byte_offset;
	first_dir_clus = get_first_sector_of_cluster(directory_clus);
	if ((entry_first_byte_offset = 32*entry_num) >= img_info.bytes_per_sec*img_info.sec_per_clus) {
		// bad offset
		return 0;
	}
	write_chars(ptr, entry_first_byte_offset + first_dir_clus*img_info.bytes_per_sec, sizeof(union directory_entry));
	return 1;
}

int get_next_directory_entry(union directory_entry *ptr, unsigned int directory_clus, unsigned int entry_num) {
	unsigned int first_dir_clus, entry_first_byte_offset, next_num;
	next_num = entry_num + 1;
	if ((entry_first_byte_offset = 32*next_num) >= img_info.bytes_per_sec*img_info.sec_per_clus) {
		first_dir_clus = get_first_sector_of_cluster(get_next_cluster_in_fat(directory_clus));
		entry_first_byte_offset -= img_info.bytes_per_sec*img_info.sec_per_clus;
	} else {
		first_dir_clus = get_first_sector_of_cluster(directory_clus);
	}
	read_chars(ptr, entry_first_byte_offset + first_dir_clus*img_info.bytes_per_sec, sizeof(union directory_entry));
	return 1;
}


// implementation of list and open file table
struct list *create_list(void) {
	struct list *m_list;
	m_list = calloc(1, sizeof(struct list));
	m_list->clear = &list_clear;
	m_list->add = &list_add;
	m_list->remove = &list_remove;
	m_list->find = &list_find;
	m_list->get_head = &list_get_head;
	m_list->empty = &list_empty;
	m_list->head = NULL;
	m_list->size = 0;
	return m_list;
}

// function for free'ing all the memory
void delete_list(struct list *old_list) {
	old_list->clear(old_list);
	free(old_list);
}

// frees each entry in the list
void list_clear(struct list *m_list) {
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
int list_add(struct list *m_list, unsigned int file_clus, char *mode) {
	struct node *new_node;
	unsigned char flags;
	flags = file_mode_to_byte(mode);
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
int list_remove(struct list *m_list, unsigned int file_clus) {
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
struct node *list_find(struct list *m_list, unsigned int file_clus) {
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
struct node *list_get_head(struct list *m_list) {
	return m_list->head;
}

int list_empty(struct list *m_list) {
	return !m_list->size;
}

unsigned char file_mode_to_byte(char *mode) {
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

int check_read(struct node *file) {
	if ((file->flags & OPEN_READ) == OPEN_READ) {
		return 1;
	} else {
		return 0;
	}
}

int check_write(struct node *file) {
	if ((file->flags & OPEN_WRITE) == OPEN_WRITE) {
		return 1;
	} else {
		return 0;
	}
}

unsigned int get_size(union directory_entry *file) {
	unsigned int size;
	size = file->sf.file_size;
	if (endianness) {
		size = swap_32(size);
	}
	return size;
}

int read_file(union directory_entry *file, unsigned int position, unsigned int size) {
	unsigned int offset, bytes_left, cur_clus, bytes_per_clus, byte_position, nmemb;
	char *buffer;
	bytes_per_clus = img_info.bytes_per_sec*img_info.sec_per_clus;
	buffer = malloc(sizeof(char)*(bytes_per_clus));
	offset = position;
	bytes_left = size;
	cur_clus = get_file_cluster(file);
	while (offset > bytes_per_clus) {
		cur_clus = get_next_cluster_in_fat(cur_clus);
		if (end_of_chain(cur_clus)) {
			free(buffer);
			return 0;
		}
		offset -= bytes_per_clus;
	}
	byte_position = img_info.bytes_per_sec*get_first_sector_of_cluster(cur_clus) + offset;
	if (bytes_left > bytes_per_clus - offset) {
		nmemb = bytes_per_clus - offset;
	} else {
		nmemb = bytes_left;
	}
	read_chars(buffer, byte_position, nmemb);
	fwrite(buffer, sizeof(char), nmemb, stdout);
	bytes_left -= nmemb;
	while (bytes_left > 0) {
		cur_clus = get_next_cluster_in_fat(cur_clus);
		if (end_of_chain(cur_clus)) {
			free(buffer);
			return 0;
		}
		byte_position = img_info.bytes_per_sec*get_first_sector_of_cluster(cur_clus);
		if (bytes_left < bytes_per_clus) {
			nmemb = bytes_left;
		} else {
			nmemb = bytes_per_clus;
		}
		read_chars(buffer, byte_position, nmemb);
		fwrite(buffer, sizeof(char), nmemb, stdout);
		bytes_left -= nmemb;
	}
	return 1;
}

int delete_file(union directory_entry *file_ptr, unsigned int directory_clus, unsigned int entry_num) {
	union directory_entry next_file;
	struct list *clusters;
	struct node *clus_node;
	unsigned int file_clus;
	clusters = create_list();
	file_clus = get_file_cluster(file_ptr);
	do {
		clusters->add(clusters, file_clus, "r");
		file_clus = get_next_cluster_in_fat(file_clus);
	} while (!end_of_chain(file_clus));

	while (!clusters->empty(clusters)) {
		clus_node = clusters->get_head(clusters);
		delete_cluster(clus_node->fst_file_clus);
		clusters->remove(clusters, clus_node->fst_file_clus);
	}

	get_next_directory_entry(&next_file, directory_clus, entry_num);
	if (next_file.raw_bytes[0] == 0x00) {
		file_ptr->raw_bytes[0] = 0x00;
	} else {
		file_ptr->raw_bytes[0] = 0xE5;
	}
	set_directory_entry(file_ptr, directory_clus, entry_num);
	return 1;
}

int delete_cluster(unsigned int file_clus) {
	unsigned int true_value;
	true_value = get_next_cluster_in_fat_true(file_clus);
	true_value &= 0xF0000000;
	modify_all_fats(file_clus, true_value);
	return 1;
}

int create_directory_entry(char *file_name, unsigned int directory_clus, union directory_entry *file, unsigned int *clus_ptr, unsigned int *offset_ptr){
	unsigned int clus;
	char short_name[11];
	find_open_directory_entry(directory_clus, file, clus_ptr, offset_ptr);
	filename_to_short(file_name, short_name);
	/*for (i=0; i < 11; ++i) {
		if (file_name[i] == '\0') {
			break;
		} else if(file_name[i] == '.') {
			while(i < 8)
				file.sf.name[i++] = ' ';
			i = 7;
			period_found = 1;
		} else if((i > 7) && (period_found == 0)) {
		} else {
			file.sf.name[i] = toupper(file_name[i]);
		}
	}*/
	strncpy(file->sf.name, short_name, 11);
	file->sf.crt_time = file->sf.wrt_time = get_time();
	file->sf.crt_date = file->sf.wrt_date = file->sf.last_acc_date = get_date();
	clus = find_open_cluster();
	file->sf.first_clus_hi = get_hi(clus);
	file->sf.first_clus_lo = get_lo(clus);
	file->sf.file_size = 0;
	modify_all_fats(clus, END_OF_CHAIN);
	return 0;
}

int create_file(char *file_name, unsigned int directory_clus) {
	unsigned int clus, offset;
	union directory_entry file;
	create_directory_entry(file_name, directory_clus, &file, &clus, &offset);
	file.sf.attr = 0x00;
	set_directory_entry(&file, clus, offset);
	return 1;
}

int create_directory(char *dir_name, unsigned int directory_clus) {
	return 1;
}
