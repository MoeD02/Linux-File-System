#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>

#include "fsLow.h"
#include "mfs.h"
#include "vcb.h"
#include "bitmap.h"
#include "directory.h"
#include "parse_path.h"

int fs_mkdir(const char *pathname, mode_t mode)
{ // parsepath
	Extend *extend;
	Extend *extend_again = malloc(sizeof(Extend));
	DirectoryEntry *test_extend_extend = malloc(sizeof(DirectoryEntry));
	DirectoryEntry *root = malloc(sizeof(DirectoryEntry));
	DirectoryEntry *dummy = malloc(sizeof(DirectoryEntry)); //Root {downloads, dummy}
	DirectoryEntry *test = malloc(sizeof(DirectoryEntry));
	LBAread(root, 1, 6);
	printf("*****%d\n", root->free_entries);
	//FIRST EXTEND
	if (root->free_entries == 1)
	{
		// 
		// . , .. , downloads || dummy || pics
		// 0:6 , 1:7 , 2:8 , 3:9 -> extends -> [10, 11 , 12] -> 12 -> extend_extend[13,14,15];
		extend = extend_directory(root);
		for (int i = 0; i < MAX_ENTRIES; i++)
			printf("*****EXTENDED in ROOT:%d\n", root->data_locations[i]);
		strcpy(dummy->name, "dummy");

		//printf("######%d\n", extend->data_locations[0]);
		LBAwrite(dummy, 1, extend->data_locations[1]);
		extend->free_entries--;
		LBAwrite(extend, 1, extend->data_locations[0]);
	}
	if (root->free_entries == 1)
	{
		LBAread(test, 1, extend->data_locations[1]);
		printf("*****%s\n", test->name);

		printf("*****EXTEND SPACES: %d\n", extend->free_entries);
	}
	// 0..49 entries, if they get filled, write extend at 50
	// if extend is full write extend at 128//
	// extend AGAIN 6:<-0, 9 -> 10
	if (extend->free_entries == 1)
	{
		extend_again = extend_extend(extend);
		LBAread(extend, 1, extend->data_locations[0]);
		for (int i = 0; i < EXTENDED_ENTRIES; i++)
			printf("*****EXTENDED :%d\n", extend->data_locations[i]);
		strcpy(test_extend_extend->name, "pics");
		extend_again->free_entries--;
		printf("*****EXTENDED SPACES: %d\n", extend_again->free_entries);
		LBAwrite(test_extend_extend, 1, extend_again->data_locations[1]);
		LBAwrite(extend_again, 1, extend_again->data_locations[0]);
	}
	LBAwrite(root, 1, 6);
	return 1;
}

// Init extends
Extend *extend_directory(DirectoryEntry *dir_entry)
{
	Extend *extended = malloc(sizeof(Extend)); // 128
	set_used(EXTENDED_ENTRIES, extended->data_locations);
	dir_entry->data_locations[MAX_ENTRIES - 1] = extended->data_locations[0];
	extended->free_entries = EXTENDED_ENTRIES; // 0 is itself
	LBAwrite(extended, 1, dir_entry->data_locations[MAX_ENTRIES - 1]);
	return extended;
}

Extend *extend_extend(Extend *extended)
{
	Extend *ext = malloc(sizeof(Extend));
	set_used(EXTENDED_ENTRIES, ext->data_locations);
	extended->data_locations[EXTENDED_ENTRIES - 1] = ext->data_locations[0];
	ext->free_entries = EXTENDED_ENTRIES - 1;
	LBAwrite(extended, 1, extended->data_locations[0]); //update not necessary
	LBAwrite(ext, 1, extended->data_locations[EXTENDED_ENTRIES - 1]);
	return ext;
}
