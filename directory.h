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
#include "mfs.h"
#define MAX_ENTRIES 5 // 100 but last cannot be allocated
#define EXTENDED_ENTRIES 5
#define UNUSED 0
#define NAME_LENGTH 446 + 2 // 272 + 182 - 8
// #define USED 1
#define TRUE 1
#define FALSE 0

#define MAGIC_NUMBER 0x4261686100000000

typedef struct DirectoryEntry
{
    char name[NAME_LENGTH];                   // File Name
    unsigned int data_locations[MAX_ENTRIES]; /* Array containing locations of file.
                                                 Last item is for next extended table*/
    unsigned int isDirectory;                 // Either File or Directory Entry
    unsigned int size;                        // File Size in Bytes
    unsigned short free_entries;              // Available entries
    // unsigned short flag;
    int starting_bock;
    unsigned short extended;
    time_t creation_date; // When was it Created
    time_t last_access;   // when it was last accessed
    time_t last_mod;      // when it was last modified
} DirectoryEntry;
// [99] = starting block of extended.
// When writing, we must write Root, and extended.
// LBAwrite(root, MAX_ENTIRES, [0]);
// LBAwrite(extended, EXTENDED_ENTRIES, [99]);

// root is full.
// ask bitmap for EXTENDED_ENTRIES open spots.
// Malloc extended, pass in data_locations from extendeds(count, *data_locations)
//
typedef struct Extend
{
    unsigned short extended;
    unsigned short free_entries;
    char garbage[272 + 184 + 40 - 16 - 1 - 46 + 8 + 47];
    unsigned int data_locations[EXTENDED_ENTRIES]; // Last item is for next extended table

} Extend;

int init_root(uint64_t, DirectoryEntry *);
Extend *extend_directory(DirectoryEntry *dir_entry);
Extend *extend_extend(Extend *extended);
DirectoryEntry *check_extends_mfs(int);
int find_empty_entry(DirectoryEntry *);
void erase_extends(Extend *extend);

#endif