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
	unsigned int sizeFile;
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
	unsigned int fst_fileClus;
	unsigned char options;
	struct node *next;
};

struct list {
	void (*clear)(struct list *theList);
	int (*add)(struct list *theList, unsigned int fileClus, char *mode);
	int (*remove)(struct list *theList, unsigned int fileClus);
	struct node *(*find)(struct list *theList, unsigned int fileClus);
	struct node *(*get_head)(struct list *theList);
	int (*empty)(struct list *theList);
	struct node *head;
	unsigned int size;
};

struct list *makeList(void);
void delList(struct list *pastList);

void clearList(struct list *theList);
int addList(struct list *theList, unsigned int fileClus, char *mode);
int remList(struct list *theList, unsigned int fileClus);
struct node *lookList(struct list *theList, unsigned int fileClus);
struct node *headList(struct list *theList);
int emptyList(struct list *theList);
unsigned char toByte(char *mode);
int readCheck(struct node *file);
int writeCheck(struct node *file);
unsigned int retSize(union dirEntry *file);

int retDirEntry(union dirEntry *ptr, unsigned int dirClus, unsigned int entryDig);
int setDirEntry(union dirEntry *ptr, unsigned int dirClus, unsigned int entryDig);
int retNextDirEntry(union dirEntry *ptr, unsigned int dirClus, unsigned int entryDig);
int fileR(union dirEntry *file, unsigned int location, unsigned int size);
int fileW(union dirEntry *file, unsigned int location, unsigned int size, char *str);
int fileDel(union dirEntry *file, unsigned int dirClus, unsigned int entryDig);
int clustDel(unsigned int fileClus);

int makeDirEntry(char *nameFile, unsigned int dirClus, union dirEntry *file, unsigned int *ptrClus, unsigned int *ptrOff, int lookupClusNew);
int makeFile(char *nameFile, unsigned int dirClus);
int makeDir(char *dir_name, unsigned int dirClus);


#endif
