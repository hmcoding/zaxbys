#ifndef MYHELPERS_H__
#define MYHELPERS_H__


#define END_OF_CHAIN 0x0FFFFFF8
#define NEXT_CLUS_MASK 0x0FFFFFFF

union dirEntry;

int rChar(void *ptr, long pos, size_t use);
int wChar(void *ptr, long pos, size_t use);

unsigned short retLoVal(unsigned int clus);
unsigned short retHiVal(unsigned int clus);


unsigned int *readUnInt(unsigned int *ptr, long pos);
unsigned int *writeUnInt(unsigned int *ptr, long pos);
unsigned short *readUnSh(unsigned short *ptr, long pos);
unsigned short *writeUnSh(unsigned short *ptr, long pos);
unsigned char *readUnCh(unsigned char *ptr, long pos);
unsigned int switch32(unsigned int val);
unsigned short switch16(unsigned short val);

int endianSee(void);

unsigned int retSecClus(unsigned int clus);
unsigned long retFatClusPos(unsigned int clus, unsigned int fat);
unsigned int retFatNextClus(unsigned int clus);
unsigned int retFatNextClus_true(unsigned int clus);
int chainEnd(unsigned int clus);
int changeFats(unsigned int fileClus, unsigned int value);

int shortLow(char fNames[12], char shNames[11]);
int toShortFile(char fNames[12], char shNames[11]);
int lookupFile(char *fNames, unsigned int dirClus, union dirEntry *ptr, unsigned int *ptrClus, unsigned int *ptrOff);
unsigned int retFileClus(union dirEntry *ptr);
unsigned short retHiVal(unsigned int clus);
unsigned short retLoVal(unsigned int clus);
int emptyDir(union dirEntry *dir);

unsigned short retTime(void);
unsigned short int retDate(void);

unsigned int lookupOpenClus(void);
unsigned int lookupOpenDirEntry(unsigned int dirClus, union dirEntry *ptr, unsigned int *ptrClus, unsigned int *ptrOff);

int setEntryNull(unsigned int dirClus, unsigned int entryDig);
int expClus(unsigned int oldClus1);
#endif
