#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include "tools.h"
#include "setup.h"


FILE *fat32_img;

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
	if (endianess) {
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
	if (endianess) {
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
