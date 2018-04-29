#ifndef MYHELPERS_H__
#define MYHELPERS_H__


#define END_OF_CHAIN 0x0FFFFFF8
#define NEXT_CLUS_MASK 0x0FFFFFFF

union dirEntry;

unsigned short switch16(unsigned short val);
unsigned int switch32(unsigned int val);
int endianSee(void);
int chainEnd(unsigned int clus);
int rChar(void *ptr, long pos, size_t use);
unsigned int *readUnInt(unsigned int *ptr, long pos);
unsigned short *readUnSh(unsigned short *ptr, long pos);
unsigned char *readUnCh(unsigned char *ptr, long pos);
int wChar(void *ptr, long pos, size_t use);
unsigned int *writeUnInt(unsigned int *ptr, long pos);
unsigned short *writeUnSh(unsigned short *ptr, long pos);
int changeFats(unsigned int fileClus, unsigned int value);
int expClus(unsigned int oldClus1);
unsigned long retFatClusPos(unsigned int clus, unsigned int fat);
unsigned int retSecClus(unsigned int clus);
unsigned int retFatNextClus(unsigned int clus);
unsigned int retFatNextClus_true(unsigned int clus);
unsigned short retHiVal(unsigned int clus);
unsigned short retLoVal(unsigned int clus);
unsigned short retTime(void);
unsigned short int retDate(void);
unsigned int retFileClus(union dirEntry *ptr);
int lookupFile(char *fNames, unsigned int dirClus, union dirEntry *ptr, unsigned int *ptrClus, unsigned int *ptrOff);
unsigned int lookupOpenClus(void);
unsigned int lookupOpenDirEntry(unsigned int dirClus, union dirEntry *ptr, unsigned int *ptrClus, unsigned int *ptrOff);
int emptyDir(union dirEntry *dir);
int shortLow(char fNames[12], char shNames[11]);
int toShortFile(char fNames[12], char shNames[11]);
int setEntryNull(unsigned int dirClus, unsigned int entryDig);

#endif
