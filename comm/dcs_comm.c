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
*  PROGRAM ID :       dcs_comm.c	                                           *
*                                                                              *
*  DESCRIPTION:       집계시스템과의 통신을 위한 함수들을 제공한다.            *
*                                                                              *
*  ENTRY POINT:     void InitPkt( USER_PKT_MSG* stUserMsg );				   *
*					short SendUsrPkt2DCS( 	int fdSock,						   *
*                             				int nTimeOut,					   *
*                             				int nRetryCnt,					   *
*                             				USER_PKT_MSG* stUsrPktMsg );	   *
*					short RecvUsrPktFromDCS( int fdSock,					   *
*                                			 int nTimeOut,					   *
*                                			 int nRetryCnt,	 				   *
*                                			 USER_PKT_MSG* stUsrPktMsg );	   *
*					int OpenSession( char *pchIP, char *pchPort );			   *
*					short CloseSession( int fdSock );	  					   *
*					short AuthDCS( 	int fdSock,								   *
*                      				char* pchSessionCd,						   *
*                      				USER_PKT_MSG* pstSendUsrPktMsg,	 		   *
*                      				USER_PKT_MSG* pstRecvUsrPktMsg );		   *
*					short SendNRecvUsrPkt2DCS( int fdSock,					   *
*                          					   USER_PKT_MSG* pstSendUsrPktMsg, *
*                          					   USER_PKT_MSG* pstRecvUsrPktMsg )*
*					short SendRS( int fdSock, USER_PKT_MSG* pstSendUsrPktMsg );*
*					short SendEOS( int fdSock, USER_PKT_MSG* pstSendUsrPktMsg )*
*					short SendEOF( int fdSock, USER_PKT_MSG* pstSendUsrPktMsg )*
*					short RecvRS( int fdSock, USER_PKT_MSG* pstRecvUsrPktMsg );*
*					short RecvACK( int fdSock, USER_PKT_MSG* pstRecvUsrPktMsg )*
*					short GetDownFileIndex( char* pchCmd );					   *
*					void GetRecvFileName( USER_PKT_MSG* pstRecvUsrPktMsg,	   *
*                             			  PKT_HEADER_INFO*   pstPktHeaderInfo, *
*                             			  char*  achRecvFileName );			   *
*					short ReSetupTerm( void );								   *
*					short SetupTerm( void );								   *
*					void *DCSComm( void *arg );								   *
*                                                                              *
*  INPUT FILES:     None                                                       *
*                                                                              *
*  OUTPUT FILES:      c_op_par.dat - 운행차량파라미터 파일                     *
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
#include "../proc/file_mgt.h"
#include "../proc/main.h"
#include "des.h"
#include "wlan.h"
#include "socket_comm.h"
#include "dcs_comm.h"
#include "download_dcs_comm.h"
#include "upload_dcs_comm.h"
#include "../proc/reconcile_file_mgt.h"
#include "../proc/load_parameter_file_mgt.h"
#include "../proc/upload_file_mgt.h"
#include "../proc/version_file_mgt.h"
#include "../proc/version_mgt.h"
#include "../proc/main_process_busTerm.h"

/*******************************************************************************
*  Declaration of variables                                                    *
*******************************************************************************/
#define DCS_AUTH_CMD                    "ENQ00"		// 인증요청 cmd
#define DCS_AUTH_CMD1                   "AA000"		// 서버 초기 key
#define DCS_AUTH_CMD2                   "AB000"		// 클라이언트 인증 key
#define DCS_AUTH_CMD3                   "AC000"		// 서버 인증 key

#define RESPONSE_CMD                    "RS000"
#define ACK_CMD                         "ACK00"
#define RESPONSE_STATUS_MSG             "0000"
#define LEN_UNTIL_REAL_SEND_DATA    	19

#define DEC_CLIENT_RAND_NO          	"9876543213333333"
#define CHILD_DEFAULT_ID            	"0000000000"

#define MASTER_BL_BACKUP_FILE           "c_fi_bl.backup"
#define MASTER_PREPAY_PL__BACKUP_FILE   "c_fa_pl.backup"
#define MASTER_POSTPAY_PL__BACKUP_FILE  "c_fd_pl.backup"
#define MASTER_AI_BACKUP_FILE           "c_fi_ai.backup"
#define KPDAPPLY_FLAG_BACKUP_FILE       "driverdn.backup"

#define RANDOM_NO_LENGTH               	16
#define KEY_LENGTH                  	16

byte gbDCSCommType = 0;						// 집계통신유형
											// 1: 차량정보파일 다운로드
											// 2: 파일 업로드
											// 3: 파일 다운로드
char    achDCSCurrDtime[15];				// 집계PC 시간
static  long    lSendSeq2DCS = -1;
static  byte    abRecvSeqNo[5];
static  int     nTermIDLength = sizeof( gpstSharedInfo->abMainTermID );
static  int     nPSAMIDLength = sizeof( gpstSharedInfo->abMainPSAMID );
static  int     nPSAMIDBCDLength    = sizeof( gpstSharedInfo->abMainPSAMID )/2;

/*******************************************************************************
*  Declaration of Module Functions                                             *
*******************************************************************************/
static short SendPkt2DCS( int fdSock, int nTimeOutSec, int nRetryCnt,
	PKT_MSG* stPktMsg );
static short RecvPktFromDCS( int fdSock, int nTimeOutSec, int nRetryCnt,
	PKT_MSG* stPktMsg );

static short DisplayDCSCommCnt( int nDCSCommCnt );

static bool IsValidACAuthMsg( USER_PKT_MSG* stRecvUsrPktMsg, 
							  int* pnSecondKeyIdx );

static void SetABAuthMsg( USER_PKT_MSG* stSendUsrPktMsg,
		                  USER_PKT_MSG* stRecvUsrPktMsg,
		                  char* pchSessionCd, int* pnSecondKeyIdx );

static short ReqDCSAuth( int fdSock,
		                 USER_PKT_MSG* stSendUsrPktMsg,
		                 USER_PKT_MSG* stRecvUsrPktMsg );

static short GetAuthIndex( char* pchRandomNo );

static short ChkNDownOperParmFile( void );

static void InitNextVerApplFlag( void );

static void DelBackupFile( void );

static void DelFileOnReset( void );

static void ReqKeysetRegist( void );

static bool ConnDCS( void );

static void ReqApplyNextVer( byte bCmd,
		                     bool boolIsApplyNextVer,
		                     bool boolIsApplyNextVerParm,
		                     bool boolIsApplyNextVerAppl,
		                     bool boolIsApplyNextVerVoice,
		                     bool boolIsApplyNextVerDriverAppl);



/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      InitPkt                                                  *
*                                                                              *
*  DESCRIPTION :      패킷의 내용을 초기화한다.                                *
*                                                                              *
*  INPUT PARAMETERS:  USER_PKT_MSG* stUserMsg                                  *
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
void InitPkt( USER_PKT_MSG* stUserMsg )
{
    byte*   pbDataPtr;
    int     nMaxDataSize;

    pbDataPtr       = stUserMsg->pbRealSendRecvData;
    nMaxDataSize    = stUserMsg->nMaxDataSize;

    memset( stUserMsg, 0x00, sizeof( USER_PKT_MSG ) );

    stUserMsg->pbRealSendRecvData        = pbDataPtr;
    stUserMsg->nMaxDataSize              = nMaxDataSize;

    memset( stUserMsg->pbRealSendRecvData, 0x00, nMaxDataSize);

}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SendUsrPkt2DCS                                           *
*                                                                              *
*  DESCRIPTION :     USER_PKT_MSG을 PKT_MSG로 변환후 해당 소켓을 통해 패킷을     *
*                       송신한다.                                              *
*                                                                              *
*  INPUT PARAMETERS:    int fdSock,                                            *
*                       int nTimeOut,                                          *
*                       int nRetryCnt,                                         *
*                       USER_PKT_MSG* stUsrPktMsg                              *
*                                                                              *
*  RETURN/EXIT VALUE:  SendPkt2DCS( fdSock,  nTimeOut,  nRetryCnt, &stPktMsg ) *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short SendUsrPkt2DCS( int fdSock,
                      int nTimeOut,
                      int nRetryCnt,
                      USER_PKT_MSG* stUsrPktMsg )
{
    PKT_MSG     stPktMsg            = { 0, };
    dword       dwUsrMsgSize;

    lSendSeq2DCS++;

    stPktMsg.bSTX = STX;
    stPktMsg.bETX = ETX;
    stPktMsg.chEncryptionYN = stUsrPktMsg->chEncryptionYN;

    memcpy( stPktMsg.abConnSeqNo,
            (char*)&lSendSeq2DCS,
            sizeof( stPktMsg.abConnSeqNo ) );
    memcpy( stPktMsg.achConnCmd,
            stUsrPktMsg->achConnCmd,
            sizeof( stPktMsg.achConnCmd ) );

    dwUsrMsgSize = stUsrPktMsg->lDataSize;
    memcpy( stPktMsg.achDataSize,
            (char*)&dwUsrMsgSize,
            sizeof( stPktMsg.achDataSize ) );

    dwUsrMsgSize = stUsrPktMsg->lDataSize +
                  ( sizeof( PKT_MSG ) - sizeof( stPktMsg.pbRealSendRecvData ) );
    memcpy( stPktMsg.achPktSize,
            (char*)&dwUsrMsgSize,
            sizeof( stPktMsg.achPktSize ) );

    stPktMsg.pbRealSendRecvData = stUsrPktMsg->pbRealSendRecvData;

    return SendPkt2DCS( fdSock,  nTimeOut,  nRetryCnt, &stPktMsg );

}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SendPkt2DCS                                              *
*                                                                              *
*  DESCRIPTION :      해당 패킷에 대한 BCC를 생성하고 해당 소켓으로 메세지를 송신 *
*                       한다.                                                  *
*                                                                              *
*  INPUT PARAMETERS:    int fdSock,                                            *
*                       int nTimeOut,                                          *
*                       int nRetryCnt,                                         *
*                       PKT_MSG* stPktMsg                                      *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       ERR_SOCKET_SEND                                        *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short SendPkt2DCS( int fdSock, int nTimeOutSec, int nRetryCnt,
	PKT_MSG* stPktMsg )
{
    short sResult = SUCCESS;
    long lDataSize = 0;
    long lPktSize = 0;
    byte abSendBuf[2048] = {0, };

    /*
     *  패킷 전체의 전체 size
     */
	memcpy( (char*)&lPktSize,
		stPktMsg->achPktSize,
		sizeof( stPktMsg->achPktSize ) );

    /*
     *  데이터 size
     */
	memcpy( (char*)&lDataSize,
		stPktMsg->achDataSize,
		sizeof( stPktMsg->achDataSize ) );

    memset( abSendBuf, 0x00, lPktSize );

    /*
     *  데이터 이전까지 프로토콜 정보 copy
     */
    memcpy( abSendBuf, (void*)&stPktMsg->bSTX, LEN_UNTIL_REAL_SEND_DATA );

    /*
     *  데이터 copy
     */
	memcpy( abSendBuf + LEN_UNTIL_REAL_SEND_DATA,
		(void*)stPktMsg->pbRealSendRecvData,
		lDataSize );

    /*
     *  데이터 이후의 프로토콜 정보 copy
     */
    memcpy( abSendBuf + LEN_UNTIL_REAL_SEND_DATA + lDataSize,
		(void *)&stPktMsg->bETX,
		sizeof( stPktMsg->bETX ) );

    /*
     *  BCC정보
     */
    abSendBuf[lPktSize-1] = MakeBCC( &abSendBuf[1], lPktSize-3 );
	
    while ( nRetryCnt-- )
    {
        sResult = SockSendPkt( fdSock, nTimeOutSec, lPktSize, abSendBuf );
        if ( sResult == SUCCESS )
        {
            break;
        }
    }

    return sResult;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      RecvUsrPktFromDCS                                        *
*                                                                              *
*  DESCRIPTION :      해당 socket으로부터 패킷을 수신받는다                      *
*                                                                              *
*  INPUT PARAMETERS:    int fdSock,                                            *
*                       int nTimeOut,                                          *
*                       int nRetryCnt,                                         *
*                       USER_PKT_MSG* stUsrPktMsg                              *
*                                                                              *
*  RETURN/EXIT VALUE:     SUCCESS                                              *
*                       ERR_PACKET_STX                                         *
*                       ERR_PACKET_ETX                                         *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short RecvUsrPktFromDCS( int fdSock,
                         int nTimeOut,
                         int nRetryCnt,
                         USER_PKT_MSG* stUsrPktMsg )
{
    short sResult = SUCCESS;
    PKT_MSG stPktMsg;

    stPktMsg.pbRealSendRecvData = stUsrPktMsg->pbRealSendRecvData;
    sResult = RecvPktFromDCS( fdSock, nTimeOut, nRetryCnt, &stPktMsg );
    if ( sResult < 0 )
    {
//    	printf("[RecvUsrPktFromDCS] RecvPktFromDCS() 실패\n");
        return sResult;
    }

    if ( stPktMsg.bSTX != STX )
    {
    	LogDCS("[RecvUsrPktFromDCS] STX로 시작 않음\n");
    	printf("[R_NOT_STX]");
		fflush( stdout );
        return ErrRet( ERR_PACKET_STX );
    }

    if ( stPktMsg.bETX != ETX )
    {
    	LogDCS("[RecvUsrPktFromDCS] ETX로 끝나지 않음\n");
    	printf("[R_NOT_ETX]");
		fflush( stdout );
        return ErrRet( ERR_PACKET_ETX );
    }

    stUsrPktMsg->chEncryptionYN = stPktMsg.chEncryptionYN;
    memcpy( stUsrPktMsg->achConnCmd,
		stPktMsg.achConnCmd,
		sizeof( stPktMsg.achConnCmd ) );

	memcpy( abRecvSeqNo,
		stPktMsg.abConnSeqNo,
		sizeof( stPktMsg.abConnSeqNo ) );
	memcpy( (char*)&stUsrPktMsg->lDataSize,
		stPktMsg.achDataSize,
		sizeof( stPktMsg.achDataSize ) );

    return SUCCESS;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      RecvPktFromDCS                                           *
*                                                                              *
*  DESCRIPTION :      해당 소캣으로 메세지를 수신받는다.                         *
*                                                                              *
*  INPUT PARAMETERS: int fdSock, int nTimeOut, int nRetryCnt, PKT_MSG* stPktMsg*
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       ERR_PACKET_BCC                                         *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short RecvPktFromDCS( int fdSock, int nTimeOutSec, int nRetryCnt,
	PKT_MSG* stPktMsg )
{
    short sResult = SUCCESS;
    int nPktSize = 0;
    byte abRecvBuf[MAX_RECV_PKT_SIZE] = {0, };

    while ( nRetryCnt-- )
    {
        sResult = SockRecvPkt( fdSock, nTimeOutSec, &nPktSize, abRecvBuf );
        if ( sResult == SUCCESS )
        {
            break;
        }
    }
    if ( sResult < SUCCESS )
    {
        return sResult;
    }

    /*
     *  STX 다음부터 ETX정보 전까지 BCC체크 대상이 된다.
     */
    if ( abRecvBuf[nPktSize-1] != MakeBCC( &abRecvBuf[1], nPktSize - 3 ) )
    {
		LogDCS( "[RecvPktFromDCS] BCC 오류\n" );
        printf( "[R_BCC]" );
		fflush( stdout );
        return ErrRet( ERR_PACKET_BCC );
    }

    memcpy( &stPktMsg->bSTX, abRecvBuf, LEN_UNTIL_REAL_SEND_DATA );
    memcpy( stPktMsg->pbRealSendRecvData,
		abRecvBuf + LEN_UNTIL_REAL_SEND_DATA,
		nPktSize - 21 );

    memcpy( &stPktMsg->bETX, abRecvBuf + nPktSize - 2, 2 );

    return SUCCESS;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      OpenSession                                              *
*                                                                              *
*  DESCRIPTION :      세션을 오픈한다.                                          *
*                                                                              *
*  INPUT PARAMETERS:  char *pchIP, char *pchPort                               *
*                                                                              *
*  RETURN/EXIT VALUE:     OpenSock( pchIP, pchPort )                           *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
int OpenSession( char *pchIP, char *pchPort )
{
	/*
	 *	송신순번, receive순번 초기화
	 */
    lSendSeq2DCS = 0;
    memset( abRecvSeqNo, 0, sizeof( abRecvSeqNo ) );

    return OpenSock( pchIP, pchPort );
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      CloseSession                                             *
*                                                                              *
*  DESCRIPTION :      세션을 닫는다.                                            *
*                                                                              *
*  INPUT PARAMETERS:  int fdSock                                               *
*                                                                              *
*  RETURN/EXIT VALUE:     CloseSock( fdSock )                                  *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short CloseSession( int fdSock )
{
	short sResult = SUCCESS;

    lSendSeq2DCS = -1;
    memset( abRecvSeqNo, 0, sizeof( abRecvSeqNo ) );

	sResult = CloseSock( fdSock );

	sleep( 1 );

    return sResult;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      DisplayDCSCommCnt                                        *
*                                                                              *
*  DESCRIPTION :      집계통신횟수를 FND에 디스플레이 한다.                      *
*                                                                              *
*  INPUT PARAMETERS:  int nDCSCommCnt                                          *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short DisplayDCSCommCnt( int nDCSCommCnt )
{
    int     nRecCnt         = 0;                    // 전체 레코드 수
    int     nSendWaitCnt    = 0;                    // 송신할 파일 수

    char    achDCSCommCnt[7]    = { 0, };
    char    achReconcileCnt[7]  = { 0, };

    nSendWaitCnt = GetReconcileCnt( &nRecCnt );     // 송신할파일수 /레코드 수

    if ( nSendWaitCnt < 0 )
    {
        nRecCnt = 0;
        nSendWaitCnt = 0;
    }

    sprintf( achDCSCommCnt, "%6d", nDCSCommCnt );
    sprintf( achReconcileCnt, "%03d%03d", nRecCnt, nSendWaitCnt );

    DisplayDWORDInUpFND( GetDWORDFromASC( achReconcileCnt,
                                          sizeof( achReconcileCnt ) -1 ) );
    usleep( 20000 );

    DisplayASCInDownFND( achDCSCommCnt );

    return SUCCESS;

}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      AuthDCS                                                  *
*                                                                              *
*  DESCRIPTION :      집계시스템으로 인증을 받는다.                              *
*                                                                              *
*  INPUT PARAMETERS:  int fdSock,                                              *
*                     char* pchSessionCd,                                      *
*                     USER_PKT_MSG* stSendUsrPktMsg,                           *
*                     USER_PKT_MSG* stRecvUsrPktMsg                            *
*                                                                              *
*  RETURN/EXIT VALUE:     SUCCESS                                              *
*                         ERR_DCS_AUTH                                         *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short AuthDCS( int fdSock,
               char* pchSessionCd,
               USER_PKT_MSG* pstSendUsrPktMsg,
               USER_PKT_MSG* pstRecvUsrPktMsg )
{
    int     nSecondKeyIdx;

    DisplayCommUpDownMsg( 0, GetWORDFromASC( pchSessionCd, 2 ) );
	gbDCSCommType = GetWORDFromASC( pchSessionCd, 2 );

    InitPkt( pstSendUsrPktMsg );
    InitPkt( pstRecvUsrPktMsg );

    /*
     *  집계인증 요청
     */
    if ( ReqDCSAuth( fdSock, pstSendUsrPktMsg, pstRecvUsrPktMsg ) != SUCCESS )
    {
    	printf( "[AuthDCS] ReqDCSAuth() 실패\n" );
        return ERR_DCS_AUTH;
    }

    /*
     *  AB 메세지 세팅
     */
    SetABAuthMsg( pstSendUsrPktMsg, 
    			  pstRecvUsrPktMsg, 
    			  pchSessionCd, 
    			  &nSecondKeyIdx );

    /*
     *  AB 메세지 송신 & AC 메세지 수신
     */
    if ( SendNRecvUsrPkt2DCS( fdSock, pstSendUsrPktMsg, pstRecvUsrPktMsg )
         != SUCCESS )
    {
    	printf( "[AuthDCS] 'AB' 메시지 송신 & 'AC' 메시지 수신 실패\n" );
        return ErrRet( ERR_DCS_AUTH );
    }

    /*
     *  AC 메세지 validation
     */
    if ( IsValidACAuthMsg( pstRecvUsrPktMsg, &nSecondKeyIdx ) == FALSE )
    {
    	printf( "[AuthDCS] 'AC' 메세지 validation 실패\n" );
        return ErrRet( ERR_DCS_AUTH );
    }

    return SUCCESS;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      IsValidACAuthMsg                                         *
*                                                                              *
*  DESCRIPTION :      AC메세지를 validation한다.                                *
*                                                                              *
*  INPUT PARAMETERS:  USER_PKT_MSG* stRecvUsrPktMsg                            *
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
static bool IsValidACAuthMsg( USER_PKT_MSG* stRecvUsrPktMsg, 
							  int* pnSecondKeyIdx )
{
    char    achEncSrvRandNo[RANDOM_NO_LENGTH + 1]  	= { 0, };
    char    achDecSrvRandNo[RANDOM_NO_LENGTH + 1]  	= { 0, };
    char    achDecClntRandNo[RANDOM_NO_LENGTH + 1] 	= { 0, };
    int     i;
    int     nMsgLength                          	= RANDOM_NO_LENGTH;
    int     nCompFlag				  				= 0;
    
    /*
     *  수신된 AC 메세지 validation
     */
    if ( memcmp( stRecvUsrPktMsg->achConnCmd,
                 DCS_AUTH_CMD3,
                 sizeof( stRecvUsrPktMsg->achConnCmd )
               ) != 0 )
    {
        return ErrRet( ERR_DCS_AUTH );
    }

    memcpy( achDecClntRandNo, DEC_CLIENT_RAND_NO, RANDOM_NO_LENGTH );

    /*
     *  서버 난수를 해독한다.
     */
    memcpy( achEncSrvRandNo, stRecvUsrPktMsg->pbRealSendRecvData, KEY_LENGTH );
    TransDecrypt( achEncSrvRandNo,
                  achDecSrvRandNo,
                  &nMsgLength,
                  achFixedKey[*pnSecondKeyIdx] );

    /*
     *  서버 난수와 클라이언트 난수를 비교한다.
     */
    for ( i = 0; i < RANDOM_NO_LENGTH; i++ )
    {

        if ( achDecClntRandNo[i] != achDecSrvRandNo[i] )
        {
            nCompFlag = 1;
            break;
        }
    }

    if ( nCompFlag == 1 )
    {
        return FALSE;
    }

    return TRUE;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SetABAuthMsg                                             *
*                                                                              *
*  DESCRIPTION :      AB 인증 메세지를 세팅한다.                                *
*                                                                              *
*  INPUT PARAMETERS:  USER_PKT_MSG* stSendUsrPktMsg                            *
*                     USER_PKT_MSG* stRecvUsrPktMsg                            *
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
static void SetABAuthMsg( USER_PKT_MSG* stSendUsrPktMsg,
	                      USER_PKT_MSG* stRecvUsrPktMsg,
	                      char* pchSessionCd, 
	                      int* pnSecondKeyIdx )
{
    char    achEncSrvRandNo[RANDOM_NO_LENGTH + 1]  = { 0, };
    char    achDecSrvRandNo[RANDOM_NO_LENGTH + 1]  = { 0, };
    char    achDecClntRandNo[RANDOM_NO_LENGTH + 1] = { 0, };
    char    achEncClntRandNo[RANDOM_NO_LENGTH + 1] = { 0, };
    int     nFirstKeyIdx;
    int     nTmpAdd         					= 0;
    int     nMsgLength                          = RANDOM_NO_LENGTH;

    time_t  tCurrDtime;
    byte    abTmp[11];

    /*
     *  암호화된 서버 난수
     */
    memcpy( achEncSrvRandNo, stRecvUsrPktMsg->pbRealSendRecvData, KEY_LENGTH );

    /*
     *  DCS Current Dtime
     */
    memcpy( achDCSCurrDtime,
            stRecvUsrPktMsg->pbRealSendRecvData + KEY_LENGTH,
            sizeof( achDCSCurrDtime ) - 1 );

    /*
     *  key 생성
     */
    TransDecrypt( achEncSrvRandNo,
                  achDecSrvRandNo,
                  &nMsgLength,
                  achInitialKey );      // 초기 key로 decryption
    nFirstKeyIdx = GetAuthIndex( achDecSrvRandNo );

    TransEncrypt( achDecSrvRandNo,
                  achEncSrvRandNo,
                  &nMsgLength,
                  achFixedKey[nFirstKeyIdx] );  // first key로 encryption

    memcpy( achDecClntRandNo, DEC_CLIENT_RAND_NO, RANDOM_NO_LENGTH );

    *pnSecondKeyIdx = GetAuthIndex( achDecClntRandNo );
    *pnSecondKeyIdx = ( nFirstKeyIdx + *pnSecondKeyIdx ) % 10 ;

    TransEncrypt( achDecClntRandNo,
                  achEncClntRandNo,
                  &nMsgLength,
                  achFixedKey[nFirstKeyIdx] );  // first key로 encryption

    /*
     *  AB 메세지 세팅
     */
    stSendUsrPktMsg->chEncryptionYN = ENCRYPTION;
    memcpy( stSendUsrPktMsg->achConnCmd, DCS_AUTH_CMD2, COMMAND_LENGTH );

    memcpy( stSendUsrPktMsg->pbRealSendRecvData,
            achEncSrvRandNo,
            KEY_LENGTH );               // 암호화된 서버쪽 난수
    nTmpAdd = nTmpAdd + KEY_LENGTH;

    memcpy( stSendUsrPktMsg->pbRealSendRecvData + nTmpAdd,
            achEncClntRandNo,
            KEY_LENGTH );               // 암호화된 클라이언트쪽 난수
    nTmpAdd = nTmpAdd + KEY_LENGTH;

    memcpy( stSendUsrPktMsg->pbRealSendRecvData + nTmpAdd,
            pchSessionCd,
            JOB_SESSION_CODE_LEN );
    nTmpAdd = nTmpAdd + JOB_SESSION_CODE_LEN;              // session code

    memcpy( stSendUsrPktMsg->pbRealSendRecvData + nTmpAdd,
             gpstSharedInfo->abMainTermID,
             nTermIDLength );           // Main terminal ID
    nTmpAdd = nTmpAdd + nTermIDLength;

    memcpy( stSendUsrPktMsg->pbRealSendRecvData + nTmpAdd,
             gstCommInfo.abVehicleID,
             sizeof( gstCommInfo.abVehicleID ) );    // 차량 ID
    nTmpAdd = nTmpAdd + sizeof( gstVehicleParm.abVehicleID );

    memcpy( stSendUsrPktMsg->pbRealSendRecvData + nTmpAdd,
            gstVehicleParm.abTranspBizrID,
            sizeof( gstVehicleParm.abTranspBizrID ) );
                                        // transportation Business company ID
    nTmpAdd = nTmpAdd + sizeof( gstVehicleParm.abTranspBizrID );

    ASC2BCD( gpstSharedInfo->abMainPSAMID,
             stSendUsrPktMsg->pbRealSendRecvData + nTmpAdd,
             nPSAMIDLength );           // main terminal SAM ID
    nTmpAdd = nTmpAdd + nPSAMIDBCDLength;

    memcpy( stSendUsrPktMsg->pbRealSendRecvData + nTmpAdd,
            &gbSubTermCnt,
            2 );                        // sub terminal count

    nTmpAdd = nTmpAdd + 2;
    ASC2BCD( gpstSharedInfo->abSubPSAMID[0],
             stSendUsrPktMsg->pbRealSendRecvData + nTmpAdd,
             nPSAMIDLength );           // sub terminal 1  SAM ID
    nTmpAdd = nTmpAdd + nPSAMIDBCDLength;

    ASC2BCD( gpstSharedInfo->abSubPSAMID[1],
             stSendUsrPktMsg->pbRealSendRecvData + nTmpAdd,
             nPSAMIDLength );           // sub terminal 2  SAM ID
    nTmpAdd = nTmpAdd + nPSAMIDBCDLength;

    ASC2BCD( gpstSharedInfo->abSubPSAMID[2],
             stSendUsrPktMsg->pbRealSendRecvData + nTmpAdd,
             nPSAMIDLength );           // sub terminal 3  SAM ID
    nTmpAdd = nTmpAdd + nPSAMIDBCDLength;

    memcpy( abTmp, CHILD_DEFAULT_ID, 10 );  // driver operator ID
    ASC2BCD( abTmp, stSendUsrPktMsg->pbRealSendRecvData + nTmpAdd, 10 );
    nTmpAdd = nTmpAdd + ( 10 / 2 );

    memcpy( abTmp, gpstSharedInfo->abSubTermID[0], nTermIDLength );
                                        // subterminal 1  ID
    abTmp[nTermIDLength] = 0x30;
    ASC2BCD( abTmp, stSendUsrPktMsg->pbRealSendRecvData + nTmpAdd, 10 );
    nTmpAdd = nTmpAdd + ( 10 / 2 );

    memcpy( abTmp, gpstSharedInfo->abSubTermID[1], nTermIDLength );
                                        // subterminal 2  ID
    abTmp[nTermIDLength] = 0x30;
    ASC2BCD( abTmp, stSendUsrPktMsg->pbRealSendRecvData + nTmpAdd, 10 );
    nTmpAdd = nTmpAdd + ( 10 / 2 );

    memcpy( abTmp, gpstSharedInfo->abSubTermID[2], nTermIDLength );
                                        // subterminal 3  ID
    abTmp[nTermIDLength] = 0x30;
    ASC2BCD( abTmp, stSendUsrPktMsg->pbRealSendRecvData + nTmpAdd, 10 );
    nTmpAdd = nTmpAdd + ( 10 / 2 );

    memcpy( abTmp, CHILD_DEFAULT_ID, 10 );  // receipt Printer ID
    memcpy ( abTmp, MAIN_RELEASE_VER, 4 );
    ASC2BCD( abTmp, stSendUsrPktMsg->pbRealSendRecvData + nTmpAdd, 10 );
    nTmpAdd = nTmpAdd + ( 10 / 2 );

    memcpy( abTmp, CHILD_DEFAULT_ID, 10 );  // GPS  ID
    ASC2BCD( abTmp, stSendUsrPktMsg->pbRealSendRecvData + nTmpAdd, 10 );
    nTmpAdd = nTmpAdd + ( 10 / 2 );

    GetRTCTime( &tCurrDtime );
    TimeT2ASCDtime( tCurrDtime, stSendUsrPktMsg->pbRealSendRecvData + nTmpAdd );
    nTmpAdd = nTmpAdd + 14;

    stSendUsrPktMsg->lDataSize = nTmpAdd;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      ReqDCSAuth                                               *
*                                                                              *
*  DESCRIPTION :      집계시스템으로 인증 요청을 한다.                           *
*                                                                              *
*  INPUT PARAMETERS:  int fdSock                                               *
*                     USER_PKT_MSG* stSendUsrPktMsg                            *
*                     USER_PKT_MSG* stRecvUsrPktMsg                            *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       ERR_DCS_AUTH                                           *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short ReqDCSAuth( int fdSock,
		                 USER_PKT_MSG* pstSendUsrPktMsg,
		                 USER_PKT_MSG* pstRecvUsrPktMsg )
{
	short sResult = SUCCESS;

    /*
     *  인증 요청 정보 세팅(ENQ)
     */
    pstSendUsrPktMsg->chEncryptionYN = ENCRYPTION;       // 암호화 여부
    memcpy( pstSendUsrPktMsg->achConnCmd,
            DCS_AUTH_CMD,
            sizeof( pstSendUsrPktMsg->achConnCmd ) );
    pstSendUsrPktMsg->lDataSize = 0;

	sResult = SendNRecvUsrPkt2DCS( fdSock, pstSendUsrPktMsg, pstRecvUsrPktMsg );
    if ( sResult != SUCCESS )
    {
    	printf( "[ReqDCSAuth] 'ENQ' 메시지 송신 & 'AA' 메시지 수신 실패\n" );
        return ErrRet( ERR_DCS_AUTH );
    }


    /*
     *  "AA"메세지 인지 COMMAND 비교
     */
    if ( memcmp( pstRecvUsrPktMsg->achConnCmd,
                 DCS_AUTH_CMD1,
                 sizeof( pstSendUsrPktMsg->achConnCmd )
               ) != 0 )
    {
    	printf( "[ReqDCSAuth] 'AA' 메시지 아님\n" );
        return ErrRet( ERR_DCS_AUTH );
    }

    return SUCCESS;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      GetAuthIndex                                             *
*                                                                              *
*  DESCRIPTION :      인증 인덱스 구하기                                        *
*                                                                              *
*  INPUT PARAMETERS:  char* pchRandomNo                                        *
*                                                                              *
*  RETURN/EXIT VALUE:     nAuthIdx                                             *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short GetAuthIndex( char* pchRandomNo )
{
    int i;
    int j;

    char achHexTable[17] = {"0123456789ABCDEF"};

    int nAuthIdx;
    int nHexTotal = 0;


    for ( i = 0 ; i < RANDOM_NO_LENGTH ; i++ )
    {
        for ( j = 0 ; j < 16 ; j++ )
        {
            if ( pchRandomNo[i] == achHexTable[j] )
            {
                nHexTotal += j;
                break;
            }
        }
    }

    nAuthIdx = nHexTotal % 10;
    return nAuthIdx;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SendNRecvUsrPkt2DCS                                      *
*                                                                              *
*  DESCRIPTION :      패킷을 송신 및 수신 한다.                                 *
*                                                                              *
*  INPUT PARAMETERS:  int fdSock                                               *
*                     USER_PKT_MSG* stSendUsrPktMsg                            *
*                     USER_PKT_MSG* stRecvUsrPktMsg                            *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       ERR_SOCKET_SEND                                        *
*                       ERR_PACKET_STX                                         *
*                       ERR_PACKET_ETX                                         *
*                       ERR_PACKET_BCC                                         *
*                       ERR_SOCKET_TIMEOUT                                     *
*                       ERR_SOCKET_RECV                                        *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short SendNRecvUsrPkt2DCS( int fdSock,
                           USER_PKT_MSG* stSendUsrPktMsg,
                           USER_PKT_MSG* stRecvUsrPktMsg )
{
    short sRetVal = SUCCESS;

    sRetVal = SendUsrPkt2DCS( fdSock, TIMEOUT, MAX_RETRY_COUNT,
		stSendUsrPktMsg );
    if ( sRetVal != SUCCESS )
    {
        return -1;
    }

    sRetVal = RecvUsrPktFromDCS( fdSock, TIMEOUT, MAX_RETRY_COUNT,
		stRecvUsrPktMsg );
    if ( sRetVal != SUCCESS )
    {
        return -1;
    }

    return SUCCESS;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      ReSetupTerm                                              *
*                                                                              *
*  DESCRIPTION :      단말기를 재설치하는 함수로 tc_leap, setup, 고정 BLPL, 운전 *
*                     자 조작기 플래그 파일을 제외한 모든 파일을 삭제하고 집계로  *
*                     새롭게 파일을 다운로드 받는다.                             *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE:     SUCCESS                                              *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short ReSetupTerm( void )
{
	short       sReturnVal          = -1;
	byte i = 0;
	char        achCmdData[6]       = { 0, };
	bool        boolIsApplyOperParm = FALSE;    // 운영파라미터 다운여부 세팅
	VER_INFO    stTmpVerInfo;
	time_t tStartDtime = 0;
	time_t tEndDtime = 0;

	time( &tStartDtime );
	printf( "[ReSetupTerm] ReSetupTerm 시작 ------------------------------------------------\n" );

    /*
     *  tc_leap.dat, setup.dat, 고정 BLPL파일, 운전자 조작기 플래그 파일을
     *  제외한 모든 파일을 삭제한다.
     */
    DelBackupFile();
    DelFileOnReset();

    InitNextVerApplFlag();  // 미래버전적용여부 초기화

	/*
	 *  운영파라미터 파일 수신
	 */
	for ( i = 0; i < 3; i++ )
    {
    	if ( ConnDCS() == TRUE )
    	{
    		break;
    	}
	}
	if ( i >= 3 )
	{
		printf( "[ReSetupTerm] ConnDCS() 실패\n" );
		time( &tEndDtime );
		printf( "[ReSetupTerm] 실행시간 : %d sec\n", (int)( tEndDtime - tStartDtime ) );
		printf( "[ReSetupTerm] ReSetupTerm 종료 ------------------------------------------------\n" );
		return -1;
	}

	for ( i = 0; i < 3; i++ )
    {
    	sReturnVal = DownVehicleParmFile();	// 차량정보파일 수신
		if ( sReturnVal == SUCCESS )
		{
			break;
		}
	}
	if ( i >= 3 )
	{
		printf( "[ReSetupTerm] DownVehicleParmFile() 실패\n" );
		time( &tEndDtime );
		printf( "[ReSetupTerm] 실행시간 : %d sec\n", (int)( tEndDtime - tStartDtime ) );
		printf( "[ReSetupTerm] ReSetupTerm 종료 ------------------------------------------------\n" );
		return -1;
	}

	LoadVehicleParm();     // 운행 파라미터 정보 로드

    CreateInstallInfoFile();            // 인스톨 파일 생성

    InitVerInfo();                      // 버전정보 초기화
    LoadVerInfo( &stTmpVerInfo );       // 버전정보 로드

    sReturnVal = DownFromDCS();			// 집계시스템으로 파일 다운로드
    if ( sReturnVal != SUCCESS )
    {
    	LogDCS("[ReSetupTerm] DownFromDCS() 실패\n");
    	printf("[ReSetupTerm] DownFromDCS() 실패\n");
    }
    ApplyNextVer();                     // 미래적용버전 적용

    if ( SetUploadVerInfo() < 0 )       // 집계용 버전정보 생성
    {
        DebugOut( "\r\n SetUploadVerInfo Fail[%d] \r\n", sReturnVal );
        LogWrite( ERR_SET_UPLOAD_VER );
    }

    /*
     *  재설치결과를 세팅
     *  [0] - re-apply request
     *  [1] - parameter Loadrequest
     *  [2] - file Load request
     *  [3] - voice file Load
     *  [4] - driver oper. Load
     *  [5] - operator parameter
     */
    achCmdData[0] = GetASCNoFromDEC( SUCCESS );
    achCmdData[1] = GetASCNoFromDEC( boolIsApplyNextVerParm );
    achCmdData[2] = GetASCNoFromDEC( boolIsApplyNextVerAppl );
    achCmdData[3] = GetASCNoFromDEC( boolIsApplyNextVerVoice );
    achCmdData[4] = GetASCNoFromDEC( boolIsApplyNextVerDriverAppl );
    achCmdData[5] = GetASCNoFromDEC( boolIsApplyOperParm );

    SetSharedDataforCmd( CMD_RESETUP, achCmdData, sizeof( achCmdData ) );

    InitNextVerApplFlag();  // 미래버전적용여부 초기화

	ctrl_event_info_write( EVENT_RESETUP_TERM );

	time( &tEndDtime );
	printf( "[ReSetupTerm] 실행시간 : %d sec\n", (int)( tEndDtime - tStartDtime ) );
	printf( "[ReSetupTerm] ReSetupTerm 종료 ------------------------------------------------\n" );

    return SUCCESS;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SetupTerm                                                *
*                                                                              *
*  DESCRIPTION :      단말기 부팅시 수행되며 운영파라미터 파일을 체크하여 없으면  *
*                       다운로드하고 인스톨파일과 버전파일을 생성하고             *
*                       집계 업로드와 다운로드를 수행 및 keyset 등록 요청을 한다  *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE:     SUCCESS                                              *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short SetupTerm( void )
{
    short       sReturnVal          = SUCCESS;
    short       sInstallFileExistYN = SUCCESS;
    char        achCmdData[5]       = { 0, };
    VER_INFO    stTmpVerInfo;
	byte i = 0;
	time_t tStartDtime = 0;
	time_t tEndDtime = 0;

	time( &tStartDtime );
	printf( "[SetupTerm] SetupTerm 시작 ----------------------------------------------------\n" );

	DelBackupFile();							// backup파일 제거

	InitIP( NETWORK_DEVICE );
	for ( i = 0; i < 3; i++ )
    {
    	if ( ConnDCS() == TRUE )
    	{
    		break;
    	}
	}
	if ( i >= 3 )
	{
		printf( "[SetupTerm] ConnDCS() 실패\n" );
	}

	sReturnVal = ChkNDownOperParmFile();		// 차량정보 체크 및 다운로드

    sReturnVal = LoadVehicleParm();				// 차량정보 로드
    if ( sReturnVal < SUCCESS )
    {
        /*
         *  공유메모리에 작업 실패 세팅
         */
        achCmdData[0] = CMD_FAIL_RES;
        SetSharedDataforCmd( CMD_SETUP, achCmdData, sizeof( achCmdData )  );

		printf( "[SetupTerm] 차량정보 로드 실패\n" );
		time( &tEndDtime );
		printf( "[SetupTerm] 실행시간 : %d sec\n", (int)( tEndDtime - tStartDtime ) );
		printf( "[SetupTerm] SetupTerm 종료 ----------------------------------------------------\n" );

        return sReturnVal;
    }

    /*
     *  인스톨파일이 존재하는지 체크
     *  존재한다면 이전 부팅시 집계와 통신이 안 된것으로 간주하여
     *  버전정보를 다시 체크한다.
     */
    sInstallFileExistYN = access( INSTALL_INFO_FILE, F_OK );
    CreateInstallInfoFile();				// 인스톨파일 생성

	sReturnVal = Upload2DCS();				// 집계시스템으로 파일 업로드
	if ( sReturnVal != SUCCESS )
	{
		LogDCS( "[SetupTerm] Upload2DCS() 실패\n" );
		printf( "[SetupTerm] Upload2DCS() 실패\n" );
	}

    if ( sInstallFileExistYN == SUCCESS )
    {
        /*
         *  버전정보 로드시 BLPL 파일이 존재여부 및 이어받기정보를 세팅한다.
         */
        LoadVerInfo( &stTmpVerInfo );
        SaveVerFile();
    }

    sReturnVal = DownFromDCS();			// 집계시스템으로 파일 다운로드
    if ( sReturnVal != SUCCESS )
    {
    	LogDCS("[SetupTerm] DownFromDCS() 실패\n");
    	printf("[SetupTerm] DownFromDCS() 실패\n");
    }
    ApplyNextVer();						// 미래적용버전 적용

    if ( boolIsApplyNextVer == TRUE )	//미래적용버전이 적용이 되었다면
    {
        if ( SetUploadVerInfo() < 0 )	// 집계송신용 버전정보를 생성
        {
            printf("[SetupTerm] SetUploadVerInfo() 실패\n");
        }
    }

    /*
     *  셋업결과를 세팅해준다.
     *  [0] - re-apply request
     *  [1] - parameter Loadrequest
     *  [2] - file Load request
     *  [3] - voice file Load
     *  [4] - driver oper. Load
     */
    achCmdData[0] = GetASCNoFromDEC( SUCCESS );
    achCmdData[1] = GetASCNoFromDEC( boolIsApplyNextVerParm );
    achCmdData[2] = GetASCNoFromDEC( boolIsApplyNextVerAppl );
    achCmdData[3] = GetASCNoFromDEC( boolIsApplyNextVerVoice );
    achCmdData[4] = GetASCNoFromDEC( boolIsApplyNextVerDriverAppl );
    SetSharedDataforCmd( CMD_SETUP, achCmdData, sizeof( achCmdData ) );

	printf( "[SetupTerm] 운영정보     적용여부 : %s\n", GetBoolString( boolIsApplyNextVerParm ) );
	printf( "[SetupTerm] 프로그램     적용여부 : %s\n", GetBoolString( boolIsApplyNextVerAppl ) );
	printf( "[SetupTerm] 음성파일     적용여부 : %s\n", GetBoolString( boolIsApplyNextVerVoice ) );
	printf( "[SetupTerm] 운전자조작기 적용여부 : %s\n", GetBoolString( boolIsApplyNextVerDriverAppl ) );
	
    InitNextVerApplFlag();  // 미래버전적용여부 초기화
    ReqKeysetRegist();      // keyset 등록요청

	time( &tEndDtime );
	printf( "[SetupTerm] 실행시간 : %d sec\n", (int)( tEndDtime - tStartDtime ) );
	printf( "[SetupTerm] SetupTerm 종료 ----------------------------------------------------\n" );

    return SUCCESS;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      ChkNDownOperParmFile                                     *
*                                                                              *
*  DESCRIPTION :      통신연결을 하여 집계로부터 운영파라미터 파일을 다운로드 받고*
*                       실패시 실패를 등록한다.                                 *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE:     SUCCESS                                              *
*						ERR_DCS_SETUP										   *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short ChkNDownOperParmFile( void )
{
    short sResult = SUCCESS;

	if ( IsExistFile( VEHICLE_PARM_FILE ) == FALSE )
	{
		printf( "[ChkNDownOperParmFile] 차량정보파일이 존재하지 않아 다운로드함\n" );
		sResult = DownVehicleParmFile();		// 차량정보파일 수신
        if ( sResult != SUCCESS )
        {
			printf( "[ChkNDownOperParmFile] DownVehicleParmFile() 실패\n" );
            return ERR_DCS_SETUP;
        }
    }

    return SUCCESS;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      InitNextVerApplFlag                                      *
*                                                                              *
*  DESCRIPTION :      미래버전 적용여부 플래그 초기화                            *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
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
static void InitNextVerApplFlag( void )
{

   /*
    *   미래버전 적용여부 초기화
    */
    boolIsApplyNextVer              = FALSE;
    boolIsApplyNextVerParm          = FALSE;
    boolIsApplyNextVerVoice         = FALSE;
    boolIsApplyNextVerAppl          = FALSE;
    boolIsApplyNextVerDriverAppl    = FALSE;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      DelBackupFile                                            *
*                                                                              *
*  DESCRIPTION :      tc_leap, setup, 고정 BLPL파일과 운전자조작기프로그램 적용  *
*                       플래그파일의 백업 파일을 삭제한다.                       *
*                       재설치시에 파일을 백업하는데 재설치 중 전원을 끌 경우를   *
*                       대비                                                   *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
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
static void DelBackupFile( void )
{

    /*
     *  재설치가 실패했을 경우 backup 파일이 존재함
     *
     */
//    unlink( TC_LEAP_BACKUP_FILE );
//    unlink( SETUP_BACKUP_FILE );
    unlink( MASTER_BL_BACKUP_FILE );
    unlink( MASTER_PREPAY_PL__BACKUP_FILE );
    unlink( MASTER_POSTPAY_PL__BACKUP_FILE );
    unlink( MASTER_AI_BACKUP_FILE );
//    unlink( KPDAPPLY_FLAG_BACKUP_FILE );
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      DelFileOnReset                                           *
*                                                                              *
*  DESCRIPTION :      tc_leap, setup, 고정 BLPL, 운전자 조작기 플래그 파일을     *
*                       제외한 모든 파일을 삭제                                 *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
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
static void DelFileOnReset( void )
{

    rename( TC_LEAP_FILE,           TC_LEAP_BACKUP_FILE );
    rename( SETUP_FILE,             SETUP_BACKUP_FILE );
    rename( MASTER_BL_FILE,         MASTER_BL_BACKUP_FILE );
    rename( MASTER_PREPAY_PL_FILE,  MASTER_PREPAY_PL__BACKUP_FILE );
    rename( MASTER_POSTPAY_PL_FILE, MASTER_POSTPAY_PL__BACKUP_FILE );
    rename( MASTER_AI_FILE,         MASTER_AI_BACKUP_FILE );
    rename( KPDAPPLY_FLAGFILE,      KPDAPPLY_FLAG_BACKUP_FILE );

    system( "rm *.dat" );
    system( "rm *.trn" );
    system( "rm *.tmp" );
    system( "rm *.flg" );
    system( "rm 2* " );
    system( "rm *.cfg " );

    rename( TC_LEAP_BACKUP_FILE,            TC_LEAP_FILE );
    rename( SETUP_BACKUP_FILE,              SETUP_FILE );
    rename( MASTER_BL_BACKUP_FILE,          MASTER_BL_FILE );
    rename( MASTER_PREPAY_PL__BACKUP_FILE,  MASTER_PREPAY_PL_FILE );
    rename( MASTER_POSTPAY_PL__BACKUP_FILE, MASTER_POSTPAY_PL_FILE );
    rename( MASTER_AI_BACKUP_FILE,          MASTER_AI_FILE );
    rename( KPDAPPLY_FLAG_BACKUP_FILE,      KPDAPPLY_FLAGFILE );
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      ReqKeysetRegist                                          *
*                                                                              *
*  DESCRIPTION :      메인 프로세스에 keyset등록을 요청하고 처리 실패시에 SAM관련 *
*                       버전정보를 초기화해준다.                                *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE:     SUCCESS                                              *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static void ReqKeysetRegist( void )
{
    short       sReturnVal 		= SUCCESS;
    byte        SharedMemoryCmd;
    char        achCmdData[5]       = { 0, };
    word        wCmdDataSize        = 0;

    /*
     *  keyset 등록 요청
     */
    achCmdData[0] = CMD_REQUEST;
    sReturnVal = SetSharedCmdnDataLooping( CMD_KEYSET_IDCENTER_WRITE,
                                           &achCmdData[0],
                                           sizeof( achCmdData[0] ) );

    usleep( 500000 );

    while( ( achCmdData[0] != CMD_FAIL_RES ) &&
           ( achCmdData[0] != CMD_SUCCESS_RES ) )
    {

        GetSharedCmd( &SharedMemoryCmd, achCmdData, &wCmdDataSize );
        DebugOut( "수행결과는?=>[%d]", achCmdData[0] );

        /*
         *  처리 완료의 경우
         */
        if( ( SharedMemoryCmd == CMD_KEYSET_IDCENTER_WRITE ) &&
            ( achCmdData[0] == CMD_FAIL_RES ||
              achCmdData[0] == CMD_SUCCESS_RES ) )
        {
            /*
             *  요청정보 clear
             */
            ClearSharedCmdnData  ( CMD_KEYSET_IDCENTER_WRITE );

            if( achCmdData[0] == CMD_FAIL_RES )
            {
                DebugOut( "K command 수행 실패 \n" );
                boolIsRegistMainTermPSAM = FALSE;

                /*
                 *  SAM관련 버전정보를 초기화하여 집계통신시 SAM관련 정보를 다시
                 *  수신 받는다
                 */
                InitSAMRegistInfoVer( );
            }
            else if( achCmdData[0] == CMD_SUCCESS_RES )
            {
                DebugOut( "K command 수행 성공 \n" );
                boolIsRegistMainTermPSAM = TRUE;
            }

            break;
        }
        else if ( SharedMemoryCmd != CMD_KEYSET_IDCENTER_WRITE )
        {
            break;
        }

        usleep( 500000 );

    }
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SendRS                                                   *
*                                                                              *
*  DESCRIPTION :      집계시스템으로 Response를 송신                            *
*                                                                              *
*  INPUT PARAMETERS:    int fdSock                                             *
*                       USER_PKT_MSG* pstSendUsrPktMsg                         *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       ERR_SOCKET_SEND                                        *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short SendRS( int fdSock, USER_PKT_MSG* pstSendUsrPktMsg )
{

    RS_COMMAND *pRsCommand = (RS_COMMAND *)pstSendUsrPktMsg->pbRealSendRecvData;

    pstSendUsrPktMsg->chEncryptionYN = NOT_ENCRYPTION;
    memcpy( pstSendUsrPktMsg->achConnCmd, RESPONSE_CMD, COMMAND_LENGTH );

    memcpy( pRsCommand->abMainTermID,
            gpstSharedInfo->abMainTermID,
            nTermIDLength );
    memcpy( pRsCommand->abRecvSeqNo,
            abRecvSeqNo,
            sizeof( pRsCommand->abRecvSeqNo ) );
    memcpy( pRsCommand->abRecvStatusCode,
            RESPONSE_STATUS_MSG,
            sizeof( pRsCommand->abRecvStatusCode ) );
    memset( pRsCommand->abRecvStatusMsg,
            SPACE,
            sizeof( pRsCommand->abRecvStatusMsg ) );

    pstSendUsrPktMsg->lDataSize = sizeof( RS_COMMAND );


    if ( SendUsrPkt2DCS( fdSock, TIMEOUT, MAX_RETRY_COUNT, pstSendUsrPktMsg )
                       != SUCCESS )
    {
        return ErrRet( ERR_SOCKET_RS_SEND );
    }

    return SUCCESS;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SendEOS                                                  *
*                                                                              *
*  DESCRIPTION :      집계시스템으로 EOS를 송신                                 *
*                                                                              *
*  INPUT PARAMETERS:    int fdSock                                             *
*                       USER_PKT_MSG* pstSendUsrPktMsg                         *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       ERR_SOCKET_SEND                                        *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short SendEOS( int fdSock, USER_PKT_MSG* pstSendUsrPktMsg )
{

    pstSendUsrPktMsg->chEncryptionYN = NOT_ENCRYPTION;
    memcpy( pstSendUsrPktMsg->achConnCmd, EOS_CMD, COMMAND_LENGTH );
    pstSendUsrPktMsg->lDataSize = 0;

    if ( SendUsrPkt2DCS( fdSock, TIMEOUT, MAX_RETRY_COUNT, pstSendUsrPktMsg )
         != SUCCESS )
    {
        return ErrRet( ERR_SOCKET_SEND );
    }

    return SUCCESS;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SendEOF                                                  *
*                                                                              *
*  DESCRIPTION :      집계시스템으로 EOF를 송신                                 *
*                                                                              *
*  INPUT PARAMETERS:    int fdSock                                             *
*                       USER_PKT_MSG* pstSendUsrPktMsg                         *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       ERR_SOCKET_SEND                                        *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short SendEOF( int fdSock, USER_PKT_MSG* pstSendUsrPktMsg )
{

    pstSendUsrPktMsg->chEncryptionYN = NOT_ENCRYPTION;
    pstSendUsrPktMsg->lDataSize = 0;
    memcpy( pstSendUsrPktMsg->achConnCmd, EOF_CMD, COMMAND_LENGTH );

    if ( SendUsrPkt2DCS( fdSock, TIMEOUT, MAX_RETRY_COUNT, pstSendUsrPktMsg )
         != SUCCESS )
    {
        return ErrRet( ERR_SOCKET_SEND );
    }

    return SUCCESS;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      RecvRS                                                   *
*                                                                              *
*  DESCRIPTION :      집계시스템으로 Response를 수신                            *
*                                                                              *
*  INPUT PARAMETERS:    int fdSock                                             *
*                       USER_PKT_MSG* pstRecvUsrPktMsg                         *
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
short RecvRS( int fdSock, USER_PKT_MSG* pstRecvUsrPktMsg )
{
    short   sReturnVal  = SUCCESS;

    sReturnVal = RecvUsrPktFromDCS( fdSock,
                                    TIMEOUT,
                                    MAX_RETRY_COUNT,
                                    pstRecvUsrPktMsg );

    if ( sReturnVal == SUCCESS )
    {
        if ( memcmp( pstRecvUsrPktMsg ->achConnCmd, RESPONSE_CMD, COMMAND_LENGTH
                   ) != SUCCESS )
        {
            sReturnVal = ERR_SOCKET_RS_RECV;
        }
    }
    else
    {
        sReturnVal = ERR_SOCKET_RS_RECV;
    }

    return ErrRet( sReturnVal );
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      RecvACK                                                  *
*                                                                              *
*  DESCRIPTION :      집계시스템으로 ACK를 수신                                 *
*                                                                              *
*  INPUT PARAMETERS:    int fdSock                                             *
*                       USER_PKT_MSG* pstRecvUsrPktMsg                         *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       ERR_SOCKET_ACK_RECV                                    *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS : 0334버전 이후부터 ACK를 수신 받는다.                               *
*                                                                              *
*******************************************************************************/
short RecvACK( int fdSock, USER_PKT_MSG* pstRecvUsrPktMsg )
{
    short   sReturnVal  = SUCCESS;

    /*
     *  0334버전 이후부터 ACK를 수신 받는다.
     */
    if ( ( sReturnVal = RecvUsrPktFromDCS( fdSock,
                                           TIMEOUT,
                                           MIN_RETRY_COUNT,
                                           pstRecvUsrPktMsg )
         ) == SUCCESS )
    {
        if ( memcmp( pstRecvUsrPktMsg ->achConnCmd, ACK_CMD, COMMAND_LENGTH
                   ) != 0 )
        {
            sReturnVal = ERR_SOCKET_ACK_RECV;
        }
    }
    else
    {
        sReturnVal = ERR_SOCKET_ACK_RECV;
    }

    return ErrRet( sReturnVal );
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      ConnDCS                                                  *
*                                                                              *
*  DESCRIPTION :      집계시스템에 LEAP 인증을 하고 IP를 얻어온다.               *
*                                                                              *
*  INPUT PARAMETERS:  void                                                     *
*                                                                              *
*  RETURN/EXIT VALUE:     boolIsCommAble - 통신가능 여부                        *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static bool ConnDCS( void )
{
	byte i = 0;
	int nResult = 0;

	for ( i = 0; i < 3; i++ )
	{
		if ( i != 0 )
		{
			printf( "[ConnDCS] LEAPMain() 재시도\n" );
		}
		nResult = LEAPMain();
		if ( nResult >= 0 )
		{
			break;
		}
	}

	if ( nResult < 0 )
	{
    	printf( "[ConnDCS] LEAPMain() 실패\n" );
		return FALSE;
	}

	for ( i = 0; i < 3; i++ )
	{
		if ( i != 0 )
		{
			printf( "[ConnDCS] SetLocalIP() 재시도\n" );
		}
		nResult = SetLocalIP();
		if ( nResult >= 0 )
		{
			break;
		}
	}

	if ( nResult < 0 )
	{
    	printf( "[ConnDCS] SetLocalIP() 실패\n" );
		return FALSE;
	}

	return TRUE;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      DCSComm                                                  *
*                                                                              *
*  DESCRIPTION :      집계시스템에 업로드와 다운로드 및 파일 적용의 기능을 수행함 *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
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
void *DCSComm( void *arg )
{
	short sResult = SUCCESS;
	byte i = 0;
	time_t tStartDtime = 0;
	time_t tEndDtime = 0;
	bool boolIsConnected = FALSE;

	time( &tStartDtime );
	printf( "\n" );
    printf( "[DCSComm] 집계통신 Thread 시작 ////////////////////////////////////////////////\n" );
	LogDCS( "[집계통신] Thread 시작\n" );

    /*
     *  미래적용 버전 적용
     */
    ApplyNextVer();

    /*
     *  집계통신 성공횟수 FND에 디스플레이
     */
    DisplayDCSCommCnt( nDCSCommSuccCnt );

    /*
     *  집계시스템에 연결하여 업로드와 다운로드를 수행한다.
     */
	boolIsConnected = ConnDCS();
	if ( boolIsConnected == FALSE )
	{
		LogDCS("[DCSComm] ConnDCS() 실패\n");
		printf("[DCSComm] ConnDCS() 실패\n");
	}

    if ( boolIsConnected == TRUE )
    {
        /*
         *  업로드 수행
         */
		for ( i = 0; i < 3; i++ )
	    {
	    	sResult = Upload2DCS();	// 차량정보파일 수신
			if ( sResult == SUCCESS )
			{
				break;
			}
		}
		if ( sResult != SUCCESS )
		{
			LogDCS("[DCSComm] Upload2DCS() 실패\n");
			printf("[DCSComm] Upload2DCS() 실패\n");
		}

        /*
         *  집계통신 성공횟수 FND에 디스플레이
         */
        DisplayDCSCommCnt( nDCSCommSuccCnt );

        /*
         *  다운로드 수행
         */
		sResult = DownFromDCS();
		if ( sResult == SUCCESS )
        {
            /*
             *  집계통신 성공횟수 증가
             */
            nDCSCommSuccCnt++;
        }
		else
		{
			LogDCS("[DCSComm] DownFromDCS() 실패\n");
			printf("[DCSComm] DownFromDCS() 실패\n");
		}
		

        /*
         *  미래적용 버전 적용
         */
        ApplyNextVer();        // next -> curr

    }

    if ( boolIsApplyNextVer == TRUE )
    {
        ReqApplyNextVer( CMD_NEW_CONF_IMG,
                         boolIsApplyNextVer,
                         boolIsApplyNextVerParm,
                         boolIsApplyNextVerAppl,
                         boolIsApplyNextVerVoice,
                         boolIsApplyNextVerDriverAppl );
    }

    /*
     *  통신프로세스에서도 운영 파라미터 정보를 load한다.
     */
    LoadVehicleParm();

    /*
     *  TermComm에서 집계 Thread 핸들링 용으로 세팅
     */
    boolIsDCSThreadComplete = TRUE;
    boolIsDCSThreadStartEnable = TRUE;
	
	time( &tEndDtime );
	printf( "[DCSComm] 실행시간 : %d sec\n", (int)( tEndDtime - tStartDtime ) );
	printf( "[DCSComm] 집계통신 Thread 종료 ////////////////////////////////////////////////\n" );
	printf( "\n" );
	LogDCS( "[집계통신] Thread 종료\n" );

	TimerStart( &gtDCSCommStart );

    return (void*)NULL;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      ReqApplyNextVer                                          *
*                                                                              *
*  DESCRIPTION :      공유메모리에 미래적용버전 적용을 요청한다.               *
*                                                                              *
*  INPUT PARAMETERS:  byte bCmd,                                               *
*                     boolIsApplyNextVer,                                      *
*                     boolIsApplyNextVerParm,                                  *
*                     boolIsApplyNextVerAppl,                                  *
*                     boolIsApplyNextVerVoice,                                 *
*                     boolIsApplyNextVerDriverAppl                             *
*                                                                              *
*  RETURN/EXIT VALUE:     void                                                 *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2006-03-16                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static void ReqApplyNextVer( byte bCmd,
                      bool boolIsApplyNextVer,
                      bool boolIsApplyNextVerParm,
                      bool boolIsApplyNextVerAppl,
                      bool boolIsApplyNextVerVoice,
                      bool boolIsApplyNextVerDriverAppl)
{

    char    achCmdData[5]   = { 0, };
    word   wCmdDataSize;

    /*
     *  ASCII 값으로 세팅을 해줌
     */
    achCmdData[0] = boolIsApplyNextVer + 0x30;        // re-apply request
    achCmdData[1] = boolIsApplyNextVerParm + 0x30;    // parameter Loadrequest
    achCmdData[2] = boolIsApplyNextVerAppl + 0x30;    // file Load request
    achCmdData[3] = boolIsApplyNextVerVoice + 0x30;   // voice file Load request
    achCmdData[4] = boolIsApplyNextVerDriverAppl + 0x30;// driver operator Load

    /*
     *  공유메모리에 적용되는 정보를 세팅해준다.
     */
    SetSharedCmdnDataLooping( bCmd,
                              achCmdData,
                              sizeof( achCmdData ) );

    /*
     *  메인에서 처리 완료하면 세팅한 정보를 clear해준다.
     */
    while( TRUE )
    {
        /*
         *  처리완료했는지 체크
         */
        GetSharedCmd( &bCmd, achCmdData, &wCmdDataSize );

        /*
         *  처리 성공
         */
        if ( ( bCmd  == bCmd ) && ( achCmdData[0] == CMD_SUCCESS_RES ) )
        {
            /*
             *  요청정보 clear
             */
            ClearSharedCmdnData( CMD_NEW_CONF_IMG );

            /*
             *  적용 flag값 초기화
             */
            boolIsApplyNextVer              = FALSE;
            boolIsApplyNextVerParm          = FALSE;
            boolIsApplyNextVerVoice         = FALSE;
            boolIsApplyNextVerAppl          = FALSE;
            boolIsApplyNextVerDriverAppl    = FALSE;

            break;

        }
        /*
         *  처리 실패
         */
        else if ( ( bCmd  == CMD_NEW_CONF_IMG ) &&
                  ( achCmdData[0] == CMD_FAIL_RES ) )
        {
            /*
             *  요청정보 clear
             */
            ClearSharedCmdnData( CMD_NEW_CONF_IMG );

            break;
        }

        usleep( 500000 );
    }
}
