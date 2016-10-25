/******************************************************************************
*
* Shuidushi Software Inc.
* (c) Copyright 2014 Shuidushi Software, Inc.
* ALL RIGHTS RESERVED.
*
*******************************************************************************
*
*  File Name: ttMP3DecHuffman.h
*
*  File Description:TT MP3 decoder huffman decoder header file
*
*******************************Change History**********************************
* 
*    DD/MM/YYYY      Code Ver     Description       Author
*    -----------     --------     -----------       ------
*    30/June/2013      v1.0        Initial version   Kevin
*
*******************************************************************************/

#ifndef __TT_MP3DECHUFFMAN_H__
#define __TT_MP3DECHUFFMAN_H__

typedef unsigned short U16;

union huff_pair {
	struct {
		U16 finalB :  1;
		U16 bits   :  3;
		U16 offset : 12;
	} ptr;  
	struct {
		U16 finalB :  1;
		U16 hLen   :  3;
		U16 xHuff  :  4;
		U16 yHuff  :  4;
	} value;

	U16 finalB   :  1;
};
union huff_quad {
	struct {
		U16 finalB :  1;
		U16 bits   :  3;
		U16 offset : 12;
	} ptr;  
	struct {
		U16 finalB :  1;
		U16 hLen   :  3;
		U16 vHuff  :  1;
		U16 wHuff  :  1;
		U16 xHuff  :  1;
		U16 yHuff  :  1;
	} value;

	U16 finalB   :  1;
};

struct huff_table {
	union huff_pair const *pTable;
	U16 linBits;
	U16 startBits;
};

extern union huff_quad const *const pHuff_quad_table[2];
extern struct huff_table const huff_pair_table[32];

#endif //__TT_MP3DECHUFFMAN_H__
