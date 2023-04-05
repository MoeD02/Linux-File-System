
#ifndef FS_STRUCTS
#define FS_STRUCTS .H
#include <stdio.h>
#include <time.h>
#define MAX_ENTRIES 50
#define MAGIC_NUMBER 0x4261686100000000
// Needs to take only 1 block
typedef struct VCB
{
    unsigned long magic_n;         // Validity of File System
    unsigned int root;             // Pos of root
    unsigned int block_size;       // Will be 512
    unsigned int free_blocks;      // Number of free blocks
    unsigned int number_of_blocks; // Total number of blocks in VCB
    unsigned int bitmap_total;    //total number of blocks ocupied by bitmap
} VCB;
typedef struct BitMap
{   size_t size;
    unsigned int bitmap[]; // To keep track of free spaces
} BitMap;

typedef struct DirectoryEntry
{                              
    char file_name[128];          // File Name
    unsigned int data_locations[MAX_ENTRIES]; // Array containing locations of file
    unsigned int type;       // Either File or Directory Entry
    unsigned int file_size;       // File Size in Bytes
    time_t creation_date;         // When was it Created
    time_t last_access;           // when it was last accessed
    time_t last_mod;              // when it was last modified
} DirectoryEntry;

int init_vcb(int, int);
int init_bitmap();
void test_bitmap();
#endif