#include <stdio.h>
#include <stdlib.h>

#include "tools.h"
#include "file_types.h"
#include "setup.h"


struct fat32_info img_info;


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

void delete_list(struct list *old_list) {
	old_list->clear(old_list);
	free(old_list);
}

void list_clear(struct list *m_list) {
	struct node *ptr, *next_ptr;
	ptr = m_list->head;
	while (ptr != NULL) {
		next_ptr = ptr->next;
		free(ptr);
		ptr = next_ptr;
	}
}

int list_add(struct list *m_list, unsigned int file_clus) {
	struct node *new_node;
	if (!m_list->find(m_list, file_clus)) {
		new_node = calloc(1, sizeof(struct node));
		new_node->fst_file_clus = file_clus;
		new_node->next = m_list->head;
		m_list->head = new_node;
		++(m_list->size);
		return 1;
	}
	return 0;
}

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

int list_find(struct list *m_list, unsigned int file_clus) {
	struct node *ptr;
	ptr = m_list->head;
	while (ptr != NULL) {
		if (ptr->fst_file_clus == file_clus) {
			break;
		}
		ptr = ptr->next;
	}
	if (ptr == NULL) {
		return 0;
	} else {
		return 1;
	}
}
