/**************************************************************
 * Class:  CSC-415-02 Fall 2021
 * Names: Diego Flores
 * Student IDs:	920372463
 * GitHub Name: DiegoF001
 * Group Name: The Baha Blast
 * Project: Basic File System
 *
 * File: fsInit.c
 *
 * Description: Main driver for file system assignment.
 *
 * This file is where you will start and initialize your system
 *
 **************************************************************/

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>

#include "fsLow.h"
#include "mfs.h"
#include "fs_structs.h"

VCB *vcb;
BitMap *bitmap;
int initFileSystem(uint64_t numberOfBlocks, uint64_t blockSize)
{
	printf("Initializing File System with %ld blocks with a block size of %ld\n",
		   numberOfBlocks, blockSize);
	/* TODO: Add any code you need to initialize your file system. */
	vcb = malloc(blockSize);
	int magic_n;
	// will save block 0 to vcb
	LBAread(vcb, 1, 0);
	// check if has be initialized already
	if (vcb->magic_n == MAGIC_NUMBER)
	{
		printf("--Already initialized--\n");
		magic_n = vcb->magic_n;
		free(vcb);
		return magic_n;
	}

	init_vcb(numberOfBlocks, blockSize);
	init_bitmap();

	init_root(blockSize);
	LBAwrite(vcb, 1, 0);
	LBAwrite(bitmap, vcb->bitmap_total, 1);

	free(bitmap);
	magic_n = vcb->magic_n;
	free(vcb);
	return magic_n;
}

//Inits the VCB
int init_vcb(uint64_t numberOfBlocks, uint64_t blockSize)
{
	vcb->magic_n = MAGIC_NUMBER;
	vcb->block_size = blockSize;
	vcb->number_of_blocks = numberOfBlocks;
	vcb->free_blocks = numberOfBlocks - 1;
	int bitmap_blocks = 0;
	// to bytes:
	bitmap_blocks = (numberOfBlocks + (8 - 1)) / 8;
	// to blocks:
	bitmap_blocks = (bitmap_blocks + (blockSize - 1)) / blockSize;
	// this is how many blocks bitmap will occupy
	vcb->bitmap_total = bitmap_blocks;
	vcb->free_blocks -= bitmap_blocks;
	return 1;
}

// Inits Root and 48 other entries
int init_root(uint64_t blockSize)
{
	DirectoryEntry dir_entries[MAX_ENTRIES];
	int size = MAX_ENTRIES * sizeof(DirectoryEntry);
	memset(&dir_entries[0], 0, vcb->block_size);
	memset(&dir_entries[1], 0, vcb->block_size);

	//Root: .
	dir_entries[0].file_name[0] = '.';
	//Root: .. 
	dir_entries[1].file_name[0] = '.';
	dir_entries[1].file_name[1] = '.';

	dir_entries[0].type = DIR;
	dir_entries[0].file_size = size;
	// will set to current time
	dir_entries[0].creation_date = time(NULL);
	dir_entries[0].last_access = time(NULL);
	dir_entries[0].last_mod = time(NULL);

	dir_entries[1].type = DIR;
	dir_entries[1].file_size = size;
	dir_entries[1].creation_date = time(NULL);
	dir_entries[1].last_access = time(NULL);
	dir_entries[1].last_mod = time(NULL);

	// Get first index of first free spot
	unsigned short root_index = get_next_free();
	unsigned int write_to = root_index;
	vcb->root = root_index;
	// Update locations of root
	for (int i = 0; i < MAX_ENTRIES; i++)
	{
		dir_entries[0].data_locations[i] = root_index;
		dir_entries[1].data_locations[i] = root_index;
		bitmap->bitmap[root_index] = USED;
		root_index++;
	}
	// Init all 48 entries
	for (int i = 2; i < MAX_ENTRIES; i++)
	{
		// works for some reason
		memset(&dir_entries[i], 0, vcb->block_size);
		dir_entries[i].file_name[0] = ' ';
		dir_entries[i].type = -1;
		dir_entries[i].file_size = 0;
	}
	//update available blocks
	vcb->free_blocks -= MAX_ENTRIES;
	// write Root
	LBAwrite(dir_entries, MAX_ENTRIES, write_to);
	return 1;
}

// inits bitmap and marks vcb and bitmap blocks as used and unused
int init_bitmap()
{
	int bytes = vcb->bitmap_total * vcb->block_size;
	bitmap = malloc(sizeof(BitMap) + bytes * sizeof(int));
	short used_space = 1 + vcb->bitmap_total;
	for (int i = 0; i < used_space; i++)
	{
		bitmap->bitmap[i] = USED;
	}
	for (int i = used_space + 1; i < bytes; i++)
	{
		bitmap->bitmap[i] = UNUSED;
	}
	return 1;
}

// prints bitmap in backwards order
void test_bitmap()
{
	printf("\n");
	int bytes = vcb->bitmap_total * vcb->block_size;
	for (int j = bytes - 1; j >= 0; j--)
	{
		printf("%d\t%d\n", bitmap->bitmap[j], j);
	}
}

// returns index in blocks
int get_next_free()
{
	int bytes = vcb->bitmap_total * vcb->block_size;
	for (int i = 0; i < bytes; i++)
	{
		if (bitmap->bitmap[i] == UNUSED)
		{
			return i;
		}
	}
	return -1;
}
void exitFileSystem()
{
	printf("System exiting\n");
}