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
*  PROGRAM ID :       file_mgt.c                                               *
*                                                                              *
*  DESCRIPTION:       이 프로그램은 파일을 관리하기 위한 파일번호와 파일명을 제공 *
*						하거나 파일의 Lock과 Unlock을 제어한다.                 *
*                                                                              *
*  ENTRY POINT: 	short GetFileNo( char *pchFileName );                      *
*					short GetDownFileIndex( char* pchCmd );                    *
*					void GetRecvFileName( USER_PKT_MSG*      pstRecvUsrPktMsg, *
*					                      PKT_HEADER_INFO*   pstPktHeaderInfo, *
*					                      char*  			 achRecvFileName );*
*					int LockFile( int fd );									   *
*					int UnlockFile( int fd ); 								   *
*																			   *
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

/*******************************************************************************
*  Inclusion of System Header Files                                            *
*******************************************************************************/
#include "file_mgt.h"

/*******************************************************************************
*  Declaration of variables ( FILE NAME )                                      *
*******************************************************************************/
int 	nFileCnt	= 102;

char* achFileCode[] =
    {   RECONCILE_FILE ,                    // 0x01
        RECONCILE_BACKUP_FILE ,             // 0x02
        RECONCILE_TMP_FILE ,                // 0x03
        DOWN_FILE ,                         // 0x04
        DOWN_FILE_BACKUP_FILE ,             // 0x05
        RELAY_DOWN_INFO_FILE ,              // 0x06
        PG_LOADER_VER_FILE ,                // 0x07
        INSTALL_INFO_FILE ,                 // 0x08
        SETUP_FILE ,                        // 0x09
        SETUP_BACKUP_FILE ,                 // 0x0a
        TC_LEAP_FILE ,                      // 0x0b
        VEHICLE_PARM_FILE ,                 // 0x0c
        NEXT_VEHICLE_PARM_FILE ,            // 0x0e
        TMP_VEHICLE_PARM_FILE ,             // 0x0f
        TMP_NEXT_VEHICLE_PARM_FILE ,        // 0x10
        ROUTE_PARM_FILE ,                   // 0x11
        NEXT_ROUTE_PARM_FILE ,              // 0x12
        TMP_ROUTE_PARM_FILE ,               // 0x13
        TMP_NEXT_ROUTE_PARM_FILE ,          // 0x14
        PREPAY_ISSUER_INFO_FILE ,           // 0x15
        NEXT_PREPAY_ISSUER_INFO_FILE ,      // 0x16
        TMP_PREPAY_ISSUER_INFO_FILE ,       // 0x17
        TMP_NEXT_PREPAY_ISSUER_INFO_FILE ,  // 0x18
        POSTPAY_ISSUER_INFO_FILE ,          // 0x19
        NEXT_POSTPAY_ISSUER_INFO_FILE ,     // 0x1a
        TMP_POSTPAY_ISSUER_INFO_FILE ,      // 0x1b
        TMP_NEXT_POSTPAY_ISSUER_INFO_FILE , // 0x1c
        XFER_APPLY_INFO_FILE ,              // 0x1d
        NEXT_XFER_APPLY_INFO_FILE ,         // 0x1e
        TMP_XFER_APPLY_INFO_FILE ,          // 0x1f
        TMP_NEXT_XFER_APPLY_INFO_FILE ,     // 0x20
        DIS_EXTRA_INFO_FILE ,               // 0x21
        NEXT_DIS_EXTRA_INFO_FILE ,          // 0x22
        TMP_DIS_EXTRA_INFO_FILE ,           // 0x23
        TMP_NEXT_DIS_EXTRA_INFO_FILE ,      // 0x24
        HOLIDAY_INFO_FILE ,                 // 0x25
        NEXT_HOLIDAY_INFO_FILE ,            // 0x26
        TMP_HOLIDAY_INFO_FILE ,             // 0x27
        TMP_NEXT_HOLIDAY_INFO_FILE ,        // 0x28
        NEW_FARE_INFO_FILE ,                // 0x29
        NEXT_NEW_FARE_INFO_FILE ,           // 0x2a
        TMP_NEW_FARE_INFO_FILE ,            // 0x2b
        TMP_NEXT_NEW_FARE_INFO_FILE ,       // 0x2c
        BUS_STATION_INFO_FILE ,             // 0x2d
        NEXT_BUS_STATION_INFO_FILE ,        // 0x2e
        TMP_BUS_STATION_INFO_FILE ,         // 0x2f
        TMP_NEXT_BUS_STATION_INFO_FILE ,    // 0x30
        EPURSE_ISSUER_REGIST_INFO_FILE ,    // 0x31
        NEXT_EPURSE_ISSUER_REGIST_INFO_FILE ,       // 0x32
        TMP_EPURSE_ISSUER_REGIST_INFO_FILE ,        // 0x33
        TMP_NEXT_EPURSE_ISSUER_REGIST_INFO_FILE ,   // 0x34
        PSAM_KEYSET_INFO_FILE ,             // 0x35
        NEXT_PSAM_KEYSET_INFO_FILE ,        // 0x36
        TMP_PSAM_KEYSET_INFO_FILE ,         // 0x37
        TMP_NEXT_PSAM_KEYSET_INFO_FILE ,    // 0x38
        AUTO_CHARGE_SAM_KEYSET_INFO_FILE ,  // 0x39
        NEXT_AUTO_CHARGE_SAM_KEYSET_INFO_FILE ,     // 0x3a
        TMP_AUTO_CHARGE_SAM_KEYSET_INFO_FILE ,      // 0x3b
        TMP_NEXT_AUTO_CHARGE_SAM_KEYSET_INFO_FILE , // 0x3c
        AUTO_CHARGE_PARM_INFO_FILE ,        // 0x3d
        NEXT_AUTO_CHARGE_PARM_INFO_FILE ,   // 0x3e
        TMP_AUTO_CHARGE_PARM_INFO_FILE ,    // 0x3f
        TMP_NEXT_AUTO_CHARGE_PARM_INFO_FILE ,   // 0x40
        KPD_IMAGE_FILE ,                    // 0x41
        NEXT_KPD_IMAGE_FILE ,               // 0x42
        TMP_KPD_IMAGE_FILE ,                // 0x43
        TMP_NEXT_KPD_IMAGE_FILE ,           // 0x44
        BUS_MAIN_IMAGE_FILE ,               // 0x45
        NEXT_BUS_MAIN_IMAGE_FILE ,          // 0x46
        TMP_BUS_MAIN_IMAGE_FILE ,           // 0x47
        TMP_NEXT_BUS_MAIN_IMAGE_FILE ,      // 0x48
        BUS_SUB_IMAGE_FILE ,                // 0x49
        NEXT_BUS_SUB_IMAGE_FILE ,           // 0x4a
        TMP_BUS_SUB_IMAGE_FILE ,            // 0x4b
        TMP_NEXT_BUS_SUB_IMAGE_FILE ,       // 0x4c
        VOICE0_FILE ,                       // 0x4d
        NEXT_VOICE0_FILE ,                  // 0x4e
        TMP_VOICE0_FILE ,                   // 0x4f
        TMP_NEXT_VOICE0_FILE ,              // 0x50
        VER_INFO_FILE ,                     // 0x51
        UPLOAD_VER_INFO_FILE ,              // 0x52
        COMMDCS_SUCCDATE_FILE ,          // 0x53
        MERGE_FLAG_FILE ,                   // 0x54
        MASTER_BL_FILE ,                    // 0x55
        DOWN_ING_MASTER_BL_FILE ,           // 0x56
        UPDATE_BL_FILE ,                    // 0x57
        DOWN_ING_UPDATE_BL_FILE ,           // 0x58
        HOT_BL_FILE ,                       // 0x59
        DOWN_ING_HOT_BL_FILE ,              // 0x5a
        MASTER_PREPAY_PL_FILE ,             // 0x5b
        DOWN_ING_MASTER_PREPAY_PL_FILE ,    // 0x5c
        MASTER_POSTPAY_PL_FILE ,            // 0x5d
        DOWN_ING_MASTER_POSTPAY_PL_FILE ,   // 0x5e
        UPDATE_PL_FILE ,                    // 0x5f
        DOWN_ING_UPDATE_PL_FILE ,           // 0x60
        HOT_PL_FILE ,                       // 0x61
        DOWN_ING_HOT_PL_FILE ,              // 0x62
        MASTER_AI_FILE ,                    // 0x63
        DOWN_ING_MASTER_AI_FILE ,           // 0x64
        UPDATE_AI_FILE ,                    // 0x65
        DOWN_ING_UPDATE_AI_FILE ,           // 0x66
        HOT_AI_FILE ,                       // 0x67
        DOWN_ING_HOT_AI_FILE,               // 0x68
        UPDATE_BL_BEFORE_VER_FILE,          // 0x69
        UPDATE_PL_BEFORE_VER_FILE,          // 0x70
        UPDATE_AI_BEFORE_VER_FILE,          // 0x71
        ERR_LOG_MODE_FLAG_FILE              // 0x72
    };

/*******************************************************************************
*  Declaration of Extern variables                                             *
*******************************************************************************/
char* achDCSCommFile[][2] =
{
    {"B023A",  "dr_pro.dat"},   //  0 terminal Application 1 : driver operator
    {"B023B",  "en_pro.dat"},   //  1 terminal Application 2 : main  terminal
    {"B023C",  "ex_pro.dat"},   //  2 terminal Application 3 : sub  terminal
    {"B023D",  "ap4.dat"},      //  3 terminal Application 4 :
    {"B023E",  "ap5.dat"},      //  4 terminal Application 5 :
    {"B023F",  "ap6.dat"},      //  5 terminal Application 6 :
    {"B023G",  "ap7.dat"},      //  6 terminal Application 7 :
    {"B023H",  "ap8.dat"},      //  7 terminal Application 8 :
    {"B023I",  "ap9.dat"},      //  8 terminal Application 9 :
    {"B023J",  "ap10.dat"},     //  9 terminal Application 10 :
    {"B024A",  "v0.dat"},       // 10 terminal voice data 1 :
    {"B024B",  "v1.dat"},       // 11 terminal voice data 2 :
    {"B024C",  "v2.dat"},       // 12 terminal voice data 3 :
    {"B024D",  "v3.dat"},       // 13 terminal voice data 4 :
    {"B024E",  "v4.dat"},       // 14 terminal voice data 5 :
    {"B024F",  "v5.dat"},       // 15 terminal voice data 6 :
    {"B024G",  "v6.dat"},       // 16 terminal voice data 7 :
    {"B024H",  "v7.dat"},       // 17 terminal voice data 8 :
    {"B024I",  "v8.dat"},       // 18 terminal voice data 9 :
    {"B024J",  "v9.dat"},       // 19 terminal voice data 10 :
    {"B0570",  "op_par.dat"},   // 20 run vehicle parameter
    {"B0590",  "li_par.dat"},   // 21 bus  terminal route parameter
    {"B0070",  "n_far.dat"},    // 22 new fare data
    {"B0100",  "os_far.dat"},   // 23 old single fare data
    {"B0110",  "oa_far.dat"},   // 24 old airport fare data
    {"B0120",  "ob_far.dat"},   // 25 old  range fare data
    {"B0620",  "st_inf.dat"},   // 26 station data
    {"B0670",  "ap_inf.dat"},   // 27 mifare prepay card issuer data
    {"B0690",  "dp_inf.dat"},   // 28 postpay issuer data
    {"B0640",  "de_inf.dat"},   // 29 discount /extra data
    {"B0630",  "ho_inf.dat"},   // 30 holiday data
    {"B0950",  "tc_inf.dat"},   // 31 transfer condition data
    {"B1520",  "idcenter.dat"}, // 32 electronic purse Issuer registration data
    {"B1530",  "keyset.dat"},   // 33 paymentSAM Key Set data
    {"B1540",  "alsam_key.dat"},    // 34 auto-charge SAM Key Set data
    {"B1550",  "alsam_para.dat"},   // 35 auto-charge Parameter data
    {"B1700",  "tr_inf.dat"},   // 36 transfer apply  data retrieve
    {"B1840",  "cd_inf.dat"},   // 37 issuer valid period check
    {"B0480",  "fi_bl.dat"},    // 38 old fixed B/L data
    {"B0500",  "ch_bl.dat"},    // 39 old update B/L data
    {"B0520",  "ho_bl.dat"},    // 40 old HOT B/L data
    {"B0960",  "fa_pl.dat"},    // 41 old fixed P/L data -  prepay
    {"B0490",  "fd_pl.dat"},    // 42 old fixed P/L data -  postpay
    {"B0510",  "ch_pl.dat"},    // 43 old update P/L data
    {"B0530",  "ho_pl.dat"},    // 44 old HOT P/L data
    {"B0540",  "fi_ai.dat"},    // 45 fixed A/I data-new fixed prepay
    {"B0550",  "ch_ai.dat"},    // 46 update A/I data-new update prepay
    {"B0560",  "ho_ai.dat"},    // 47 HOT A/I data- new Hot prepay
    {NULL,     NULL}
};


/*******************************************************************************
*  Declaration of function                                                     *
*******************************************************************************/
static char* GetFileName( char* pchCmd );

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      GetFileNo                                                *
*                                                                              *
*  DESCRIPTION :      This program reads file number.                          *
*                                                                              *
*  INPUT PARAMETERS:  char *pchFileName                                        *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       file No - achFileCode                                  *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short GetFileNo( char *pchFileName )
{
    short   sReturnVal = SUCCESS;
    int     i;
    short   sAdd = 0x01;

    for ( i = 0 ; i < nFileCnt ; i++ )
    {

        if ( memcmp( achFileCode[i], pchFileName, sizeof( pchFileName ) ) == 0 )
        {
            sReturnVal = sAdd;
            break;
        }
        else
        {
            sAdd = sAdd + 0x01;
        }
    }

    return sReturnVal;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      GetFileName                                              *
*                                                                              *
*  DESCRIPTION :      다운로드 되는 파일의 목록으로 부터 해당 cmd에 대한 파일명을 *
*                       구한다.                                                *
*                                                                              *
*  INPUT PARAMETERS:  char* pchCmd                                             *
*                                                                              *
*  RETURN/EXIT VALUE:     achDCSCommFile[nLoop][1]                             *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static char* GetFileName( char* pchCmd )
{
    int nLoop = 0;

    while ( achDCSCommFile[nLoop][0] != NULL )
    {
        if ( memcmp( achDCSCommFile[nLoop][0], pchCmd, COMMAND_LENGTH ) == 0 )
        {
            break;
        }

        nLoop++;
    }

    return achDCSCommFile[nLoop][1];
}

byte GetDCSCommFileNoByFileName( byte *abFileName )
{
	byte i = 0;

	while ( achDCSCommFile[i][1] != NULL )
	{
		if ( memcmp( achDCSCommFile[i][1], abFileName,
				sizeof( abFileName ) ) == 0 )
		{
			break;
		}

		i++;
	}

	return i;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      GetFileIndex                                             *
*                                                                              *
*  DESCRIPTION :      집계로부터 다운로드 받는 파일의 목록에서의 인덱스를 리턴    *
*                                                                              *
*  INPUT PARAMETERS:  char* pchCmd                                             *
*                                                                              *
*  RETURN/EXIT VALUE:     sLoop                                                *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS : achDCSCommFile : 다운로드 파일 목록                                *
*                                                                              *
*******************************************************************************/
short GetDownFileIndex( char* pchCmd )
{
    short sLoop = 0;

    while ( achDCSCommFile[sLoop][0] != NULL )
    {
        if ( memcmp( achDCSCommFile[sLoop][0], pchCmd, COMMAND_LENGTH ) == 0 )
        {
            break;
        }

        sLoop++;
    }

    return sLoop;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      GetRecvFileName                                          *
*                                                                              *
*  DESCRIPTION :      수신되는 파일명 얻기                                      *
*                                                                              *
*  INPUT PARAMETERS:    USER_PKT_MSG*       pstRecvUsrPktMsg,                  *
*                       PKT_HEADER_INFO*    pstPktHeaderInfo,                  *
*                       char*   achRecvFileName                                *
*                                                                              *
*  RETURN/EXIT VALUE:   void                                                   *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
void GetRecvFileName( USER_PKT_MSG*      pstRecvUsrPktMsg,
                      PKT_HEADER_INFO*   pstPktHeaderInfo,
                      char*  			 achRecvFileName )
{
    char*   pTmpFileName;                   // c_, n_가 제외된 파일명

    pTmpFileName = GetFileName( pstRecvUsrPktMsg->achConnCmd );

    if ( pstPktHeaderInfo->bNewVerYN == GetASCNoFromDEC( CURR ) )     // Current
    {
        sprintf( achRecvFileName, "tmp_c_%s", pTmpFileName );
    }
    else                                                        // Next
    {
        sprintf( achRecvFileName, "tmp_n_%s", pTmpFileName );
    }

}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      LockFile                                                 *
*                                                                              *
*  DESCRIPTION :      checking the file, if it is locked then unlock the file, *
*                     otherwise wait until it is                               *
*                                                                              *
*  INPUT PARAMETERS:  int fd                                                   *
*                                                                              *
*  RETURN/EXIT VALUE:     int - SUCCESS/FAIL                                   *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
int LockFile( int fd )
{
    struct flock lock;

    lock.l_type 	= F_WRLCK;
    lock.l_start 	= 0;
    lock.l_whence 	= SEEK_SET;
    lock.l_len 		= 0;

    return fcntl( fd, F_SETLKW, &lock );
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      UnlockFile                                               *
*                                                                              *
*  DESCRIPTION :      This program unlock the file.                            *
*                                                                              *
*  INPUT PARAMETERS:  int fd                                                   *
*                                                                              *
*  RETURN/EXIT VALUE:     int - SUCCESS/FAIL                                   *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
int UnlockFile( int fd )
{
    struct flock lock;

    lock.l_type 	= F_UNLCK;
    lock.l_start 	= 0;
    lock.l_whence 	= SEEK_SET;
    lock.l_len 		= 0;

    return fcntl( fd, F_SETLKW, &lock );
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      WriteCommSuccTime                                        *
*                                                                              *
*  DESCRIPTION :      This program writes communication success time.          *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: 0 - SUCCESS                                              *
*       ErrRet( ERR_FILE_OPEN | GetFileNo( COMMDCS_SUCCDATE_FILE ) )        *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short WriteCommSuccTime( void )
{
    int fdFile;

    time_t  tCurrTime;
    char achCurrTime[15] = { 0, };

    if ( ( fdFile = open ( COMMDCS_SUCCDATE_FILE, O_RDWR | O_CREAT, OPENMODE ) ) < 0 )
    {
        DebugOut( "file cannot open [%s]", COMMDCS_SUCCDATE_FILE );
        return ErrRet( ERR_FILE_OPEN | GetFileNo( COMMDCS_SUCCDATE_FILE ) );
    }

    lseek ( fdFile, 0, SEEK_SET );

    GetRTCTime( &tCurrTime );
    TimeT2ASCDtime( tCurrTime, achCurrTime );

    write( fdFile, achCurrTime, sizeof( achCurrTime ) - 1 );

    close( fdFile );

    return SUCCESS;
}



/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      GetCommSuccTimeDiff                                      *
*                                                                              *
*  DESCRIPTION :      This program gets communication success time differnce.  *
*                                                                              *
*  INPUT PARAMETERS:  char  *pchDtime                                          *
*                                                                              *
*  RETURN/EXIT VALUE:     nDateGap                                             *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/

short GetCommSuccTimeDiff( char *pchDtime )
{
    time_t  tCurrTime;
    char achDCSCommDtime[15] = { 0, };

    int fdFile;
    dword dwDateGap = 0;
    short nDateGap = 0;

    DebugOut( "GetCommSuccTimeDiff Start!!! \n" );

    if ( ( fdFile = open ( COMMDCS_SUCCDATE_FILE,  O_RDONLY, OPENMODE  ) ) < 0 )
    {
        WriteCommSuccTime();
        return SUCCESS;
    }

    lseek ( fdFile,  0,  SEEK_SET );

    GetRTCTime( &tCurrTime );
    read( fdFile, achDCSCommDtime, sizeof( achDCSCommDtime ) );
    close( fdFile );

    dwDateGap = abs( GetTimeTFromASCDtime( achDCSCommDtime ) - tCurrTime );
    nDateGap = dwDateGap/60/60/24;

    DebugOut( "date gap====>[%d] \n", nDateGap );
    memcpy( pchDtime, &achDCSCommDtime[2], 6 );

    DebugOut( "comm success time====>[%s] \n", pchDtime );
    return nDateGap;
}
