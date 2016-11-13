/******************************************************************************
*
* Shuidushi Software Inc.
* (c) Copyright 2014 Shuidushi Software, Inc.
* ALL RIGHTS RESERVED.
*
*******************************************************************************
*
*  File Name: ttMemAlign.h
*
*  File Description:Memory align operate header file
*
*******************************Change History**********************************
* 
*    DD/MM/YYYY      Code Ver     Description       Author
*    -----------     --------     -----------       ------
*    14/May/2013      v1.0        Initial version   Kevin
*
*******************************************************************************/

#ifndef __TT_MEM_ALIGN_H__
#define __TT_MEM_ALIGN_H__

#include <stdlib.h>

typedef struct
{
	int					Size;				/*!< Buffer stride */
	int					Flag;
	void*				VBuffer;			/*!< user data pointer */
	void*				PBuffer;			/*!< user data pointer */
}TT_MEM_INFO;


void *mem_malloc(unsigned int size, unsigned char alignment);
void mem_free(void *mem_ptr);


#endif/* __TT_MEM_ALIGN_H__ */
