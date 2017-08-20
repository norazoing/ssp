/*******************************************************************************
*                                                                              *
*                      KSCC - Bus Embeded System                               *
*                                                                              *
*            All rights reserved. No part of this publication may be           *
*            reproduced, stored in a retrieval system or transmitted           *
*            in any form or by any means  -  electronic, mechanical,           *
*            photocopying, recording, or otherwise, without the prior          *
*            written permission of LG CNS.                                     *
*                                                                              *
********************************************************************************
*                                                                              *
*  PROGRAM ID :       md4.c                                                    *
*                                                                              *
*  DESCRIPTION:       This program provides MD4 Message-Digest Algorithm.      *
*                                                                              *
*  ENTRY POINT:       None                   ** optional **                    *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: None                                                     *
*                                                                              *
*  INPUT FILES:       None                                                     *
*                                                                              *
*  OUTPUT FILES:      None                                                     *
*                                                                              *
*  SPECIAL LOGIC:     None                                                     *
*                                                                              *
********************************************************************************
*                         MODIFICATION LOG                                     *
*                                                                              *
*    DATE                SE NAME                      DESCRIPTION              *
* ---------- ---------------------------- ------------------------------------ *
* 2005/09/03 Solution Team Mi Hyun Noh  Initial Release                        *
*                                                                              *
*******************************************************************************/

// #include 	"../include/bus100.h"
#include "../system/bus_type.h"
#include "md4.h"

/*******************************************************************************
*  Declaration of variables                                                    *
*******************************************************************************/

typedef unsigned char *POINTER;
typedef u_int16_t UINT2;
typedef u_int32_t UINT4;

#define PROTO_LIST(list) list

// Constants for MD4Transform routine.

#define S11 3
#define S12 7
#define S13 11
#define S14 19
#define S21 3
#define S22 5
#define S23 9
#define S24 13
#define S31 3
#define S32 9
#define S33 11
#define S34 15

/*******************************************************************************
*  Declaration of function                                                     *
*******************************************************************************/

static void MD4Transform PROTO_LIST ( (UINT4 [4],  const unsigned char [64] ) ); 
static void Encode PROTO_LIST
	( (unsigned char *,  UINT4 *,  unsigned int) ); 
static void Decode PROTO_LIST
	( (UINT4 *,  const unsigned char *,  unsigned int) ); 

static unsigned char PADDING[64] = {
	0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
}; 

// F, G and H are basic MD4 functions.

#define F(x, y, z) ( ( (x) & (y) ) | ( ( ~x) & (z) ) )
#define G(x, y, z) ( ( (x) & (y) ) | ( (x) & (z) ) | ( (y) & (z) ) )
#define H(x, y, z) ( (x) ^ (y) ^ (z) )

// ROTATE_LEFT rotates x left n bits.

#define ROTATE_LEFT(x, n) ( ( (x) << (n) ) | ( (x) >> (32-(n) ) ) )

// FF, GG and HH are transformations for rounds 1, 2 and 3 
// Rotation is separate from addition to prevent recomputation 

#define FF(a, b, c, d, x, s) { \
		(a) +=  F ( (b), (c), (d) ) + (x); \
		(a) =  ROTATE_LEFT ( (a), (s) ); \
	}
#define GG(a, b, c, d, x, s) { \
		(a) +=  G ( (b), (c), (d) ) + (x) + (UINT4)0x5a827999; \
		(a) =  ROTATE_LEFT ( (a), (s) ); \
	}
#define HH(a, b, c, d, x, s) { \
		(a) +=  H ( (b), (c), (d) ) + (x) + (UINT4)0x6ed9eba1; \
		(a) =  ROTATE_LEFT ( (a), (s) ); \
	}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      MD4Init                                                  *
*                                                                              *
*  DESCRIPTION :      This program Begins an MD4 operation,writing a new       *
* 		      context. - MD4 initialization.                           *
*                                                                              *
*  INPUT PARAMETERS:  pstContext, pbyteInput, nInputLen      		       *
*                                                                              *
*  RETURN/EXIT VALUE:     None                                                 *
*                                                                              *
*  Author  : Mi Hyun Noh   						       *
*                                                                              *
*  DATE    : 2005-09-03 						       *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
 
void MD4Init (pstContext)
MD4_CTX *pstContext;	
{
	pstContext->count[0] =  pstContext->count[1] =  0;

	// Load magic initialization constants.

	pstContext->state[0] =  0x67452301;
	pstContext->state[1] =  0xefcdab89;
	pstContext->state[2] =  0x98badcfe;
	pstContext->state[3] =  0x10325476;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      MD4Update                                                *
*                                                                              *
*  DESCRIPTION :      This program continues an MD4 message-digest operation,  *
*		      processing another message block,and updating the context*
*                                                                              *
*  INPUT PARAMETERS:  pstContext, pbyteInput, nInputLen      		       *
*                                                                              *
*  RETURN/EXIT VALUE:     None                                                 *
*                                                                              *
*  Author  : Mi Hyun Noh   						       *
*                                                                              *
*  DATE    : 2005-09-03 						       *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/ 
 
void MD4Update (pstContext, pbyteInput, nInputLen)
MD4_CTX *pstContext;					// pstContext 
const unsigned char *pbyteInput;			// pbyteInput block
unsigned int nInputLen;	// length of pbyteInput block 
{
	unsigned int i;
	unsigned int nIndex;
	unsigned int nPartLen;

	// Compute number of bytes mod 64 
	nIndex  = (unsigned int)( (pstContext->count[0] >> 3) & 0x3F); 
	// Update number of bits 
	if ( (pstContext->count[0] += ( (UINT4)nInputLen << 3) )
			< ( (UINT4)nInputLen << 3) )
		pstContext->count[1]++; 
	pstContext->count[1] += ( (UINT4)nInputLen >> 29); 

	nPartLen  =  64 - nIndex;
	// Transform as many times as possible.
	 
	if (nInputLen >=  nPartLen) {
		memcpy ( (POINTER)&pstContext->buffer[nIndex], 
			(POINTER)pbyteInput, nPartLen); 
		MD4Transform (pstContext->state, pstContext->buffer); 

		for (i  =  nPartLen; i + 63 < nInputLen; i +=  64)
			MD4Transform (pstContext->state, &pbyteInput[i] ); 

		nIndex  =  0;
	}
	else
		i  =  0;

	// Buffer remaining pbyteInput 
	memcpy ( (POINTER)&pstContext->buffer[nIndex], (POINTER)&pbyteInput[i], 
		 nInputLen-i); 
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      MD4Final                                                 *
*                                                                              *
*  DESCRIPTION :      This program ends an MD4 message-digest operation,       *
*		      writing the the message digest and zeroizing the context.*
*                                                                              *
*  INPUT PARAMETERS:  abyteDigest, pstContext                                  *
*                                                                              *
*  RETURN/EXIT VALUE:     None                                                 *
*                                                                              *
*  Author  : Mi Hyun Noh   						       *
*                                                                              *
*  DATE    : 2005-09-03 						       *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/

void MD4Final (abyteDigest, pstContext)
unsigned char abyteDigest[16]; // message abyteDigest 
MD4_CTX *pstContext;	// pstContext 
{
	unsigned char bits[8]; 
	unsigned int index, padLen;

	// Save number of bits 
	Encode (bits, pstContext->count, 8); 

	// Pad out to 56 mod 64.
	 
	index  = (unsigned int)( (pstContext->count[0] >> 3) & 0x3f); 
	padLen  = (index < 56) ? (56 - index) : (120 - index); 
	MD4Update (pstContext, PADDING, padLen); 

	// Append length (before padding) 
	MD4Update (pstContext, bits, 8); 
	// Store state in abyteDigest 
	Encode (abyteDigest, pstContext->state, 16); 

	// Zeroize sensitive information.
	 
	memset ( (POINTER)pstContext, 0, sizeof ( *pstContext) ); 
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      MD4Transform                                             *
*                                                                              *
*  DESCRIPTION :      This program transforms state based on block.            *
*                                                                              *
*  INPUT PARAMETERS:  anState, byteBlock                                       *
*                                                                              *
*  RETURN/EXIT VALUE:     None                                                 *
*                                                                              *
*  Author  : Mi Hyun Noh   						       *
*                                                                              *
*  DATE    : 2005-09-03 						       *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/

static void MD4Transform (  anState, 
			   byteBlock  )
UINT4 anState[4]; 
const unsigned char byteBlock[64]; 
{
	UINT4 a  =  anState[0],  b  =  anState[1],  c  =  anState[2], 
			d  =  anState[3],  x[16]; 

	Decode (x, byteBlock, 64); 

	// Round 1 
	FF (a, b, c, d, x[ 0],  S11); // 1 
	FF (d, a, b, c, x[ 1],  S12); // 2 
	FF (c, d, a, b, x[ 2],  S13); // 3 
	FF (b, c, d, a, x[ 3],  S14); // 4 
	FF (a, b, c, d, x[ 4],  S11); // 5 
	FF (d, a, b, c, x[ 5],  S12); // 6 
	FF (c, d, a, b, x[ 6],  S13); // 7 
	FF (b, c, d, a, x[ 7],  S14); // 8 
	FF (a, b, c, d, x[ 8],  S11); // 9 
	FF (d, a, b, c, x[ 9],  S12); // 10 
	FF (c, d, a, b, x[10],  S13); // 11 
	FF (b, c, d, a, x[11],  S14); // 12 
	FF (a, b, c, d, x[12],  S11); // 13 
	FF (d, a, b, c, x[13],  S12); // 14 
	FF (c, d, a, b, x[14],  S13); // 15 
	FF (b, c, d, a, x[15],  S14); // 16 

	// Round 2 
	GG (a, b, c, d, x[ 0],  S21); // 17 
	GG (d, a, b, c, x[ 4],  S22); // 18 
	GG (c, d, a, b, x[ 8],  S23); // 19 
	GG (b, c, d, a, x[12],  S24); // 20 
	GG (a, b, c, d, x[ 1],  S21); // 21 
	GG (d, a, b, c, x[ 5],  S22); // 22 
	GG (c, d, a, b, x[ 9],  S23); // 23 
	GG (b, c, d, a, x[13],  S24); // 24 
	GG (a, b, c, d, x[ 2],  S21); // 25 
	GG (d, a, b, c, x[ 6],  S22); // 26 
	GG (c, d, a, b, x[10],  S23); // 27 
	GG (b, c, d, a, x[14],  S24); // 28 
	GG (a, b, c, d, x[ 3],  S21); // 29 
	GG (d, a, b, c, x[ 7],  S22); // 30 
	GG (c, d, a, b, x[11],  S23); // 31 
	GG (b, c, d, a, x[15],  S24); // 32 

	// Round 3 
	HH (a, b, c, d, x[ 0],  S31); // 33 
	HH (d, a, b, c, x[ 8],  S32); // 34 
	HH (c, d, a, b, x[ 4],  S33); // 35 
	HH (b, c, d, a, x[12],  S34); // 36 
	HH (a, b, c, d, x[ 2],  S31); // 37 
	HH (d, a, b, c, x[10],  S32); // 38 
	HH (c, d, a, b, x[ 6],  S33); // 39 
	HH (b, c, d, a, x[14],  S34); // 40 
	HH (a, b, c, d, x[ 1],  S31); // 41 
	HH (d, a, b, c, x[ 9],  S32); // 42 
	HH (c, d, a, b, x[ 5],  S33); // 43 
	HH (b, c, d, a, x[13],  S34); // 44 
	HH (a, b, c, d, x[ 3],  S31); // 45 
	HH (d, a, b, c, x[11],  S32); // 46 
	HH (c, d, a, b, x[ 7],  S33); // 47 
	HH (b, c, d, a, x[15],  S34); // 48 

	anState[0] +=  a;
	anState[1] +=  b;
	anState[2] +=  c;
	anState[3] +=  d;

	// Zeroize sensitive information.
	
	memset ( (POINTER)x, 0, sizeof (x) ); 
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      Encode                                                   *
*                                                                              *
*  DESCRIPTION :      This program encodes input (unsigned char) into output   *
*			(UINT4).Assumes len is a multiple of 4		       *
*                                                                              *
*  INPUT PARAMETERS:  unsigned char *pchOutput, UINT4 *pchInput,               * 
*		      unsigned int nLength          			       *
*                                                                              *
*  RETURN/EXIT VALUE:     None                                                 *
*                                                                              *
*  Author  : Mi Hyun Noh   						       *
*                                                                              *
*  DATE    : 2005-09-03 						       *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
 
static void Encode (pbyteOutput, 
		    pnIntput, 
		    nLength)
unsigned char *pbyteOutput;
UINT4 *pnIntput;
unsigned int nLength;
{
	unsigned int i, j;

	for (i  =  0, j  =  0; j < nLength; i++,  j +=  4) 
	{
		pbyteOutput[j] = (unsigned char)(pnIntput[i] & 0xff); 
		pbyteOutput[j+1] = (unsigned char)( (pnIntput[i] >> 8) & 0xff); 
		pbyteOutput[j+2] = (unsigned char)( (pnIntput[i] >> 16) & 0xff); 
		pbyteOutput[j+3] = (unsigned char)( (pnIntput[i] >> 24) & 0xff); 
	}
 
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      Decode                                                   *
*                                                                              *
*  DESCRIPTION :      This program decodes input (unsigned char) into output   *
*			(UINT4).Assumes len is a multiple of 4		       *
*                                                                              *
*  INPUT PARAMETERS:  UINT4 *pOutput, const unsigned char *pchInput,           *
*  		      unsigned int psLen           			       *
*                                                                              *
*  RETURN/EXIT VALUE:     None                                                 *
*                                                                              *
*  Author  : Mi Hyun Noh   						       *
*                                                                              *
*  DATE    : 2005-09-03 						       *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static void Decode (pnOutput, 
		    pbyteInput,
		    nLength)

UINT4 *pnOutput;
const unsigned char *pbyteInput;
unsigned int nLength;
{
	unsigned int i;
	unsigned int j;

	for (i  =  0, j  =  0; j < nLength; i++,  j +=  4)
		pnOutput[i] = ( (UINT4)pbyteInput[j] ) | 
			( ( (UINT4)pbyteInput[j+1] ) << 8) |
			( ( (UINT4)pbyteInput[j+2] ) << 16) | 
			( ( (UINT4)pbyteInput[j+3] ) << 24); 
}
