/**************************************************************
* Class:  CSC-415-02 Fall 2021
* Names: Diego Flores, Kemi Adebisi, Mohammad Dahbour
* Student IDs:	920372463, 921140633, 921246050
* GitHub Name: DiegoF001
* Group Name: The Baha Blast
* Project: Basic File System
*
* File: vcb.h
*
* Description: This file contains the structs for our VCB.
*
**************************************************************/

#ifndef VCB_HEADER
#define VCB_HEADER .H
#include <stdio.h>
#include <time.h>
#include "mfs.h"
// Needs to take only 1 block
typedef struct VCB
{
    unsigned long magic_n;         // Validity of File System
    unsigned int root_index;       // Pos of root
    unsigned int block_size;       // Will be 512
    unsigned int free_blocks;      // Number of free blocks
    unsigned int number_of_blocks; // Total number of blocks in VCB
    unsigned int bitmap_total;     //total number of blocks ocupied by bitmap
} VCB;

int init_vcb(uint64_t, uint64_t);
void test_bitmap();
#endif
