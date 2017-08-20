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
*  PROGRAM ID :       reconcile_file_mgt.c                                    *
*                                                                              *
*  DESCRIPTION:       이 프로그램은 레컨사일 파일의 CRUD의 기능을 제공 한다.      *
*                                                                              *
*  ENTRY POINT:       WriteReconcileFileList                                   *
*                     ReadReconcileFileList                                    *
*                     UpdateReconcileFileList                                  *
*                     DelReconcileFileList                                     *
*                     CopyReconcileFile                                        *
*                     LoadReconcileFileList                                    *
*                     GetReconcileCnt                                          *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: None                                                     *
*                                                                              *
*  INPUT FILES:       reconcileinfo.dat                                        *
*                                                                              *
*  OUTPUT FILES:      reconcileinfo.dat                                        *
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
#include "reconcile_file_mgt.h"


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      WriteReconcileFileList                                   *
*                                                                              *
*  DESCRIPTION :      레컨사일 파일에 집계시스템에 업로드할 파일명을 write한다.   *
*                                                                              *
*  INPUT PARAMETERS:  RECONCILE_DATA* stReconcileData - write할 reconcile구조체 *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       ERR_FILE_OPEN | GetFileNo( RECONCILE_FILE )            *
*                       ERR_FILE_DATA_NOT_FOUND | GetFileNo( RECONCILE_FILE )  *
*                       ERR_FILE_READ | GetFileNo( RECONCILE_FILE )            *
*                       ERR_FILE_WRITE | GetFileNo( RECONCILE_FILE )           *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short WriteReconcileFileList( RECONCILE_DATA* pstReconcileData )
{
    short           sReturnVal          = SUCCESS;
    int             fdReconcileFile;
    RECONCILE_DATA  stReadReconcileData;        // 읽어들일 레컨사일 구조체
    long            lOffset             = 0;    // read 시작 위치
    int             nReadByte          = 0;    // read한 byte수
    bool            boolIsWrite         = FALSE;// write 여부

    fdReconcileFile = open( RECONCILE_FILE, O_RDWR | O_CREAT, OPENMODE );

    if ( fdReconcileFile < 0 )
    {
        /*
         *  file open시 오류가 발생하면 오류코드와 함께 파일코드를 리턴
         */
        return ErrRet( ERR_FILE_OPEN | GetFileNo( RECONCILE_FILE ) );
    }

    /*
     *  reconcile file에 write하기 전에 lock을 건다.
     */
    LockFile( fdReconcileFile );

	/*
	 *	기존에 등록됐을 경우 update하도록 한다.
	 */
    while( TRUE )
    {
        /*
         *  거래내역 파일의 송신대기 파일인 경우
         */
        if ( pstReconcileData->bSendStatus == RECONCILE_SEND_WAIT )
        {
            break;
        }

        nReadByte = read( fdReconcileFile,
                          (void*)&stReadReconcileData,
                          sizeof( RECONCILE_DATA ) );

        if ( nReadByte == 0 )
        {
            sReturnVal = ERR_FILE_DATA_NOT_FOUND | GetFileNo( RECONCILE_FILE );
            break;
        }

        if ( nReadByte < 0 )
        {
            sReturnVal = ERR_FILE_READ | GetFileNo( RECONCILE_FILE );
            break;
        }

        /*
         *  write할 파일명과 read한 파일명이 같을 경우 덮어쓰기
         */
        if ( strcmp( pstReconcileData->achFileName,
                     stReadReconcileData.achFileName
                   ) == 0 )
        {
            /*
             *  read한 시작 위치 찾기
             */
            lseek ( fdReconcileFile, lOffset, SEEK_SET );

            if ( ( write( fdReconcileFile,
                          (void*)pstReconcileData,
                          sizeof( RECONCILE_DATA ) )
                        ) != sizeof( RECONCILE_DATA ) )
            {
                sReturnVal = ERR_FILE_WRITE | GetFileNo( RECONCILE_FILE );
            }

            boolIsWrite = TRUE;
            break;

        }

        /*
         *  다음 read 시작 위치 계산
         */
        lOffset += sizeof( RECONCILE_DATA );
    }

    /*
     *  덮어쓰기가 안 된 경우
     */
    if ( boolIsWrite == FALSE )
    {
        /*
         *  파일의 끝 위치 찾기
         */
        lseek ( fdReconcileFile, 0, SEEK_END );

        if ( ( write( fdReconcileFile,
                      (void*)pstReconcileData,
                      sizeof( RECONCILE_DATA ) )
                    ) != sizeof( RECONCILE_DATA ) )
        {
            sReturnVal = ERR_FILE_WRITE | GetFileNo( RECONCILE_FILE );
        }
    }

    // 파일잠김을 푼다.
    UnlockFile( fdReconcileFile );

    close( fdReconcileFile );

    return ErrRet( sReturnVal );
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      ReadReconcileFileList                                    *
*                                                                              *
*  DESCRIPTION :      해당 파일명의 레컨사일 레코드를 읽는다.                    *
*                                                                              *
*  INPUT PARAMETERS:  char* pchFileName, - read할 파일명                        *
*                     RECONCILE_DATA* pstReconcileData  - 읽어들일 구조체       *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       ERR_FILE_OPEN | GetFileNo( RECONCILE_FILE )            *
*                       ERR_FILE_DATA_NOT_FOUND | GetFileNo( RECONCILE_FILE )  *
*                       ERR_FILE_READ | GetFileNo( RECONCILE_FILE )            *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short ReadReconcileFileList( char* pchFileName,
                             RECONCILE_DATA* pstReconcileData )
{
    short           sReturnVal          = SUCCESS;
    int             fdReconcileFile;
    RECONCILE_DATA  stReadReconcileData;            // 읽어들일 레컨사일 구조체
    int             nReadByte           = 0;        // read한 byte수

    fdReconcileFile = open( RECONCILE_FILE, O_RDONLY, OPENMODE );

    if ( fdReconcileFile < 0 )
    {
        return ErrRet( ERR_FILE_OPEN | GetFileNo( RECONCILE_FILE ) );
    }

    while( TRUE )
    {
        nReadByte = read( fdReconcileFile,
                          (void*)&stReadReconcileData,
                          sizeof( RECONCILE_DATA ) );

        if ( nReadByte == 0 )
        {
            sReturnVal =  ERR_FILE_DATA_NOT_FOUND | GetFileNo( RECONCILE_FILE );
            break;
        }

        if ( nReadByte < 0 )
        {
            sReturnVal = ERR_FILE_READ | GetFileNo( RECONCILE_FILE );
            break;
        }

        /*
         *  read할 파일명과 read한 파일명이 같을 경우
         */
        if ( strcmp( pchFileName,
                     stReadReconcileData.achFileName
                   ) == 0  )
        {
            memcpy( pstReconcileData,
                    &stReadReconcileData,
                    sizeof( RECONCILE_DATA ) );
            break;
        }
    }

    close( fdReconcileFile );

    return ErrRet( sReturnVal );

}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      UpdateReconcileFileList                                  *
*                                                                              *
*  DESCRIPTION :      기존에 등록된 레컨사일 레코드를 찾아 update한다.           *
*                                                                              *
*  INPUT PARAMETERS:  char* pchFileName, - update할 파일명                     *
*                     RECONCILE_DATA* pstReconcileData  - 업데이트할 구조체     *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       ERR_FILE_OPEN | GetFileNo( RECONCILE_FILE )            *
*                       ERR_FILE_DATA_NOT_FOUND | GetFileNo( RECONCILE_FILE )  *
*                       ERR_FILE_READ | GetFileNo( RECONCILE_FILE )            *
*                       ERR_FILE_WRITE | GetFileNo( RECONCILE_FILE )           *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short UpdateReconcileFileList( char* pchFileName,
                               RECONCILE_DATA* pstReconcileData )
{
    short           sReturnVal          = SUCCESS;
    int             fdReconcileFile;
    RECONCILE_DATA  stReadReconcileData;            // 읽어들일 레컨사일 구조체
    int             nReadByte           = 0;        // read한 byte수
    long            lOffset             = 0;        // read 시작 위치

    fdReconcileFile = open( RECONCILE_FILE, O_RDWR, OPENMODE );

    if ( fdReconcileFile < 0 )
    {
        return ErrRet( ERR_FILE_OPEN | GetFileNo( RECONCILE_FILE ) );
    }

    /*
     *  reconcile file에 write하기 전에 lock을 건다.
     */
    LockFile( fdReconcileFile );

    while( TRUE )
    {
        nReadByte = read( fdReconcileFile,
                          (void*)&stReadReconcileData,
                          sizeof( RECONCILE_DATA ) );

        if ( nReadByte == 0 )
        {
            sReturnVal = ERR_FILE_DATA_NOT_FOUND | GetFileNo( RECONCILE_FILE );
            break;
        }

        if ( nReadByte < 0 )
        {
            sReturnVal = ERR_FILE_READ | GetFileNo( RECONCILE_FILE );
            break;
        }

        /*
         *  update 파일명과 read한 파일명이 같을 경우 덮어쓰기
         */
        if ( strcmp( pchFileName,
                     stReadReconcileData.achFileName
                   ) == 0 )
        {
            /*
             *  read한 시작 위치 찾기
             */
            lseek ( fdReconcileFile, lOffset, SEEK_SET );

            if ( ( write( fdReconcileFile,
                          (void*)pstReconcileData,
                          sizeof( RECONCILE_DATA ) )
                        ) != sizeof( RECONCILE_DATA ) )
            {
                sReturnVal = ERR_FILE_WRITE | GetFileNo( RECONCILE_FILE );
            }

            break;

        }

        /*
         *  다음 read 시작 위치 계산
         */
        lOffset += sizeof( RECONCILE_DATA );
    }

    // 파일잠김을 푼다.
    UnlockFile( fdReconcileFile );

    close( fdReconcileFile );

    return ErrRet( sReturnVal );

}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      DelReconcileFileList                                     *
*                                                                              *
*  DESCRIPTION :      해당 파일명의 레코드를 삭제한다.		                   *
*                                                                              *
*  INPUT PARAMETERS:  char* pchFileName, - 삭제할 파일명                        *
*                     RECONCILE_DATA* pstReconcileData  - 삭제한 데이터 구조체  *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS,                                               *
*                       ERR_FILE_OPEN | GetFileNo( RECONCILE_FILE )            *
*                       ERR_FILE_DATA_NOT_FOUND | GetFileNo( RECONCILE_FILE )  *
*                       ERR_FILE_READ | GetFileNo( RECONCILE_FILE )            *
*                       ERR_FILE_WRITE | GetFileNo( RECONCILE_BACKUP_FILE )    *
*                       ERR_FILE_REMOVE | GetFileNo( RECONCILE_FILE )          *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short DelReconcileFileList( char* pchFileName,
                            RECONCILE_DATA* pstReconcileData )
{
    short           sReturnVal          = SUCCESS;
    int             fdSourceFile;                   // reconcileinfo.dat
    int             fdDestFile;                     // reconcileinfo.dat.back

    char            chTmpReconcileFileName[40];     // reconcileinfo.dat.back
    RECONCILE_DATA  stReadSourceReconcileData;
    int             nReadByte          = 0;        // read한 byte수

    /*
     *  임시파일명 reconcileinfo.dat.back 생성
     */
    sprintf( chTmpReconcileFileName, "%s.back", RECONCILE_FILE );

    /*
     *   reconcileinfo.dat file open
     */
    fdSourceFile = open( RECONCILE_FILE, O_RDONLY, OPENMODE );

    if ( fdSourceFile < 0 )
    {
        return ErrRet( ERR_FILE_OPEN | GetFileNo( RECONCILE_FILE ) );
    }

    /*
     *  reconcileinfo.dat.back file open
     */
    fdDestFile = open( chTmpReconcileFileName, O_WRONLY | O_CREAT, OPENMODE );

    if ( fdDestFile < 0 )
    {
        close( fdSourceFile );
        return ErrRet( ERR_FILE_OPEN | GetFileNo( RECONCILE_BACKUP_FILE ) );
    }

    while( TRUE )
    {

        nReadByte = read( fdSourceFile,
                          (void*)&stReadSourceReconcileData,
                          sizeof( RECONCILE_DATA ) );

        if ( nReadByte == 0 )
        {
            sReturnVal = ERR_FILE_DATA_NOT_FOUND | GetFileNo( RECONCILE_FILE );
            break;
        }

        if ( nReadByte < 0 )
        {
            sReturnVal = ERR_FILE_READ | GetFileNo( RECONCILE_FILE );
            break;
        }

        /*
         *  삭제할 파일명이 읽어들인 파일명과 같으면
         */
        if ( strcmp( pchFileName,
                     stReadSourceReconcileData.achFileName
                    ) == 0 )
        {
            memcpy( pstReconcileData,
                    &stReadSourceReconcileData,
                    sizeof( RECONCILE_DATA ) );
            continue;
        }

        /*
         *  해당 파일명을 제외한 내용을 write
         */
        nReadByte = write( fdDestFile,
                           (void*)&stReadSourceReconcileData,
                           sizeof( RECONCILE_DATA ) );

        if ( nReadByte < sizeof( RECONCILE_DATA ) )
        {
            sReturnVal = ERR_FILE_WRITE | GetFileNo( RECONCILE_BACKUP_FILE );
            break;
        }
    }

    close( fdSourceFile );
    close( fdDestFile );

	/*
	 *	목록 삭제전 파일을 삭제한다.
	 */
    if ( ( remove( RECONCILE_FILE ) ) < 0 )
    {
        sReturnVal = ERR_FILE_REMOVE | GetFileNo( RECONCILE_FILE );
        return ErrRet( sReturnVal );
    }

	/*
	 *	목록 삭제한 파일을 원파일로 rename
	 */
    if ( ( rename( chTmpReconcileFileName, RECONCILE_FILE ) ) < 0 )
    {
        sReturnVal = ERR_FILE_RENAME | GetFileNo( RECONCILE_BACKUP_FILE );
    }


    return ErrRet( sReturnVal );

}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      CopyReconcileFile                                        *
*                                                                              *
*  DESCRIPTION :      레컨사일 목록을 pchTmpFileName로 copy한다.			       *
*                                                                              *
*  INPUT PARAMETERS:  char* pchTmpFileName                                     *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       ERR_FILE_OPEN | GetFileNo( RECONCILE_FILE )            *
*                       ERR_FILE_READ | GetFileNo( RECONCILE_FILE )            *
*                       ERR_FILE_WRITE | GetFileNo( RECONCILE_TMP_FILE )       *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS : 레컨사일 작업 전에 파일을 tmp파일로 copy해서 작업한다.              *
*                                                                              *
*******************************************************************************/
short CopyReconcileFile( char* pchTmpFileName )
{
    short           sReturnVal          = SUCCESS;
    int             fdSourceFile;                   // reconcileinfo.dat
    int             fdDestFile;                     // reconcileinfo.dat.back
    RECONCILE_DATA  stReadReconcileData;
    int             nReadByte          = 0;        // read한 byte수

    /*
     *  reconcileinfo.dat file open
     */
    fdSourceFile = open( RECONCILE_FILE, O_RDONLY, OPENMODE );

    if ( fdSourceFile < 0 )
    {
        return ErrRet( ERR_FILE_OPEN | GetFileNo( RECONCILE_FILE ) );
    }

    /*
     *  reconcileinfo.dat.tmp file open
     */
    fdDestFile = open( pchTmpFileName, O_WRONLY | O_CREAT | O_TRUNC, OPENMODE );

    if ( fdDestFile < 0 )
    {
        close( fdSourceFile );
        return ErrRet( ERR_FILE_OPEN | GetFileNo( RECONCILE_TMP_FILE ) );
    }

    while( TRUE )
    {
        nReadByte = read (  fdSourceFile,
                            (void*)&stReadReconcileData,
                            sizeof( RECONCILE_DATA ) );

        if ( nReadByte == 0 )
        {
            break;
        }

        if ( nReadByte < 0 )
        {
            sReturnVal = ERR_FILE_READ | GetFileNo( RECONCILE_FILE );
            break;
        }

        nReadByte = write( fdDestFile,
                          (void*)&stReadReconcileData,
                           sizeof( RECONCILE_DATA ) );

        if ( nReadByte < sizeof( RECONCILE_DATA ) )
        {
            sReturnVal = ERR_FILE_WRITE | GetFileNo( RECONCILE_TMP_FILE );
            break;
        }

    }

    close( fdSourceFile );
    close( fdDestFile );

    return ErrRet( sReturnVal );
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      LoadReconcileFileList                                    *
*                                                                              *
*  DESCRIPTION :      레컨사일 목록을 로드한다.				                   *
*                                                                              *
*  INPUT PARAMETERS:    RECONCILE_DATA* pstReconcileData,					   *
*						int* nReconcileCnt, - reconcile에 등록된 레코드 수      *
*                       int* pnFileCnt,     - upload할 파일                    *
*                       int* pnMsgCnt       - 기 upload 파일                   *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       ERR_FILE_OPEN | GetFileNo( RECONCILE_FILE )            *
*                       ERR_FILE_READ | GetFileNo( RECONCILE_FILE )            *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short LoadReconcileFileList( RECONCILE_DATA* pstReconcileData,
							 int* nReconcileCnt,
							 int* pnFileCnt,
							 int* pnMsgCnt )
{

    short           sReturnVal          = SUCCESS;
    int             fdReconcileFile;
    RECONCILE_DATA  stReadReconcileData;
    int             nReadByte          = 0;        // read한 byte수

    *nReconcileCnt  = 0;
    *pnFileCnt      = 0;
    *pnMsgCnt       = 0;

    fdReconcileFile = open( RECONCILE_FILE, O_RDONLY, OPENMODE );

    if ( fdReconcileFile < 0 )
    {
        return ErrRet( ERR_FILE_OPEN | GetFileNo( RECONCILE_FILE ) );
    }

    while( TRUE )
    {
        nReadByte = read( fdReconcileFile,
                          (void*)&stReadReconcileData,
                          sizeof( RECONCILE_DATA ) );

        if ( nReadByte == 0 )
        {
            break;
        }

        if ( nReadByte < 0 )
        {
            sReturnVal = ERR_FILE_READ | GetFileNo( RECONCILE_FILE );
            break;
        }

        *nReconcileCnt += 1;

        /*
         *  load reconcileinfo.dat into RECONCILE_DATA_LOAD
         */
        memcpy( &(pstReconcileData[*nReconcileCnt-1]),
                      &stReadReconcileData,
                sizeof( RECONCILE_DATA ) );

        switch ( stReadReconcileData.bSendStatus )
        {
            case RECONCILE_SEND_WAIT:       // TR wait
            case RECONCILE_RESEND_REQ:      // TR resend request
            case RECONCILE_SEND_SETUP:      // setup registration File
            case RECONCILE_SEND_ERR_LOG:    // terminal error data File
            case RECONCILE_SEND_VERSION:    // terminal Version File
            case RECONCILE_SEND_GPS:        // GPS
            case RECONCILE_SEND_GPSLOG:     // GPS LOG
            case RECONCILE_SEND_GPSLOG2:    // GPS LOG2
            case RECONCILE_SEND_TERM_LOG:   // terminal log
            case RECONCILE_SEND_STATUS_LOG: // status log
                *pnFileCnt += 1;
                break;
            case RECONCILE_SEND_COMP:   // TR send completion Recincile wait
            case RECONCILE_RESEND_COMP: // TR resend completion Reconcile wait
                *pnMsgCnt += 1;
                break;
        }
    }

    close( fdReconcileFile );

    return sReturnVal;

}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      GetReconcileCnt                                          *
*                                                                              *
*  DESCRIPTION :      레컨사일에 등록된 레코드 갯수와 거래내역의 송신대기 목록의  *
*						갯수를 count한다.				                       *
*                                                                              *
*  INPUT PARAMETERS:  int *pnRecCnt   - reconcileinfo.dat에 등록된 레코드 수    *
*                                                                              *
*  RETURN/EXIT VALUE:   nSendWaitCnt    - upload할 거래내역 수                  *
*                       ERR_FILE_OPEN | GetFileNo( RECONCILE_FILE )            *
*                       ERR_FILE_READ | GetFileNo( RECONCILE_FILE )            *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short GetReconcileCnt( int *pnRecCnt )
{
 
    int             fdReconcileFile;
    RECONCILE_DATA  stReadReconcileData;
    int             nReadByte           = 0;        // read한 byte수
    short          sSendWaitCnt        = 0;

    DebugOut( "GetReconcileCnt start \n" );

    fdReconcileFile = open( RECONCILE_FILE, O_RDONLY, OPENMODE );

    if ( fdReconcileFile < 0 )
    {
        return ErrRet( ERR_FILE_OPEN | GetFileNo( RECONCILE_FILE ) );
    }

    while( TRUE )
    {
        nReadByte = read( fdReconcileFile,
                                    &stReadReconcileData,
                          sizeof( RECONCILE_DATA ) );

        if ( nReadByte == 0 )
        {
            break;
        }

        if ( nReadByte < 0 )
        {
            sSendWaitCnt = ERR_FILE_READ | GetFileNo( RECONCILE_FILE );
            break;
        }

        *pnRecCnt = *pnRecCnt + 1;

        /*
         *  업로드 대기중인 거래내역 수
         */
        if ( stReadReconcileData.bSendStatus == RECONCILE_SEND_WAIT )
        {
            sSendWaitCnt++;
        }

    }

    close( fdReconcileFile );

    return sSendWaitCnt;

}
