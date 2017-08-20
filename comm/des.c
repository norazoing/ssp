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
*  PROGRAM ID :       des.c                                                    *
*                                                                              *
*  DESCRIPTION:       This program provides data encryption and decrytion      *
*		      functions which handle key conversion tables.	       *
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

#include "../system/bus_type.h"
#include "des.h"


/*******************************************************************************
*  Declaration of variables                                                    *
*******************************************************************************/

// Key CONVERSION TABLE FOR EACH ROUND

char	 achKeyTable[16][49] = {{10, 51, 34, 60, 49, 17, 33, 57, 2, 9, 19, 42, 
				  3, 35, 26, 25, 44, 58, 59, 1, 36, 27, 18, 41, 
				 22, 28, 39, 54, 37, 4, 47, 30, 5, 53, 23, 29, 
				61, 21, 38, 63, 15, 20, 45, 14, 13, 62, 55, 31}, 
				{ 2, 43, 26, 52, 41, 9, 25, 49, 59, 1, 11, 34, 
				 60, 27, 18, 17, 36, 50, 51, 58, 57, 19, 10, 33, 
				 14, 20, 31, 46, 29, 63, 39, 22, 28, 45, 15, 21, 
				 53, 13, 30, 55, 7, 12, 37, 6, 5, 54, 47, 23}, 
				{51, 27, 10, 36, 25, 58, 9, 33, 43, 50, 60, 18, 
				 44, 11, 2, 1, 49, 34, 35, 42, 41, 3, 59, 17, 
				 61, 4, 15, 30, 13, 47, 23, 6, 12, 29, 62, 5, 
				 37, 28, 14, 39, 54, 63, 21, 53, 20, 38, 31, 7}, 
				{35, 11, 59, 49, 9, 42, 58, 17, 27, 34, 44, 2, 
				 57, 60, 51, 50, 33, 18, 19, 26, 25, 52, 43, 1, 
				 45, 55, 62, 14, 28, 31, 7, 53, 63, 13, 46, 20, 
				 21, 12, 61, 23, 38, 47, 5, 37, 4, 22, 15, 54}, 
				{19, 60, 43, 33, 58, 26, 42, 1, 11, 18, 57, 51, 
				 41, 44, 35, 34, 17, 2, 3, 10, 9, 36, 27, 50, 
				 29, 39, 46, 61, 12, 15, 54, 37, 47, 28, 30, 4, 
				  5, 63, 45, 7, 22, 31, 20, 21, 55, 6, 62, 38}, 
				{ 3, 44, 27, 17, 42, 10, 26, 50, 60, 2, 41, 35, 
				 25, 57, 19, 18, 1, 51, 52, 59, 58, 49, 11, 34, 
				 13, 23, 30, 45, 63, 62, 38, 21, 31, 12, 14, 55, 
				 20, 47, 29, 54, 6, 15, 4, 5, 39, 53, 46, 22}, 
				{52, 57, 11, 1, 26, 59, 10, 34, 44, 51, 25, 19, 
				  9, 41, 3, 2, 50, 35, 36, 43, 42, 33, 60, 18, 
				 28, 7, 14, 29, 47, 46, 22, 5, 15, 63, 61, 39, 
				  4, 31, 13, 38, 53, 62, 55, 20, 23, 37, 30, 6}, 
				{36, 41, 60, 50, 10, 43, 59, 18, 57, 35, 9, 3, 
				 58, 25, 52, 51, 34, 19, 49, 27, 26, 17, 44, 2, 
				 12, 54, 61, 13, 31, 30, 6, 20, 62, 47, 45, 23, 
				 55, 15, 28, 22, 37, 46, 39, 4, 7, 21, 14, 53}, 
				{57, 33, 52, 42, 2, 35, 51, 10, 49, 27, 1, 60, 
				 50, 17, 44, 43, 26, 11, 41, 19, 18, 9, 36, 59, 
				  4, 46, 53, 5, 23, 22, 61, 12, 54, 39, 37, 15, 
				 47, 7, 20, 14, 29, 38, 31, 63, 62, 13, 6, 45}, 
				{41, 17, 36, 26, 51, 19, 35, 59, 33, 11, 50, 44, 
				 34, 1, 57, 27, 10, 60, 25, 3, 2, 58, 49, 43, 
				 55, 30, 37, 20, 7, 6, 45, 63, 38, 23, 21, 62, 
				 31, 54, 4, 61, 13, 22, 15, 47, 46, 28, 53, 29}, 
				{25, 1, 49, 10, 35, 3, 19, 43, 17, 60, 34, 57, 
				 18, 50, 41, 11, 59, 44, 9, 52, 51, 42, 33, 27, 
				 39, 14, 21, 4, 54, 53, 29, 47, 22, 7, 5, 46, 
				 15, 38, 55, 45, 28, 6, 62, 31, 30, 12, 37, 13}, 
				{ 9, 50, 33, 59, 19, 52, 3, 27, 1, 44, 18, 41, 
				  2, 34, 25, 60, 43, 57, 58, 36, 35, 26, 17, 11, 
				 23, 61, 5, 55, 38, 37, 13, 31, 6, 54, 20, 30, 
				62, 22, 39, 29, 12, 53, 46, 15, 14, 63, 21, 28}, 
				{58, 34, 17, 43, 3, 36, 52, 11, 50, 57, 2, 25, 
				 51, 18, 9, 44, 27, 41, 42, 49, 19, 10, 1, 60, 
				  7, 45, 20, 39, 22, 21, 28, 15, 53, 38, 4, 14, 
				 46, 6, 23, 13, 63, 37, 30, 62, 61, 47, 5, 12}, 
				{42, 18, 1, 27, 52, 49, 36, 60, 34, 41, 51, 9, 
				 35, 2, 58, 57, 11, 25, 26, 33, 3, 59, 50, 44, 
				 54, 29, 4, 23, 6, 5, 12, 62, 37, 22, 55, 61, 
				 30, 53, 7, 28, 47, 21, 14, 46, 45, 31, 20, 63}, 
				{26, 2, 50, 11, 36, 33, 49, 44, 18, 25, 35, 58, 
				 19, 51, 42, 41, 60, 9, 10, 17, 52, 43, 34, 57, 
				 38, 13, 55, 7, 53, 20, 63, 46, 21, 6, 39, 45, 
				 14, 37, 54, 12, 31, 5, 61, 30, 29, 15, 4, 47}, 
				{18, 59, 42, 3, 57, 25, 41, 36, 10, 17, 27, 50, 
				 11, 43, 34, 33, 52, 1, 2, 9, 44, 35, 26, 49, 
				 30, 5, 47, 62, 45, 12, 55, 38, 13, 61, 31, 37, 
				  6, 29, 46, 4, 23, 28, 53, 22, 21, 7, 63, 39}
			    }; 


// CONVERSION TABLE FOR MAKING chResultL( 0 ) FROM ORIGINAL MESSAGE	  

char	 achLTable[33]  = 
                 { 58, 50, 42, 34, 26, 18, 10, 2, 60, 52, 44, 36, 28, 20, 12, 4, 
		  62, 54, 46, 38, 30, 22, 14, 6, 64, 56, 48, 40, 32, 24, 16, 8}; 
									      
//  CONVERSION TABLE FOR MAKING chResultR( 0 ) FROM ORIGINAL MESSAGE

char	 achRTable[33]  = 
		{ 57, 49, 41, 33, 25, 17, 9, 1, 59, 51, 43, 35, 27, 19, 11, 3, 
		  61, 53, 45, 37, 29, 21, 13, 5, 63, 55, 47, 39, 31, 23, 15, 7}; 

//  CONVERSION TABLE FOR EXPANDING chResultR( i )

char	 achERTable[49] =  
		{ 32, 1, 2, 3, 4, 5, 4, 5, 6, 7, 8, 9, 8, 9, 10, 11, 
		 12, 13, 12, 13, 14, 15, 16, 17, 16, 17, 18, 19, 20, 21, 20, 21, 
		 22, 23, 24, 25, 24, 25, 26, 27, 28, 29, 28, 29, 30, 31, 32, 1}; 

//  CONVERSION TABLE FOR PERMUTATION chResultP( chResultSBox )

char	 achPTable[33]  = 
		{ 16, 7, 20, 21, 29, 12, 28, 17, 1, 15, 23, 26, 5, 18, 31, 10, 
		  2, 8, 24, 14, 32, 27, 3, 9, 19, 13, 30, 6, 22, 11, 4, 25}; 

//  CONVERSION TABLE FOR INVERSE INITIAL PERMUTATION

char	 achIPTable[65] = 
		{ 40, 8, 48, 16, 56, 24, 64, 32, 39, 7, 47, 15, 55, 23, 63, 31, 
		  38, 6, 46, 14, 54, 22, 62, 30, 37, 5, 45, 13, 53, 21, 61, 29, 
		  36, 4, 44, 12, 52, 20, 60, 28, 35, 3, 43, 11, 51, 19, 59, 27, 
		  34, 2, 42, 10, 50, 18, 58, 26, 33, 1, 41, 9, 49, 17, 57, 25}; 

// S - BOX 

char	 achSBoxTable[32][17] = {
			{14, 4, 13, 1, 2, 15, 11, 8, 3, 10, 6, 12, 5, 9, 0, 7}, 
			{ 0, 15, 7, 4, 14, 2, 13, 1, 10, 6, 12, 11, 9, 5, 3, 8}, 
			{ 4, 1, 14, 8, 13, 6, 2, 11, 15, 12, 9, 7, 3, 10, 5, 0}, 
			{15, 12, 8, 2, 4, 9, 1, 7, 5, 11, 3, 14, 10, 0, 6, 13}, 
			{15, 1, 8, 14, 6, 11, 3, 4, 9, 7, 2, 13, 12, 0, 5, 10}, 
			{ 3, 13, 4, 7, 15, 2, 8, 14, 12, 0, 1, 10, 6, 9, 11, 5}, 
			{ 0, 14, 7, 11, 10, 4, 13, 1, 5, 8, 12, 6, 9, 3, 2, 15}, 
			{13, 8, 10, 1, 3, 15, 4, 2, 11, 6, 7, 12, 0, 5, 14, 9}, 
			{10, 0, 9, 14, 6, 3, 15, 5, 1, 13, 12, 7, 11, 4, 2, 8}, 
			{13, 7, 0, 9, 3, 4, 6, 10, 2, 8, 5, 14, 12, 11, 15, 1}, 
			{13, 6, 4, 9, 8, 15, 3, 0, 11, 1, 2, 12, 5, 10, 14, 7}, 
			{ 1, 10, 13, 0, 6, 9, 8, 7, 4, 15, 14, 3, 11, 5, 2, 12}, 
			{ 7, 13, 14, 3, 0, 6, 9, 10, 1, 2, 8, 5, 11, 12, 4, 15}, 
			{13, 8, 11, 5, 6, 15, 0, 3, 4, 7, 2, 12, 1, 10, 14, 9}, 
			{10, 6, 9, 0, 12, 11, 7, 13, 15, 1, 3, 14, 5, 2, 8, 4}, 
			{ 3, 15, 0, 6, 10, 1, 13, 8, 9, 4, 5, 11, 12, 7, 2, 14}, 
			{ 2, 12, 4, 1, 7, 10, 11, 6, 8, 5, 3, 15, 13, 0, 14, 9}, 
			{14, 11, 2, 12, 4, 7, 13, 1, 5, 0, 15, 10, 3, 9, 8, 6}, 
			{ 4, 2, 1, 11, 10, 13, 7, 8, 15, 9, 12, 5, 6, 3, 0, 14}, 
			{11, 8, 12, 7, 1, 14, 2, 13, 6, 15, 0, 9, 10, 4, 5, 3}, 
			{12, 1, 10, 15, 9, 2, 6, 8, 0, 13, 3, 4, 14, 7, 5, 11}, 
			{10, 15, 4, 2, 7, 12, 9, 5, 6, 1, 13, 14, 0, 11, 3, 8}, 
			{ 9, 14, 15, 5, 2, 8, 12, 3, 7, 0, 4, 10, 1, 13, 11, 6}, 
			{ 4, 3, 2, 12, 9, 5, 15, 10, 11, 14, 1, 7, 6, 0, 8, 13}, 
			{ 4, 11, 2, 14, 15, 0, 8, 13, 3, 12, 9, 7, 5, 10, 6, 1}, 
			{13, 0, 11, 7, 4, 9, 1, 10, 14, 3, 5, 12, 2, 15, 8, 6}, 
			{ 1, 4, 11, 13, 12, 3, 7, 14, 10, 15, 6, 8, 0, 5, 9, 2}, 
			{ 6, 11, 13, 8, 1, 4, 10, 7, 9, 5, 0, 15, 14, 2, 3, 12}, 
			{13, 2, 8, 4, 6, 15, 11, 1, 10, 9, 3, 14, 5, 0, 12, 7}, 
			{ 1, 15, 13, 8, 10, 3, 7, 4, 12, 5, 6, 11, 0, 14, 9, 2}, 
			{ 7, 11, 4, 1, 9, 12, 14, 2, 0, 6, 10, 13, 15, 3, 5, 8}, 
			{ 2, 1, 14, 7, 4, 10, 8, 13, 15, 12, 9, 0, 3, 5, 6, 11}
			}; 
									  
//  GENERAL PURPOSE BUFFERS  ( INITIAL Key, FIXED Key )	

char  achInitialKey[17] = { "F37C37FAE5A05D18" };  // final Key 96.1.28 
//	{36, 46, 33, 31, 32, 35, 35, 32, 39, 39, 33, 34, 34, 35, 32, 37 }; 
				 
char achFixedKey[10][17] =  { 
	{0x37, 0x39, 0x42, 0x34, 0x39, 0x46, 0x34, 0x41, 0x46, 0x35, 0x42, 0x30,
	 0x36, 0x42, 0x31, 0x36}, 				
	{0x44, 0x46, 0x34, 0x41, 0x46, 0x35, 0x41, 0x33, 0x36, 0x45, 0x34, 0x39,
	 0x46, 0x34, 0x41, 0x30}, 			
	{0x45, 0x30, 0x36, 0x42, 0x31, 0x37, 0x43, 0x31, 0x37, 0x44, 0x32, 0x38,
	 0x44, 0x33, 0x38, 0x31}, 			
	{0x35, 0x37, 0x44, 0x32, 0x38, 0x44, 0x33, 0x39, 0x45, 0x37, 0x39, 0x32,
	 0x37, 0x44, 0x32, 0x38}, 			
	{0x32, 0x34, 0x39, 0x45, 0x35, 0x39, 0x31, 0x35, 0x43, 0x31, 0x38, 0x43,
	 0x32, 0x38, 0x44, 0x33}, 			   
	{0x33, 0x36, 0x42, 0x30, 0x36, 0x43, 0x31, 0x37, 0x43, 0x32, 0x37, 0x44,
	 0x33, 0x38, 0x45, 0x33}, 			
	{0x34, 0x36, 0x43, 0x45, 0x37, 0x43, 0x32, 0x38, 0x44, 0x33, 0x38, 0x45, 
	 0x33, 0x39, 0x45, 0x34}, 			   
	{0x36, 0x39, 0x45, 0x34, 0x39, 0x30, 0x34, 0x42, 0x31, 0x37, 0x43, 0x32, 
	 0x37, 0x45, 0x32, 0x41}, 			
	{0x37, 0x43, 0x45, 0x37, 0x43, 0x32, 0x38, 0x44, 0x33, 0x38, 0x45, 0x34,
	 0x39, 0x46, 0x34, 0x44}, 			
	{0x42, 0x44, 0x32, 0x38, 0x45, 0x33, 0x39, 0x46, 0x35, 0x42, 0x30, 0x37,
	 0x43, 0x31, 0x32, 0x38}
	}; 

//  GENERAL PURPOSE BUFFERS 

char	 achConvTable[17] = {'0', '1', '2', '3', '4', '5', '6', '7', 
	 '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'}; 

char	 achSwapTable[17] = {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 
			 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 
			 0x3C, 0x3D, 0x3E, 0x3F}; 

char	 chResultR[4]; 		/* result of chResultR( i ) */
char	 chResultL[4]; 		/* result of chResultL( i ) */
char	 chResultE[6]; 		/* result of chResultE( chResultR( i-1 ) ) */
char	 chResultEXorK[6]; 	/* result of chResultE( chResultR( i-1 ) ) 
					XOR K( i ) */
char	 chResultSBox[6]; 	/* result of looking up S-BOX */
char	 chResultP[6]; 		/* result of chResultP( chResultSBox ) */
char	 achInputKey[17]; 	/* bit mapped input Key data */
char	 achInputMsg[17]; 	/* bit mapped input message */
char	 achOutMsg[17]; 	/* result of Encryption or Decryption */
char	 achOrgMsg[17]; 


char	 achKey[9]; 		   /* Key for each round */
int	 nLength; 

/*******************************************************************************
*  Declaration of function                                                     *
*******************************************************************************/

void TransEncrypt( char *, char *, int *, char * );  
		//old TransEncrypt new TransEncrypt
void TransDecrypt( char *, char *, int *, char * );  
		//old TransDecrypt new TransDecrypt


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      TransEncrypt                                             *
*                                                                              *
*  DESCRIPTION :      This program provides data encryption main function      *
*                                                                              *
*  INPUT PARAMETERS:  char *pchOriginalMsg,char *pchEncryptMsg,		       *
*		      int *pnWknLength,char *pchEnKey			       *
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

void TransEncrypt( char *pchOriginalMsg, 
		   char *pchEncryptMsg,
		   int *pnWknLength, 
		   char *pchEnKey )
{
	int i; 
	int j; 
	int k; 
	int lm; 
	char	 chTwoChar[3]; 

	strncpy( &achInputKey[0], &pchEnKey[0],  ( int )16 ); 

	nLength = *pnWknLength; 

	for ( i = 0; i<16; i++ ) 
	{
	  if ( achInputKey[i] > '@' ) achInputKey[i] = achInputKey[i] - 7; 
	}
	
	for ( i = 0; i<8; i++ ) 
	{
	  achInputKey[i] = ( ( achInputKey[i*2] << 4 ) & 0x00f0 ) |
		    ( achInputKey[i*2+1] & 0x000f ); 
	}

	j = nLength/16; 

	for( k = 0; k < j; k++ )
	{
	  strncpy( achInputMsg, &pchOriginalMsg[k*16], 16 ); 

	  for ( i = 0; i<16; i++ ) {
		  if ( achInputMsg[i] > '@' ) 
		  	achInputMsg[i] = achInputMsg[i] - 7; 
	  }
	  for ( i = 0; i<8; i++ ) {
		  achInputMsg[i] = ( ( achInputMsg[i*2] << 4 ) & 0x00f0 ) |
			 ( achInputMsg[i*2+1] & 0x000f ); 
	  }
	  for ( i = 0; i<3; i++ ) Encrypt( ); 

	  for ( i = 0; i <8; i++ )
	  {
		  lm = ( int ) achOutMsg[i] & 0x00ff; 
		  sprintf( chTwoChar, "%02X", lm ); 
		  achOrgMsg[( i*2 )] = chTwoChar[0]; 
		  achOrgMsg[( i*2 )+1] = chTwoChar[1]; 
		  achOrgMsg[( i*2 )+2] = 0x00; 
	  }
	  strncpy( &pchEncryptMsg[k*16], achOrgMsg, 16 ); 
	}
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      TransDecrypt                                             *
*                                                                              *
*  DESCRIPTION :      This program provides data decryption main function.     *
*                                                                              *
*  INPUT PARAMETERS:  char *pchSubMsg,char *pchDecryptMsg,		       *
*	       	      int *pnWknLength,char *pchEnKey			       *
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

void TransDecrypt( char *pchSubMsg, 
	       char *pchDecryptMsg, 
	       int *pnWknLength, 
	       char *pchEnKey )

{
	int i; 
	int j; 
	int k; 
	int lm;
	int mm; 
	char	 achTwoChar[3]; 

	nLength = *pnWknLength; 
	strncpy( achInputKey, &pchEnKey[0],  16 ); 

	for ( i = 0; i<16; i++ ) 
	{
	  if ( achInputKey[i] > '@' ) achInputKey[i] = achInputKey[i] - 7; 
	}
	
	for ( i = 0; i<8; i++ ) 
	{
	  achInputKey[i] = ( ( achInputKey[i*2] << 4 ) & 0x00f0 ) |
			  ( achInputKey[i*2+1] & 0x000f ); 
	}

	j = nLength/16; 
	for( k = 0; k<j; k++ )
	{
	  strncpy( achOutMsg, &pchSubMsg[k*16], 16 ); 
	  achOutMsg[16] = 0x00; 

	  for ( i = 0; i<16; i++ ) 
	  {
		  for ( mm = 0; mm<16; mm++ ){
		    if ( achOutMsg[i] ==  achConvTable[mm] ) 
		    	achOutMsg[i] = achSwapTable[mm]; 
		  }
	  }
	  for ( i = 0; i<8; i++ ) 
	  {
		    achOutMsg[i] = ( ( achOutMsg[i*2] << 4 ) & 0x00f0 ) |
				  ( achOutMsg[i*2+1] & 0x000f ); 
	  }
	  Decrypt( ); 
	  for ( i = 0; i <8; i++ )
	  {
		  lm = ( int ) achOutMsg[i] & 0x00ff; 
		  sprintf( achTwoChar, "%02X", lm ); 
		  achOrgMsg[( i*2 )] = achTwoChar[0]; 
		  achOrgMsg[( i*2 )+1] = achTwoChar[1]; 
		  achOrgMsg[( i*2 )+2] = 0x00; 
	  }
	  strncpy( &pchDecryptMsg[k*16], achOrgMsg, 16 ); 
	 }

}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      Encrypt	                                               *
*                                                                              *
*  DESCRIPTION :      This program provides data encryption sub function       *
*                                                                              *
*  INPUT PARAMETERS:  char *pchOriginalMsg,char *pchEncryptMsg,		       *
*		      int *pnWknLength,char *pchEnKey			       *
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

void Encrypt( void )  //old Encrypt new Encrypt
{
	int	i; 

	MakeRAndLZero( ); 	/* make chResultR( 0 ) and chResultL( 0 )*/
	for ( i = 1; i<17; i++ ) { /* from round 1 to round 16 */
	  ExpandR( ); 		   /* expanding chResultR( i ) */
	  ModuloToAdd( i ); 	   /* chResultE( chResultR( i-1 ) XOR K( i ) */
	  MakePB( ); 		   /* make chResultR( i ) and chResultL( i ) */
	}
	InverseIP( ); 		   /* inverse INITIAL PERMUTATION */
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      Decrypt                                                  *
*                                                                              *
*  DESCRIPTION :      This program provides data decryption sub function.      *
*                                                                              *
*  INPUT PARAMETERS:  char *pchSubMsg,char *pchDecryptMsg,		       *
*	       	      int *pnWknLength,char *pchEnKey			       *
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

void Decrypt( void )  //old Decrypt new Decrypt
{
	int	i; 

	for ( i = 0; i<8; i++ ) achInputMsg[i] = achOutMsg[i]; 
	MakeRAndLZero( ); 	   /* make chResultR( 0 ) and chResultL( 0 ) */
	for ( i = 16; i>0; i-- ) 
	{ /* from round 16 to round 1 */
	  ExpandR( ); 		   /* expanding chResultR( i ) */
	  ModuloToAdd( i ); 	   /* chResultE( chResultR( i-1 ) XOR K( i ) */
	  MakePB( ); 		   /* make chResultR( i ) and chResultL( i ) */
	}
	InverseIP( ); 		   /* inverse INITIAL PERMUTATION */
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      MakeRAndLZero                                            *
*                                                                              *
*  DESCRIPTION :      This program converts from source data to R_table and    *
*		      L_table.			                               *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
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

void MakeRAndLZero( void )  //old MakeRAndLZero new MakeRAndLZero
{
	BitPermutation( achInputMsg, chResultR, 4, achRTable ); 
	BitPermutation( achInputMsg, chResultL, 4, achLTable ); 
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      ExpandR                                                  *
*                                                                              *
*  DESCRIPTION :      This program converts from R_table to E_table.	       *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
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

void ExpandR( void )  //old ExpandR new ExpandR
{
	BitPermutation( chResultR, chResultE, 6, achERTable ); 
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      ModuloToAdd                                              *
*                                                                              *
*  DESCRIPTION :      This program generates keys by reading input keys.       *
*                                                                              *
*  INPUT PARAMETERS:  int round                                                *
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

void ModuloToAdd( int round ) //old ModuloToAdd new ModuloToAdd
{
	int	i; 

	BitPermutation( achInputKey, achKey, 6, achKeyTable[round-1] ); 
	
	for ( i = 0; i<6; i++ )
	{
		chResultEXorK[i] = ( chResultE[i] ^ achKey[i] ); 
	}
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      MakePB                                                   *
*                                                                              *
*  DESCRIPTION :      This program converts from B_table and P_Table to R_table*
*		      and L_table.		                               *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
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

void MakePB( void )  //old MakePB new MakePB
{
	int	i; 
	char	achTemp[5]; 

	LookUpSBox( ); 
	BitPermutation( chResultSBox, chResultP, 4, achPTable ); 
	for ( i = 0; i<4; i++ ) achTemp[i] = chResultR[i]; 
	for ( i = 0; i<4; i++ ) chResultR[i] = ( chResultP[i] ^ chResultL[i] ); 
	for ( i = 0; i<4; i++ ) chResultL[i] = achTemp[i]; 
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      LookUpSBox                                               *
*                                                                              *
*  DESCRIPTION :      This program converts from A_Table and S_Box to B_table  *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
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

void LookUpSBox( void )  //old LookUpSBox new LookUpSBox
{
	char c; 
	char d; 

	c = chResultEXorK[0]; 
	c = ( c >> 2 ) & 0x3f; 
	d = GetSBox( 0, c ); 
	chResultSBox[0] = d << 4; 
	c = chResultEXorK[0]; 
	c = ( c << 4 ) & 0x30; 
	d = chResultEXorK[1]; 
	d = ( d >> 4 ) & 0x0f; 
	c = ( c | d ) & 0x3f; 
	d = GetSBox( 1, c ); 
	chResultSBox[0] = chResultSBox[0] | d; 
	c = chResultEXorK[1]; 
	c = ( c << 2 ) & 0x3c; 
	d = chResultEXorK[2]; 
	d = ( d >> 6 ) & 0x03; 
	c = ( c | d ) & 0x3f; 
	d = GetSBox( 2, c ); 
	chResultSBox[1] = d << 4; 
	c = chResultEXorK[2] & 0x3f; 
	chResultSBox[1] = chResultSBox[1] | GetSBox( 3, c ); 
	c = chResultEXorK[3]; 
	c = ( c >> 2 ) & 0x3f; 
	d = GetSBox( 4, c ); 
	chResultSBox[2] = d << 4; 
	c = chResultEXorK[3]; 
	c = ( c << 4 ) & 0x30; 
	d = chResultEXorK[4]; 
	d = ( d >> 4 ) & 0x0f; 
	c = ( c | d ) & 0x3f; 
	d = GetSBox( 5, c ); 
	chResultSBox[2] = chResultSBox[2] | d; 
	c = chResultEXorK[4]; 
	c = ( c << 2 ) & 0x3c; 
	d = chResultEXorK[5]; 
	d = ( d >> 6 ) & 0x03; 
	c = ( c | d ) & 0x3f; 
	d = GetSBox( 6, c ); 
	chResultSBox[3] = d << 4; 
	c = chResultEXorK[5] & 0x3f; 
	chResultSBox[3] = chResultSBox[3] | GetSBox( 7, c ); 
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      GetSBox                                                  *
*                                                                              *
*  DESCRIPTION :      This program reads S_Box data.                           *
*                                                                              *
*  INPUT PARAMETERS:  short int sRound, char chC			       *
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

char GetSBox( short int sRound, 
	       char chC )  //old GetSBox new GetSBox
{
	short int j; 
	short int k; 
	char	chD; 

	j = ( chC >> 4 ) & 0x02; 
	j = ( j | ( chC & 0x01 ) ) + ( sRound *4 ); 
	k = ( chC >> 1 ) & 0x0f; 
	chD = achSBoxTable[j][k]; 
	return ( chD ); 
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      InverseIP                                                *
*                                                                              *
*  DESCRIPTION :      This program gerates out_msg by reading IP_table.        *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
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

void InverseIP( void )  //old InverseIP new InverseIP
{
	int	i; 
	char	chTemp[9]; 

	for ( i = 0; i<4; i++ ) chTemp[i]	 = chResultR[i]; 
	for ( i = 0; i<4; i++ ) chTemp[i+4] = chResultL[i]; 
	BitPermutation( chTemp, achOutMsg, 8, achIPTable ); 
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      BitPermutation                                           *
*                                                                              *
*  DESCRIPTION :      This program converts from input data to output data by  *
*		      reading file name.                                       *
*                                                                              *
*  INPUT PARAMETERS:  char *pchInData, char *pchOutData, 		       *
*		      int nDatanLength, char *pchTableName 		       *
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

void BitPermutation( char *pchInData, 
		     char *pchOutData, 
		     int nDatanLength, 
		     char *pchTableName )
  //old BitPermutation new BitPermutation
{
	char c; 
	int i; 
	int j; 
	int k; 
	int l; 
	int m; 
	int o; 
	unsigned char achBitTable[9] = 
		{ 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 }; 

	for ( i = 0; i<nDatanLength; i++ ) 
	{
	  c = 0; 
	  for ( j = 0; j<8; j++ ) 
	  {
		  k = ( int ) pchTableName[i*8+j] - 1; 
		  l = k / 8; 
		  m = k % 8; 
		  if ( ( pchInData[l] & achBitTable[m] ) !=  0 ) o = 0x01; 
		  else o = 0x00; 
		  c = ( int ) ( ( c << 1 ) & 0xfe ) | o; 
	  }
	  pchOutData[i] = c; 
	}
}
