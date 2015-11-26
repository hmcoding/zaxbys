#ifndef SETUP_H__
#define SETUP_H__

#define INIT_CUR_DIR_CAP 256

// struct for the BPB header variables
struct fat32_info {
	unsigned short bytes_per_sec;
	unsigned char sec_per_clus;
	unsigned short rsvd_sec_cnt;
	unsigned char num_fat;
	unsigned short root_ent_cnt;
	unsigned int tot_sec32;
	unsigned int fat_sz32;
	unsigned short ext_flags;
	unsigned int root_clus;
};

// global variables 
extern int endianess;
extern FILE *fat32_img;
extern struct fat32_info img_info;
extern char *current_directory;

// setup functions
int setup(char *img_filename);
int extract_fat32_info(void);
void print_prompt(void);
void print_introduction(char *img_filename);

#endif
