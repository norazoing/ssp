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
*  PROGRAM ID :       md4.h                                                    *
*                                                                              *
*  DESCRIPTION:       Header file for MD4C.C MD4 Message-Digest  Algorithm     *
*                                                                              *
*  REMARKS    :                                                                *
*									       *
********************************************************************************
*                         MODIFICATION LOG                                     *
*                                                                              *
*    DATE                SE NAME                      DESCRIPTION              *
* ---------- ---------------------------- ------------------------------------ *
* 2005/09/03 Solution Team Mi Hyun Noh  Initial Release                        *
*                                                                              *
*******************************************************************************/

#ifndef _MD4_H_
#define _MD4_H_

#define MD4_HASHBYTES  16

/*******************************************************************************
*  Declaration of Structure Type  MD4 context.                                 *
*******************************************************************************/

typedef struct MD4Context 
{
  u_int32_t state[4];	// state (ABCD) 
  u_int32_t count[2];	// number of bits, modulo 2^64 (lsb first) 
  unsigned char buffer[64];	// input buffer 
} MD4_CTX;

#include <sys/cdefs.h>

__BEGIN_DECLS

/*******************************************************************************
*  Declaration of Function                                                     *
*******************************************************************************/

void   MD4Init(MD4_CTX *);
void   MD4Update(MD4_CTX *, const unsigned char *, unsigned int);
void   MD4Final(unsigned char [MD4_HASHBYTES], MD4_CTX *);
char * MD4End(MD4_CTX *, char *);
char * MD4File(const char *, char *);
char * MD4Data(const unsigned char *, unsigned int, char *);
__END_DECLS

#endif // _MD4_H_
