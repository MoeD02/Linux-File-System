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
#include "fs_structs.h"
#include "directory.h"
#include "fsLow.h"
Path* parse_path(char *filePath, void* entry){
    Path* container = malloc(sizeof(Path));
    DirectoryEntry* temp_directory = malloc (sizeof(DirectoryEntry));
    DirectoryEntry* dir_entry = (DirectoryEntry*) entry;
    char **path=NULL;
    int number_of_words=0;
    char* temp_buffer=NULL;
    temp_buffer = strdup(filePath);
    char *token = strtok(temp_buffer, "/");
    
    while(token != NULL){
        number_of_words++;
        path = realloc( path, number_of_words * sizeof(char *));
        path[number_of_words-1] = strdup(token);
        token = strtok(NULL,"/");
        
    }
    if (number_of_words==0){
        printf("EMPTY\n");
        container->dir_entry = dir_entry;
        container->index = dir_entry->starting_block_index;
        //printf("print: name: %s, j: %d\n", dir_entry->name, dir_entry->starting_block_index);
        return container;
    }

    //THIS IS HITTING EVERYTHING BEFORE LAST ITEM
    for(int i=0; i<number_of_words-1; i++){
        
        for(int j=0; j<MAX_ENTRIES; j++){
            //if match, load next directory
            LBAread(temp_directory,1, dir_entry->data_locations[j]); 
            if(strcmp(path[i], temp_directory->name)==0){ //temp: Downloads    
                LBAread(dir_entry, 1, dir_entry->data_locations[j]); //  dir_entry; Downloads
                //handle error of invalid path
                if(dir_entry->isDirectory!=1 && number_of_words>0){
                    //printf("NULL\n");
                    return NULL;
                }
                
                
            }
            

        }
        
    }
    //THIS IS HITTING LAST ITEM
    for(int i=0; i<MAX_ENTRIES;i++){
        LBAread(temp_directory,1, dir_entry->data_locations[i]); 
        // Valid and exists
        if(strcmp(path[number_of_words-1], temp_directory->name)==0){
            container->dir_entry = dir_entry;
            container->index = i;
            //printf("should print: name: %s, j: %d\n", dir_entry->name, dir_entry->data_locations[i]);
            return container;
        }
        
    }
    //Valid and last piece doenst exist
    container->dir_entry = dir_entry;
    container->index = -1;
    //printf("should print: name: %s, j: %d\n", dir_entry->name, -1);
    //Adding NULL terminator at the end of array
    number_of_words++;
    path = realloc(path, number_of_words * sizeof(char *));
    path[number_of_words - 1] = NULL;
    //printf("PATH: %s\n", path[0]);

    
    return container;
}