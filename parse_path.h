/**************************************************************
 * Class:  CSC-415-02 Fall 2021
 * Names: Diego Flores, Kemi Adebisi, Mohammad Dahbour
 * Student IDs:	920372463, 921140633, 921246050
 * GitHub Name: DiegoF001
 * Group Name: The Baha Blast
 * Project: Basic File System
 *
 * File: parse_path.h
 *
 * Description: Contains parse_path function prototype
 **************************************************************/
#ifndef PARSE_PATH
#define PARSE_PATH .H
#include "directory.h"
typedef struct Path
{
    void *dir_entry;
    int index;
} Path;

Path *parse_path(char *file_path, void *);
DirectoryEntry *check_extends(char *name, int starting_block, char *piece);

#endif