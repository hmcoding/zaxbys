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


// implementation of list and open file table
struct list *create_list(void) {
	struct list *m_list;
	m_list = calloc(1, sizeof(struct list));
	m_list->clear = &list_clear;
	m_list->add = &list_add;
	m_list->remove = &list_remove;
	m_list->find = &list_find;
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
}

// adds the entry with the given file cluster and read/write mode
// will only add with a valid file mode
int list_add(struct list *m_list, unsigned int file_clus, char *mode) {
	struct node *new_node;
	unsigned char flags;
	flags = file_mode_to_byte(mode);
	if ((m_list->find(m_list, file_clus) != NULL) && (flags != 0)) {
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
		prev->next = ptr->next;
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

unsigned char file_mode_to_byte(char *mode) {
	if (strcmp(mode, "r") == 0 ) {
		return 0x1;
	} else if (strcmp(mode, "w") == 0) {
		return 0x2;
	} else if ((strcmp(mode, "rw") == 0) | (strcmp(mode, "wr") == 0)) {
		return 0x3;
	} else {
		return 0x0;
	}
}
