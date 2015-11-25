#ifndef SETUP_H__
#define SETUP_H__

#define INIT_CUR_DIR_CAP 256

/* current_directory is a null terminated string array
 *
 * Example:
 * current_directory = [] -> cwd = "/"
 * current_directory = ["home", "person"] -> "/home/person"
 */
extern char *current_directory;
extern unsigned int cur_dir_size;


void setup(char *img_filename);
void print_prompt(void);
void print_introduction(char *img_filename);

#endif
