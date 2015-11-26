#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "setup.h"
#include "tools.h"
#include "parse.h"

// global variables
int endianess;
FILE *fat32_img;
struct fat32_info img_info;
char *current_directory;


/* setup function which initializes all the global variables and opens the image
 * in read and write mode.
 */
int setup(char *img_filename) {
	if ((fat32_img = fopen(img_filename, "r+")) == NULL) {
		perror(NULL);
		return 0;
	}
	endianess = check_endian();
	extract_fat32_info();
	print_introduction(img_filename);
	current_directory = calloc(INIT_CUR_DIR_CAP, sizeof(char));
	strcat(current_directory, "/");

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

/* prints the prompt */
void print_prompt(void) {
	printf("%s]", current_directory);
}

/* prints the intro message */
void print_introduction(char *img_filename) {
	printf("Welcome\nImage, %s, is ready to view\n", img_filename);
}
