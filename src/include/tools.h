#ifndef TOOLS_H__
#define TOOLS_H__


#define END_OF_CHAIN 0x0FFFFFF8
#define NEXT_CLUS_MASK 0x0FFFFFFF

// forward declaration of type from another header file
union directory_entry;

int read_chars(void *ptr, long pos, size_t nmemb);

unsigned int *read_uint(unsigned int *ptr, long pos);
unsigned short *read_ushort(unsigned short *ptr, long pos);
unsigned char *read_uchar(unsigned char *ptr, long pos);
unsigned int swap_32(unsigned int val);
unsigned short swap_16(unsigned short val);

int check_endian(void);

unsigned int get_first_sector_of_cluster(unsigned int clus);
unsigned long get_fat_cluster_position(unsigned int clus, unsigned int fat);
unsigned int get_next_cluster_in_fat(unsigned int clus);
int end_of_chain(unsigned int clus);

int short_to_lowercase(char filename[12], char short_name[11]);
int filename_to_short(char filename[12], char short_name[11]);
int find_file(char *filename, unsigned int directory_clus, union directory_entry *ptr);
unsigned int get_file_cluster(union directory_entry *ptr);

short int get_time(void);
short int get_date(void);

#endif
