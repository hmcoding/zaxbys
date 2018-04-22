#ifndef SETUP_H__
#define SETUP_H__

#define INIT_CUR_DIR_CAP 256

// struct for the BPB header variables
struct fatData {
	unsigned short bps;    //bytes_per_sec;
	unsigned short r_e_c;  //root_ent_cnt;
	unsigned short other_options;   //ext_options;
	unsigned short rsvd_s_c;   //rsvd_sec_cnt;
	unsigned char spc;  //sec_per_clus;
	unsigned char n_fat;   //num_fat;
	unsigned int tsec32;   //tot_sec32;
	unsigned int fsz32;   //fat_sz32;
	unsigned int rclustr;   //root_clus;
};

// the setup functions
int SRD(void);   //setRootDir
int su(char *fNm_img, char *nm_run);   //setup   fNamesImage   nameRun
int g_f_info(void);  //getfatinfo
int Openset(void);   //setOpened
void showprompt(void);   //displayprompt
void showintro(char *fNm_img, char *nm_run);   //displayintro

// the global variables 
extern char *thisDir;
extern int endianVar;
extern FILE *fatImage;
extern struct fatData imageData;  
extern struct list *theOpen;
extern unsigned int thisDirCap;
extern unsigned int thisDirClus;
extern unsigned int thisDirSec;
extern unsigned int dataSec;
extern unsigned int rootSec;
extern unsigned int numClus;

#endif
