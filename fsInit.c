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
#include "vcb.h"
#include "bitmap.h"

#include "directory.h"
#include "parse_path.h"

#include "mfs.h"
VCB *vcb;
int initFileSystem(uint64_t numberOfBlocks, uint64_t blockSize)
{

	DirectoryEntry *downloads = malloc(sizeof(DirectoryEntry));
	DirectoryEntry *root = malloc(sizeof(DirectoryEntry));
	// DirectoryEntry *file = malloc(sizeof(DirectoryEntry));
	// DirectoryEntry *dummy = malloc(sizeof(DirectoryEntry));
	printf("Initializing File System with %ld blocks with a block size of %ld\n",
		   numberOfBlocks, blockSize);
	/* TODO: Add any code you need to initialize your file system. */
	vcb = malloc(blockSize);
	int magic_n;
	// will save block 0 to vcb
	LBAread(vcb, 1, 0);
	// check if has be initialized already
	// if (vcb->magic_n == MAGIC_NUMBER)
	// {
	// 	printf("--Already initialized--\n");
	// 	magic_n = vcb->magic_n;
	// 	free(vcb);
	// 	return magic_n;
	// }
	init_bitmap(numberOfBlocks, blockSize);
	init_vcb(numberOfBlocks, blockSize);

	vcb->root_index = init_root(blockSize, NULL);
	LBAread(root, 1, vcb->root_index);
	downloads->isDirectory = TRUE;
	//file->isDirectory = FALSE; // file
	strcpy(downloads->name, "downloads");
	//strcpy(file->name, "file");
	// 6...55 marked as root
	int k = 56;
	for (int i = 0; i < MAX_ENTRIES; i++)
	{
		downloads->data_locations[i] = k;
		k++;
	}
	root->free_entries--;
	printf("*****%d\n", root->free_entries);
	LBAwrite(downloads, 1, vcb->root_index + 2); // root, parent, downloads  || extends
	//LBAwrite(file, 1, 57);
	LBAwrite(vcb, 1, 0);
	LBAwrite(root, 1, 6);
	//LBAwrite(dummy, 1, 57);
	magic_n = vcb->magic_n;
	printf("should print: name: downloads, j: -1\n");
	fs_mkdir("doesnt matter", 2);
	LBAread(root, 1, vcb->root_index);
	parse_path("/Moe", root);
	free(vcb);
	return magic_n;
}

//Inits the VCB
int init_vcb(uint64_t numberOfBlocks, uint64_t blockSize)
{
	printf("INITIALIZING VCB\n");
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
	printf("DONE WITH VCB\n");
	return 1;
}

void exitFileSystem()
{
	printf("System exiting\n");
}