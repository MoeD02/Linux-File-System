/**************************************************************
* Class:  CSC-415-02 Fall 2021
* Names: Diego Flores, Kemi Adebisi, Mohammad Dahbour
* Student IDs:	920372463, 921140633, 921246050
* GitHub Name: DiegoF001
* Group Name: The Baha Blast
* Project: Basic File System
*
* File: fs_structs.h
*
* Description: This file contains the structs for our VCB,
*               Bitmap, and Directory Entries, as well as some
*               additonal information that the initialization
*               of file system needs.
*
**************************************************************/

#include "bitmap.h"
#include <stdio.h>
#include <stdlib.h>
#include "fsLow.h"
BitMap *bitmap;
int vcb_free;
int bytes_needed;
int init_bitmap(int numberOfBlocks, int blockSize)
{
	printf("INITIALIZING BITMAP\n");
	bytes_needed = (numberOfBlocks + 8 - 1) / 8;
	int blocks_needed = (bytes_needed + sizeof(BitMap) + blockSize - 1) / blockSize;
	bitmap = malloc(sizeof(BitMap) + blockSize * blocks_needed);
	bitmap->bitmap = (int *)(bitmap + 1);
	vcb_free = numberOfBlocks;
	//printf("%d\t%d\n", bytes_needed, blocks_needed);
	// for (int i = 0; i < 8; i++)
	// {
	// 	bit_set(0, i);
	// }
	//bit_set(0, 8);
	//set_free(11);
	//10111111
	//set_free(5);
	//1011111
	// int s = 64;
	// int *a = malloc(sizeof(int) * s);
	// set_used(s, a);
	// for (int i = 0; i < s; i++)
	// {
	// 	//printf("****%d\t%d\n", a[i], i);
	// }
	// bit_set(2, 73);
	// //1111 1111 0010 - 73
	// //1111 1111 0100 - 74
	// set_free(s, a);
	// for (int i = 0; i < s; i++)
	// {
	// 	printf("****%d\t%d\n", a[i], i);
	// }

	//LBAwrite(bitmap, blocks_needed, 1);
	return 1;
}

int bit_set(int int_index, int bit_index)
{
	// error: int_index is out of bounds
	if (bit_index < 0 || bit_index > vcb_free)
		return -1;

	bitmap->bitmap[int_index] |= (1 << bit_index);
	return 1;
}

int bit_free(int int_index, int bit_index)
{
	// error: int_index is out of bounds
	if (bit_index < 0 || bit_index > vcb_free)
		return -1;
	bitmap->bitmap[int_index] &= ~(1 << bit_index);
	return 1;
}

int set_free(int count, int *data_locations)
{
	
	if (count <= 0 || count >= vcb_free)
		return -1;
	int index;
	int k = 0;
	for (int i = 0; i < count; i++)
	{
		index = data_locations[k];
		data_locations[k] = 0;
		bit_free(i, index);
		k++;
	}
	return 1;
}
int set_used(int count, int *data_locations)
{
	if (vcb_free - count <= 0)
		return -1;
	// number of ints in the bitmap
	int int_total = (bytes_needed + 31) / 32;
	int index;
	int k = 0;
	for (int i = 0; i < int_total; i++)
	{
		for (int j = 0; j < 32; j++) //32 = size of int
		{
			int mask = 1 << j;
			if ((bitmap->bitmap[i] & mask) == 0) // if bit is 0, block is free
			{
				index = i * 32 + j % 32; // index of free block
				//printf("***%d\n", k);
				data_locations[k] = index;
				//printf("%d\n", index);
				bit_set(i, index);
				k++;
				if (k == count)
					return 1;
			}
		}
	}
}
int get_next_free()
{
	int int_total = (bytes_needed + 31) / 32;
	for (int i = 0; i < int_total; i++)
	{
		int int_index = i / 32;
		int bit_index = i % 32;
		if ((bitmap->bitmap[int_index] & (1 << bit_index)) == 0)
		{
			return i;
		}
	}
	return -1;
}

// void test_bitmap(VCBT *vcb, BitMap *bitmap)
// {
// 	printf("\n");
// 	int bytes = vcb->bitmap_total * vcb->block_size;
// 	for (int j = bytes - 1; j >= 0; j--)
// 	{
// 		printf("%d\t%d\n", bitmap->bitmap[j], j);
// 	}
// }