/******************************************************************************
*
* Shuidushi Software Inc.
* (c) Copyright 2014 Shuidushi Software, Inc.
* ALL RIGHTS RESERVED.
*
*******************************************************************************
*
*  File Name: ttMemAlign.c
*
*  File Description:TT Memory Align Operate
*
*******************************Change History**********************************
* 
*    DD/MM/YYYY      Code Ver     Description       Author
*    -----------     --------     -----------       ------
*    14/May/2013      v1.0        Initial version   Kevin
*
*******************************************************************************/

#include	"ttMemAlign.h"
#include    <stdlib.h>
#include    <string.h>
#include    <stdio.h>


void *mem_malloc(unsigned int size, unsigned char alignment)
{
	unsigned char *mem_ptr;
	TT_MEM_INFO MemInfo;

	if (!alignment) {
		MemInfo.Flag = 0;
		MemInfo.Size = size + 1;

		MemInfo.VBuffer = malloc(MemInfo.Size);
		if(!MemInfo.VBuffer)
			return 0;

		mem_ptr = (unsigned char *)MemInfo.VBuffer;
		memset(mem_ptr, 0, size + 1);
		*mem_ptr = (unsigned char)1;

		/* Return the mem_ptr pointer */
		return ((void *)(mem_ptr+1));
	} else {
		unsigned char *tmp;
		MemInfo.Flag = 0;
		MemInfo.Size = size + alignment;

		MemInfo.VBuffer = malloc(MemInfo.Size);
		if(!MemInfo.VBuffer)
			return 0;

		tmp = (unsigned char *)MemInfo.VBuffer;
        memset(tmp, 0, size + alignment);
		/* Align the tmp pointer */
		mem_ptr =
			(unsigned char *) ((unsigned long) (tmp + alignment - 1) &
			(~((unsigned long) (alignment - 1))));

		/* Special case where malloc have already satisfied the alignment
		* We must add alignment to mem_ptr because we must store
		* (mem_ptr - tmp) in *(mem_ptr-1)
		* If we do not add alignment to mem_ptr then *(mem_ptr-1) points
		* to a forbidden memory space */
		if (mem_ptr == tmp)
			mem_ptr += alignment;

		/* (mem_ptr - tmp) is stored in *(mem_ptr-1) so we are able to retrieve
		* the real malloc block allocated and free it in xvid_free */
		*(mem_ptr - 1) = (unsigned char) (mem_ptr - tmp);

		/* Return the aligned pointer */
		return ((void *)mem_ptr);
	}

	return(0);
}

void mem_free(void *mem_ptr)
{

	unsigned char *ptr;

	if (mem_ptr == 0)
		return;

	/* Aligned pointer */
	ptr = mem_ptr;

	/* *(ptr - 1) holds the offset to the real allocated block
	* we sub that offset os we free the real pointer */
	ptr -= *(ptr - 1);

	/* Free the memory */
	free(ptr);
}
