#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include "tools.h"
#include "setup.h"


// global variables used in this file
int endianness;
FILE *fat32_img;
struct fat32_info img_info;
unsigned int first_data_sec;
unsigned int first_root_sec;


/* img_read
 *
 * read from the global fat32_img which is in little endian with respect to
 * bytes and convert the data to big endian.
 */
int img_read(void *ptr, long pos, size_t size, size_t nmemb) {
	if (fseek(fat32_img, pos, SEEK_SET) == -1) {
		perror(NULL);
		return 0;
	}
	if (fread(ptr, size, nmemb, fat32_img) != nmemb) {
		perror(NULL);
		return 0;
	}
	// convert to 
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

unsigned int get_first_sec_of_clus(unsigned int clus) {
	return (clus - 2)*img_info.sec_per_clus + first_data_sec;
}

/* returns the absolute position of the given cluster in the given FAT in terms
 * of bytes
 */
unsigned long get_fat_cluster_position(unsigned int clus, unsigned int fat) {
	unsigned int fat_start_sec = img_info.rsvd_sec_cnt + fat*img_info.fat_sz32;
	return fat_start_sec*img_info.bytes_per_sec + 4*clus;
}
