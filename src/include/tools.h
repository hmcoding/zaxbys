#ifndef TOOLS_H__
#define TOOLS_H__


#define END_OF_CHAIN 0x0FFFFFF8
#define NEXT_CLUS_MASK 0x0FFFFFFF

// forward declaration of type from another header file
union dirEntry;

int rChar(void *ptr, long pos, size_t nmemb);
int wChar(void *ptr, long pos, size_t nmemb);

unsigned short retLoVal(unsigned int clus);
unsigned short retHiVal(unsigned int clus);


unsigned int *readUnInt(unsigned int *ptr, long pos);
unsigned int *writeUnInt(unsigned int *ptr, long pos);
unsigned short *readUnSh(unsigned short *ptr, long pos);
unsigned short *writeUnSh(unsigned short *ptr, long pos);
unsigned char *readUnCh(unsigned char *ptr, long pos);
unsigned int switch32(unsigned int val);
unsigned short switch16(unsigned short val);

int check_endian(void);

unsigned int retSecClus(unsigned int clus);
unsigned long retFatClusPos(unsigned int clus, unsigned int fat);
unsigned int retFatNextClus(unsigned int clus);
unsigned int retFatNextClus_true(unsigned int clus);
int chainEnd(unsigned int clus);
int changeFats(unsigned int file_clus, unsigned int value);

int shortLow(char filename[12], char short_name[11]);
int fileShort(char filename[12], char short_name[11]);
int lookupFile(char *filename, unsigned int directory_clus, union dirEntry *ptr, unsigned int *clus_ptr, unsigned int *offset_ptr);
unsigned int retFileClus(union dirEntry *ptr);
unsigned short retHiVal(unsigned int clus);
unsigned short retLoVal(unsigned int clus);
int emptyDir(union dirEntry *dir);

unsigned short retTime(void);
unsigned short int retDate(void);

unsigned int lookupOpenClus(void);
unsigned int lookupOpenDirEntry(unsigned int directory_clus, union dirEntry *ptr, unsigned int *clus_ptr, unsigned int *offset_ptr);

int setEntryNull(unsigned int directory_clus, unsigned int entry_num);
int expClus(unsigned int old_clus);
#endif
