/**************************************************************
 * Class:  CSC-415-02 Fall 2021
 * Names: Diego Flores, Kemi Adebisi, Mohammad Dahbour
 * Student IDs:	920372463, 921140633, 921246050
 * GitHub Name: DiegoF001
 * Group Name: The Baha Blast
 * Project: Basic File System
 *
 * File: mfs.c
 *
 * Description: This file implements mfs.h (directory 	
 * 				operations). It also extends our
 * 				directorories/files if needed.
 *
 **************************************************************/

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
		memcpy(cwd_path, "/", sizeof(char)+1);
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

		//init names to so they are usable
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
		//init self
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

// Extend the Directory
Extend *extend_directory(DirectoryEntry *dir_entry)
{
	dir_entry->extended = TRUE;
	DirectoryEntry *temp = malloc(sizeof(DirectoryEntry));
	Extend *extended = malloc(sizeof(Extend));
	//set the bits of new dir as empty to have block usage make sense
	set_free(MAX_ENTRIES, new_dir->data_locations);
	set_used(EXTENDED_ENTRIES, extended->data_locations);
	set_used(MAX_ENTRIES, new_dir->data_locations);

	dir_entry->data_locations[MAX_ENTRIES - 1] = extended->data_locations[0];
	extended->free_entries = EXTENDED_ENTRIES - 1; // 1 block used: itself
	temp->name[0] = ' ';
	//init entries to a known free state
	for (int i = 1; i < EXTENDED_ENTRIES - 1; i++)
	{

		LBAwrite(temp, 1, extended->data_locations[i]);
	}

	extended->extended = FALSE;

	LBAwrite(extended, 1, extended->data_locations[0]);
	free(temp);
	return extended;
}

//Extend the extend
Extend *extend_extend(Extend *extended)
{
	extended->extended = TRUE;
	DirectoryEntry *temp = malloc(sizeof(DirectoryEntry));
	Extend *ext = malloc(sizeof(Extend));
	//same as extend_directory
	set_free(MAX_ENTRIES, new_dir->data_locations);
	set_used(EXTENDED_ENTRIES, ext->data_locations);
	set_used(MAX_ENTRIES, new_dir->data_locations);
	extended->data_locations[EXTENDED_ENTRIES - 1] = ext->data_locations[0]; // -1 because block 0 is itself
	ext->free_entries = EXTENDED_ENTRIES - 1;
	// write itself to the first block
	LBAwrite(extended, 1, extended->data_locations[0]);
	temp->name[0] = ' ';
	// init to a free state
	for (int i = 1; i < EXTENDED_ENTRIES - 1; i++)
	{
		LBAwrite(temp, 1, ext->data_locations[i]);
	}
	// init to a free state 
	LBAwrite(ext, 1, ext->data_locations[0]);
	LBAread(temp, 1, ext->data_locations[1]);
	free(temp);
	return ext;
}

//Finds free entry in a given directory
int find_empty_entry(DirectoryEntry *dir_entry)
{
	container = malloc(sizeof(Container));
	DirectoryEntry *temp = malloc(sizeof(DirectoryEntry));
	int k = 0;
	for (int i = 2; i < MAX_ENTRIES; i++)
	{
		//check each entry in data_locations
		LBAread(temp, 1, dir_entry->data_locations[i]);

		char *t = dir_entry->name;

		k = i;
		// if block is available
		if (strcmp(" ", temp->name) == 0)
		{

			//decrease the open entries of dir
			if (dir_entry->free_entries != 1)
				dir_entry->free_entries--;
			LBAwrite(dir_entry, 1, dir_entry->starting_bock);

			return dir_entry->data_locations[i];
		}
	// IF dir has been extended, check extends
		if (i == MAX_ENTRIES - 1 && dir_entry->free_entries == 1 && dir_entry->extended == TRUE)
		// check extended same piece // check of extended exists
		{

			temp = check_extends_mfs(dir_entry->data_locations[MAX_ENTRIES - 1]);

			// no more open spaces
			if (temp == NULL)
			{

				return 0;
			}
			//return open space
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

//Loop through extended and check for open spaces
DirectoryEntry *check_extends_mfs(int starting_block)
{
	Extend *extend = malloc(sizeof(Extend));

	LBAread(extend, 1, starting_block);

	DirectoryEntry *temp = malloc(sizeof(DirectoryEntry));

	for (size_t i = 1; i < EXTENDED_ENTRIES - 1; i++)
	{

		LBAread(temp, 1, extend->data_locations[i]);
		//if open space found
		if (strcmp(" ", temp->name) == 0)
		{

			//decrease free entries in extended
			if (extend->free_entries != 1)
			{
				extend->free_entries--;
			}
			//update free entries in extended
			LBAwrite(extend, 1, extend->data_locations[0]);
			//fill up where open space was found
			container->dir_entry = temp;
			container->index = extend->data_locations[i];
			return temp;
		}
	}

	//IF extended has been extended, recursive call to check again
	if (extend->free_entries == 1 && extend->extended == TRUE)
	{

		temp = check_extends_mfs(extend->data_locations[EXTENDED_ENTRIES - 1]);
	}
	//extended has not more open spaces, extended again.
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
		LBAread(cwd, 1, 6); //init cwd to root
	}

	container = parse_path(pathname, cwd);
	// INVALID
	if (container->index == -1)
	{
		perror("INVALID PATH");
		return 0;
	}
	else
	{
		// load dir into memory
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
		LBAread(cwd, 1, 6); // init cwd to root if null
	}
	container = parse_path(filename, cwd);
	if (container->index == -1)
	{
		perror("INVALID PATH");
		return 0;
	}
	else
	{
		//load file into memory
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

// Function to open a directory and return a file descriptor for the directory
fdDir *fs_opendir(const char *pathname)
{
    // Check if the current working directory is set, if not, allocate memory and set root directory
    if (cwd == NULL)
    {
        cwd = malloc(sizeof(DirectoryEntry));
        LBAread(cwd, 1, 6); // root
    }

    container = parse_path(pathname, cwd);
    directory_position++;

    fdDir *fd = malloc(sizeof(fdDir));

    // Check if the directory is already open, return NULL if it is
    if (is_directory_open(pathname) == 1)
    {
        directory_position--;
        return NULL;
    }

    // If the given path is invalid, return 0
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

        // Mark this directory as open in the open_dirs array
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
        // Initialize file descriptor for the directory
        fd->dir = malloc(sizeof(DirectoryEntry));
        LBAread(fd->dir, 1, container->index);

        fd->d_reclen = sizeof(fdDir);
        fd->dirEntryPosition = directory_position; 

        fd->directoryStartLocation = fd->dir->starting_bock;
        strcpy(fd->pathname, pathname);
        fd->read_index = 0;
        fd->extended_read_index = 1;

        // Free the temporary DirectoryEntry
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
    // Check if the directory is already open
    if (is_directory_open(dirp->pathname) == 1)
    {
        struct fs_diriteminfo *dir_info = malloc(sizeof(struct fs_diriteminfo));
        DirectoryEntry *temp = malloc(sizeof(DirectoryEntry));

        // If the read index reaches the maximum allowed entries and the directory is not extended
        if (dirp->read_index == MAX_ENTRIES - 1 && dirp->dir->extended == FALSE)
        {
            return NULL;
        }
        // If the read index reaches the maximum allowed entries and the directory is extended
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
            // Read the directory entry
            LBAread(temp, 1, dirp->dir->data_locations[dirp->read_index]);
            dirp->read_index++;

            // If the entry name is empty, keep reading until a non-empty entry is found or the read index reaches the maximum allowed entries
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

            // Set the directory item info structure
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
    // Initialize the current working directory if it's not set
    if (cwd == NULL)
    {
        cwd = malloc(sizeof(DirectoryEntry));
        LBAread(cwd, 1, 6); // root
    }
    if (cwd_path == NULL)
    {
        cwd_path = malloc(sizeof('/') + 1);
        memcpy(cwd_path, "/", sizeof(char));
    }

    Container *container = malloc(sizeof(Container));
    DirectoryEntry *erased = malloc(sizeof(DirectoryEntry));
    Extend *extend = malloc(sizeof(Extend));
    DirectoryEntry *cleaner = malloc(sizeof(DirectoryEntry));

    // Parse the given path
    container = parse_path(pathname, cwd);

    // Error, invalid path
    if (container == NULL)
    {
        return -1;
    }
    // Error, directory doesn't exist
    if (container->index == -1)
    {
        return -1;
    }
    // It must be a directory, not a file
    if (container->dir_entry->isDirectory == FALSE)
    {
        return -1;
    }

    // Read the directory to be removed
    LBAread(erased, 1, container->index);

    // If the directory is extended, recursively delete all extends
    if (erased->extended == 1)
    {
        erase_extends(extend);
    }

    // Set the bits as free so they can be overwritten later
    set_free(MAX_ENTRIES, erased->data_locations);

    // Clear the erased directory entry
    memset(cleaner, 0, sizeof(DirectoryEntry));
    cleaner->name[0] = ' ';

    // Write the cleaned entry back to the storage
    LBAwrite(cleaner, 1, erased->starting_bock);

    // Free allocated memory
    free(extend);
    free(cleaner);
    free(erased);
    free(container);

    return 0;
}

char *fs_getcwd(char *pathname, size_t size)
{
    if (cwd_path == NULL)
    {
        cwd_path = malloc(sizeof('/') + 1);
        memcpy(cwd_path, "/", sizeof(char) + 1);
    }

    // Return an empty path if the size is not enough
    if (sizeof(cwd_path) > size)
        return pathname;

    // Return the filled-up path
    strcpy(pathname, cwd_path);
    return pathname;
}

int fs_setcwd(char *pathname)
{
    // Initialize the current working directory if it's not set
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
    // Error, directory doesn't exist
    if (container->index == -1)
    {
        return -1;
    }

    // Read the new current working directory
    LBAread(cwd, 1, container->index);

    // If the new current working directory is the parent directory
    if (strcmp(container->dir_entry->name, "..") == 0)
    {
        LBAread(cwd, 1, cwd->data_locations[0]);
    }

    // Reallocate and update the current working directory path
    cwd_path = realloc(cwd_path, sizeof(pathname) + sizeof(cwd_path) + 2);
	if(pathname[strlen(pathname) - 1] != '/'){
		strcat(cwd_path, pathname);
		strcat(cwd_path, "/");
	}
		
	
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
    // Free the extend data locations
    set_free(EXTENDED_ENTRIES, extend->data_locations);

    // If the extend has another extend, erase that as well
    if (extend->extended == 1)
    {
        LBAread(extend, 1, extend->data_locations[EXTENDED_ENTRIES - 1]);
        erase_extends(extend);
    }
}

int fs_closedir(fdDir *dirp)
{
    // If the directory is open, close it
    if (is_directory_open(dirp->pathname) == 1)
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
    // Initialize the current working directory if it's not set
    if (cwd == NULL)
    {
        cwd = malloc(sizeof(DirectoryEntry));
        LBAread(cwd, 1, 6); // root
    }

    Container *container = malloc(sizeof(Container));
    container = parse_path(path, cwd);

    // Error, invalid path
    if (container == NULL)
    {
        return -1;
    }
    // Error, directory doesn't exist
    if (container->index == -1)
    {
        return -1;
    }

    // Fill the struct fs_stat buffer
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

    // Iterate through the extended entries
    for (size_t i = dirp->extended_read_index; i < EXTENDED_ENTRIES - 1; i++)
    {
        LBAread(temp, 1, extend->data_locations[i]);

        // If a valid entry is found, load the next directory
        if (strcmp(" ", temp->name) != 0)
        {
            dirp->extended_read_index = i;
            return temp;
        }
    }

    // If there are free entries and the extend is extended, recursively check the next extend
    if (extend->free_entries == 1 && extend->extended == TRUE)
    {
        dirp->extended_read_index = 1;
        temp = check_extends_read(extend->data_locations[EXTENDED_ENTRIES - 1], dirp);
    }
    // If there are free entries but the extend is not extended, return NULL
    else if (extend->free_entries == 1 && extend->extended == FALSE)
    {
        return NULL;
    }
}
