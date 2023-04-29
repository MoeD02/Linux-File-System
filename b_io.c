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
#include <stdlib.h>			// for malloc
#include <string.h>			// for memcpy
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
	char * buf;		//holds the open file buffer
	int index;		//holds the current position in the buffer
	int buflen;		//holds how many valid bytes are in the buffer
	int flags;		//holds the linux flags
	int currBlk;
    int numBlk;

	} b_fcb;
	
b_fcb fcbArray[MAXFCBS];

int startup = 0;	//Indicates that this has not been initialized

//Method to initialize our file system
void b_init ()
	{
	//init fcbArray to all free
	for (int i = 0; i < MAXFCBS; i++)
		{
		fcbArray[i].buf = NULL; //indicates a free fcbArray
		}
		
	startup = 1;
	}

//Method to get a free FCB element
b_io_fd b_getFCB ()
	{
	for (int i = 0; i < MAXFCBS; i++)
		{
		if (fcbArray[i].buf == NULL)
			{
			return i;		//Not thread safe (But do not worry about it for this assignment)
			}
		}
	return (-1);  //all in use
	}
	
// Interface to open a buffered file
// Modification of interface for this assignment, flags match the Linux flags for open
// O_RDONLY, O_WRONLY, or O_RDWR
b_io_fd b_open (char * filename, int flags)
	{
	b_io_fd returnFd;

	//*** TODO ***:  Modify to save or set any information needed
	//
	//
		
	if (startup == 0) b_init();  //Initialize our system
	
	DirectoryEntry *fi;
	Container *parsedPath = parse_path(filename, fi);

	if (parsedPath == NULL){
		printf("invalid path\n");
	return -1;
	}

	// printf("parsed path:\n");
	// print_container(parsedPath);

	returnFd = b_getFCB();				// get our own file descriptor

	b_fcb *fcb = &fcbArray[returnFd];

	if(returnFd == -1){
		printf("FCB Error\n");
	}									// check for error - all used FCB's
	fcb->fi = fi;
	fcb->buf = malloc(B_CHUNK_SIZE);
	fcb->index = 0;
	fcb->buflen = 0;

	// if(flags == O_RDONLY){
	// 	fcb->flags = B_READ;
	// }
	// else if(flags == O_WRONLY){
	// 	fcb->flags = B_WRITE;
	// }
	// else if(flags == O_RDWR){
	// 	fcb->flags = B_READ | B_WRITE;
	// }
	fcb->flags = flags;
	fcb->currBlk = 0;
	fcb->numBlk = (fi->size + (B_CHUNK_SIZE -1)) / B_CHUNK_SIZE;



	return (returnFd);						// all set
	}


// Interface to seek function	
int b_seek (b_io_fd fd, off_t offset, int whence)
	{
	if (startup == 0) b_init();  //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
		{
		return (-1); 					//invalid file descriptor
		}
		
		
	return (0); //Change this
	}



// Interface to write function	
int b_write (b_io_fd fd, char * buffer, int count)
	{
		b_fcb *fcb = &fcbArray[fd];
		DirectoryEntry *fi = fcb->fi;

	if (startup == 0) b_init();  //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
		{
		return (-1); 					//invalid file descriptor
		}

		if(fcb->fi == -1){
			printf("ERROR with file descriptor\n");
			return -1;
		}

	if(fcb->flags == B_READ){
		printf("File opened as read-only, cannot write.\n");
		return -1;
	}
	else if(fcb->flags == B_WRITE){
		printf("File opened as write-only, cannot read.\n");
		return -1;
	}
	if(fcb->index >= fi->size){
		return 0;
	}
	int bytes_read = 0;
    while (bytes_read < count) {
        // Determine which block to read from
        int block_num = (fcb->index + bytes_read) / B_CHUNK_SIZE;


        // Determine the position within the block
        int block_offset = (fcb->index + bytes_read) % B_CHUNK_SIZE;

        // Determine how much to read from the block
        int remaining_block_bytes = B_CHUNK_SIZE - block_offset;
        int bytes_to_read = count - bytes_read;

        if (bytes_to_read > remaining_block_bytes) {
            bytes_to_read = remaining_block_bytes;
        }

        // Read from the block into the buffer
        memcpy(buffer + bytes_read, fcb->buf + block_offset, bytes_to_read);

        // Update bytes_read and fcb position
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
int b_read (b_io_fd fd, char * buffer, int count)
	{
        int bytes_read = 0;
		int bytes_left = count;
        //int bytes_returned;
        // int p1,p2,p3;
        // int num_blocks_to_copy;
        // int remaining_bytes;

	if (startup == 0) b_init();  //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
		{
		return (-1); 					//invalid file descriptor
		}

	while(bytes_left > 0 && fcbArray[fd].index < fcbArray[fd].buflen){
		int bytes_to_copy = fcbArray[fd].buflen - fcbArray[fd].index;
		if(bytes_to_copy > bytes_left){
			bytes_to_copy = bytes_left;
		}
		memcpy(buffer + bytes_read, fcbArray[fd].buf + fcbArray[fd].index, bytes_to_copy);
		fcbArray[fd].index += bytes_to_copy;
		bytes_read += bytes_to_copy;
		bytes_left -= bytes_to_copy;
		}

    // if we still need to read more bytes, read from disk
    while (bytes_left > 0 && fcbArray[fd].currBlk < fcbArray[fd].numBlk) {
        int bytes_to_read = B_CHUNK_SIZE;
        if (bytes_to_read > bytes_left) {
            bytes_to_read = bytes_left;
        }
        char *block_buffer = malloc(B_CHUNK_SIZE);

        b_read_block(fcbArray[fd].fi->data_locations[fcbArray[fd].currBlk], block_buffer);
        memcpy(buffer + bytes_read, block_buffer, bytes_to_read);
        fcbArray[fd].currBlk++;
        fcbArray[fd].buflen = bytes_to_read;
        fcbArray[fd].index = bytes_to_read;
        free(block_buffer);
        bytes_read += bytes_to_read;
        bytes_left -= bytes_to_read;
    }
		return (bytes_read);
// remaining_bytes = fcbArray[fd].buflen - fcbArray[fd].index;
// int file_position = (fcbArray[fd].currBlk * B_CHUNK_SIZE) - remaining_bytes;
// if((count + file_position) >fcbArray[fd].fi->size){
//     count = fcbArray[fd].fi->size - file_position;

//     if(count < 0){
//         printf("ERROR: Count: %d  \nDelivered: %d \nCurrBlk: %d \nIndex: %d\n",
//         count, file_position, fcbArray[fd].currBlk, fcbArray[fd].index);
//     }
// }
// if(remaining_bytes >= count){
//     p1 = count;
//     p2 = 0;
//     p3 = 0;
// }
// else{
//     p1 = remaining_bytes;

//     p3 = count - remaining_bytes;

//     num_blocks_to_copy = p3 / B_CHUNK_SIZE;
//     p2 = num_blocks_to_copy * B_CHUNK_SIZE;
//     p3 = p3 - p2;
// }
// if(p1 > 0){
//     memcpy(buffer, fcbArray[fd].buf + fcbArray[fd].index, p1);
//     fcbArray[fd].index = fcbArray[fd].index + p1;
// }
// if(p2 > 0){
//     bytes_read = LBAread (buffer+p1, num_blocks_to_copy, fcbArray[fd].currBlk +fcbArray[fd].index);

//     fcbArray[fd].currBlk += num_blocks_to_copy;
//     p2 = bytes_read * B_CHUNK_SIZE;

// }

// 	return (bytes_returned);	//Change this


	
	}
	
// Interface to Close the file	
int b_close (b_io_fd fd)
	{
        free(fcbArray[fd].buf);
        fcbArray[fd].buf = NULL;
        return 0;
	}
