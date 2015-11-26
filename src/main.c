#include <stdio.h>
#include <stdlib.h>

#include "setup.h"
#include "parse.h"
#include "execute.h"
#include "clean.h"
#include "tools.h"


FILE *fat32_img;

int main(int argc, char *argv[]) {
	char *cmd_line;
	char **cmd_args;
	int status;

	if (argc < 2) {
		printf("USAGE: %s <FAT32 img>", argv[0]);
		return 0;
	}

	if (!setup(argv[1])) {
		return 0;
	}
	printf("bytes_per_sec: %u\n", img_info.bytes_per_sec);
	printf("sec_per_clus: %u\n", img_info.sec_per_clus);
	printf("rsvd_sec_cnt: %u\n", img_info.rsvd_sec_cnt);
	printf("num_fat: %u\n", img_info.num_fat);
	printf("root_ent_cnt: %u\n", img_info.root_ent_cnt);
	printf("tot_sec32: %u\n", img_info.tot_sec32);
	printf("fat_sz32: %u\n", img_info.fat_sz32);
	printf("ext_flags: %u\n", img_info.ext_flags);
	printf("root_clus: %u\n", img_info.root_clus);
	/*status = 1;
	while (status) {
		print_prompt();
		if((cmd_line = read_input()) == NULL) {
			fprintf(stderr, "Bad read, try again\n");
			continue;
		}
		cmd_args = parse(cmd_line);
		status = execute_cmd(cmd_args);
		clean_up_loop(cmd_line, cmd_args);
	}*/
	clean_up_globals();

	return 0;
}
