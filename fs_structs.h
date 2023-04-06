
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

/*
Lets say we have 50 max entries, which is 36 blocks, and then that means
let's say you want entry number 5, how would you go about finding that?
sizeof(DirectoryEntry), which gives us 360 bytes, so the 5th entry would be
360*5 which would give us the starting byte. If we want the starting block
we divide (360*5)/512=#of block we're currently on.
*/
//Padding could be useful
//Add more chars to file and parent name to avoid padding? 



//. -->parent ' '
//. --> data_locations[0].name '..'?



typedef struct DirectoryEntry
{   
    char parent_name[128];         //Parent Name                           
    char file_name[128];          // File Name
    unsigned int data_locations[MAX_ENTRIES]; // Array containing locations of file
    unsigned int type;       // Either File or Directory Entry
    unsigned int file_size;       // File Size in Bytes
    time_t creation_date;         // When was it Created
    time_t last_access;           // when it was last accessed
    time_t last_mod;              // when it was last modified
} DirectoryEntry;

typedef struct DirectoryEntries{
    DirectoryEntry dir_entry[MAX_ENTRIES];
    
}DirectoryEntries;

int init_vcb(uint64_t, uint64_t);
int init_bitmap();
int init_root(uint64_t);
void test_bitmap();
#endif