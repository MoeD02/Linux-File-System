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
char **parse_path(char *filePath, void* entry){
    Path* container = malloc(sizeof(Path));
    DirectoryEntry* temp_directory = malloc (sizeof(DirectoryEntry));
    DirectoryEntry* dir_entry = (DirectoryEntry*) entry;
    //dir_entry[i]->name
    char **path=NULL;
    int number_of_words=0;
    char* temp_buffer=NULL;
    temp_buffer = strdup(filePath);
    char *token = strtok(temp_buffer, "/");
    while(token != NULL){
        number_of_words++;
        path = realloc( path, number_of_words * sizeof(char *));
        path[number_of_words-1] = strdup(token);
        for(int i=0; i<MAX_ENTRIES; i++){
            //if match, load next directory
            LBAread(temp_directory,1, dir_entry->data_locations[i]);
            if(strcmp(token, temp_directory->name)==0){
                LBAread(dir_entry, 1, dir_entry->data_locations[i]); //  /Downloads/Diego
                if(dir_entry->isDirectory!=1){
                    
                }
            }

        }     
       token = strtok(NULL,"/");
        
    }
    number_of_words++;
    path = realloc(path, number_of_words * sizeof(char *));
    path[number_of_words - 1] = NULL;
    printf("PATH: %s\n", path[0]);

    // /downloads/Diego 
    return path;
}
    
    
    //Adding NULL terminator at the end of array
    
    // go to root
    //go through the entries in root
    //USE LBA READ
    // find download
    // make sure its a directory
    // mamake sure it contains diego
    //take directory entries array
    //loop through it and make sure the path is correct
    

    // root/downloads/Diego
    // dir entries array
    // {Diego, root, moe, kemi, blah , dabidi, boo} 
    // parent: downloads
    


    //Find starting index of downloads 50
    //go to root
    //loop through data locations to find downloads
    //done
    //check if downloads is a directory
    // if it is continue
    // if not throw error
    // 



