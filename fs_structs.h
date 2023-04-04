
#ifndef FS_STRUCTS.H
#define FS_STRUCTS .H
#include <stdio.h>
#include <time.h>
#define MAX_BLOCKS 1000

// Needs to take only 1 block
typedef struct VCB
{
    unsigned int magic_n;          // Validity of File System
    unsigned int root;             // Pos of root
    unsigned int block_size;       // Will be 512
    unsigned int free_blocks;      // Number of free blocks
    unsigned int number_of_blocks; // Total number of blocks in VCB
} VCB;

// Could take multiple blocks
typedef struct BitMap
{
    unsigned int bitmap[MAX_BLOCKS]; // To keep track of free spaces
} BitMap;

typedef struct DirectoryEntry
{                                    // home -> 20
                                     // [1,2,3], 4, 5 , [6,7], 8 .....] 3
    char file_name[128];             // File Name
    unsigned int data_locations[20]; // Array containing locations of file
    unsigned int type;               // Either File or Directory Entry
    unsigned int file_size;          // File Size in Bytes
    time_t creation_date;            // When was it Created
} DirectoryEntry;
#endif