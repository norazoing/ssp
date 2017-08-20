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
*  PROGRAM ID :       des.h                                                    *
*                                                                              *
*  DESCRIPTION:       Includes prototypes of encrytion and decrytion functions *
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

/*******************************************************************************
*									       *
*  Declaration of Extern variables                                             *
*  KEY CONVERSION TABLE FOR EACH ROUND			      		       *
*									       *
*******************************************************************************/

extern char	 achKeyTable[16][49];
extern char	 achLTable[33];
extern char	 achRTable[33];
extern char	 achERTable[49];
extern char	 achPTable[33];
extern char	 achIPTable[65];
extern char	 achSBoxTable[32][17];
extern char      achInitialKey[17];
extern char      achFixedKey[10][17];
extern char	 achConvTable[17];
extern char	 achSwapTable[17];
extern char	 chResultR[4];		/* nesult of R( i ) */
extern char	 chResultL[4];		/* result of L( i ) */
extern char	 chResultE[6];		/* result of E( R( i-1 ) ) */
extern char	 chResultEXorK[6];	/* result of E( R( i-1 ) ) XOR K( i ) */
extern char	 chResultSBox[6];	/* result of looking up S-BOX */
extern char	 chResultP[6];		/* result of P( B ) */
extern char	 achInputKey[17]; 	/* bit mapped input key data */
extern char	 achInputMsg[17]; 	/* bit mapped input message */
extern char	 achOutMsg[17];		/* result of encryption or decryption*/
extern char	 achOrgMsg[17];


extern char	 achKey[9];		   /* KEY for each round */
extern int	 nLength;

/*******************************************************************************
*  Declaration of Function                                                     *
*******************************************************************************/

extern  void TransEncrypt( char *, char *, int *, char * );
extern  void TransDecrypt( char *, char *, int *, char * );
extern  void Encrypt ( void );
extern  void Decrypt ( void );
extern  void MakeRAndLZero ( void );
extern  void ExpandR ( void );
extern  void ModuloToAdd ( int round );
extern  void MakePB ( void );
extern  void LookUpSBox ( void );
extern  char GetSBox ( short int round, char c );
extern  void InverseIP ( void );
extern  void BitPermutation ( char *in_data, char *out_data, int data_length, 
			      char *table_name );

