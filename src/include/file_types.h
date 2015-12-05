#ifndef FILE_TYPES_H__
#define FILE_TYPES_H__


#define ATTR_READ_ONLY			0x01
#define ATTR_HIDDEN				0x02
#define ATTR_SYSTEM				0x04
#define ATTR_VOLUME_ID			0x08
#define ATTR_DIRECTORY			0x10
#define ATTR_ARCHIVE			0x20
#define ATTR_LONG_NAME 			(ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID)
#define ATTR_LONG_NAME_MASK 	(ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID | ATTR_DIRECTORY | ATTR_ARCHIVE)

#define OPEN_BAD				0x00
#define OPEN_READ				0x01
#define OPEN_WRITE				0x02


struct short_file {
	char name[11];
	unsigned char attr;
	unsigned char ntr;
	unsigned char crt_time_tenth;
	unsigned short crt_time;
	unsigned short crt_date;
	unsigned short last_acc_date;
	unsigned short first_clus_hi;
	unsigned short wrt_time;
	unsigned short wrt_date;
	unsigned short first_clus_lo;
	unsigned int file_size;
};

struct long_file {
	unsigned char ord;
	char name1[10];
	unsigned char attr;
	unsigned char type;
	unsigned char check_sum;
	char name2[12];
	unsigned short first_clus_lo;
	char name3[4];
};

union directory_entry {
	unsigned char raw_bytes[32];
	struct short_file sf;
	struct long_file lf;
};

// singlely-linked list definition
struct node {
	unsigned int fst_file_clus;
	unsigned char flags;
	struct node *next;
};

struct list {
	void (*clear)(struct list *m_list);
	int (*add)(struct list *m_list, unsigned int file_clus, char *mode);
	int (*remove)(struct list *m_list, unsigned int file_clus);
	struct node *(*find)(struct list *m_list, unsigned int file_clus);
	struct node *(*get_head)(struct list *m_list);
	int (*empty)(struct list *m_list);
	struct node *head;
	unsigned int size;
};

struct list *create_list(void);
void delete_list(struct list *old_list);

void list_clear(struct list *m_list);
int list_add(struct list *m_list, unsigned int file_clus, char *mode);
int list_remove(struct list *m_list, unsigned int file_clus);
struct node *list_find(struct list *m_list, unsigned int file_clus);
struct node *list_get_head(struct list *m_list);
int list_empty(struct list *m_list);
unsigned char file_mode_to_byte(char *mode);
int check_read(struct node *file);
int check_write(struct node *file);
unsigned int get_size(union directory_entry *file);

int get_directory_entry(union directory_entry *ptr, unsigned int directory_clus, unsigned int entry_num);
int set_directory_entry(union directory_entry *ptr, unsigned int directory_clus, unsigned int entry_num);
int get_next_directory_entry(union directory_entry *ptr, unsigned int directory_clus, unsigned int entry_num);
int read_file(union directory_entry *file, unsigned int position, unsigned int size);
int write_file(union directory_entry *file, unsigned int position, unsigned int size, char *str);
int delete_file(union directory_entry *file, unsigned int directory_clus, unsigned int entry_num);
int delete_cluster(unsigned int file_clus);

int create_directory_entry(char *file_name, unsigned int directory_clus, union directory_entry *file, unsigned int *clus_ptr, unsigned int *offset_ptr, int find_new_clus);
int create_file(char *file_name, unsigned int directory_clus);
int create_directory(char *dir_name, unsigned int directory_clus);


#endif
