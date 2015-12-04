# project3
FAT32 Filesystem

Group Member's:
	- Michael Duckett (mgd08d)
	- Alekhya Gade (ag13aj)
	- Travis Hett (tah12c)

Contents:
	- Readme
	- Paper
	- src
		- main.c
		- Makefile
		- include
			- clean.h
			- commands.h
			- directory.h
			- execute.h
			- file_types.h
			- parse.h
			- setup.h
			- shell_error.h
			- tools.h
		- src
			- clean.c
			- commands.c
			- directory.c
			- execute.c
			- file_types.c
			- parse.c
			- setup.c
			- shell_error.c
			- tools.c

To compile:
	$ cd ./src
	$ make

To execute:
	$ ./noticeablyFAT32 <FAT32_IMG>
