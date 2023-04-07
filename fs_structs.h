/**************************************************************
* Class:  CSC-415-02 Fall 2021
* Names: Diego Flores
* Student IDs:	920372463
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
#define MAX_ENTRIES 50
#define UNUSED 0
#define USED 1
#define FILE 1
#define DIR 0
#define MAGIC_NUMBER 0x4261686100000000
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
typedef struct BitMap
{
    size_t size;
    unsigned int bitmap[]; // To keep track of free spaces
} BitMap;

typedef struct DirectoryEntry
{                                             //Parent Name
    char file_name[280];                      // File Name
    unsigned int data_locations[MAX_ENTRIES]; // Array containing locations of file
    unsigned int type;                        // Either File or Directory Entry
    unsigned int file_size;                   // File Size in Bytes
    time_t creation_date;                     // When was it Created
    time_t last_access;                       // when it was last accessed
    time_t last_mod;                          // when it was last modified
} DirectoryEntry;

int init_vcb(uint64_t, uint64_t);
int init_bitmap();
int init_root(uint64_t);
int get_next_free();
void test_bitmap();
#endif
