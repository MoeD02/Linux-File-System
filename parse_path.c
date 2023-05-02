/**************************************************************
 * Class:  CSC-415-02 Fall 2021
 * Names: Diego Flores, Kemi Adebisi, Mohammad Dahbour
 * Student IDs:	920372463, 921140633, 921246050
 * GitHub Name: DiegoF001
 * Group Name: The Baha Blast
 * Project: Basic File System
 *
 * File: parse_path.c
 *
 * Description: This file makes sure that paths are valid
 *              it looks for pieces of path and it returns 
 *              directories and indexes.
 *
 **************************************************************/
#include "parse_path.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include "vcb.h"
// #include "directory.h"
#include "fsLow.h"
// free shit
char **path;

Container *container;
Container *parse_path(const char *filePath, void *entry)
{
    container = malloc(sizeof(Container));
    DirectoryEntry *temp_directory = malloc(sizeof(DirectoryEntry));

    DirectoryEntry *dir_entry = (DirectoryEntry *)entry;

    path = NULL;
    int number_of_words = 0;
    char *temp_buffer = NULL;
    temp_buffer = strdup(filePath);
    char *token = strtok(temp_buffer, "/");
    //tokenize path by /
    while (token != NULL)
    {
        number_of_words++;
        path = realloc(path, number_of_words * sizeof(char *));
        path[number_of_words - 1] = strdup(token);
        token = strtok(NULL, "/");
    }
    container->name = path[number_of_words - 1];
    // EMPTY PATH: return the same dir that was passed in
    if (number_of_words == 0)
    {

        container->dir_entry = dir_entry;
        container->index = dir_entry->data_locations[0];

        return container;
    }
    int temp = number_of_words;
    int k = 0;
    //check path pieces and data locations of each dir
    for (int i = 0; i < temp; i++)
    {
        for (int j = 0; j < MAX_ENTRIES; j++)
        {
            LBAread(temp_directory, 1, dir_entry->data_locations[j]);
            if (strcmp(path[i], temp_directory->name) == 0)
            {
                k = dir_entry->data_locations[j];
                LBAread(dir_entry, 1, dir_entry->data_locations[j]);

                // Outer: Is not dectory and is not last piece
                if (dir_entry->isDirectory != 1 && number_of_words > 0)
                {
                    return NULL;
                }
                // Path was found
                if (number_of_words == 1)
                {
                    LBAread(temp_directory, 1, k);

                    container->dir_entry = dir_entry;
                    container->index = k;
                    return container;
                }
                number_of_words--;
                break;
            }
            else if (j == MAX_ENTRIES - 1 && dir_entry->free_entries == 1 && dir_entry->extended == 1)
            // check extended same piece // check of extended exists
            {
                temp_directory = check_extends(dir_entry->name,
                                               dir_entry->data_locations[MAX_ENTRIES - 1],
                                               path[i]);
                // Extended: more path pieces left, and doesnt exist: INVALID
                if (temp_directory == NULL && number_of_words > 1) // not found in extended,
                {

                    return NULL;
                }
                // Extended: last piece, and doesnt exist
                else if (temp_directory == NULL && number_of_words == 1)
                {
                    container->dir_entry = dir_entry;
                    container->index = -1;

                    return container;
                }
                // Extended: last piece, and exists
                else if (temp_directory != NULL && number_of_words == 1)
                {
                    container->dir_entry = dir_entry;
                    container->index = temp_directory->data_locations[0];

                    return container;
                }
                // Extended: not last piece, and not a directory: INVALID
                else if (temp_directory != NULL && number_of_words > 1 && dir_entry->isDirectory != 1)
                {

                    return NULL;
                }
                // Found and valid, move on to next piece
                else if (temp_directory != NULL && strcmp(path[i], temp_directory->name) == 0)
                {

                    LBAread(dir_entry, 1, container->index);
                }

                number_of_words--;
            }
        }
    }
    container->dir_entry = dir_entry;
    container->index = -1;
    return container;
}
// check if path piece exists in extended dir
DirectoryEntry *check_extends(char *name, int starting_block, char *piece)
{
    Extend *extend = malloc(sizeof(Extend));
    //load extended
    LBAread(extend, 1, starting_block);

    DirectoryEntry *temp_entry = malloc(sizeof(DirectoryEntry));
    int k = 0;
    for (int i = 1; i < EXTENDED_ENTRIES - 1; i++)
    {
        LBAread(temp_entry, 1, extend->data_locations[i]);

        //  if match, load next directory
        if (strcmp(piece, temp_entry->name) == 0)
        {
            container->dir_entry = temp_entry;
            container->index = extend->data_locations[i];
            return temp_entry;
        }
    }
    // We checked all entries. We need to check the extend of extend for path piece
    if (extend->free_entries == 1 && extend->extended == TRUE)
    {

        //starting block of the next extend table
        temp_entry = check_extends(name, extend->data_locations[EXTENDED_ENTRIES - 1], piece);
    }
    else
    {
        return NULL;
    }
}
