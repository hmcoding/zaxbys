#ifndef MYSTARTER_H__
#define MYSTARTER_H__

#define INIT_CUR_DIR_CAP 256

struct fatData {
	unsigned short bps;
	unsigned char spc;
	unsigned short rsvd_s_c;
	unsigned char n_fat;
	unsigned short rec;
	unsigned int tsec32;
	unsigned int fsz32;
	unsigned short other_options;
	unsigned int rclustr;
};

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

int toStart(char *fNamesImage, char *nameRun);
int getFatInfo(void);
int setRootDir(void);
int setOpened(void);
void displayPrompt(void);
void displayIntro(char *fNamesImage, char *nameRun);

#endif
