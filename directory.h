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
#ifndef DIRECTORY
#define DIRECTORY .H
#include <stdio.h>
#include <time.h>
#define MAX_ENTRIES 50
#define UNUSED 0
// #define USED 1
#define TRUE 1
#define FALSE 0
#define MAGIC_NUMBER 0x4261686100000000
typedef struct DirectoryEntry
{                                             //Parent Name
    char name[272];                           // File Name
    unsigned int data_locations[MAX_ENTRIES]; // Array containing locations of file
    unsigned int isDirectory;                 // Either File or Directory Entry
    unsigned int size;                        // File Size in Bytes
    int starting_block_index;                   
    time_t creation_date;                     // When was it Created
    time_t last_access;                       // when it was last accessed
    time_t last_mod;                          // when it was last modified
} DirectoryEntry;
int init_root(uint64_t, DirectoryEntry*);
int assign_locations(DirectoryEntry *dir_entry, int);
#endif