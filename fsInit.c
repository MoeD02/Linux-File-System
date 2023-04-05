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

VCB * vcb;
BitMap * bitmap;
DirectoryEntry * dir_entry;
//Init: VCB, BitMap, DirEntry - Root
// returns: 1 = sucess
//		   -1 = error
int initFileSystem(uint64_t numberOfBlocks, uint64_t blockSize)
{
	printf("Initializing File System with %ld blocks with a block size of %ld\n", numberOfBlocks, blockSize);
	/* TODO: Add any code you need to initialize your file system. */
	vcb = malloc(blockSize);

	LBAread(vcb, 1, 0);
	if (vcb->magic_n == MAGIC_NUMBER)
	{
		free(vcb);
		return 1;
	}
	init_vcb(blockSize, numberOfBlocks);
	init_bitmap();
	test_bitmap();

	// DirEntry
	return 0;
}

int init_vcb(int blockSize, int numberOfBlocks){
	vcb->magic_n = MAGIC_NUMBER;
	vcb->block_size = blockSize;
	vcb->number_of_blocks = numberOfBlocks;
	vcb->free_blocks = numberOfBlocks;
	int bitmap_blocks = 0;
	// to bytes:
	bitmap_blocks = (numberOfBlocks + (8 - 1)) / 8;
	// to blocks:
	bitmap_blocks = (bitmap_blocks + (blockSize - 1)) / blockSize;
	// this is how many blocks bitmap will occupy
	vcb->bitmap_total = bitmap_blocks;
	return 1;
}
// inits bitmap and marks vcb and bitmap blocks as used
int init_bitmap(){
	int bytes = vcb->bitmap_total * vcb->block_size;
	bitmap = malloc(sizeof(BitMap) + bytes * sizeof(int));
	short used_space = 1 + vcb->bitmap_total;
	for(int i = 0; i < used_space; i++){
		bitmap->bitmap[i] = 1; // used
	}
	for(int i = used_space + 1; i < bytes; i++){
		bitmap->bitmap[i] = 0; // unused
	}
	return 1;
}
// prints bitmap in backwards order
void test_bitmap(){
	printf("\n");
	int bytes = vcb->bitmap_total * vcb->block_size;
	for(int j = bytes - 1; j >=0; j--){
		printf("%d\t%d\n", bitmap->bitmap[j], j);
	}
	printf("%d\n", bytes);
}



void exitFileSystem()
{
	printf("System exiting\n");
}