#ifndef TOOLS_H__
#define TOOLS_H__


#define END_OF_CHAIN 0x0FFFFFF8
#define NEXT_CLUS_MASK 0x0FFFFFFF

// forward declaration of type from another header file
union dirEntry;   

unsigned short low(unsigned int clustr);  //retloval  //clus
unsigned short high(unsigned int clustr);   //rethival

int readChar(void *ptr, long p, size_t use);     //rChar
int writeChar(void *ptr, long p, size_t use);     //wChar
int wrChar(void *ptr, long p, size_t use);     //////////////////////////combo of read and write char 

int c_endian(void);   //check_endian

unsigned int RCS(unsigned int clustr);
unsigned long RFCP(unsigned int clustr, unsigned int fat);
unsigned int RFNC(unsigned int clustr);
unsigned int RFNC_true(unsigned int clustr);
int endofchain(unsigned int clustr);
int changeF(unsigned int fileClus, unsigned int v);   //changeFat

unsigned int *RUInt(unsigned int *ptr, long p);   //readUnInt
unsigned int *WUInt(unsigned int *ptr, long p);    //writeUnInt
unsigned short *RUSh(unsigned short *ptr, long p);   //readUnSh
unsigned short *WUSh(unsigned short *ptr, long p);    //writeUnSh
unsigned char *RUCh(unsigned char *ptr, long p);
unsigned int S32(unsigned int v);
unsigned short S16(unsigned short v);

int SLow(char fNames[12], char shNames[11]);  //shortlow
int fShort(char fNames[12], char shNames[11]);  //fileshort
int filelookup(char *fNames, unsigned int dirClus, union dirEntry *ptr, unsigned int *ptrClus, unsigned int *ptrOff);  //lookupfile
unsigned int RFC(union dirEntry *ptr);  //retfileclus
//there was an extra rethival and retloval functions
//unsigned short retHiVal(unsigned int clus);
//unsigned short retLoVal(unsigned int clus);
int emptyD(union dirEntry *dir);   //emptyDir

int SENull(unsigned int dirClus, unsigned int entryDig);  //setentrynull
int e_Clustr(unsigned int oldClus1);   //expClus

unsigned short RTime(void);  //rettime
unsigned short int RDate(void);    //retdate

unsigned int luOpenClus(void);  //lookup
unsigned int luOpenDirEntry(unsigned int dirClus, union dirEntry *ptr, unsigned int *ptrClus, unsigned int *ptrOff);   //lookup
