#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "tools.h"
#include "setup.h"
#include "file_types.h"
#include "shell_error.h"

// global variables used in this file
int endianness;
FILE *fat32_img;
struct fat32_info img_info;
unsigned int first_data_sec;
unsigned int first_root_sec;
unsigned int count_of_clusters;


// reads nmemb amount of characters from fat32_img file at pos
int read_chars(void *ptr, long pos, size_t nmemb) {
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
int write_chars(void *ptr, long pos, size_t nmemb) {
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
unsigned int *read_uint(unsigned int *ptr, long pos) {
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
		*ptr = swap_32(*ptr);
	}
	return ptr;
}

// writes an unsigned int to fat32_img at pos
unsigned int *write_uint(unsigned int *ptr, long pos) {
	long offset;
	unsigned int copy;
	offset = pos - ftell(fat32_img);
	if (endianness) {
		copy = swap_32(*ptr);
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
unsigned short *read_ushort(unsigned short *ptr, long pos) {
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
		*ptr = swap_16(*ptr);
	}
	return ptr;
}

// writes an unsigned short to fat32_img at pos
unsigned short *write_ushort(unsigned short *ptr, long pos) {
	long offset;
	unsigned short copy;
	offset = pos - ftell(fat32_img);
	if (endianness) {
		copy = swap_16(*ptr);
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
unsigned char *read_uchar(unsigned char *ptr, long pos) {
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
unsigned int swap_32(unsigned int val) {
	return ((val>>24)&0xff) | ((val<<8)&0xff0000) | ((val>>8)&0xff00) | ((val<<24)&0xff000000);
}

unsigned short swap_16(unsigned short val) {
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
unsigned int get_first_sector_of_cluster(unsigned int clus) {
	return (clus - 2)*img_info.sec_per_clus + first_data_sec;
}

/* returns the absolute position of the given cluster in the given FAT in terms
 * of bytes
 */
unsigned long get_fat_cluster_position(unsigned int clus, unsigned int fat) {
	unsigned int i, fat_start_sec;
	i = fat < img_info.num_fat ? fat : 0;
	fat_start_sec = img_info.rsvd_sec_cnt + i*img_info.fat_sz32;
	return fat_start_sec*img_info.bytes_per_sec + 4*clus;
}

/* returns the next cluster in the chain
 */
unsigned int get_next_cluster_in_fat(unsigned int clus) {
	unsigned long position;
	unsigned int next_clus;
	position = get_fat_cluster_position(clus, 0);
	read_uint(&next_clus, position);
	return next_clus & NEXT_CLUS_MASK;
}

// same as above but doesn't apply the NEXT_CLUS_MASK to result
unsigned int get_next_cluster_in_fat_true(unsigned int clus) {
	unsigned long position;
	unsigned int next_clus;
	position = get_fat_cluster_position(clus, 0);
	read_uint(&next_clus, position);
	return next_clus;
}

// checks if the value is one of the end of chain values
int end_of_chain(unsigned int clus) {
	if ((clus & END_OF_CHAIN) == END_OF_CHAIN) {
		return 1;
	} else {
		return 0;
	}
}

// changes the values in all file allocation tables at file_clus to "value"
int modify_all_fats(unsigned int file_clus, unsigned int value) {
	unsigned long position;
	int i;
	for (i = 0; i < img_info.num_fat; ++i) {
		position = get_fat_cluster_position(file_clus, i);
		write_uint(&value, position);
	}
	return 1;
}

// takes a short name and stores transformed name to filename
int short_to_lowercase(char filename[12], char short_name[11]) {
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
int filename_to_short(char filename[12], char short_name[11]) {
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
int find_file(char *filename, unsigned int directory_clus, union directory_entry *ptr, unsigned int *clus_ptr, unsigned int *offset_ptr) {
	unsigned int current_clus, i, limit, done;
	union directory_entry file;
	char short_name[11];
	filename_to_short(filename, short_name);
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
		current_clus = get_next_cluster_in_fat(current_clus);
	} while (!end_of_chain(current_clus) && !done);
	return 0;
}

// gets the first file cluster in the given directory entry
unsigned int get_file_cluster(union directory_entry *ptr) {
	unsigned int file_clus;
	unsigned short hi, lo;
	hi = ptr->sf.first_clus_hi;
	lo = ptr->sf.first_clus_lo;
	if (endianness) {
		hi = swap_16(hi);
		lo = swap_16(lo);
	}
	file_clus = (hi << 16) | (lo & 0xFFFF);
	return file_clus;
}

// checks if the directory is empty
int empty_directory(union directory_entry *dir) {
	unsigned int current_clus, i, j, limit, done;
	union directory_entry file;
	current_clus = get_file_cluster(dir);
	limit = img_info.bytes_per_sec*img_info.sec_per_clus/32;
	done = 0;
	j = 0;
	do {
		for (i = 0; i < limit; ++i, ++j) {
			get_directory_entry(&file, current_clus, i);
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
		current_clus = get_next_cluster_in_fat(current_clus);
	} while (!end_of_chain(current_clus) && !done);
	return 1;
}

// returns the hi value of the given cluster
unsigned short get_hi(unsigned int clus) {
	unsigned short hi;
	hi = (unsigned short)clus >> 16;
	if (endianness) {
		hi = swap_16(hi);
	}
	return hi;
}

// returns the lo value of the given cluster
unsigned short get_lo(unsigned int clus) {
	unsigned short lo;
	lo = (unsigned short)clus & 0xFFFF;
	if (endianness) {
		lo = swap_16(lo);
	}
	return lo;
}

// returns the time in the FAT32 format
unsigned short get_time(void){
	time_t rawtime;
	struct tm * cur_time;
	unsigned short t;

	time(&rawtime);
	cur_time = localtime(&rawtime);
	t = ((cur_time->tm_hour*0x0800)+(cur_time->tm_min*0x0020)+(cur_time->tm_sec/2));
	if (endianness) {
		t = swap_16(t);
	}
	return t;
}

// returns the date in the FAT32 format
unsigned short get_date(void){
	time_t rawtime;
	struct tm * cur_time;
	unsigned short d;

	time(&rawtime);
	cur_time = localtime(&rawtime);
	d = (((cur_time->tm_year-80)*0x0200)+((cur_time->tm_mon+1)*0x0020)+(cur_time->tm_mday));
	if (endianness) {
		d = swap_16(d);
	}
	return d;
}

// searches for the next open cluster in the FAT
unsigned int find_open_cluster() {
	unsigned int value;	//cluster data
	unsigned int found = 1;	//free space found
	unsigned int i;			//counter
	for (i = 2; i < count_of_clusters; ++i) {
		value = get_next_cluster_in_fat(i);
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
unsigned int find_open_directory_entry(unsigned int directory_clus, union directory_entry *ptr, unsigned int *clus_ptr, unsigned int *offset_ptr) {
	unsigned int current_clus, i, limit, found;
	union directory_entry file;
	current_clus = directory_clus;
	limit = img_info.bytes_per_sec*img_info.sec_per_clus/32;
	found = 0;
	do {
		for (i = 0; i < limit; ++i) {
			get_directory_entry(&file, current_clus, i);
			if (file.raw_bytes[0] == 0x00) {
				set_next_entry_to_null(current_clus, i);
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
		current_clus = get_next_cluster_in_fat(current_clus);
	} while (!end_of_chain(current_clus));
	return 0;
}

// sets the next directory entry to 0x00. Expands the directory if necessary
int set_next_entry_to_null(unsigned int directory_clus, unsigned int entry_num) {
	union directory_entry file;
	unsigned int first_dir_clus, entry_first_byte_offset, next_num;
	next_num = entry_num + 1;
	if ((entry_first_byte_offset = 32*next_num) >= img_info.bytes_per_sec*img_info.sec_per_clus) {
		if (end_of_chain(get_next_cluster_in_fat(directory_clus))) {
			expand_cluster(directory_clus);
		}
		first_dir_clus = get_first_sector_of_cluster(get_next_cluster_in_fat(directory_clus));
		entry_first_byte_offset -= img_info.bytes_per_sec*img_info.sec_per_clus;
	} else {
		first_dir_clus = get_first_sector_of_cluster(directory_clus);
	}
	read_chars(&file, entry_first_byte_offset + first_dir_clus*img_info.bytes_per_sec, sizeof(union directory_entry));
	file.raw_bytes[0] = 0x00;
	write_chars(&file, entry_first_byte_offset + first_dir_clus*img_info.bytes_per_sec, sizeof(union directory_entry));
	return 1;
}

// function for making room in the file
int expand_cluster(unsigned int old_clus) {
	unsigned int new_clus;
	new_clus = find_open_cluster();
	if (new_clus != 0) {
		modify_all_fats(old_clus, new_clus);
		modify_all_fats(new_clus, END_OF_CHAIN);
	}
	return 1;
}
