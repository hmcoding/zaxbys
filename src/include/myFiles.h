#ifndef MYFILES_H__
#define MYFILES_H__


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
	unsigned char trait;
	unsigned char ntr;
	unsigned short timeCRT;
	unsigned short dateCRT;
	unsigned short dateLast;
	unsigned short initHiClus;
	unsigned short timeWRT;
	unsigned short dateWRT;
	unsigned short intitLoClus;
	unsigned int sizeFile;
};

struct fileLong {
	unsigned char trait;
	unsigned short intitLoClus;
};

union dirEntry {
	unsigned char natBytes[32];
	struct fileShort shFi;
	struct fileLong lonFi;
};

struct node {
	unsigned int fileClusFST;
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

int setDirEntry(union dirEntry *ptr, unsigned int dirClus, unsigned int entryDig);
int retDirEntry(union dirEntry *ptr, unsigned int dirClus, unsigned int entryDig);
int retNextDirEntry(union dirEntry *ptr, unsigned int dirClus, unsigned int entryDig);
int makeFile(char *nameFile, unsigned int dirClus);
int makeDir(char *dir_name, unsigned int dirClus);
int makeDirEntry(char *nameFile, unsigned int dirClus, union dirEntry *file, unsigned int *ptrClus, unsigned int *ptrOff, int lookupClusNew);
struct list *makeList(void);
void clearList(struct list *theList);
int remList(struct list *theList, unsigned int fileClus);
int emptyList(struct list *theList);
int addList(struct list *theList, unsigned int fileClus, char *mode);
struct node *lookList(struct list *theList, unsigned int fileClus);
struct node *headList(struct list *theList);
unsigned char toByte(char *mode);
unsigned int retSize(union dirEntry *file);
int readCheck(struct node *file);
int writeCheck(struct node *file);
int fileR(union dirEntry *file, unsigned int location, unsigned int size);
int fileW(union dirEntry *file, unsigned int location, unsigned int size, char *str);
int clustDel(unsigned int fileClus);
int fileDel(union dirEntry *file, unsigned int dirClus, unsigned int entryDig);






#endif
