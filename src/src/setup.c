#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "setup.h"
#include "tools.h"
#include "parse.h"
#include "file_types.h"

// global variables
int endianness;
FILE *fat32_img;
struct fat32_info img_info;
unsigned int first_data_sec;
unsigned int cur_dir_sec;
char *current_directory;
struct list *opened_files;


/* setup function which initializes all the global variables and opens the image
 * in read and write mode.
 */
int setup(char *img_filename, char *exe_name) {
	if ((fat32_img = fopen(img_filename, "r+")) == NULL) {
		perror(NULL);
		return 0;
	}
	endianness = check_endian();
	extract_fat32_info();
	set_root_directory();
	set_open_list();
	print_introduction(img_filename, exe_name);

	return 1;
}

/* extracts the vital information for traversing the fat32 image from the boot
 * sector
 */
int extract_fat32_info(void) {
	read_ushort(&img_info.bytes_per_sec, 11);
	read_uchar(&img_info.sec_per_clus, 13);
	read_ushort(&img_info.rsvd_sec_cnt, 14);
	read_uchar(&img_info.num_fat, 16);
	read_ushort(&img_info.root_ent_cnt, 17);
	read_uint(&img_info.tot_sec32, 32);
	read_uint(&img_info.fat_sz32, 36);
	read_ushort(&img_info.ext_flags, 40);
	read_uint(&img_info.root_clus, 44);
	return 0;
}

/* sets the first root directory sector, current directory sector and names, and
 * the first data sector
 */
int set_root_directory(void) {
	first_data_sec = img_info.rsvd_sec_cnt + (img_info.num_fat*img_info.fat_sz32);
	first_root_sec = get_first_sector_of_cluster(img_info.root_clus);
	cur_dir_clus = img_info.root_clus;
	cur_dir_sec = first_root_sec;
	current_directory_capacity = INIT_CUR_DIR_CAP;
	current_directory = calloc(current_directory_capacity, sizeof(char));
	strcat(current_directory, "/");
	return 1;
}

int set_open_list(void) {
	opened_files = create_list();
	return 1;
}

/* prints the prompt */
void print_prompt(void) {
	printf("%s] ", current_directory);
}

/* prints the intro message */
void print_introduction(char *img_filename, char *exe_name) {
	printf("Welcome to the %s shell utility\n", exe_name);
	printf("Image, %s, is ready to view\n", img_filename);
	printf("For a list of commands, type \"help\" or \"h\"\n");
}
