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

#define EMPTY_FILE				{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} 


struct fileShort {
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

struct fileLong {
	unsigned char ord;
	char name1[10];
	unsigned char attr;
	unsigned char type;
	unsigned char check_sum;
	char name2[12];
	unsigned short first_clus_lo;
	char name3[4];
};

union dirEntry {
	unsigned char raw_bytes[32];
	struct fileShort sf;
	struct fileLong lf;
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

struct list *makeList(void);
void delList(struct list *old_list);

void clearList(struct list *m_list);
int addList(struct list *m_list, unsigned int file_clus, char *mode);
int remList(struct list *m_list, unsigned int file_clus);
struct node *lookList(struct list *m_list, unsigned int file_clus);
struct node *headList(struct list *m_list);
int emptyList(struct list *m_list);
unsigned char toByte(char *mode);
int readCheck(struct node *file);
int writeCheck(struct node *file);
unsigned int retSize(union dirEntry *file);

int retDirEntry(union dirEntry *ptr, unsigned int directory_clus, unsigned int entry_num);
int setDirEntry(union dirEntry *ptr, unsigned int directory_clus, unsigned int entry_num);
int retNextDirEntry(union dirEntry *ptr, unsigned int directory_clus, unsigned int entry_num);
int fileR(union dirEntry *file, unsigned int position, unsigned int size);
int fileW(union dirEntry *file, unsigned int position, unsigned int size, char *str);
int fileDel(union dirEntry *file, unsigned int directory_clus, unsigned int entry_num);
int clustDel(unsigned int file_clus);

int makeDirEntry(char *file_name, unsigned int directory_clus, union dirEntry *file, unsigned int *clus_ptr, unsigned int *offset_ptr, int find_new_clus);
int makeFile(char *file_name, unsigned int directory_clus);
int makeDir(char *dir_name, unsigned int directory_clus);


#endif
