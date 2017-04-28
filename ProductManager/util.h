/*
 * util.h
 *
 *  Created on: 2017. 4. 21.
 *      Author: LGCNS
 */

#ifndef UTIL_H_
#define UTIL_H_
#include<stdio.h>
#include<time.h>
#include<string.h>
#include<stdlib.h>

typedef unsigned char byte, BYTE; //1 // 1byte
typedef unsigned short int word, WORD; // 2byte
typedef unsigned long dword, DWORD; // 4bYTE
typedef unsigned char bool, BOOL;


void Timet2ASC(time_t tTime, char* abASCTime,int nStart, int nLen);
time_t ASCTime2Timet(char* abASCTime);
time_t ASCDate2Timet(char* abASCDate);
int ASC2INT(char* abASC, int nLen);
int IsExistFile( char *abFileName);
int IsDigitASC(byte *abASCInput, byte bLengthSrc);
int GetINTFromASC (char * abSrcASC, byte bLengthSrc);
int IsValidASCDtime ( char *abASCDtime);
byte MakeBCC( char *abData, dword dwDataLengh );
void WORD2BIG( word wSrcWORD, byte *abDesBIG );
void WORD2LITTLE( word wSrcWORD, byte *abDesLITTLE );
void DWORD2BIG( dword dwSrcDWORD, byte  *abDesBIG );
void DWORD2LITTLE( dword dwSrcDWORD, byte * abDesLITTLE );



#endif /* UTIL_H_ */
