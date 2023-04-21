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
Container *container;
int fs_mkdir(const char *pathname, mode_t mode)
{ // parsepath
	Extend *extend;
	Extend *extend_again = malloc(sizeof(Extend));
	DirectoryEntry *test_extend_extend = malloc(sizeof(DirectoryEntry));
	DirectoryEntry *root = malloc(sizeof(DirectoryEntry));
	DirectoryEntry *downloads = malloc(sizeof(DirectoryEntry));
	DirectoryEntry *dummy = malloc(sizeof(DirectoryEntry)); // Root {downloads, dummy}
	DirectoryEntry *test = malloc(sizeof(DirectoryEntry));
	DirectoryEntry *test1 = malloc(sizeof(DirectoryEntry));

	LBAread(root, 1, 6);
	// printf("*****%d\n", root->free_entries);
	//  FIRST EXTEND
	if (root->free_entries != 1)
	{
		//
		// . , .. , downloads || dummy || pics
		// 0:6 , 1:7 , 2:8 , 3:9 -> extends -> [10, 11 , 12] -> 12 -> extend_extend[13,14,15];
		int smthn1 = 0;
		printf("STATUS OF EXTENSION: %d\n", root->extended);

		extend = extend_directory(root);
		strcpy(test1->name, "Diego");
		smthn1 = find_empty_entry(root);
		LBAwrite(root, 1, 6);

		printf("THE BLOCK IM WRITING TO: %d\n", smthn1);
		LBAwrite(test1, 1, smthn1);
		// extend->free_entries--;

		LBAwrite(extend, 1, extend->data_locations[0]);
		// printf("CHECKING FOR 12: %d", extend->data_locations[2]);
		// for (int i = 0; i < MAX_ENTRIES; i++)
		// {
		// 	printf("*****EXTENDED in ROOT:%d\n", root->data_locations[i]);
		// }
		// strcpy(dummy->name, "dummy");

		// printf("######%d\n", extend->data_locations[0]);
		// writing to block 11
		// LBAwrite(dummy, 1, extend->data_locations[1]);
		return 1;
	}
	else
	{
		int smthn = 0;
		strcpy(test1->name, "Moe");
		smthn = find_empty_entry(root);
		printf("THE BLOCK IM WRITING TO: %d\n", smthn);
		LBAwrite(test1, 1, smthn);
		LBAwrite(root, 1, 6);
		int smthn2 = 0;
		strcpy(test1->name, "NEMO");
		smthn2 = find_empty_entry(root);
		printf("THE BLOCK IM WRITING TO: %d\n", smthn2);
		LBAwrite(test1, 1, smthn2);
		LBAwrite(root, 1, 6);
		return 1;
	}
	// if (root->free_entries == 1)
	// {
	// 	LBAread(test, 1, extend->data_locations[1]);
	// 	printf("*****%s\n", test->name);

	// 	printf("*****EXTEND SPACES: %d\n", extend->free_entries);
	// }
	// 0..49 entries, if they get filled, write extend at 50
	// if extend is full write extend at 128//
	// extend AGAIN 6:<-0, 9 -> 10
	// if (extend->free_entries == 1)
	// {
	// 	extend_again = extend_extend(extend);
	// 	LBAread(extend, 1, extend->data_locations[0]);
	// 	for (int i = 0; i < EXTENDED_ENTRIES; i++)
	// 		printf("*****EXTENDED :%d\n", extend->data_locations[i]);
	// 	strcpy(test_extend_extend->name, "pics");
	// 	extend_again->free_entries--;
	// 	printf("*****EXTENDED SPACES: %d\n", extend_again->free_entries);
	// 	LBAwrite(test_extend_extend, 1, extend_again->data_locations[1]);
	// 	LBAwrite(extend_again, 1, extend_again->data_locations[0]);
	// }
	// if (extend->free_entries == 1)
	// {
	// 	extend_again = extend_extend(extend_again);
	// 	strcpy(test1->name, "Moe");
	// 	extend_again->free_entries--;
	// 	printf("*****EXTENDED SPACES: %d\n", extend_again->free_entries);
	// 	LBAwrite(test1, 1, extend_again->data_locations[1]);
	// 	LBAwrite(extend_again, 1, extend_again->data_locations[0]);
	// }
	// 6 7 8 9
	// 10 11 12
}
// Init extends
Extend *extend_directory(DirectoryEntry *dir_entry)
{
	dir_entry->extended = TRUE;
	DirectoryEntry *temp = malloc(sizeof(DirectoryEntry));

	Extend *extended = malloc(sizeof(Extend)); // 3
	set_used(EXTENDED_ENTRIES, extended->data_locations);
	// Gives: 10,11,12
	dir_entry->data_locations[MAX_ENTRIES - 1] = extended->data_locations[0];
	// EDITED THIS
	extended->free_entries = EXTENDED_ENTRIES - 1; // 0 is itself
	printf("WHERE WE WROTE: %d\n", extended->data_locations[0]);
	temp->name[0] = 'E';
	for (int i = 1; i < EXTENDED_ENTRIES - 1; i++)
	{
		LBAwrite(temp, 1, extended->data_locations[i]);
	}
	LBAwrite(dir_entry, 1, dir_entry->data_locations[0]);
	extended->extended = FALSE;
	LBAwrite(extended, 1, extended->data_locations[0]);
	free(temp);
	return extended;
}

Extend *extend_extend(Extend *extended)
{
	extended->extended = TRUE;
	DirectoryEntry *temp = malloc(sizeof(DirectoryEntry));

	// 13,14,15
	Extend *ext = malloc(sizeof(Extend));
	set_used(EXTENDED_ENTRIES, ext->data_locations);
	extended->data_locations[EXTENDED_ENTRIES - 1] = ext->data_locations[0];
	ext->free_entries = EXTENDED_ENTRIES - 1;

	LBAwrite(extended, 1, extended->data_locations[0]); // update not necessary
	temp->name[0] = 'E';
	for (int i = 1; i < EXTENDED_ENTRIES - 1; i++)
	{
		// memset(&temp, 0, sizeof(temp));
		LBAwrite(temp, 1, ext->data_locations[i]);
	}
	LBAwrite(ext, 1, ext->data_locations[0]);
	LBAread(temp, 1, ext->data_locations[1]);
	printf("TEST PRINTING NAME: %s\n", temp->name);
	free(temp);
	printf("DATA LOCATIONS FOR EXTENDED_EXTENDED: %d\n", ext->data_locations[2]);
	return ext;
}

int find_empty_entry(DirectoryEntry *dir_entry)
{
	container = malloc(sizeof(Container));
	DirectoryEntry *temp = malloc(sizeof(DirectoryEntry));

	for (int i = 0; i < MAX_ENTRIES; i++)
	{
		printf("WHAT We are READING: %d\n", dir_entry->data_locations[i]);
		LBAread(temp, 1, dir_entry->data_locations[i]);
		// if null, then it's free
		printf("NAME OF DIRECTORY IM ON: %s\n", temp->name);
		if (strcmp("E", temp->name) == 0)
		{
			printf("HERE!!!!!!1\n");
			dir_entry->free_entries--;
			LBAwrite(dir_entry, 1, dir_entry->data_locations[0]);
			return dir_entry->data_locations[i];
		}
		printf("STATUS OF EXTENSION: %d\n FREE ENTRIES IN ROOT: %d\n", dir_entry->extended, dir_entry->free_entries);

		if (i == MAX_ENTRIES - 1 && dir_entry->free_entries == 1 && dir_entry->extended == TRUE)
		// check extended same piece // check of extended exists
		{

			printf("!!!!!!!HIT TRUE\n");
			// printf("!!!!!!!BLOCK:%d\n", dir_entry->data_locations[MAX_ENTRIES - 1]);
			// if (temp->extended == FALSE)
			// {
			// 	Extend *extend = malloc(sizeof(Extend));
			// 	extend = extend_directory(temp);
			// }
			temp = check_extends_mfs(dir_entry->data_locations[MAX_ENTRIES - 1]);

			if (temp == NULL)
			{
				return 0;
			}
			else
			{
				printf("RETURNING CONTAINER***\n");
				return container->index;
			}
		}
		else if (i == MAX_ENTRIES - 1 && dir_entry->free_entries == 1 && dir_entry->extended == FALSE)
		{
			printf("!!!!!!!HIT FALSE\n");

			Extend *extend = malloc(sizeof(Extend));
			extend = extend_directory(temp);
		}
	}
}
DirectoryEntry *check_extends_mfs(int starting_block)
{
	Extend *extend = malloc(sizeof(Extend));
	// printf("!!!!!!!STARTING BLOCK:%d\n", starting_block);
	LBAread(extend, 1, starting_block);

	printf("!!!!!!!RECUR FREE ENTRIES:%d\n", extend->data_locations[2]);
	// printf("!!!!!! HOW MANY FREE ENTRIES LEFT IN EXTENDED: %d\n", extend->free_entries);
	// extend->free_entries--;
	DirectoryEntry *temp = malloc(sizeof(DirectoryEntry));
	// CHANGED THIS FROM <= to <
	//
	for (size_t i = 1; i < EXTENDED_ENTRIES - 1; i++)
	{

		LBAread(temp, 1, extend->data_locations[i]);
		printf("NAME OF DIRECTORY IM ON: %s\n", temp->name);
		// printf("!!!!!!!temp NAME:%s\n", temp_entry->name);
		// if match, load next directory
		if (strcmp("E", temp->name) == 0)
		{
			printf("FOUND IN ONE OF THE EXTENDS and the block is %d\n", extend->data_locations[i]);
			// LBAread(temp_entry, 1, temp_entry->data_locations[i]);
			extend->free_entries--;
			LBAwrite(extend, 1, extend->data_locations[0]);
			container->dir_entry = temp;
			container->index = extend->data_locations[i];
			return temp;
		}
	}

	if (extend->free_entries == 1 && extend->extended == TRUE)
	{
		// free(extend);
		// change
		printf("EXTENDED TRUE HIT!!\n");

		printf("@@@START:%d\n", extend->data_locations[2]);
		//                               starting block of the next extend table
		temp = check_extends_mfs(extend->data_locations[EXTENDED_ENTRIES - 1]);
	}
	else if (extend->free_entries == 1 && extend->extended == FALSE)
	{
		Extend *temp_extension;
		printf("EXTENDED FALSE HIT!!\n");
		temp_extension = extend_extend(extend);
		temp = check_extends_mfs(extend->data_locations[EXTENDED_ENTRIES - 1]);
	}
	// else
	// {
	// 	extend = extend_extend(extend);

	// 	LBAread(temp, 1, extend->data_locations[1]);
	// 	extend->free_entries--;
	// 	LBAwrite(extend, 1, extend->data_locations[0]);
	// 	return temp;
	// }
}
