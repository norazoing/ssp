/*
 * util.c
 *
 *  Created on: 2017. 4. 21.
 *      Author: LGCNS
 */

#include "./util.h"

int ASC2INT(char* abASC, int nLen)
{
	char abBuf[256]={0,};
	memcpy(abBuf, abASC, nLen);
	abBuf[nLen]='\0';
	return strtoul(abBuf,NULL,10);

}

void Timet2ASC(time_t tTime, char* abASCTime,int nStart, int nLen)
{
	struct tm timeInfo;
	char abASCTime_temp[15]={0,};

	localtime_r(&tTime,&timeInfo);
	strftime(abASCTime_temp,15,"%Y%m%d%H%M%S",&timeInfo);
	memcpy(abASCTime, &abASCTime_temp[nStart], nLen);
}

//YYYYMMDDHHMMSS ->time_t
time_t ASCTime2Timet(char* abASCTime)
{
	struct tm timeInfo;

	timeInfo.tm_year=ASC2INT(abASCTime, 4)-1900;
	timeInfo.tm_mon=ASC2INT(abASCTime+4, 2);
	timeInfo.tm_mday=ASC2INT(abASCTime+6, 2);
	timeInfo.tm_hour=ASC2INT(abASCTime+8, 2);
	timeInfo.tm_min=ASC2INT(abASCTime+10, 2);
	timeInfo.tm_sec=ASC2INT(abASCTime+12, 2);

	return mktime(&timeInfo);
}
//YYYYMMDD ->time_t
time_t ASCDate2Timet(char* abASCDate)
{
	struct tm timeInfo;

	timeInfo.tm_year=ASC2INT(abASCDate, 4)-1900;
	timeInfo.tm_mon=ASC2INT(abASCDate+4, 2);
	timeInfo.tm_mday=ASC2INT(abASCDate+6, 2);

	return mktime(&timeInfo);

}





int IsExistFile( char *abFileName)
{
   if ( access( abFileName, 0 ) == 0 )
   {
      return 1;
   }

   return 0;
}

int IsDigitASC(byte *abASCInput, byte bLengthSrc)
{
    int result = 1;
    char i = 0;

    for ( i = 0 ; i < bLengthSrc ; i++ )
    {
        if ( !isdigit(abASCInput[i]))
            result = 0;
    }

    return result;
}

int GetINTFromASC (char * abSrcASC, byte bLengthSrc)
{
    byte abBuf[256];

    memcpy( abBuf, abSrcASC, bLengthSrc );
    abBuf[bLengthSrc] = '\0';

    return atoi(abBuf);
}


int IsValidASCDtime ( char *abASCDtime)
{
    int yyyy =0;
    int mm = 0;
    int dd = 0;
    int hh = 0;
    int mn = 0;
    int ss = 0;

    if ( IsDigitASC(abASCDtime, 14) == -1)
        return -1;

    yyyy = GetINTFromASC( &abASCDtime[0], 4);
    mm = GetINTFromASC(  &abASCDtime[4], 2 );
    dd = GetINTFromASC(  &abASCDtime[6], 2 );
    hh = GetINTFromASC(  &abASCDtime[8], 2 );
    mn = GetINTFromASC(  &abASCDtime[10], 2 );
    ss = GetINTFromASC(  &abASCDtime[12], 2 );

    if ( yyyy < 2004 || yyyy > 2030 ) return 0;
    if ( mm < 1 || mm > 12 ) return 0;
    if ( dd < 1 || dd < 31 ) return 0;
    if ( hh < 0 || hh > 23 ) return 0;
    if ( ss < 0 || ss > 59 ) return 0;

    return 1;
}


byte MakeBCC( char *abData, dword dwDataLengh )
{
    unsigned long i = 0;
    unsigned char bBCC = 0;

    for ( i = 0 ; i < dwDataLengh ; i ++ )
        bBCC ^= abData[i];

    return bBCC;
}

void WORD2BIG( word wSrcWORD, byte *abDesBIG )
{
    byte* p;
    p = (byte*)&wSrcWORD;
    abDesBIG[0] = *(p+1);
    abDesBIG[1] = *p;
}

void WORD2LITTLE( word wSrcWORD, byte *abDesLITTLE )
{
   byte* p;
   p = (byte*)&wSrcWORD;
   abDesLITTLE[0] = *p;
   abDesLITTLE[1] = *(p+1);
}

void DWORD2BIG( dword dwSrcDWORD, byte  *abDesBIG )
{
    byte *p;
    p = (byte*)&dwSrcDWORD;
    abDesBIG[0] = *(p + 3);
    abDesBIG[1] = *(p + 2);
    abDesBIG[2] = *(p + 1);
    abDesBIG[3] = *p;
}

void DWORD2LITTLE( dword dwSrcDWORD, byte * abDesLITTLE )
{
    byte *p;
    p = (byte*)&dwSrcDWORD;
    abDesLITTLE[0] = *p;
    abDesLITTLE[1] = *(p + 1);
    abDesLITTLE[2] = *(p + 2);
    abDesLITTLE[3] = *(p + 3);
}

inline dword GetDWORDFromBIG( byte *abSrcBIG)
{
    return abSrcBIG[0] * 256 * 256 * 256 +
           abSrcBIG[1] * 256 * 256 +
           abSrcBIG[2] * 256 +
           abSrcBIG[3];

}

inline dword GetDWORDFromLITTLE( byte *abSrcLITTLE)
{
    return abSrcLITTLE[0] +
           abSrcLITTLE[1] * 256 +
           abSrcLITTLE[2] * 256 * 256 +
           abSrcLITTLE[3] * 256 * 256 * 256 ;
}

inline dword GetWORDFromBIG( byte *abSrcBIG)
{
    return abSrcBIG[0] * 256 +
           abSrcBIG[1];
}

inline dword GetWORDFromLITTLE( byte *abSrcLITTLE)
{
    return abSrcLITTLE[0] +
           abSrcLITTLE[1] * 256 ;
}


