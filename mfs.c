#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include "fsLow.h"
#include "mfs.h"
#include "vcb.h"
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

char *cwd_path;
int directory_position = 0;
OpenDir open_dirs[MAX_OPEN_DIRS] = {0};
int fs_mkdir(const char *pathname, mode_t mode)
{
	int checker = 0;
	if (cwd == NULL)
	{
		cwd = malloc(sizeof(DirectoryEntry));
		cwd_path = malloc(sizeof('/') + 1);
		memcpy(cwd_path, "/", sizeof(char));
		LBAread(cwd, 1, 6);
	}
	// absolute: make cwd root and save state to recover later
	if (pathname[0] == '/')
	{
		checker = cwd->starting_bock;
		LBAread(cwd, 1, 6); // from toot
	}
	Container *parse;
	new_dir = malloc(sizeof(DirectoryEntry));
	DirectoryEntry *empty = malloc(sizeof(DirectoryEntry));
	DirectoryEntry *parent = malloc(sizeof(DirectoryEntry));
	if (cwd == NULL)
		LBAread(cwd, 1, 6);
	parse = parse_path(pathname, cwd);
	int k = 0;
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
		parent = parse->dir_entry;
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

	if (checker)
		LBAread(cwd, 1, checker);
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

DirectoryEntry *check_extends_read(int starting_block, fdDir *dirp)
{
	Extend *extend = malloc(sizeof(Extend));

	LBAread(extend, 1, starting_block);

	DirectoryEntry *temp = malloc(sizeof(DirectoryEntry));

	for (size_t i = dirp->extended_read_index; i < EXTENDED_ENTRIES - 1; i++)
	{

		LBAread(temp, 1, extend->data_locations[i]);
		printf("NAME OF DIRECTORY IM ON: %s\n", temp->name);
		// printf("!!!!!!!temp NAME:%s\n", temp_entry->name);
		// if match, load next directory
		if (strcmp(" ", temp->name) != 0)
		{
			printf("FOUND IN ONE OF THE EXTENDS and the block is %d, %d\n", extend->data_locations[i], extend->free_entries);
			dirp->extended_read_index = i;
			return temp;
		}
	}
	printf("EXTENDED FREE ENTRIES IS : %d\n", extend->free_entries);

	if (extend->free_entries == 1 && extend->extended == TRUE)
	{

		printf("EXTENDED TRUE HIT!!\n");
		dirp->extended_read_index = 1;
		temp = check_extends_read(extend->data_locations[EXTENDED_ENTRIES - 1], dirp);
	}
	else if (extend->free_entries == 1 && extend->extended == FALSE)
	{

		return NULL;
	}
}
int fs_isDir(char *pathname)
{
	if (cwd == NULL)
	{
		cwd = malloc(sizeof(DirectoryEntry));
		LBAread(cwd, 1, 6); // root
	}

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
	if (cwd == NULL)
	{
		cwd = malloc(sizeof(DirectoryEntry));
		LBAread(cwd, 1, 6); // root
	}
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
	if (cwd == NULL)
	{
		cwd = malloc(sizeof(DirectoryEntry));
		LBAread(cwd, 1, 6); // root
	}
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
		fd->pathname = pathname;
		fd->read_index = 0;
		fd->extended_read_index = 1;
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

int fs_rmdir(const char *pathname)
{
	if (cwd == NULL)
	{
		cwd = malloc(sizeof(DirectoryEntry));
		LBAread(cwd, 1, 6); // root
	}
	Container *container = malloc(sizeof(Container));
	DirectoryEntry *erased = malloc(sizeof(DirectoryEntry));
	Extend *extend = malloc(sizeof(Extend));
	DirectoryEntry *cleaner = malloc(sizeof(DirectoryEntry));
	container = parse_path(pathname, cwd);
	// Error, invalid path
	if (container == NULL)
	{
		return -1;
	}
	// Error, dir doesnt exist
	if (container->index == -1)
	{
		return -1;
	}
	// it must be a file, not a dir
	if (container->dir_entry->isDirectory == FALSE)
	{
		return -1;
	}
	LBAread(erased, 1, container->index);
	// recursively deletes all extends
	if (erased->extended == 1)
	{
		erase_extends(extend);
	}
	// set bits as free so they can be overwritten later
	set_free(MAX_ENTRIES, erased->data_locations);
	memset(cleaner, 0, sizeof(DirectoryEntry));
	cleaner->name[0] = ' ';
	LBAwrite(cleaner, 1, erased->starting_bock);

	free(extend);
	free(cleaner);
	free(erased);
	free(container);
	return 0;
}

char *fs_getcwd(char *pathname, size_t size)
{
	// return empty path
	if (sizeof(cwd_path) > size)
		return pathname;
	// return filled up path
	strcpy(pathname, cwd_path);
	return pathname;
}
int fs_setcwd(char *pathname)
{
	if (cwd == NULL)
	{
		cwd = malloc(sizeof(DirectoryEntry));
		LBAread(cwd, 1, 6); // root
	}
	Container *container = malloc(sizeof(Container));
	container = parse_path(pathname, cwd);
	// Error, invalid path
	if (container == NULL)
	{
		return -1;
	}
	// Error, dir doesnt exist
	if (container->index == -1)
	{
		return -1;
	}
	LBAread(cwd, 1, container->index);
	cwd_path = strdup(pathname);
	free(container);
	return 1;
}
int fs_delete(char *filename)
{
	if (cwd == NULL)
	{
		cwd = malloc(sizeof(DirectoryEntry));
		LBAread(cwd, 1, 6); // root
	}
	Container *container = malloc(sizeof(Container));
	DirectoryEntry *erased = malloc(sizeof(DirectoryEntry));
	Extend *extend = malloc(sizeof(Extend));
	DirectoryEntry *cleaner = malloc(sizeof(DirectoryEntry));
	container = parse_path(filename, cwd);
	// Error, invalid path
	if (container == NULL)
	{
		return -1;
	}
	// Error, dir doesnt exist
	if (container->index == -1)
	{
		return -1;
	}
	// it must be a file, not a dir
	if (container->dir_entry->isDirectory == TRUE)
	{
		return -1;
	}
	LBAread(erased, 1, container->index);
	// recursively deletes all extends
	if (erased->extended == 1)
	{
		erase_extends(extend);
	}
	// set bits as free so they can be overwritten later
	set_free(MAX_ENTRIES, erased->data_locations);
	memset(cleaner, 0, sizeof(DirectoryEntry));
	cleaner->name[0] = ' ';
	LBAwrite(cleaner, 1, erased->starting_bock);

	free(extend);
	free(cleaner);
	free(erased);
	free(container);
	return 0;
}
void erase_extends(Extend *extend)
{
	set_free(EXTENDED_ENTRIES, extend->data_locations);
	if (extend->extended == 1)
	{
		LBAread(extend, 1, extend->data_locations[EXTENDED_ENTRIES - 1]);
		erase_extends(extend);
	}
}
int fs_closedir(fdDir *dirp)
{
	if (is_directory_open(dirp->pathname) == 1) // check if dir is already open
	{

		// free(open_dirs[dirp->index_in_open_dirs].dir);
		open_dirs[dirp->index_in_open_dirs].dir = NULL;
		free(open_dirs[dirp->index_in_open_dirs].pathname);
		open_dirs[dirp->index_in_open_dirs].pathname = NULL;
		// free(dirp->dir);
		dirp->dir = NULL;
		// free(dirp->pathname);
		dirp->pathname = NULL;

		return 1; // success
	}
	else
	{
		perror("DIRECTORY IS NOT OPEN\n");
		return 0;
	}
}
int fs_stat(const char *path, struct fs_stat *buf)
{
	if (cwd == NULL)
	{
		cwd = malloc(sizeof(DirectoryEntry));
		LBAread(cwd, 1, 6); // root
	}
	Container *container = malloc(sizeof(Container));
	container = parse_path(path, cwd);
	if (container == NULL)
	{
		return -1;
	}
	// Error, dir doesnt exist
	if (container->index == -1)
	{
		return -1;
	}
	buf->st_size = container->dir_entry->size;
	buf->st_modtime = container->dir_entry->last_mod;
	buf->st_createtime = container->dir_entry->creation_date;
	buf->st_accesstime = container->dir_entry->last_access;
	buf->st_blocks = container->dir_entry->starting_bock;
	return 1;
}
