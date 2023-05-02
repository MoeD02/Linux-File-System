#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>

#include "fsLow.h"
#include "mfs.h"
#include "directory.h"
#include "bitmap.h"

int init_root(uint64_t blockSize, DirectoryEntry *parent)
{
    printf("INITIALIZING ROOT\n");
    DirectoryEntry *dir_entries;
    // add extended to size
    int bytes_needed = MAX_ENTRIES * sizeof(DirectoryEntry);
    int blocks_needed = (bytes_needed + blockSize - 1) / blockSize;
    int bytes_used = blocks_needed * blockSize;
    int actual_entry_count = bytes_used / sizeof(DirectoryEntry);

    dir_entries = malloc(bytes_used);
    memset(&dir_entries[0], 0, blockSize);
    memset(&dir_entries[1], 0, blockSize);

    for (int i = 2; i < actual_entry_count - 1; i++)
    {
        memset(&dir_entries[i], 0, blockSize);
        dir_entries[i].name[0] = ' ';
        dir_entries[i].extended = FALSE;

        for (int j = 0; j < actual_entry_count; j++)
        {
            dir_entries[i].data_locations[j] = 0;
        }
    }
    // Assigning root variables
    strcpy(dir_entries[0].name, ".");
    dir_entries[0].creation_date = time(NULL);
    dir_entries[0].extended = FALSE;
    dir_entries[0].last_access = dir_entries[0].creation_date;
    dir_entries[0].last_mod = dir_entries[0].creation_date;
    dir_entries[0].isDirectory = TRUE;
    dir_entries[0].free_entries = MAX_ENTRIES - 2; // last item is saved for extended
    dir_entries[0].size = bytes_needed;
    set_used(blocks_needed, (dir_entries[0].data_locations));
    dir_entries[0].starting_bock = dir_entries[0].data_locations[0];

    if (parent == NULL)
    {
        strcpy(dir_entries[1].name, "..");
        dir_entries[1].creation_date = dir_entries[0].creation_date;
        dir_entries[1].last_access = dir_entries[0].creation_date;
        dir_entries[1].extended = dir_entries[0].extended;
        dir_entries[1].last_mod = dir_entries[0].creation_date;
        dir_entries[1].isDirectory = dir_entries[0].isDirectory;
        dir_entries[1].size = dir_entries[0].size;
        dir_entries[1].free_entries = dir_entries[0].free_entries;
        for (int i = 0; i < blocks_needed; i++)
        {
            dir_entries[1].data_locations[i] = dir_entries[0].data_locations[i];
        }
    }
    else
    {
        dir_entries[1].creation_date = parent->creation_date;
        dir_entries[1].last_access = parent->creation_date;
        dir_entries[1].last_mod = parent->creation_date;
        dir_entries[1].isDirectory = parent->isDirectory;
        dir_entries[1].size = parent->size;
        dir_entries[1].extended = parent->extended;
        dir_entries[1].starting_bock = dir_entries[0].data_locations[0];

        for (int i = 0; i < blocks_needed; i++)
        {
            dir_entries[1].data_locations[i] = parent->data_locations[i];
        }
    }

    LBAwrite(dir_entries, blocks_needed, dir_entries[0].data_locations[0]);

    DirectoryEntry *temp = malloc(sizeof(DirectoryEntry));
    int temp2 = dir_entries[0].data_locations[0];
    for (int j = 0; j < MAX_ENTRIES; j++)
    {
        *temp = dir_entries[j];

        LBAwrite(temp, 1, temp2);
        temp2++;
    }

    return dir_entries[0].data_locations[0];
}

int check_full(int entries_left)
{
}