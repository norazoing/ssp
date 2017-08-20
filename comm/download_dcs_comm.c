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
*  PROGRAM ID :       download_dcs_comm.c	                                   *
*                                                                              *
*  DESCRIPTION:       집계시스템으로부터 파일 다운로드를 위한 함수들을 제공한다.  *
*                                                                              *
*  ENTRY POINT:     short DownVehicleParmFile( void );						   *
*					short DownFromDCS( void );								   *
*                                                                              *
*  INPUT FILES:     c_ve_inf.dat                                               *
*                                                                              *
*  OUTPUT FILES:    achDCSCommFile에 등록된 파일들                      		   *
*					c_ve_inf.dat			- 버전정보						   *
*					downloadinfo.dat		- 다운로드된 파일 목록 정보		   *
*					downfilelink.dat		- 이어받던 파일의 정보			   *
*					connSucc.dat			- 집계통신 성공시간				   *
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
#include "../system/device_interface.h"
#include "dcs_comm.h"
#include "../proc/upload_file_mgt.h"
#include "../proc/file_mgt.h"
#include "../proc/version_file_mgt.h"
#include "../proc/download_file_mgt.h"
#include "../proc/version_mgt.h"

/*******************************************************************************
*  Declaration of variables                                                    *
*******************************************************************************/
#define RECV_OPER_PARM_FILE_CMD         "B0710"     // 파일 다운로드 요청
#define SEND_VER_INFO_CMD               "B0390"     // 다운로드전에 버전정보 송신
#define RECV_OPER_PARM_JOB              "01"		// 운영파라미터 파일 요청
#define DOWN_FILE_JOB                   "03"		// 파일다운로드 요청
#define RELAY_RECV_CLASS_NO         	37          // 이어받기 파일 구분 번호
#define MASTER_AI_FILE_INDEX_NO     	45          // 고정 AI파일의 인덱스 번호
#define RECV_EOF                         1
#define RECV_EOT                         2
#define RECV_EOS                         3
#define DOWN_MSG_TYPE		    		'1'

/*******************************************************************************
*  Declaration of Module Functions                                             *
*******************************************************************************/
static short RecvVehicleParmFile( int fdSock,
		                          USER_PKT_MSG* stSendUsrPktMsg,
		                          USER_PKT_MSG* stRecvUsrPktMsg );

static short RecvFilePkt( int fdSock, USER_PKT_MSG* pstRecvUsrPktMsg,
	bool* pboolIsMoreRecvYn );

static short SendVerInfoPkt( int fdSock,
                             bool boolIsCurrVer,
                             USER_PKT_MSG* stSendUsrPktMsg,
                             USER_PKT_MSG* stRecvUsrPktMsg );

static short RecvFileFromDCS( int fdSock,
                              USER_PKT_MSG* pstSendUsrPktMsg,
                              USER_PKT_MSG* pstRecvUsrPktMsg,
                              bool* pboolIsMoreRecvYn,
                              int* pnCurrVerFileNo,
                              int* pnNextVerFileNo );

static short WriteRecvdFile( FILE*      	fdRecvFile,
                             USER_PKT_MSG* 	pstRecvUsrPktMsg );

static void SetRelayRecvInfo( FILE_RELAY_RECV_INFO *pstFileRelayRecvInfo,
                              int nFileNo,
                              PKT_HEADER_INFO *pstPktHeaderInfo );


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      DownVehicleParmFile                                      *
*                                                                              *
*  DESCRIPTION :      운행차량 파라미터 파일을 집계로 부터 다운로드 받는다        *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*						ErrRet( ERR_SESSION_OPEN )							   *
*           			ErrRet( ERR_SESSION_CLOSE )                            *
*						ErrRet( ERR_DCS_AUTH )								   *
*						ErrRet( ERR_RECV_VEHICLE_FILE )						   *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short DownVehicleParmFile( void )
{
    short   sRetVal         = SUCCESS;
    int     fdSock;

    byte    abSendBuff[MAX_PKT_SIZE] = { 0, };
    byte    abRecvBuff[MAX_PKT_SIZE] = { 0, };

    USER_PKT_MSG stSendUsrPktMsg;
    USER_PKT_MSG stRecvUsrPktMsg;

    stSendUsrPktMsg.pbRealSendRecvData = abSendBuff;
    stRecvUsrPktMsg.pbRealSendRecvData = abRecvBuff;
    stSendUsrPktMsg.nMaxDataSize = sizeof( abSendBuff );
    stRecvUsrPktMsg.nMaxDataSize = sizeof( abRecvBuff );

    fdSock = OpenSession( gabDCSIPAddr, SERVER_COMM_PORT );    //세션 오픈
    if ( fdSock < 0 )
    {
        LogDCS( "[DownVehicleParmFile] OpenSession() 실패\n" );
		printf( "[DownVehicleParmFile] OpenSession() 실패\n" );
//        CloseSession( fdSock );
        return ErrRet( ERR_SESSION_OPEN );
    }

    /*
     *  집계 인증
     */
    if ( AuthDCS( fdSock, RECV_OPER_PARM_JOB, &stSendUsrPktMsg, &stRecvUsrPktMsg
                ) != SUCCESS )
    {
        LogDCS( "[DownVehicleParmFile] AuthDCS() 실패\n" );
		printf( "[DownVehicleParmFile] AuthDCS() 실패\n" );
		CloseSession( fdSock );
		DisplayASCInDownFND( FND_READY_MSG );	// FND 초기화
        return ErrRet( ERR_DCS_AUTH );
    }

    /*
     *  차량 파라미터 파일 수신
     */
    sRetVal = RecvVehicleParmFile( fdSock, &stSendUsrPktMsg, &stRecvUsrPktMsg );
    if( sRetVal != SUCCESS )
    {
        LogDCS( "[DownVehicleParmFile] RecvVehicleParmFile() 실패\n" );
		printf( "[DownVehicleParmFile] RecvVehicleParmFile() 실패\n" );
    	CloseSession( fdSock );
		DisplayASCInDownFND( FND_READY_MSG );	// FND 초기화
		return ErrRet( ERR_RECV_VEHICLE_FILE );
    }

    /*
     *  운영 파라미터 파일 쓰기
     */
    sRetVal = WriteOperParmFile( stRecvUsrPktMsg.pbRealSendRecvData,
                                 stRecvUsrPktMsg.lDataSize );
    if( sRetVal != SUCCESS )
    {
        LogDCS( "[DownVehicleParmFile] WriteOperParmFile() 실패\n" );
		printf( "[DownVehicleParmFile] WriteOperParmFile() 실패\n" );
    	CloseSession( fdSock );
		DisplayASCInDownFND( FND_READY_MSG );	// FND 초기화
        return ErrRet( ERR_RECV_VEHICLE_FILE );
    }

	/*
	 *  EOS 송신
	 */
	SendEOS( fdSock, &stSendUsrPktMsg );

	boolIsApplyNextVer = TRUE;					// 0 - can not apply, 1 - can apply
	boolIsApplyNextVerParm = TRUE;

	CloseSession( fdSock );
	DisplayASCInDownFND( FND_READY_MSG );		// FND 초기화

	printf( "[DownVehicleParmFile] 차량정보 다운로드 성공\n" );

	return sRetVal;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      RecvVehicleParmFile                                      *
*                                                                              *
*  DESCRIPTION :      집계시스템으로 부터 운행차량 파라미터 파일을 받는다.        *
*                                                                              *
*  INPUT PARAMETERS:    int fdSock    - 소켓                                   *
*                       USER_PKT_MSG* stSendUsrPktMsg  - 송신 패킷             *
*                       USER_PKT_MSG* stRecvUsrPktMsg  - 수신 패킷             *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       ERR_RECV_OPER_PARM_FILE                                *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short RecvVehicleParmFile( int fdSock,
		                          USER_PKT_MSG* pstSendUsrPktMsg,
		                          USER_PKT_MSG* pstRecvUsrPktMsg )
{
    /*
     *  운영파라미터 파일 요청정보 세팅
     */
    pstSendUsrPktMsg->chEncryptionYN = NOT_ENCRYPTION;    // 암호화 여부
    memcpy( pstSendUsrPktMsg->achConnCmd,                 // 운영파라미터 요청CMD
            RECV_OPER_PARM_FILE_CMD,
            sizeof( pstSendUsrPktMsg->achConnCmd ) );
    pstSendUsrPktMsg->lDataSize = 0;                      // data size

    memcpy( pstSendUsrPktMsg->pbRealSendRecvData,
            gstCommInfo.abVehicleID,
            sizeof( gstCommInfo.abVehicleID ) );

    /*
     *  운영파라미터 파일 요청 메세지 송신 및 수신
     */
    if ( SendNRecvUsrPkt2DCS( fdSock, pstSendUsrPktMsg, pstRecvUsrPktMsg )
         != SUCCESS )
    {
        return ErrRet( ERR_RECV_OPER_PARM_FILE );
    }

    return SUCCESS;
}



/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      RecvFileHeader                                           *
*                                                                              *
*  DESCRIPTION :      집계시스템으로부터 파일의 헤더를 수신                      *
*                                                                              *
*  INPUT PARAMETERS:    int fdSock                                             *
*                       USER_PKT_MSG* pstRecvUsrPktMsg                         *
*						bool* pboolIsMoreRecvYn	- 더 받을 것이 있는지 여부	   *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       ERR_SOCKET_RS_RECV                                     *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short RecvFilePkt( int fdSock, USER_PKT_MSG* pstRecvUsrPktMsg,
	bool* pboolIsMoreRecvYn )
{
	short sResult = SUCCESS;

    /*
     *  파일 헤더 수신
     */
	sResult = RecvUsrPktFromDCS( fdSock, TIMEOUT, MAX_RETRY_COUNT,
		pstRecvUsrPktMsg );
    if ( sResult != SUCCESS )
    {
        return ErrRet( ERR_RECV_FILE );
    }

    /*
     *  EOF의 경우
     */
	if ( memcmp( pstRecvUsrPktMsg->achConnCmd, EOF_CMD,
			COMMAND_LENGTH ) == 0 )
    {
        *pboolIsMoreRecvYn = TRUE;
        return RECV_EOF;
    }
    /*
     *  EOT의 경우
     */
    else if ( memcmp( pstRecvUsrPktMsg->achConnCmd, EOT_CMD,
			COMMAND_LENGTH ) == 0 )
    {
        *pboolIsMoreRecvYn = FALSE;
        return RECV_EOT;
    }
    /*
     *  EOS의 경우
     */
    else if ( memcmp( pstRecvUsrPktMsg->achConnCmd, EOS_CMD,
			COMMAND_LENGTH ) == 0 )
    {
        *pboolIsMoreRecvYn = FALSE;
        return RECV_EOS;
    }

    return SUCCESS;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      DownFromDCS                                              *
*                                                                              *
*  DESCRIPTION :      집계시스템으로부터 파일을 다운로드한다.                    *
*                                                                              *
*  INPUT PARAMETERS:  void                                                     *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       ErrRet( ERR_SESSION_OPEN )                             *
*                       ErrRet( ERR_SESSION_CLOSE )                            *
*                       ErrRet( ERR_DCS_AUTH )                                 *
*                       ErrRet( ERR_SEND_VER_IFNO )                            *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short DownFromDCS( void )
{
    short   sRetVal         = SUCCESS;
    int     fdSock;

    USER_PKT_MSG    stSendUsrPktMsg;
    USER_PKT_MSG    stRecvUsrPktMsg;
    byte            abSendBuff[MAX_PKT_SIZE];
    byte            abRecvBuff[MAX_PKT_SIZE];

    int 		nVerTypeLoop;
    bool 		boolIsMoreRecvYn;
    VER_INFO 	stTmpVerInfo;

    int 		nCurrVerFileNo = 0;
    int 		nNextVerFileNo = 0;


    LoadVerInfo( &stTmpVerInfo );			// 버전정보 로딩

    /*
     *  세션 오픈
     */
    fdSock = OpenSession( gstCommInfo.abDCSIPAddr, SERVER_COMM_PORT );
    if ( fdSock < 0 )
    {
        LogDCS( "[DownFromDCS] OpenSession() 실패\n" );
		printf( "[DownFromDCS] OpenSession() 실패\n" );
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
	if ( AuthDCS( fdSock, DOWN_FILE_JOB, &stSendUsrPktMsg,
			&stRecvUsrPktMsg ) != SUCCESS )
    {
        LogDCS( "[DownFromDCS] AuthDCS() 실패\n" );
		printf( "[DownFromDCS] AuthDCS() 실패\n" );
		CloseSession( fdSock );
		DisplayASCInDownFND( FND_READY_MSG );	// FND 초기화
        return ErrRet( ERR_DCS_AUTH );
    }

    /*
     *  0 - 현재버전
     *  1 - 미래버전
     */
    for ( nVerTypeLoop = 0 ; nVerTypeLoop < 2 ; nVerTypeLoop++ )
    {
        /*
         *  버스단말기의 버전을 집계시스템으로 송신하고 응답을 받는다.
         */
        if ( SendVerInfoPkt( fdSock, 
        					 nVerTypeLoop, 
        					 &stSendUsrPktMsg, 
        					 &stRecvUsrPktMsg )
             != SUCCESS )
        {
        	CloseSession( fdSock );
			DisplayASCInDownFND( FND_READY_MSG );	// FND 초기화
            return ErrRet( ERR_SEND_VER_IFNO );
        }

        while( TRUE )
        {
            /*
             *  파일 수신
             */
            sRetVal = RecvFileFromDCS( fdSock,
                                       &stSendUsrPktMsg,
                                       &stRecvUsrPktMsg,
                                       &boolIsMoreRecvYn,
                                       &nCurrVerFileNo,
                                       &nNextVerFileNo );

            if ( sRetVal != SUCCESS )   // 파일 수신 에러의 경우
            {
		        LogDCS( "[DownFromDCS] RecvFileFromDCS() 실패\n" );
				printf( "[DownFromDCS] RecvFileFromDCS() 실패\n" );
                /*
                 *  이어받기가 아닐 경우 그냥 종료하고
                 *  이어받기의 경우 버전을 update하고 종료한다.
                 */
                if ( nCurrVerFileNo >= RELAY_RECV_CLASS_NO )
                {
                	UpdateVerFile();    // 버전 업데이트
                }
				
                break;
                goto end;
            }
            else    // 파일 수신 성공의 경우 
            {
                if ( boolIsMoreRecvYn == FALSE )		// 더 받을 것이 없으면 
                {
                    UpdateVerFile();
                    break;
                }
                else       								// 더 받을 것이 존재하면 
                {
                    if ( (  ( nCurrVerFileNo >= RELAY_RECV_CLASS_NO &&
                              ( nNextVerFileNo == 0 ) )  ||
                            nNextVerFileNo >= RELAY_RECV_CLASS_NO )
                            )	// 이어받기 파일 구분 번호 이상의 경우 
                    {
                        UpdateVerFile();
                    }

                }

            }

        }
    }

    end :

	/*
	 *	미래적용버전이 적용될 경우 업로드용 버전파일을 생성한다.
	 */
    if ( boolIsApplyNextVer == TRUE )
    {
        SetUploadVerInfo();
    }

    CloseSession( fdSock );
	DisplayASCInDownFND( FND_READY_MSG );	// FND 초기화

    WriteCommSuccTime();

    return sRetVal;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SendVerInfoPkt                                           *
*                                                                              *
*  DESCRIPTION :      버전정보를 생성하여 집계시스템으로 송신하고 응답을 받는다.  *
*                                                                              *
*  INPUT PARAMETERS:    int fdSock,                                            *
*                       bool boolIsCurrVer,                                    *
*                       USER_PKT_MSG* stSendPktMsg,                            *
*                       USER_PKT_MSG* stRecvPktMsg                             *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       ErrRet( ERR_SEND_VER_IFNO )                            *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short SendVerInfoPkt( int fdSock,
                             bool boolIsCurrVer,
                             USER_PKT_MSG* pstSendUsrPktMsg,
                             USER_PKT_MSG* pstRecvUsrPktMsg )
{
	if ( boolIsCurrVer == 0 )
	{
		printf( "[SendVerInfoPkt] 현재 버전정보를 집계로 전송\n" );
	}
	else
	{
		printf( "[SendVerInfoPkt] 미래 버전정보를 집계로 전송\n" );
	}

    pstSendUsrPktMsg->chEncryptionYN = NOT_ENCRYPTION;
    memcpy( pstSendUsrPktMsg->achConnCmd, SEND_VER_INFO_CMD, COMMAND_LENGTH );

    pstSendUsrPktMsg->lDataSize =
                       CreateVerInfoPkt( boolIsCurrVer,
                                         pstSendUsrPktMsg->pbRealSendRecvData );

    if ( SendUsrPkt2DCS( fdSock, TIMEOUT, MAX_RETRY_COUNT, pstSendUsrPktMsg ) 
    	 != SUCCESS )
    {
        return ErrRet( ERR_SEND_VER_IFNO );
    }

    if ( RecvRS( fdSock, pstRecvUsrPktMsg )!= SUCCESS )
    {
        return ErrRet( ERR_SEND_VER_IFNO );
    }

    return SUCCESS;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      RecvFileFromDCS                                          *
*                                                                              *
*  DESCRIPTION :      집계시스템으로부터 파일을 수신한다.                        *
*                                                                              *
*  INPUT PARAMETERS:    int fdSock,                                            *
*                       USER_PKT_MSG* stSendUsrPktMsg,                         *
*                       USER_PKT_MSG* stRecvUsrPktMsg,                         *
*                       bool* pboolIsMoreRecvYn,                               *
*                       int* pnCurrVerFileNo,                                  *
*                       int* pnNextVerFileNo                                   *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                   ErrRet( ERR_RECV_FILE )                                    *
*                   ErrRet( ERR_FILE_OPEN | GetFileNo( achRecvFileName ) )     *
*                   ErrRet( ERR_FILE_OPEN | GetFileNo( RELAY_DOWN_INFO_FILE )  *
*                   ErrRet( ERR_FILE_WRITE | GetFileNo( achRecvFileName )      *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short RecvFileFromDCS( int fdSock,
                              USER_PKT_MSG* pstSendUsrPktMsg,
                              USER_PKT_MSG* pstRecvUsrPktMsg,
                              bool* pboolIsMoreRecvYn,
                              int* pnCurrVerFileNo,
                              int* pnNextVerFileNo )
{
    short   sRetVal         = SUCCESS;
    short   sReturnVal      = SUCCESS;
    FILE    *fdRecvFile;                        // 다운로드 받을 파일
    FILE    *fdRelayRecv;                       // 이어받기정보 파일

    int     nFileNo         = 0;                // 다운로드될 파일의 인덱스
    char    achRecvFileName[30] = { 0, };       // 수신될 파일명
    int     nRecvdFileSize  = 0;                // 수신된 파일의 size
	int     nPktNo          = 0;                // download pkt No.

	long lFreeMemorySize = 0;

    PKT_HEADER_INFO         stPktHeaderInfo;        // 수신받을 파일의 헤더
    DOWN_FILE_INFO          stDownFileInfo;         // 다운로드파일 정보 구조체
    FILE_RELAY_RECV_INFO    stFileRelayRecvInfo;    // 이어받기 정보 구조체

	time_t tStartDtime = 0;
	time_t tEndDtime = 0;

	time( &tStartDtime );

    /*
     *  파일헤더 수신
     */
    sRetVal = RecvFilePkt( fdSock, pstRecvUsrPktMsg, pboolIsMoreRecvYn );
    if( sRetVal > SUCCESS )     // EOF(1), EOT(2), EOS(3)가 수신될 경우 return
    {
        return SUCCESS;
    }
    else if ( sRetVal < SUCCESS )   // 수신 에러의 경우
    {
        return ErrRet( ERR_RECV_FILE );
    }

    /*
     *  파일 헤더 세팅
     */
    memcpy( (byte *)&stPktHeaderInfo,
            pstRecvUsrPktMsg->pbRealSendRecvData,
            sizeof( stPktHeaderInfo ) );

    /*
     *  다운될 파일의 인덱스와 파일명 세팅
     */
    nFileNo = GetDownFileIndex( pstRecvUsrPktMsg->achConnCmd );
    GetRecvFileName( pstRecvUsrPktMsg, &stPktHeaderInfo, achRecvFileName );

	printf( "[RecvFileFromDCS] 집계로부터 다운로드 : [%s] [FP : %lu] ",
		achRecvFileName, GetDWORDFromLITTLE( stPktHeaderInfo.abFirstPktNo ) );

    /*
     *  다운로드 될 파일 오픈
     *  첫패킷의 번호가 0이 아닐 경우 이어받기 중
     *  이어받기일 경우 다운로드정보파일에서 해당 레코드를 삭제하고
     *  기수신한 파일의 size를 세팅하고
     *  append mode로 다운로드 받는 파일을 오픈한다.
     */
    if ( GetDWORDFromLITTLE( stPktHeaderInfo.abFirstPktNo ) != SUCCESS )
    {
        DebugOut( "이어받는 중 %ld \n", 
				GetDWORDFromLITTLE( stPktHeaderInfo.abFirstPktNo ) );
		
        DelDownFileList( achRecvFileName, &stDownFileInfo );
        nRecvdFileSize = GetDWORDFromLITTLE( stDownFileInfo.abDownSize );

        if ( ( fdRecvFile = fopen( achRecvFileName, "ab" ) ) == NULL  )
        {
            return ErrRet( ERR_FILE_OPEN | GetFileNo( achRecvFileName ) );
        }
    }
    else
    /*
     *  이어받기 중이 아닐 경우
     *  write mode로 다운로드 받는 파일을 오픈한다.
     */
    {
        nRecvdFileSize = 0;

        if ( ( fdRecvFile = fopen( achRecvFileName, "wb" ) ) == NULL )
        {
            return ErrRet( ERR_FILE_OPEN | GetFileNo( achRecvFileName ) );
        }

    }

    /*
     *  이어받기 정보 파일을 오픈한다.
     */
    if ( ( fdRelayRecv = fopen( RELAY_DOWN_INFO_FILE, "wb" ) ) == NULL )
    {
        fclose( fdRecvFile );
        return ErrRet( ERR_FILE_OPEN | GetFileNo( RELAY_DOWN_INFO_FILE ) );
    }

    /*
     *  파일헤더에 대한 response 송신
     */
    sRetVal = SendRS( fdSock, pstSendUsrPktMsg );

    if ( sRetVal != SUCCESS )
    {
        fclose( fdRecvFile );
        fclose( fdRelayRecv );

        return sRetVal;
    }

    /*
     *  이어받기 대상 파일의 경우 ( 38번 이상의 파일 ) 이어받기 정보를 세팅
     */
    if ( nFileNo > RELAY_RECV_CLASS_NO )
    {
        /*
         *  파일 다운로드 중 update할 이어받기 정보를 세팅
         */
        SetRelayRecvInfo( &stFileRelayRecvInfo, nFileNo, &stPktHeaderInfo );
        nPktNo = GetDWORDFromLITTLE( stPktHeaderInfo.abFirstPktNo ) - 1 ;

        if ( nPktNo < 0 )
        {
            nPktNo = 0;
        }
    }

    if ( stPktHeaderInfo.bNewVerYN == GetASCNoFromDEC( CURR ) ) // Current
    {
        *pnNextVerFileNo = 0;
        *pnCurrVerFileNo = nFileNo;
    }
    else                                        // next
    {
        *pnNextVerFileNo = nFileNo;
    }

    /*
     *  실제 파일 수신
     */
    while( TRUE )
    {
    	printf( "." );
		fflush( stdout );

        InitPkt( pstRecvUsrPktMsg );
        InitPkt( pstSendUsrPktMsg );

        /*
         *  파일Pkt 수신
         */
        sReturnVal = RecvFilePkt( fdSock, pstRecvUsrPktMsg, pboolIsMoreRecvYn );

        /*
         *  EOF(1), EOT(2), EOS(3)가 수신될 경우 break
         */
        if( sReturnVal > SUCCESS )
        {
            sRetVal = SUCCESS;
            break;
        }
        else if ( sReturnVal < SUCCESS )
        {
            fclose( fdRecvFile );
            fclose( fdRelayRecv );
            return ErrRet( ERR_RECV_FILE );
        }

        nRecvdFileSize += pstRecvUsrPktMsg->lDataSize;

        /*
         *  수신된 pkt을 파일로 write
         */
        sRetVal = WriteRecvdFile( fdRecvFile, pstRecvUsrPktMsg );

        if ( sRetVal != SUCCESS )
        {
            fclose( fdRecvFile );
            fclose( fdRelayRecv );

			// 디스크 남은 공간 계산
        	lFreeMemorySize = MemoryCheck();
			// 디스크 남은 공간이 5MB 미만이면 파일 삭제
			if ( lFreeMemorySize >= 0 && lFreeMemorySize < 5 )
			{
				printf( "[RecvFileFromDCS] 디스크 남은 공간 부족으로 인하여 다운받은 TEMP 파일 삭제 : [%s] ", achRecvFileName );
				// TEMP 파일 삭제
				unlink( achRecvFileName );
				// 이어받기 파일인 경우 이어받기정보파일 삭제
				if ( nFileNo > RELAY_RECV_CLASS_NO )
				{
					unlink( RELAY_DOWN_INFO_FILE );
				}
				// 이벤트를 센터로 전송
				ctrl_event_info_write( EVENT_INSUFFICIENT_DISK_DURING_DOWNLOAD );
			}

            return sRetVal;
        }

        /*
         *  이어받기대상 파일의 경우 이어받기정보 파일을 update해준다
         *  AI정보는 이어받지 않음
         */
        if ( nFileNo > RELAY_RECV_CLASS_NO &&
             nFileNo < MASTER_AI_FILE_INDEX_NO )
        {
            nPktNo++;
            DWORD2LITTLE( nPktNo, stFileRelayRecvInfo.abDownFilePktNo );

            UpdateRelayRecvFile( fdRelayRecv, &stFileRelayRecvInfo );
        }

//		fflush( fdRecvFile );
        fflush( fdRelayRecv );

        /*
         *  response 송신
         */
        sRetVal = SendRS( fdSock, pstSendUsrPktMsg );

        if ( sRetVal != SUCCESS )
        {
            break;
        }
    }

    /*
     *  파일수신 에러일 경우
     */
    if ( sRetVal != SUCCESS )
    {
    	printf( "e\n" );

        /*
         *  다운로드 받는 파일의 타입이 메세지일 경우(37이전의 파일)
         *  다운로드정보파일에 write하지 않는다.
         */
        if ( stPktHeaderInfo.bDataType == DOWN_MSG_TYPE )
        {
            fclose( fdRecvFile );
            fclose( fdRelayRecv );
            remove( achRecvFileName );
            return sReturnVal;
        }

        WriteDownFileInfo(  achRecvFileName,
                            stPktHeaderInfo.abFileVer,
                            UNDER_DOWN,
                            nRecvdFileSize );

    }
    else
    {
    	printf( "/\n" );

        WriteDownFileInfo(  achRecvFileName,
                            stPktHeaderInfo.abFileVer,
                            DOWN_COMPL,
                            nRecvdFileSize );
    }

    fclose( fdRecvFile );
    fclose( fdRelayRecv );

	time( &tEndDtime );
	printf( "[RecvFileFromDCS] 실행시간 : %d sec\n", (int)( tEndDtime - tStartDtime ) );

    return sRetVal;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      WriteRecvdFile                                           *
*                                                                              *
*  DESCRIPTION :      수신되는 파일 쓰기                                        *
*                                                                              *
*  INPUT PARAMETERS:    File*       fdRecvFile,                                *
*                       USER_PKT_MSG* pstRecvUsrPktMsg                         *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       ErrRet( ERR_FILE_WRITE | GetFileNo( achRecvFileName ) )*
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short WriteRecvdFile( FILE*      	fdRecvFile,
                             USER_PKT_MSG* 	pstRecvUsrPktMsg )
{
	int nResult = 0;

	/*
	 *  수신된 파일 pkt을 write
	 */
	nResult = fwrite( pstRecvUsrPktMsg->pbRealSendRecvData,
		pstRecvUsrPktMsg->lDataSize,
		1,
		fdRecvFile );
	if ( nResult < 1 )
	{
		return ERR_FILE_WRITE;
	}

	nResult = fflush( fdRecvFile );
	if ( nResult != SUCCESS )
	{
		return ERR_FILE_WRITE;
	}

	return SUCCESS;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SetRelayRecvInfo                                         *
*                                                                              *
*  DESCRIPTION :      이어받기 정보를 세팅                                      *
*                                                                              *
*  INPUT PARAMETERS:    FILE_RELAY_RECV_INFO *pstFileRelayRecvInfo,            *
*                       int nFileNo,                                           *
*                       PKT_HEADER_INFO *pstPktHeaderInfo                      *
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
static void SetRelayRecvInfo( FILE_RELAY_RECV_INFO *pstFileRelayRecvInfo,
                              int nFileNo,
                              PKT_HEADER_INFO *pstPktHeaderInfo )
{
    /*
     *  이어받기 대상 파일의 경우 ( 38번 이상의 파일 )
     *  이어받기 정보를 세팅
     */
    if ( nFileNo > RELAY_RECV_CLASS_NO )
    {
        pstFileRelayRecvInfo->chFileNo = nFileNo;

        memcpy( pstFileRelayRecvInfo->abDownFileVer,
                pstPktHeaderInfo->abFileVer,
                sizeof( pstFileRelayRecvInfo->abDownFileVer ) );

        memcpy( pstFileRelayRecvInfo->abDownFileSize,
                pstPktHeaderInfo->abFileSize,
                sizeof( pstFileRelayRecvInfo->abDownFileSize ) );

    }
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      GetRelayRecvFileSize                                     *
*                                                                              *
*  DESCRIPTION :      이어받는 중의 파일의 size를 구한다.                       *
*                                                                              *
*  INPUT PARAMETERS:  char chFileNo, long lFileSize                            *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       ERR_FILE_READ | GetFileNo(  RELAY_DOWN_INFO_FILE )     *
*                       ERR_FILE_DATA_NOT_FOUND |                              *
                     		GetFileNo( RELAY_DOWN_INFO_FILE )                  *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short GetRelayRecvFileSize( char chFileNo, long *plFileSize )
{
    short   sReturnVal = SUCCESS;
    int     fdRelayRecvInfoFile;
    int     nReadByte;
    char    achFileName[50] = { 0, };

    FILE_RELAY_RECV_INFO stRelayRecvInfo;

    system( "chmod 755 /mnt/mtd8/bus/?*" );

    fdRelayRecvInfoFile = open( RELAY_DOWN_INFO_FILE, O_RDONLY, OPENMODE );

    if ( fdRelayRecvInfoFile < 0 )
    {
        return  SUCCESS;
    }

    lseek( fdRelayRecvInfoFile, 0, SEEK_SET );

    nReadByte = read( fdRelayRecvInfoFile,
                      (void*)&stRelayRecvInfo,
                      sizeof( FILE_RELAY_RECV_INFO ) );

    if ( nReadByte == 0 )
    {
        sReturnVal = ERR_FILE_READ | GetFileNo(  RELAY_DOWN_INFO_FILE );
    }

    if ( nReadByte < 0 )
    {
        sReturnVal = ERR_FILE_READ | GetFileNo( RELAY_DOWN_INFO_FILE );
    }

    if ( chFileNo != stRelayRecvInfo.chFileNo )
    {
        sReturnVal = ERR_FILE_DATA_NOT_FOUND |
                     GetFileNo( RELAY_DOWN_INFO_FILE );
    }

    close( fdRelayRecvInfoFile );

    /*
     *  파일명 세팅
     */
    sprintf( achFileName, "tmp_c_%s",
             achDCSCommFile[(int)chFileNo][1] );
//PrintlnASC( "파일 사이즈 =>", achFileName, 16 );
    *plFileSize = GetFileSize( achFileName );
//printf( "사이즈는 %ld\n", GetFileSize( achFileName ));
    return sReturnVal;

}
