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

		for (int i = 2; i < MAX_ENTRIES - 1; i++)
		{

			LBAwrite(empty, 1, new_dir->data_locations[i]);
		}
		write_to = find_empty_entry(parse->dir_entry);
		if (write_to == 0)
		{

			write_to = find_empty_entry(parse->dir_entry);
		}
		new_dir->starting_bock = write_to;

		LBAwrite(new_dir, 1, write_to);

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

		LBAwrite(parent, 1, new_dir->data_locations[1]); // parent
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
	temp->name[0] = ' ';
	for (int i = 1; i < EXTENDED_ENTRIES - 1; i++)
	{

		LBAwrite(temp, 1, extended->data_locations[i]);
	}

	extended->extended = FALSE;

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

	LBAwrite(extended, 1, extended->data_locations[0]); // update not necessary
	temp->name[0] = ' ';

	for (int i = 1; i < EXTENDED_ENTRIES - 1; i++)
	{
		LBAwrite(temp, 1, ext->data_locations[i]);
	}

	LBAwrite(ext, 1, ext->data_locations[0]);
	LBAread(temp, 1, ext->data_locations[1]);
	free(temp);
	return ext;
}

int find_empty_entry(DirectoryEntry *dir_entry)
{ // 0, 1 ,
	container = malloc(sizeof(Container));
	DirectoryEntry *temp = malloc(sizeof(DirectoryEntry));
	int k = 0;
	for (int i = 2; i < MAX_ENTRIES; i++)
	{
		LBAread(temp, 1, dir_entry->data_locations[i]);

		char *t = dir_entry->name;

		k = i;
		if (strcmp(" ", temp->name) == 0)
		{

			if (dir_entry->free_entries != 1)
				dir_entry->free_entries--;
			LBAwrite(dir_entry, 1, dir_entry->starting_bock);

			return dir_entry->data_locations[i]; // 8
		}

		if (i == MAX_ENTRIES - 1 && dir_entry->free_entries == 1 && dir_entry->extended == TRUE)
		// check extended same piece // check of extended exists
		{

			temp = check_extends_mfs(dir_entry->data_locations[MAX_ENTRIES - 1]);

			if (temp == NULL)
			{

				return 0;
			}
			else
			{

				return container->index;
			}
		}
		// Extend dir_entry if needed
		else if (i == MAX_ENTRIES - 1 && dir_entry->free_entries == 1 && dir_entry->extended == FALSE)
		{

			extend_directory(dir_entry);
			LBAwrite(dir_entry, 1, dir_entry->starting_bock);

			return 0;
		}
	}
}
DirectoryEntry *check_extends_mfs(int starting_block)
{
	Extend *extend = malloc(sizeof(Extend));

	LBAread(extend, 1, starting_block);

	DirectoryEntry *temp = malloc(sizeof(DirectoryEntry));

	for (size_t i = 1; i < EXTENDED_ENTRIES - 1; i++)
	{

		LBAread(temp, 1, extend->data_locations[i]);

		if (strcmp(" ", temp->name) == 0)
		{

			if (extend->free_entries != 1)
			{
				extend->free_entries--;
			}

			LBAwrite(extend, 1, extend->data_locations[0]);
			container->dir_entry = temp;
			container->index = extend->data_locations[i];
			return temp;
		}
	}

	if (extend->free_entries == 1 && extend->extended == TRUE)
	{

		temp = check_extends_mfs(extend->data_locations[EXTENDED_ENTRIES - 1]);
	}
	else if (extend->free_entries == 1 && extend->extended == FALSE)
	{
		Extend *temp_extension;

		temp_extension = extend_extend(extend);
		temp = check_extends_mfs(extend->data_locations[EXTENDED_ENTRIES - 1]);
	}
}

int fs_isDir(char *pathname)
{
	if (cwd == NULL)
	{
		cwd = malloc(sizeof(DirectoryEntry));
		LBAread(cwd, 1, 6); // root
	}

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

		fd->d_reclen = sizeof(fdDir);			   // might have to change this
		fd->dirEntryPosition = directory_position; // might have to change this

		fd->directoryStartLocation = fd->dir->starting_bock;
		strcpy(fd->pathname, pathname);
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
struct fs_diriteminfo *fs_readdir(fdDir *dirp)

{
	if (is_directory_open(dirp->pathname) == 1) // check if dir is already open
	{
		struct fs_diriteminfo *dir_info = malloc(sizeof(struct fs_diriteminfo));
		DirectoryEntry *temp = malloc(sizeof(DirectoryEntry));
		if (dirp->read_index == MAX_ENTRIES - 1 && dirp->dir->extended == FALSE)
		{
			return NULL;
		}
		else if (dirp->read_index == MAX_ENTRIES - 1 && dirp->dir->extended == TRUE)
		{
			temp = check_extends_read(dirp->dir->data_locations[dirp->read_index], dirp);
			if (temp == NULL)
			{
				return NULL;
			}
			dirp->extended_read_index++;
			strcpy(dir_info->d_name, temp->name);
			dir_info->d_reclen = sizeof(struct fs_diriteminfo);

			if (temp->isDirectory == TRUE)
			{

				dir_info->fileType = FT_DIRECTORY;
			}
			else
			{

				dir_info->fileType = FT_REGFILE;
			}
			return dir_info;
		}
		else
		{
			LBAread(temp, 1, dirp->dir->data_locations[dirp->read_index]);

			dirp->read_index++;
			if (strcmp(temp->name, " ") == 0)
			{
				while (strcmp(temp->name, " ") == 0)
				{
					if (dirp->read_index == MAX_ENTRIES - 1)
					{
						break;
					}
					LBAread(temp, 1, dirp->dir->data_locations[dirp->read_index]);
					dirp->read_index++;
				}
			}

			strcpy(dir_info->d_name, temp->name);

			dir_info->d_reclen = sizeof(struct fs_diriteminfo);
			if (temp->isDirectory == TRUE)
			{

				dir_info->fileType = FT_DIRECTORY;
			}
			else
			{

				dir_info->fileType = FT_REGFILE;
			}

			return dir_info;
		}
	}
	else
	{
		perror("DIRECTORY IS NOT OPEN\n");
		return NULL;
	}
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
	if (strcmp(container->dir_entry->name, "..") == 0)
	{
		LBAread(cwd, 1, cwd->data_locations[0]);
	}

	cwd_path = strdup(pathname);
	free(container);
	return 0;
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
DirectoryEntry *check_extends_read(int starting_block, fdDir *dirp)
{
	Extend *extend = malloc(sizeof(Extend));

	LBAread(extend, 1, starting_block);

	DirectoryEntry *temp = malloc(sizeof(DirectoryEntry));

	for (size_t i = dirp->extended_read_index; i < EXTENDED_ENTRIES - 1; i++)
	{

		LBAread(temp, 1, extend->data_locations[i]);

		// if match, load next directory
		if (strcmp(" ", temp->name) != 0)
		{
			dirp->extended_read_index = i;
			return temp;
		}
	}

	if (extend->free_entries == 1 && extend->extended == TRUE)
	{

		dirp->extended_read_index = 1;
		temp = check_extends_read(extend->data_locations[EXTENDED_ENTRIES - 1], dirp);
	}
	else if (extend->free_entries == 1 && extend->extended == FALSE)
	{

		return NULL;
	}
}