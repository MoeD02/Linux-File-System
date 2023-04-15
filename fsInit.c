/**************************************************************
 * Class:  CSC-415-02 Fall 2021
 * Names: Diego Flores, Kemi Adebisi, Mohammad Dahbour
 * Student IDs:	920372463, 921140633, 921246050
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
#include "bitmap.h"

VCB *vcb;
int initFileSystem(uint64_t numberOfBlocks, uint64_t blockSize)
{
	init_bitmap(numberOfBlocks, blockSize);
	// printf("Initializing File System with %ld blocks with a block size of %ld\n",
	// 	   numberOfBlocks, blockSize);
	// /* TODO: Add any code you need to initialize your file system. */
	// vcb = malloc(blockSize);
	// int magic_n;
	// // will save block 0 to vcb
	// LBAread(vcb, 1, 0);
	// // check if has be initialized already
	// if (vcb->magic_n == MAGIC_NUMBER)
	// {
	// 	printf("--Already initialized--\n");
	// 	magic_n = vcb->magic_n;
	// 	free(vcb);
	// 	return magic_n;
	// }

	// init_vcb(numberOfBlocks, blockSize);
	// init_bitmap();

	// init_root(blockSize);
	// LBAwrite(vcb, 1, 0);
	// LBAwrite(bitmap, vcb->bitmap_total, 1);

	// free(bitmap);
	// magic_n = vcb->magic_n;
	// free(vcb);
	// return magic_n;
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

void exitFileSystem()
{
	printf("System exiting\n");
}