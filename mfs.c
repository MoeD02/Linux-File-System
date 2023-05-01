#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include "fsLow.h"
#include "mfs.h"
#include "vcb.h"
//#include "fsInit.c"
#include "bitmap.h"
#include "directory.h"
#include "parse_path.h"
#define MAX_OPEN_DIRS 200
Container *container;
DirectoryEntry *eraser;
DirectoryEntry *new_dir;
DirectoryEntry *cwd;

// global path: 
/* Whenever I use a path, i check if the given path
has a slash or not:
	if it does NOT: relative, attactch my full wd to theirs
	if it does: leave as it is and pass it to parsepath
	*/
	
char* cwd_path;
int directory_position = 0;
OpenDir open_dirs[MAX_OPEN_DIRS] = {0};

// pics
int fs_mkdir(const char *pathname, mode_t mode)
{ // CALL get curr dir
	// root: 6 7 8 9 10 11 12
	// test1 :
	// test2 : 9 21 22 23 24 25 26
	// lmao should get written to 22
	//
	printf("2\n");
	cwd = malloc(sizeof(DirectoryEntry));
	printf("3\n");
	Container *parse;
	printf("4\n");
	// eraser = malloc(sizeof(DirectoryEntry));
	printf("5\n");
	// memset(&eraser, 0, sizeof(DirectoryEntry));
	printf("6\n");
	new_dir = malloc(sizeof(DirectoryEntry));
	printf("7\n");
	DirectoryEntry *empty = malloc(sizeof(DirectoryEntry));
	printf("8\n");
	DirectoryEntry *parent = malloc(sizeof(DirectoryEntry));
	printf("9\n");
	LBAread(cwd, 1, 6);

	parse = parse_path(pathname, cwd);

	// DirectoryEntry *temp3 = malloc(sizeof(DirectoryEntry));
	int k = 0;
	// for (int j = 0; j < MAX_ENTRIES; j++)
	// {
	// 	// printf("\t\t\t\t\tHERE!!!!!![%s]\n", dir_entry->name);
	// 	LBAread(temp3, 1, root->data_locations[j]);
	// 	k++;
	// 	printf("\t\t\t\t!!!!!!!!!!!!!!!!!!!TEST!!!!!![%d]\n[%s]\n", parse->dir_entry->data_locations[j], parse->name);
	// }
	int write_to = 0;
	// Invalid path
	if (parse == NULL)
		return -1;
	// Already exists
	else if (parse != NULL && parse->index != -1)
		return -1;
	// Can be created || ADD PARENT
	else if (parse && parse->index == -1)
	{
		// this read root
		parent = parse->dir_entry;
		// LBAread(parent, 1, parse->dir_entry->data_locations[0]);
		//  LBAread(parent, 1, parse->dir_entry->data_locations[0]);
		//   parent->free_entries--;
		//   LBAwrite(parent, 1, parse->dir_entry->data_locations[0]);
		strcpy(new_dir->name, parse->name);
		new_dir->isDirectory = TRUE;
		new_dir->size = sizeof(DirectoryEntry);
		new_dir->creation_date = time(NULL);
		new_dir->last_access = time(NULL);
		new_dir->last_mod = time(NULL);
		new_dir->free_entries = MAX_ENTRIES - 3; //

		set_used(MAX_ENTRIES, new_dir->data_locations);

		empty->name[0] = ' ';
		printf("\n\n\n%d\n\n\n", parse->dir_entry->extended);
		for (int i = 2; i < MAX_ENTRIES - 1; i++)
		{
			// LBAwrite(eraser, 1, new_dir->data_locations[i]);
			LBAwrite(empty, 1, new_dir->data_locations[i]);
		}
		write_to = find_empty_entry(parse->dir_entry);
		if (write_to == 0)
		{
			printf("*******checked %d\n", write_to);
			write_to = find_empty_entry(parse->dir_entry);
		}
		new_dir->starting_bock = write_to;
		printf("*******WRITE TO %d\n", write_to);
		// LBAwrite(eraser, 1, write_to);
		//  new_dir->free_entries -= 2;
		LBAwrite(new_dir, 1, write_to);
		// if (strcmp(new_dir->name, "test") == 0)

		// new_dir is setting blocks as used when it shouldnt
		DirectoryEntry *dot = malloc(sizeof(DirectoryEntry));
		LBAread(dot, 1, write_to);
		for (int i = 0; i < NAME_LENGTH; i++)
		{
			parent->name[i] = '\0';
			dot->name[i] = '\0';
		}

		dot->name[0] = '.';
		dot->free_entries--;
		dot->data_locations[0] = write_to;
		strcpy(parent->name, "..");

		LBAwrite(dot, 1, new_dir->data_locations[0]);
		// new_dir->data_locations[0] = write_to;
		LBAwrite(parent, 1, new_dir->data_locations[1]); // parent

		// LBAwrite(new_dir, 1, write_to);
		printf("\n\n%s\t%d\t%d\n\n", new_dir->name, new_dir->data_locations[0], new_dir->free_entries);
		// TerminalTest->free_entries--;
		// LBAwrite(TerminalTest, 1, write_to);
		// LBAwrite(TerminalTest, 1, TerminalTest->data_locations[0]);

		// printf("*******PARENT NAME: %s\nPARENT FREE ENTRIES LEFT: %d\n", TerminalTest->name, TerminalTest->free_entries);

		for (int j = 0; j < MAX_ENTRIES; j++)
		{
			printf("THE BLOCKS THAT %s OWNS ARE: %d\n", new_dir->name, new_dir->data_locations[j]);
		}
	}
	
	return 1;
}

// Init extends
Extend *extend_directory(DirectoryEntry *dir_entry)
{
	dir_entry->extended = TRUE;
	DirectoryEntry *temp = malloc(sizeof(DirectoryEntry));
	Extend *extended = malloc(sizeof(Extend));
	set_free(MAX_ENTRIES, new_dir->data_locations);
	set_used(EXTENDED_ENTRIES, extended->data_locations);
	set_used(MAX_ENTRIES, new_dir->data_locations);

	dir_entry->data_locations[MAX_ENTRIES - 1] = extended->data_locations[0];
	extended->free_entries = EXTENDED_ENTRIES - 1; // 0 is itself
	printf("\t\t\t\t\tWHERE WE WROTE IN EXTENDED: %d\n", extended->data_locations[0]);
	temp->name[0] = ' ';
	for (int i = 1; i < EXTENDED_ENTRIES - 1; i++)
	{
		printf("DATA LOCATION OF EXTENDED ARE: %d\n", extended->data_locations[i]);
		// LBAwrite(eraser, 1, extended->data_locations[i]);

		LBAwrite(temp, 1, extended->data_locations[i]);
	}
	// LBAwrite(eraser, 1, dir_entry->data_locations[0]);
	// LBAwrite(dir_entry, 1, dir_entry->data_locations[0]);
	extended->extended = FALSE;
	// LBAwrite(eraser, 1, extended->data_locations[0]);

	LBAwrite(extended, 1, extended->data_locations[0]);
	free(temp);
	return extended;
}

Extend *extend_extend(Extend *extended)
{
	extended->extended = TRUE;
	DirectoryEntry *temp = malloc(sizeof(DirectoryEntry));
	Extend *ext = malloc(sizeof(Extend));
	set_free(MAX_ENTRIES, new_dir->data_locations);
	set_used(EXTENDED_ENTRIES, ext->data_locations);
	set_used(MAX_ENTRIES, new_dir->data_locations);
	extended->data_locations[EXTENDED_ENTRIES - 1] = ext->data_locations[0];
	ext->free_entries = EXTENDED_ENTRIES - 1;
	// LBAwrite(eraser, 1, extended->data_locations[0]); // update not necessary

	LBAwrite(extended, 1, extended->data_locations[0]); // update not necessary
	temp->name[0] = ' ';

	for (int i = 1; i < EXTENDED_ENTRIES - 1; i++)
	{
		printf("DATA LOCATION OF EXTENDED ARE: %d\n", ext->data_locations[i]);
		// LBAwrite(eraser, 1, ext->data_locations[i]);
		LBAwrite(temp, 1, ext->data_locations[i]);
	}
	// LBAwrite(eraser, 1, ext->data_locations[0]);

	LBAwrite(ext, 1, ext->data_locations[0]);
	LBAread(temp, 1, ext->data_locations[1]);
	free(temp);
	// printf("DATA LOCATIONS FOR EXTENDED_EXTENDED: %d\n", ext->data_locations[2]);
	return ext;
}

int find_empty_entry(DirectoryEntry *dir_entry)
{ // 0, 1 ,
	container = malloc(sizeof(Container));
	DirectoryEntry *temp = malloc(sizeof(DirectoryEntry));
	int k = 0;
	for (int i = 2; i < MAX_ENTRIES; i++)
	{
		printf("##WHAT We are READING: %d\n", dir_entry->data_locations[i]);
		LBAread(temp, 1, dir_entry->data_locations[i]);
		// 
		// if null, then it's free

		printf("NAME OF DIRECTORY IM ON: [%s]\n", temp->name);
		char *t = dir_entry->name;
		printf("FREE ENTRIES ON: %s,****** \t%d\n", t, dir_entry->free_entries);
		k = i;
		if (strcmp(" ", temp->name) == 0)
		{
			// printf("HERE!!!!!![%s]\n[%d]\n", temp->name, dir_entry->data_locations[i]);
			if (dir_entry->free_entries != 1)
				dir_entry->free_entries--;
			//    LBAwrite(eraser, 1, dir_entry->data_locations[0]);
			printf("DECREASING FREE ENTRIES AND WRITING BACK TO DISK OF: %s\n AT BLOCK: %d\n", dir_entry->name, dir_entry->data_locations[0]);
			LBAwrite(dir_entry, 1, dir_entry->starting_bock);

			return dir_entry->data_locations[i]; // 8
		}
		// printf("STATUS OF EXTENSION: %d\n FREE ENTRIES IN ROOT: %d\n", dir_entry->extended, dir_entry->free_entries);
		// char*c = dir_entry->name;
		// printf("extra FREE ENTRIES ON: %s,********\t %d\n", c, dir_entry->free_entries );
		if (i == MAX_ENTRIES - 1 && dir_entry->free_entries == 1 && dir_entry->extended == TRUE)
		// check extended same piece // check of extended exists
		{

			printf("!!!!!!!HIT TRUE\n");

			temp = check_extends_mfs(dir_entry->data_locations[MAX_ENTRIES - 1]);

			if (temp == NULL)
			{
				printf("HITS NULL\n");
				return 0;
			}
			else
			{
				printf("RETURNING CONTAINER***\n");
				return container->index;
			}
		}
		// Extend dir_entry if needed
		else if (i == MAX_ENTRIES - 1 && dir_entry->free_entries == 1 && dir_entry->extended == FALSE)
		{
			printf("!!!!!!!HIT FALSE\n");
			extend_directory(dir_entry);
			LBAwrite(dir_entry, 1, dir_entry->starting_bock);

			return 0;
		}
	}
}
DirectoryEntry *check_extends_mfs(int starting_block)
{
	Extend *extend = malloc(sizeof(Extend));
	// printf("!!!!!!!STARTING BLOCK:%d\n", starting_block);
	LBAread(extend, 1, starting_block);

	// printf("!!!!!!!RECUR FREE ENTRIES:%d\n", extend->data_locations[2]);
	//  printf("!!!!!! HOW MANY FREE ENTRIES LEFT IN EXTENDED: %d\n", extend->free_entries);
	//  extend->free_entries--;
	DirectoryEntry *temp = malloc(sizeof(DirectoryEntry));
	// CHANGED THIS FROM <= to <
	//
	for (size_t i = 1; i < EXTENDED_ENTRIES - 1; i++)
	{

		LBAread(temp, 1, extend->data_locations[i]);
		printf("NAME OF DIRECTORY IM ON: %s\n", temp->name);
		// printf("!!!!!!!temp NAME:%s\n", temp_entry->name);
		// if match, load next directory
		if (strcmp(" ", temp->name) == 0)
		{
			printf("FOUND IN ONE OF THE EXTENDS and the block is %d, %d\n", extend->data_locations[i], extend->free_entries);
			// LBAread(temp_entry, 1, temp_entry->data_locations[i]);
			if (extend->free_entries != 1)
			{
				extend->free_entries--;
			}

			// LBAwrite(eraser, 1, extend->data_locations[0]);
			LBAwrite(extend, 1, extend->data_locations[0]);
			container->dir_entry = temp;
			container->index = extend->data_locations[i];
			return temp;
		}
	}

	if (extend->free_entries == 1 && extend->extended == TRUE)
	{
		// free(extend);
		// change
		printf("EXTENDED TRUE HIT!!\n");

		// printf("@@@START:%d\n", extend->data_locations[2]);
		//                                starting block of the next extend table
		temp = check_extends_mfs(extend->data_locations[EXTENDED_ENTRIES - 1]);
	}
	else if (extend->free_entries == 1 && extend->extended == FALSE)
	{
		Extend *temp_extension;
		printf("EXTENDED FALSE HIT!!\n");
		temp_extension = extend_extend(extend);
		temp = check_extends_mfs(extend->data_locations[EXTENDED_ENTRIES - 1]);
	}
}

int fs_isDir(char *pathname)
{
	// dir = malloc(sizeof(DirectoryEntry));
	container = parse_path(pathname, cwd);
	if (container->index == -1)
	{
		perror("INVALID PATH");
		return 0;
	}
	else
	{
		DirectoryEntry *temp = malloc(sizeof(DirectoryEntry));
		LBAread(temp, 1, container->index);
		LBAread(temp, 1, temp->data_locations[0]);

		if (temp->isDirectory == TRUE)
		{
			free(temp);

			return 1;
		}
		else
		{
			free(temp);

			return 0;
		}
	}
}
int fs_isFile(char *filename)
{

	container = parse_path(filename, cwd);
	if (container->index == -1)
	{
		perror("INVALID PATH");
		return 0;
	}
	else
	{
		DirectoryEntry *temp = malloc(sizeof(DirectoryEntry));
		LBAread(temp, 1, container->index);
		LBAread(temp, 1, temp->data_locations[0]);

		// if not directory then has to be file
		if (temp->isDirectory == FALSE)
		{
			free(temp);
			return 1;
		}
		else
		{
			free(temp);
			return 0;
		}
	}
}
/*
typedef struct
{
	TO DO:  Fill in this structure with what your open/read directory needs
	unsigned short d_reclen;
	unsigned short dirEntryPosition;
	uint64_t directoryStartLocation;
} fdDir;
*/

fdDir *fs_opendir(const char *pathname)
{
	LBAread(cwd, 1, 6);
	container = parse_path(pathname, cwd);
	directory_position++;

	fdDir *fd = malloc(sizeof(fdDir));

	if (is_directory_open(pathname) == 1) // if dir is already open return NULL
	{
		directory_position--;
		return NULL;
	}

	if (container->index == -1)
	{
		directory_position--;
		perror("INVALID PATH WHILE OPENING DIR");
		return 0;
	}
	else
	{
		DirectoryEntry *temp = malloc(sizeof(DirectoryEntry));

		LBAread(temp, 1, container->index);
		// LBAread(temp, 1, temp->starting_bock);
		char *c = temp->name;

		// mark this directory as open
		for (int i = 0; i < directory_position; i++)
		{
			if (open_dirs[i].dir == NULL)
			{
				open_dirs[i].dir = temp;
				open_dirs[i].pathname = strdup(pathname);
				fd->index_in_open_dirs = i;
				break;
			}
		}
		fd->dir = malloc(sizeof(DirectoryEntry));
		LBAread(fd->dir, 1, container->index);
		// LBAread(fd->dir, 1 , fd->dir->data_locations[0]);
		fd->d_reclen = sizeof(fdDir);			   // might have to change this
		fd->dirEntryPosition = directory_position; // might have to change this
		printf("!![%d]!!", container->index);
		fd->directoryStartLocation = fd->dir->starting_bock;
		free(temp);
	}
	return fd;
}
// helper function to check if dir is already open
int is_directory_open(const char *pathname)
{
	for (int i = 0; i < directory_position; i++)
	{
		if (open_dirs[i].dir != NULL && strcmp(open_dirs[i].pathname, pathname) == 0)
		{
			return 1;
		}
	}
	return 0;
}
struct fs_diriteminfo *fs_readdir(fdDir *dirp);

int fs_rmdir(const char *pathname);

char *fs_getcwd(char *pathname, size_t size)
{
	char *directory_string = NULL;
	size_t buffer_size = size;
}
// void init_cwd(){
// 	cwd = malloc(sizeof(DirectoryEntry));
// 	LBAread(cwd, 1, vcb->root_index); // load root in
// }
int fs_setcwd(char *pathname)
{
	
	Container* container = malloc(sizeof(Container));
	container = parse_path(pathname, cwd);
	//Error, invalid path
	if(container == NULL){
		return -1;
	}
	//Error, dir doesnt exist
	if(container->index == -1){
		return -1;
	}
	cwd_path = strdup(pathname);

	return 1;
}
int fs_delete(char *filename)
{
	return 0;
}

int fs_closedir(fdDir *dirp)
{
	if (is_directory_open(dirp->pathname) == 1) // if dir is already open return NULL
	{
		free(open_dirs[dirp->index_in_open_dirs].dir);
		open_dirs[dirp->index_in_open_dirs].dir = NULL;
		open_dirs[dirp->index_in_open_dirs].pathname = NULL;
		return 1; // success
	}
	else
	{
		perror("DIRECTORY IS NOT OPEN\n");
		return 0;
	}
}
