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
    while (token != NULL)
    {
        number_of_words++;
        path = realloc(path, number_of_words * sizeof(char *));
        path[number_of_words - 1] = strdup(token);
        token = strtok(NULL, "/");
    }
    container->name = path[number_of_words - 1];
    if (number_of_words == 0)
    {
        printf("EMPTY\n");
        container->dir_entry = dir_entry;
        container->index = dir_entry->data_locations[0];
        // printf("print: name: %s, j: %d\n", dir_entry->name, dir_entry->starting_block_index);
        return container;
    }
    int temp = number_of_words;
    int k = 0;
    for (int i = 0; i < temp; i++)
    {
        for (int j = 0; j < MAX_ENTRIES; j++)
        {
            LBAread(temp_directory, 1, dir_entry->data_locations[j]);
            printf("NAME OF DIR THAT We are READING: %s\nBLOCK NUMBER WE ARE READING: %d\nPATH RN: %s\n NUMBER OF WORDS: %d\n", dir_entry->name, dir_entry->data_locations[j], path[i], number_of_words);
            if (strcmp(path[i], temp_directory->name) == 0)
            {
                k = dir_entry->data_locations[j];
                printf("FOUND MATCHING NAMES ON BLOCK %d\n", dir_entry->data_locations[j]);
                LBAread(dir_entry, 1, dir_entry->data_locations[j]);

                // Outer: Is not dectory and is not last piece
                if (dir_entry->isDirectory != 1 && number_of_words > 0)
                {
                    printf("NULL\n");
                    return NULL;
                }
                if (number_of_words == 1)
                {
                    LBAread(temp_directory, 1, k);
                    printf("WORDS HIT 1\n");
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
                printf("!!!!!!!NULL\n");
                printf("!!!!!!!BLOCK:%d\n", dir_entry->data_locations[MAX_ENTRIES - 1]);

                temp_directory = check_extends(dir_entry->name,
                                               dir_entry->data_locations[MAX_ENTRIES - 1],
                                               path[i]);
                // Extended: more path pieces left, and doesnt exist: INVALID
                if (temp_directory == NULL && number_of_words > 1) // not found in extended,
                {
                    printf("****INVALID PATH\n");
                    return NULL;
                }
                // Extended: last piece, and doesnt exist
                else if (temp_directory == NULL && number_of_words == 1)
                {
                    container->dir_entry = dir_entry;
                    container->index = -1;
                    printf("****DOESNT EXIST\n");
                    return container;
                }
                // Extended: last piece, and exists
                else if (temp_directory != NULL && number_of_words == 1)
                {
                    container->dir_entry = dir_entry;
                    container->index = temp_directory->data_locations[0];
                    printf("****EXISTS\n");
                    return container;
                }
                // Extended: not last piece, and not a directory: INVALID
                else if (temp_directory != NULL && number_of_words > 1 && dir_entry->isDirectory != 1)
                {
                    printf("****INVALID PATH\n");
                    return NULL;
                }
                // Found and valid, move on to next piece
                else if (temp_directory != NULL && strcmp(path[i], temp_directory->name) == 0)
                {
                    printf("****FOUND IN EXTENDED %s\n", temp_directory->name);
                    LBAread(dir_entry, 1, container->index);
                }
            
            number_of_words--;
            }
        }
    }

    container->dir_entry = dir_entry;
    container->index = -1;
    printf("*******************************NOT FOUND IN:%s\n %d\n", container->dir_entry->name, container->index);
    return container;
}

DirectoryEntry *check_extends(char *name, int starting_block, char *piece)
{
    Extend *extend = malloc(sizeof(Extend));
    printf("!!!!!!!STARTING BLOCK:%d\n", starting_block);
    LBAread(extend, 1, starting_block);

    // printf("!!!!!!!RECUR FREE ENTRIES:%d\n", extend->data_locations[2]);
    printf("!!!!!! HOW MANY FREE ENTRIES LEFT IN EXTENDED: %d\n", extend->free_entries);
    DirectoryEntry *temp_entry = malloc(sizeof(DirectoryEntry));
    int k = 0;
    for (int i = 1; i < EXTENDED_ENTRIES - 1; i++)
    {
        LBAread(temp_entry, 1, extend->data_locations[i]);
        // printf("!!!!!!!temp NAME:%s\n", temp_entry->name);
        //  if match, load next directory
        if (strcmp(piece, temp_entry->name) == 0)
        {
            // LBAread(temp_entry, 1, extend->data_locations[i]);
            //LBAread(temp_entry, 1, temp_entry->data_locations[i]);
            container->dir_entry = temp_entry;
            container->index = extend->data_locations[i];
            printf("\n*********\nCONTENTS OF CONTAINER: \nDIRECTORY NAME: %s\nINDEX: %d\n*********\n", temp_entry->name, container->index);
            return temp_entry;
        }
    }

    if (extend->free_entries == 1 && extend->extended == TRUE)
    {
        // free(extend);
        // change
        // printf("@@@@HERE\n");
        // printf("@@@START:%d\n", extend->data_locations[2]);
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
