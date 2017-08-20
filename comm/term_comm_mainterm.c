/*******************************************************************************
*                                                                              *
*                      KSCC - Bus Embeded System                               *
*                                                                              *
*            All rights reserved-> No part of this publication may be          *
*            reproduced, stored in a retrieval system or transmitted           *
*            in any form or by any means  -  electronic, mechanical,           *
*            photocopying, recording, or otherwise, without the prior          *
*            written permission of LG CNS.                                     *
*                                                                              *
********************************************************************************
*                                                                              *
*  PROGRAM ID :       term_comm_mainterm.c                                     *
*                                                                              *
*  DESCRIPTION:       승차기에서  통신을 담당하는 프로그램으로 승하차간 실행할 *
*                     명령어를 하차기로 전송하며 집계통신을 위한 쓰레드를 생성 *
*                     하여 집계통신을 실행한다.				 				   *
*                                                                              *
*  ENTRY POINT:       MainTermProc()                                           *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: 0 - Success                                              *
*                     (-) - Error                                              *
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
* ---------- ----------------------------------------------------------------- *
* 2005/09/27 Solution Team  woolim        Initial Release                      *
* 2006/04/17 F/W Dev. Team  wangura       파일분리 및 추가 구조화              *
*                                                                              *
*******************************************************************************/

/*******************************************************************************
*  Declaration of Header Files                                                 *
*******************************************************************************/
#include "dcs_comm.h"
#include "term_comm.h"
#include "term_comm_mainterm.h"
#include "../proc/main.h"
#include "../proc/load_parameter_file_mgt.h"
#include "../proc/version_mgt.h"
#include "../proc/write_trans_data.h"
#include "../proc/blpl_proc.h"
#include "../system/bus_type.h"
#include "../system/device_interface.h"
#include "../proc/card_proc_util.h"
#include "../proc/main_process_busTerm.h"

/*******************************************************************************
*  Definition of Macro                                                         *
*******************************************************************************/

/* 
 * 하차기개수별 집계통신 및 CMD실행주기( LoopCnt) - 3분 마다 접속하도록 실제
 * 실행하여 LoopCount를 설정
 * 0대 : 12500000, 1대 : 6000, 2대 :  5000, 3대 : 4000
 */
 
#define COMM_MAINSUB_DCS_ALLOW_CNT  \
	(( gbSubTermCnt == 0 )? 70000000 : \
	  (( gbSubTermCnt == 1 )? 1500 : 500 + ( 3 - gbSubTermCnt ) * 280 ))

/*******************************************************************************
*  Declaration of Global Variables inside this module                          *
*******************************************************************************/
/*******************************************************************************
*  Declaration of Global Variables for CommProc process                        *
*******************************************************************************/
/* 
 * 최초 부팅여부를 나타내는 Flag 변수 
 */	
bool boolIsBootNow = TRUE;   

/*
 * 대사는 운행종료시 1회만 실행이 필요하므로 전역변수로 기실행여부 설정 
 */
static bool boolIsDoDaeSa= FALSE;
/*
 * 운전자조작기에서 현재 운행상태(gpstSharedInfo->dwDriveChangeCnt)를 받기위한 변수
 * 예) gpstSharedInfo->dwDriveChangeCnt는 2의 배수로 증가한다.
 *     값이 초기부팅시 : 0 
 *     1회 운행 후 운행종료 - 2 
 *     2회 운행 후 종료 - 4 
 *     ....
 */
static word wDriveChangeCnt = 0; 

/* 
 * 명령어가 'V'에서 'K' 한싸이클을 모두 실행했는지 여부 
 */
static bool boolIsCmdOneCycle = FALSE;

/* 
 * BL Check Result 와 검색결과 성공실패여부 
 */
static byte bBLCheckResult;   
static short bBLCheckStatus;
	
/* 
 * PL Check Result 와 검색결과 성공실패여부 
 */
static byte bPLCheckResult;  
static short bPLCheckStatus; 
/* 
 * Card Error Log 존재 여부  
 */
static bool boolIsCardErrLog ;  
/* 
 * 에러난 card info 
 */
static TRANS_INFO stCardErrInfo;    
/*******************************************************************************
*  Declaration of Global Variables for DCS thread                              *
*******************************************************************************/
/* 
 * thread id for DCS Comm 
 */
static pthread_t nCommDCSThreadID = 0;  
/* 
 * 집계통신 쓰레드 종료여부  
 */
bool boolIsDCSThreadComplete = FALSE;  
/* 
 * 집계통신 쓰레드 현재 실행가능여부 
 */
bool boolIsDCSThreadStartEnable = TRUE;
/* 
 * 집계통신 성공횟수 
 */ 
int nDCSCommSuccCnt = 0; 			   
/*
 * 집계통신을 위한 정보셋팅 성공여부 변수 
 */    
bool boolIsGetLEAPPasswd = FALSE;  /* tc_leap.dat에서 Password, 단말기 ID Load */    
bool boolIsLoadInstallInfo = FALSE;/* 설치정보파일(SetUp.dat)에서 서버 IP, 차량 ID Load */	
/* 
 * 승차기의 PSAM에 집계로부터 받은 Keyset/idcenter등록 성공여부 
 */
bool boolIsRegistMainTermPSAM = FALSE; 

/* 
 * 집계통신 간격 조정을 위한 시간 
 */
time_t gtDCSCommStart = 0;
int nDCSCommIntv = 180;

/*******************************************************************************
*  Declaration of Function Prototype                                           *
*******************************************************************************/
static short CommMain2Sub( int nIndex );
static short CreateSendData( byte bCmd, int nDevNo );
static short CreateReqImgVer( int nDevNo );
static short CreateDownImg( int nDevNo );
static short CreateReqSubTermID( int nDevNo );
static short CreateReqVoiceVer( int nDevNo );
static short CreateDownVoice( int nDevNo );
static short CreateDownPar( int nDevNo );
static short CreateReqTDFile( int nDevNo );
static short CreateReqKeySet( int nDevNo );
static short CreatePoll( int nDevNo );
static short CreateBLResult( int nDevNo );
static short CreatePLResult( int nDevNo );
static short CreatePutSubTermID( int nDevNo );
static short SendParameterFile( int nDevNo );
static short ProcRecvData( void );
static short RecvNAKResp( void );
static short RecvACKResp( void );
static short RecvImgVer( void );
static short RecvSubTermID( void );
static short RecvVoiceVer( void );
static short RecvKeySet( void );
static short RecvBLCheckReq( void );
static short RecvPLCheckReq( void );
static short RecvTransDataOnPolling( void );
static short RecvTDFileFromSubTerm( int nDevNo, byte *pbFileName );
static short SendSubTermImgFileOldProtocol( int nDevNo, char* pchFileName );
static short CheckSubTermID( byte *pbCheckID );
static void CheckDCSCommParameterReq( bool *boolIsDCSReady, byte *pcCmdData );
short Polling2SubTerm( void );
short GetSubTermPSAMVer( void );
short GetSubTermId( void );
short ProcMainOrKpdProcessReq( void );
short ProcDaeSa( void );
short CreateDCSThread( void );	
short ProcMainSubCmdComm( void );
short ProcInitDCSComm( void );
short SendSubTermImgFile( int nDevNo, char* pchFileName );
/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       MainTermProc                                             *
*                                                                              *
*  DESCRIPTION:       단말기가 승차기인 경우 하차기와 통신을 담당하는 메인부분 *
*                     으로 승차기에서 하차기에 필요한 정보 또는 파일들을 미리  *
*					  규약된 명령어로 전송/통신을 실행한다.                    *
*                     또한 집계통신을 위한 사전정보를 설정하고 thread형태로    *
*                     집계통신을 실행한다.                                     *
*  INPUT PARAMETERS:  void                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: void                                                     *
*                                                                              *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:          														   *
*                                                                              *
*******************************************************************************/
void MainTermProc( void )
{

	bool boolIsDriveCloseYN = TRUE;
	time_t  tCurrTime;

	byte bCmd = 0;
	byte abCmdData[64] = { 0, };
	word wCmdDataSize = 0;

    LogTerm( "====MainTermProc start!!!!====\n\n" );	

    /* 
     * 집계통신을 위한 사전필요 정보 셋팅 
     */    
	ProcInitDCSComm();

	LogTerm( "====MainTermProc 1!!!!====\n" );	
	
    /* Power On한 경우 SetUp()함수내의 집계통신실행 전에 승하차기의 
     * PSAMID와 단말기ID를 최신으로 갱신하고 있어야 한다.  
     * 따라서, command v와 g를 실행한다. 
     */
    /* CMD V를 이용해 하차기 PSAMID 체크후 PSAMID가 변경되었을 경우
     * IDCENTER,KEYSET을 초기화하고 SIMID.FLG에 새로 WRITE한다 
     * 이유 - 집계통신 전에 승하차단말기의  PSAMID 정보 갱신  
     */
	GetSubTermPSAMVer();
	LogTerm( "====MainTermProc 2!!!!====\n" );	
	
	/* 
	 * CMD G를 이용해 하차기 ID를 승차기공유메모리에 LOAD한다 
	 * 이유 - 집계통신 전에 하차단말기의 id정보 갱신 
	 */
	GetSubTermId();
	LogTerm( "====MainTermProc 3!!!!====\n" );	
	

    /* 집계통신주기 초기값 설정 
     * - Boot시에 바로 집계에 접속하기 위해 초기값을 접속시작주기로 설정  
     */
    LogTerm( "하차기 개수 : [%d]\n", gbSubTermCnt );

	/* 
	 * 하차기 메인 Loop 시작 
	 */    
    while(1)
    {

		/* 
		 * 기본 폴링 
		 */
		Polling2SubTerm();
		
		/*
		 * 운행대기중 또는 부팅시 처리
		 */
        if (( boolIsBootNow == TRUE )||
            ( gpstSharedInfo->boolIsDriveNow == FALSE )) 
        {
            /*
             * 타 프로세스(메인,운전자조작기 프로세스)에서 공유메모리를 통해
             * 통신프로세스로 작업 요청시 이를 처리한다.
             */
			ProcMainOrKpdProcessReq();			
            
			/* 
			 * 집계통신을 위한 정보셋팅 실패시 
			 */
            if ( ( boolIsGetLEAPPasswd == FALSE ) ||
                 ( boolIsLoadInstallInfo == FALSE ) )
            {
                /* 
                 * 운전자 조작기의 "작업중입니다" 메세지 해제 
                 */
                gpstSharedInfo->boolIsKpdLock = FALSE; 
				/*
				 * 알람을 울리고 운행시작 가능상태 허용 
                 */
                gpstSharedInfo->boolIsReadyToDrive = TRUE; 
                continue;
            }

            /* 
             * 매 운행종료시 변수초기화 
             */
            if ( wDriveChangeCnt != gpstSharedInfo->dwDriveChangeCnt )
            {            	
                printf( " POWER ON 한경우를 제외하고는 매 운행종료시 \n\n\n" );
                wDriveChangeCnt = gpstSharedInfo->dwDriveChangeCnt;
				boolIsDriveCloseYN = TRUE;
                boolIsDoDaeSa = FALSE;    
            }

			GetSharedCmd( &bCmd, abCmdData, &wCmdDataSize );
			if ( bCmd == CMD_START_STOP ||
				 gpstSharedInfo->boolIsDriveNow == TRUE )
			{
				printf( "[MainTermProc] 운행시작/종료 시도중 또는 이미 운행중이므로 집계통신 시도하지 않음\n" );
				continue;
			}

            /* 
             * 집계통신 및 CMD실행시간 도래 
             */
	     	if ( boolIsDriveCloseYN == TRUE ||
				 ( boolIsDCSThreadStartEnable == TRUE &&
				   TimerCheck( nDCSCommIntv, gtDCSCommStart ) == 0 ) )
            {
			    GetRTCTime( &tCurrTime );
				PrintlnTimeT( "[MainTermProc] 집계통신시간 도래 : ", tCurrTime );
			
			   /*
				* 대사작업 실행 
				*/
				ProcDaeSa();
			   
			   /*
				* 집계통신 쓰레드 실행 
				*/                
				CreateDCSThread();
			    boolIsDriveCloseYN = FALSE;
            }

		   /*
			* 승하차간 명령어 통신(V~K CMD) 실행  
			*/
			ProcMainSubCmdComm();

		   /* 초기부팅이 아님을 설정 */		   
        	boolIsBootNow = FALSE;

        }
    }
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       ProcInitDCSComm                                          *
*                                                                              *
*  DESCRIPTION:       집계통신에 접속하기 이전에 필요한 정보들을 설정한다.     *
*					  														   *
*                                                                              *
*  INPUT PARAMETERS:  void                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공 	                                   *
*                                                                              *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:   호출되는 함수들은 Load_parameter_file_mgt.c에 존재한다.          *
*                                                                              *
*******************************************************************************/
short ProcInitDCSComm( void )
{
	short sRetVal = SUCCESS;
	
    /* 
     * tc_leap.dat에서 Password, 단말기 ID Load 
     */    
    if ( GetLEAPPasswd()== SUCCESS )
    {
        boolIsGetLEAPPasswd = TRUE;
    }
    /* 
     * 설치정보파일(SetUp.dat)에서 서버 IP, 차량 ID Load
     */	
    if ( LoadInstallInfo() == SUCCESS )
    {
        boolIsLoadInstallInfo = TRUE;
    }

    DebugOut( "boolIsGetLEAPPasswd : %d\n",boolIsGetLEAPPasswd );
    DebugOut( "boolIsLoadInstallInfo : %d\n",boolIsLoadInstallInfo );
    /*
     * 운행차량 파라미터 Load
     * 이유 - 통신프로세스가 메인Process와 별도의 Process이므로 집계동신을 위해 
     *        메인프로세스가 운행차량파라미터를 구조체에 Load하는 것처럼 동일하게 
     *        처리해준다.
     */
    LoadVehicleParm();

	return sRetVal;
}
/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       ProcMainSubCmdComm                                       *
*                                                                              *
*  DESCRIPTION:       승하차간의 명령어 실행 통신을 처리한다.				   *
*					  														   *
*                                                                              *
*  INPUT PARAMETERS:  void                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공 	                                   *
*                                                                              *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*         MAIN_SUB_COMM_SUB_TERM_IMG_VER  'V'     //Program Version            *
* 		MAIN_SUB_COMM_SUB_TERM_IMG_DOWN   'D'     // program Download          * 
* 		MAIN_SUB_COMM_SUB_TERM_VOICE_VER  'H'     // Voice Version 			   *
* 		MAIN_SUB_COMM_SUB_TERM_VOIC_DOWN  'A'     // Voice Download            *
* 		MAIN_SUB_COMM_SUB_TERM_PARM_DOWN  'M'     // Parameter Download        *
* 		MAIN_SUB_COMM_GET_SUB_TERM_ID     'G'     // Sub Term ID               *
* 		MAIN_SUB_COMM_ASSGN_SUB_TERM_ID   'I'     // Assign Sub Term ID        *
* 		MAIN_SUB_COMM_REQ_BL_SEARCH     'B'       // req BL search             *
* 		MAIN_SUB_COMM_REQ_PL_SEARCH     'U'       // req PL search             * 
* 		MAIN_SUB_COMM_POLLING           'P'       // Polling                   *
* 		MAIN_SUB_COMM_GET_TRANS_CONT    'X'     // get transaction contents    * 
* 		MAIN_SUB_COMM_GET_TRANS_FILE    'T'     // get transaction file 	   *
* 		MAIN_SUB_COMM_CHK_CRC           'C'     // check CRC command - old ver *
* 		MAIN_SUB_COMM_REGIST_KEYSET     'K'     // regist keyset 			   *
*                                                                              *
*******************************************************************************/
short ProcMainSubCmdComm( void )
{	
	short sRetVal = SUCCESS;
	int nTmpIndex = -1;
	int status;


    /* 집계통신 종료여부 확인 */
	if ( boolIsDCSThreadComplete == TRUE )
    {
    	pthread_join( nCommDCSThreadID, (void *)&status );
		
		printf( "\n" );
        printf( "[ProcMainSubCmdComm] %d개의 하차단말기에 대한 CMD 실행 /////////////////////////\n",
			gbSubTermCnt );
		
        /* 하차기 프로그램 버전체크 명령어부터 실행 */
        nIndex = 0;
        bCurrCmd = MAIN_SUB_COMM_SUB_TERM_IMG_VER;

        while( nIndex < gbSubTermCnt )
        {
            if ( nTmpIndex != nIndex )
            {
                nTmpIndex = nIndex;
				printf( "\n" );
				printf( "[ProcMainSubCmdComm] %d번 하차단말기 -------------------------------------------\n",
					nIndex + 1 );
            }

			DebugOut( "\n %d개 하차기중 %d번 하차기 Cmd %c 시작//////\n",
                      gbSubTermCnt,
                      nIndex+1,
                      bCurrCmd );

            sRetVal = CommMain2Sub( nIndex );

            /* CMD실행 실패시 재실행 후 다음 CMD로 JUMP *///////////////
            if ( sRetVal < 0 )
            {
                if ( sRetVal != SUCCESS )
                {
                    usleep(50000);
                    tcflush(nfdMSC, TCIOFLUSH);
                }
				
                /* 해당 CMD로 재실행 */
                printf(" %c CMD 실행실패 : %x\n 한번더실행", bCurrCmd, sRetVal);
                sRetVal = CommMain2Sub( nIndex );	                    
                if ( sRetVal < 0 )
                {
                    if ( sRetVal != SUCCESS )
                    {
                        usleep(50000);
                        tcflush(nfdMSC, TCIOFLUSH);
                    }	                    	
                		                    	
                    switch( bCurrCmd )
                    {
                        case MAIN_SUB_COMM_SUB_TERM_IMG_VER :
                        case MAIN_SUB_COMM_SUB_TERM_IMG_DOWN :
                            bCurrCmd = MAIN_SUB_COMM_GET_SUB_TERM_ID;
                            printf( ": V,D->G  JUMP\n\n\n" );
                            break;

                        case MAIN_SUB_COMM_GET_SUB_TERM_ID :
                            bCurrCmd = MAIN_SUB_COMM_SUB_TERM_PARM_DOWN;
                            printf( "G->M JUMP\n\n\n" );
                            break;

                        case MAIN_SUB_COMM_SUB_TERM_PARM_DOWN :
                            bCurrCmd = MAIN_SUB_COMM_SUB_TERM_VOICE_VER;
                            printf( "M->H JUMP\n\n\n" );
                            break;

                        case MAIN_SUB_COMM_SUB_TERM_VOICE_VER :
                        case MAIN_SUB_COMM_SUB_TERM_VOIC_DOWN :
                             bCurrCmd = MAIN_SUB_COMM_REGIST_KEYSET;
                            printf( "H,A->K JUMP\n\n\n" );
                            break;

                        case MAIN_SUB_COMM_REGIST_KEYSET :
                        	printf( "K->Next JUMP\n\n\n" );
                            boolIsCmdOneCycle = TRUE;
                            break;
                    }
                }
            }

            /* 마지막 CMD인지 체크 */
            if ( boolIsCmdOneCycle == TRUE )
            {
                nIndex++;
                bCurrCmd = MAIN_SUB_COMM_SUB_TERM_IMG_VER;
                usleep( 50000 );
            }

        }
		
		if ( gbSubTermCnt == 0 )
			printf( "[ProcMainSubCmdComm] 하차단말기 없음 -> 전체 CMD 종료 /////////////////////////\n" );
		else
		{
			printf( "\n" );
			printf( "[ProcMainSubCmdComm] 전체 CMD 종료 ////////////////////////////////////////////\n" );
		}
		printf( "\n" );

		TimerStart( &gtDCSCommStart );

		boolIsDCSThreadComplete = FALSE;
        gpstSharedInfo->boolIsKpdLock = FALSE;
        gpstSharedInfo->boolIsReadyToDrive = TRUE;
	
    }
	

	return sRetVal;
}
/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       CreateDCSThread	                                       *
*                                                                              *
*  DESCRIPTION:       집계통신을 위한 쓰레드를 생성하고 실행한다. 			   *
*					  														   *
*                                                                              *
*  INPUT PARAMETERS:  void                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공 	                                   *
*                                                                              *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*                                                                              *
*******************************************************************************/
short CreateDCSThread( void )
{
	short sRetVal = SUCCESS;

    /* 집계통신 쓰레드 생성 */
    /* 기존 집계통신 실행여부 - 운행중/비운행중에 따라 판단 */	
    if ( ( boolIsGetLEAPPasswd == TRUE ) &&
         ( boolIsLoadInstallInfo == TRUE ) &&
         ( boolIsDCSThreadStartEnable == TRUE ) )
    {

        DebugOut( "\n\n\n[집계통신] Thread 시작\n" );
        sRetVal = pthread_create( &nCommDCSThreadID, NULL, DCSComm, NULL );
        if ( sRetVal == SUCCESS )
        {
            boolIsDCSThreadComplete = FALSE;
            boolIsDCSThreadStartEnable = FALSE;
		   /*
			* 집계통신 및 승하차명령어 통신 실행 주기 초기화
			*/
			DebugOut("집계통신 및 승하차명령어 통신 실행 주기 초기화/n");
            DebugOut( "[집계통신] Thread 시작 " );
            DebugOut( "=<TermComm> nCommDCSThreadID=%ld=\n",
                      nCommDCSThreadID );
        }
        else
        {
            DebugOut( "[집계통신] Thread 생성실패 " );
        }
    }

	return sRetVal;
}
/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       ProcDaeSa			                                       *
*                                                                              *
*  DESCRIPTION:       대사를 실행한다.										   *
*					  														   *
*                                                                              *
*  INPUT PARAMETERS:  void                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공 	                                   *
*                     폴링이 존재하던 상태에서 대사확인작업이 실패한 경우      *
*                     실패한 단말기 번호를 보여주고 시스템 종료하게 된다.      *
*                     이유 - 하차기를 누가 바꾼다던지하는 문제발생가능성 때문  *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*                                                                              *
*******************************************************************************/
short ProcDaeSa( void )
{
	short sRetVal = SUCCESS;
	short ashDaeSaResult[3] = { 1, 1, 1 }; /* 대사 결과를 위한 변수 */
	int nDaeSaRetryCnt = 3;           /* 대사작업여부 확인을 위한 통신 재시도횟수*/   
    dword dwDisplayDevNo;             /* 대사작업여부 실패된 하차단말기 번호     */   
    
	DebugOut( " 집계통신 및 CMD실행시간 도래 %d\n\n\n",wDriveChangeCnt );
	DebugOut( " 집계통신 및 CMD실행시간 도래 %u\n\n\n",gpstSharedInfo->dwDriveChangeCnt );
    gpstSharedInfo->boolIsKpdLock = TRUE;

	if ( gpstSharedInfo->boolIsDriveNow == TRUE)
	{
		ctrl_event_info_write( "9002" );
	}

    /* 초기대사 방지-부팅시 대사파일명이 없으므로*/
    if ( wDriveChangeCnt != 0 )
    {
        if ( boolIsDoDaeSa == FALSE )
        {
            LogTerm( "[대사 작업확인 시작]\n" );
			
            /* 대사파일 전송-CMD T */
            nIndex = 0;
            while ( nIndex < gbSubTermCnt )
            {
				nDaeSaRetryCnt = 3;
				
	            while( nDaeSaRetryCnt-- )/*CMD T- 최대 3회 시도*/
	            {

	                LogTerm( "\n 대사작업필요여부 확인 시도 %d번 단말기 - %d 회]\n",
	                        nIndex+1, 3-nDaeSaRetryCnt );

                    bCurrCmd = MAIN_SUB_COMM_GET_TRANS_FILE;
					
                    ashDaeSaResult[nIndex] = CommMain2Sub( nIndex );
                    LogTerm( "대사결과 :%02x\n",
                             ashDaeSaResult[nIndex] );

                    sleep(1);
                    tcflush( nfdMSC, TCIOFLUSH );	
					
                   if (( ashDaeSaResult[nIndex] == SUCCESS ) ||
                       ( ashDaeSaResult[nIndex] ==
                              			ERR_MAINSUB_COMM_REQ_FILE_NOT_EXIST))
                   {
                        // 파일이 없는 에러를 제외한 에러 발생시
                        break;
                   }
				   
                }

                nIndex++;				

            }

            /* 폴링은 살아있고 대사필요여부 확인실패시 프로세스 종료 */
            /* 폴링이 오다가 중간에 하차기를 뗀다던지 할 경우 종료 */
            /* 상단LED 000001, 하단LED에는 단말기번호가 표시 */
            if (( nDaeSaRetryCnt < 0 )&&
                ( gpstSharedInfo->bPollingResCnt == 1 ))
            {
                dwDisplayDevNo =  nIndex + 1;
                DisplayASCInUpFND( "000001" );
                DisplayDWORDInDownFND( dwDisplayDevNo );
                Buzzer( 3, 50000 );
                VoiceOut( VOICE_CHECK_TERM );     // 단말기를 점검 하시기 바랍니다.
                gpstSharedInfo->bPollingResCnt = 2;
                sleep(5);
                LogTerm( "\r\n 승차 통신 프로세스 종료\r\n" );
                exit(0);
            }

            /* 대사전송성공여부 체크*/
            nIndex = 0;
            while ( nIndex < gbSubTermCnt )
            {
                DebugOut( "%d번 하차기 ashDaeSaResult : %d\n",
                          nIndex+1, ashDaeSaResult[nIndex] );

                if ( ashDaeSaResult[nIndex] == SUCCESS )
                {

                    LogTerm( "[대사작업대상 존재]\n" );

                    boolIsDoDaeSa = TRUE;
                    ashDaeSaResult[nIndex] = 1;
                    break;
                }
                nIndex++;
            }

            /* 대사작업실행*/
            if ( boolIsDoDaeSa == TRUE )
            {
                DebugOut( "[대사작업실행]/n" );
                sRetVal = RemakeTrans();

                if ( sRetVal < 0 )
                {
                    LogTerm( "[T : 대사작업실패]\n" );
                }
                else
                {
                    LogTerm( "[T : 대사작업완료]\n" );
                }
            }
            else
            {
                boolIsDoDaeSa = TRUE;
                LogTerm( "\n\n[대사작업대상 없음]\n\n" );
            }

        }
        else
        {
            LogTerm( "\n\n[대사작업 기실행]\n\n" );

        }

    }
    else
    {
    	LogTerm( "[초기대사 방지]-부팅시엔 대사파일명이 없음 \n\n" );
    }

	return sRetVal;

}
/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       ProcMainOrKpdProcessReq                                  *
*                                                                              *
*  DESCRIPTION:       메인 혹은 운전자조작기 프로세스가 요청하는 작업이        *
*					  존재하는지 확인하고 존재하는 경우                        *                                                                              *
*  INPUT PARAMETERS:  void                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공 	                                   *
*                     폴링이 존재하던 상태에서 대사확인작업이 실패한 경우      *
*                     실패한 단말기 번호를 보여주고 시스템 종료하게 된다.      *
*                     이유 - 하차기를 누가 바꾼다던지하는 문제발생가능성 때문  *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*                                                                              *
*******************************************************************************/
short ProcMainOrKpdProcessReq( void )
{
	short sRetVal = SUCCESS;
	
    /* variables for get command/data in shared memory */
    byte bSharedMemoryCmd;           /* 공유메모리내의 Command */
    char achCmdData[40] = { 0, };    /* 공유메모리내의 Data    */
    word wCmdDataSize;               /* 공유메모리내의 DataSize */
	bool boolIsAssgnSubTermID = TRUE;		

    /* 공유메모리 CMD체크 */////////////////////////////////////////////
    DebugOut("/* /n공유메모리 CMD체크 */ \n" );
    GetSharedCmd( &bSharedMemoryCmd, achCmdData, &wCmdDataSize );

    if ( achCmdData[0] == CMD_REQUEST )
    {

        switch( bSharedMemoryCmd )
        {
            case CMD_SETUP :
				if ( gpstSharedInfo->boolIsDriveNow == TRUE )
				{
					ctrl_event_info_write( "9003" );
					break;
				}
				
                if ( boolIsGetLEAPPasswd == TRUE &&
                     boolIsLoadInstallInfo == TRUE )
                {
		     	 	boolIsDCSThreadStartEnable = FALSE;
		     	 	gpstSharedInfo->boolIsKpdLock = TRUE;
                    sRetVal = SetupTerm();
		      		gpstSharedInfo->boolIsKpdLock = FALSE;
					if ( sRetVal == SUCCESS )
					{
	                	achCmdData[0] = CMD_SUCCESS_RES;
	                    SetSharedDataforCmd( bSharedMemoryCmd, achCmdData, 1 );
					}
					else
					{
	                	achCmdData[0] = CMD_FAIL_RES;
	                    SetSharedDataforCmd( bSharedMemoryCmd, achCmdData, 1 );
					}
                }
                else
                {
                	achCmdData[0] = CMD_FAIL_RES;
                    SetSharedDataforCmd( bSharedMemoryCmd, achCmdData, 1 );
                }
		  		boolIsDCSThreadStartEnable = TRUE;
                break;

            case CMD_RESETUP:
				if ( gpstSharedInfo->boolIsDriveNow == TRUE )
				{
					ctrl_event_info_write( "9004" );
					break;
				}

                if ( GetLEAPPasswd()== SUCCESS )
                {
                    boolIsGetLEAPPasswd = TRUE;
                }
				else
				{
					boolIsGetLEAPPasswd = FALSE;
				}

                if ( LoadInstallInfo() == SUCCESS )
                {
                    boolIsLoadInstallInfo = TRUE;
                }
				else
				{
					boolIsLoadInstallInfo = FALSE;
				}

                if ( boolIsGetLEAPPasswd == TRUE &&
                     boolIsLoadInstallInfo == TRUE )
                {
					boolIsDCSThreadStartEnable = FALSE;
                    gpstSharedInfo->boolIsKpdLock = TRUE;
                    sRetVal = ReSetupTerm();
	   		        gpstSharedInfo->boolIsKpdLock = FALSE;
					if ( sRetVal == SUCCESS )
					{
	                	achCmdData[0] = CMD_SUCCESS_RES;
	                    SetSharedDataforCmd( bSharedMemoryCmd, achCmdData, 1 );
					}
					else
					{
	                	achCmdData[0] = CMD_FAIL_RES;
	                    SetSharedDataforCmd( bSharedMemoryCmd, achCmdData, 1 );
					}
                }
                else
                {
                	achCmdData[0] = CMD_FAIL_RES;
                    SetSharedDataforCmd( bSharedMemoryCmd, achCmdData, 1 );
                }
		 		boolIsDCSThreadStartEnable = TRUE;				
                break;

            case CMD_PARMS_RESET:

                if ( achCmdData[1] == '1' )
                {
                    CheckDCSCommParameterReq( &boolIsGetLEAPPasswd,
                                              achCmdData );
                }
                else
                {
                    CheckDCSCommParameterReq( &boolIsLoadInstallInfo,
                                              achCmdData );
                }
                break;

            case MAIN_SUB_COMM_ASSGN_SUB_TERM_ID:
                DebugOut( "I    cmd 수행 시작 \n" );
                nIndex = 0;
                boolIsAssgnSubTermID = TRUE;
                while( nIndex < gbSubTermCnt )
                {
                    bCurrCmd = MAIN_SUB_COMM_ASSGN_SUB_TERM_ID;
                    sRetVal = CommMain2Sub( nIndex );
                    if ( sRetVal < 0 )
                    {
                        boolIsAssgnSubTermID = FALSE;
                        DebugOut( "[4.하차응답없음- I Cmd : %d \r\n",
                                sRetVal );								
                    }

                    nIndex++;
                }
                
                if ( boolIsAssgnSubTermID == FALSE )
                {
                	achCmdData[0] = CMD_FAIL_RES;
                    SetSharedDataforCmd( MAIN_SUB_COMM_ASSGN_SUB_TERM_ID, achCmdData, 1 );                
                }
                else
                {
                	achCmdData[0] = CMD_SUCCESS_RES;
                    SetSharedDataforCmd( MAIN_SUB_COMM_ASSGN_SUB_TERM_ID, achCmdData, 1 );                
                }
                break;
        }
    }
	return sRetVal;
}		
/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       GetSubTermPSAMVer                                  	   *
*                                                                              *
*  DESCRIPTION:      하차기 PSAMID 체크후 PSAMID가 변경되었을 경우 			   *
* 					 IDCENTER,KEYSET을 초기화하고 SIMID.FLG에 새로 WRITE한다   *
*                    - Command V(Protocol V)                                   *
*																			   *
*  INPUT PARAMETERS:  void                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공 	                                   *
* 																			   *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*                                                                              *
*******************************************************************************/
short GetSubTermPSAMVer( void )
{
	bool boolPSAMChkRetryNeed = FALSE;
	int nPSAMChkRetry = 3;
	short sRetVal = SUCCESS;
	int nSubIDInfoExistYN = 0;
	
//    sleep( 1); /* 부팅시간 확보 */
	nSubIDInfoExistYN = access( SUBTERM_ID_FILENAME, F_OK );  // subid.dat

	if ( nSubIDInfoExistYN == 0 )
	{
        nPSAMChkRetry = 3; /* 재시도 횟수 */
	}
	else
	{
		nPSAMChkRetry = 2; /* 재시도 횟수 */
	}
	
	while( nPSAMChkRetry-- )/* 재시도 2회 */
    { 
   	    nIndex = 0;	    	
	    while ( nIndex < gbSubTermCnt )
	    {
	        bCurrCmd = MAIN_SUB_COMM_SUB_TERM_IMG_VER;
	        sRetVal = CommMain2Sub( nIndex );

	        if ( sRetVal != SUCCESS )
	        {
	        	usleep(50000);
	       		tcflush(nfdMSC, TCIOFLUSH);
				boolPSAMChkRetryNeed = TRUE;					
			    LogTerm( "[하차기 %d번 PSAMID체크실패] : %x\n",nIndex+1, sRetVal);	
	        }
	        else
	        {	        	
	            LogTerm( "[하차기 %d번 PSAMID체크성공]\n",nIndex+1);		            
	        }
	        nIndex++;
	    }  
	      
        if ( boolPSAMChkRetryNeed == FALSE )
        {
        	break;
        }
	}

	/*CMD V 실행 후 결과 Message화면에 출력 */
    nIndex = 0;
    PrintlnASC( "[GetSubTermPSAMVer]     승차단말기PSAMID : ",
		gpstSharedInfo->abMainPSAMID, 16 );

    while( nIndex < gbSubTermCnt)
    {
        printf( "[GetSubTermPSAMVer] %d번 하차단말기PSAMID : ", nIndex+1 );
        PrintASC( gpstSharedInfo->abSubPSAMID[nIndex], 16 );
		printf( "\n" );
        nIndex++;
    }

	return sRetVal;
}
/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       GetSubTermId                                      	   *
*                                                                              *
*  DESCRIPTION:      하차기 PSAMID 체크후 PSAMID가 변경되었을 경우 			   *
* 					 IDCENTER,KEYSET을 초기화하고 SIMID.FLG에 새로 WRITE한다   *
*                    - Command V(Protocol V)                                   *
*																			   *
*  INPUT PARAMETERS:  void                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공 	                                   *
* 																			   *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*                                                                              *
*******************************************************************************/
short GetSubTermId( void )
{
	short sRetVal = SUCCESS;
    bool boolGetSubTermIdRetryNeed = FALSE;
    int nGetSubTermIdRetry = 3;
	int nSubIDInfoExistYN = 0;
	
//	sleep( 1); /* 부팅시간 확보 */
	nSubIDInfoExistYN = access( SUBTERM_ID_FILENAME, F_OK );  // subid.dat

	if ( nSubIDInfoExistYN == 0 )
	{
        nGetSubTermIdRetry = 3; /* 재시도 횟수 */
	}
	else
	{
		nGetSubTermIdRetry = 2; /* 재시도 횟수 */
	}
	
	while( nGetSubTermIdRetry-- )/* 재시도 2회 */
    { 
   	    nIndex = 0;	    	
	    while ( nIndex < gbSubTermCnt )
	    {
	        bCurrCmd = MAIN_SUB_COMM_GET_SUB_TERM_ID;
	        sRetVal = CommMain2Sub( nIndex );
			if ( sRetVal != SUCCESS )
	        {
				usleep(50000);
		       	tcflush(nfdMSC, TCIOFLUSH);
		        boolGetSubTermIdRetryNeed = TRUE;					
			    LogTerm( "[하차기 %d번 단말기 ID체크실패] : %x\n",nIndex+1, sRetVal);	
	        }
	        else
	        {
	            LogTerm( "[하차기 %d번 단말기 ID체크성공]\n",nIndex+1);		            
	        }
	        nIndex++;
	    }
	      
        if ( boolGetSubTermIdRetryNeed == FALSE )
        {
        	break;
		}
	}

	nIndex = 0;
    PrintlnASC( "[GetSubTermId]     승차단말기ID : ",
		gpstSharedInfo->abMainTermID, 9 );

    while( nIndex < gbSubTermCnt)
    {
        printf( "[GetSubTermId] %d번 하차단말기ID : ", nIndex + 1 );
        PrintlnASC( "", gpstSharedInfo->abSubTermID[nIndex],
			sizeof( gpstSharedInfo->abSubTermID[nIndex] ) );
        nIndex++;
    }

	return sRetVal;
}
/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       Polling2SubTerm                                      	   *
*                                                                              *
*  DESCRIPTION:      하차기에 현재시간,현재역ID,운행상태,승차기ID,             *
*                    거래내역파일명 등의 정보를 송신한다.                      *
*                     														   *
*																			   *
*  INPUT PARAMETERS:  void                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공 	                                   *
* 																			   *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*                                                                              *
*******************************************************************************/
short Polling2SubTerm( void )
{
	short sRetVal = SUCCESS;
	
    /* 기본 폴링 */
    nIndex = 0;
    while ( nIndex < gbSubTermCnt )
    {
        bCurrCmd = MAIN_SUB_COMM_POLLING;
        sRetVal = CommMain2Sub( nIndex ); 
        usleep(50000);
        tcflush( nfdMSC, TCIOFLUSH );			

		usleep( 32000 );
		
		if ( sRetVal < SUCCESS )
		{
			LogTerm( "polling sRetVal=>[%02x]\n", sRetVal );
		}
        nIndex++;
    }

	return sRetVal;
}
/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       CommMain2Sub                                      	   *
*                                                                              *
*  DESCRIPTION:      하차기로 보낼데이터 생성, 전송, 수신, 수신데이터 처리를   *
*                    하는 메인함수이다. 각각의 함수들을 호출하여 수행한다.     *
*                   														   *
*                     														   *
*																			   *
*  INPUT PARAMETERS:  int nIndex - 하차단말기 번호                             *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공 	                                   *
* 																			   *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*                                                                              *
*******************************************************************************/
short CommMain2Sub( int nIndex )
{
    short sRetVal = SUCCESS;

    if ( bCurrCmd != MAIN_SUB_COMM_REGIST_KEYSET )
    {
        boolIsCmdOneCycle = FALSE;
    }

	//printf( "%d번 하차기와 통신\n", nIndex + 1 );
    sRetVal = CreateSendData( bCurrCmd, nIndex + 1 );

    if ( sRetVal < 0 )
    {
    	LogTerm( "%02x command CreateSendData error 발생( %02x )\n", 
				 bCurrCmd, sRetVal );
        return ErrRet( sRetVal );
    }

    bCurrCmd = stSendPkt.bCmd;
    sRetVal = SendPkt();

    if ( sRetVal < 0 )
    {
    	LogTerm( "%02x command SendPkt error 발생( %02x )\n", 
				 bCurrCmd, sRetVal );
        return ErrRet( sRetVal );
    }
	
    /* 운행중 단말기고장의 경우 잔여단말기의 폴링속도를 위해 timeout 단축 */
    if ( bCurrCmd == MAIN_SUB_COMM_POLLING )
    {
    	sRetVal = RecvPkt( 400, nIndex + 1 ); //receive ack 도 처리
	}
	else
	{
		sRetVal = RecvPkt( 3000, nIndex + 1 ); //receive ack 도 처리		
	}
	
    if ( sRetVal < 0 )
    {
            
        LogTerm( "%02x command RecvPkt error 발생( %02x )\n", 
				 bCurrCmd, sRetVal );
        return ErrRet( sRetVal );
    }

    if ( stRecvPkt.bCmd == ACK )
    {
        switch( bCurrCmd )
        {
            case MAIN_SUB_COMM_SUB_TERM_PARM_DOWN :
                DisplayCommUpDownMsg( 1, 2 );
                sRetVal = SendParameterFile( nIndex + 1 );
                DisplayCommUpDownMsg( 2, 2 );
                if ( sRetVal < 0 )
                {
                    return ErrRet( ERR_MAINSUB_COMM_CMD_M_SEND_FINAL );
                }

                DebugOut( "\n[Final RecvPkt-From %d번 단말기]",nIndex+1 );

                // 하차단말기에서 keyset 등록을 위해 timeout을 더 준다.
                sRetVal = RecvPkt( 6000, nIndex + 1 );
                if ( sRetVal < 0 )
                {
                    return ErrRet( ERR_MAINSUB_COMM_CMD_M_RECV_FINAL );
                }

                DebugOut( "[ProcRecvData]final ack 수신/n" );
                sRetVal = ProcRecvData();
                if ( sRetVal < 0 )
                {
                    return ErrRet( ERR_MAINSUB_COMM_CMD_M_PARSE_FINAL );
                }

                DebugOut( "[ProcRecvData]final ack Parse성공/n" );

                return ErrRet( sRetVal );

            case MAIN_SUB_COMM_ASSGN_SUB_TERM_ID :
                ProcRecvData();
                return ErrRet( sRetVal );

            case MAIN_SUB_COMM_SUB_TERM_IMG_DOWN :
                DisplayCommUpDownMsg( 1, 2 );
                sRetVal = SendSubTermImgFile( nIndex + 1, BUS_EXECUTE_FILE );
                DisplayCommUpDownMsg( 2, 2 );
                if ( sRetVal < 0 )
                {
                    return ErrRet( ERR_MAINSUB_COMM_CMD_D_SEND_FINAL );
                }
                sRetVal = RecvPkt( 3000, nIndex + 1 );
                if ( sRetVal < 0 )
                {
                    return ErrRet( ERR_MAINSUB_COMM_CMD_D_RECV_FINAL );
                }

                sRetVal = ProcRecvData();
                if ( sRetVal < 0 )
                {
                    return ErrRet( ERR_MAINSUB_COMM_CMD_D_PARSE_FINAL );
                }

                return ErrRet( sRetVal );

            case MAIN_SUB_COMM_SUB_TERM_VOIC_DOWN :
                DisplayCommUpDownMsg( 1, 3 );
				printf( "[CommMain2Sub] 음성파일 하차로 전송 : %s ",
					VOICE0_FILE );
                sRetVal = SendFile( nIndex + 1, VOICE0_FILE );
                DisplayCommUpDownMsg( 2, 3 );

                if ( sRetVal < 0 )
                {
                    return ErrRet( ERR_MAINSUB_COMM_CMD_A_SEND_FINAL );
                }

                sRetVal = RecvPkt( 3000, nIndex + 1 );
                if ( sRetVal < 0 )
                {
                    return ErrRet( ERR_MAINSUB_COMM_CMD_A_RECV_FINAL );
                }

                sRetVal = ProcRecvData();
                if ( sRetVal < 0 )
                {
                    return ErrRet( ERR_MAINSUB_COMM_CMD_A_PARSE_FINAL );
                }

                return ErrRet( sRetVal );

            case MAIN_SUB_COMM_GET_TRANS_FILE :
                DisplayCommUpDownMsg(1 , 2);
                sRetVal = RecvTDFileFromSubTerm( nIndex + 1,
                                            gpstSharedInfo->abTransFileName);
                DisplayCommUpDownMsg(2 , 2);

                return ErrRet( sRetVal );

            case MAIN_SUB_COMM_POLLING :
                DebugOut( "[4.RecvPkt-From %d번 단말기]", nIndex+1 );
                sRetVal = RecvPkt( 1000, nIndex + 1 );
                if ( sRetVal < 0 )
                {
                	LogTerm( "%02x command RecvPkt1 error 발생( %02x )\n", 
				 			 bCurrCmd, sRetVal );
        
                    return ErrRet( ERR_MAINSUB_COMM_CMD_P_RECV_FINAL );
                }

                sRetVal = ProcRecvData();
                if ( sRetVal < 0 )
                {
                	LogTerm( "%d 에 NAK send\n", nIndex + 1 );
                    // 폴링에 대한 파싱 실패일 경우에는 NAK PACKET 구성
                    CreateSendData( NAK,  nIndex + 1);
                }
                else
                {
                    // 폴링에 대한 파싱성공일 경우에는
                    // ACK,X : ACK, B,U : 결과 패킷 구성
                    CreateSendData( stRecvPkt.bCmd,  nIndex + 1);
                }

                DebugOut( "[7.Final SendPkt]" );
                sRetVal = SendPkt();
                if ( sRetVal < 0 )
                {
                    LogTerm( "Error ERR_MAINSUB_COMM_CMD_P_SEND_FINAL\n" );
                    return ErrRet( ERR_MAINSUB_COMM_CMD_P_SEND_FINAL );
                }
				
				else
			  	{
			       if ( boolIsCardErrLog == TRUE )
			       {
			  			DeleteCardErrLog( stCardErrInfo.abCardNo );
						boolIsCardErrLog = FALSE;
			       }
			  	}

                return ErrRet( sRetVal );

            case MAIN_SUB_COMM_REGIST_KEYSET :
            case MAIN_SUB_COMM_SUB_TERM_VOICE_VER :
            case MAIN_SUB_COMM_GET_SUB_TERM_ID :
            case MAIN_SUB_COMM_SUB_TERM_IMG_VER :
                DebugOut( "\n[4.RecvPkt-From %d번 단말기]", nIndex+1 );
                sRetVal = RecvPkt( 6000, nIndex + 1 );
                if ( sRetVal < 0 )
                {
                    return ErrRet( ERR_MAINSUB_COMM_CMD_V_RECV_FINAL );
                }

                CreateSendData( ACK,  nIndex + 1 );
                DebugOut( "[5.SendPkt-To %d번 단말기]", nIndex+1 );
                sRetVal = SendPkt();
                if ( sRetVal < 0 )
                {
                    return ErrRet( ERR_MAINSUB_COMM_CMD_V_SEND_FINAL );
                }

                DebugOut( "\n[6.ProcRecvData]" );
                sRetVal = ProcRecvData();
                if ( sRetVal < 0 )
                {
                    return -1;
                }

                return ErrRet( sRetVal );

            default :
                return ErrRet( sRetVal );
        }
    }
    else // NAK
    {
        DebugOut( "Command NAK Received\n" );
        LogTerm( "Command NAK Received\n" );
        switch( stSendPkt.bCmd )
        {
            // 다른 cmd들이 NAK를 수신했을 경우 처리 추가
            case MAIN_SUB_COMM_ASSGN_SUB_TERM_ID : //하차단말기 ID 부여 실패 설정
                DebugOut( "[4.NAK응답-return]" );
                DebugOut( "[5.NAK응답- I Cmd -공유메모리 실패 설정 \r\n" );
                //SetSharedDataforCmd( MAIN_SUB_COMM_ASSGN_SUB_TERM_ID, "9", 1 );
                sRetVal = ERR_MAINSUB_COMM_ASSGN_SUM_TERM_ID;
                return ErrRet( sRetVal );

            case MAIN_SUB_COMM_REGIST_KEYSET :
                DebugOut( "\r\n[다음명령으로]Cmd K에서 NAK 최종응답 \r\n" );
                boolIsCmdOneCycle = TRUE;
                return ErrRet( sRetVal );

            default :
                return ErrRet( sRetVal );
        }
    }

    return sRetVal;
}
/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       SendParameterFile			                               *
*                                                                              *
*  DESCRIPTION:       하차기에 필요하 파라미터 파일들을 전송한다    		   *
*					  														   *
*                                                                              *
*  INPUT PARAMETERS:  INT nDevNo - 하차단말기번호                              *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공 	                                   *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*     NEW_FARE_INFO_FILE            // 신 요금 정보 					`	   *
*     PREPAY_ISSUER_INFO_FILE       // 선불 발행사 정보                        *
*     POSTPAY_ISSUER_INFO_FILE      // 후불 발행사 정보                        *
*     DIS_EXTRA_INFO_FILE           // 할인 할증 정보                          *
*     HOLIDAY_INFO_FILE             // 휴일 정보                               *
*     XFER_TERM_INFO_FILE           // 환승조건 정보                           *
*     BUS_STATION_INFO_FILE         // 정류장 정보 파일                        *
*     VEHICLE_PARM_FILE             // 운행차량 파라미터 버전                  *
*     ROUTE_PARM_FILE               // 노선별 파라메터 파일                    *
*     XFER_APPLY_INFO_FILE          // 환승 적용                               *
*     ISSUER_VALID_PERIOD_INFO_FILE // 후불 발행사 체크 정보           *
*     AUTO_CHARGE_SAM_KEYSET_INFO_FILE // 자동충전 SAM Key Set 정보            *
*     AUTO_CHARGE_PARM_INFO_FILE       //자동충전 Parameter 정보               *
*     EPURSE_ISSUER_REGIST_INFO_FILE   // 전자 화폐사 등록정보                 *
*     PSAM_KEYSET_INFO_FILE            //지불SAM Key Set 정보                  *          
*                                                                              *
*******************************************************************************/
short SendParameterFile( int nDevNo )
{
    int nLoopNo = 0;
    short sRetVal = 0;

    byte* FileDownfile [][2] =
    {
        { "N", NEW_FARE_INFO_FILE },			/* 신 요금 정보 */
        { "N", PREPAY_ISSUER_INFO_FILE },		/* 선불 발행사 정보 */
        { "N", POSTPAY_ISSUER_INFO_FILE },		/* 후불 발행사 정보 */
        { "N", DIS_EXTRA_INFO_FILE },			/* 할인 할증 정보 */
        { "N", HOLIDAY_INFO_FILE },				/* 휴일 정보 */
        { "N", XFER_TERM_INFO_FILE },			/* 환승조건 정보 */
        { "N", BUS_STATION_INFO_FILE },			/* 정류장 정보 파일 */
        { "N", VEHICLE_PARM_FILE },				/* 운행차량 파라미터 버전 */
        { "N", ROUTE_PARM_FILE },				/* 노선별 파라메터 파일 */
        { "N", XFER_APPLY_INFO_FILE },			/* 환승 적용 */
        { "N", ISSUER_VALID_PERIOD_INFO_FILE },	/* 후불 발행사 체크 정보*/
        { "N", AUTO_CHARGE_SAM_KEYSET_INFO_FILE },
												/* 자동충전 SAM Key Set 정보 */
        { "N", AUTO_CHARGE_PARM_INFO_FILE },
												/* 자동충전 Parameter 정보 */
        { "N", EPURSE_ISSUER_REGIST_INFO_FILE },/* 전자 화폐사 등록정보 */
        { "N", PSAM_KEYSET_INFO_FILE },			/* 지불SAM Key Set 정보 */
        { NULL, NULL }
    };

    FileDownfile[0][0] = "Y";
    FileDownfile[1][0] = "Y";
    FileDownfile[2][0] = "Y";
    FileDownfile[3][0] = "Y";
    FileDownfile[4][0] = "Y";
    FileDownfile[5][0] = "Y";
    FileDownfile[6][0] = "Y";
    FileDownfile[7][0] = "Y";
    FileDownfile[8][0] = "Y";
    FileDownfile[9][0] = "Y";
    FileDownfile[10][0] = "Y";
    FileDownfile[11][0] = "Y";
    FileDownfile[12][0] = "Y";
    FileDownfile[13][0] = "Y";
    FileDownfile[14][0] = "Y";

    stSendPkt.bCmd = MAIN_SUB_COMM_SUB_TERM_PARM_DOWN;
    stSendPkt.bDevNo = nDevNo + '0';

    while( FileDownfile[nLoopNo][0] != NULL ) // 끝까지
    {
        stSendPkt.wSeqNo = 0;
        DebugOut( "file index nLoopNo : %d", nLoopNo );

        if ( memcmp( FileDownfile[nLoopNo][0], "Y", 1 ) == 0 )
        {

            if ( access( FileDownfile[nLoopNo][1], F_OK ) != 0 )
            {
                DebugOut( "\r\n ======no====== FileDownfile[%s]",
                          FileDownfile[nLoopNo][1] );
                nLoopNo++;
                continue;
            }
            else
            {
				printf( "[SendParameterFile] 하차전송 : [%s] ",
					FileDownfile[nLoopNo][1]);
            }

            stSendPkt.nDataSize = strlen( FileDownfile[nLoopNo][1] );
            memcpy( &stSendPkt.abData[0], FileDownfile[nLoopNo][1],
                    strlen(FileDownfile[nLoopNo][1]));

            DebugOut( "\r\n보낼파일명[%s],길이:[%d]  \n",
                      FileDownfile[nLoopNo][1],
                      strlen(FileDownfile[nLoopNo][1]) );
            DebugOut( "[SendPkt- 보낼파일명 -To %d번 단말기]", nIndex+1 );

            sRetVal = SendPkt();
            if ( sRetVal < 0 )
            {
                LogTerm( "\r\n======result====== [%s] 파일명 보내기 에러\n",
                          FileDownfile[nLoopNo][1]);
                nLoopNo++;
                continue;
            }

            DebugOut( "[RecvPkt- 보낼파일명응답 -To %d번 단말기]", nIndex+1 );

            sRetVal = RecvPkt( 3000, nIndex + 1 );
            if ( sRetVal < 0 || stRecvPkt.bCmd == NAK )
            {
                LogTerm( "\r\n======RecvPkt result====== [%s] 파일명 응답 에러\n",
                          FileDownfile[nLoopNo][1]);
                nLoopNo++;
                continue;
            }

            sRetVal = SendFile(nDevNo, FileDownfile[nLoopNo][1]);
            if ( sRetVal < 0 )
            {
                LogTerm( "\r\n====== SendFile result====== [%s] 파일 보내기 에러\n",
                          FileDownfile[nLoopNo][1]);
                nLoopNo++;
                continue;
            }

        }

        nLoopNo++;
    }

    stSendPkt.bCmd = ETB;
    stSendPkt.bDevNo = nDevNo + '0';
    stSendPkt.wSeqNo = 0;
    stSendPkt.nDataSize = 0;

    sRetVal = SendPkt();//다보냈을을 알리는 신호
    if ( sRetVal < 0 )
    {
        return ErrRet( sRetVal );
    }

    return SUCCESS;

}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       CreateSendData                                      	   *
*                                                                              *
*  DESCRIPTION:      하차기로 보낼데이터를 생성한다.                           *
*                   														   *
*                     														   *
*																			   *
*  INPUT PARAMETERS:  byte bCmd - 실행할 명령어                                *
*                     int nDevNo - 하차단말기 번호 	                           *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공 	                                   *
* 																			   *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*                                                                              *
*******************************************************************************/
short CreateSendData( byte bCmd, int nDevNo )
{
    short sRetVal = SUCCESS;

    switch( bCmd )
    {
        case MAIN_SUB_COMM_SUB_TERM_PARM_DOWN :   //Program Version
            DebugOut( "\r\n[1.CreateDownPar]" );
            sRetVal = CreateDownPar( nDevNo );
            return ErrRet( sRetVal );

        case NAK :
            sRetVal = CreateNAK( nDevNo );
            return ErrRet( sRetVal );

        case ACK :
        case MAIN_SUB_COMM_GET_TRANS_CONT :
            sRetVal = CreateACK( nDevNo );
            return ErrRet( sRetVal );

        case MAIN_SUB_COMM_SUB_TERM_IMG_VER :   //Program Version
            DebugOut( "\r\n[1.CreateReqImgVer]" );
            sRetVal = CreateReqImgVer( nDevNo );
            return ErrRet( sRetVal );

        case MAIN_SUB_COMM_SUB_TERM_IMG_DOWN :   //program Download
            DebugOut( "\r\n[1.CreateDownImg]" );
            sRetVal = CreateDownImg( nDevNo );
            return ErrRet( sRetVal );

        case MAIN_SUB_COMM_GET_SUB_TERM_ID :  //SubId 승차 전송 //0330 2005.2.28
            DebugOut( "\r\n[1.CreateReqSubTermID]" );
            sRetVal = CreateReqSubTermID( nDevNo );
            return ErrRet( sRetVal );

        case MAIN_SUB_COMM_SUB_TERM_VOICE_VER : //Voice Version //0330 2005.2.28
            DebugOut( "\r\n[1.CreateReqVoiceVer]" );
            sRetVal = CreateReqVoiceVer( nDevNo );
            return ErrRet( sRetVal );

        case MAIN_SUB_COMM_SUB_TERM_VOIC_DOWN ://Voice Download //0330 2005.2.28
            DebugOut( "\r\n[1.CreateDownVoice]" );
            sRetVal = CreateDownVoice( nDevNo );
            return ErrRet( sRetVal );

        case MAIN_SUB_COMM_REGIST_KEYSET :   //Keyset Result
            DebugOut( "\r\n[1.CreateReqKeySet]" );
            sRetVal = CreateReqKeySet( nDevNo );
            return ErrRet( sRetVal );

        case MAIN_SUB_COMM_POLLING :        //polling
            DebugOut( "\n[1.CreatePoll]" );
            sRetVal = CreatePoll( nDevNo );
            return ErrRet( sRetVal );

        case MAIN_SUB_COMM_REQ_BL_SEARCH :     //polling
            DebugOut( "\r\n[6.CreateBLResult]" );
            sRetVal = CreateBLResult( nDevNo );
            return ErrRet( sRetVal );

        case MAIN_SUB_COMM_REQ_PL_SEARCH :     //polling
            DebugOut( "\r\n[6.CreatePLResult]" );
            sRetVal = CreatePLResult( nDevNo );
            return ErrRet( sRetVal );

        case MAIN_SUB_COMM_GET_TRANS_FILE :  //TD File 전송요구
            DebugOut( "\r\n[1.CreateReqTDFile]" );
            sRetVal = CreateReqTDFile( nDevNo );
            return ErrRet( sRetVal );

        case MAIN_SUB_COMM_ASSGN_SUB_TERM_ID :   //하차 단말기 아이디 부여
            DebugOut( "\r\n[1.CreatePutSubTermID]" );
            sRetVal = CreatePutSubTermID( nDevNo );
            return ErrRet( sRetVal );

        default :
            return ErrRet( sRetVal );
    }
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       CreateReqImgVer                                      	   *
*                                                                              *
*  DESCRIPTION:      하차기 펌웨어버전정보를 요청하는 데이터를 생성한다.       *
*                    승차기 PSAMID,단말기ID 정보도 요청하는 데이터에 실어서    *
*                    보낸다.                                                   *
*                   														   *
*																			   *
*  INPUT PARAMETERS:  int nDevNo - 하차단말기 번호 	                           *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공 	                                   *
* 																			   *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*                                                                              *
*******************************************************************************/
short CreateReqImgVer( int nDevNo )
{
    short sRetVal = SUCCESS;

    stSendPkt.bCmd = MAIN_SUB_COMM_SUB_TERM_IMG_VER;
    stSendPkt.bDevNo = nDevNo + '0';
    stSendPkt.wSeqNo = 0;
    stSendPkt.nDataSize = 25;

    DebugOut( "stSendPkt.abDataSide %d:", stSendPkt.nDataSize );

    //승차 sniper sam id
    memcpy( stSendPkt.abData, gpstSharedInfo->abMainPSAMID, 16 );
    //단말기 아이디
    memcpy( &stSendPkt.abData[16], gpstSharedInfo->abMainTermID, 9 );

    DebugOutlnASC( "abMainPSAMID :", gpstSharedInfo->abMainPSAMID, 16 );
    DebugOutlnASC( "abMainTermID :", gpstSharedInfo->abMainTermID, 9 );
    DebugOutlnASC( "\n<MainTerm> CreateReqImgVer stSendPkt Ready :",
                    &stSendPkt.abData[0], 25 );

    return ErrRet( sRetVal );
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       CreateReqTDFile                                      	   *
*                                                                              *
*  DESCRIPTION:      하차기 대사를 위한 거래내역파일을 요청하는 데이터를 생성  *
*                    한다.   												   *
*                   														   *
*																			   *
*  INPUT PARAMETERS:  int nDevNo - 하차단말기 번호 	                           *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공 	                                   *
* 																			   *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*                                                                              *
*******************************************************************************/
short CreateReqTDFile( int nDevNo )
{
    short sRetVal = SUCCESS;

    stSendPkt.bCmd = MAIN_SUB_COMM_GET_TRANS_FILE;
    stSendPkt.bDevNo = nDevNo + '0';
    stSendPkt.wSeqNo = 0;
    stSendPkt.nDataSize = 18;

#ifdef  TEST_DAESA
    PrintlnASC( "요청TD파일명 :", gpstSharedInfo->abTransFileName, 18 );
#endif

    memcpy( stSendPkt.abData, gpstSharedInfo->abTransFileName, 18 );

    return ErrRet( sRetVal );

}
/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       CreateDownPar                                      	   *
*                                                                              *
*  DESCRIPTION:      하차기 파라미터 파일 전송을 알리는 데이터를 생성한다.     *
*                   														   *
*																			   *
*  INPUT PARAMETERS:  int nDevNo - 하차단말기 번호 	                           *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공 	                                   *
* 																			   *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*                                                                              *
*******************************************************************************/
short CreateDownPar( int nDevNo )
{

    stSendPkt.bCmd = MAIN_SUB_COMM_SUB_TERM_PARM_DOWN;
    stSendPkt.bDevNo = nDevNo + '0';
    stSendPkt.wSeqNo = 0;
    stSendPkt.nDataSize = 0;

    return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       CreateDownImg                                      	   *
*                                                                              *
*  DESCRIPTION:      하차기 펌웨어 전송을 알리는 데이터를 생성한다.   		   *
*                   														   *
*																			   *
*  INPUT PARAMETERS:  int nDevNo - 하차단말기 번호 	                           *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공 	                                   *
* 																			   *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*                                                                              *
*******************************************************************************/
short CreateDownImg( int nDevNo )
{

    stSendPkt.bCmd = MAIN_SUB_COMM_SUB_TERM_IMG_DOWN;
    stSendPkt.bDevNo = nDevNo + '0';
    stSendPkt.wSeqNo = 0;
    stSendPkt.nDataSize = 12;
    memcpy(  stSendPkt.abData, BUS_SUB_IMAGE_FILE, 12 );  //승차 sniper sam id

    return SUCCESS;
}
/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       CreateReqSubTermID                                       *
*                                                                              *
*  DESCRIPTION:       하차기ID전송을 요청하는 데이터를 생성한다.   		       *
*                   														   *
*																			   *
*  INPUT PARAMETERS:  int nDevNo - 하차단말기 번호 	                           *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공 	                                   *
* 																			   *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*                                                                              *
*******************************************************************************/
short CreateReqSubTermID( int nDevNo )
{
    stSendPkt.bCmd = MAIN_SUB_COMM_GET_SUB_TERM_ID;
    stSendPkt.bDevNo = nDevNo + '0';
    stSendPkt.wSeqNo = 0;
    stSendPkt.nDataSize = 0;

    return SUCCESS;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       CreateReqVoiceVer                                        *
*                                                                              *
*  DESCRIPTION:       하차기음성버전 전송을 요청하는 데이터를 생성한다.        *
*                   														   *
*																			   *
*  INPUT PARAMETERS:  int nDevNo - 하차단말기 번호 	                           *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공 	                                   *
* 																			   *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*                                                                              *
*******************************************************************************/
short CreateReqVoiceVer( int nDevNo )
{
    stSendPkt.bCmd = MAIN_SUB_COMM_SUB_TERM_VOICE_VER;
    stSendPkt.bDevNo = nDevNo + '0';
    stSendPkt.wSeqNo = 0;
    stSendPkt.nDataSize = 0;

    return SUCCESS;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       CreateReqVoiceVer                                        *
*                                                                              *
*  DESCRIPTION:       하차기음성파일 전송을 알리는 데이터를 생성한다.          *
*                   														   *
*																			   *
*  INPUT PARAMETERS:  int nDevNo - 하차단말기 번호 	                           *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공 	                                   *
* 																			   *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*                                                                              *
*******************************************************************************/
short CreateDownVoice( int nDevNo )
{
    stSendPkt.bCmd = MAIN_SUB_COMM_SUB_TERM_VOIC_DOWN;
    stSendPkt.bDevNo = nDevNo + '0';
    stSendPkt.wSeqNo = 0;
    stSendPkt.nDataSize = 8;
    memcpy( stSendPkt.abData, VOICE0_FILE, 8 );  //승차 sniper sam id

    return SUCCESS;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       CreateReqKeySet                                          *
*                                                                              *
*  DESCRIPTION:       Keyset/발생사 등록결과 전송을 요청하는 데이터를 생성한다.*
*                   														   *
*																			   *
*  INPUT PARAMETERS:  int nDevNo - 하차단말기 번호 	                           *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공 	                                   *
* 																			   *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*                                                                              *
*******************************************************************************/
short CreateReqKeySet( int nDevNo )
{
    stSendPkt.bCmd = MAIN_SUB_COMM_REGIST_KEYSET;
    stSendPkt.bDevNo = nDevNo + '0';
    stSendPkt.wSeqNo = 0;
    stSendPkt.nDataSize = 0;

    return SUCCESS;
}
/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       CreatePutSubTermID                                       *
*                                                                              *
*  DESCRIPTION:       하차기 ID를 부여하기위해 전송을 알리는 데이터를 생성한다.*
*                   														   *
*																			   *
*  INPUT PARAMETERS:  int nDevNo - 하차단말기 번호 	                           *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공 	                                   *
* 																			   *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*                                                                              *
*******************************************************************************/
short CreatePutSubTermID( int nDevNo )
{
    short sRetVal = SUCCESS;
    byte bCmd;
    byte pcCmdData[40];
    word wCmdDataSize;
    byte abSubTermID[10];

    stSendPkt.bCmd = MAIN_SUB_COMM_ASSGN_SUB_TERM_ID;
    stSendPkt.bDevNo = nDevNo + '0';
    stSendPkt.wSeqNo = 0;

    GetSharedCmd( &bCmd, pcCmdData, &wCmdDataSize );
  
    memcpy( abSubTermID, &pcCmdData[(nDevNo -1) * 9 + 2], 9 );
    abSubTermID[9] = 0x00;

    stSendPkt.nDataSize = strlen( abSubTermID );
    memcpy( &stSendPkt.abData[0], abSubTermID, strlen( abSubTermID ) );
	DebugOut("&stSendPkt.abData[0] : %s",&stSendPkt.abData[0]);
    return sRetVal;
}
/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       CreatePoll                                  			   *
*                                                                              *
*  DESCRIPTION:       기본폴링 전송을 위한 데이터를 생성한다.				   *
*                   														   *
*																			   *
*  INPUT PARAMETERS:  int nDevNo - 하차단말기 번호 	                           *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공 	                                   *
* 																			   *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*                                                                              *
*******************************************************************************/
short CreatePoll( int nDevNo )
{
    short sRetVal = SUCCESS;
    byte abCurrTime[14] = { 0, };

    stSendPkt.bCmd = MAIN_SUB_COMM_POLLING;
    stSendPkt.bDevNo = nDevNo + '0';
    stSendPkt.wSeqNo = 0;
    stSendPkt.nDataSize = 50;

    TimeT2ASCDtime(  gpstSharedInfo->tTermTime, abCurrTime );
    memcpy( stSendPkt.abData, abCurrTime, 14 );
    memcpy( &stSendPkt.abData[14], gpstSharedInfo->abNowStationID, 7 );
    stSendPkt.abData[21] = gpstSharedInfo->boolIsDriveNow;
    memcpy( &stSendPkt.abData[22], gpstSharedInfo->abMainTermID, 9 );
    memcpy( &stSendPkt.abData[31], gpstSharedInfo->abTransFileName, 18 );
	stSendPkt.abData[49] = gpstSharedInfo->gbGPSStatusFlag;

    return ErrRet( sRetVal);
}
/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       CreateBLResult                                  		   *
*                                                                              *
*  DESCRIPTION:       BL 검색결과 전송을 위한 데이터를 생성한다.			   *
*                   														   *
*																			   *
*  INPUT PARAMETERS:  int nDevNo - 하차단말기 번호 	                           *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공 	                                   *
* 																			   *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*                                                                              *
*******************************************************************************/
short CreateBLResult( int nDevNo )
{
    short sRetVal = SUCCESS;

    stSendPkt.bCmd = MAIN_SUB_COMM_REQ_BL_SEARCH;
    stSendPkt.bDevNo = nDevNo + '0';
    stSendPkt.wSeqNo = 0;

    if ( boolIsCardErrLog == TRUE )
    {
        stSendPkt.nDataSize =  sizeof( TRANS_INFO ) + 3;
    }
    else 
    {
        stSendPkt.nDataSize =  3;
    }
	memcpy( stSendPkt.abData, &bBLCheckStatus, 2 );
	stSendPkt.abData[2] = bBLCheckResult;
	
	bBLCheckStatus = 0;
	
	if ( boolIsCardErrLog == TRUE )
    {
        memcpy( &stSendPkt.abData[3], &stCardErrInfo, sizeof( TRANS_INFO ) );
    }

//	LogTerm( "\r\ [CreateBLResult]BL Check Result ==> %d\n", bBLCheckResult  );
#ifdef TEST_BLPL_CHECK
    printf( "\r\ [CreateBLResult] BL 검색 성공여부 : %d,BL Check Result ==> %d\n",
              , bBLCheckStatus, bBLCheckResult  );
#endif
    return ErrRet( sRetVal );
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       CreatePLResult                                  		   *
*                                                                              *
*  DESCRIPTION:       PL 검색결과 전송을 위한 데이터를 생성한다.			   *
*                   														   *
*																			   *
*  INPUT PARAMETERS:  int nDevNo - 하차단말기 번호 	                           *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공 	                                   *
* 																			   *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*                                                                              *
*******************************************************************************/
short CreatePLResult( int nDevNo )
{
    short sRetVal = SUCCESS;

    stSendPkt.bCmd = MAIN_SUB_COMM_REQ_PL_SEARCH;
    stSendPkt.bDevNo = nDevNo + '0';
    stSendPkt.wSeqNo = 0;

    if ( boolIsCardErrLog == TRUE )
    {
        stSendPkt.nDataSize =  sizeof( TRANS_INFO ) + 3;
    }
    else 
    {
        stSendPkt.nDataSize =  3;
    }
	memcpy( stSendPkt.abData, &bPLCheckStatus, 2 );	
    stSendPkt.abData[2] = bPLCheckResult;

	bPLCheckStatus = 0;

    if ( boolIsCardErrLog == TRUE )
    {
        memcpy( &stSendPkt.abData[3], &stCardErrInfo, sizeof( TRANS_INFO ) );
    }
	
//	LogTerm( "\r\ [CreatePLResult]PL Check Result ==> %02x\n", bPLCheckResult );
	
    return ErrRet( sRetVal );
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       ProcRecvData                                  		   *
*                                                                              *
*  DESCRIPTION:       명령어 통신을 실행하여 하차기로부터 받은 응답을 파싱하고 *
*                     처리한다.												   *
*                   														   *
*																			   *
*  INPUT PARAMETERS:  void 							                           *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공 	                                   *
* 																			   *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*                                                                              *
*******************************************************************************/
short ProcRecvData( void )
{
    short sRetVal = SUCCESS;

    switch( stRecvPkt.bCmd  )
    {
       /*
        * Program Version Check
        */
        case MAIN_SUB_COMM_SUB_TERM_IMG_VER :       
            DebugOut( "\r\nRecvImgVer\r\n" );
            sRetVal = RecvImgVer();
            return ErrRet( sRetVal );
	   /*
		* Request SubTerminal ID
		*/
        case MAIN_SUB_COMM_GET_SUB_TERM_ID :        //
            DebugOut( "\r\nRecvSubTermID \r\n" );
            sRetVal = RecvSubTermID();

            return ErrRet( sRetVal );
	   /*
		* voice Version Check
		*/
        case MAIN_SUB_COMM_SUB_TERM_VOICE_VER :    
            DebugOut( "\r\nRecvVoiceVer\r\n" );
            sRetVal = RecvVoiceVer();

            return ErrRet( sRetVal );
	   /*
		* KeySet Result
		*/			
        case MAIN_SUB_COMM_REGIST_KEYSET :          
            DebugOut( "\r\nRecvKeySet\r\n" );
            sRetVal = RecvKeySet();
            return ErrRet( sRetVal );
	   /*
		* ACK Response
		*/	
        case ACK :
            DebugOut( "\n[4.RecvACK-From %d번 단말기]",nIndex+1 );
            sRetVal = RecvACKResp();
            return ErrRet( sRetVal );
	   /*
		* NAK Response
		*/
        case NAK :
            DebugOut( "\r\n  NAK \r\n" );
            sRetVal = RecvNAKResp();
            return ErrRet( sRetVal );
	   /*
		* Polling 응답으로 BLSearch 수신
		*/
        case MAIN_SUB_COMM_REQ_BL_SEARCH :  
            DebugOut( "[5.BL체크-main]" );
            sRetVal = RecvBLCheckReq();
			if( sRetVal < SUCCESS )
			{
				LogTerm( "RecvBLCheckReq Error %02x\n", sRetVal );
			}
            bCurrCmd = MAIN_SUB_COMM_POLLING;
            return ErrRet( sRetVal );
	   /*
		* Polling 응답으로 PLSearch 수신
		*/
        case MAIN_SUB_COMM_REQ_PL_SEARCH :  
            DebugOut( "[5.PL체크-main]" );
            sRetVal = RecvPLCheckReq();
			if( sRetVal < SUCCESS )
			{
				LogTerm( "RecvPLCheckReq Error %02x\n", sRetVal );
			}
            bCurrCmd = MAIN_SUB_COMM_POLLING;
            return ErrRet( sRetVal );
	   /*
		* Polling 응답으로 대사를 위한 거래내역파일(TransFile) 수신
		*/
        case MAIN_SUB_COMM_GET_TRANS_CONT : 
            DebugOut( "[5.거래내역파일처리]" );
            sRetVal = RecvTransDataOnPolling();
			if( sRetVal < SUCCESS )
			{
				LogTerm( "RecvPLCheckReq Error %02x\n", sRetVal );
			}
            bCurrCmd = MAIN_SUB_COMM_POLLING;
            return ErrRet( sRetVal );
		/*
		* 그 외외의 응답은 수신한 데이터 파싱에러로 에러코드 리턴
		*/
        default :
            sRetVal = ERR_MAINSUB_COMM_MAIN_RECV_DATA_PARSE;
            return ErrRet( sRetVal );
    }
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       RecvNAKResp                                  		       *
*                                                                              *
*  DESCRIPTION:       명령어 통신을 실행하여 하차기로부터 받은 NAK응답을       *
*                     파싱하고                        						   *											   *
*                   														   *
*																			   *
*  INPUT PARAMETERS:  void							                           *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공 	                                   *
* 																			   *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*                                                                              *
*******************************************************************************/
short RecvNAKResp( void )
{
    short sRetVal = SUCCESS;

    switch( bCurrCmd  )
    {
        case MAIN_SUB_COMM_SUB_TERM_PARM_DOWN :   // parameter file down
            DebugOut( "\r\n[다음명령으로]Cmd M에서 NAK 최종응답 \r\n" );
            DisplayCommUpDownMsg( 2, 2 );
            //파일을 내리다가 결국 하나라도 실패해서 NAK가 온 경우
            bCurrCmd = MAIN_SUB_COMM_SUB_TERM_VOICE_VER;
            return SUCCESS;

        case MAIN_SUB_COMM_SUB_TERM_IMG_DOWN :   // Img file down
            DebugOut( "\r\n[다음명령으로]Cmd D에서 NAK 최종응답 \r\n" );
            bCurrCmd = MAIN_SUB_COMM_GET_SUB_TERM_ID;
            return SUCCESS;

        case MAIN_SUB_COMM_SUB_TERM_VOIC_DOWN :   // voice file down
            DebugOut( "\r\n[다음명령으로]Cmd A에서 NAK 최종응답 \r\n" );
            bCurrCmd = MAIN_SUB_COMM_REGIST_KEYSET;
            return SUCCESS;

        default :
            return ErrRet( sRetVal );
    }

}
/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       RecvACKResp                                  		       *
*                                                                              *
*  DESCRIPTION:       명령어 통신을 실행하여 하차기로부터 받은 ACK응답을       *
*                     파싱하고                        						   *											   *
*                   														   *
*																			   *
*  INPUT PARAMETERS:  void								                       *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공 	                                   *
* 																			   *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*                                                                              *
*******************************************************************************/
short RecvACKResp( void )
{
    int nDevNo;
    short sRetVal = SUCCESS;
    nDevNo = stRecvPkt.bDevNo;

    switch( bCurrCmd  )
    {
        case MAIN_SUB_COMM_SUB_TERM_PARM_DOWN :   // parameter file down
            DebugOut( "\r\n[최종처리]Cmd M \r\n" );
            DisplayCommUpDownMsg( 2, 2 );
            //모든 파일을 내린 경우 VOICE로
            bCurrCmd = MAIN_SUB_COMM_SUB_TERM_VOICE_VER;
            DebugOut( "\n[M : 파라미터파일 다운로드 완료]\n" );
            return SUCCESS;

        case MAIN_SUB_COMM_SUB_TERM_IMG_DOWN :   // img file down
            DebugOut( "\r\n[최종처리]Cmd D  \r\n" );
            DisplayCommUpDownMsg( 2, 2 );
            memcpy( gpstSharedInfo->abSubVer[nDevNo-1],
                    gpstSharedInfo->abMainVer, 4 );
            // 'G' command로
            bCurrCmd = MAIN_SUB_COMM_GET_SUB_TERM_ID;
            DebugOut( "[D : 프로그램 다운로드 완료]\n" );
            return SUCCESS;

        case MAIN_SUB_COMM_SUB_TERM_VOIC_DOWN :   // voice file down
            DebugOut( "\r\n[최종처리]Cmd A  \r\n" );
            DisplayCommUpDownMsg( 2, 3 );
            memcpy( gpstSharedInfo->abSubVoiceVer[nDevNo-1],
                    gpstSharedInfo->abMainVoiceVer, 4 );
            //성공시 K
            bCurrCmd = MAIN_SUB_COMM_REGIST_KEYSET;
            DebugOut( "[A : 음성파일 다운로드 완료]\n" );
            return SUCCESS;

        case MAIN_SUB_COMM_ASSGN_SUB_TERM_ID :   //하차단말기 ID 부여
            DebugOut( "\n[5.공유메모리 I cmd결과 설정" );
            //SetSharedDataforCmd( MAIN_SUB_COMM_ASSGN_SUB_TERM_ID, "0", 1 );
            return SUCCESS; //ErrRet( sRetVal );

        case MAIN_SUB_COMM_POLLING : // 폴링 응답에서 대해서 ACK가 왔을 경우
            DebugOut( "[5.단순폴링응답-BLPL요청/거래내역파일 없음]" );
            gpstSharedInfo->bPollingResCnt = 1;
            return SUCCESS;

        default :
            //sRetVal = ERR_CODE;
            return ErrRet( sRetVal );
    }
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       RecvImgVer                                  		       *
*                                                                              *
*  DESCRIPTION:       명령어 통신을 실행하여 하차기로부터 응답받은 펌웨어 버전 *
*                     파싱하고 처리한다.              						   *											   *
*                   														   *
*																			   *
*  INPUT PARAMETERS:  void								                       *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공 	                                   *
* 																			   *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*                                                                              *
*******************************************************************************/
short RecvImgVer( void )
{
    short sRetVal = SUCCESS;
    int nDevNo = 0;
    time_t tCurrTime;
    byte abCurrDtime[15] = { 0, };
    bool boolIsImgVerChangeNeeded = FALSE;

    nDevNo = stRecvPkt.bDevNo;

    GetRTCTime( &tCurrTime );
    TimeT2ASCDtime( tCurrTime, abCurrDtime );


    if ( stRecvPkt.nDataSize == 37 )
    {
        memcpy( gpstSharedInfo->abSubVer[nDevNo-1], stRecvPkt.abData, 4 );
        memcpy( gpstSharedInfo->abSubCSAMID[nDevNo-1], &stRecvPkt.abData[4], 8 );
        memcpy( gpstSharedInfo->abSubPSAMID[nDevNo-1],
                &stRecvPkt.abData[12], 16 );
        DebugOutlnASC( "SubTerm Imgage Version Received : ",
                       stRecvPkt.abData, 4 );

        memcpy( &gpstSharedInfo->abSubPSAMPort[nDevNo-1],
                &stRecvPkt.abData[28], 1 );
        DebugOut( "\nabSubPSAMPort [%d]\r\n",
                  gpstSharedInfo->abSubPSAMPort[nDevNo-1] );
        DebugOutlnASC( "PSamID",
                    gpstSharedInfo->abSubPSAMID[nDevNo-1], 16   );

        memcpy( &gpstSharedInfo->abSubISAMID[nDevNo-1],
                &stRecvPkt.abData[29], 7 );

	// TPSAM 여부 
        memcpy( &gpstSharedInfo->abSubPSAMVer[nDevNo-1],
                      &stRecvPkt.abData[36], 1 );

	DebugOut( "\n TPSAM 여부 => 하차기 %d는 %d 이다 \n", nDevNo, 
			  gpstSharedInfo->abSubPSAMVer[nDevNo-1]);
        
        if  ( memcmp( gpstSharedInfo->abMainVer,
                      gpstSharedInfo->abSubVer[nDevNo-1], 4 )  > 0 )
        {
            //적용 Current
            DebugOut( "\r\nboolIsImgVerChangeNeeded=TRUE셋팅" );
            boolIsImgVerChangeNeeded = TRUE;
			printf( "[RecvImgVer] 승차단말기 : " );
			PrintASC( gpstSharedInfo->abMainVer, 4 );
			printf( ", %d번 하차단말기 : ", nDevNo );
			PrintASC( gpstSharedInfo->abSubVer[nDevNo-1], 4 );
			printf( "\n" );
        }
        else
        {
			printf( "[RecvImgVer] 승차단말기 : " );
			PrintASC( gpstSharedInfo->abMainVer, 4 );
			printf( ", %d번 하차단말기 : ", nDevNo );
			PrintASC( gpstSharedInfo->abSubVer[nDevNo-1], 4 );
			printf( "\n" );
        }

        if ( nIndex == gbSubTermCnt -1 )
        {
            DebugOut( "마지막 단말기에 대해 'V'실행 후" );
            if ( PSAMIDCompareMainWithSubTerm() == 1 )
            {
               DebugOut( "\r\n sim_flag = 0 버전 변경안함" );
            }
            else
            {
                DebugOut( "\r\n sim_flag = 1 버전 변경함" );
            }
        }

        sRetVal = SUCCESS;
    }
    else
    {
        sRetVal = ERR_DATASIZE_SUBTERM_SEND_BY_CMD_V;
    }

    DebugOut( "\n[V : 버전체크 완료]\n" );

    if  ( boolIsImgVerChangeNeeded == TRUE )
    {
        bCurrCmd = MAIN_SUB_COMM_SUB_TERM_IMG_DOWN;
		printf( "[RecvImgVer] %d번 하차단말기에 대한 이미지 다운로드 필요\n",
			nDevNo );
    }
    else
    {
        bCurrCmd = MAIN_SUB_COMM_GET_SUB_TERM_ID;
		printf( "[RecvImgVer] %d번 하차단말기에 대한 이미지 다운로드 불필요\n",
			nDevNo );
        DebugOut( "[D : 다운로드 불필요]\n" );
    }


    return ErrRet( sRetVal );
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       RecvSubTermID                                  		   *
*                                                                              *
*  DESCRIPTION:       명령어 통신을 실행하여 하차기로부터 응답받은 하차기ID를  *
*                     파싱하고 처리한다.              						   *											   *
*                   														   *
*																			   *
*  INPUT PARAMETERS:  void								                       *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공 	                                   *
* 																			   *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*                                                                              *
*******************************************************************************/
short RecvSubTermID( void )
{
    int nDevNo;
    short sRetVal;
    byte abTmpSubID[10] = { 0, };

    nDevNo = stRecvPkt.bDevNo;

    if ( stRecvPkt.nDataSize == 9 )
    {
        //하차 단말기 아이디 체크
        memcpy( abTmpSubID, stRecvPkt.abData, 9 );
        DebugOutlnASC( "tempSubId", abTmpSubID, 9);

        if ( CheckSubTermID( abTmpSubID ) == SUCCESS )
        {
            memcpy( gpstSharedInfo->abSubTermID[nDevNo-1], abTmpSubID, 9 );
            DebugOut( "\r\n SubId[%d][%s]\r\n",
                      nDevNo-1,
                      gpstSharedInfo->abSubTermID[nDevNo-1] );
            sRetVal = SUCCESS;
            DebugOut( "[G : 하차단말기ID 수신완료]\n" );
            bCurrCmd = MAIN_SUB_COMM_SUB_TERM_PARM_DOWN;
        }
        else
        {
            DebugOut( "\r\n CheckSubTermID check결과 실패-NULL값" );
            sRetVal = ERR_DATA_SUBTERM_SEND_BY_CMD_G;
        }
    }
    else
    {
        sRetVal = ERR_DATASIZE_SUBTERM_SEND_BY_CMD_G;
    }


    return ErrRet( sRetVal );
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       RecvVoiceVer                                  		   *
*                                                                              *
*  DESCRIPTION:       명령어 통신을 실행하여 하차기로부터 응답받은 음성버전을  *
*                     파싱하고 처리한다.              						   *											   *
*                   														   *
*																			   *
*  INPUT PARAMETERS:  void								                       *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공 	                                   *
* 																			   *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*                                                                              *
*******************************************************************************/
short RecvVoiceVer( void )
{
    short sRetVal = SUCCESS;
    int nDevNo = 0;
    bool boolIsVoiceVerChangeNeeded = FALSE;

    nDevNo = stRecvPkt.bDevNo;

    if ( stRecvPkt.nDataSize == 4 )
    {
        memcpy( gpstSharedInfo->abSubVoiceVer[nDevNo-1], stRecvPkt.abData, 4 );
        DebugOut( "SubTerm Voice Version Received [%s]\n",
                  gpstSharedInfo->abSubVoiceVer[nDevNo-1]);

        if  ( memcmp( gpstSharedInfo->abMainVoiceVer,
                      gpstSharedInfo->abSubVoiceVer[nDevNo-1], 4
                    )  > 0 )  //적용 Current
        {
            boolIsVoiceVerChangeNeeded = TRUE;
        }

        sRetVal = SUCCESS;
    }
    else
    {
        sRetVal = ERR_DATASIZE_SUBTERM_SEND_BY_CMD_H;
    }

    DebugOut( "[H : 음성버전 체크 완료]\n" );
    if  ( boolIsVoiceVerChangeNeeded == TRUE )
    {
        bCurrCmd = MAIN_SUB_COMM_SUB_TERM_VOIC_DOWN;
    }
    else
    {
        bCurrCmd = MAIN_SUB_COMM_REGIST_KEYSET;
        DebugOut( "[A : 음성파일 다운로드 불필요]\n" );;
    }

    return ErrRet( sRetVal );
}
/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       RecvKeySet                                  		       *
*                                                                              *
*  DESCRIPTION:       명령어 통신을 실행하여 하차기로부터 응답받은             *
*                     keyset/발행사 등록결과 정보를 파싱하고 처리한다. 		   *											   *
*                   														   *
*																			   *
*  INPUT PARAMETERS:  void								                       *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공 	                                   *
* 																			   *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*                                                                              *
*******************************************************************************/
short RecvKeySet( void )
{
    short sRetVal = SUCCESS;
    int nDevNo = 0;
    int i = 0;
	/* 
	 * 하차기 PSAM에 집계로부터 받은 Keyset/idcenter등록 성공여부 
	 */ 
    bool boolIsRegistSubTermPSAM = TRUE;

    nDevNo = stRecvPkt.bDevNo;

    if ( stRecvPkt.nDataSize == 2 )
    {
        memcpy( &stSubTermPSAMAddResult.aboolIsKeySetAdded[nDevNo-1], stRecvPkt.abData, 1 );
        memcpy( &stSubTermPSAMAddResult.aboolIsIDCenterAdded[nDevNo-1],
                &stRecvPkt.abData[1], 1 );

        DebugOut( "[K : Keyset Write 결과 수신 완료 : KeySet[%s] IDCenter[%s]\n",
                (( stSubTermPSAMAddResult.aboolIsKeySetAdded[nDevNo-1] == 0x01 )?
                    "성공" : "실패" ),
                (( stSubTermPSAMAddResult.aboolIsIDCenterAdded[nDevNo-1] == 0x01 )?
                    "성공" : "실패" ));

        if ( nIndex == gbSubTermCnt -1 ) //마지막 단말기에 대해 'K'실행 후
        {

            for( i = 0; i < gbSubTermCnt; i++ )
            {
                if ( ( stSubTermPSAMAddResult.aboolIsKeySetAdded[i] == FALSE )  ||
                     ( stSubTermPSAMAddResult.aboolIsIDCenterAdded[i] == FALSE ) )
                {
#ifdef TEST_IDCENTER_KEYSET_REGIST
                    DebugOut( "[하차기 %d번에서부터 등록실패발생]\n", i+1);
#endif
                    boolIsRegistSubTermPSAM = FALSE;
                    break;
                }
            }

            if ( ( boolIsRegistMainTermPSAM== FALSE )&&
                 ( boolIsRegistSubTermPSAM == FALSE ) )
            {
                //   승 하차모두 등록실패
                DebugOut( "[K : Keyset Write 최종결과 : 승하차모두 등록실패]\n" );
                DebugOut( "[전체 CMD실행완료]////////////////////////////////\n" );
//                ctrl_event_info_write( KEYSET_FAIL3 );
                sRetVal = SUCCESS;
                /* ctrl_event_info_write함수를 없애고 에러루틴 보강할 때 
                   에러코드 ERR_MAINSUB_COMM_ALLTERM_KEYSET_IDCENTER_ADD_PSAM;
                   로 대체  */
            }
            else if ( ( boolIsRegistMainTermPSAM == FALSE )&&
                      ( boolIsRegistSubTermPSAM == TRUE ) )
            {
                //승차기만 등록실패
                DebugOut( "[K : Keyset Write 최종결과 : 승차기만   등록실패]\n" );
                DebugOut( "[전체 CMD실행완료]////////////////////////////////\n" );
//                ctrl_event_info_write( KEYSET_FAIL1 );
                sRetVal = SUCCESS;
                /* ctrl_event_info_write함수를 없애고 에러루틴 보강할 때 
                   에러코드 ERR_MAINSUB_COMM_MAINTERM_KEYSET_IDCENTER_ADD_PSAM;
                   로 대체  */
            }
            else if ( ( boolIsRegistMainTermPSAM == TRUE )&&
                      ( boolIsRegistSubTermPSAM == FALSE ) )
            {
                //하차기만 등록실패
                DebugOut( "[K : Keyset Write 최종결과 : 하차기만 등록실패]\n" );
                DebugOut( "[전체 CMD실행완료]///////////////////////////////\n" );
//                ctrl_event_info_write( KEYSET_FAIL2 );
                /* ctrl_event_info_write함수를 없애고 에러루틴 보강할 때 
                   에러코드 ERR_MAINSUB_COMM_SUBTERM_KEYSET_IDCENTER_ADD_PSAM;
                   로 대체  */
                sRetVal = SUCCESS;
                
            }
            else
            {
                DebugOut( "[K : Keyset Write 최종결과 : 성공]\n" );
                DebugOut( "[전체 CMD실행완료]////////////////////////////////\n" );
                DebugOut( "\r\n KEYSET_IDCENTER_SUCC " );
//                ctrl_event_info_write( KEYSET_SUCC );
                sRetVal = SUCCESS;
            }

            boolIsCmdOneCycle = TRUE;
        }

        boolIsCmdOneCycle = TRUE;

    }
    else
    {
        DebugOut( "ERR_DATASIZE_SUBTERM_SEND_BY_CMD_K" );
        sRetVal = ERR_DATASIZE_SUBTERM_SEND_BY_CMD_K;
    }

    return ErrRet( sRetVal );
}
/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       RecvBLCheckReq                                  		   *
*                                                                              *
*  DESCRIPTION:       폴링에 대한 응답으로 하차기로부터 받은 BL검색요청에 따라 *
*                     BL검색 후 결과값을 저장한다.					 		   *										   *
*                   														   *
*																			   *
*  INPUT PARAMETERS:  void								                       *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공 	                                   *
* 																			   *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*                                                                              *
*******************************************************************************/
short RecvBLCheckReq( void )
{
    short sRetVal = SUCCESS;
    byte abPrefix[6] = { 0, };
    dword dwCardNum = 0;
    int nDevNo = 0;
    
    short sSearchErrLog = SUCCESS;

  //  printf( "\nRecvBLCheckReq\n" );
    boolIsCardErrLog = FALSE;
    nDevNo = stRecvPkt.bDevNo;

    memset( &stCardErrInfo, 0, sizeof( stCardErrInfo ) );
    memcpy( stCardErrInfo.abCardNo, stRecvPkt.abData, 20 );
    memcpy( abPrefix, &stRecvPkt.abData[20], 6 );
    memcpy( ( byte*)&dwCardNum, &stRecvPkt.abData[26], 4 );

    sSearchErrLog = SearchCardErrLog( &stCardErrInfo );
    if ( sSearchErrLog == SUCCESS )
    {
    	boolIsCardErrLog = TRUE;
    }
	 
	
    //DebugOut("SearchBL시작");
    sRetVal = SearchBLinBus( stCardErrInfo.abCardNo, abPrefix, dwCardNum, &bBLCheckResult );

    if ( sRetVal != SUCCESS )
    {
    	LogTerm( "RecvBLCheckReq-SearchBLinBus error 발생( %02x )\n", 
				 			 sRetVal );
    }
	/*
	* BL CHECK 성공여부 저장 - 하차기로 전송을 위해
	*/
	memcpy( &bBLCheckStatus, &sRetVal, 2); 
//	printf("bBLCheckStatus-%02d", bPLCheckStatus);	

#ifdef TEST_BLPL_CHECK
    PrintlnASC( "\r\nSubTerm BL Check Card No ==> ",
                abCardNo, 20 );
    PrintlnASC( "\r\nSubTerm BL Check Card Prefix ==> ",
                abPrefix, 6 );
    printf( "\r\nSubTerm [%02d] BL Check Card CardNum ==> %ld\n",
            nDevNo, dwCardNum );
    printf( "\r\nSubTerm [%02d] BL Check Result ==> %d\n",
            nDevNo, bBLCheckResult );
#endif

    return SUCCESS;

}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       RecvPLCheckReq                                  		   *
*                                                                              *
*  DESCRIPTION:       폴링에 대한 응답으로 하차기로부터 받은 PL검색요청에 따라 *
*                     PL검색 후 결과값을 저장한다.					 		   *										   *
*                   														   *
*																			   *
*  INPUT PARAMETERS:  void								                       *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공 	                                   *
* 																			   *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*                                                                              *
*******************************************************************************/
short RecvPLCheckReq( void )
{
    short sRetVal = SUCCESS;
    dword dwAliasNo = 0;
    int nDevNo = 0;
    short sSearchErrLog = SUCCESS;

//    printf( "\nRecvPLCheckReq\n" );
    boolIsCardErrLog = FALSE;
    nDevNo = stRecvPkt.bDevNo;

    memset( &stCardErrInfo, 0, sizeof( stCardErrInfo ) );
    memcpy( stCardErrInfo.abCardNo, stRecvPkt.abData, 20);
    memcpy( ( byte*)&dwAliasNo, &stRecvPkt.abData[20], 4 );

    
    sSearchErrLog = SearchCardErrLog( &stCardErrInfo );
    if ( sSearchErrLog == SUCCESS )
    {
    	boolIsCardErrLog = TRUE;
    }

    sRetVal = SearchPL( dwAliasNo, &bPLCheckResult );

    if ( sRetVal != SUCCESS )
    {
		LogTerm( "RecvPLCheckReq-SearchPLinBus error 발생( %02x )\n", 
				 			 sRetVal );
    }
	/*
	* PL CHECK 성공여부 저장 - 하차기로 전송을 위해
	*/
	memcpy( &bPLCheckStatus, &sRetVal, 2); 
//	printf("bPLCheckStatus-%02d", bPLCheckStatus);
    

#ifdef TEST_BLPL_CHECK
    printf( "[term_comm]하차 [%02d]번 요청 PL Check Card AliasNo ==> [%02ld,
            Result ==> %d\n\n", nDevNo, dwAliasNo, bPLCheckResult );
#endif

    return SUCCESS;
}
/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       RecvTransDataOnPolling                                   *
*                                                                              *
*  DESCRIPTION:       폴링에 대한 응답으로 하차기로부터 받은 거래내역(202BYTE) *
*                     을 승차거래내역에 추가한다.					 		   *										   *
*                   														   *
*																			   *
*  INPUT PARAMETERS:  void								                       *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공 	                                   *
* 																			   *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:  Write_Trans_Data.c의 WriteTransDataForCommProcess()함수를 호출하여*
*            승차기의 거래내역파일에 합친다.                                   *
*                                                                              *
*******************************************************************************/
short RecvTransDataOnPolling( void )
{
    short sRetVal = SUCCESS;

    if ( stRecvPkt.nDataSize == 202 )
    {
        DebugOut( "WriteTransDataForCommProcess start\n" );
        sRetVal = WriteTransDataForCommProcess( stRecvPkt.abData );
        DebugOut( "WriteTransDataForCommProcess end\n" );
    }
    else
    {
        DebugOut( "하차거래내역 파일 사이즈 202byte아님\n" );
        sRetVal = ERR_DATASIZE_SUBTERM_SEND_BY_CMD_X;
		LogTerm( "하차거래내역 파일 사이즈 202byte아님\n" );
        //또는 데이타가 없을 경우
    }

	if ( sRetVal < 0 )
	{
		LogTerm( "거래내역 데이터수신 오류 %02x\n", sRetVal );
	}
    return ErrRet( sRetVal );
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       RecvTDFileFromSubTerm                            		   *
*                                                                              *
*  DESCRIPTION:       하차기로부터 대사를 위한 거래내역파일을 수신한다.        *
*                   														   *
*																			   *
*  INPUT PARAMETERS:  int  nDevNo - 하차단말기 번호                            *
*                     byte *pFileName - 대사파일명                             *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공 	                                   *
* 																			   *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*                                                                              *
*******************************************************************************/
short RecvTDFileFromSubTerm( int  nDevNo, byte *pFileName )
{

    short sRetVal = SUCCESS;
    char achRecvFileName[19] = { 0, };
    char achRecvFileReName[19] = { 0, };

    usleep( 100000 );
    memcpy( achRecvFileName, pFileName, 14 );
    memcpy( &achRecvFileName[14], ".tmp", 4 );

	printf( "[RecvTDFileFromSubTerm] 하차로부터 수신 : %s ", achRecvFileName );

    sRetVal = RecvFile( nDevNo, achRecvFileName );

    if (( sRetVal < 0 ) && ( sRetVal != ERR_MAINSUB_COMM_REQ_FILE_NOT_EXIST ))
    {
#ifdef TEST_DAESA
        printf( "\r\n 하차기로부터 TD파일 수신  실패 : [%d]\r\n", sRetVal );
        printf( "\r\n 기존 승차 통신 프로세스 종료\n" );
#endif
        unlink( achRecvFileName );
    }
    else if ( sRetVal == ERR_MAINSUB_COMM_REQ_FILE_NOT_EXIST)
    {
        unlink( achRecvFileName );
#ifdef  TEST_DAESA
        printf( "승차기요청 거래내역 파일이 없어 임시화일삭제\n" );
#endif
    }
    else
    {
        memcpy( achRecvFileReName, pFileName, 15 );
        achRecvFileReName[15] = nDevNo+ '0' ;
        achRecvFileReName[16] = '0';

        DebugOut( "\r\n recv Temp Filerename [%s] \r\n", achRecvFileReName );
        rename( achRecvFileName, achRecvFileReName );
        DebugOut( "\r\n SubTermTDFILE End ret : [%d]", sRetVal );
#ifdef  TEST_DAESA
        printf( "\r\n TD file 생성 성공\r\n" );
#endif
    }

    return ErrRet( sRetVal );
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       ChkSubTermImgVerOldProtcol                               *		       *
*                                                                              *
*  DESCRIPTION:       하차기가 구프로토콜로 되어있는지 검사한다.               *
*                   														   *
*																			   *
*  INPUT PARAMETERS:  int nDevNo - 하차단말기 번호                             *
*                     char *pachhNewVerYn - 신프로토콜 여부 T: 신 F:구         *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공 	                                   *
* 																			   *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*                                                                              *
*******************************************************************************/
short ChkSubTermImgVerOldProtcol( int nDevNo, char *pachhNewVerYn )
{
    short sRetVal;

    RS485_FILE_HEADER_PKT_OLD stFileHeaderPkt;
	memset( &stFileHeaderPkt, 0x00, sizeof(RS485_FILE_HEADER_PKT_OLD));

#ifdef TEST_SYNC_FW
    printf( "==========version check start!!!===========\n" );
#endif
    /* 버전체크 명령어 */
    stSendPkt.bCmd = MAIN_SUB_COMM_SUB_TERM_IMG_VER;
	/* 단말기 번호 */
    stSendPkt.bDevNo = nDevNo + '0';
	/* 데이터 사이즈 */
    stSendPkt.nDataSize = sizeof( RS485_FILE_HEADER_PKT_OLD );

    /* 1.CSAMID BCD변환 후 stFileHeaderPkt 구조체 저장 */
    ASC2BCD( gpstSharedInfo->abMainCSAMID, stFileHeaderPkt.achFileName, 8 );
	/* 2.PSAMID BCD변환 후 stFileHeaderPkt 구조체 저장 */
    ASC2BCD( gpstSharedInfo->abMainPSAMID, &stFileHeaderPkt.achFileName[4], 16 );
	
	/* 3.승차단말기 ID stFileHeaderPkt 구조체 저장 */
    memcpy( &stFileHeaderPkt.achFileName[12], gpstSharedInfo->abMainTermID, 9 );
    /* 4.승차기 버전 stFileHeaderPkt 구조체 저장 */
    memcpy( &stFileHeaderPkt.achFileName[21], gpstSharedInfo->abMainVer, 4 );
	/* 송신구조체 데이터 영역에 단말기번호 재저장 */
    memcpy( &stSendPkt.abData[0], &stSendPkt.bDevNo, sizeof( stSendPkt.bDevNo ) );
	/* 1,2,3,4에서 저장한 stFileHeaderPkt구조체를 송신구조체에 데이터 영역에 복사 */	
    memcpy( &stSendPkt.abData[1], &stFileHeaderPkt, sizeof(RS485_FILE_HEADER_PKT_OLD));

	/* 
	 * 데이터 송신 
	 */ 
    sRetVal = SendPkt();
    if ( sRetVal < 0 )
    {
        printf( "\r\n ChkSubTermImgVerOldProtcol rs485PktSend sRetVal[%d] \r\n", 
				sRetVal );
        return sRetVal;
    }

	/* 
	 * 데이터 수신  
	 */
    sRetVal = RecvPkt( 1000, nDevNo );
    if ( sRetVal < 0 )
    {
        printf( "\r\n ChkSubTermImgVerOldProtcol rs485PktRecv sRetVal[%d] \r\n",
				sRetVal );
        return sRetVal;
    }
	/* 
	 * 구 프로토콜로 ACK수신시 프로그램 버전데이터 수신후 신/구버전 판단.
	 */
    if ( stRecvPkt.nDataSize == 1 && stRecvPkt.abData[0] == ACK )
    {
#ifdef TEST_SYNC_FW    
        printf( "V Command ACK Received [%d]\n", nDevNo );
#endif
        /* 
         * Version 데이터 수신 
         */
        sRetVal = RecvPkt( 1000, nDevNo );
        /* 
         * nDataSize 데이터 수신 
         * - 구 프로토콜 버전데이터 사이즈는 17
         *   STX LEN LEN CMD DEVNO DATA ETX CRC CRC CRC CRC
         */
	    DebugOut( "구 프로토콜로 프로그램 version 만든다\n" );
	    /*
	     *  program version copy- 4byte
	     *  new - 3byte
	     *  ? - 1byte
	     * 0000000000 - 8byte
	     * 포트번호 0x32 - 1byte 
	     */
	    /*
	     *  new를 버전에 덧붙여 보낸다.
	     *  이유 : 0401이후의 버전, 즉 0402,0403이 꼭 신프로토콜이라고 할 수 없다.
	     *         0401이 승하차통신에 문제가 생겨 이전 구 프로토콜로 복원하여
	     *         0402,0403을 만들어서 배포할 경우 버전만가지고는 프로트콜의 
	     *         신,구여부를 판단하기에 어려우므로 new를 덧붙여 버전이 04xx대라도
	     *         승차기에서 new를 수신하지 못하여 구프로토콜로 간주한다.
	     */    
		
		/*
		 * TPSAM 확인용으로 1byte늘어나서 18byte가 되었음 
		 */
        if ( stRecvPkt.nDataSize == 17 || stRecvPkt.nDataSize == 18 )
        {
        	printf("[%d] 번", nDevNo );
        	PrintlnASC( "하차기 Ver :", gpstSharedInfo->abSubVer[nDevNo-1], 4);
			/* 하차기 버전 공유메모리에 Copy  */ 
            memcpy( gpstSharedInfo->abSubVer[nDevNo-1], stRecvPkt.abData, 4 );

            if ( memcmp( &stRecvPkt.abData[4], "new", 3 ) == 0 )
            {
			    /* new라는 데이터가 오면 하차기는 신프로토콜 버전  */            
                pachhNewVerYn[nDevNo-1] = SUBTERM_USE_NEW_PROTOCOL;
                printf( "[%d] 번 하차기 F/W는 신프로토콜 사용 버전\n", nDevNo );
            }
            else
            {
				/* new라는 데이터가 오지않으면  구프로토콜 버전 */                  
                pachhNewVerYn[nDevNo-1] = SUBTERM_USE_OLD_PROTOCOL;
                printf( "[%d] 번 하차기 F/W는 구프로토콜 사용 버전\n", nDevNo );

                return sRetVal;
            }
        }
        else
        {
            printf( "VersionTest After ACK Received..\n" );
            printf( "Data = %s====\n", stRecvPkt.abData );
        }

        return SUCCESS;
    }
	
	/* 
	 * 구 프로토콜로 NAK수신시 
	 */	
    else if ( stRecvPkt.nDataSize == -1 && stRecvPkt.abData[0] == NAK )
    {
        DebugOut ( "V Command NAK Received [%d]\n", nDevNo );
    }
	/* 
	 * 그 이외의 값일 경우 
	 */		
    else
    {
        DebugOut( "VersionTest Received Data = %s====\n", stRecvPkt.abData );
    }

    return sRetVal;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       SendSubTermImg				                           *		       *
*                                                                              *
*  DESCRIPTION:       구 프로토콜을 사용하여 하차기 프로그램 다운로드 명령어를 *
*                     수행한다.                                                *
*                   														   *
*																			   *
*  INPUT PARAMETERS:  int nDevNo - 단말기 번호                                 *
*                     char* pchFileName - 전송할 하차기프로그램 파일           *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공 	                                   *
* 																			   *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*                                                                              *
*******************************************************************************/
short SendSubTermImgOldProtcol( int nDevNo, char* pchFileName )
{
    short sRetVal;
    int fdFile = -1;
    RS485_FILE_HEADER_PKT_OLD fileHeaderPkt;/* 전송구조체에 넣기 위한 임시변수 */

    /* 초기화 */
    memset( &fileHeaderPkt, 0, sizeof( RS485_FILE_HEADER_PKT_OLD ) );
	/* 
	 * 전송할파일명 임시변수에 저장 
	 * 03xx는 c_ex_pro.dat파일을 전송 받을 파일명으로 인지 하므로 
	 * 전송파일 명 c_ex_pro.dat로 설정한다. 
	 * 하지만 파라미터 pchFileName는 bus100으로 설정한다.
	 * 하차기에 bus100 + 18byte(일시버전)을 붙여서 보내야 하므로...
	 */
    strcpy( fileHeaderPkt.achFileName, BUS_SUB_IMAGE_FILE );

    /* bCmd : 명렁어 설정 */
    stSendPkt.bCmd = MAIN_SUB_COMM_SUB_TERM_IMG_DOWN_OLD;
    /* bDevNo : 단말기 번호 설정 */
    stSendPkt.bDevNo = nDevNo + '0';
    /* nDataSize : 단말기 번호 설정 */	
    stSendPkt.nDataSize = sizeof( RS485_FILE_HEADER_PKT_OLD );

    /* abData에 bDevNo 저장 */	
    memcpy( &stSendPkt.abData[0], &stSendPkt.bDevNo, sizeof( stSendPkt.bDevNo ) );
    /* abData에 임시변수인 fileHeaderPkt를 Copy */	
    memcpy( &stSendPkt.abData[1], &fileHeaderPkt, sizeof( RS485_FILE_HEADER_PKT_OLD ) );

   /*
    * 패킷 전송
    */
    sRetVal = SendPkt();
    if ( sRetVal < 0 )
    {
        printf( "\r\n Upload rs485PktSend sRetVal [%d] \r\n", sRetVal );
        return ErrRet( sRetVal );
    }
   /*
    * 패킷 수신
    */
    sRetVal = RecvPkt( 1000, nDevNo );
    if ( sRetVal < 0 )
    {
        printf( "\r\n Upload rs485PktRecv sRetVal [%d] \r\n", sRetVal );
        return ErrRet( sRetVal );
    }
   /*
    * ACK수신시 
    */
    if ( stRecvPkt.nDataSize == 1 && stRecvPkt.abData[0] == ACK )
    {
        DebugOut( "ACK Received\n" );

        DisplayCommUpDownMsg( 1 , 2 );
        usleep( 100000 );
		
		/* 구 프로토콜을 이용하여 파일전송 시작 */
		printf(" 구 프로토콜을 이용하여 파일전송 시작 \n");
        sRetVal = SendSubTermImgFileOldProtocol( nDevNo, pchFileName );
        if ( sRetVal < 0 )
        {
            DebugOut( "\r\n SendSubImgFileOld sRetVal [%d] \r\n", sRetVal );
        }

        DisplayCommUpDownMsg( 2 , 2 );

        return ErrRet( sRetVal );
    }
   /*
    * NAK수신시  
    */	
    else if ( stRecvPkt.nDataSize == -1 && stRecvPkt.abData[0] == NAK )
    {
        DebugOut( "NAK Received\n" );
    }

    close( fdFile );

    return ErrRet( sRetVal );
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       SendSubTermImgFileOldProtocol                            *		       
*                                                                              *
*  DESCRIPTION:       03xx버전이 사용하는 실행이미지인 bus100에 파일에 일자와  *
*                     버전을 덧붙여 구프로토콜로 전송한다.                     *
*                   														   *
*																			   *
*  INPUT PARAMETERS:  int nDevNo - 단말기 번호                                 *
*                     char* pchFileName - 전송할 하차기프로그램 파일           *
* 																			   *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공 	                                   *
*                     음수값 - 실행 실패                                       *
* 																			   *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     	 신/구 프로토콜의 최대패킷사이즈가 서로 달라 데이터를      *
*                    위한 변수를 원래 stSendPkt구조체의 abData변수를 사용      *
*                    하지 않고 abSendData로 별도 선언하여 사용하고 있다.       *
*                    이를 알려주기위한 전역변수로 boolIsMainSubCommFileDownIng *
*                    Flag을 사용하고 있다.                                     *
*                                                                              *
*******************************************************************************/
short SendSubTermImgFileOldProtocol( int nDevNo, char* pchFileName )
{
    int fdFile;
    int nByte;
    short sRetVal;
    int nEOTCheck = 0;
    /* 현재일시와 승차기펌웨어 버전 */	
	byte abCurrDataNVer[18] = { 0, }; 
	time_t tCurrentDtime = 0;
   /* 
    * 구 프로토콜로 전송하기위한 구조체 초기화
    */
	memset( abSendData, 0x00, sizeof( abSendData ));
   /* 
    * bus100이미지를 패킷으로 보내고 뒤에 현재시간과 메인버전을 덧붙여 보내기 
    */
   /*
    * 현재시간 14byte Copy
    */	
    GetRTCTime( &tCurrentDtime );
    TimeT2ASCDtime( tCurrentDtime, abCurrDataNVer );
    abCurrDataNVer[12] = '0';
    abCurrDataNVer[13] = '0';
   /* 
    * 메인버전과 동일하게 버전 4byte도 Copy
	*/
    memcpy( &abCurrDataNVer[14], MAIN_RELEASE_VER, 4 );

	
   /*
    * BUS100 파일 오픈 
    */
    fdFile = open( pchFileName, O_RDONLY, OPENMODE );
    if ( fdFile < 0 )
    {
        DebugOut( "\r\n 파일 오픈 안됨\r\n" );
        return -1;
    }
	/*
     * BUS100파일의 끝에 일시와 승차기버전 추가 
     */
    write( fdFile, abCurrDataNVer, 14 );
    write( fdFile, MAIN_RELEASE_VER, 4 );

	lseek( fdFile, 0L, SEEK_SET );
		
   /*
    * 파일 다운로드 중임을 나타내는 전역변수 설정
    * TRUE :  SendPkt()함수내에서 데이터버퍼에 abSendData값을 Copy하여 전송
    * FALSE :  SendPkt()함수내에서 데이터버퍼에 stSendPkt.abData값을 Copy하여 전송
    */
    boolIsMainSubCommFileDownIng = TRUE;

	while(1)
    {
		/* 명령어 : 구프로토콜을 이용한 하차기 펌웨어 다운로드 'D'*/
		stSendPkt.bCmd = MAIN_SUB_COMM_SUB_TERM_IMG_DOWN_OLD;
		/* 단말기 번호 : 단말기번호 + ASCII 0 */		
        stSendPkt.bDevNo = nDevNo + '0';   
		
        nByte = read( fdFile, abSendData, DOWN_DATA_SIZE_OLD );

        DebugOut( "nByte %d \n", nByte );

        if ( nByte > 0 )
        {
        	/* 데이터 사이즈 - read byte */        
            stSendPkt.nDataSize = nByte;
        }
        else if ( nByte == 0 )
        {
            DebugOut( "화일끝으로 들어오다 %d \n", nByte );
		   /*
		    * EOT 패킷 생성
		    */ 			
            stSendPkt.nDataSize = 1;
            abSendData[0] = EOT;
            nEOTCheck = 1;
        }
        else
        {
            sRetVal = -1;
            break;
        }

    	/*
	     * 패킷 전송
	     */
        sRetVal = SendPkt();
        if ( sRetVal < 0 )
        {
            break;
        }
		/*
	     * 응답 수신
	     */      
   	    sRetVal = RecvPkt( 4000, nDevNo );
        if ( sRetVal < 0 )
        {
            printf( "\r\n 파일 전송중 응답없음[%d]\r\n", sRetVal );
            break;
        }
		/*
	     * 패킷에 EOT를 사용한 후 ACK수신한 경우가 정상다운로드 임
	     */   
        if ( stRecvPkt.nDataSize == 1 &&
             stRecvPkt.abData[0] == ACK &&
             nEOTCheck == 1 )
        {
            printf( "\r\n 정상 다운로드 !\r\n" );
            sRetVal =  SUCCESS;
            break;
        }

    }

    boolIsMainSubCommFileDownIng = FALSE;
    close( fdFile );

    return sRetVal;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       CheckSubTermID				                           *		       *
*                                                                              *
*  DESCRIPTION:       수신한 하차단말기 ID 데이터를 검증한다.                  *
*                   														   *
*																			   *
*  INPUT PARAMETERS:  byte *pbCheckID - 하차단말기 ID                          *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공 	                                   *
* 																			   *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*                                                                              *
*******************************************************************************/
short CheckSubTermID( byte *pbCheckID )
{

    int i = 0;
    int nID = 0;

    /*
    * 하차단말기 ID 자릿수 검증 
    */
    if ( strlen( pbCheckID ) != 9 )
    {
        return -1;
    }
    /*
    * 하차단말기 ID가 버스인지 검증 - 버스는 15로 시작함.
    */
    if ( memcmp( pbCheckID, "15", 2 ) != 0 )
    {
        return -1;
    }
    /*
    * 각 자릿수 숫자(numeric)여부 검증
    */
    for( i = 0; i < 9 ; i++ )
    {
        nID = GetINTFromASC( &pbCheckID[i], 1 );
        if ( nID < 0 || nID > 9 )
        {
            return -1;
        }
    }

    return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       CheckDCSCommParameterReq   	                           *		       *
*                                                                              *
*  DESCRIPTION:       수신한 하차단말기 ID 데이터를 검증한다.                  *
*                   														   *
*																			   *
*  INPUT PARAMETERS:  bool *pboolIsDCSReady					                   *
*                     byte *pbCmdData 										   *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공 	                                   *
* 																			   *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*                                                                              *
*******************************************************************************/
void CheckDCSCommParameterReq( bool *pboolIsDCSReady, byte *pbCmdData )
{

    short sRetVal = SUCCESS;
    byte abCmdData[2] = { 0, };

    if ( pbCmdData[1] == CMD_REQUEST )
    {

        sRetVal = GetLEAPPasswd();
        if ( sRetVal != SUCCESS )
        {
            abCmdData[0] = CMD_FAIL_RES;
            sRetVal = SetSharedDataforCmd( CMD_PARMS_RESET, abCmdData, 1 );

        }
        else
        {
            abCmdData[0] = CMD_SUCCESS_RES;
            sRetVal = SetSharedDataforCmd( CMD_PARMS_RESET, abCmdData, 1 );

            *pboolIsDCSReady = TRUE;

        }
    }

    if ( pbCmdData[2] == CMD_REQUEST )
    {
        sRetVal = LoadInstallInfo();
        if ( sRetVal < 0 )
        {
            abCmdData[0] = CMD_FAIL_RES;
            sRetVal = SetSharedDataforCmd( CMD_PARMS_RESET, abCmdData, 1 );
        }
        else
        {
            abCmdData[0] = CMD_SUCCESS_RES;
            sRetVal = SetSharedDataforCmd( CMD_PARMS_RESET, abCmdData, 1 );
            *pboolIsDCSReady = TRUE;
        }
    }

}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       PSAMIDCompareMainWithSubTerm	                           *	
*                                                                              *
*  DESCRIPTION:       명령어 통신을 통해 수신한 하차기의 PSAMID 및 승차기의    *
*                     PSAMID를 PSAMID파일(simid.flg)내의 PSAMID와 비교한다.    *
*                     틀릴경우에는 SAM등록정보를 초기화한다.            	   *
*																			   *
*  INPUT PARAMETERS:  void 									                   *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공 	                                   *
* 																			   *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*                                                                              *
*******************************************************************************/
short PSAMIDCompareMainWithSubTerm( void )
{

    FILE* fdPSAM;
    int nRetVal;
	byte i = 0;
    byte abPSAMId[65] = { 0, };
    byte abMainTermPSAMIDInSharedMemory[17] = { 0, };
    byte abSubTermPSAMIDInSharedMemory[3][17] ;
    byte abMainTermPSAMIDInFile[17] = { 0, };
    byte abSubTermPSAMIDInFile[3][17] ;

    memset( abSubTermPSAMIDInSharedMemory, 0,
            sizeof( abSubTermPSAMIDInSharedMemory ) );
    memset( abSubTermPSAMIDInFile, 0, sizeof( abSubTermPSAMIDInFile ) );

    nRetVal = access( PSAMID_FILE, F_OK );

    system ( "chmod 755 /mnt/mtd8/bus/*" );

    if ( nRetVal != 0 )
    {
        if ( ( fdPSAM = fopen( PSAMID_FILE, "wb+" ) )  == NULL )
        {
            DebugOut( "\r\n  <MAIN> Error!!! open() failed\n" );
            return -1;
        }
    }
    else
    {
        DebugOut( "\r\n  7.2.4 \r\n" );
        if ( ( fdPSAM = fopen( PSAMID_FILE, "rb+" ) ) == NULL )
        {
            DebugOut( "Error!!! open() failed\n" );
            return -1;
        }

        fread( abPSAMId, sizeof( abPSAMId ), 1, fdPSAM );
    }

    /* simid.flg에서 PSAMID 읽기 */
    memcpy( abMainTermPSAMIDInFile, abPSAMId, 16 );
    memcpy( abSubTermPSAMIDInFile[0], &abPSAMId[16], 16 );
    memcpy( abSubTermPSAMIDInFile[1], &abPSAMId[32], 16 );
    memcpy( abSubTermPSAMIDInFile[2], &abPSAMId[48], 16 );

    /*
     * 공유메모리에서 PSAMID 읽기
     */
    memcpy( abMainTermPSAMIDInSharedMemory,   gpstSharedInfo->abMainPSAMID, 16);
    memcpy( abSubTermPSAMIDInSharedMemory[0],
            gpstSharedInfo->abSubPSAMID[0], 16);
    memcpy( abSubTermPSAMIDInSharedMemory[1],
            gpstSharedInfo->abSubPSAMID[1], 16);
    memcpy( abSubTermPSAMIDInSharedMemory[2],
            gpstSharedInfo->abSubPSAMID[2], 16);

	if ( IsAllZero( abMainTermPSAMIDInSharedMemory,
		sizeof( abMainTermPSAMIDInSharedMemory ) ) )
	{
		memset( abMainTermPSAMIDInSharedMemory, '0',
			sizeof( abMainTermPSAMIDInSharedMemory ) );
	}
	for ( i = 0; i < 3; i++ )
	{
		if ( IsAllZero( abSubTermPSAMIDInSharedMemory[i],
			sizeof( abSubTermPSAMIDInSharedMemory[i] ) ) )
		{
			memset( abSubTermPSAMIDInSharedMemory[i], '0',
				sizeof( abSubTermPSAMIDInSharedMemory[i] ) );
		}
	}

    DebugOut( "\r\n <simidLoadCompare> in MainTermPSAMIDInSharedMemory [%s]",
              abMainTermPSAMIDInSharedMemory);
    DebugOut( "\r\n <simidLoadCompare> in SubTermPSAMIDInSharedMemory 0 [%s]",
              abSubTermPSAMIDInSharedMemory[0]);
    DebugOut( "\r\n <simidLoadCompare> in SubTermPSAMIDInSharedMemory 1 [%s]",
              abSubTermPSAMIDInSharedMemory[1]);
    DebugOut( "\r\n <simidLoadCompare> in SubTermPSAMIDInSharedMemory 2 [%s]",
              abSubTermPSAMIDInSharedMemory[2]);
    DebugOut( "\r\n 초기부팅시에는 승차 PSAMID만 이후는 모든 값이
              in simid.flg 에서 읽은 값 [%s]", abPSAMId );
    /*
     * 공유메모리의 값과 파일내 PSAMID가 서로 다른지 비교 
     */
    if ( ( memcmp( abMainTermPSAMIDInSharedMemory,
                   abMainTermPSAMIDInFile, 16 ) != 0 ) ||
         ( memcmp( abSubTermPSAMIDInSharedMemory[0],
                   abSubTermPSAMIDInFile[0], 16 ) != 0 ) ||
         ( memcmp( abSubTermPSAMIDInSharedMemory[1],
                   abSubTermPSAMIDInFile[1], 16) != 0 ) ||
         ( memcmp( abSubTermPSAMIDInSharedMemory[2],
                   abSubTermPSAMIDInFile[2], 16) != 0 ) )
    {
        DebugOut( "공유메모리와 simid.flg이 서로 다르다\n" );
        DebugOut( "\n <simidLoadCompare> in MainTermPSAMIDInSharedMemory [%s]",
                  abMainTermPSAMIDInSharedMemory);
        DebugOut( "\n <simidLoadCompare> in SubTermPSAMIDInSharedMemory 0 [%s]",
                  abSubTermPSAMIDInSharedMemory[0]);
        DebugOut( "\n <simidLoadCompare> in SubTermPSAMIDInSharedMemory 1 [%s]",
                  abSubTermPSAMIDInSharedMemory[1]);
        DebugOut( "\n <simidLoadCompare> in SubTermPSAMIDInSharedMemory 2 [%s]",
                  abSubTermPSAMIDInSharedMemory[2]);
        DebugOut( "\n <simidLoadCompare> in 공유메모리에 있는거 저장한 값 [%s]",
                  abPSAMId);
	    /*
	     * 공유메모리의 PSAMID값을 변수에 저장 
	     */
        memcpy( abPSAMId,      abMainTermPSAMIDInSharedMemory,   16 );
        memcpy( &abPSAMId[16], abSubTermPSAMIDInSharedMemory[0], 16 );
        memcpy( &abPSAMId[32], abSubTermPSAMIDInSharedMemory[1], 16 );
        memcpy( &abPSAMId[48], abSubTermPSAMIDInSharedMemory[2], 16 );
	    /*
	     * SAM등록정보 초기화 후 공유메모리으 PSAMID를 PSAMID파일(simid.flg)에 저장
	     */		
        if ( InitSAMRegistInfoVer() == SUCCESS )
        {
            fseek( fdPSAM, 0, SEEK_SET );
            fwrite( abPSAMId, sizeof(abPSAMId), 1, fdPSAM );
        }

        fclose( fdPSAM );

        return -1;
    }

    fclose( fdPSAM );

    return 1;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       SendSubTermImgFile                             	       *
*                                                                              *
*  DESCRIPTION:       승차기의 실행이미지인 bus100파일에 일시버전 18byte를 더해*
*                     하차기로 전송한다.   								       *
*                                                                              *
*  INPUT PARAMETERS:  int nDevNo- 단말기번호 				                   *
*                     char* pchFileName - 송신대상 파일명 				       *
* 																			   *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공 	                                   *
*                     ERR_MAINSUB_COMM_SUBTERM_IMG_FILE_OPEN                   *
*                      - bus100실행이미지 파일 오픈 에러                 	   *
*                     ERR_MAINSUB_COMM_SUBTERM_IMG_DOWN_NAK                    *
*                      - 파일전송중 NAK로 응답                                 *
*                                                                              *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:       	 송신할 파일이 존재하는 경우에는 파일의 마지막 패킷에      *
*                    마지막 패킷의 의미로 EOT를 전송                           *
*                    각 패킷 전송마다 ACK로서 패킷전송완료를 확인한다.         *
*																			   *
*******************************************************************************/
short SendSubTermImgFile( int nDevNo, char* pchFileName )
{
    int fdFile;
    short sRetVal = SUCCESS;
    bool boolIsFileEnd = FALSE;   /* 파일의 끝을 나타내는 Flag */         
	int nByte;					  /* 송신파일에서 읽은 Byte수 */
    word wSeqNo = 0;			  /* Sequence 번호 */
	int nCopyLen	= 0;          /* 일시버전(18byte)중 패킷에 덧붙일 길이 */
	/* 
	 * 1012byte로 파일에서 데이터를 Read 했는지 여부를 나타내는 Flag
	 * 이유 - bus100에서 마지막으로 읽은 데이터 크기가 정확인 1012인 경우 
	 *        EOT전송전에 일시버전(18byte)를 추가로 데이터를 전송해주어야 하므로 
	 */ 
	bool boolIsMaxPktSendYN	= FALSE;  
    /* 현재일시와 승차기펌웨어 버전 */	
	byte abCurrDataNVer[18] = { 0, }; 
	time_t tCurrentDtime = 0;
  
    usleep( 100000 );

   /* 
    * bus100이미지를 패킷으로 보내고 뒤에 현재시간과 메인버전을 덧붙여 보내기 
    */
   /*
    * 현재시간 14byte Copy
    */	
    GetRTCTime( &tCurrentDtime );
    TimeT2ASCDtime( tCurrentDtime, abCurrDataNVer );
    abCurrDataNVer[12] = '0';
    abCurrDataNVer[13] = '0';
   /* 
    * 메인버전과 동일하게 버전 4byte도 Copy
	*/
    memcpy( &abCurrDataNVer[14], MAIN_RELEASE_VER, 4 );

   /*
   	* 전송할 파일 열기
   	*/
    fdFile = open( pchFileName, O_RDONLY, OPENMODE );

    if ( fdFile < 0 )
    {
        printf( "\r\n 파일 오픈 안됨\r\n" );
	 	return ERR_MAINSUB_COMM_SUBTERM_IMG_FILE_OPEN;
	}
   /*
   	* 파일전송 Loop 시작 
   	*/
    while( 1 )
    {
    	/* 명령어 : 하차기 펌웨어 다운로드 'd'*/
   		stSendPkt.bCmd = MAIN_SUB_COMM_SUB_TERM_IMG_DOWN;
		/* 단말기 번호 : 단말기번호 + ASCII 0 */
        stSendPkt.bDevNo = nDevNo + '0'; 
	   /*
	   	* 전송할 파일 최대 1012byte까지 데이터영역에 읽기  
	   	*/    
        nByte = read( fdFile, stSendPkt.abData, DOWN_DATA_SIZE );

	   /*
	    * 1. 최대 Read size와 같다면 
	    */ 
	   	if ( nByte == DOWN_DATA_SIZE )
        {
			/* nDataSize(데이터사이즈) : 파일에서 읽은 byte  */ 					
            stSendPkt.nDataSize = nByte;     
			/* wSeqNo(시퀀스 번호) : 패킷의 시퀀스 번호  */ 
            stSendPkt.wSeqNo = wSeqNo;        
			/* 1012 Byte로 전송 Flag 설정*/
			boolIsMaxPktSendYN = TRUE;			
        }
	   /*
	    * 2. Read Byte가 0보다 크고 1012byte 사이면 
	    */		
		else if ( nByte > 0 && nByte < DOWN_DATA_SIZE )
		{
			/* 1024 Byte로 전송 Flag 설정*/
			boolIsMaxPktSendYN = FALSE;
			/* 
			 * 1) 마지막 패킷과 일시버전을 더한것이 최대 Read Size보다 큰 경우 
			 *    두 패킷으로 나누어 전송
			 */
			if ( nByte + sizeof(abCurrDataNVer) > DOWN_DATA_SIZE )	 
			{
				/* 
				 * nCopyLen : 일시버전 18byte중 몇바이트를 덧붙여야 1012byte
				 *            가 되는지 계산
				 */ 			
				nCopyLen = DOWN_DATA_SIZE - nByte;
				/* nDataSize(데이터사이즈) : 1012byte 
				 * 파일에서 읽은 byte와 일시버전 17byte중 n byte를 1012가 되도록
				 * 합쳤기 때문
				 */ 
				stSendPkt.nDataSize = DOWN_DATA_SIZE;
				/* wSeqNo(시퀀스 번호) : 패킷의 시퀀스 번호  */ 
	            stSendPkt.wSeqNo = wSeqNo;      
				 /* 1-1).데이터에 일시버전 Byte중 nCopyLen길이 만큼 추가 */ 
				memcpy( &stSendPkt.abData[nByte], abCurrDataNVer, nCopyLen );

				/*
			     * 마지막 데이터 패킷 전송
			     */
		        sRetVal = SendPkt();
		        if ( sRetVal < 0 )
		        {
		            break;
		        }
				/*
			     * 마지막 데이터 패킷 응답 수신
			     */      
			    sRetVal = RecvPkt( 4000, nDevNo );
		        if ( sRetVal < 0 )
		        {
		            DebugOut( "\r\n 파일 전송중 응답없음[%d]\r\n", sRetVal );
		            break;
		        }
				/*
			     * 1-2).일시버전 Byte중 nCopyLen만큼 1.에서 추가하고 남은 데이터를
			     *     데이터에 추가하여 전송  
			     */ 
				/* nDataSize(데이터사이즈) : 일시버전 18byte - 앞패킷에 추가한 byte */ 					
	            stSendPkt.nDataSize = sizeof(abCurrDataNVer)-nCopyLen; 
				/* wSeqNo(시퀀스 번호) : 패킷의 시퀀스 번호  */ 
	            stSendPkt.wSeqNo = ++wSeqNo;        			     
			    /* 데이터에 일시버전 잔여 Byte를 넣어 생성  */ 
				memcpy( stSendPkt.abData, 
						&abCurrDataNVer[nCopyLen],
						sizeof(abCurrDataNVer)-nCopyLen );
				
				
			}
			/* 
			 * 2) 마지막 패킷과 일시버전을 더한것이 최대 Read Size보다 작은 경우 
			 *    마지막 데이터 패킷에 일시버전을 덧붙여 전송하고 EOT를 전송한다. 
			 */			
			else	
			{
			    /* nDataSize(데이터사이즈) : read byte + 일시버전 18byte */ 
				stSendPkt.nDataSize = nByte + sizeof(abCurrDataNVer);
				/* wSeqNo(시퀀스 번호) : 패킷의 시퀀스 번호  */ 
	            stSendPkt.wSeqNo = wSeqNo;  
				/* 2-1).파일에서 읽은 데이터에 일시버전 18byte 모두를 Copy  */ 
				memcpy( &stSendPkt.abData[nByte], abCurrDataNVer, sizeof(abCurrDataNVer) );
			}


		}	
		
	   /*
	   	* 읽어 들인 데이터가 0인 경우 즉, 파일의 끝인경우 EOT 패킷 생성
	   	*/  		
        else if ( nByte == 0 ) 
        {
            DebugOut( "화일끝으로 들어오다 %d \n", nByte );        
   			/*
			* 데이터 패킷이 정확히 1012byte로 읽힌 경우 일시버전은 미전송상태
			* 이르로 일시버전만 한 패킷을 만들어 전송
			*/
			if ( boolIsMaxPktSendYN == TRUE )	
			{
				stSendPkt.nDataSize = sizeof(abCurrDataNVer);
				/* 일시버전 데이터(18byte) Copy  */				
				memcpy( stSendPkt.abData, abCurrDataNVer, sizeof(abCurrDataNVer) );
				/* wSeqNo(시퀀스 번호) : 패킷의 시퀀스 번호  */ 		
	            stSendPkt.wSeqNo = wSeqNo++;				
				
				/*
			     * 일시버전 패킷 전송
			     */
		        sRetVal = SendPkt();
		        if ( sRetVal < 0 )
		        {
		            break;
		        }
				/*
			     * 응답 수신
			     */      
			    sRetVal = RecvPkt( 4000, nDevNo );
		        if ( sRetVal < 0 )
		        {
		            DebugOut( "\r\n 파일 전송중 응답없음[%d]\r\n", sRetVal );
		            break;
		        }
			}
		   /*
		    * EOT 패킷 생성
		    */         

			/* Cmd(커맨드) : 현재 단말기가 실행중인 커맨드 */			
            stSendPkt.bCmd = bCurrCmd;
			/* bDevNo(단말기번호) : 단말기번호 + ASCII 0   */					
            stSendPkt.bDevNo = nDevNo + '0';
			/* nDataSize(데이터사이즈) : 1  */ 			
            stSendPkt.nDataSize = 1;
			/* wSeqNo(시퀀스 번호) : 패킷의 시퀀스 번호  */ 	
           	stSendPkt.wSeqNo = wSeqNo;
			/* abData[0] : 데이터에 EOT를 넣어 수신측에서 파일끝을 표시 */ 
            stSendPkt.abData[0] = EOT;
		   /*
		   	* 파일 끝 표시 
		   	*/ 			
            boolIsFileEnd = TRUE;
        }
        else
        {
            sRetVal = -1;
            break;
        }

        if ( boolIsFileEnd == TRUE )
        {
            DebugOut( "\n[SendPkt-EOTto %d번 단말기]", nIndex + 1 );
        }
        else
        {
            DebugOut( "\n[SendPkt-to %d번 단말기]", nIndex + 1 );
        }
        DebugOut( "[송신 SEQ] %d 번 패킷", wSeqNo );

	   /*
	   	* 패킷 전송 
	   	*/ 			
        sRetVal = SendPkt();
        if ( sRetVal < 0 )
        {
            break;
        }

        if ( boolIsFileEnd == TRUE )
        {
            DebugOut( "\n[RecvPkt-EOT응답-from %d번 단말기]", nIndex+  1 );
        }
        else
        {
            DebugOut( "\n[RecvPkt-from %d번 단말기]", nIndex + 1 );
        }
	   /*
	   	* 응답패킷 수신
	   	*/ 	
        sRetVal = RecvPkt( 3000, nDevNo );
        if ( sRetVal < 0 )
        {
            printf( "\r\n 파일 전송중 응답없음[%d]\r\n", sRetVal );
            break;
        }
	   /*
	   	* 파일 전송완료 
	   	* - 파일이 전송완료가 되었음을 판단하는 조건으로는 파일끝에 도달하고
	   	*   ACK가 수신된 경우라야 한다. 
	   	*/ 
        if ( ( stRecvPkt.bCmd == ACK ) && ( boolIsFileEnd == TRUE) )
        {
            printf( "\r\n 파일 전송완료! 파일 전송완료! 파일 전송완료!\n" );
            sRetVal = SUCCESS;
            break;
        }
	   /*
	   	* 파일 전송중 NAK 응답으로 종료
	   	*/ 		
        else if ( stRecvPkt.bCmd == NAK )
        {
            sRetVal = ERR_MAINSUB_COMM_SUBTERM_IMG_DOWN_NAK;
            break;
        }
	   /*
	   	* 패킷 Sequence 번호 증가
	   	*/ 	
        wSeqNo++;
    }

    close( fdFile );
    return ErrRet( sRetVal );

}
















































