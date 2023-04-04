
#ifndef FS_STRUCTS
#define FS_STRUCTS .H
#include <stdio.h>
#include <time.h>
#define MAX_BLOCKS 1000 // 1000 * 512
#define MAGIC_NUMBER 0x4261686100000000
// Needs to take only 1 block
typedef struct VCB
{
    unsigned long magic_n;         // Validity of File System
    unsigned int root;             // Pos of root
    unsigned int block_size;       // Will be 512
    unsigned int free_blocks;      // Number of free blocks
    unsigned int number_of_blocks; // Total number of blocks in VCB
    unsigned int bitmap_start;     // Starting pos of bitmap within VCB
    unsigned int bitmap_end;       // Ending pos of bitmap within VCB
} VCB;

// 1000
// VCB = start, end
// Root = 20
// 0 = VCB
// 1 = BitMap
// 2... 22
// 3 = Home -> 23 and 43
// 4 = pics -> 44 and 64 ....
// Bitmap = 0  o 64, = 1
// Could take multiple blocks
typedef struct BitMap
{
    unsigned int blocks;
    unsigned int bitmap[]; // To keep track of free spaces
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