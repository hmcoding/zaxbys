#ifndef SETUP_H__
#define SETUP_H__

#define INIT_CUR_DIR_CAP 256

// struct for the BPB header variables
struct fatData {
	unsigned short bytes_per_sec;
	unsigned char sec_per_clus;
	unsigned short rsvd_sec_cnt;
	unsigned char num_fat;
	unsigned short root_ent_cnt;
	unsigned int tot_sec32;
	unsigned int fat_sz32;
	unsigned short ext_options;
	unsigned int root_clus;
};

// global variables 
extern int endianVar;
extern FILE *fatImage;
extern struct fatData imageData;
extern unsigned int dataSec;
extern unsigned int rootSec;
extern char *thisDir;
extern unsigned int thisDirCap;
extern unsigned int thisDirClus;
extern unsigned int thisDirSec;
extern struct list *theOpen;
extern unsigned int numClus;

// setup functions
int setup(char *fNamesImage, char *nameRun);
int getFatInfo(void);
int setRootDir(void);
int setOpened(void);
void displayPrompt(void);
void displayIntro(char *fNamesImage, char *nameRun);

#endif
