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
 * Description: This file contains the parse path function
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
Path *container;
Path *parse_path(char *filePath, void *entry)
{
    container = malloc(sizeof(Path));
    DirectoryEntry *temp_directory = malloc(sizeof(DirectoryEntry));
    DirectoryEntry *dir_entry = (DirectoryEntry *)entry;
    path = NULL;
    int number_of_words = 0;
    char *temp_buffer = NULL;
    temp_buffer = strdup(filePath);
    char *token = strtok(temp_buffer, "/");

    while (token != NULL)
    {
        number_of_words++;
        path = realloc(path, number_of_words * sizeof(char *));
        path[number_of_words - 1] = strdup(token);
        token = strtok(NULL, "/");
    }
    // temp(path[0], dir_entry, MAX_ENTRIES, dir_entry->data_locations);
    /*extended->data_locations[EXTENDED_ENTRIES - 1]
    while(number_of_words){

        do{
            int free = dir_entry->free_entries;

            // extends exist
            // loop through data locations and look for dir
            // loop through extended and look for dir


            // update free on each extend table
            // free != 1 means this is last extend table
            // if found LBA block

        }while(free == 1)
    number_of_words--;
    }
    */

    if (number_of_words == 0)
    {
        printf("EMPTY\n");
        container->dir_entry = dir_entry;
        container->index = dir_entry->data_locations[0];
        // printf("print: name: %s, j: %d\n", dir_entry->name, dir_entry->starting_block_index);
        return container;
    }
    int temp = number_of_words;
    // THIS IS HITTING EVERYTHING BEFORE LAST ITEM
    for (int i = 0; i < temp; i++)
    {
        for (int j = 0; j < MAX_ENTRIES; j++)
        {
            printf("WHAT We are READING: %d\nPATH RN: %s\n", dir_entry->data_locations[j], path[i]);
            LBAread(temp_directory, 1, dir_entry->data_locations[j]);
            // if match, load next directory
            if (strcmp(path[i], temp_directory->name) == 0)
            {
                LBAread(dir_entry, 1, dir_entry->data_locations[j]);
                // handle error of invalid path
                if (dir_entry->isDirectory != 1 && number_of_words > 0)
                {
                    printf("NULL\n");
                    return NULL;
                }
                else
                {
                    // fill container with found dir
                }
            }
            if (j == MAX_ENTRIES - 1 && dir_entry->free_entries == 1)
            // check extended same piece // check of extended exists
            {
                printf("!!!!!!!NULL\n");
                printf("!!!!!!!BLOCK:%d\n", dir_entry->data_locations[MAX_ENTRIES - 1]);

                temp_directory = check_extends(dir_entry->name,
                                               dir_entry->data_locations[MAX_ENTRIES - 1],
                                               path[i]);
                if (temp_directory == NULL)
                {
                    return NULL;
                }
            }
        }
        number_of_words--;
    }
    // THIS IS HITTING LAST ITEM
    //  for (int i = 0; i < MAX_ENTRIES - 1; i++)
    //  {
    //      LBAread(temp_directory, 1, dir_entry->data_locations[i]);
    //      // Valid and exists
    //      if (strcmp(path[number_of_words - 1], temp_directory->name) == 0)
    //      {
    //          container->dir_entry = dir_entry;
    //          container->index = i;
    //          //printf("should print: name: %s, j: %d\n", dir_entry->name, dir_entry->data_locations[i]);
    //          return container;
    //      }
    //      if (i == MAX_ENTRIES - 2)
    //      {
    //          printf("!!!!!!!NULL\n");
    //          temp_directory = check_extends(dir_entry->name,
    //                                         dir_entry->data_locations[MAX_ENTRIES - 1],
    //                                         path[0]);
    //          printf("!!!!!!!NULL%s\n", path[0]);
    //          if (temp_directory == NULL)
    //          {
    //              container->dir_entry = dir_entry;
    //              container->index = -1;
    //              printf("!!!!!!!NULL%s\n", temp_directory->name);
    //              return container;
    //          }
    //      }
    //  }
    // Valid and last piece doenst exist
    container->dir_entry = dir_entry;
    container->index = -1;
    // printf("should print: name: %s, j: %d\n", dir_entry->name, -1);
    // Adding NULL terminator at the end of array
    number_of_words++;
    path = realloc(path, number_of_words * sizeof(char *));
    path[number_of_words - 1] = NULL;
    // printf("PATH: %s\n", path[0]);

    return container;
}
// we get all free blocks inside extended [0..128]
// Directory temp = known free state
// loop 0-127,
// write[temp, 1, i];

// for other functions
// read(entry, 1, extended[1])
// entry->name
DirectoryEntry *check_extends(char *name, int starting_block, char *piece)
{
    Extend *extend = malloc(sizeof(Extend));
    printf("!!!!!!!STARTING BLOCK:%d\n", starting_block);
    LBAread(extend, 1, starting_block);

    printf("!!!!!!!RECUR FREE ENTRIES:%d\n", extend->data_locations[2]);
    printf("!!!!!! HOW MANY FREE ENTRIES LEFT IN EXTENDED: %d\n", extend->free_entries);
    // extend->free_entries--;
    DirectoryEntry *temp_entry = malloc(sizeof(DirectoryEntry));
    // CHANGED THIS FROM <= to <
    for (size_t i = 1; i < EXTENDED_ENTRIES - 1; i++)
    {
        // two issues 1 is its looping twice
        // figure this out
        LBAread(temp_entry, 1, extend->data_locations[i]);
        printf("!!!!!!!temp NAME:%s\n", temp_entry->name);
        // if match, load next directory
        if (strcmp(piece, temp_entry->name) == 0)
        {
            // LBAread(temp_entry, 1, temp_entry->data_locations[i]);
            container->dir_entry = temp_entry;
            container->index = extend->data_locations[i];
            printf("\n*********\nCONTENTS OF CONTAINER: \nDIRECTORY NAME: %s\nINDEX: %d\n*********\n", temp_entry->name, container->index);
            return temp_entry;
        }
    }

    if (extend->free_entries == 1)
    {
        // free(extend);
        // change
        printf("@@@@HERE\n");
        printf("@@@START:%d\n", extend->data_locations[2]);
        //                               starting block of the next extend table
        temp_entry = check_extends(name, extend->data_locations[EXTENDED_ENTRIES - 1], piece);
    }
    else
    {
        printf("ELSE !!!!!!!NULL%s\n", temp_entry->name);
        // free(extend);
        return NULL;
    }
}
