#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "tools.h"
#include "setup.h"
#include "file_types.h"


// global variables used in this file
int endianness;
FILE *fat32_img;
struct fat32_info img_info;
unsigned int first_data_sec;
unsigned int first_root_sec;


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

int end_of_chain(unsigned int clus) {
	if ((clus & END_OF_CHAIN) == END_OF_CHAIN) {
		return 1;
	} else {
		return 0;
	}
}

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

int find_file(char *filename, unsigned int directory_clus, union directory_entry *ptr) {
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
				return 1;
			}
		}
		current_clus = get_next_cluster_in_fat(current_clus);
	} while (!end_of_chain(current_clus) && !done);
	return 0;
}

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

unsigned short get_time(void){
	time_t rawtime;
	struct tm * cur_time;
	short int t;

	time(&rawtime);
	cur_time = localtime(&rawtime);
	t = ((cur_time->tm_hour*0x0800)+(cur_time->tm_min*0x0020)+(cur_time->tm_sec/2));
	if (endianness) {
		t = swap_16(t);
	}
	return t;
}

unsigned short get_date(void){
	time_t rawtime;
	struct tm * cur_time;
	short int d;

	time(&rawtime);
	cur_time = localtime(&rawtime);
	d = (((cur_time->tm_year-80)*0x0200)+((cur_time->tm_mon+1)*0x0020)+(cur_time->tm_mday));
	if (endianness) {
		d = swap_16(d);
	}
	return d;
}
