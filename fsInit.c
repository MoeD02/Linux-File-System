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
int initFileSystem(uint64_t numberOfBlocks, uint64_t blockSize)
{
	VCB *vcb = malloc(sizeof(VCB));

	printf("%ld\n", sizeof(VCB));
	printf("Initializing File System with %ld blocks with a block size of %ld\n", numberOfBlocks, blockSize);
	/* TODO: Add any code you need to initialize your file system. */

	return 0;
}

void exitFileSystem()
{
	printf("System exiting\n");
}