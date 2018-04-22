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
extern int endian_V;   //endianVar
extern FILE *fImage;   //fatimage
extern struct fatData imgdata;  //imageData
extern struct list *theOpen;
extern unsigned int this_Dr_Cap;   //thisDirCap
extern unsigned int this_Dr_Clustr;   //thisDirClus
extern unsigned int this_Dr_Sec;   //thisDirSec
extern unsigned int d_sec;    //dataSec
extern unsigned int r_sec;     //rootSec
extern unsigned int nClustr;   //numClus

#endif
