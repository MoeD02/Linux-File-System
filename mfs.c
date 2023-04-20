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
	DirectoryEntry *root = malloc(sizeof(DirectoryEntry));
	DirectoryEntry *dummy = malloc(sizeof(DirectoryEntry)); //Root {downloads, dummy}
	DirectoryEntry *test = malloc(sizeof(DirectoryEntry));
	LBAread(root, 1, 6);
	printf("*****%d\n", root->free_entries);
	if (root->free_entries == 1)
	{
		extend = malloc(sizeof(Extend));
		extend = extend_directory(root);
		strcpy(dummy->name, "dummy");
		//printf("######%d\n", extend->data_locations[0]);
		LBAwrite(dummy, 1, extend->data_locations[0]);
	}
	if (root->free_entries == 1)
	{
		LBAread(test, 1, root->data_locations[MAX_ENTRIES - 1]);
		printf("*****%s\n", test->name);
		// 0 1 2 = root
		// 3 ... 258 = extend
		// 
	}

	return 1;
}

Extend *extend_directory(DirectoryEntry *dir_entry)
{
	Extend *extended = malloc(sizeof(Extend));
	set_used(EXTENDED_ENTRIES, extended->data_locations);
	dir_entry->data_locations[MAX_ENTRIES - 1] = extended->data_locations[0];
	LBAwrite(extended, EXTENDED_ENTRIES, dir_entry->data_locations[MAX_ENTRIES - 1]);
	return extended;
}

Extend *extend_extend(Extend *extended)
{
	Extend *ext = malloc(sizeof(Extend));
	set_used(EXTENDED_ENTRIES, ext->data_locations);
	extended->data_locations[MAX_ENTRIES - 1] = ext->data_locations[0];
	LBAwrite(ext, EXTENDED_ENTRIES, extended->data_locations[MAX_ENTRIES - 1]);
	return ext;
}
