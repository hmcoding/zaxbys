#include <stdio.h>

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
