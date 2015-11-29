#ifndef FILE_TYPES_H__
#define FILE_TYPES_H__


#define ATTR_READ_ONLY			0x01
#define ATTR_HIDDEN				0x02
#define ATTR_SYSTEM				0x04
#define ATTR_VOLUME_ID			0x08
#define ATTR_DIRECTORY			0x10
#define ATTR_ARCHIVE			0x20
#define ATTR_LONG_NAME 			(ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID)
#define ATTR_LONG_NAME_MASK 	(ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID | ATTR_DIRECTORY | ATTR_ARCHIVE)


struct short_file {
	char name[11];
	unsigned char attr;
	unsigned char ntr;
	unsigned char crt_time_tenth;
	unsigned short crt_time;
	unsigned short crt_date;
	unsigned short last_acc_date;
	unsigned short first_clus_hi;
	unsigned short wrt_time;
	unsigned short wrt_date;
	unsigned short first_clus_lo;
	unsigned int file_size;
};

struct long_file {
	unsigned char ord;
	char name1[10];
	unsigned char attr;
	unsigned char type;
	unsigned char check_sum;
	char name2[12];
	unsigned short first_clus_lo;
	char name3[4];
};

union directory_entry {
	unsigned char raw_bytes[32];
	struct short_file sf;
	struct long_file lf;
};


int get_directory_entry(union directory_entry *ptr, unsigned int directory_clus, unsigned int entry_num);


#endif
