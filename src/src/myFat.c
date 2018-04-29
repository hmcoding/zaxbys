#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "myUtility.h"
#include "myHelpers.h"
#include "myParser.h"
#include "myStarter.h"
#include "myFiles.h"


FILE *fatImage;
struct list *theOpen;
struct fatData imageData;
unsigned int thisDirClus;
char *thisDir;
unsigned int thisDirCap;
int endianVar;
unsigned int dataSec;
unsigned int thisDirSec;
unsigned int rootSec;
unsigned int numClus;



// MYUTILITY.H ****************************************** START

void loopClean(char *progLine, char **progArgs) {
	
	free(progLine);
	int i;
		i = 0;
		while (progArgs[i] != NULL) {
			free(progArgs[i]);
			progArgs[i] = NULL;
			++i;
		}
	
	
		free(progArgs);
}

void globClean(void) {
	
	theOpen->clear(theOpen);
	free(theOpen);
	free(thisDir);
	fclose(fatImage);
}

void error(int type, char* print){
	switch(type){
		case 1:
		printf("Error: %s: File is already open\n", print);
		break;
		case 2:
		printf("Error: %s: File does not exist\n", print);
		break;
		case 3:
		printf("Error: %s: Cannot open directory\n", print);
		break;
		case 4:
		printf("Error: %s: Incorrect parameter\n", print);
		break;
		case 5:
		printf("Error: %s: Cannot close directory\n", print);
		break;
		case 6:
		printf("Error: %s: File is not open\n", print);
		break;
		case 7:
		printf("Error: %s: File already exists\n", print);
		break;
		case 8:
		printf("Error: %s: Must specify a file name and mode\n", print);
		break;
		case 9:
		printf("Error: %s: Must specify a file name\n", print);
		break;
		case 10:
		printf("Error: %s: Must specify a directory name\n", print);
		break;
		case 11:
		printf("Error: %s: Not a directory\n", print);
		break;
		case 12:
		printf("Error: %s: Does not exist\n", print);
		break;
		case 13:
		printf("Error: %s: Must specify a file name, location, and size\n", print);
		break;
		case 14:
		printf("Error: %s: Must specify a file name, location, size, and string\n", print);
		break;
		case 15:
		printf("Error: %s: File not open in read mode\n", print);
		break;
		case 16:
		printf("Error: %s: File not open in write mode\n", print);
		break;
		case 17:
		printf("Error: %s: Not a directory\n", print);
		break;
		case 18:
		printf("Error: %s: Directory is not empty\n", print);
		break;
		case 19:
		printf("Error: %s: File or directory already exists\n", print);
		break;
		case 20:
		printf("Error: %s: Not a file\n", print);
		break;
	}
}

int displayDir(unsigned int dirClus) {
	unsigned int i;
	union dirEntry file;
	char fNames[12];
	unsigned int thisClus = dirClus;
	unsigned int bound = imageData.bps*imageData.spc/32;
	unsigned int finish = 0;
	do {
		
		for (i = 0; i < bound; ++i) {
			retDirEntry(&file, thisClus, i);
			
			if (file.natBytes[0] == 0x00) {
				finish = 1;
				break;
			} else if (file.natBytes[0] == 0xE5) {
				continue;
			} else if ((file.lonFi.trait & ATTR_LONG_NAME_MASK) == ATTR_LONG_NAME) {
				continue;
			} else if ((file.shFi.trait & (ATTR_DIRECTORY | ATTR_VOLUME_ID)) == 0x00) {
				shortLow(fNames, file.shFi.name);
				printf("%s\t", fNames);
			} else if ((file.shFi.trait & (ATTR_DIRECTORY | ATTR_VOLUME_ID)) == ATTR_DIRECTORY) {
				shortLow(fNames, file.shFi.name);
				printf("%s/\t", fNames);
			} else if ((file.shFi.trait & (ATTR_DIRECTORY | ATTR_VOLUME_ID)) == ATTR_VOLUME_ID) {
				continue;
			} else {
				
			}
			
			
		}
	
		
		thisClus = retFatNextClus(thisClus);
	} while (!chainEnd(thisClus) && !finish);
	return 1;
}

int changeDirClus(union dirEntry *ptr) {
	unsigned int fileClus;
	if ((ptr->shFi.trait & ATTR_DIRECTORY) == ATTR_DIRECTORY) {
		fileClus = retFileClus(ptr);
		
		
		if (fileClus != 0) {
			thisDirClus = fileClus;
		}
		else {
			thisDirClus = imageData.rclustr;
		}
	}
	return 1;
}

int changeCurDir(union dirEntry *ptr) {
	char fNames[12];
	unsigned int slashNTL;
	if ((ptr->shFi.trait & ATTR_DIRECTORY) == ATTR_DIRECTORY) {
		shortLow(fNames, ptr->shFi.name);
		
		if (strcmp(fNames, "..") == 0) {
			slashNTL = lookupNextToLastSlash();
			thisDir[slashNTL + 1] = '\0';
		} else if (strcmp(fNames, ".") == 0) {
			// do nothing
		} else if (retFileClus(ptr) == 0) {
			strcpy(thisDir, "/");
		} else {
			if (strlen(thisDir) + strlen(fNames) + 1 > thisDirCap) {
				//thisDirCap *= 2;
				thisDirCap = thisDirCap * 2;
				thisDir = realloc(thisDir, thisDirCap);
			}
			strcat(thisDir, fNames);
			strcat(thisDir, "/");
		}
	}
	return 1;
}

unsigned int lookupNextToLastSlash() {
	
	unsigned int i;
	unsigned int lastSlash = 0; 
	unsigned int ntlSlash = 0;
	
	
	i = 1;
	while (thisDir[i] != '\0'){
		if (thisDir[i] == '/') {
			ntlSlash = lastSlash;
			lastSlash = i;
		}
		++i;
	}
	
	
	return ntlSlash;
}

int userCmd(char **progArgs) {
	
	
	if (progArgs[0] == NULL) {
		
	} else if (strcmp(progArgs[0], "exit") == 0) {
		 return exitCmd();
	} else if (strcmp(progArgs[0], "info") == 0) {
		infoCmd();
	} else if (strcmp(progArgs[0], "ls") == 0) {
		lsCmd(progArgs);
	} else if (strcmp(progArgs[0], "cd") == 0) {
		cdCmd(progArgs);
	} else if (strcmp(progArgs[0], "size") == 0) {
		sizeCmd(progArgs);
	} else if (strcmp(progArgs[0], "create") == 0) {
		createCmd(progArgs);
	} else if (strcmp(progArgs[0], "mkdir") == 0) {
		mkdirCmd(progArgs);
	} else if (strcmp(progArgs[0], "rm") == 0) {
		rmCmd(progArgs);
	} else if (strcmp(progArgs[0], "rmdir") == 0) {
		rmdirCmd(progArgs);
	} else if (strcmp(progArgs[0], "open") == 0) {
		openCmd(progArgs);
	} else if (strcmp(progArgs[0], "close") == 0) {
		closeCmd(progArgs);
	} else if (strcmp(progArgs[0], "read") == 0) {
		readCmd(progArgs);
	} else if (strcmp(progArgs[0], "write") == 0) {
		writeCmd(progArgs);
	}  else {
		printf("command: %s: Command is not recognized\n", progArgs[0]);
	}
	
	return 1;
}

int exitCmd(void) {
	printf("Exiting.\n");
	return 0;
}

int infoCmd(){
	printf("Number of FATs: %d\n", imageData.n_fat);
	printf("Bytes per Sector: %d\n", imageData.bps);
	printf("Total Sectors: %d\n",imageData.tsec32);
	printf("Sectors per FAT: %d\n", imageData.fsz32);
	printf("Sectors per Cluster: %d\n", imageData.spc);

	return 0;
}

int lsCmd(char **progArgs) {
	int check;
	union dirEntry file;
	unsigned int DirClus;

	
	if (progArgs[1] != NULL) {
		check = lookupFile(progArgs[1], thisDirClus, &file, NULL, NULL);
		
		if ((file.shFi.trait & ATTR_DIRECTORY) != ATTR_DIRECTORY) {
			error(11, progArgs[1]);
			return 0;
		}
		else if (!check) {
			error(12, progArgs[1]);
			return 0;
		}
		else {
			DirClus = retFileClus(&file);
		}
	}
	else {
		DirClus = thisDirClus;
	}
	
	
	displayDir(DirClus);
	printf("\n");
	return 0;
}

int cdCmd(char **progArgs) {
	int check;
	union dirEntry file;
	
	if (progArgs[1] != NULL) {
		check = lookupFile(progArgs[1], thisDirClus, &file, NULL, NULL);
	
		if ((file.shFi.trait & ATTR_DIRECTORY) != ATTR_DIRECTORY) {
			error(11, progArgs[1]);
		}
		else if (!check) {
			error(12, progArgs[1]);
		}
		else {
			changeDirClus(&file);
			changeCurDir(&file);
		}
		
	}
	
	return 0;
}

int sizeCmd(char **progArgs) { 
	union dirEntry file;
	int doCheck;

	if (progArgs[1] != NULL) {
	doCheck = lookupFile(progArgs[1], thisDirClus, &file, NULL, NULL);
		if (doCheck) {
			printf("%u\n", retSize(&file));
		} else {
			error(2,progArgs[1]);
		}
	} else {
		error(9, progArgs[0]);
	}
	return 0;
}

int createCmd(char **progArgs) { 
	union dirEntry file;
	int doCheck;
	unsigned int DirClus;
	unsigned int offset;


	if (progArgs[1] != NULL) {
	doCheck = lookupFile(progArgs[1], thisDirClus, &file, &DirClus, &offset);
	
	
		if (!doCheck) {
			makeFile(progArgs[1], thisDirClus);
		} else {
			error(19, progArgs[1]);
		}
	
	
	}
	else {
	     error(9, progArgs[0]);
	}


	return 0;
}

int mkdirCmd(char **progArgs) {
	int check;
	union dirEntry file;
	unsigned int DirClus;
	unsigned int offset;

	if (progArgs[1] != NULL) {
		check = lookupFile(progArgs[1], thisDirClus, &file, &DirClus, &offset);
		
		if (!check) {
			makeDir(progArgs[1], thisDirClus);
		}
		else {
			error(19, progArgs[1]);
		}
	}
	else {
		error(9, progArgs[0]);
	}
	
	return 0;
}

int rmCmd(char **progArgs) { 
union dirEntry file;
	int doCheck;
	char *nameFile;
	struct node *ptrFile;
	unsigned int fileClus;
	unsigned int DirClus;
	unsigned int offset;
	nameFile = progArgs[1];
	if (nameFile != NULL) {
	doCheck = lookupFile(progArgs[1], thisDirClus, &file, &DirClus, &offset);
		if (!doCheck) {
			error(2,progArgs[1]);
		} else if ((file.shFi.trait & ATTR_DIRECTORY) == ATTR_DIRECTORY) {
			error(20,progArgs[1]);
		} else {
			fileClus = retFileClus(&file);
			ptrFile = theOpen->find(theOpen, fileClus);
			if (ptrFile != NULL) {
				theOpen->remove(theOpen, fileClus);
			}
			fileDel(&file, DirClus, offset);
		}
	} else {
		error(9, progArgs[0]);
	}
	return 0;
}

int rmdirCmd(char **progArgs) {
	int check;
	char *nameFile;
	union dirEntry file;
	unsigned int DirClus;
	unsigned int offset;
	nameFile = progArgs[1];
	
	
	if (nameFile != NULL) {
		check = lookupFile(progArgs[1], thisDirClus, &file, &DirClus, &offset);
		
		
		if (!emptyDir(&file)) {
			error(18, progArgs[1]);
		} else if (!check) {
			error(2, progArgs[1]);
		} else if ((file.shFi.trait & ATTR_DIRECTORY) != ATTR_DIRECTORY) {
			error(11, progArgs[1]);
		} else {
			fileDel(&file, DirClus, offset);
		}
		
	}
	else {
		error(10, progArgs[0]);
	}
	
	return 0;
}


int openCmd(char **progArgs) { 
	int doCheck;
	union dirEntry file;
	unsigned int doFile;
	unsigned int doDir;
	unsigned int offset;
	struct node *ptrFile;
	
	
	if (progArgs[2] == NULL) {
		error(8,progArgs[0]);
	} else if (progArgs[1] == NULL) {
		error(8,progArgs[0]);
	} else {
		doCheck = lookupFile(progArgs[1], thisDirClus, &file, &doDir, &offset);
		
		
		if (!doCheck) {
			error(2,progArgs[1]);
		}
		else if ((file.shFi.trait & ATTR_DIRECTORY) == ATTR_DIRECTORY) {
			error(3,progArgs[1]);
		}
		else if (toByte(progArgs[2]) == 0x0) {
			error(4,progArgs[2]);
		}
		else {
			doFile = retFileClus(&file);
			ptrFile = theOpen->find(theOpen, doFile);
		
			if (ptrFile == NULL) {
				file.shFi.dateLast = retDate();
				setDirEntry(&file, doDir, offset);
				theOpen->add(theOpen, retFileClus(&file), progArgs[2]);
			}
			 else {
				error(1,progArgs[1]);
			}
		}
		
	}

	
	return 0;
}

int closeCmd(char **progArgs) { 
	union dirEntry file;
	int doCheck;
	struct node *ptrFile;
	unsigned int fileClus;

	if (progArgs[1] != NULL) {
	doCheck = lookupFile(progArgs[1], thisDirClus, &file, NULL, NULL);
		if (!doCheck) {
			error(2,progArgs[1]);
		} else if ((file.shFi.trait & ATTR_DIRECTORY) == ATTR_DIRECTORY) {
			error(5, progArgs[1]);
		} else {
			fileClus = retFileClus(&file);
			ptrFile = theOpen->find(theOpen, fileClus);
			if (ptrFile != NULL) {
				theOpen->remove(theOpen, fileClus);
			} else {
				error(6,progArgs[1]);
			}
		}
	} else {
			error(9, progArgs[0]);
	}
	return 0;
}


int readCmd(char **progArgs) { 
	union dirEntry file;
	struct node *ptrFile;
	int doCheck;
	unsigned int myFile, myDir, loc, size, sizeFile, offset;
	char *fNames;
	
	if (progArgs[1] == NULL) {
		error(13, progArgs[0]);
	} else if (progArgs[2] == NULL) {
		error(13, progArgs[0]);
	} else if (progArgs[3] == NULL) {
		error(13, progArgs[0]);
	} else {
		fNames = progArgs[1];
		loc = strtoul(progArgs[2], NULL, 10);
		size = strtoul(progArgs[3], NULL, 10);
		doCheck = lookupFile(fNames, thisDirClus, &file, &myDir, &offset);
		if (!doCheck) {
			error(2,fNames);
		} else if ((file.shFi.trait & ATTR_DIRECTORY) == ATTR_DIRECTORY) {
			error(5, fNames);
		} else {
			myFile = retFileClus(&file);
			ptrFile = theOpen->find(theOpen, myFile);
			sizeFile = retSize(&file);
			if (ptrFile == NULL) {
				error(6, fNames);
			} else if (loc + size > sizeFile) {
				printf("Error: %u + %u > %u: Cannot read beyond EOF\n", loc, size, sizeFile);
			} else if (!readCheck(ptrFile)) {
				error(15, fNames);
			} else {
				file.shFi.dateLast = retTime();
				setDirEntry(&file, myDir, offset);
				fileR(&file, loc, size);
				printf("\n");
			}
		}
	}
	return 0;
}

int writeCmd(char **progArgs) {
	unsigned int myFile, myDir, loc, size, offset, lengthStr, sizeOrig;
	union dirEntry file;
	struct node *ptrToFile;
	char *fNames, *str;
	int doCheck;
	
	
	if (progArgs[1] == NULL) {
		error(14, progArgs[0]);
	} else if (progArgs[2] == NULL) {
		error(14, progArgs[0]);
	} else if (progArgs[3] == NULL) {
		error(14, progArgs[0]);
	}  else if (progArgs[4] == NULL) {
		error(14, progArgs[0]);
	} else {
		fNames = progArgs[1];
		str = progArgs[4];
		sizeOrig = strtoul(progArgs[3], NULL, 10);
		lengthStr = strlen(str);
		loc = strtoul(progArgs[2], NULL, 10);
		
		
		if (lengthStr <= sizeOrig) {
			size = lengthStr;
		} else {
			size = sizeOrig;
		}
		
		
		doCheck = lookupFile(fNames, thisDirClus, &file, &myDir, &offset);
		if (!doCheck) {
			error(2,fNames);
		} else if ((file.shFi.trait & ATTR_DIRECTORY) == ATTR_DIRECTORY) {
			error(5, fNames);
		} else {
			myFile = retFileClus(&file);
			ptrToFile = theOpen->find(theOpen, myFile);
			if (ptrToFile == NULL) {
				error(6, fNames);
			} else if (!writeCheck(ptrToFile)) {
				error(16, fNames);
			} else if (loc > UINT_MAX - size) {
				printf("Error: %u + %u > %u: Cannot make file too large\n", loc, size, UINT_MAX);
			} else {
				fileW(&file, loc, size, str);
				file.shFi.dateLast = retTime();
				setDirEntry(&file, myDir, offset);
			}
		}
	}
	return 0;
}

// MYUTILITY.H ****************************************** END




// MYFILES.H ****************************************** START

int setDirEntry(union dirEntry *ptr, unsigned int dirClus, unsigned int entryDig) {
	unsigned int entryByteOff;
	unsigned int firstDirClus = retSecClus(dirClus);
	
	if (((entryByteOff = 32*entryDig) > imageData.bps*imageData.spc) || ((entryByteOff = 32*entryDig) == imageData.bps*imageData.spc)) {
		
		return 0;
	}
	
	long addVal = entryByteOff + firstDirClus*imageData.bps;
	wChar(ptr, addVal, sizeof(union dirEntry));
	
	return 1;
}

int retDirEntry(union dirEntry *ptr, unsigned int dirClus, unsigned int entryDig) {
	unsigned int entryByteOff;
	unsigned int firstDirClus = retSecClus(dirClus);
	
	if (((entryByteOff = 32*entryDig) > imageData.bps*imageData.spc) || ((entryByteOff = 32*entryDig) == imageData.bps*imageData.spc)) {
		
		return 0;
	}
	long addVal = entryByteOff + firstDirClus*imageData.bps;
	rChar(ptr, addVal, sizeof(union dirEntry));
	
	return 1;
}


int retNextDirEntry(union dirEntry *ptr, unsigned int dirClus, unsigned int entryDig) {
	unsigned int firstDirClus, entryByteOff;
	unsigned int nextDig = entryDig + 1;
	
	if (((entryByteOff = 32*nextDig) > imageData.bps*imageData.spc) || ((entryByteOff = 32*nextDig) == imageData.bps*imageData.spc)) {
		firstDirClus = retSecClus(retFatNextClus(dirClus));
		entryByteOff = entryByteOff - imageData.bps*imageData.spc;
		
	} else {
		firstDirClus = retSecClus(dirClus);
	}
	
	long addVal = entryByteOff + firstDirClus*imageData.bps;
	rChar(ptr, addVal, sizeof(union dirEntry));
	
	return 1;
}

int makeFile(char *nameFile, unsigned int dirClus) {
	unsigned int clus, offset;
	union dirEntry file;
	makeDirEntry(nameFile, dirClus, &file, &clus, &offset, 1);
	file.shFi.trait = 0x00;
	setDirEntry(&file, clus, offset);
	return 1;
}

int makeDir(char *dir_name, unsigned int dirClus) {
	unsigned int clus, offset, dClus1, dOff1, dClus2, dOff2, newDirClus;
	union dirEntry file, dFile1, dFile2; 
	makeDirEntry(dir_name, dirClus, &file, &clus, &offset, 1);
	file.shFi.trait = 0x10;
	setDirEntry(&file, clus, offset);
	newDirClus = retFileClus(&file);
	makeDirEntry(".", newDirClus, &dFile1, &dClus1, &dOff1, 0);
	dFile1.shFi.initHiClus = retHiVal(newDirClus);
	dFile1.shFi.intitLoClus = retLoVal(newDirClus);
	dFile1.shFi.trait = 0x10;
	setDirEntry(&dFile1, dClus1, dOff1);
	makeDirEntry("..", newDirClus, &dFile2, &dClus2, &dOff2, 0);
	
	
	if (dirClus != imageData.rclustr) {
		dFile2.shFi.initHiClus = retHiVal(dirClus);
		dFile2.shFi.intitLoClus = retLoVal(dirClus);
	} else {
		dFile2.shFi.initHiClus = 0x0000;
		dFile2.shFi.intitLoClus = 0x0000;
	}
	
	
	dFile2.shFi.trait = 0x10;
	setDirEntry(&dFile2, dClus2, dOff2);
	return 1;
}

int makeDirEntry(char *nameFile, unsigned int dirClus, union dirEntry *file, unsigned int *ptrClus, unsigned int *ptrOff, int lookupClusNew){
	unsigned int clus;
	char shNames[11];
	lookupOpenDirEntry(dirClus, file, ptrClus, ptrOff);
	toShortFile(nameFile, shNames);
	strncpy(file->shFi.name, shNames, 11);
	file->shFi.timeCRT = file->shFi.timeWRT = retTime();
	file->shFi.dateCRT = file->shFi.dateWRT = file->shFi.dateLast = retDate();
	file->shFi.sizeFile = 0;
	if (lookupClusNew) {
		clus = lookupOpenClus();
		file->shFi.initHiClus = retHiVal(clus);
		file->shFi.intitLoClus = retLoVal(clus);
		changeFats(clus, END_OF_CHAIN);
	}
	return 0;
}

struct list *makeList(void) {
	struct list *theList;
	theList = calloc(1, sizeof(struct list));
	theList->clear = &clearList;
	theList->add = &addList;
	theList->remove = &remList;
	theList->find = &lookList;
	theList->get_head = &headList;
	theList->empty = &emptyList;
	theList->head = NULL;
	theList->size = 0;
	return theList;
}




void clearList(struct list *theList) {
	struct node *ptr, *ptrNext;
	ptr = theList->head;
	while (ptr != NULL) {
		ptrNext = ptr->next;
		free(ptr);
		ptr = ptrNext;
	}
	theList->size = 0;
}

int remList(struct list *theList, unsigned int fileClus) {
	struct node *ptr, *previous;
	ptr = theList->head;
	while (ptr != NULL) {
		if (ptr->fileClusFST == fileClus) {
			break;
		}
		previous = ptr;
		ptr = ptr->next;
	}
	if (ptr != NULL) {
		
		
		if (ptr != theList->head) {
			previous->next = ptr->next;
		} else {
			theList->head = ptr->next;
		}
		
		
		--(theList->size);
		free(ptr);
		return 1;
	}
	return 0;
}

int emptyList(struct list *theList) {
	return !theList->size;
}

int addList(struct list *theList, unsigned int fileClus, char *mode) {
	struct node *addNode;
	unsigned char options = toByte(mode);
	
	
	if ((theList->find(theList, fileClus) == NULL)){
		if (options != 0x0){
			addNode = calloc(1, sizeof(struct node));
			addNode->fileClusFST = fileClus;
			addNode->options = options;
			addNode->next = theList->head;
			theList->head = addNode;
			++(theList->size);
			return 1;
		}
	}
	
	
	
	return 0;
}


struct node *lookList(struct list *theList, unsigned int fileClus) {
	struct node *ptr;
	ptr = theList->head;
	while (ptr != NULL) {
		if (ptr->fileClusFST == fileClus) {
			break;
		}
		ptr = ptr->next;
	}
	return ptr;
}


struct node *headList(struct list *theList) {
	return theList->head;
}

unsigned char toByte(char *mode) {
	if (strcmp(mode, "r") == 0 ) {
		return OPEN_READ;
	} else if (strcmp(mode, "w") == 0) {
		return OPEN_WRITE;
	} else if ((strcmp(mode, "rw") == 0) || (strcmp(mode, "wr") == 0)) {
		return OPEN_READ | OPEN_WRITE;
	} else {
		return OPEN_BAD;
	}
}



unsigned int retSize(union dirEntry *file) {
	unsigned int size;
	size = file->shFi.sizeFile;
	if (endianVar) {
		size = switch32(size);
	}
	return size;
}

int readCheck(struct node *file) {
	
	if ((file->options & OPEN_READ) != OPEN_READ) {
		return 0;
	} else {
		return 1;
	}
	
}

int writeCheck(struct node *file) {

	if ((file->options & OPEN_WRITE) != OPEN_WRITE) {
		return 0;
	} else {
		return 1;
	}
}

int fileR(union dirEntry *file, unsigned int location, unsigned int size) {
	
	unsigned int use;
	char *buff;
	unsigned int clusBytes = imageData.bps*imageData.spc;
	buff = malloc(sizeof(char)*(clusBytes));
	unsigned int offset = location;
	unsigned int remainBytes = size;
	unsigned int thisClus = retFileClus(file);
	while (offset > clusBytes) {
		thisClus = retFatNextClus(thisClus);
		if (chainEnd(thisClus)) {
			free(buff);
			return 0;
		}
		
		offset = offset - clusBytes;
	}
	unsigned int posByte = imageData.bps*retSecClus(thisClus) + offset;
	
	if (remainBytes <= clusBytes - offset) {
		use = remainBytes;
	} else {
		use = clusBytes - offset;
	}
	
	rChar(buff, posByte, use);
	fwrite(buff, sizeof(char), use, stdout);
	
	remainBytes = remainBytes - use;
	while (remainBytes > 0) {
		thisClus = retFatNextClus(thisClus);
		if (chainEnd(thisClus)) {
			free(buff);
			return 0;
		}
		posByte = imageData.bps*retSecClus(thisClus);
		
		
		if (remainBytes >= clusBytes) {
			use = clusBytes;
		} else {
			use = remainBytes;
		}
		
		rChar(buff, posByte, use);
		fwrite(buff, sizeof(char), use, stdout);
		remainBytes = remainBytes - use;
	}
	return 1;
}

int fileW(union dirEntry *file, unsigned int location, unsigned int size, char *str) {
	
	unsigned int use;
	unsigned int clusBytes = imageData.bps*imageData.spc;
	unsigned int offset = location;
	unsigned int remainBytes = size;
	unsigned int begin = 0;
	unsigned int thisClus = retFileClus(file);
	while (offset > clusBytes) {
		thisClus = retFatNextClus(thisClus);
		if (chainEnd(thisClus)) {
			expClus(thisClus);
		}
		
		offset = offset - clusBytes;
	}
	unsigned int posByte = imageData.bps*retSecClus(thisClus) + offset;
	
	if (remainBytes <= clusBytes - offset) {
		use = remainBytes;
	} else {
		use = clusBytes - offset;
	}
	
	wChar(&str[begin], posByte, use);
	remainBytes = remainBytes - use;
	begin = begin + use;
	while (remainBytes > 0) {
		thisClus = retFatNextClus(thisClus);
		if (chainEnd(thisClus)) {
			expClus(thisClus);
		}
		posByte = imageData.bps*retSecClus(thisClus);
		
		
		if (remainBytes >= clusBytes) {
			use = clusBytes;
		} else {
			use = remainBytes;
		}
		
		wChar(&str[begin], posByte, use);
		remainBytes = remainBytes - use;
		begin = begin + use;
	}
	if (location + size > file->shFi.sizeFile) {
		file->shFi.sizeFile = location + size;
	}
	return 1;
}

int clustDel(unsigned int fileClus) {
	unsigned int actualNum;
	actualNum = retFatNextClus_true(fileClus);
	actualNum &= 0xF0000000;
	changeFats(fileClus, actualNum);
	return 1;
}

int fileDel(union dirEntry *ptrToFile, unsigned int dirClus, unsigned int entryDig) {
	union dirEntry fileNext;
	struct list *theClus;
	struct node *theClusNode;
	unsigned int fileClus;
	theClus = makeList();
	fileClus = retFileClus(ptrToFile);
	do {
		theClus->add(theClus, fileClus, "r");
		fileClus = retFatNextClus(fileClus);
	} while (!chainEnd(fileClus));
	
	while (!theClus->empty(theClus)) {
		theClusNode = theClus->get_head(theClus);
		clustDel(theClusNode->fileClusFST);
		theClus->remove(theClus, theClusNode->fileClusFST);
	}

	retNextDirEntry(&fileNext, dirClus, entryDig);
	
	if (fileNext.natBytes[0] != 0x00) {
		ptrToFile->natBytes[0] = 0xE5;
	} else {
		ptrToFile->natBytes[0] = 0x00;
	}
	
	theClus->clear(theClus);
	free(theClus);
	setDirEntry(ptrToFile, dirClus, entryDig);
	return 1;
}



// MYFILES.H ****************************************** END


// MYPARSER.H ****************************************** START

char *readIn() {
	char *progLine = calloc(INPUT_BUFFER_SIZE, sizeof(char));
	
	if (!fgets(progLine, INPUT_BUFFER_SIZE, stdin)) {
		return NULL;
	} else {
		return progLine;
	}
	
}

char **parseFunc(char *line) {
	char **args;
	line = parseFuncWhite(line);
	args = parseFuncArgs(line);
	return args;
}


char *parseFuncWhite(char *progLine) {
	remLeadWhite(progLine);
	remMidWhite(progLine);
	remTrailWhite(progLine);
	return progLine;
}

char *addMidWhite(char *progLine){
	char tmp[INPUT_BUFFER_SIZE];
	char *ptr = progLine;
	int i = 0;
	while (*ptr != 0) {
		
		if (*ptr == '|' || *ptr == '<' || *ptr == '>' || *ptr == '&') {
			if (i > 0 && tmp[i - 1] != ' ') {
				tmp[i++] = ' ';
			}
			tmp[i++] = *ptr;
			if (*(ptr + 1) != ' ') {
				tmp[i++] = ' ';
			} else if ((ptr + 1) != 0) {
				tmp[i++] = ' ';
				++ptr;
			}
			++ptr;
			break;

		} else {
			tmp[i++] = *ptr;
			++ptr;
		}
		
		
	}
	tmp[i] = '\0';
	if (!trackCmdSize(tmp)) {
		
	}
	strcpy(progLine, tmp);
	return progLine;
}

char *remLeadWhite(char *progLine){
	char *tmp = progLine, *ptr = progLine;
	short notWhite = 0;
	while (*ptr != 0) {
		if (!isspace(*ptr)){
			notWhite = 1;
		}
		if (notWhite) {
			*tmp = *ptr;
			tmp++;
		}
		ptr++;
	}
	*tmp = '\0';
	return progLine;
}


char *remMidWhite(char *progLine){
	char *tmp = progLine, *ptr = progLine;
	int checkQuotes = 0;
	int counterWhite = 0;
	while (*ptr != 0) {
		
		
		if (checkQuotes){
			*tmp = *ptr;
			tmp++;
			counterWhite = 0;
		} else if (*ptr == '"'){
			*tmp = *ptr;
			tmp++;
			
			
			if (checkQuotes != 0) {
				checkQuotes = 0;
			} else {
				if (checkLastQuote(ptr)){
					printf("Error: Closing quotation missing\n");
					progLine[0] = 0;
					break;	
				}
				checkQuotes = 1;
			}

		} else if (isspace(*ptr)) {
			if (counterWhite == 0) {
				*tmp = ' ';
				tmp++;
			}
			counterWhite++;
		} else {
			*tmp = *ptr;
			tmp++;
			counterWhite = 0;
		}
		
		
		ptr++;
	}
	*tmp = '\0';
	return progLine;
}

char *remTrailWhite(char *progLine){
	char *last = progLine, *ptr = progLine;
	while (*ptr != 0) {
		if (!isspace(*ptr)) {
			last = ptr;
		}
		ptr++;
	}
	*(++last) = '\0';
	return progLine;
}

char **parseFuncArgs(char *progLine) {
	size_t argQuantity;
	char *lineTemp;
	int i = 0;
	int offset, location;
	char **progArgs;
	argQuantity = 0;
	argQuantity = trackArgs(progLine);
	progArgs = calloc(argQuantity + 1, sizeof(char *));
	lineTemp = strndup(progLine, strlen(progLine) + 1);
	location = 0;
	offset = strcspn(&lineTemp[location], " ");
	while (offset != 0) {
		progArgs[i] = (char *)malloc(offset + 1);
		strncpy(progArgs[i], &lineTemp[location], offset);
		progArgs[i++][offset] = '\0';
		if (lineTemp[location + offset] == '\0') {
			break;
		}
		if (lineTemp[location + offset + 1] == '\"') {
			location = location + offset + 2;
			offset = strcspn(&lineTemp[location], "\"");
			progArgs[i] = (char *)malloc(offset + 1);
			strncpy(progArgs[i], &lineTemp[location], offset);
			progArgs[i++][offset] = '\0';
			++offset;
		}
		location = location + offset + 1;
		if (lineTemp[location - 1] == '\0') {
			break;
		}
		offset = strcspn(&lineTemp[location], " ");
	}
	
	progArgs[i] = NULL;
	free(lineTemp);
	return progArgs;
}


size_t trackArgs(char *progLine) {
	int i, checkQuotes;
	size_t counter;
	checkQuotes = 0;
	counter = 0;
	
	i = 0;
	while (progLine[i] != '\0') {
		
		
		if (progLine[i] == '\"' && !checkQuotes) {
			++counter;
			checkQuotes = 1;
		} else if (progLine[i] == '\"' && checkQuotes) {
			checkQuotes = 0;
		} else if (progLine[i] == ' ' && !checkQuotes) {
			++counter;
		}
		
		
		
		++i;
	}
	
	
	return counter + 1;
}

int trackCmdSize(char *progLine) {
	int i;
	
	
	i = 0;
	while (i < INPUT_BUFFER_SIZE) {
		if (progLine[i] == '\0') {
			return 1;
		}
		++i;
	}
	
	return 0;
}


int checkLastQuote(char* ptr){
	char* ptrInitial = ptr++;
	while (*ptr != 0){
		if (*ptr == '"'){
			ptr = ptrInitial;
			return 0;
		}
		++ptr;
	}
	ptr = ptrInitial;
	return 1;
}


// MYPARSER.H ****************************************** END



// MYSTARTER.H ****************************************** START
void displayPrompt(void) {
	printf("%s] ", thisDir);
}

int toStart(char *fNamesImage, char *nameRun) {
	if ((fatImage = fopen(fNamesImage, "r+")) == NULL) {
		perror(NULL);
		return 0;
	}
	endianVar = endianSee();
	getFatInfo();
	setRootDir();
	setOpened();

	return 1;
}

int setOpened(void) {
	theOpen = makeList();
	return 1;
}

int getFatInfo(void) {
	readUnSh(&imageData.bps, 11);
	readUnCh(&imageData.spc, 13);
	readUnSh(&imageData.rsvd_s_c, 14);
	readUnCh(&imageData.n_fat, 16);
	readUnSh(&imageData.rec, 17);
	readUnInt(&imageData.tsec32, 32);
	readUnInt(&imageData.fsz32, 36);
	readUnSh(&imageData.other_options, 40);
	readUnInt(&imageData.rclustr, 44);
	return 0;
}


int setRootDir(void) {
	unsigned int funcDataSec = imageData.tsec32 - (imageData.rsvd_s_c + (imageData.n_fat*imageData.fsz32));
	numClus = funcDataSec/imageData.spc;
	dataSec = imageData.rsvd_s_c + (imageData.n_fat*imageData.fsz32);
	rootSec = retSecClus(imageData.rclustr);
	thisDirClus = imageData.rclustr;
	thisDirSec = rootSec;
	thisDirCap = INIT_CUR_DIR_CAP;
	thisDir = calloc(thisDirCap, sizeof(char));
	strcat(thisDir, "/");
	return 1;
}




// MYSTARTER.H ****************************************** END


// MYHELPERS.H ****************************************** START

unsigned short switch16(unsigned short val) {
	unsigned int switchVal = (val<<8) | (val>>8);
	return switchVal;
}

unsigned int switch32(unsigned int val) {
	unsigned int switchVal = ((val>>24)&0xff) | ((val<<8)&0xff0000) | ((val>>8)&0xff00) | ((val<<24)&0xff000000);
	return switchVal;
}

int endianSee(void) {
	int num = 1;
	
	if (*(char *)&num != 1) {
		return 1;
	} else {
		return 0;
	}
}

int chainEnd(unsigned int clus) {

	if ((clus & END_OF_CHAIN) != END_OF_CHAIN) {
		return 0;
	} else {
		return 1;
	}
}

int rChar(void *ptr, long pos, size_t use) {
	long offset;
	offset = pos - ftell(fatImage);
	if (fseek(fatImage, offset, SEEK_CUR) == -1) {
		perror(NULL);
		return 0;
	}
	if (fread(ptr, sizeof(char), use, fatImage) != use) {
		perror(NULL);
		return 0;
	}
	return 1;
}

unsigned int *readUnInt(unsigned int *ptr, long pos) {
	long offset;
	offset = pos - ftell(fatImage);
	if (fseek(fatImage, offset, SEEK_CUR) == -1) {
		perror(NULL);
		return NULL;
	}
	if (fread(ptr, sizeof(unsigned int), 1, fatImage) != 1) {
		perror(NULL);
		return NULL;
	}
	if (endianVar) {
		*ptr = switch32(*ptr);
	}
	return ptr;
}

unsigned short *readUnSh(unsigned short *ptr, long pos) {
	long offset;
	offset = pos - ftell(fatImage);
	if (fseek(fatImage, offset, SEEK_CUR) == -1) {
		perror(NULL);
		return NULL;
	}
	if (fread(ptr, sizeof(unsigned short), 1, fatImage) != 1) {
		perror(NULL);
		return NULL;
	}
	if (endianVar) {
		*ptr = switch16(*ptr);
	}
	return ptr;
}

unsigned char *readUnCh(unsigned char *ptr, long pos) {
	long offset;
	offset = pos - ftell(fatImage);
	if (fseek(fatImage, offset, SEEK_CUR) == -1) {
		perror(NULL);
		return NULL;
	}
	if (fread(ptr, sizeof(char), 1, fatImage) != 1) {
		perror(NULL);
		return NULL;
	}
	return ptr;
}

int wChar(void *ptr, long pos, size_t use) {
	long offset;
	offset = pos - ftell(fatImage);
	if (fseek(fatImage, offset, SEEK_CUR) == -1) {
		perror(NULL);
		return 0;
	}
	if (fwrite(ptr, sizeof(char), use, fatImage) != use) {
		perror(NULL);
		return 0;
	}
	return 1;
}

unsigned int *writeUnInt(unsigned int *ptr, long pos) {
	long offset;
	unsigned int tmp;
	offset = pos - ftell(fatImage);
	
	if (!endianVar) {
		tmp = *ptr;
	} else {
		tmp = switch32(*ptr);
	}
	
	
	if (fseek(fatImage, offset, SEEK_CUR) == -1) {
		perror(NULL);
		return NULL;
	}
	if (fwrite(&tmp, sizeof(unsigned int), 1, fatImage) != 1) {
		perror(NULL);
		return NULL;
	}
	return ptr;
}


unsigned short *writeUnSh(unsigned short *ptr, long pos) {
	long offset;
	unsigned short tmp;
	offset = pos - ftell(fatImage);
	
	if (!endianVar) {
		tmp = *ptr;
	} else {
		tmp = switch16(*ptr);
	}
	
	if (fseek(fatImage, offset, SEEK_CUR) == -1) {
		perror(NULL);
		return NULL;
	}
	if (fwrite(&tmp, sizeof(unsigned short), 1, fatImage) != 1) {
		perror(NULL);
		return NULL;
	}
	return ptr;
}

int changeFats(unsigned int fileClus, unsigned int value) {
	unsigned long location;
	int i;
	
	
	i = 0;
	while (i < imageData.n_fat) {
		location = retFatClusPos(fileClus, i);
		writeUnInt(&value, location);
		++i;
	}
	return 1;
}

int expClus(unsigned int oldClus1) {
	unsigned int new_clus;
	new_clus = lookupOpenClus();
	if (new_clus != 0) {
		changeFats(oldClus1, new_clus);
		changeFats(new_clus, END_OF_CHAIN);
	}
	return 1;
}

unsigned long retFatClusPos(unsigned int clus, unsigned int fat) {
	unsigned int i, beginSecFAT;
	
	if (fat < imageData.n_fat) {
		i = fat;
	} else {
		i = 0;
	}
	beginSecFAT = imageData.rsvd_s_c + i*imageData.fsz32;
	return beginSecFAT*imageData.bps + 4*clus;
}

unsigned int retSecClus(unsigned int clus) {
	unsigned int val = (clus - 2)*imageData.spc + dataSec;
	return val;
}


unsigned int retFatNextClus(unsigned int clus) {
	unsigned int nextClus;
	unsigned long location = retFatClusPos(clus, 0);
	readUnInt(&nextClus, location);
	return nextClus & NEXT_CLUS_MASK;
}


unsigned int retFatNextClus_true(unsigned int clus) {
	
	unsigned int nextClus;
	unsigned long location = retFatClusPos(clus, 0);
	readUnInt(&nextClus, location);
	return nextClus;
}

unsigned short retHiVal(unsigned int clus) {
	
	unsigned short val;
	val = (unsigned short)clus >> 16;
	if (endianVar) {
		val = switch16(val);
	}
	return val;
	
}


unsigned short retLoVal(unsigned int clus) {
	unsigned short val;
	val = (unsigned short)clus & 0xFFFF;
	if (endianVar) {
		val = switch16(val);
	}
	return val;
	
}


unsigned short retTime(void){
	time_t timeFatForm;
	struct tm * timeNow;
	unsigned short timeVal;

	time(&timeFatForm);
	timeNow = localtime(&timeFatForm);
	timeVal = ((timeNow->tm_hour*0x0800)+(timeNow->tm_min*0x0020)+(timeNow->tm_sec/2));
	if (endianVar) {
		timeVal = switch16(timeVal);
	}
	return timeVal;
}


unsigned short retDate(void){
	time_t timeFatForm;
	struct tm * timeNow;
	unsigned short dateVal;

	time(&timeFatForm);
	timeNow = localtime(&timeFatForm);
	dateVal = (((timeNow->tm_year-80)*0x0200)+((timeNow->tm_mon+1)*0x0020)+(timeNow->tm_mday));
	if (endianVar) {
		dateVal = switch16(dateVal);
	}
	return dateVal;
}

unsigned int retFileClus(union dirEntry *ptr) {
	unsigned int fileClus;
	unsigned short hi, lo;
	hi = ptr->shFi.initHiClus;
	lo = ptr->shFi.intitLoClus;
	if (endianVar) {
		hi = switch16(hi);
		lo = switch16(lo);
	}
	fileClus = (hi << 16) | (lo & 0xFFFF);
	return fileClus;
}


int lookupFile(char *fNames, unsigned int dirClus, union dirEntry *ptr, unsigned int *ptrClus, unsigned int *ptrOff) {
	unsigned int thisClus, i, bound, finish;
	union dirEntry file;
	char shNames[11];
	toShortFile(fNames, shNames);
	thisClus = dirClus;
	bound = imageData.bps*imageData.spc/32;
	finish = 0;
	do {
		for (i = 0; i < bound; ++i) {
			retDirEntry(&file, thisClus, i);
			if (file.natBytes[0] == 0x00) {
				finish = 1;
				break;
			} else if (file.natBytes[0] == 0xE5) {
				continue;
			} else if ((file.lonFi.trait & ATTR_LONG_NAME_MASK) == ATTR_LONG_NAME) {
				continue;
			} else if ((file.shFi.trait & (ATTR_DIRECTORY | ATTR_VOLUME_ID)) == ATTR_VOLUME_ID) {
				continue;
			} else if ((strncmp(file.shFi.name, shNames, 11) == 0)) {
				*ptr = file;
				if (ptrClus != NULL) {
					*ptrClus = thisClus;
				}
				if (ptrOff != NULL) {
					*ptrOff = i;
				}
				return 1;
			}
		}
		thisClus = retFatNextClus(thisClus);
	} while (!chainEnd(thisClus) && !finish);
	return 0;
}


unsigned int lookupOpenClus() {
	unsigned int val;	
	unsigned int searched = 1;	
	unsigned int i;			
	i = 2;
	while (i < numClus) {
		val = retFatNextClus(i);
		if (val == 0){
			searched = 0;
			break;
		}
		++i;
	}
	
	
	if (searched == 1){
		printf("Error: No space available on image\n");
		return 0;
	}
	return i;
}


unsigned int lookupOpenDirEntry(unsigned int dirClus, union dirEntry *ptr, unsigned int *ptrClus, unsigned int *ptrOff) {
	unsigned int i;
	union dirEntry file;
	unsigned int thisClus = dirClus;
	unsigned int bound = imageData.bps*imageData.spc/32;
	unsigned int searched = 0;
	do {
		
		for (i = 0; i < bound; ++i) {
			retDirEntry(&file, thisClus, i);
			
			
			if ((file.shFi.trait & (ATTR_DIRECTORY | ATTR_VOLUME_ID)) == ATTR_VOLUME_ID) {
				continue;
			} else if ((file.lonFi.trait & ATTR_LONG_NAME_MASK) == ATTR_LONG_NAME) {
				continue;
			} else if (file.natBytes[0] == 0x00) {
				setEntryNull(thisClus, i);
				*ptr = file;
				searched = 1;
				break;
			} else if (file.natBytes[0] == 0xE5) {
				*ptr = file;
				searched = 1;
				break;
			}
			
		}
		
		
		if (searched) {
			if (ptrClus != NULL) {
				*ptrClus = thisClus;
			}
			if (ptrOff != NULL) {
				*ptrOff = i;
			}
			return 1;
		}
		thisClus = retFatNextClus(thisClus);
	} while (!chainEnd(thisClus));
	return 0;
}

int emptyDir(union dirEntry *dir) {
	unsigned int thisClus, i, j, bound, finish;
	union dirEntry file;
	thisClus = retFileClus(dir);
	bound = imageData.bps*imageData.spc/32;
	finish = 0;
	j = 0;
	do {
		for (i = 0; i < bound; ++i, ++j) {
			retDirEntry(&file, thisClus, i);
			
			if ( j == 0 ) {
				continue;
			} else if (j == 1) {
				continue;
			} else if (file.natBytes[0] == 0x00) {
				return 1;
			} else if (file.natBytes[0] == 0xE5) {
				continue;
			} else {
				return 0;
			}
			
			
			
		}
		thisClus = retFatNextClus(thisClus);
	} while (!chainEnd(thisClus) && !finish);
	return 1;
}

int shortLow(char fNames[12], char shNames[11]) {
	int i;
	int j;
	
	i = 0;
	j = 0;
	while (i < 11) {
		if (shNames[i] != ' ') {
			if (i == 8) {
				fNames[j++] = '.';
			}
			fNames[j++] = tolower(shNames[i]);
		}
		++i;
	}
	
	j = j;
	while (j < 12) {
		fNames[j] = '\0';
		++j;
	}
	
	
	return 1;
}


int toShortFile(char fNames[12], char shNames[11]) {
	int i;
	int j;
	
	if (strcmp(fNames, "..") == 0) {
		strcpy(shNames, "..         ");
		return 1;
	} else if (strcmp(fNames, ".") == 0) {
		strcpy(shNames, ".          ");
		return 1;
	}
	
	
	
	i = 0, j = 0;
	while (i < 8 && fNames[j] != '.' && fNames[j] !='\0') {
		shNames[i++] = toupper(fNames[j++]);
	}
	
	
	i = i;
	while (i < 8) {
		shNames[i] = ' ';
		i++;
	}
	
	
	
	if (fNames[j++] == '.') {
		while (i < 11 && fNames != '\0') {
			shNames[i++] = toupper(fNames[j++]);
		}
	}
	
	i = i;
	while (i < 11) {
		shNames[i] = ' ';
		++i;
	}
	
	
	return 1;
}

int setEntryNull(unsigned int dirClus, unsigned int entryDig) {
	union dirEntry file;
	unsigned int firstDirClus, entryByteOff;
	unsigned int nextDig = entryDig + 1;
	
	if (((entryByteOff = 32*nextDig) > imageData.bps*imageData.spc) || ((entryByteOff = 32*nextDig) == imageData.bps*imageData.spc)) {
		if (chainEnd(retFatNextClus(dirClus))) {
			expClus(dirClus);
		}
		firstDirClus = retSecClus(retFatNextClus(dirClus));
		entryByteOff = entryByteOff - imageData.bps*imageData.spc;
	} else {
		firstDirClus = retSecClus(dirClus);
	}
	
	
	
	rChar(&file, entryByteOff + firstDirClus*imageData.bps, sizeof(union dirEntry));
	file.natBytes[0] = 0x00;
	wChar(&file, entryByteOff + firstDirClus*imageData.bps, sizeof(union dirEntry));
	return 1;
}


// MYHELPERS.H ****************************************** END

