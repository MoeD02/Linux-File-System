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

//Init: VCB, BitMap, DirEntry - Root
// returns: 1 = sucess
//		   -1 = error
int initFileSystem(uint64_t numberOfBlocks, uint64_t blockSize)
{
	printf("Initializing File System with %ld blocks with a block size of %ld\n", numberOfBlocks, blockSize);
	/* TODO: Add any code you need to initialize your file system. */
	VCB *vcb = malloc(blockSize);

	LBAread(vcb, 1, 0);
	if (vcb->magic_n == MAGIC_NUMBER)
	{
		free(vcb);
		return 1;
	}
	vcb->magic_n = MAGIC_NUMBER;
	vcb->block_size = blockSize;
	vcb->number_of_blocks = numberOfBlocks;
	vcb->free_blocks = numberOfBlocks;
	vcb->bitmap_start = 1;
	vcb->bitmap_end = (numberOfBlocks + (blockSize - 1)) / blockSize;
	// root

	
	printf("###%d\n", vcb->bitmap_end);

	BitMap *bitmap = malloc(sizeof(BitMap));
	DirectoryEntry *dir_entry = malloc(sizeof(DirectoryEntry));

	return 0;
}

void exitFileSystem()
{
	printf("System exiting\n");
}