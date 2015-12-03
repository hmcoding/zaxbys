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
extern int endianness;
extern FILE *fat32_img;
extern struct fat32_info img_info;
extern unsigned int first_data_sec;
extern unsigned int first_root_sec;
extern char *current_directory;
extern unsigned int current_directory_capacity;
extern unsigned int cur_dir_clus;
extern unsigned int cur_dir_sec;
extern struct list *opened_files;

// setup functions
int setup(char *img_filename, char *exe_name);
int extract_fat32_info(void);
int set_root_directory(void);
int set_open_list(void);
void print_prompt(void);
void print_introduction(char *img_filename, char *exe_name);

#endif
