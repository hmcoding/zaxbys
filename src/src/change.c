#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <time.h>



#include "clean.h"
#include "parse.h"
#include "setup.h"
#include "file_types.h"
#include "execute.h"
#include "file_commands.h"
#include "directory_commands.h"
#include "tools.h"
#include "shell_error.h"




// VARIABLES FOR ALL FILES

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



// END VARIABLES


// CLEAN



void loopClean(char *progLine, char **progArgs) {
	ridCmdLine(progLine);
	ridCmdArgs(progArgs);
}

void globClean(void) {
	delList(theOpen);
	free(thisDir);
	fclose(fatImage);
}
// END CLEAN

// DIRECTORY




int cdCmd(char **progArgs) {
	int check;
	union dirEntry file;
	if (progArgs[1] != NULL) {
		check = lookupFile(progArgs[1], thisDirClus, &file, NULL, NULL);
		if (!check) {
			error_cd_not_here(progArgs[1]);
		} else if((file.sf.attr & ATTR_DIRECTORY) != ATTR_DIRECTORY) {
			error_cd_file(progArgs[1]);
		} else { // it's a directory
			changeDirClus(&file);
			changeCurDir(&file);
		}
	}
	return 0;
}

/* prints the entries in the given directory in progArgs[1]
 */
int lsCmd(char **progArgs) {
	int check;
	union dirEntry file;
	unsigned int DirClus;
	if (progArgs[1] == NULL) {
		DirClus = thisDirClus;
	} else {
		check = lookupFile(progArgs[1], thisDirClus, &file, NULL, NULL);
		if (!check) {
			error_cd_not_here(progArgs[1]);
			return 0;
		} else if ((file.sf.attr & ATTR_DIRECTORY) != ATTR_DIRECTORY) {
			error_cd_file(progArgs[1]);
			return 0;
		} else {
			DirClus = retFileClus(&file);
		}
	}
	displayDir(DirClus);
	printf("\n");
	return 0;
}

int mkdirCmd(char **progArgs) {
	int check;
	union dirEntry file;
	unsigned int DirClus, offset;
	if (progArgs[1] == NULL) {
		error_specify_file(progArgs[0]);
	} else {
		check = lookupFile(progArgs[1], thisDirClus, &file, &DirClus, &offset);
		if (check) {
			error_file_or_directory_exists(progArgs[1]);
		} else {
			makeDir(progArgs[1], thisDirClus);
		}
	}
	return 0;
}

int rmdirCmd(char **progArgs) {
	int check;
	char *nameFile;
	union dirEntry file;
	unsigned int DirClus, offset;
	nameFile = progArgs[1];
	if (nameFile == NULL) {
		error_specify_directory(progArgs[0]);                
	} else {
		check = lookupFile(progArgs[1], thisDirClus, &file, &DirClus, &offset);
		if (!check) {
			error_open_no_file(progArgs[1]);
		} else if ((file.sf.attr & ATTR_DIRECTORY) != ATTR_DIRECTORY) {
			error_bad_directory(progArgs[1]);
		} else if (!emptyDir(&file)) {
			error_not_empty(progArgs[1]);
		} else {
			fileDel(&file, DirClus, offset);
		}
	}
	return 0;
}

int displayDir(unsigned int dirClus) {
	unsigned int thisClus, i, bound, finish;
	union dirEntry file;
	char fNames[12];
	thisClus = dirClus;
	bound = imageData.bytes_per_sec*imageData.sec_per_clus/32;
	finish = 0;
	do {
		for (i = 0; i < bound; ++i) {
			retDirEntry(&file, thisClus, i);
			if (file.raw_bytes[0] == 0x00) {
				finish = 1;
				break;
			} else if (file.raw_bytes[0] == 0xE5) {
				continue;
			} else if ((file.lf.attr & ATTR_LONG_NAME_MASK) == ATTR_LONG_NAME) {
				continue;
			} else if ((file.sf.attr & (ATTR_DIRECTORY | ATTR_VOLUME_ID)) == 0x00) {
				shortLow(fNames, file.sf.name);
				printf("%s\t", fNames);
			} else if ((file.sf.attr & (ATTR_DIRECTORY | ATTR_VOLUME_ID)) == ATTR_DIRECTORY) {
				shortLow(fNames, file.sf.name);
				printf("%s/\t", fNames);
			} else if ((file.sf.attr & (ATTR_DIRECTORY | ATTR_VOLUME_ID)) == ATTR_VOLUME_ID) {
				continue;
			} else {
				// bad file
			}
		}
		thisClus = retFatNextClus(thisClus);
	} while (!chainEnd(thisClus) && !finish);
	return 1;
}

int changeDirClus(union dirEntry *ptr) {
	unsigned int fileClus;
	if ((ptr->sf.attr & ATTR_DIRECTORY) == ATTR_DIRECTORY) {
		fileClus = retFileClus(ptr);
		if (fileClus == 0) {
			thisDirClus = imageData.root_clus;
		} else {
			thisDirClus = fileClus;
		}
	}
	return 1;
}

int changeCurDir(union dirEntry *ptr) {
	char fNames[12];
	unsigned int slashNTL;
	if ((ptr->sf.attr & ATTR_DIRECTORY) == ATTR_DIRECTORY) {
		shortLow(fNames, ptr->sf.name);
		if (retFileClus(ptr) == 0) {
			strcpy(thisDir, "/");
		} else if (strcmp(fNames, ".") == 0) {
			// do nothing
		} else if (strcmp(fNames, "..") == 0) {
			slashNTL = lookupNextToLastSlash();
			thisDir[slashNTL + 1] = '\0';
		} else {
			if (strlen(thisDir) + strlen(fNames) + 1 > thisDirCap) {
				thisDirCap *= 2;
				thisDir = realloc(thisDir, thisDirCap);
			}
			strcat(thisDir, fNames);
			strcat(thisDir, "/");
		}
	}
	return 1;
}

unsigned int lookupNextToLastSlash() {
	unsigned int lastSlash, ntlSlash, i;
	lastSlash = 0, ntlSlash = 0;
	for (i = 1; thisDir[i] != '\0'; ++i) {
		if (thisDir[i] == '/') {
			ntlSlash = lastSlash;
			lastSlash = i;
		}
	}
	return ntlSlash;
}

// END DIRECTORY

// EXECUTE

/* userCmd
 *
 */
int userCmd(char **progArgs) {
	if (progArgs[0] == NULL) {
		// do nothing
	} else if (strcmp(progArgs[0], "open") == 0) {
		openCmd(progArgs);
	} else if (strcmp(progArgs[0], "close") == 0) {
		closeCmd(progArgs);
	} else if (strcmp(progArgs[0], "create") == 0) {
		createCmd(progArgs);
	} else if (strcmp(progArgs[0], "rm") == 0) {
		rmCmd(progArgs);
	} else if (strcmp(progArgs[0], "size") == 0) {
		sizeCmd(progArgs);
	} else if (strcmp(progArgs[0], "cd") == 0) {
		cdCmd(progArgs);
	} else if (strcmp(progArgs[0], "ls") == 0) {
		lsCmd(progArgs);
	} else if (strcmp(progArgs[0], "mkdir") == 0) {
		mkdirCmd(progArgs);
	} else if (strcmp(progArgs[0], "rmdir") == 0) {
		rmdirCmd(progArgs);
	} else if (strcmp(progArgs[0], "read") == 0) {
		readCmd(progArgs);
	} else if (strcmp(progArgs[0], "write") == 0) {
		writeCmd(progArgs);
	} else if (strcmp(progArgs[0], "exit") == 0) {
		 return exitCmd();
	} else if (strcmp(progArgs[0], "info") == 0) {
		infoCmd();
	}  else {
		printf("command: %s: Command is not recognized\n", progArgs[0]);
	}
	return 1;
}

// END EXECUTE



// FILE COMMANDS




int infoCmd(){
	printf("Bytes per Sector: %d\n", imageData.bytes_per_sec);
	printf("Sectors per Cluster: %d\n", imageData.sec_per_clus);
	printf("Total Sectors: %d\n",imageData.tot_sec32);
	printf("Sectors per FAT: %d\n", imageData.fat_sz32);
	printf("Number of FATs: %d\n", imageData.num_fat);

	return 0;
}



int openCmd(char **progArgs) {
	int check;
	union dirEntry file;
	unsigned int fileClus, DirClus, offset;
	struct node *ptrToFile;
	if (progArgs[1] == NULL || progArgs[2] == NULL) {
		error_specify_file_and_mode(progArgs[0]);
	} else {
		check = lookupFile(progArgs[1], thisDirClus, &file, &DirClus, &offset);
		if (!check) {
			error_open_no_file(progArgs[1]);
		} else if ((file.sf.attr & ATTR_DIRECTORY) == ATTR_DIRECTORY) {
			error_open_directory(progArgs[1]);
		} else if (toByte(progArgs[2]) == 0x0) {
			error_open_bad_param(progArgs[2]);
		} else {
			fileClus = retFileClus(&file);
			ptrToFile = theOpen->find(theOpen, fileClus);
			if (ptrToFile != NULL) {
				error_open_already(progArgs[1]);
			} else {
				file.sf.last_acc_date = retDate();
				setDirEntry(&file, DirClus, offset);
				theOpen->add(theOpen, retFileClus(&file), progArgs[2]);
			}
		}
	}
	return 0;
}

int closeCmd(char **progArgs) {
	int check;
	union dirEntry file;
	unsigned int fileClus;
	struct node *ptrToFile;
	if (progArgs[1] == NULL) {
		error_specify_file(progArgs[0]);
	} else {
		check = lookupFile(progArgs[1], thisDirClus, &file, NULL, NULL);
		if (!check) {
			error_open_no_file(progArgs[1]);
		} else if ((file.sf.attr & ATTR_DIRECTORY) == ATTR_DIRECTORY) {
			error_close_directory(progArgs[1]);
		} else {
			fileClus = retFileClus(&file);
			ptrToFile = theOpen->find(theOpen, fileClus);
			if (ptrToFile == NULL) {
				error_not_open(progArgs[1]);
			} else {
				theOpen->remove(theOpen, fileClus);
			}
		}
	}
	return 0;
}

int createCmd(char **progArgs) {
	int check;
	union dirEntry file;
	unsigned int DirClus, offset;
	if (progArgs[1] == NULL) {
		error_specify_file(progArgs[0]);
	} else {
		check = lookupFile(progArgs[1], thisDirClus, &file, &DirClus, &offset);
		if (check) {
			error_file_or_directory_exists(progArgs[1]);
		} else {
			makeFile(progArgs[1], thisDirClus);
		}
	}
	return 0;
}

int rmCmd(char **progArgs) {
	int check;
	char *nameFile;
	union dirEntry file;
	unsigned int fileClus, DirClus, offset;
	struct node *ptrToFile;
	nameFile = progArgs[1];
	if (nameFile == NULL) {
		error_specify_file(progArgs[0]);                
	} else {
		check = lookupFile(progArgs[1], thisDirClus, &file, &DirClus, &offset);
		if (!check) {
			error_open_no_file(progArgs[1]);
		} else if ((file.sf.attr & ATTR_DIRECTORY) == ATTR_DIRECTORY) {
			error_remove_directory(progArgs[1]);
		} else {
			fileClus = retFileClus(&file);
			ptrToFile = theOpen->find(theOpen, fileClus);
			if (ptrToFile != NULL) {
				theOpen->remove(theOpen, fileClus);
			}
			fileDel(&file, DirClus, offset);
		}
	}
	return 0;
}

int sizeCmd(char **progArgs) {
	int check;
	union dirEntry file;
	if (progArgs[1] == NULL) {
		error_specify_file(progArgs[0]);
	} else {
		check = lookupFile(progArgs[1], thisDirClus, &file, NULL, NULL);
		if (!check) {
			error_open_no_file(progArgs[1]);
		} else {
			printf("%u\n", retSize(&file));
		}
	}
	return 0;
}

int readCmd(char **progArgs) {
	int check;
	union dirEntry file;
	char *fNames;
	unsigned int fileClus, location, size, sizeFile, DirClus, offset;
	struct node *ptrToFile;
	if (progArgs[1] == NULL || progArgs[2] == NULL || progArgs[3] == NULL) {
		error_specify_file_pos_size(progArgs[0]);
	} else {
		fNames = progArgs[1];
		location = strtoul(progArgs[2], NULL, 10);
		size = strtoul(progArgs[3], NULL, 10);
		check = lookupFile(fNames, thisDirClus, &file, &DirClus, &offset);
		if (!check) {
			error_open_no_file(fNames);
		} else if ((file.sf.attr & ATTR_DIRECTORY) == ATTR_DIRECTORY) {
			error_close_directory(fNames);
		} else {
			fileClus = retFileClus(&file);
			ptrToFile = theOpen->find(theOpen, fileClus);
			sizeFile = retSize(&file);
			if (ptrToFile == NULL) {
				error_not_open(fNames);
			} else if (!readCheck(ptrToFile)) {
				error_not_readable(fNames);
			} else if (location + size > sizeFile) {
				error_beyond_EOF(location, size, sizeFile);
			} else {
				file.sf.last_acc_date = retTime();
				setDirEntry(&file, DirClus, offset);
				fileR(&file, location, size);
				printf("\n");
			}
		}
	}
	return 0;
}

int writeCmd(char **progArgs) {
	int check;
	union dirEntry file;
	char *fNames, *str;
	unsigned int fileClus, location, size, DirClus, offset, lengthStr, sizeOrig;
	struct node *ptrToFile;
	if (progArgs[1] == NULL || progArgs[2] == NULL || progArgs[3] == NULL || progArgs[4] == NULL) {
		error_specify_file_pos_size_str(progArgs[0]);
	} else {
		fNames = progArgs[1];
		str = progArgs[4];
		location = strtoul(progArgs[2], NULL, 10);
		lengthStr = strlen(str);
		sizeOrig = strtoul(progArgs[3], NULL, 10);
		if (lengthStr > sizeOrig) {
			size = sizeOrig;
		} else {
			size = lengthStr;
		}
		check = lookupFile(fNames, thisDirClus, &file, &DirClus, &offset);
		if (!check) {
			error_open_no_file(fNames);
		} else if ((file.sf.attr & ATTR_DIRECTORY) == ATTR_DIRECTORY) {
			error_close_directory(fNames);
		} else {
			fileClus = retFileClus(&file);
			ptrToFile = theOpen->find(theOpen, fileClus);
			if (ptrToFile == NULL) {
				error_not_open(fNames);
			} else if (!writeCheck(ptrToFile)) {
				error_not_writeable(fNames);
			} else if (location > UINT_MAX - size) {
				error_too_large(location, size);
			} else {
				fileW(&file, location, size, str);
				file.sf.last_acc_date = retTime();
				setDirEntry(&file, DirClus, offset);
			}
		}
	}
	return 0;
}



int exitCmd(void) {
	printf("exiting fat32 utility\n");
	return 0;
}



// END FILE COMMANDS



// FILE TYPES




int retDirEntry(union dirEntry *ptr, unsigned int dirClus, unsigned int entryDig) {
	unsigned int firstDirClus, entryByteOff;
	firstDirClus = retSecClus(dirClus);
	if ((entryByteOff = 32*entryDig) >= imageData.bytes_per_sec*imageData.sec_per_clus) {
		// bad offset
		return 0;
	}
	rChar(ptr, entryByteOff + firstDirClus*imageData.bytes_per_sec, sizeof(union dirEntry));
	return 1;
}

int setDirEntry(union dirEntry *ptr, unsigned int dirClus, unsigned int entryDig) {
	unsigned int firstDirClus, entryByteOff;
	firstDirClus = retSecClus(dirClus);
	if ((entryByteOff = 32*entryDig) >= imageData.bytes_per_sec*imageData.sec_per_clus) {
		// bad offset
		return 0;
	}
	wChar(ptr, entryByteOff + firstDirClus*imageData.bytes_per_sec, sizeof(union dirEntry));
	return 1;
}

int retNextDirEntry(union dirEntry *ptr, unsigned int dirClus, unsigned int entryDig) {
	unsigned int firstDirClus, entryByteOff, nextDig;
	nextDig = entryDig + 1;
	if ((entryByteOff = 32*nextDig) >= imageData.bytes_per_sec*imageData.sec_per_clus) {
		firstDirClus = retSecClus(retFatNextClus(dirClus));
		entryByteOff -= imageData.bytes_per_sec*imageData.sec_per_clus;
	} else {
		firstDirClus = retSecClus(dirClus);
	}
	rChar(ptr, entryByteOff + firstDirClus*imageData.bytes_per_sec, sizeof(union dirEntry));
	return 1;
}


// implementation of list and open file table
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

// function for free'ing all the memory
void delList(struct list *pastList) {
	pastList->clear(pastList);
	free(pastList);
}

// frees each entry in the list
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

// adds the entry with the given file cluster and read/write mode
// will only add with a valid file mode
int addList(struct list *theList, unsigned int fileClus, char *mode) {
	struct node *addNode;
	unsigned char options;
	options = toByte(mode);
	if ((theList->find(theList, fileClus) == NULL) && (options != 0x0)) {
		addNode = calloc(1, sizeof(struct node));
		addNode->fst_fileClus = fileClus;
		addNode->options = options;
		addNode->next = theList->head;
		theList->head = addNode;
		++(theList->size);
		return 1;
	}
	return 0;
}

// removes the given file cluster from the list
int remList(struct list *theList, unsigned int fileClus) {
	struct node *ptr, *previous;
	ptr = theList->head;
	while (ptr != NULL) {
		if (ptr->fst_fileClus == fileClus) {
			break;
		}
		previous = ptr;
		ptr = ptr->next;
	}
	if (ptr != NULL) {
		if (ptr == theList->head) {
			theList->head = ptr->next;
		} else {
			previous->next = ptr->next;
		}
		--(theList->size);
		free(ptr);
		return 1;
	}
	return 0;
}

// returns the node pointer to the given file cluster or NULL if it's not there
struct node *lookList(struct list *theList, unsigned int fileClus) {
	struct node *ptr;
	ptr = theList->head;
	while (ptr != NULL) {
		if (ptr->fst_fileClus == fileClus) {
			break;
		}
		ptr = ptr->next;
	}
	return ptr;
}

// returns the head of the list (for stack-like behavior)
struct node *headList(struct list *theList) {
	return theList->head;
}

int emptyList(struct list *theList) {
	return !theList->size;
}

unsigned char toByte(char *mode) {
	if (strcmp(mode, "r") == 0 ) {
		return OPEN_READ;
	} else if (strcmp(mode, "w") == 0) {
		return OPEN_WRITE;
	} else if ((strcmp(mode, "rw") == 0) | (strcmp(mode, "wr") == 0)) {
		return OPEN_READ | OPEN_WRITE;
	} else {
		return OPEN_BAD;
	}
}

int readCheck(struct node *file) {
	if ((file->options & OPEN_READ) == OPEN_READ) {
		return 1;
	} else {
		return 0;
	}
}

int writeCheck(struct node *file) {
	if ((file->options & OPEN_WRITE) == OPEN_WRITE) {
		return 1;
	} else {
		return 0;
	}
}

unsigned int retSize(union dirEntry *file) {
	unsigned int size;
	size = file->sf.sizeFile;
	if (endianVar) {
		size = switch32(size);
	}
	return size;
}

int fileR(union dirEntry *file, unsigned int location, unsigned int size) {
	unsigned int offset, remainBytes, thisClus, clusBytes, posByte, use;
	char *buff;
	clusBytes = imageData.bytes_per_sec*imageData.sec_per_clus;
	buff = malloc(sizeof(char)*(clusBytes));
	offset = location;
	remainBytes = size;
	thisClus = retFileClus(file);
	while (offset > clusBytes) {
		thisClus = retFatNextClus(thisClus);
		if (chainEnd(thisClus)) {
			free(buff);
			return 0;
		}
		offset -= clusBytes;
	}
	posByte = imageData.bytes_per_sec*retSecClus(thisClus) + offset;
	if (remainBytes > clusBytes - offset) {
		use = clusBytes - offset;
	} else {
		use = remainBytes;
	}
	rChar(buff, posByte, use);
	fwrite(buff, sizeof(char), use, stdout);
	remainBytes -= use;
	while (remainBytes > 0) {
		thisClus = retFatNextClus(thisClus);
		if (chainEnd(thisClus)) {
			free(buff);
			return 0;
		}
		posByte = imageData.bytes_per_sec*retSecClus(thisClus);
		if (remainBytes < clusBytes) {
			use = remainBytes;
		} else {
			use = clusBytes;
		}
		rChar(buff, posByte, use);
		fwrite(buff, sizeof(char), use, stdout);
		remainBytes -= use;
	}
	return 1;
}

int fileW(union dirEntry *file, unsigned int location, unsigned int size, char *str) {
	unsigned int offset, remainBytes, thisClus, clusBytes, posByte, use, begin;
	clusBytes = imageData.bytes_per_sec*imageData.sec_per_clus;
	offset = location;
	remainBytes = size;
	begin = 0;
	thisClus = retFileClus(file);
	while (offset > clusBytes) {
		thisClus = retFatNextClus(thisClus);
		if (chainEnd(thisClus)) {
			expClus(thisClus);
		}
		offset -= clusBytes;
	}
	posByte = imageData.bytes_per_sec*retSecClus(thisClus) + offset;
	if (remainBytes > clusBytes - offset) {
		use = clusBytes - offset;
	} else {
		use = remainBytes;
	}
	wChar(&str[begin], posByte, use);
	remainBytes -= use;
	begin += use;
	while (remainBytes > 0) {
		thisClus = retFatNextClus(thisClus);
		if (chainEnd(thisClus)) {
			expClus(thisClus);
		}
		posByte = imageData.bytes_per_sec*retSecClus(thisClus);
		if (remainBytes < clusBytes) {
			use = remainBytes;
		} else {
			use = clusBytes;
		}
		wChar(&str[begin], posByte, use);
		remainBytes -= use;
		begin += use;
	}
	if (location + size > file->sf.sizeFile) {
		file->sf.sizeFile = location + size;
	}
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
		clustDel(theClusNode->fst_fileClus);
		theClus->remove(theClus, theClusNode->fst_fileClus);
	}

	retNextDirEntry(&fileNext, dirClus, entryDig);
	if (fileNext.raw_bytes[0] == 0x00) {
		ptrToFile->raw_bytes[0] = 0x00;
	} else {
		ptrToFile->raw_bytes[0] = 0xE5;
	}
	delList(theClus);
	setDirEntry(ptrToFile, dirClus, entryDig);
	return 1;
}

int clustDel(unsigned int fileClus) {
	unsigned int actualNum;
	actualNum = retFatNextClus_true(fileClus);
	actualNum &= 0xF0000000;
	changeFats(fileClus, actualNum);
	return 1;
}

int makeDirEntry(char *nameFile, unsigned int dirClus, union dirEntry *file, unsigned int *ptrClus, unsigned int *ptrOff, int lookupClusNew){
	unsigned int clus;
	char shNames[11];
	lookupOpenDirEntry(dirClus, file, ptrClus, ptrOff);
	fileShort(nameFile, shNames);
	strncpy(file->sf.name, shNames, 11);
	file->sf.crt_time = file->sf.wrt_time = retTime();
	file->sf.crt_date = file->sf.wrt_date = file->sf.last_acc_date = retDate();
	file->sf.sizeFile = 0;
	if (lookupClusNew) {
		clus = lookupOpenClus();
		file->sf.first_clus_hi = retHiVal(clus);
		file->sf.first_clus_lo = retLoVal(clus);
		changeFats(clus, END_OF_CHAIN);
	}
	return 0;
}

int makeFile(char *nameFile, unsigned int dirClus) {
	unsigned int clus, offset;
	union dirEntry file;
	makeDirEntry(nameFile, dirClus, &file, &clus, &offset, 1);
	file.sf.attr = 0x00;
	setDirEntry(&file, clus, offset);
	return 1;
}

int makeDir(char *dir_name, unsigned int dirClus) {
	unsigned int clus, offset, dClus1, dOff1, dClus2, dOff2, newDirClus;
	union dirEntry file, d_file, dd_file; // CHANGE HERE
	makeDirEntry(dir_name, dirClus, &file, &clus, &offset, 1);
	file.sf.attr = 0x10;
	setDirEntry(&file, clus, offset);
	newDirClus = retFileClus(&file);
	makeDirEntry(".", newDirClus, &d_file, &dClus1, &dOff1, 0);
	d_file.sf.first_clus_hi = retHiVal(newDirClus);
	d_file.sf.first_clus_lo = retLoVal(newDirClus);
	d_file.sf.attr = 0x10;
	setDirEntry(&d_file, dClus1, dOff1);
	makeDirEntry("..", newDirClus, &dd_file, &dClus2, &dOff2, 0);
	if (dirClus == imageData.root_clus) {
		dd_file.sf.first_clus_hi = 0x0000;
		dd_file.sf.first_clus_lo = 0x0000;
	} else {
		dd_file.sf.first_clus_hi = retHiVal(dirClus);
		dd_file.sf.first_clus_lo = retLoVal(dirClus);
	}
	dd_file.sf.attr = 0x10;
	setDirEntry(&dd_file, dClus2, dOff2);
	return 1;
}



// END FILE TYPES

// PARSE

/* readIn
 *
 * Dynamically allocates memory for a c-string, reads stdin and places the input
 * into the c-string. Returns the c-string.
 */
char *readIn() {
	char *progLine = calloc(INPUT_BUFFER_SIZE, sizeof(char));
	if (fgets(progLine, INPUT_BUFFER_SIZE, stdin)) {
		return progLine;
	} else {
		//Do something about this error, NULL on return
		return NULL;
	}
}

/* parse
 * 
 * The grand parsing function which wraps around many other smaller functions
 * necessary for parsing the commands given in the terminal.
 */
char **parse(char *line) {
	char **args;
	line = parseWhite(line);
	args = parseArgs(line);
	return args;
}

/* parseWhite
 *
 * Takes a c-string, progLine, and removes unnecessary whitespace and adds
 * whitespace where needed.
 *
 * special characters: |, <, >, &, $, ~
 * don't do this for: ., /
 * whitespace: <space>, \t, \v, \f, \r (\n is used to debound commands)
 */
char *parseWhite(char *progLine) {
	remLeadWhite(progLine);
	remMidWhite(progLine);
	//addMidWhite(progLine);
	remTrailWhite(progLine);
	return progLine;
}

/* parseArgs
 *
 * Takes a c-string, progLine, and splits it into tokens debounded by space. An
 * array of strings is returned. The size of the array of strings is governed by
 * the constant PARAM_LIMIT. The array and it's elements use allocated memory
 * and should be freed later. The last element is followed by a NULL pointer to
 * mark the end of the array.
 */
char **parseArgs(char *progLine) {
	size_t argQuantity;
	char *lineTemp;
	int i = 0;
	int offset, location;
	char **progArgs;
	argQuantity = 0;
	argQuantity = trackArgs(progLine);
	// argQuantity + 1 for the null value at the end
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
			location += offset + 2;
			offset = strcspn(&lineTemp[location], "\"");
			progArgs[i] = (char *)malloc(offset + 1);
			strncpy(progArgs[i], &lineTemp[location], offset);
			progArgs[i++][offset] = '\0';
			++offset;
		}
		location += offset + 1;
		if (lineTemp[location - 1] == '\0') {
			break;
		}
		offset = strcspn(&lineTemp[location], " ");
	}
	/*token = strtok_r(lineTemp, sep, &save_ptr);
	while (token != NULL) {
		progArgs[i] = (char *) malloc(strlen(token) + 1);
		strcpy(progArgs[i++], token);
		token = strtok_r(NULL, sep, &save_ptr);
	}*/
	progArgs[i] = NULL;
	free(lineTemp);
	return progArgs;
}

/* trackArgs
 *
 * This take a c-string and counters the number of arguments which corresponds to
 * the number of spaces plus one.
 */
size_t trackArgs(char *progLine) {
	int i, checkQuotes;
	size_t counter;
	checkQuotes = 0;
	counter = 0;
	for (i = 0; progLine[i] != '\0'; ++i) {
		if (progLine[i] == ' ' && !checkQuotes) {
			++counter;
		} else if (progLine[i] == '\"' && !checkQuotes) {
			++counter;
			checkQuotes = 1;
		} else if (progLine[i] == '\"' && checkQuotes) {
			checkQuotes = 0;
		}
	}
	return counter + 1;
}

/* remLeadWhite
 *
 * Takes the input c-string and loops through string. The leading whitespace is
 * ignored while the rest of the string is shifted to the front.
 */
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

/* remMidWhite
 *
 * Takes c-string and shifts the characters so that there is at most one space
 * character between each token separated by whitespace. E.g.
 * 
 * "file1   \t \v    file2" ==> "file1 file2"
 * 
 * A null byte is added to the end of the string -- doesn't worry about the 
 * whitespace at the end.
 */
char *remMidWhite(char *progLine){
	// tmp holds place where char is modified, ptr holds place where checking
	char *tmp = progLine, *ptr = progLine;
	int checkQuotes = 0;
	int counterWhite = 0;
	while (*ptr != 0) {
		if (*ptr == '"'){
			*tmp = *ptr;
			tmp++;
			if (checkQuotes == 0){
				if (checkLastQuote(ptr)){
					error_dangling_quote();
					progLine[0] = 0;
					break;	
				}
				checkQuotes = 1;
			} else {
				checkQuotes = 0;
			} 

		} else if (checkQuotes){
			*tmp = *ptr;
			tmp++;
			counterWhite = 0;
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

/* addMidWhite
 *
 * Takes c-string and adds space character between tokens which may not have any
 * whitespace initially. Special characters include <, >, |, &. Also in
 * consideration is >>.
 */
char *addMidWhite(char *progLine){
	char tmp[INPUT_BUFFER_SIZE];
	char *ptr = progLine;
	int i = 0;
	while (*ptr != 0) {
		switch (*ptr) {
			case '|':
			case '<':
			case '>':
			case '&':
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
			default:
				tmp[i++] = *ptr;
				++ptr;
		}
	}
	tmp[i] = '\0';
	if (!trackCmdSize(tmp)) {
		// error stuff goes here
	}
	strcpy(progLine, tmp);
	return progLine;
}

/* remTrailWhite
 *
 * Takes c-string and adds the a null byte after the last non-whitespace char.
 */
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

/* trackCmdSize
 *
 * Makes sure the given string is shorter than the maximum allowed size of the
 * command string. Returns 1 if the string is good and 0 if the string is bad
 */
int trackCmdSize(char *progLine) {
	int i;
	for (i = 0; i < INPUT_BUFFER_SIZE; ++i) {
		if (progLine[i] == '\0') {
			return 1;
		}
	}
	return 0;
}

/* ridCmdLine
 * 
 * Takes a c-string and frees the memory allocated for that
 */
void ridCmdLine(char *progLine) {
	free(progLine);
}

/* ridCmdArgs
 *
 * Takes array of c-strings and frees each element and then frees the memory of
 * the array
 */
void ridCmdArgs(char **progArgs) {
	int i;
	for (i = 0; progArgs[i] != NULL; ++i) {
		free(progArgs[i]);
		progArgs[i] = NULL;
	}
	free(progArgs);
}



// END PARSE

// SETUP

// global variables



/* setup function which initializes all the global variables and opens the image
 * in read and write mode.
 */
int setup(char *fNamesImage, char *nameRun) {
	if ((fatImage = fopen(fNamesImage, "r+")) == NULL) {
		perror(NULL);
		return 0;
	}
	endianVar = check_endian();
	getFatInfo();
	setRootDir();
	setOpened();
	displayIntro(fNamesImage, nameRun);

	return 1;
}

/* extracts the vital information for traversing the fat32 image from the boot
 * sector
 */
int getFatInfo(void) {
	readUnSh(&imageData.bytes_per_sec, 11);
	readUnCh(&imageData.sec_per_clus, 13);
	readUnSh(&imageData.rsvd_sec_cnt, 14);
	readUnCh(&imageData.num_fat, 16);
	readUnSh(&imageData.root_ent_cnt, 17);
	readUnInt(&imageData.tot_sec32, 32);
	readUnInt(&imageData.fat_sz32, 36);
	readUnSh(&imageData.ext_options, 40);
	readUnInt(&imageData.root_clus, 44);
	return 0;
}

/* sets the first root directory sector, current directory sector and names, and
 * the first data sector
 */
int setRootDir(void) {
	unsigned int funcDataSec;
	funcDataSec = imageData.tot_sec32 - (imageData.rsvd_sec_cnt + (imageData.num_fat*imageData.fat_sz32));
	numClus = funcDataSec/imageData.sec_per_clus;
	dataSec = imageData.rsvd_sec_cnt + (imageData.num_fat*imageData.fat_sz32);
	rootSec = retSecClus(imageData.root_clus);
	thisDirClus = imageData.root_clus;
	thisDirSec = rootSec;
	thisDirCap = INIT_CUR_DIR_CAP;
	thisDir = calloc(thisDirCap, sizeof(char));
	strcat(thisDir, "/");
	return 1;
}

int setOpened(void) {
	theOpen = makeList();
	return 1;
}

/* prints the prompt */
void displayPrompt(void) {
	printf("%s] ", thisDir);
}

/* prints the intro message */
void displayIntro(char *fNamesImage, char *nameRun) {
	printf("Welcome to the %s shell utility\n", nameRun);
	printf("Image, %s, is ready to view\n", fNamesImage);
	printf("For a list of commands, type \"help\" or \"h\"\n");
}


// END SETUP

// ERROR

//parsing errors
void error_dangling_quote(){
	printf("Error: closing quotation missing!\n");
}


// open/close errors
void error_open_already(char *fNames) {
	printf("Error: %s: file already open!\n", fNames);
}

void error_open_no_file(char *fNames) {
	printf("Error: %s: file does not exist\n", fNames);
}

void error_open_directory(char *directory) {
	printf("Error: %s: cannot open directory\n", directory);
}

void error_open_bad_param(char *param) {
	printf("Error: %s: incorrect parameter\n", param);
}

void error_close_directory(char *directory) {
	printf("Error: %s: cannot close directory\n", directory);
}

void error_not_open(char *fNames) {
	printf("Error: %s: file not open\n", fNames);
}


// create errors
void error_used_file(char *nameFile) {
	printf("Error: %s: file already exists\n", nameFile);
}

void error_specify_file_and_mode(char *command){
	printf("Error: %s: please specify a file name and mode\n", command);
} 

void error_specify_file(char *command){
	printf("Error: %s: please specify a file name\n", command);
} 

void error_specify_directory(char *command){
	printf("Error: %s: please specify a directory name\n", command);
} 

void error_no_more_space(){
	printf("Error: No more space available on the image!\n");
}


// ls, cd errors
void error_cd_file(char *fNames) {
	printf("Error: %s: not a directory\n", fNames);
}

void error_cd_not_here(char *directory) {
	printf("Error: %s: does not exist\n", directory);
}


// read/write errors
void error_specify_file_pos_size(char *command) {
	printf("Error: %s: please specify a file name, location, and size\n", command);
}

void error_specify_file_pos_size_str(char *command) {
	printf("Error: %s: please specify a file name, location, size, and string\n", command);
}

void error_not_readable(char *fNames) {
	printf("Error: %s: this file is not open in read mode\n", fNames);
}

void error_beyond_EOF(unsigned int location, unsigned int size, unsigned int sizeFile) {
	printf("Error: %u + %u > %u: attempt to read beyond EOF\n", location, size, sizeFile);
}

void error_not_writeable(char *fNames) {
	printf("Error: %s: File is not open for writing\n", fNames);
}

void error_too_large(unsigned int location, unsigned int size) {
	printf("Error: %u + %u > %u: attempt to make file too large\n", location, size, UINT_MAX);
}


// other
void error_bad_directory(char *fNames) {
	printf("Error: %s: not a directory\n", fNames);
}

void error_not_empty(char *directory) {
	printf("Error: %s: directory not empty\n", directory);
}

void error_file_or_directory_exists(char *fNames) {
	printf("Error: %s: file or directory exists already\n", fNames);
}

void error_remove_directory(char *directory) {
	printf("Error: %s: not a file\n", directory);
}



// END ERROR

// TOOLS

// global variables used in this file





// reads use amount of characters from fatImage file at pos
int rChar(void *ptr, long pos, size_t use) {
	long offset = pos - ftell(fatImage);
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

// writes use amount of characters from fatImage file at pos
int wChar(void *ptr, long pos, size_t use) {
	long offset = pos - ftell(fatImage);
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

// reads an unsigned int from fatImage at pos
unsigned int *readUnInt(unsigned int *ptr, long pos) {
	long offset = pos - ftell(fatImage);
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

// writes an unsigned int to fatImage at pos
unsigned int *writeUnInt(unsigned int *ptr, long pos) {
	long offset;
	unsigned int tmp;
	offset = pos - ftell(fatImage);
	if (endianVar) {
		tmp = switch32(*ptr);
	} else {
		tmp = *ptr;
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

// reads an unsigned short from fatImage at pos
unsigned short *readUnSh(unsigned short *ptr, long pos) {
	long offset = pos - ftell(fatImage);
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

// writes an unsigned short to fatImage at pos
unsigned short *writeUnSh(unsigned short *ptr, long pos) {
	long offset;
	unsigned short tmp;
	offset = pos - ftell(fatImage);
	if (endianVar) {
		tmp = switch16(*ptr);
	} else {
		tmp = *ptr;
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

// reads an unsigned char from fatImage at pos
unsigned char *readUnCh(unsigned char *ptr, long pos) {
	long offset = pos - ftell(fatImage);
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

// writes an unsigned char to fatImage at pos
unsigned int switch32(unsigned int val) {
	return ((val>>24)&0xff) | ((val<<8)&0xff0000) | ((val>>8)&0xff00) | ((val<<24)&0xff000000);
}

unsigned short switch16(unsigned short val) {
	return (val<<8) | (val>>8);
}

int check_endian(void) {
	int num = 1;
	if(*(char *)&num == 1) {
		return 0; // little endian
	} else {
		return 1; // big endian
	}
}

// follows the formula from the FAT32 specifications
unsigned int retSecClus(unsigned int clus) {
	return (clus - 2)*imageData.sec_per_clus + dataSec;
}

/* returns the absolute location of the given cluster in the given FAT in terms
 * of bytes
 */
unsigned long retFatClusPos(unsigned int clus, unsigned int fat) {
	unsigned int i, beginSecFAT;
	i = fat < imageData.num_fat ? fat : 0;
	beginSecFAT = imageData.rsvd_sec_cnt + i*imageData.fat_sz32;
	return beginSecFAT*imageData.bytes_per_sec + 4*clus;
}

/* returns the next cluster in the chain
 */
unsigned int retFatNextClus(unsigned int clus) {
	unsigned long location;
	unsigned int nextClus;
	location = retFatClusPos(clus, 0);
	readUnInt(&nextClus, location);
	return nextClus & NEXT_CLUS_MASK;
}

// same as above but doesn't apply the NEXT_CLUS_MASK to result
unsigned int retFatNextClus_true(unsigned int clus) {
	unsigned long location;
	unsigned int nextClus;
	location = retFatClusPos(clus, 0);
	readUnInt(&nextClus, location);
	return nextClus;
}

// checks if the value is one of the end of chain values
int chainEnd(unsigned int clus) {
	if ((clus & END_OF_CHAIN) == END_OF_CHAIN) {
		return 1;
	} else {
		return 0;
	}
}

// changes the values in all file allocation tables at fileClus to "value"
int changeFats(unsigned int fileClus, unsigned int value) {
	unsigned long location;
	int i;
	for (i = 0; i < imageData.num_fat; ++i) {
		location = retFatClusPos(fileClus, i);
		writeUnInt(&value, location);
	}
	return 1;
}

// takes a short name and stores transformed name to fNames
int shortLow(char fNames[12], char shNames[11]) {
	int i, j;
	for (i = 0, j = 0; i < 11; ++i) {
		if (shNames[i] != ' ') {
			if (i == 8) {
				fNames[j++] = '.';
			}
			fNames[j++] = tolower(shNames[i]);
		}
	}
	for (j = j; j < 12; ++j) {
		fNames[j] = '\0';
	}
	return 1;
}

// takes fNames and stores "short name transformed" string in shNames
int fileShort(char fNames[12], char shNames[11]) {
	int i, j;
	if (strcmp(fNames, ".") == 0) {
		strcpy(shNames, ".          ");
		return 1;
	} else if (strcmp(fNames, "..") == 0) {
		strcpy(shNames, "..         ");
		return 1;
	}
	i = 0, j = 0;
	while (i < 8 && fNames[j] != '.' && fNames[j] !='\0') {
		shNames[i++] = toupper(fNames[j++]);
	}
	for (i = i; i < 8; i++) {
		shNames[i] = ' ';
	}
	if (fNames[j++] == '.') {
		while (i < 11 && fNames != '\0') {
			shNames[i++] = toupper(fNames[j++]);
		}
	}
	for (i = i; i < 11; ++i) {
		shNames[i] = ' ';
	}
	return 1;
}

// finds the file in the given directory entry with name "fNames". returns 1
// if a file is searched and 0 if no file is searched
int lookupFile(char *fNames, unsigned int dirClus, union dirEntry *ptr, unsigned int *ptrClus, unsigned int *ptrOff) {
	unsigned int thisClus, i, bound, finish;
	union dirEntry file;
	char shNames[11];
	fileShort(fNames, shNames);
	thisClus = dirClus;
	bound = imageData.bytes_per_sec*imageData.sec_per_clus/32;
	finish = 0;
	do {
		for (i = 0; i < bound; ++i) {
			retDirEntry(&file, thisClus, i);
			if (file.raw_bytes[0] == 0x00) {
				finish = 1;
				break;
			} else if (file.raw_bytes[0] == 0xE5) {
				continue;
			} else if ((file.lf.attr & ATTR_LONG_NAME_MASK) == ATTR_LONG_NAME) {
				continue;
			} else if ((file.sf.attr & (ATTR_DIRECTORY | ATTR_VOLUME_ID)) == ATTR_VOLUME_ID) {
				continue;
			} else if ((strncmp(file.sf.name, shNames, 11) == 0)) {
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

// gets the first file cluster in the given directory entry
unsigned int retFileClus(union dirEntry *ptr) {
	unsigned int fileClus;
	unsigned short hi, lo;
	hi = ptr->sf.first_clus_hi;
	lo = ptr->sf.first_clus_lo;
	if (endianVar) {
		hi = switch16(hi);
		lo = switch16(lo);
	}
	fileClus = (hi << 16) | (lo & 0xFFFF);
	return fileClus;
}

// checks if the directory is empty
int emptyDir(union dirEntry *dir) {
	unsigned int thisClus, i, j, bound, finish;
	union dirEntry file;
	thisClus = retFileClus(dir);
	bound = imageData.bytes_per_sec*imageData.sec_per_clus/32;
	finish = 0;
	j = 0;
	do {
		for (i = 0; i < bound; ++i, ++j) {
			retDirEntry(&file, thisClus, i);
			if (j == 0 || j == 1) {
				continue;
			} else if ((file.lf.attr & ATTR_LONG_NAME_MASK) == ATTR_LONG_NAME) {
				continue;
			} else if (file.raw_bytes[0] == 0xE5) {
				continue;
			} else if (file.raw_bytes[0] == 0x00) {
				return 1;
			} else {
				return 0;
			}
		}
		thisClus = retFatNextClus(thisClus);
	} while (!chainEnd(thisClus) && !finish);
	return 1;
}

// returns the hi value of the given cluster
unsigned short retHiVal(unsigned int clus) {
	unsigned short hi;
	hi = (unsigned short)clus >> 16;
	if (endianVar) {
		hi = switch16(hi);
	}
	return hi;
}

// returns the lo value of the given cluster
unsigned short retLoVal(unsigned int clus) {
	unsigned short lo;
	lo = (unsigned short)clus & 0xFFFF;
	if (endianVar) {
		lo = switch16(lo);
	}
	return lo;
}

// returns the time in the FAT32 format
unsigned short retTime(void){
	time_t timeFatForm;
	struct tm * timeNow;
	unsigned short t;

	time(&timeFatForm);
	timeNow = localtime(&timeFatForm);
	t = ((timeNow->tm_hour*0x0800)+(timeNow->tm_min*0x0020)+(timeNow->tm_sec/2));
	if (endianVar) {
		t = switch16(t);
	}
	return t;
}

// returns the date in the FAT32 format
unsigned short retDate(void){
	time_t timeFatForm;
	struct tm * timeNow;
	unsigned short d;

	time(&timeFatForm);
	timeNow = localtime(&timeFatForm);
	d = (((timeNow->tm_year-80)*0x0200)+((timeNow->tm_mon+1)*0x0020)+(timeNow->tm_mday));
	if (endianVar) {
		d = switch16(d);
	}
	return d;
}

// searches for the next open cluster in the FAT
unsigned int lookupOpenClus() {
	unsigned int value;	//cluster data
	unsigned int searched = 1;	//free space searched
	unsigned int i;			//counterer
	for (i = 2; i < numClus; ++i) {
		value = retFatNextClus(i);
		if (value == 0){
			searched = 0;
			break;
		}
	}
	if (searched == 1){
		error_no_more_space();
		return 0;
	}
	return i;
}

// finds the next available directory entry
unsigned int lookupOpenDirEntry(unsigned int dirClus, union dirEntry *ptr, unsigned int *ptrClus, unsigned int *ptrOff) {
	unsigned int thisClus, i, bound, searched;
	union dirEntry file;
	thisClus = dirClus;
	bound = imageData.bytes_per_sec*imageData.sec_per_clus/32;
	searched = 0;
	do {
		for (i = 0; i < bound; ++i) {
			retDirEntry(&file, thisClus, i);
			if (file.raw_bytes[0] == 0x00) {
				setEntryNull(thisClus, i);
				*ptr = file;
				searched = 1;
				break;
			} else if (file.raw_bytes[0] == 0xE5) {
				*ptr = file;
				searched = 1;
				break;
			} else if ((file.lf.attr & ATTR_LONG_NAME_MASK) == ATTR_LONG_NAME) {
				continue;
			} else if ((file.sf.attr & (ATTR_DIRECTORY | ATTR_VOLUME_ID)) == ATTR_VOLUME_ID) {
				continue;
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

// sets the next directory entry to 0x00. Expands the directory if necessary
int setEntryNull(unsigned int dirClus, unsigned int entryDig) {
	union dirEntry file;
	unsigned int firstDirClus, entryByteOff, nextDig;
	nextDig = entryDig + 1;
	if ((entryByteOff = 32*nextDig) >= imageData.bytes_per_sec*imageData.sec_per_clus) {
		if (chainEnd(retFatNextClus(dirClus))) {
			expClus(dirClus);
		}
		firstDirClus = retSecClus(retFatNextClus(dirClus));
		entryByteOff -= imageData.bytes_per_sec*imageData.sec_per_clus;
	} else {
		firstDirClus = retSecClus(dirClus);
	}
	rChar(&file, entryByteOff + firstDirClus*imageData.bytes_per_sec, sizeof(union dirEntry));
	file.raw_bytes[0] = 0x00;
	wChar(&file, entryByteOff + firstDirClus*imageData.bytes_per_sec, sizeof(union dirEntry));
	return 1;
}

// function for making room in the file
int expClus(unsigned int oldClus1) {
	unsigned int new_clus;
	new_clus = lookupOpenClus();
	if (new_clus != 0) {
		changeFats(oldClus1, new_clus);
		changeFats(new_clus, END_OF_CHAIN);
	}
	return 1;
}


// END TOOLS