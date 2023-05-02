/**************************************************************
 * Class:  CSC-415-02 Fall 2021
 * Names: Diego Flores, Kemi Adebisi, Mohammad Dahbour
 * Student IDs:	920372463, 921140633, 921246050
 * GitHub Name: DiegoF001
 * Group Name: The Baha Blast
 * Project: Basic File System
 *
 * File: b_io.c
 *
 * Description: Basic File System - Key File I/O Operations
 *
 **************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h> // for malloc
#include <string.h> // for memcpy
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "b_io.h"
#include "parse_path.h"
#include "directory.h"

#define MAXFCBS 20
#define B_CHUNK_SIZE 512

typedef struct b_fcb
{
	/** TODO add al the information you need in the file control block **/
	DirectoryEntry *fi;
	char *buf;	// holds the open file buffer
	int index;	// holds the current position in the buffer
	int buflen; // holds how many valid bytes are in the buffer
	int flags;	// holds the linux flags
	int currBlk; //keeps track of current block being read
	int numBlk;  //total # of blocks

} b_fcb;

b_fcb fcbArray[MAXFCBS];

int startup = 0; // Indicates that this has not been initialized

// Method to initialize our file system
void b_init()
{
	// init fcbArray to all free
	for (int i = 0; i < MAXFCBS; i++)
	{
		fcbArray[i].buf = NULL; // indicates a free fcbArray
	}

	startup = 1;
}

// Method to get a free FCB element
b_io_fd b_getFCB()
{
	for (int i = 0; i < MAXFCBS; i++)
	{
		if (fcbArray[i].buf == NULL)
		{
			return i; // Not thread safe (But do not worry about it for this assignment)
		}
	}
	return (-1); // all in use
}

// Interface to open a buffered file
// Modification of interface for this assignment, flags match the Linux flags for open
// O_RDONLY, O_WRONLY, or O_RDWR
b_io_fd b_open(char *filename, int flags)
{
	b_io_fd returnFd;

	if (startup == 0)
		b_init(); // Initialize our system

	DirectoryEntry *fi;
	Container *container = parse_path(filename, &fi);
	

	if (container == NULL)
	{
		printf("invalid path\n");
		return -1;
	}

	returnFd = b_getFCB(); // get our own file descriptor

	b_fcb *fcb = &fcbArray[returnFd];

	if (returnFd == -1)
	{
		printf("FCB Error\n"); // check for error - all used FCB's
		return -1;
	}

	//Setting the file's access mode based on the flag
	if (flags == O_RDONLY)
	{
		fcb->flags = B_READ;
	}
	else if (flags == O_WRONLY)
	{
		fcb->flags = B_WRITE;
	}
	else if(flags == O_RDWR){
		fcb->flags = B_READ | B_WRITE;
	}


	//check if container or directoryentry is null and if so, the file doesn't exist	
    if (!container || !container->dir_entry) {
        printf("File not found\n");
        return -1;
    }


	//Initializing FCB
	fcb->fi = fi;
	fcb->buf = malloc(B_CHUNK_SIZE);
	fcb->index = 0;
	fcb->buflen = 0;
	fcb->flags = flags;
	fcb->currBlk = 0;
	fcb->numBlk = (fi->size + (B_CHUNK_SIZE - 1)) / B_CHUNK_SIZE;

	return (returnFd); // all set
}
// Interface to seek function
int b_seek(b_io_fd fd, off_t offset, int whence)
{
	b_fcb *fcb = &fcbArray[fd];
	int position;
	if (startup == 0)
		b_init(); // Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
	{
		return (-1); // invalid file descriptor
	}
	//Sets the position to offset from the beginning of the file
	if (whence == SEEK_SET)
	{
		position = offset;
	}
	//Sets position to current index of fcb + offset
	else if (whence == SEEK_CUR)
	{
		position = fcb->index + offset;
	}
	//Sets the position to the size of the file + offset
	else if (whence == SEEK_END)
	{
		position = fcb->fi->size + offset;
	}
	else
	{
		printf("ERROR: Invalid starting point\n");
		return -1;
	}

	if (position < 0 || position > fcb->fi->size)
	{
		printf("ERROR: Invalid offset\n");
		return -1;
	}

	fcb->index = position;

	return (position); 
}

// Interface to write function
int b_write(b_io_fd fd, char *buffer, int count)
{
	b_fcb *fcb = &fcbArray[fd];
	DirectoryEntry *fi = fcb->fi;

	if (startup == 0)
		b_init(); // Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
	{
		return (-1); // invalid file descriptor
	}

	if (fcb->fi == NULL)
	{
		printf("ERROR with file descriptor\n");
		return -1;
	}

	//Make sure the write doesn't exceed file size, if it does it means the file
	//should be extended...
	if (fcb->index >= fi->size)
	{
		return 0;
	}

	int bytes_read = 0;
	while (bytes_read < count)
	{
		// Determine which block to read from
		int block_num = (fcb->index + bytes_read) / B_CHUNK_SIZE;

		// Determine the position within the block
		int block_offset = (fcb->index + bytes_read) % B_CHUNK_SIZE;

		// Determine how much to read from the block
		int remaining_block_bytes = B_CHUNK_SIZE - block_offset;
		int bytes_to_read = count - bytes_read;

		if (bytes_to_read > remaining_block_bytes)
		{
			bytes_to_read = remaining_block_bytes;
		}

		// Read from the block into the buffer
		memcpy(buffer + bytes_read, fcb->buf + block_offset, bytes_to_read);

		// Update bytes_read and fcb index
		bytes_read += bytes_to_read;
		fcb->index += bytes_to_read;
	}

	return bytes_read;
}

// Interface to read a buffer

// Filling the callers request is broken into three parts
// Part 1 is what can be filled from the current buffer, which may or may not be enough
// Part 2 is after using what was left in our buffer there is still 1 or more block
//        size chunks needed to fill the callers request.  This represents the number of
//        bytes in multiples of the blocksize.
// Part 3 is a value less than blocksize which is what remains to copy to the callers buffer
//        after fulfilling part 1 and part 2.  This would always be filled from a refill
//        of our buffer.
//  +-------------+------------------------------------------------+--------+
//  |             |                                                |        |
//  | filled from |  filled direct in multiples of the block size  | filled |
//  | existing    |                                                | from   |
//  | buffer      |                                                |refilled|
//  |             |                                                | buffer |
//  |             |                                                |        |
//  | Part1       |  Part 2                                        | Part3  |
//  +-------------+------------------------------------------------+--------+
int b_read(b_io_fd fd, char *buffer, int count)
{
	int bytes_read = 0;
	int bytes_left = count;

	if (startup == 0)
		b_init(); // Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
	{
		return (-1); // invalid file descriptor
	}

	// Read from the FCB buffer first
	while (bytes_left > 0 && fcbArray[fd].index < fcbArray[fd].buflen)
	{
		int bytes_to_copy = fcbArray[fd].buflen - fcbArray[fd].index;
		if (bytes_to_copy > bytes_left)
		{
			bytes_to_copy = bytes_left;
		}
		// Copy bytes_to_copy number of bytes from the current position of the file buffer
		//(fcbArray[fd].buf + fcbArray[fd].index)
		// to the buffer that will be returned to the calling function (buffer + bytes_read).
		memcpy(buffer + bytes_read, fcbArray[fd].buf + fcbArray[fd].index, bytes_to_copy);

		fcbArray[fd].index += bytes_to_copy;
		bytes_read += bytes_to_copy;
		bytes_left -= bytes_to_copy;
	}

	// if we still need to read more bytes, read from disk
	while (bytes_left > 0 && fcbArray[fd].currBlk < fcbArray[fd].numBlk)
	{
		int bytes_to_read = B_CHUNK_SIZE;
		if (bytes_to_read > bytes_left)
		{
			bytes_to_read = bytes_left;
		}
		// Allocate memory for the block buffer
		char *block_buffer = malloc(B_CHUNK_SIZE);

		// Read from disk into the block buffer
		LBAread(block_buffer, bytes_to_read, fcbArray[fd].fi->data_locations[fcbArray[fd].currBlk]);
		memcpy(buffer + bytes_read, block_buffer, bytes_to_read);

		// Update FCB information
		fcbArray[fd].currBlk++;
		fcbArray[fd].buflen = bytes_to_read;
		fcbArray[fd].index = bytes_to_read;

		// Free the block buffer
		free(block_buffer);
		
		//update values
		bytes_read += bytes_to_read;
		bytes_left -= bytes_to_read;

		//Debugging
		// printf("BYTES LEFT: %d\n", bytes_left);
		// printf("CURRBLK: %d\n", fcbArray[fd].currBlk);
		// printf("NUMBLK: %d\n", fcbArray[fd].numBlk);
	}
	return (bytes_read);
}



// Interface to Close the file
int b_close(b_io_fd fd)
{
	free(fcbArray[fd].buf);
	fcbArray[fd].buf = NULL;
	return 0;
}
