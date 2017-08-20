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
*  PROGRAM ID :       md4l.c                                                   *
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

//#include 	"../include/bus100.h"

#include "../system/bus_type.h"
#include "md4.h"

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      MD4End                                                   *
*                                                                              *
*  DESCRIPTION :      This program ends an MD4 message-digest operation,       *
*		      writing the the message digest and zeroizing the context.*
*                                                                              *
*  INPUT PARAMETERS:  abyteDigest, pstContext                                  *
*                                                                              *
*  RETURN/EXIT VALUE:     pchBuf                                               *
*                                                                              *
*  Author  : Mi Hyun Noh   						       *
*                                                                              *
*  DATE    : 2005-09-03 						       *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/

char *MD4End( MD4_CTX *pstContext, 
	      char *pchBuf )
{
	int i; 
	unsigned char digest[MD4_HASHBYTES]; 
	static const char hex[] = "0123456789abcdef"; 

	if ( !pchBuf )
		pchBuf = malloc( 33 ); 
	if ( !pchBuf )
		return 0; 
	
	MD4Final( digest, pstContext ); 
	
	for ( i = 0; i<MD4_HASHBYTES; i++ ) 
	{
		pchBuf[i+i] = hex[digest[i] >> 4]; 
		pchBuf[i+i+1] = hex[digest[i] & 0x0f]; 
	}
	pchBuf[i+i] = '\0'; 
	return pchBuf; 
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      MD4File                                                  *
*                                                                              *
*  DESCRIPTION :      This program continues an MD4 message-digest operation,  *
*		      processing another message block,and updating the context*
*                                                                              *
*  INPUT PARAMETERS:  pstContext, pbyteInput, nInputLen      		       *
*                                                                              *
*  RETURN/EXIT VALUE:     0/MD4End( &ctx, pchBuf )                             *
*                                                                              *
*  Author  : Mi Hyun Noh   						       *
*                                                                              *
*  DATE    : 2005-09-03 						       *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/ 

char *MD4File ( const char *pchFilename, 
	        char *pchBuf )
{
	unsigned char pchBuffer[BUFSIZ]; 
	MD4_CTX ctx; 
	int f, i, j; 

	MD4Init( &ctx ); 
	f = open( pchFilename, O_RDONLY ); 
	if ( f < 0 ) return 0; 
	
	while ( ( i = read( f, pchBuffer, sizeof pchBuffer ) ) > 0 ) 
	{
		MD4Update( &ctx, pchBuffer, i ); 
	}
	j = errno; 
	close( f ); 
	errno = j; 
	
	if ( i < 0 ) return 0; 
	
	return MD4End( &ctx, pchBuf ); 
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      MD4Data                                                  *
*                                                                              *
*  DESCRIPTION :      This program Begins an MD4 operation, writing a new      *
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
char *MD4Data ( const unsigned char *pbyteData, 
	        unsigned int nLength, 
	        char *pchBuf )
{
	MD4_CTX ctx; 

	MD4Init( &ctx ); 
	MD4Update( &ctx, pbyteData, nLength ); 
	
	return MD4End( &ctx, pchBuf ); 
}
