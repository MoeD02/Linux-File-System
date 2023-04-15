/**************************************************************
* Class:  CSC-415-02 Fall 2021
* Names: Diego Flores, Kemi Adebisi, Mohammad Dahbour
* Student IDs:	920372463, 921140633, 921246050
* GitHub Name: DiegoF001
* Group Name: The Baha Blast
* Project: Basic File System
*
* File: fs_structs.h
*
* Description: This file contains the structs for our VCB,
*               Bitmap, and Directory Entries, as well as some
*               additonal information that the initialization
*               of file system needs.
*
**************************************************************/

#ifndef FS_STRUCTS
#define FS_STRUCTS .H
#include <stdio.h>
#include <time.h>
// Needs to take only 1 block
typedef struct VCB
{
    unsigned long magic_n;         // Validity of File System
    unsigned int root;             // Pos of root
    unsigned int block_size;       // Will be 512
    unsigned int free_blocks;      // Number of free blocks
    unsigned int number_of_blocks; // Total number of blocks in VCB
    unsigned int bitmap_total;     //total number of blocks ocupied by bitmap
} VCB;

int init_vcb(uint64_t, uint64_t);
void test_bitmap();
#endif
