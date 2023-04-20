#ifndef BITMAP
#define BITMAP .H
typedef struct BitMap
{
	int *bitmap; // To keep track of free spaces
} BitMap;

int init_bitmap(int, int); //Returns 1 if secccess, -1 if error

/////////////

int set_used(int count, short(*data_locations)); //Returns 1 if secccess, -1 if error
int set_free(int count, short(*data_locations)); //Returns 1 if secccess, -1 if error
/* set_used saves the free indexes into data locations and mark blocks as used
in bitmap
   set_free sets all indexes in data_locations as 0 and marks blocks as free 
in bitmap
*/
/////////////
/////////////
int bit_set(int int_index, int bit_index);	// Sets a block as used. Returns 1 if secccess.
int bit_free(int int_index, int bit_index); //Sets a block as free. Returns 1 if secccess.

/* -1 if error. Error causes: arguemnt is larger than toal blocks, or negative input. */
/////////////

int get_next_free(); // returns index of next free bit
#endif