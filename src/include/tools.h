#ifndef TOOLS_H__
#define TOOLS_H__


int img_read(void *ptr, long pos, size_t size, size_t nmemb);

unsigned int *read_uint(unsigned int *ptr, long pos);
unsigned short *read_ushort(unsigned short *ptr, long pos);
unsigned char *read_uchar(unsigned char *ptr, long pos);
unsigned int swap_32(unsigned int val);
unsigned short swap_16(unsigned short val);

int check_endian(void);

unsigned int get_first_sec_of_clus(unsigned int clus);
unsigned long get_fat_cluster_position(unsigned int clus, unsigned int fat);

#endif
