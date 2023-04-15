#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>

#include "fsLow.h"
#include "mfs.h"
#include "directory.h"
#include "bitmap.h"
int init_root(uint64_t blockSize, DirectoryEntry *parent){
    printf("INITIALIZING ROOT\n");
    DirectoryEntry *dir_entries;
    int bytes_needed = MAX_ENTRIES * sizeof(DirectoryEntry);
    int blocks_needed = (bytes_needed + blockSize - 1) / blockSize;
    
    int bytes_used = blocks_needed * blockSize;
	int actual_entry_count = bytes_used / sizeof(DirectoryEntry);
    printf("BYTES NEEEDED %d\n BLOCKS NEEDED: %d\n BYTES USED: %d\n ACTUAL ENTRY COUNT: %d\n",
    bytes_needed, blocks_needed, bytes_used, actual_entry_count);
    dir_entries = malloc (bytes_used);
    for(int i=2; i<actual_entry_count; i++){
        dir_entries[i].name[0]= '\0'; 
        for(int j=0; j<MAX_ENTRIES; j++){
            dir_entries[i].data_locations[j]=0;
        }
    }
    //Assigning root variables
    strcpy(dir_entries[0].name,".");
    dir_entries[0].creation_date = time(NULL);
	dir_entries[0].last_access = dir_entries[0].creation_date;
	dir_entries[0].last_mod = dir_entries[0].creation_date;
    dir_entries[0].isDirectory = TRUE;
    dir_entries[0].size = bytes_needed;
    dir_entries[0].starting_block_index=assign_locations(&dir_entries[0], blockSize);
    if(parent == NULL){
        strcpy(dir_entries[1].name,"..");
        dir_entries[1].creation_date = dir_entries[0].creation_date;
	    dir_entries[1].last_access = dir_entries[0].creation_date;
	    dir_entries[1].last_mod = dir_entries[0].creation_date;
        dir_entries[1].isDirectory =  dir_entries[0].isDirectory;
        dir_entries[1].size = dir_entries[0].size;
        for(int i=0; i<blocks_needed;i++){
            dir_entries[1].data_locations[i]=dir_entries[0].data_locations[i];
        }
    }else{
        
        dir_entries[1].creation_date = parent->creation_date;
	    dir_entries[1].last_access = parent->creation_date;
	    dir_entries[1].last_mod = parent->creation_date;
        dir_entries[1].isDirectory =  parent->isDirectory;
        dir_entries[1].size = parent->size;
         for(int i=0; i<blocks_needed;i++){
            dir_entries[1].data_locations[i]=parent->data_locations[i];
        }
    }
    printf("SIZE OF DIR: %d\n", dir_entries[0].size);
    for(int i=0; i<MAX_ENTRIES; i++){
        printf("%d\n", dir_entries[0].data_locations[i]);
    }
    LBAwrite(dir_entries, blocks_needed, dir_entries[0].starting_block_index);
    //LBAwrite(dir_entries, blocks_needed, 7);
    printf("\nBYTES MALLOCED : %d\n", bytes_used);

    return 0;
}

int assign_locations(DirectoryEntry *dir_entry,int blockSize){
    
    (*dir_entry).size=1;
    int blocks_to_be_used =((*dir_entry).size + blockSize - 1) / blockSize;
    int index=0;
    int starting_block_index =0;
    printf("NUMBER: %d\n", blocks_to_be_used);
    set_used(blocks_to_be_used, (*dir_entry).data_locations);
    starting_block_index= (*dir_entry).data_locations[0];
    return starting_block_index;
}