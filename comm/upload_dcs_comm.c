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
*  PROGRAM ID :       upload_dcs_comm.c	                                       *
*                                                                              *
*  DESCRIPTION:       집계시스템으로부터 파일 업로드를 위한 함수들을 제공한다.    *
*                                                                              *
*  ENTRY POINT:     short DownVehicleParmFile( void );						   *
*					short DownFromDCS( void );								   *
*                                                                              *
*  INPUT FILES:     reconcileinfo.dat                                          *
*					simxinfo.dat											   *
*					reconcileinfo.dat에 등록된 파일							   *
*                                                                              *
*  OUTPUT FILES:    reconcileinfo.dat                                          *
*					reconcileinfo.dat에 등록된 파일							   *
*                                                                              *
*  SPECIAL LOGIC:     None                                                     *
*                                                                              *
********************************************************************************
*                         MODIFICATION LOG                                     *
*                                                                              *
*    DATE                SE NAME                      DESCRIPTION              *
* ---------- ---------------------------- ------------------------------------ *
* 2006/03/27 F/W Dev Team Mi Hyun Noh  Initial Release                         *
*                                                                              *
*******************************************************************************/
/*******************************************************************************
*  Inclusion of System Header Files                                            *
*******************************************************************************/
#include "../system/bus_type.h"
#include "../system/device_interface.h"
#include "dcs_comm.h"
#include "../proc/write_trans_data.h"
#include "../proc/file_mgt.h"
#include "../proc/reconcile_file_mgt.h"

/*******************************************************************************
*  Declaration of variables ( connect command )                                *
*******************************************************************************/
#define SEND_TR_FILE_CMD                "B001H"     // 거래내역 파일 송신cmd
#define SEND_EVENT_FILE_CMD             "B041H"     // 에러내역 파일 송신cmd
#define SEND_SETUP_FILE_CMD             "B071H"     // 설치정보 파일 송신cmd
#define SEND_VER_FILE_CMD               "B039H"     // 버전정보 파일 송신cmd
#define SEND_GPS_FILE_CMD               "B173H"     // GPS 파일 송신 cmd
#define SEND_GPS_FILE_LOG_CMD           "B174H"     // GPS 로그파일 송신 cmd
#define SEND_GPS_FILE_LOG2_CMD          "B186H"     // GPS 로그2파일 송신cmd
#define SEND_TERM_LOG_CMD               "B990H"     // 로그 파일 송신cmd
#define SEND_STATUS_LOG                 "B991H"     // 상태 로그 파일 송신cmd

#define TR_FILE_NAME_HEADER             "B001.0."   // 거래내역파일명 헤더
#define EVENT_FILE_NAME_HEADERD         "B041.0."   // 에러내역파일명 헤더
#define SETUP_FILE_NAME_HEADER          "B071.0."   // 설치정보파일명 헤더
#define VER_FILE_NAME_HEADER            "B039.0."   // 버전정보파일명 헤더
#define GPS_FILE_NAME_HEADER            "B173.0."   // GPS 파일명 헤더
#define GPS_FILE_LOG_NAME_HEADER        "B174.0."   // GPS로그 파일명 헤더
#define GPS_FILE_LOG2_NAME_HEADER       "B186.0."   // GPS로그2 파일명 헤더
#define TERM_LOG_NAME_HEADER            "B990.0."   // 로그 파일 파일명 헤더
#define STATUS_NAME_HEADER              "B991.0."   // 상태 로그 파일명 헤더

#define PERIOD  						'.'
#define SEND_RECONCILE_FILE_CMD         "B0060"     // 레컨사일 요청
#define UPLOAD_FILE_JOB                 "02"
#define CURR_VER                    	'0'
#define INFO_TYPE_FILE              	'0'
#define FIRST_SEND                  	'C'
#define RE_SEND                     	'R'
#define EXTENSION_LENGTH            	4
#define RECONCILE_SEND_CNT          	20
#define TRANS_FILE						".trn"

/*******************************************************************************
*  Declaration of Module Functions                                             *
*******************************************************************************/
static short SendFile2DCS(  int fdSock,
                            char* pchFileName,
                            USER_PKT_MSG* pstSendUsrPktMsg,
                            USER_PKT_MSG* pstRecvUsrPktMsg ,
                            bool boolIsMoreSend,
                            char chSendStatus );

static short UpdateReconcileSendResult( char* pchFileName,
                                		char chSendStatus );

static void SetSendFilePkt(  byte* pabPeadBuff,
		                     int nReadSize,
		                     USER_PKT_MSG* pstSendUsrPktMsg,
		                     char chSendStatus );

static void SetSendFileHeaderPkt(  USER_PKT_MSG* pstSendUsrPktMsg,
		                           char* pchFileName,
		                           bool boolIsMoreSend,
		                           char chSendStatus );
		                           
static short Reconcile( int fdSock,
                        USER_PKT_MSG* pstSendUsrPktMsg,
                        USER_PKT_MSG* pstRecvUsrPktMsg );

static void SetReconcilePkt( USER_PKT_MSG* pstSendUsrPktMsg, 
							 RECONCILE_DATA*  pstReadData, 
							 int nMsgCnt );

static short SendReconcileFile( int fdSock,
                                USER_PKT_MSG* pstSendUsrPktMsg,
                                USER_PKT_MSG* pstRecvUsrPktMsg );

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      Upload2DCS                                               *
*                                                                              *
*  DESCRIPTION :      집계시스템으로 레컨사일 파일에 등록된 파일들을 업로드한다.  *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE:     SUCCESS				                               *
*						ErrRet( ERR_SESSION_OPEN )							   *
*						ErrRet( ERR_SESSION_CLOSE )							   *
*						ErrRet( ERR_DCS_AUTH )								   *
*						ErrRet( ERR_DCS_UPLOAD )							   *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short Upload2DCS( void )
{
    short   sReturnVal      = SUCCESS;
	int     fdSock;
    int     nReconcileCnt;
    int     nFileCnt;
    int     nMsgCnt;

    RECONCILE_DATA stRecData;
    
    byte abSendBuff[2048];
    byte abRecvBuff[2048];
    USER_PKT_MSG stSendUsrPktMsg;
    USER_PKT_MSG stRecvUsrPktMsg;
    RECONCILE_DATA 	stReconcileData[RECONCILE_MAX];
	
    int     i;
    int     nSentCnt        = 0;
    bool    boolIsMoreSend;
    
    /*
     * GPS LOG 파일의 사이즈가 0일 경우 레컨사일 파일에서 해당 레코드 삭제
     */
    if ( GetFileSize( GPS_LOG_FILE ) == 0 )
    {
        DelReconcileFileList( GPS_LOG_FILE, &stRecData );
    }

    /*
     * reconcile data load
     */
    sReturnVal = LoadReconcileFileList( stReconcileData, &nReconcileCnt,  &nFileCnt, &nMsgCnt );
    if ( nFileCnt == 0 && nMsgCnt == 0 )
    {
    	printf( "[Upload2DCS] 업로드할 파일이 존재하지 않음\n" );
        return SUCCESS;
    }

    /*
     *  세션 오픈
     */
    fdSock = OpenSession( gstCommInfo.abDCSIPAddr, SERVER_COMM_PORT );
    if ( fdSock < 0 )
    {
        LogDCS( "[Upload2DCS] OpenSession() 실패\n" );
		printf( "[Upload2DCS] OpenSession() 실패\n" );
//        CloseSession( fdSock );
        return ErrRet( ERR_SESSION_OPEN );
    }

    stSendUsrPktMsg.pbRealSendRecvData = abSendBuff;
    stRecvUsrPktMsg.pbRealSendRecvData = abRecvBuff;
    stSendUsrPktMsg.nMaxDataSize = sizeof( abSendBuff );
    stRecvUsrPktMsg.nMaxDataSize = sizeof( abRecvBuff );

    InitPkt( &stSendUsrPktMsg );
    InitPkt( &stRecvUsrPktMsg );

    /*
     *  집계 인증
     */
    if ( AuthDCS( fdSock, UPLOAD_FILE_JOB, &stSendUsrPktMsg, &stRecvUsrPktMsg
                ) != SUCCESS )
    {
        LogDCS( "[Upload2DCS] AuthDCS() 실패\n" );
		printf( "[Upload2DCS] AuthDCS() 실패\n" );
        CloseSession( fdSock );
		DisplayASCInDownFND( FND_READY_MSG );	// FND 초기화
        return ErrRet( ERR_DCS_AUTH );
    }

	/*
	 *	레컨사일에 등록된 레코드수 만큼 파일 송신
	 */
    for ( i = 0 ; i < nReconcileCnt ; i++ )
    {

        if ( stReconcileData[i].bSendStatus != RECONCILE_SEND_WAIT     &&
             stReconcileData[i].bSendStatus != RECONCILE_RESEND_REQ    &&
             stReconcileData[i].bSendStatus != RECONCILE_SEND_SETUP    &&
             stReconcileData[i].bSendStatus != RECONCILE_SEND_ERR_LOG  &&
             stReconcileData[i].bSendStatus != RECONCILE_SEND_VERSION  &&
             stReconcileData[i].bSendStatus != RECONCILE_SEND_GPS      &&
             stReconcileData[i].bSendStatus != RECONCILE_SEND_GPSLOG   &&
             stReconcileData[i].bSendStatus != RECONCILE_SEND_GPSLOG2  &&
             stReconcileData[i].bSendStatus != RECONCILE_SEND_TERM_LOG &&
             stReconcileData[i].bSendStatus != RECONCILE_SEND_STATUS_LOG )
        {
            continue;
        }

        nSentCnt++;

        if ( nSentCnt == nFileCnt )     /* EOT next send File does not exist */
        {
            boolIsMoreSend = FALSE;
        }
        else                            /* EOF next send File exists */
        {
            boolIsMoreSend = TRUE;
        }

        sReturnVal = SendFile2DCS ( fdSock,
                                    stReconcileData[i].achFileName,
                                    &stSendUsrPktMsg,
                                    &stRecvUsrPktMsg,
                                    boolIsMoreSend,
                                    stReconcileData[i].bSendStatus );

		if ( sReturnVal != SUCCESS )
		{
			LogDCS( "[Upload2DCS] SendFile2DCS() 실패\n" );
			printf( "[Upload2DCS] SendFile2DCS() 실패\n" );
		}

        if ( boolIsMoreSend == FALSE )
        {
            break;
        }

    }

	/*
	 *	업로드 결과를 레컨사일 파일에 반영
	 */
    sReturnVal = Reconcile( fdSock, &stSendUsrPktMsg, &stRecvUsrPktMsg );
    if ( sReturnVal < SUCCESS )
    {
        return ErrRet( ERR_DCS_UPLOAD );
    }

	CloseSession( fdSock );
	DisplayASCInDownFND( FND_READY_MSG );		// FND 초기화

    return sReturnVal;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SendFile2DCS                                             *
*                                                                              *
*  DESCRIPTION :      집계시스템으로 해당 파일을 송신한다.                       *
*                                                                              *
*  INPUT PARAMETERS:    int fdSock,                                            *
*                       char* pchFileName,                                     *
*                       USER_PKT_MSG* stSendUsrPktMsg,                         *
*                       USER_PKT_MSG* stRecvUsrPktMsg ,                        *
*                       bool boolIsMoreSend,                                   *
*                       char chSendStatus                                      *
*                                                                              *
*  RETURN/EXIT VALUE:   sReturnVal                                             *
*                       ErrRet( sReturnVal )                                   *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short SendFile2DCS(  int fdSock,
                            char* pchFileName,
                            USER_PKT_MSG* pstSendUsrPktMsg,
                            USER_PKT_MSG* pstRecvUsrPktMsg ,
                            bool boolIsMoreSend,
                            char chSendStatus )
{

    short   sReturnVal                  = SUCCESS;
    int     fdFile;
    byte    abReadBuff[MAX_PKT_SIZE]    = { 0, };
    int     nReadSize;

	printf( "[SendFile2DCS] 집계로 업로드 : [%s] ", pchFileName );

    /*
     *  파일 존재 여부 체크하여 없으면 로그를 남긴다.
     */
    sReturnVal = access( pchFileName, F_OK );

    if ( sReturnVal != 0 &&
         ( chSendStatus == RECONCILE_SEND_WAIT ||
           chSendStatus == RECONCILE_RESEND_REQ ) )
    {
    	printf( "e1\n" );
        ctrl_event_info_write( TRFILE_EXIST_ERROR_EVENT );
        return ErrRet( ERR_TRFILE_NOT_EXIST );
    }

    /*
     *  TR File일 경우 파일전송시간을 update한다.
     */
    if ( chSendStatus == RECONCILE_SEND_WAIT ||
         chSendStatus == RECONCILE_RESEND_REQ )
    {
        UpdateTransDCSRecvDtime( pchFileName );
    }

    fdFile = open( pchFileName, O_RDWR, OPENMODE );

    if ( fdFile < 0  )
    {
    	printf( "e2\n" );
        return ErrRet( ERR_FILE_OPEN | GetFileNo( pchFileName ) );
    }

    /*
     * 송신할 파일의 헤더를 세팅
     */
    SetSendFileHeaderPkt( pstSendUsrPktMsg,
                          pchFileName,
                          boolIsMoreSend,
                          chSendStatus );

    if ( SendUsrPkt2DCS( fdSock,
                         TIMEOUT,
                         MAX_RETRY_COUNT,
                         pstSendUsrPktMsg )
        != SUCCESS )
    {
    	printf( "e3\n" );
        sReturnVal = ERR_SOCKET_SEND;
        goto end;
    }

    if ( RecvUsrPktFromDCS( fdSock,
                            TIMEOUT,
                            MAX_RETRY_COUNT,
                            pstRecvUsrPktMsg  )
        != SUCCESS )
    {
    	printf( "e4\n" );
        sReturnVal = ERR_SOCKET_RECV;
        goto end;
    }

    /*
     * 파일 내용을 송신
     */
    while( TRUE )
    {
    	printf( "." );
		fflush( stdout );

        nReadSize = read( fdFile, abReadBuff, MAX_PKT_SIZE );

        if ( nReadSize == 0 )
        {
            break;
        }

        /*
         *  파일 내용을 세팅
         */
        SetSendFilePkt( abReadBuff, nReadSize, pstSendUsrPktMsg, chSendStatus );

        sReturnVal = SendNRecvUsrPkt2DCS( fdSock,
                                          pstSendUsrPktMsg,
                                          pstRecvUsrPktMsg );

        if ( sReturnVal != SUCCESS )
        {
        	printf( "e5\n" );
            sReturnVal = ERR_SEND_FILE;
            goto end;
        }

    }

    /*
     *  EOF 송신
     */
    if ( SendEOF( fdSock, pstSendUsrPktMsg ) != SUCCESS )
    {
       	printf( "e6\n" );
        sReturnVal = ERR_SEND_FILE;
        goto end;
    }

    sReturnVal = RecvACK( fdSock, pstRecvUsrPktMsg );
	if ( sReturnVal != SUCCESS )
	{
       	printf( "e7\n" );
	}

end :
    close( fdFile );

    if ( sReturnVal != SUCCESS )
    {
        return ErrRet( sReturnVal );
    }

    sReturnVal = UpdateReconcileSendResult( pchFileName, chSendStatus );

	printf( "/\n" );

    return SUCCESS;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      UpdateReconcileSendResult                                *
*                                                                              *
*  DESCRIPTION :      송신한 결과를 레컨사일파일에 업데이트 한다.                *
*                                                                              *
*  INPUT PARAMETERS:    char* pchFileName,									   *
*                  		char chSendStatus                                      *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*						ErrRet( ERR_UPDATE_SEND_RESULT )					   *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short UpdateReconcileSendResult( char* pchFileName,
                                		char chSendStatus )
{
    short   			sRetVal 		= SUCCESS;
    RECONCILE_DATA      stRecFileData;

	/*
	 *	레컨사일 파일 read
	 */
    sRetVal = ReadReconcileFileList( pchFileName, &stRecFileData );

    if ( sRetVal != SUCCESS )
    {
        return ErrRet( ERR_UPDATE_SEND_RESULT );
    }

	/*
	 *	거래내역 파일의 경우 업로드 결과만 업데이트하고
	 *  나머지 파일들은 레컨사일 목록에서 삭제한다.
	 */
    switch ( chSendStatus )
    {
        case RECONCILE_SEND_WAIT:                   // 0
        case RECONCILE_RESEND_REQ:                  // 3
            if ( stRecFileData.bSendStatus  == RECONCILE_SEND_WAIT )
            {
                stRecFileData.bSendStatus  = RECONCILE_SEND_COMP;
            }
            else if ( stRecFileData.bSendStatus  == RECONCILE_RESEND_REQ )
            {
                stRecFileData.bSendStatus  = RECONCILE_RESEND_COMP;
            }

            stRecFileData.nSendSeqNo++;

            sRetVal = UpdateReconcileFileList( pchFileName, &stRecFileData );
            break;

        case RECONCILE_SEND_GPS:
        case RECONCILE_SEND_VERSION:
        case RECONCILE_SEND_ERR_LOG:
        case RECONCILE_SEND_GPSLOG:
        case RECONCILE_SEND_GPSLOG2:
        case RECONCILE_SEND_TERM_LOG:
        case RECONCILE_SEND_STATUS_LOG:
            sRetVal = DelReconcileFileList( pchFileName, &stRecFileData );
            unlink ( pchFileName );
            break;

        case RECONCILE_SEND_SETUP:
            sRetVal = DelReconcileFileList( pchFileName, &stRecFileData );
            break;
    }

    if ( sRetVal != SUCCESS )
    {
        return ErrRet( ERR_UPDATE_SEND_RESULT );
    }
    return ErrRet( sRetVal );
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SetSendFilePkt                                           *
*                                                                              *
*  DESCRIPTION :      송신할 파일의 패킷을 세팅한다.                             *
*                                                                              *
*  INPUT PARAMETERS:  byte* pabPeadBuff,									   *
*                     int nReadSize,									   	   *
*                     USER_PKT_MSG* pstSendUsrPktMsg,						   *
*                     char chSendStatus                                        *
*                                                                              *
*  RETURN/EXIT VALUE:     void                                                 *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static void SetSendFilePkt( byte* pabPeadBuff,
                     int nReadSize,
                     USER_PKT_MSG* pstSendUsrPktMsg,
                     char chSendStatus )
{

    pstSendUsrPktMsg->chEncryptionYN = NOT_ENCRYPTION;
    pstSendUsrPktMsg->lDataSize = nReadSize;
    memcpy( pstSendUsrPktMsg->pbRealSendRecvData,
            pabPeadBuff,
            nReadSize );

    switch ( chSendStatus  )
    {
        case RECONCILE_SEND_WAIT:                   // 0
        case RECONCILE_RESEND_REQ:                  // 3
            memcpy( pstSendUsrPktMsg->achConnCmd,
                    SEND_TR_FILE_CMD,
                    COMMAND_LENGTH );
            break;
        case RECONCILE_SEND_ERR_LOG:                // 8
            memcpy( pstSendUsrPktMsg->achConnCmd,
                    SEND_EVENT_FILE_CMD,
                    COMMAND_LENGTH );
            break;
        case RECONCILE_SEND_SETUP:                  // 7
            memcpy( pstSendUsrPktMsg->achConnCmd,
                    SEND_SETUP_FILE_CMD,
                    COMMAND_LENGTH );
            break;
        case RECONCILE_SEND_VERSION:                // 9
            memcpy( pstSendUsrPktMsg->achConnCmd,
                    SEND_VER_FILE_CMD,
                    COMMAND_LENGTH );
            break;
        case RECONCILE_SEND_GPS:                    // a
            memcpy( pstSendUsrPktMsg->achConnCmd,
                    SEND_GPS_FILE_CMD,
                    COMMAND_LENGTH );
            break;
        case RECONCILE_SEND_GPSLOG:                 // b
            memcpy( pstSendUsrPktMsg->achConnCmd,
                    SEND_GPS_FILE_LOG_CMD,
                    COMMAND_LENGTH );
            break;
        case RECONCILE_SEND_GPSLOG2:                // c
            memcpy( pstSendUsrPktMsg->achConnCmd,
                    SEND_GPS_FILE_LOG2_CMD,
                    COMMAND_LENGTH );
        case RECONCILE_SEND_TERM_LOG:               // d
            memcpy( pstSendUsrPktMsg->achConnCmd,
                    SEND_TERM_LOG_CMD,
                    COMMAND_LENGTH );
        case RECONCILE_SEND_STATUS_LOG:             // e
            memcpy( pstSendUsrPktMsg->achConnCmd,
                    SEND_STATUS_LOG,
                    COMMAND_LENGTH );
            break;
    }
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SetSendFileHeaderPkt                                     *
*                                                                              *
*  DESCRIPTION :      송신할 파일의 헤더 패킷을 세팅한다.                        *
*                                                                              *
*  INPUT PARAMETERS:    USER_PKT_MSG* stSendUsrPktMsg,                         *
*                       char* pchFileName,                                     *
*                       bool boolIsMoreSend,                                   *
*                       char chSendStatus                                      *
*                                                                              *
*  RETURN/EXIT VALUE:     void                                                 *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static void SetSendFileHeaderPkt(  USER_PKT_MSG* pstSendUsrPktMsg,
                            char* pchFileName,
                            bool boolIsMoreSend,
                            char chSendStatus )
{

    long    lFileSize;
    time_t  tCurrDtime;
    char    aBCDCurrDtime[7]    = { 0, };
    long    lTmpPktCnt;
    char    chFileSize[5]       = { 0, };

    PKT_HEADER_INFO     stPktHeaderInfo;

    /*
     *  파일 사이즈 세팅
     */
    lFileSize = GetFileSize( pchFileName );

    pstSendUsrPktMsg->chEncryptionYN = NOT_ENCRYPTION;
    pstSendUsrPktMsg->lDataSize = sizeof( PKT_HEADER_INFO );

    /*
     *  패킷 헤더 세팅
     */
    memset( &stPktHeaderInfo, 0, sizeof( PKT_HEADER_INFO ) );

    stPktHeaderInfo.bNextSendFileExistYN = GetASCNoFromDEC( boolIsMoreSend );
    memcpy( stPktHeaderInfo.abVehicleID,
            gstVehicleParm.abVehicleID,
            sizeof( gstVehicleParm.abVehicleID ) ); // vehicle ID
    stPktHeaderInfo.bDataType = INFO_TYPE_FILE;     // Data Tpye - file
    stPktHeaderInfo.bNewVerYN = CURR_VER;           // File Version Type

    /*
     * command and fileNameHeader 세팅
     */
    switch ( chSendStatus )
    {
        case RECONCILE_SEND_WAIT:                   // 0
        case RECONCILE_RESEND_REQ:                  // 3
            memcpy( pstSendUsrPktMsg->achConnCmd,
                    SEND_TR_FILE_CMD,
                    COMMAND_LENGTH );
            memcpy( &stPktHeaderInfo.stDCSFileName.abDCSFileNameHeader,
                    TR_FILE_NAME_HEADER,
                    sizeof( stPktHeaderInfo.stDCSFileName.abDCSFileNameHeader )
                  );
            break;
        case RECONCILE_SEND_ERR_LOG:
            memcpy( pstSendUsrPktMsg->achConnCmd,
                    SEND_EVENT_FILE_CMD,
                    COMMAND_LENGTH );               // error  information file
            memcpy( &stPktHeaderInfo.stDCSFileName.abDCSFileNameHeader,
                    EVENT_FILE_NAME_HEADERD,
                    sizeof( stPktHeaderInfo.stDCSFileName.abDCSFileNameHeader )
                  );
            break;
        case RECONCILE_SEND_SETUP:                  // setup  information file
            memcpy( pstSendUsrPktMsg->achConnCmd,
                    SEND_SETUP_FILE_CMD,
                    COMMAND_LENGTH );
            memcpy( &stPktHeaderInfo.stDCSFileName.abDCSFileNameHeader,
                    SETUP_FILE_NAME_HEADER,
                    sizeof( stPktHeaderInfo.stDCSFileName.abDCSFileNameHeader )
                  );
            break;
        case RECONCILE_SEND_VERSION:               // version file
            memcpy( pstSendUsrPktMsg->achConnCmd,
                    SEND_VER_FILE_CMD,
                    COMMAND_LENGTH );
            memcpy( &stPktHeaderInfo.stDCSFileName.abDCSFileNameHeader,
                    VER_FILE_NAME_HEADER,
                    sizeof( stPktHeaderInfo.stDCSFileName.abDCSFileNameHeader )
                  );
            break;
        case RECONCILE_SEND_GPS:                    // GPS
            memcpy( pstSendUsrPktMsg->achConnCmd,
                    SEND_GPS_FILE_CMD,
                    COMMAND_LENGTH );
            memcpy( &stPktHeaderInfo.stDCSFileName.abDCSFileNameHeader,
                    GPS_FILE_NAME_HEADER,
                    sizeof( stPktHeaderInfo.stDCSFileName.abDCSFileNameHeader )
                  );
            break;
        case RECONCILE_SEND_GPSLOG:                 // GPS Log
            memcpy( pstSendUsrPktMsg->achConnCmd,
                    SEND_GPS_FILE_LOG_CMD,
                    COMMAND_LENGTH );
            memcpy( &stPktHeaderInfo.stDCSFileName.abDCSFileNameHeader,
                    GPS_FILE_LOG_NAME_HEADER,
                    sizeof( stPktHeaderInfo.stDCSFileName.abDCSFileNameHeader )
                  );
            break;
        case RECONCILE_SEND_GPSLOG2:                // GPS Log2
            memcpy( pstSendUsrPktMsg->achConnCmd,
                    SEND_GPS_FILE_LOG2_CMD,
                    COMMAND_LENGTH );
            memcpy( &stPktHeaderInfo.stDCSFileName.abDCSFileNameHeader,
                    GPS_FILE_LOG2_NAME_HEADER,
                    sizeof( stPktHeaderInfo.stDCSFileName.abDCSFileNameHeader )
                  );
            break;
        case RECONCILE_SEND_TERM_LOG:               // TERMINAL LOG
            memcpy( pstSendUsrPktMsg->achConnCmd,
                    SEND_TERM_LOG_CMD,
                    COMMAND_LENGTH );
            memcpy( &stPktHeaderInfo.stDCSFileName.abDCSFileNameHeader,
                    TERM_LOG_NAME_HEADER,
                    sizeof( stPktHeaderInfo.stDCSFileName.abDCSFileNameHeader )
                  );
            break;
        case RECONCILE_SEND_STATUS_LOG:             // TERMINAL STATUS
            memcpy( pstSendUsrPktMsg->achConnCmd,
                    SEND_STATUS_LOG,
                    COMMAND_LENGTH );
            memcpy( &stPktHeaderInfo.stDCSFileName.abDCSFileNameHeader,
                    STATUS_NAME_HEADER,
                    sizeof( stPktHeaderInfo.stDCSFileName.abDCSFileNameHeader )
                  );
            break;

    }

    /*
     *  DCS 파일명 세팅
     */
    switch ( chSendStatus )
    {
        case RECONCILE_SEND_WAIT:
        case RECONCILE_RESEND_REQ:
            memcpy( &stPktHeaderInfo.stDCSFileName.abTranspBizrID,
                    gstVehicleParm.abTranspBizrID,
                    sizeof( stPktHeaderInfo.stDCSFileName.abTranspBizrID ) );
            stPktHeaderInfo.stDCSFileName.bSeperator1 = PERIOD;
            memcpy( stPktHeaderInfo.stDCSFileName.abMainTermID,
                    gpstSharedInfo->abMainTermID,
                    sizeof( stPktHeaderInfo.stDCSFileName.abMainTermID ) );
            stPktHeaderInfo.stDCSFileName.bSeperator2 = PERIOD;
            memcpy( &stPktHeaderInfo.stDCSFileName.abFileName,
                    pchFileName,
                    sizeof( stPktHeaderInfo.stDCSFileName.abFileName ) );

            memset ( &stPktHeaderInfo.stDCSFileName.abDCSFileNameTail,
                     SPACE,
                     sizeof( stPktHeaderInfo.stDCSFileName.abDCSFileNameTail ));

            stPktHeaderInfo.stDCSFileName.abDCSFileNameTail[0] = PERIOD;

            if ( chSendStatus == RECONCILE_SEND_WAIT )
            {
                stPktHeaderInfo.stDCSFileName.abDCSFileNameTail[1] = FIRST_SEND;
            }
            else if ( chSendStatus == RECONCILE_RESEND_REQ )
            {
                stPktHeaderInfo.stDCSFileName.abDCSFileNameTail[1] = RE_SEND;
            }
            break;

        case RECONCILE_SEND_ERR_LOG:
        case RECONCILE_SEND_SETUP:              // setup  information file
        case RECONCILE_SEND_VERSION:            // version file
        case RECONCILE_SEND_GPS:
        case RECONCILE_SEND_GPSLOG:
        case RECONCILE_SEND_GPSLOG2:
        case RECONCILE_SEND_TERM_LOG:           // TERMINAL LOG
        case RECONCILE_SEND_STATUS_LOG:         // TERMINAL STATUS
            memcpy( &stPktHeaderInfo.stDCSFileName.abTranspBizrID,
                    gstVehicleParm.abTranspBizrID,
                    sizeof( stPktHeaderInfo.stDCSFileName.abTranspBizrID ) );
            stPktHeaderInfo.stDCSFileName.bSeperator1 = PERIOD;
            memcpy( stPktHeaderInfo.stDCSFileName.abMainTermID,
                    gpstSharedInfo->abMainTermID,
                    sizeof( stPktHeaderInfo.stDCSFileName.abMainTermID ) );
            stPktHeaderInfo.stDCSFileName.bSeperator2 = PERIOD;
            GetRTCTime( &tCurrDtime );
            TimeT2ASCDtime( tCurrDtime,
                            stPktHeaderInfo.stDCSFileName.abFileName );
            memset ( &stPktHeaderInfo.stDCSFileName.abDCSFileNameTail,
                     SPACE,
                     sizeof( stPktHeaderInfo.stDCSFileName.abDCSFileNameTail ));
            break;
    }

    TimeT2BCDDtime( tCurrDtime, aBCDCurrDtime );
    memcpy( stPktHeaderInfo.abFileVer,
            aBCDCurrDtime,
            sizeof( stPktHeaderInfo.abFileVer ) );  // file version

    DWORD2ASC( lFileSize, chFileSize );
    memcpy( stPktHeaderInfo.abFileSize,
            chFileSize,
            sizeof( stPktHeaderInfo.abFileSize ) ); // file size

    lTmpPktCnt = lFileSize / MAX_PKT_SIZE;
    if ( lFileSize % MAX_PKT_SIZE )
    {
        lTmpPktCnt++;
    }

    memcpy( stPktHeaderInfo.abTotalPktCnt,
            (char*)&lTmpPktCnt,
            sizeof( stPktHeaderInfo.abTotalPktCnt ) );  // tatal pkt cnt
    memcpy( stPktHeaderInfo.abFileCheckSum, "00", 2 );  // Reserved
    memset( stPktHeaderInfo.abFirstPktNo,
            0x00,
            sizeof( stPktHeaderInfo.abFirstPktNo ) );   // send start Packet NO
    memcpy( stPktHeaderInfo.achSendDtime,
            aBCDCurrDtime,
            sizeof( stPktHeaderInfo.achSendDtime ) );   // send time

    memcpy( pstSendUsrPktMsg->pbRealSendRecvData,
            &stPktHeaderInfo,
            sizeof( PKT_HEADER_INFO ) );
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      Reconcile                                                *
*                                                                              *
*  DESCRIPTION :      집계로 레컨사일내용을 송신하고 삭제명령을 받으면 삭제한다.  *
*                                                                              *
*  INPUT PARAMETERS:  	int fdSock, 										   *
*						USER_PKT_MSG* stSendUsrPktMsg,               		   *
*             			USER_PKT_MSG* stRecvUsrPktMsg                          *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*						ERR_FILE_READ | GetFileNo( RECONCILE_FILE )			   *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short Reconcile( int fdSock,
                        USER_PKT_MSG* pstSendUsrPktMsg,
                        USER_PKT_MSG* pstRecvUsrPktMsg )
{
    short   sReturnVal          = SUCCESS;
    int     fdReconFile;                    // 레컨사일 파일
    int     nReadByte;
    int     nMsgCnt             = 0;        // 20개씩 짤라서 보내기 위함

    RECONCILE_DATA  stReadData;                 // 레컨사일 파일 읽은 데이터
    time_t          tCurrTime;
    time_t          tCommTime;

    InitPkt( pstSendUsrPktMsg );

    /*
     *  레컨사일 파일을 reconcileinfo.dat.tmp로 copy하여 작업한다.
     */
    CopyReconcileFile( RECONCILE_TMP_FILE );

    fdReconFile = open( RECONCILE_TMP_FILE, O_RDWR, OPENMODE );
    if ( fdReconFile < 0 )
    {
        goto end;
    }

    while( TRUE )
    {
        nReadByte = read( fdReconFile,
                          (void*)&stReadData,
                          sizeof( RECONCILE_DATA ) );

        if ( nReadByte == 0 )
        {
            if ( nMsgCnt != 0 )
            {
                sReturnVal = SendReconcileFile( fdSock, pstSendUsrPktMsg,
					pstRecvUsrPktMsg );
				if ( sReturnVal != SUCCESS )
				{
					LogDCS( "[Reconcile] SendReconcileFile() 1 실패\n" );
					printf( "[Reconcile] SendReconcileFile() 1 실패\n" );
				}
            }
            break;
        }
        else if ( nReadByte < 0 )
        {
            if ( nMsgCnt != 0 )
            {
                sReturnVal = SendReconcileFile( fdSock, pstSendUsrPktMsg,
					pstRecvUsrPktMsg );
				if ( sReturnVal != SUCCESS )
				{
					LogDCS( "[Reconcile] SendReconcileFile() 2 실패\n" );
					printf( "[Reconcile] SendReconcileFile() 2 실패\n" );
				}
            }
            sReturnVal = ERR_FILE_READ | GetFileNo( RECONCILE_FILE );
            break;
        }

        /*
         * 오늘과 내일 오전 5시까지의 데이터는 송신하지 않는다.
         */
        tCommTime = stReadData.tWriteDtime -
                    ( stReadData.tWriteDtime % ( 60 * 60 * 24 ) ) +
                    ( 60 * 60 * 24 ) + ( 60 * 60 * 5 ) - (9 * 60 * 60);
		// next day 5 o'clock

        GetRTCTime( &tCurrTime );
        if ( tCurrTime < tCommTime )
        {
            continue;
        }
        else
        {
			stReadData.tWriteDtime = tCurrTime;
            UpdateReconcileFileList( stReadData.achFileName, &stReadData );
        }

        nMsgCnt++;
        SetReconcilePkt( pstSendUsrPktMsg, &stReadData, nMsgCnt );

        /*
         *  레컨사일 데이터가 20개가 되면 집계시스템으로 송신
         */
        if ( nMsgCnt == RECONCILE_SEND_CNT )     // 20
        {
            sReturnVal = SendReconcileFile( fdSock, pstSendUsrPktMsg,
				pstRecvUsrPktMsg );
			if ( sReturnVal != SUCCESS )
			{
				LogDCS( "[Reconcile] SendReconcileFile() 3 실패\n" );
				printf( "[Reconcile] SendReconcileFile() 3 실패\n" );
			}
			nMsgCnt = 0;
        }

    }

    close( fdReconFile );

    end:

    if ( SendEOS( fdSock, pstSendUsrPktMsg ) != SUCCESS )
    {
		LogDCS( "[Reconcile] SendEOS() 실패\n" );
		printf( "[Reconcile] SendEOS() 실패\n" );
    }

    return sReturnVal;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SetReconcilePkt                                          *
*                                                                              *
*  DESCRIPTION :      집계로 보내는 레컨사일 메세지를 세팅한다.                  *
*                                                                              *
*  INPUT PARAMETERS:  USER_PKT_MSG* pstSendUsrPktMsg, 						   *
*					  RECONCILE_DATA*  pstReadData, 						   *
*					  int nMsgCnt                                              *
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
static void SetReconcilePkt( USER_PKT_MSG* pstSendUsrPktMsg, 
							 RECONCILE_DATA*  pstReadData, 
							 int nMsgCnt )
{
    CONN_FILE_NAME  stConnFileName;         // 집계로 송신될 파일명
    char    achReconFileCnt[2]  = { 0, };   // 집계시스템으로 보낼 레컨사일 갯수
    byte abTempBuf[11] = {0, };

    /*
     *  reconcile message 세팅
     */
    memset( &stConnFileName, 0x00, sizeof( stConnFileName ) );

    memcpy( stConnFileName.achFileNameHeader,
            TR_FILE_NAME_HEADER,
            sizeof( stConnFileName.achFileNameHeader ) );
    memcpy( stConnFileName.abTranspBizrID,
            gstVehicleParm.abTranspBizrID,
            sizeof( stConnFileName.abTranspBizrID ) );
    stConnFileName.chClass1 = PERIOD;
    memcpy( stConnFileName.abMainTermID,
            gpstSharedInfo->abMainTermID,
            sizeof( stConnFileName.abMainTermID ) );
    stConnFileName.chClass2 = PERIOD;
    memcpy( stConnFileName.achFileName,
            pstReadData->achFileName,
            sizeof( stConnFileName.achFileName ) );
    memset( stConnFileName.achSpace,
            SPACE,
            sizeof( stConnFileName.achSpace ) );

    sprintf( abTempBuf, "%02d", nMsgCnt );
    memcpy( achReconFileCnt, abTempBuf, sizeof( achReconFileCnt ) );

    memcpy( pstSendUsrPktMsg->pbRealSendRecvData,
            achReconFileCnt,
            sizeof( achReconFileCnt ) );

    memcpy( &(pstSendUsrPktMsg->pbRealSendRecvData[sizeof( achReconFileCnt )+
							((sizeof( stConnFileName )+1)*(nMsgCnt-1))]),
            &stConnFileName,
            sizeof( stConnFileName ) );

	// 끝에 '0'을 붙이다.
	pstSendUsrPktMsg->pbRealSendRecvData[( sizeof( achReconFileCnt )+
										   sizeof( stConnFileName ))+
							((sizeof( stConnFileName )+1)*(nMsgCnt-1))] = ZERO;
										    
    pstSendUsrPktMsg->lDataSize = ( nMsgCnt * ( sizeof( stConnFileName ) + 1 ) )
                                        + ( sizeof( achReconFileCnt )  );

}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SendReconcileFile                                        *
*                                                                              *
*  DESCRIPTION :      레컨사일 패킷을 송신하고 집계시스템으로 부터 삭제요구가 있을*
*                     경우 레컨사일 파일로부터 해당 레코드를 삭제한다.           *
*                                                                              *
*  INPUT PARAMETERS:    int fdSock,                                            *
*                       USER_PKT_MSG* pstSendUsrPktMsg,                        *
*                       USER_PKT_MSG* pstRecvUsrPktMsg                         *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*						ErrRet( ERR_SEND_RECONCILE )						   *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short SendReconcileFile( int fdSock,
                                USER_PKT_MSG* pstSendUsrPktMsg,
                                USER_PKT_MSG* pstRecvUsrPktMsg )
{
    short   sRetVal                 = SUCCESS;
    int     nReconCnt               = 0;
    int     i;
    char    achReconFileName[19]    = { 0, };
    byte    bSendStatus;
    RECONCILE_DATA          	stReconcileData;
    RECONCILE_RESULT_PKT*    	pstReconResultPkt;

    /*
     *  레컨사일 파일 송신
     */
    pstSendUsrPktMsg->chEncryptionYN = NOT_ENCRYPTION;
    memcpy( pstSendUsrPktMsg->achConnCmd,
            SEND_RECONCILE_FILE_CMD,
            COMMAND_LENGTH );

    sRetVal = SendNRecvUsrPkt2DCS( fdSock, pstSendUsrPktMsg, pstRecvUsrPktMsg );

    if ( sRetVal != SUCCESS )
    {
        return ErrRet( ERR_SEND_RECONCILE );
    }

    /*
     *  집계로부터 받은 결과건수 세팅
     */
    pstReconResultPkt = (RECONCILE_RESULT_PKT*)
    						pstRecvUsrPktMsg->pbRealSendRecvData;
    nReconCnt = GetINTFromASC( pstReconResultPkt->achReconDataCnt,
                               sizeof( pstReconResultPkt->achReconDataCnt ) );

    /*
     *  집계로부터 받은 레컨사일 결과를 반영한다.
     */
    for ( i = 0 ; i < nReconCnt ; i++ )
    {
        /*
         *  파일명 세팅
         */
        memset( achReconFileName, 0x00, sizeof( achReconFileName ) );
        memcpy( achReconFileName,
                &(pstReconResultPkt->stReconDtailResultList[i].stReconFileName.achFileName ),
                VER_INFO_LENGTH );

        /*
         *  파일이 없으면 .trn을 붙여서 체크
         */
        sRetVal = access( achReconFileName, F_OK );

        if ( sRetVal != SUCCESS )   // 파일이 없을 경우
        {
            memcpy( &achReconFileName[VER_INFO_LENGTH],
                    TRANS_FILE,
                    EXTENSION_LENGTH );
            achReconFileName[18] = 0x00;
        }

        sRetVal = ReadReconcileFileList( achReconFileName,
                                         &stReconcileData );

        /*
         *  해당 레코드가 없으면 continue
         */
        if ( sRetVal != SUCCESS )
        {
            continue;
        }

        bSendStatus = pstReconResultPkt->
        				stReconDtailResultList[i].chReconResult;

        /*
         * 송신 완료 일 경우에 레컨사일파일에서 해당 레코드를 삭제하고
         * 재송신 요청일 경우 레컨사일 파일에서 해당 레코드를 갱신해준다.
         */
        if ( stReconcileData.bSendStatus == RECONCILE_SEND_COMP ||
             stReconcileData.bSendStatus == RECONCILE_RESEND_COMP ||
             stReconcileData.bSendStatus == RECONCILE_RESULT_RESEND )
        {
            switch ( bSendStatus )
            {
                case RECONCILE_RESULT_DEL:
                case RECONCILE_RESULT_ERR_DEL:
					printf("[SendReconcileFile] 파일삭제 : [%s]\n", achReconFileName);
                    DelReconcileFileList( achReconFileName,
                                          &stReconcileData );
                    unlink ( achReconFileName );
                    break;

                case RECONCILE_RESULT_RESEND:
                    stReconcileData.bSendStatus = RECONCILE_RESEND_REQ;
                    UpdateReconcileFileList( achReconFileName,
                                             &stReconcileData );
                    break;
            }
        }

    }

    return SUCCESS;

}
