/**************************************************************
 * Class:  CSC-415-0# Fall 2021
 * Names:
 * Student IDs:
 * GitHub Name:
 * Group Name:
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
DirectoryEntry *dir_entry;
// Init: VCB, BitMap, DirEntry - Root
//  returns: 1 = sucess
//		   -1 = error
int initFileSystem(uint64_t numberOfBlocks, uint64_t blockSize)
{
	printf("Initializing File System with %ld blocks with a block size of %ld\n", numberOfBlocks, blockSize);
	/* TODO: Add any code you need to initialize your file system. */
	vcb = malloc(blockSize);
	LBAread(vcb, 1, 0);
	if (vcb->magic_n == MAGIC_NUMBER)
	{
		printf("Already initialized: \n***%d***\n", vcb->free_blocks);
		free(vcb);
		return 1;
	}

	init_vcb(numberOfBlocks, blockSize);
	init_bitmap(); 
	init_root(blockSize);
	LBAwrite(vcb, 1, 0);
	LBAwrite(bitmap, vcb->bitmap_total, 1);

	free(bitmap);
	free(dir_entry);
	test_bitmap();

	// DirEntry
	return 0;
}

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
	printf("****VCB bitmap total: %d\n", vcb->bitmap_total);
	vcb->free_blocks -= bitmap_blocks;
	return 1;
}

int init_root(uint64_t blockSize)
{
	DirectoryEntry dir_entries[MAX_ENTRIES];
	int size = MAX_ENTRIES * sizeof(DirectoryEntry);
	int size_to_be_malloced = 512 * MAX_ENTRIES;
	//dir_entries = malloc(size_to_be_malloced);
	//printf("SIZE NEEDED: %d\nnum_Of_Blocks: %d\n Size of struct: %ld\n",
	//	   size, num_of_blocks, sizeof(DirectoryEntry));

	// if (dir_entry == NULL)
	// {
	// 	perror("Failed to allocate memory for the root directory");
	// 	return -1;
	// }
	// each has locations for their data
	printf("%d\n", MAX_ENTRIES);
	dir_entries[0].file_name[0] = '.';

	dir_entries[1].file_name[0] = '.';
	dir_entries[1].file_name[1] = '.';

	dir_entries[0].type = DIR;
	dir_entries[0].file_size = size;
	dir_entries[0].creation_date = time(NULL);
	dir_entries[0].last_access = time(NULL);
	dir_entries[0].last_mod = time(NULL);

	dir_entries[1].type = DIR;
	dir_entries[1].file_size = size;
	dir_entries[1].creation_date = time(NULL);
	dir_entries[1].last_access = time(NULL);
	dir_entries[1].last_mod = time(NULL);

	unsigned int root_index = get_next_free();
	int write_to = root_index;
	vcb->root = root_index;
	//Update locations of root
	for (int i = 0; i < MAX_ENTRIES; i++)
	{
		dir_entries[0].data_locations[i] = root_index;
		dir_entries[1].data_locations[i] = root_index;
		bitmap->bitmap[root_index] = USED;
		root_index++;
	}

	for (int i = 0; i < MAX_ENTRIES; i++)
	{
		dir_entries[i].file_name[0] = ' ';
		dir_entries[i].type = -1;
		dir_entries[i].file_size = 0;
	}
	vcb->free_blocks -= MAX_ENTRIES;

	printf("\n*****%d\n", write_to);
	LBAwrite(dir_entries, MAX_ENTRIES, write_to);
	return 1;
}

// inits bitmap and marks vcb and bitmap blocks as used
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