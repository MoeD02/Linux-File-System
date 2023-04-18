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
#include <stdio.h>
#include <string.h>
#include "parse_path.h"

char **parse_path(char *filePath){
    char **path=NULL;
    int number_of_words=0;
    char* temp_buffer=NULL;
    temp_buffer = strdup(filePath);
    char *token = strtok(temp_buffer, "/");
    while(token != NULL){

        number_of_words++;
        path = realloc( path, number_of_words * sizeof(char *));
        //strcpy(path[number_of_words-1], token);
        path[number_of_words-1] = strdup(token);
        token = strtok(NULL,"/");
    }
    //Adding NULL terminator at the end of array
    number_of_words++;
    path = realloc(path, number_of_words * sizeof(char *));
    path[number_of_words - 1] = NULL;
    printf("PATH: %s\n", path[1]);
    return path;


//     char path[120]; //filePath buffer
//     char delimiter[] = "/";
    

//     strcpy(path, filePath);

//     char *token = strtok(path, delimiter);
//     char full_path[120] = ""; //buffer for full path


// while (token != NULL) {
        
//         // Concatenate the token with the delimiter to construct the full path
//         strcat(full_path, token);
//         strcat(full_path, delimiter);
        
//         token = strtok(NULL, delimiter); //get next token

//         if(token == NULL){
//             printf("No path\n");
//         }
//     }


//     printf("Full Path: %s\n", full_path);
}