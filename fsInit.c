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

	// DirectoryEntry *downloads = malloc(sizeof(DirectoryEntry));
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

	LBAwrite(vcb, 1, 0);
	LBAwrite(root, 1, 6);
	magic_n = vcb->magic_n;

	printf("***SIZE OF DIR: %ld\n", sizeof(DirectoryEntry));
	fs_mkdir("/test1", 2);
	fs_mkdir("/test1/test2", 2);
	fs_mkdir("/test1/test3", 2);
	fdDir *test = malloc(sizeof(fdDir));

	// printf("****RESULT OF IS FILE: %d\n", fs_isFile("test1"));
	//fdDir *test = malloc(sizeof(fdDir));

	// test = fs_opendir("/test1");

	// outer
	// fs_mkdir("test1/test2", 2);

	// first
	// fs_mkdir("test1/test1", 2);
	// fs_mkdir("test1/test3", 2);
	fs_mkdir("/test1/test4", 2);

	fs_mkdir("/test1/test5", 2);

	fs_mkdir("/test1/test7", 2);
	fs_mkdir("/test1/test7/lmao", 2);
	fs_setcwd("/test1/test7");
	// fs_mkdir("lmao2", 1);

	// fs_mkdir("/lmao", 2);
	// fs_mkdir("lmao3", 1);
	//test = fs_opendir("lmao");
	//fs_closedir(test);
	//fdDir *test1 = malloc(sizeof(fdDir));
	//fs_mkdir("test1/test6", 2);
	//test1 = fs_opendir("test1/test6");
	//printf("NAME OF DIR: [%s]\n BLOCK OF DIR: %ld\n", test->dir->name, test->directoryStartLocation);
	// //second

	// fs_mkdir("test1/test6", 2);
	//  fs_mkdir("test1/test7", 2);
	//  fs_mkdir("test1/test8", 2);
	//  fs_mkdir("test1/test9", 2);
	//  fs_mkdir("test1/test10", 2);

	// fs_mkdir("test1/test11", 2);
	// fs_mkdir("test1/test12", 2);
	// fs_mkdir("test1/test13", 2);
	// fs_mkdir("test1/test14", 2);
	// fs_mkdir("test1/test15", 2);
	// fs_mkdir("test1/test16", 2);
	// fs_mkdir("test1/test17", 2);
	// fs_mkdir("test1/test18", 2);
	// fs_mkdir("test1/test19", 2);
	// fs_mkdir("test1/test20", 2);

	// fs_mkdir("test1/test21", 2);
	// fs_mkdir("test1/test22", 2);
	// fs_mkdir("test1/test23", 2);
	// fs_mkdir("test1/test24", 2);
	// fs_mkdir("test1/test25", 2);
	// fs_mkdir("test1/test26", 2);
	// fs_mkdir("test1/test27", 2);
	// fs_mkdir("test1/test28", 2);
	// fs_mkdir("/lmao", 2);

	free(vcb);
	return magic_n;
}

// Inits the VCB
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