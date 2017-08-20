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
*  PROGRAM ID :       term_comm_subterm.c                                      *
*                                                                              *
*  DESCRIPTION:       하차기에서 승하차간 통신을 담당하는 프로그램으로 승차기  *
*                     에서 수신한 명령어를 파싱하여 하차기에서 해당 명령어에   *
*                     적합한 함수를 실행하여 처리하게 된다.                    *
*                                                                              *
*  ENTRY POINT:       SubTermProc()                                            *
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
#include "term_comm_subterm.h"
#include "../proc/main.h"
#include "../proc/file_mgt.h"
#include "../proc/write_trans_data.h"
#include "../proc/blpl_proc.h"
#include "../system/bus_type.h"
#include "../system/device_interface.h"
#include "../proc/card_proc_util.h"
#include "../proc/main_process_busTerm.h"
#include "../proc/main_environ_init.h"
#include "../proc/check_valid_file_mgt.h"

/*******************************************************************************
*  Definition of Macro                                                         *
*******************************************************************************/
/* 하차기 폴링시 전송 거래내역 크기 */
#define SUB_TERM_TR_RECORD_SIZE     ( 202 )

/*******************************************************************************
*  Declaration of Global Variables inside this module                          *
*******************************************************************************/

static OFFLINE_KEYSET_DATA      stKeySetData;       /* KeySet Data   */
static OFFLINE_IDCENTER_DATA    stIDCenterData;     /* IDCenter Data */

static int nSubTermPollingCheckCnt;    /* Check Count for MainTerm's Polling */

/*******************************************************************************
*  Declaration of Function Prototype                                           *
*******************************************************************************/
static short SubTermProcMain( void );
static short ProcRecvPkt( short sRetVal );
static short ParseDataNProcCmd( void );
static short CreateRespData( byte bCmd );
static short CreateRespImgVer( void );
static short CreateRespSubTermID( void );
static short CreateRespVoiceVer( void );
static short CreateRespKeySet( void );
static short CreateReqPLSearch( void );
static short CreateReqBLSearch( void );
static short ProcSubTermRecvData( void );
static short ProcSubTermTransData( void );
static short ProcSubTermPolling( void );
static short SubRecvImgVer( void );
static short SubRecvSubTermID( void );
static short SubRecvPoll( void );
static short SubRecvACK( void );
static short SubRecvNAK( void );
static short SubRecvBLCheckResult( void );
static short SubRecvPLCheckResult( void );
static short SendTDFile2MainTerm( int nDevNo );
static short ImgFileRecvFromMainTerm( void );
static short VoiceFileRecvFromMainTerm( void );
static short ParameterFileRecvFromMainTerm( void );
static short CheckReqBLPLSearch( bool *pboolIsBLPLCheckReq, byte *pbBLPLCmd );
static short SetStationToSubTermial( void );
static short ImgFileRecvFromMainTermOldProtocol( void );
static short RecvFileOldProtocol( int nDevNo, char* pchFileName );
/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       SubTermProc                                              *
*                                                                              *
*  DESCRIPTION:       단말기가 하차기인 경우 승차기와 통신을 담당하는 메인부분 *
*                     으로 하차기처리부분함수를 호출하고 에러를 처리한다.      *
*                                                                              *
*  INPUT PARAMETERS:  void                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: void                                                     *
*                                                                              *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:           하차기쪽 통신메인함수인 SubTermProcMain을 호출한다.      *
*                     하차기의 TimeOut이 1000회 이상 발생한 경우 로그를        *
*                     기록한다.                                                *
*                                                                              *
*******************************************************************************/
void SubTermProc( void )
{
    int nTimeOutCnt = 0;
    short sRetVal = SUCCESS;
    
//	PrintlnASC( "하차PSAMID :", gpstSharedInfo->abSubPSAMID[gbSubTermNo-1], 16 );

    while(1)
    {    	
		/*
		 * 하차기 통신처리 메인함수 호출 
		 */
	 	sRetVal = SubTermProcMain(); 
		
        if ( sRetVal < 0 ) 
        {
            if ( (sRetVal == ERR_UART_RECV_SELECT) ||
				 (sRetVal == ERR_UART_RECV_TIMEOUT) ||
                 (sRetVal == ERR_MAINSUB_COMM_PACKET) )
            {
                DebugOut( "SubTermProcMain 재시도" );
                nTimeOutCnt++;
            }
			/* 
			 * 타임아웃 1000회 후 ErrProc에서 로그기록 
			 */
            if ( nTimeOutCnt >= MAX_TIMEOUT_COUNT )
            {
                DebugOut( "하차 Timout 발생 1000회 이상시 로그에 기록\n" );
                ErrProc( sRetVal ); 
                nTimeOutCnt = 0;
            }
        }
    }

	/*
	 * 하차기 통신종료 
	 */	
    sRetVal = CloseMainSubComm();

    if ( sRetVal < 0 )
    {
        ErrProc(sRetVal);
    }	
}
/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       SubTermProcMain                             			   *
*                                                                              *
*  DESCRIPTION:       하차기의 통신부분의 실질적인 메인처리부분으로 승차기에서 *
*                     통신데이터를 수신/파싱하여 해당 처리함수를 실행시킨다.   *
*                                                                              *
*                                                                              *
*  INPUT PARAMETERS:  void                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: ERR_MAINSUB_COMM_SELECT                                  *
*                     ERR_UART_RECV_TIMEOUT                                    *
*                     ERR_MAINSUB_COMM_ETX_IN_PKT                              *
*                     ERR_MAINSUB_COMM_STX_IN_PKT                              *
*                     ERR_MAINSUB_COMM_CRC_IN_PKT                              *
*                                                                              *
*                                                                              *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
short SubTermProcMain( void )
{
    short sRetVal = SUCCESS;
    while(1)
    {
        DebugOut( "\n=============하차단말기 처리 시작 ==================" );
		/*
		 * 승차기로부터 초기 데이터 수신 ///////////////////////////////////////
		 */
        sRetVal = RecvPkt( 3000,  gbSubTermNo );
		
		/*
		* 초기데이터 수신데이터 검증  
		*/
		sRetVal = ProcRecvPkt( sRetVal );
		if ( sRetVal != SUCCESS )
		{			
			if ( sRetVal == ERR_MAINSUB_COMM_IGNORE_IN_PKT ) 
			{
				/* 
				 * STX ETX 에러등은 데이터를 재수신한다.
				 */			
				continue;
			}
			else 
			{
				/* 
				 * TimeOut이나 SELECT에러인 경우에만 상위 호출함수로 리턴
				 * 이유 : 상위함수(SubTermProc)에서 하차기 폴링이 일정횟수만큼
				 *        없는 경우 에러로그를 기록하기 위해서 
				 */
				LogTerm( "[SubTermProcMain]ProcRecvPkt Error 발생 %02x\n",
						sRetVal );
				return ErrRet( sRetVal );
			}
		}

        /* 
         * 하차기로 ACK 송신을 위한 패킷구성 ///////////////////////////////////
         */
        CreateRespData( ACK );
        /* 
         * 하차기로 ACK 송신-초기데이터 잘 수신했음을 알려줌////////////////////
         */
        sRetVal = SendPkt();
        if ( sRetVal < 0 )
        {
			LogTerm( "[SubTermProcMain]ack SendPkt Error 발생 %02x\n",
					sRetVal );
            return ErrRet( sRetVal );
        }

        /* 
         * 초기수신데이터 파싱 및 파싱커맨드에 따른 명령어실행//////////////////
         */
        sRetVal = ParseDataNProcCmd();  
        if ( sRetVal < 0 )
        {
			LogTerm( "[SubTermProcMain]ParseDataNProcCmd Error 발생 %02x\n",
					sRetVal );
        }
		
    }
	
}
/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       ProcRecvPkt                             			           *
*                                                                              *
*  DESCRIPTION:       초기데이터 수신시 처리를 담당한다.     		           *
*                                                                              *
*                                                                              *
*  INPUT PARAMETERS:  void                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: ERR_MAINSUB_COMM_SELECT                                  *
*                     ERR_UART_RECV_TIMEOUT                                    *
*                     ERR_MAINSUB_COMM_ETX_IN_PKT                              *
*                     ERR_MAINSUB_COMM_STX_IN_PKT                              *
*                     ERR_MAINSUB_COMM_CRC_IN_PKT                              *
*                                                                              *
*                                                                              *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
short ProcRecvPkt( short sRetVal )
{

 	switch( sRetVal )
    {
    	case SUCCESS :
	        /*
	        *  성공인경우 에러가 없는 경우 하차기 폴링수신상태인지 체크하는 변수 초기화
			*  하차기 폴링이 없는 경우가 8회 이상 되면 Display에 999999를 표시하고 
			*  nSubTermPollingCheckCnt값이 8로 되어있게 된다.
			*  그 이후 하차기에 폴링이 성공적으로 수신되게 되면 이 변수값을 0으로 
			*  초기화하고 Display에 0으로 표시해야한다.
	        */
	        if ( nSubTermPollingCheckCnt >= 8 )
	        {
	            DisplayASCInUpFND( "0" );
	        }
        	nSubTermPollingCheckCnt = 0;
			return SUCCESS;
			
		   /*
	        *  아래의 5가지의 패킷상의 에러는 ERR_MAINSUB_COMM_IGNORE_IN_PKT 에러코드를
	        *  리턴하게 되는데 상위함수에서는 이 에러가 발생한 경우에는 다시 
	        *  초기데이터 수신을 하도록 처리하게 된다.
	        */			
        case ERR_MAINSUB_COMM_NOT_MINE_PKT :
        case ERR_MAINSUB_COMM_INVALID_LENGTH :
        case ERR_MAINSUB_COMM_ETX_IN_PKT :
        case ERR_MAINSUB_COMM_CRC_IN_PKT :
        case ERR_MAINSUB_COMM_STX_IN_PKT :
            DebugOut( "Error!!! Packet Wrong Data \n" );
            return ERR_MAINSUB_COMM_IGNORE_IN_PKT;
			
		   /*
	        *  아래의 2가지의 에러는 초기데이터 수신시 SELECT함수내의 에러나
	        *  TimeOut이 나는 경우이며 이 경우에는 에러코드를 그대로 리턴한다.
	        *  또한 nSubTermPollingCheckCnt에 대해서도 처리를 해준다.
	        */				
        case ERR_UART_RECV_SELECT :
        case ERR_UART_RECV_TIMEOUT :
            DebugOut( "Error!!! Select or TimeOut\n" );
		   /*
	        *  0330 2005.2.28 하차음성 chip write 시
	        *  nSubTermPollingCheckCnt를 증가시키지 않고 0으로 만들어준다.
	        *  이는 하차음성을 chip에 write하는 경우에는 폴링수신을 
	        *  못하는 경우가 발생하더라도 Display에 999999로 표시하지 않弱渼募
	        *  의도이다. 즉 하차음성을 chip에 write할 경우에는 폴링미수신횟수를
	        *  증가시키지 않는다.
	        */
	        if ( gpstSharedInfo->bVoiceApplyStatus == 2 )
            {
                nSubTermPollingCheckCnt = 0;
                usleep( 500000 );
                return ErrRet( sRetVal );
            }

            usleep( 500000 );
            nSubTermPollingCheckCnt++;
			
	        /*  
	        * 폴링미수신횟수가 8회 이상이면 승차운행상태 아님을 하차 공유메모리에 
	        * 넣어주고 999999로 DISPLAY에 표시한다.
	        */
	        if ( nSubTermPollingCheckCnt >= 8 )
            {
                /* 승차운행상태 아님을 하차 공유메모리에 넣음 */
                gpstSharedInfo->boolIsDriveNow = FALSE;

                DebugOut( "승차 폴링 없음 \r\n" );
                DisplayASCInUpFND( FND_ERR_MAIN_SUB_COMM_POLLING );
                nSubTermPollingCheckCnt = 8;
            }
            return ErrRet( sRetVal );

        default :
            DebugOut( "default" );
            return ErrRet( ERR_MAINSUB_COMM_PACKET );
    }
	
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       ParseDataNProcCmd()                             			   *
*                                                                              *
*  DESCRIPTION:       하차기의 수신데이터를 파싱하여 승차기에서 보낸 명령어에  *
*                     맞는 처리함수를 호출한다.								   *
*                                                                              *
*                                                                              *
*  INPUT PARAMETERS:  void                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공 	                                   *
*                     ERR_MAINSUB_COMM_SUBTERM_CMD_D                           *
*                     ERR_MAINSUB_COMM_SUBTERM_CMD_A                           *
*                     ERR_MAINSUB_COMM_SUBTERM_CMD_M_ACK_FINAL                 *
*                                                                              *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
short ParseDataNProcCmd()
{
	short sRetVal = SUCCESS;
	
	switch( stRecvPkt.bCmd )
    {
			
        /* 
         * 프로그램 다운로드(구 프로토콜이용)
         */    
        case MAIN_SUB_COMM_SUB_TERM_IMG_DOWN_OLD :
            sRetVal = ImgFileRecvFromMainTermOldProtocol();
            if ( sRetVal < 0 )
            {
                sRetVal = ERR_MAINSUB_COMM_SUBTERM_CMD_D;
            }
            return ErrRet( sRetVal );			
        /* 
         * 프로그램 다운로드(신 프로토콜이용)
         */
        case MAIN_SUB_COMM_SUB_TERM_IMG_DOWN :
            sRetVal = ImgFileRecvFromMainTerm();
            if ( sRetVal < 0 )
            {
                sRetVal = ERR_MAINSUB_COMM_SUBTERM_CMD_D;
            }
            return ErrRet( sRetVal );

        /* 
         * 음성 다운로드
         */
        case MAIN_SUB_COMM_SUB_TERM_VOIC_DOWN :
            sRetVal = VoiceFileRecvFromMainTerm();
            if ( sRetVal < 0 )
            {
                sRetVal=  ERR_MAINSUB_COMM_SUBTERM_CMD_A;
            }
            return ErrRet( sRetVal );
        /* 
         * 하차기필요 파일 다운로드
         */
        case MAIN_SUB_COMM_SUB_TERM_PARM_DOWN :
            sRetVal = ParameterFileRecvFromMainTerm();
            if ( sRetVal < 0 )
            {
                CreateNAK( gbSubTermNo );
                DebugOut( "파라미터파일을 정상적으로 다운하지못해 NAK전송\n" );
            }
            else
            {
                CreateACK( gbSubTermNo );
                DebugOut( "파라미터 파일을 정상적으로 다운후 ACK전송\n" );

            }
            sRetVal = SendPkt();
            if ( sRetVal < 0 )
            {
                sRetVal=  ERR_MAINSUB_COMM_SUBTERM_CMD_M_ACK_FINAL;
            }
            ClearSharedCmdnData( CMD_NEW_CONF_IMG );

            return ErrRet( sRetVal );
        /* 
         * 폴링 
         */
        case MAIN_SUB_COMM_POLLING :
            ProcSubTermRecvData();
            ProcSubTermPolling();
            return ErrRet( sRetVal );
        /* 
         * 거래내역파일 송수신
         */
        case MAIN_SUB_COMM_GET_TRANS_FILE :
            SendTDFile2MainTerm( gbSubTermNo );
            return ErrRet( sRetVal );
        /* 
         * 키셋 등록/하차기음성버전송신/하차단말기ID송신/하차단말기프로그램 버전송신
         */
        case MAIN_SUB_COMM_REGIST_KEYSET :
        case MAIN_SUB_COMM_SUB_TERM_VOICE_VER :
        case MAIN_SUB_COMM_GET_SUB_TERM_ID :
        case MAIN_SUB_COMM_SUB_TERM_IMG_VER :
			DebugOut("ProcSubTermRecvData\n");
            ProcSubTermRecvData();
			DebugOut("CreateRespData\n");
            CreateRespData( stRecvPkt.bCmd );

            SendPkt();
            if ( stRecvPkt.nDataSize == 30 &&
                 stRecvPkt.bCmd == MAIN_SUB_COMM_SUB_TERM_IMG_VER )
            {
            	boolIsOldProtocol = FALSE;				
                DebugOut( "구 프로토콜이므로 이만 끝\n" );
                return ErrRet( sRetVal );
            }
			
            RecvPkt( 3000, gbSubTermNo );
            ProcSubTermRecvData();
            return ErrRet( sRetVal );
        /* 
         * 하차단말기 ID 수신
         */
        case MAIN_SUB_COMM_ASSGN_SUB_TERM_ID :
            ProcSubTermRecvData();
            return ErrRet( sRetVal );

        default :
            return ErrRet( sRetVal );
    }
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       CreateRespData()                             			   *
*                                                                              *
*  DESCRIPTION:       승차기의 요청데이터에 대한 응답(Response)데이터를        *
*                     생성한다.                                                *
*                                                                              *
*  INPUT PARAMETERS:  byte bCmd - 승차기에서 요청받은 실행 명령                *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공 	                                   *
*                     호출된 함수의 에러코드 참조.                             *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:           승차기에서 요청받은 실행명령에 따라 응답데이터를 구성하는*
*                     함수들을 호출한다                                        *
*                                                                              *
*******************************************************************************/
short CreateRespData( byte bCmd )
{
    short sRetVal = SUCCESS;

    switch( bCmd )
    {
        /* 
         * ACK를 하차기 응답데이터 구성
         */
        case ACK :
            sRetVal = CreateACK( gbSubTermNo );
            return ErrRet( sRetVal );
        /* 
         * 프로그램 버전체크요청에 대한 응답데이터 구성
         */
        case MAIN_SUB_COMM_SUB_TERM_IMG_VER :   
            printf( "[2.CreateRespImgVer]\n" );
            sRetVal = CreateRespImgVer();
            return ErrRet( sRetVal );
        /* 
         * 하차기 ID요청에 대한 응답데이터 구성
         */
        case MAIN_SUB_COMM_GET_SUB_TERM_ID :  //SubId 승차 전송 0330 2005.2.28
            DebugOut( "[2.CreateRespSubTermID]" );
            sRetVal = CreateRespSubTermID();
            return ErrRet( sRetVal );
        /* 
         * 하차기 음성버전요청에 대한 응답데이터 구성
         */
        case MAIN_SUB_COMM_SUB_TERM_VOICE_VER : //Voice Version 0330 2005.2.28
            DebugOut( "[2.CreateRespVoiceVer]" );
            sRetVal = CreateRespVoiceVer();
            return ErrRet( sRetVal );
        /* 
         * 하차기 키셋등록에 대한 응답데이터 구성
         */
        case MAIN_SUB_COMM_REGIST_KEYSET :      //Keyset Result
            DebugOut( "[2.CreateRespKeySet]" );
            sRetVal = CreateRespKeySet();
            return ErrRet( sRetVal );

        default :
            return ErrRet( sRetVal );
    }
}
/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       CreateRespImgVer()                           			   *
*                                                                              *
*  DESCRIPTION:       신/구 프로토콜로 프로그램 버전을 송신패킷 구조체에 넣는다*
*                                                                              *
*  INPUT PARAMETERS:  void											           *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공 	                                   *
*																			   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:           stSendPkt	- 송신패킷구조체                               *
*                                                                              *
*******************************************************************************/
short CreateRespImgVer( void )
{
    short sRetVal = SUCCESS;
    char achNewSendBuffer[40] = { 0, }; /* 신 프로토콜 버전생성시 SendBuffer */
    char achOldSendBuffer[30] = { 0, }; /* 구 프로토콜 버전생성시 SendBuffer */
    
   /*
	* 구 프로토콜로 프로그램버전을 만든다  
	*/
    if ( boolIsRespVerNeedByOldProtocol == TRUE )
    {
	    printf( "구 프로토콜로 프로그램 version 만든다\n" );
	    /*
	     *  program version copy
	     */
	    memcpy( achOldSendBuffer, gpstSharedInfo->abSubVer[gbSubTermNo-1], 4 );

	    /*
	     *  new를 버전에 덧붙여 보낸다.
	     *  이유 : 0401이후의 버전, 즉 0402,0403이 꼭 신프로토콜이라고 할 수 없다.
	     *         0401이 승하차통신에 문제가 생겨 이전 구 프로토콜로 복원하여
	     *         0402,0403을 만들어서 배포할 경우 버전만가지고는 프로트콜의 
	     *         신,구여부를 판단하기에 어려우므로 new를 덧붙여 버전이 04xx대라도
	     *         승차기에서 new를 수신하지 못하여 구프로토콜로 간주한다.
	     */    
	    memcpy( &achOldSendBuffer[4], "new", 3 );
  

		/*
		* 송신 패킷구조체에 값 설정 
		*/
	    stSendPkt.bCmd = 'V';
	    stSendPkt.bDevNo = gbSubTermNo + '0';
	    stSendPkt.nDataSize = 17;
	    memcpy( stSendPkt.abData, achOldSendBuffer, 17 ); /* 데이터Buffer Copy */

	    /*
	     *  구프로토콜 사용하도록 Flag 설정
	     */	
		boolIsOldProtocol = TRUE;

    }
    else
   /*
	* 신 프로토콜로 프로그램버전을 만든다  
	*/
    {
	    bCurrCmd = MAIN_SUB_COMM_SUB_TERM_IMG_VER;
	    /*
	     *  program version SendBuffer에 copy
	     */		
	    memcpy( achNewSendBuffer, &gpstSharedInfo->abSubVer[gbSubTermNo-1], 4 );
	    DebugOutlnASC( "gpstSharedInfo->abSubVer[gbSubTermNo-1]=>",
	                   gpstSharedInfo->abSubVer[gbSubTermNo-1], 4 );
	    /*
	     *  CSAM ID, PSAM ID SendBuffer에 copy
	     */
	    memcpy( &achNewSendBuffer[4], &gpstSharedInfo->abSubCSAMID[gbSubTermNo-1], 8 );
	    memcpy( &achNewSendBuffer[12],
	            &gpstSharedInfo->abSubPSAMID[gbSubTermNo-1], 16 );
		
	    DebugOutlnASC( "gpstSharedInfo->abSubPSAMID=>",
	                   gpstSharedInfo->abSubPSAMID[gbSubTermNo-1], 16 );
	    /*
	     *  PSAM Port No, ISAMID SendBuffer에 copy
	     */		
	    memcpy( &achNewSendBuffer[28],
	            &gpstSharedInfo->abSubPSAMPort[gbSubTermNo-1], 1 );
	    memcpy( &achNewSendBuffer[29], &gpstSharedInfo->abSubISAMID[gbSubTermNo-1], 7 );

	    /*
	     *  TPSAM 여부를 SendBuffer에 copy 
	     */	
	    memcpy( &achNewSendBuffer[36], &gpstSharedInfo->abSubPSAMVer[gbSubTermNo-1], 1 );

	    DebugOut( "\n TPSAM 여부 => 하차기 %d는 %d 이다 \n", gbSubTermNo, 
				   gpstSharedInfo->abSubPSAMVer[gbSubTermNo-1] );
		/*
		* 송신 패킷구조체에 값 설정 
		*/
	    stSendPkt.bCmd = MAIN_SUB_COMM_SUB_TERM_IMG_VER;   /* 명령어 'V' */
	    stSendPkt.bDevNo = gbSubTermNo + '0';			   /* 단말기 번호 */ 	
	    stSendPkt.wSeqNo = 0;							   /* Sequence No */
	    stSendPkt.nDataSize = 37;                          /* Data Size */
	    /*
	     *  SendBuffer를 송신패킷구조체내의 데이터에 copy 
	     */	
	    memcpy( stSendPkt.abData, achNewSendBuffer, 37 );
	    /*
	     *  구프로토콜 사용하지 않도록 Flag 설정
	     */	
		boolIsOldProtocol = FALSE;

   	}

	return ErrRet( sRetVal );
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       CreateRespSubTermID()                       			   *
*                                                                              *
*  DESCRIPTION:       하차기 ID를 송신패킷 구조체에 넣는다.                    *
*                                                                              *
*  INPUT PARAMETERS:  void											           *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공 	                                   *
*																			   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:          stSendPkt - 송신패킷구조체		                           *
*                                                                              *
*******************************************************************************/
short CreateRespSubTermID( void )
{
    short sRetVal = SUCCESS;
    char achSendBuff[30] = { 0, }; /* SendBuffer */

	/*
	* 하차기 ID를 SendBuffer에 copy 
	*/
    memcpy( achSendBuff, gpstSharedInfo->abSubTermID[gbSubTermNo-1], 9 );
	PrintlnASC( "[CreateRespSubTermID] 하차단말기ID : ",
		gpstSharedInfo->abSubTermID[gbSubTermNo-1], 9);
	/*
	* 송신 패킷구조체에 값 설정 
	*/
    stSendPkt.bCmd = MAIN_SUB_COMM_GET_SUB_TERM_ID; /* 명령어 'G' */
    stSendPkt.bDevNo = gbSubTermNo + '0';           /* 단말기 번호 */
    stSendPkt.wSeqNo = 0;							/* Sequence No */
    stSendPkt.nDataSize = 9;						/* Data Size */
    /*
     *  SendBuffer를 송신패킷구조체내의 데이터에 copy 
     */	
    memcpy( stSendPkt.abData, achSendBuff, 9 );

    return ErrRet( sRetVal );
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       CreateRespVoiceVer()                       			   *
*                                                                              *
*  DESCRIPTION:       음성 버전을 송신패킷 구조체에 넣는다.                    *
*                                                                              *
*  INPUT PARAMETERS:  void											           *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공 	                                   *
*																			   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:          stSendPkt - 송신패킷구조체		                           *
*                                                                              *
*******************************************************************************/
short CreateRespVoiceVer( void )
{
    short sRetVal = SUCCESS;
    char achSendBuff[30] = { 0, }; /* SendBuffer */

	/*
	* 하차기 음성버전을 SendBuffer에 copy 
	*/
    memcpy( achSendBuff, gpstSharedInfo->abSubVoiceVer[gbSubTermNo-1], 4 );
	/*
	* 송신 패킷구조체에 값 설정 
	*/
    stSendPkt.bCmd = MAIN_SUB_COMM_SUB_TERM_VOICE_VER; /* 명령어 'H' */
    stSendPkt.bDevNo = gbSubTermNo + '0';			   /* 단말기 번호 */
    stSendPkt.wSeqNo = 0;							   /* Sequence No */
    stSendPkt.nDataSize = 4;						   /* Data Size */
    /*
     *  SendBuffer를 송신패킷구조체내의 데이터에 copy 
     */		
    memcpy(stSendPkt.abData, achSendBuff, 4);

    return ErrRet( sRetVal );
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       CreateRespKeySet()                         			   *
*                                                                              *
*  DESCRIPTION:       하차기SAM에 PSAM의 키셋(Keyset)과 발행사(IdCenter)를 등록*
*                     한 결과를 송신패킷 구조체에 넣는다    				   *				       *
*                                                                              *
*  INPUT PARAMETERS:  void											           *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공 	                                   *
*																			   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:          stSendPkt - 송신패킷구조체		                           *
*                                                                              *
*******************************************************************************/
short CreateRespKeySet( void )
{
    short sRetVal = SUCCESS;
    byte achSendBuff[30] = { 0, }; /* SendBuffer */

 	/* Keyset 등록 결과를 SendBuffer에 Copy */
    memcpy( achSendBuff, &stSubTermPSAMAddResult.aboolIsKeySetAdded[gbSubTermNo-1], 1 );
	/* IdCenter 등록 결과를 SendBuffer에 Copy */
    memcpy( &achSendBuff[1],
            &stSubTermPSAMAddResult.aboolIsIDCenterAdded[gbSubTermNo-1], 1 );

    stSendPkt.bCmd = MAIN_SUB_COMM_REGIST_KEYSET; /* 명령어 'K' */
    stSendPkt.bDevNo = gbSubTermNo + '0';
    stSendPkt.wSeqNo = 0;
    stSendPkt.nDataSize = 2;
    memcpy( stSendPkt.abData, achSendBuff, 2 );

    return ErrRet( sRetVal );
}
/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       CreateReqBLSearch()                         			   *
*                                                                              *
*  DESCRIPTION:       하차기에 카드 태그시 BL여부를 요청하는 데이터를          *
*                     송신패킷 구조체에 넣는다 		        				   *				       *
*                                                                              *
*  INPUT PARAMETERS:  void											           *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공 	                                   *
*																			   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:          stSendPkt - 송신패킷구조체		                           *
*                                                                              *
*******************************************************************************/
short CreateReqBLSearch( void )
{
    byte bCmd;
    byte pcCmdData[40]= {0,};
    word wCmdDataSize;
    short sRetVal = SUCCESS;
//    byte abPrefix[6]={0,};
//    dword dwCardNum = 0;

    sRetVal = GetSharedCmd( &bCmd, pcCmdData, &wCmdDataSize );
    stSendPkt.bCmd = MAIN_SUB_COMM_REQ_BL_SEARCH;
    stSendPkt.bDevNo = gbSubTermNo + '0';
    stSendPkt.wSeqNo = 0;
    stSendPkt.nDataSize = 30;

    memcpy( stSendPkt.abData, &pcCmdData[1], 30 );
    

#ifdef TEST_BLPL_CHECK
    PrintlnASC( " Req BL Cardo :",  &pcCmdData[1], 20 );
    PrintlnASC( " Req BL CardNum :",  &pcCmdData[21], 6 );
    printf( " Req BL CardNum : [%lu]..\n",  &pcCmdData[27], 4 );
#endif

    return ErrRet( sRetVal );
}

short CreateReqPLSearch( void )
{
    byte bCmd;
    byte pcCmdData[40] = { 0, };
    word wCmdDataSize;
    short sRetVal = SUCCESS;
//    dword AliasNo = 0;

    sRetVal = GetSharedCmd( &bCmd, pcCmdData, &wCmdDataSize );
    stSendPkt.bCmd = MAIN_SUB_COMM_REQ_PL_SEARCH;
    stSendPkt.bDevNo = gbSubTermNo + '0';
    stSendPkt.wSeqNo = 0;
    stSendPkt.nDataSize = 24;

    memcpy( stSendPkt.abData, &pcCmdData[1] , 24 );
    
#ifdef TEST_BLPL_CHECK
    PrintlnASC( " Req PL Cardo :",  &pcCmdData[1], 20 );
    memcpy( ( byte*)&AliasNo,&pcCmdData[21], 4 );
    printf( "[6-1]Req PL Alias No : [%lu]\n", AliasNo );
#endif

    return ErrRet( sRetVal );

}

short ProcSubTermRecvData( void )
{
    short sRetVal = SUCCESS;

    switch( stRecvPkt.bCmd  )
    {
        case MAIN_SUB_COMM_SUB_TERM_IMG_VER :   // Program Version Check
            DebugOut( "\r\n[4.SubRecvImgVer]  V \r\n" );
            sRetVal = SubRecvImgVer();
            return ErrRet( sRetVal );

        case MAIN_SUB_COMM_GET_SUB_TERM_ID :    // Request SubTerminal ID
            DebugOut( "\r\n[ProcSubTermRecvData]  G \r\n" );
            return ErrRet( sRetVal );

        case MAIN_SUB_COMM_REGIST_KEYSET :      // KeySet Result
            DebugOut( "\r\n[ProcSubTermRecvData]  K \r\n" );
            return ErrRet( sRetVal );

        case MAIN_SUB_COMM_ASSGN_SUB_TERM_ID :  // Assign Sub Term ID
            DebugOut( "\r\n I \r\n" );
            sRetVal = SubRecvSubTermID();
            return ErrRet( sRetVal );

        case MAIN_SUB_COMM_REQ_BL_SEARCH :      // req BL search
            DebugOut( "[9.BL요청결과 처리]" );
            sRetVal = SubRecvBLCheckResult();
            return ErrRet( sRetVal );

        case MAIN_SUB_COMM_REQ_PL_SEARCH :      // req PL search
            DebugOut( "[9.PL요청결과 처리]" );
            sRetVal = SubRecvPLCheckResult();
            return ErrRet( sRetVal );

        case ACK :
            DebugOut( "[ACK처리]" );
            sRetVal = SubRecvACK();
            return ErrRet( sRetVal );

        case NAK :
            DebugOut( "[NAK처리]" );
            sRetVal = SubRecvNAK();
            return ErrRet( sRetVal );

        case MAIN_SUB_COMM_POLLING :            // Polling
            DebugOut( "\n[4.SubRecvPoll]" );
            sRetVal = SubRecvPoll();
            return ErrRet( sRetVal );

        default :
            sRetVal = ERR_MAINSUB_COMM_SUB_RECV_DATA_PARSE;
            return ErrRet( sRetVal );
    }
}

short ProcSubTermTransData( void )
{
    short sRetVal = SUCCESS;
    int fdTR;
    int nByte;
    byte abTmpBuff[SUB_TERM_TR_RECORD_SIZE + 2];

    struct stat stFileStatus;

    DebugOut( "[6.ProcSubTermTransData]거래내역파일생성시작" );

    // 운행중이 아닐 경우
    if ( gpstSharedInfo->boolIsDriveNow == FALSE )
    {

#ifdef TEST_SENDING_NOT_SEND_SUBTERM_TRN_ON_POLLING
        printf( "[미전송 파일 전송대기]\n" );
#endif
        DebugOut( "[7.단순폴링]단순폴링" );
        CreateACK( gbSubTermNo );
        bCurrCmd = ACK;
        sRetVal = SendPkt();

        if ( sRetVal < 0 )
        {
        	printf( "[ProcSubTermTransData] Ack send error %02x",
					sRetVal );
            return ErrRet( sRetVal );
        }

        sRetVal = RecvPkt( 3000, gbSubTermNo );
        if ( sRetVal < 0 )
        {
            printf( "[ProcSubTermTransData] recv error %02x",
					sRetVal );
            return ErrRet( sRetVal );
        }
        else
        {
            ProcSubTermRecvData();
        }

        return ErrRet( sRetVal );
    }

    SemaAlloc( SEMA_KEY_TRANS ); //0327

    fdTR = open( SUB_TERM_TRANS_FILE, O_RDWR, OPENMODE);

    if (( fstat( fdTR, &stFileStatus ) != 0 ) ||
        ( stFileStatus.st_size == 0 ) )
    {
#ifdef TEST_SENDING_NOT_SEND_SUBTERM_TRN_ON_POLLING
        printf( "fstat실패거나 거래내역 Size 0" );//cmd 'p'로 ack전송" );
#endif
        CreateACK( gbSubTermNo );
        bCurrCmd = ACK;
        sRetVal = SendPkt();

        if ( sRetVal < 0 )
        {
            close( fdTR );
            SemaFree( SEMA_KEY_TRANS );         //여기서 return시 계속 폴링 P
            return ErrRet( sRetVal );
        }

        sRetVal = RecvPkt( 3000, gbSubTermNo );
        if ( sRetVal < 0 )
        {
            close( fdTR );
            SemaFree( SEMA_KEY_TRANS );         ///여기서 return시 계속 폴링 P
            return ErrRet( sRetVal );
        }
        else
        {
            ProcSubTermRecvData();
            close( fdTR );
            SemaFree( SEMA_KEY_TRANS );
            return ErrRet( sRetVal );
        }

    }

#ifdef TEST_SENDING_NOT_SEND_SUBTERM_TRN_ON_POLLING
    printf( "fstat성공하고 거래내역 Size 1이상" );
#endif

    stSendPkt.bCmd = MAIN_SUB_COMM_GET_TRANS_CONT;
    stSendPkt.bDevNo = gbSubTermNo + '0';
    stSendPkt.wSeqNo = 0;
    bCurrCmd = MAIN_SUB_COMM_GET_TRANS_CONT;
    lseek( fdTR, 0L, SEEK_SET );

    while (1)
    {
		
        nByte = read( fdTR, abTmpBuff, SUB_TERM_TR_RECORD_SIZE + 1 );

        if ( nByte <= 0 )
        {
            close( fdTR );
            fdTR = open( SUB_TERM_TRANS_FILE,
                         O_RDWR | O_CREAT |O_TRUNC,
                         OPENMODE );
            close( fdTR );
            SemaFree( SEMA_KEY_TRANS );         //여기서 return시 계속 폴링 P

            return ErrRet( sRetVal );
        }

        if ( abTmpBuff[SUB_TERM_TR_RECORD_SIZE] == '1' )
        {
            // 기 전송 Record Skip
            continue;
        }

        memcpy( ( byte *)stSendPkt.abData, abTmpBuff, SUB_TERM_TR_RECORD_SIZE );
        stSendPkt.nDataSize = nByte - 1;

#ifdef TEST_SENDING_NOT_SEND_SUBTERM_TRN_ON_POLLING
        printf( "..202byte 전송시작/n" );
#endif
        sRetVal = SendPkt();

        //0323부터
        if ( sRetVal < 0 )
        {
            close( fdTR );
            SemaFree( SEMA_KEY_TRANS );         //여기서 return시 계속 폴링 P
            return ErrRet( sRetVal );
        }

        DebugOut( "[7.RecvPkt]" );
        sRetVal = RecvPkt( 5000, gbSubTermNo );

        if ( (sRetVal < 0 ) || ( stRecvPkt.bCmd == NAK ) )
        {
            tcflush( fdTR, TCIOFLUSH );
#ifdef TEST_SENDING_NOT_SEND_SUBTERM_TRN_ON_POLLING
            printf( "[ProcSubTermTransData] 202byte 거래내역파일 보냈으나 응답없어 파일 close -종료\n" );
#endif
            LogTerm( "[ProcSubTermTransData] 202byte 거래내역파일 보냈으나 응답없어 파일 close -종료\n" );            
	    close( fdTR );
            SemaFree( SEMA_KEY_TRANS );         //여기서 return시 계속 폴링 P
            return ErrRet( sRetVal );
        }
        else
        {
#ifdef TEST_SENDING_NOT_SEND_SUBTERM_TRN_ON_POLLING
            printf( "..[ProcSubTermTransData]승차기에서 ACK가 와서
                    거래내역파일 전송FLAG 갱신시작" );
#endif

            //승차단말기로 전송 완료 Flag
            abTmpBuff[SUB_TERM_TR_RECORD_SIZE] = '1';
            lseek( fdTR, -( SUB_TERM_TR_RECORD_SIZE + 1 ), SEEK_CUR );
            write( fdTR, abTmpBuff, SUB_TERM_TR_RECORD_SIZE + 1 );

            nByte = read( fdTR, abTmpBuff, SUB_TERM_TR_RECORD_SIZE + 1 );

            if ( nByte <= 0 )
            {

#ifdef TEST_SENDING_NOT_SEND_SUBTERM_TRN_ON_POLLING
                printf( "/t[Err!!]승차기에서 ACK가 와서 거래내역파일 전송FLAG " );
                printf( " 갱신완료 후 read로 포인트 이동시 에러발생하여
                            파일 close- 종료" );
#endif
                LogTerm( "[Err!!]승차기에서 ACK->거래파일 전송FLAG 갱신완료->read로 포인트 이동시 에러->파일 close- 종료" );  
                close( fdTR );
                fdTR = open( SUB_TERM_TRANS_FILE,
                             O_RDWR | O_CREAT |O_TRUNC, OPENMODE );
                close( fdTR );
                SemaFree( SEMA_KEY_TRANS );     //여기서 return시 계속 폴링 P
                return ErrRet( sRetVal );
            }

            close( fdTR );
            break;
        }

    }

    SemaFree( SEMA_KEY_TRANS );     //통신 에서 접근해재
    bCurrCmd = MAIN_SUB_COMM_POLLING;

    return ErrRet( sRetVal );
}



short ProcSubTermPolling( void )
{
    short sRetVal = SUCCESS;
    bool boolIsBLPLCheckReq = FALSE;
    byte bBLorPLCmd = 0;
    byte pcCmdData[40] = { 0, };

    CheckReqBLPLSearch( &boolIsBLPLCheckReq, &bBLorPLCmd );

#ifdef TEST_BLPL_CHECK
    printf( "\n[5.BLPL요청확인] %s\n",
            (( boolIsBLPLCheckReq == TRUE )? "있음." : "없음." ) );
#endif

    if ( boolIsBLPLCheckReq == TRUE )
    {
        if ( bBLorPLCmd == MAIN_SUB_COMM_REQ_BL_SEARCH )
        {
#ifdef TEST_BLPL_CHECK
            printf( "\n[6.CreateReqBLSearch]" );
#endif
            CreateReqBLSearch();
            bCurrCmd = MAIN_SUB_COMM_REQ_BL_SEARCH;
        }
        else if ( bBLorPLCmd == MAIN_SUB_COMM_REQ_PL_SEARCH )
        {
#ifdef TEST_BLPL_CHECK
            printf( "\n[6.CreateReqPLSearch]\n" );
#endif
            CreateReqPLSearch();
            bCurrCmd = MAIN_SUB_COMM_REQ_PL_SEARCH;
        }
        else
        {
            //에러처리
        }
		
#ifdef TEST_BLPL_CHECK
        printf( "\n[7.blpl SendPkt]\n" );
#endif

        sRetVal = SendPkt();
        if ( sRetVal < 0 )
        {
            memcpy( pcCmdData, "8", 1);
            SetSharedDataforCmd( MAIN_SUB_COMM_REQ_PL_SEARCH, pcCmdData, 1 );
			printf( "blpl check sendpkt error 발생 %02x\n",
					sRetVal );
            return ErrRet( sRetVal );
        }

#ifdef TEST_BLPL_CHECK
        printf( "[8.blpl RecvPkt]\n" );
#endif

        sRetVal = RecvPkt( 6000,  gbSubTermNo );
        if ( sRetVal >= 0 )
        {
            DebugOut( "[8.ProcSubTermRecvData]" );
            ProcSubTermRecvData();
        }
        else
        {
            memcpy( pcCmdData, "8", 1);
            SetSharedDataforCmd( MAIN_SUB_COMM_REQ_PL_SEARCH, pcCmdData, 1 );
            printf( "blpl check RecvPkt error 발생 %02x\n",
					sRetVal );
            DebugOut( " Error!! %02x", sRetVal );
        }
    }
    else
    {
#ifndef TEST_NOT_SEND_SUBTERM_TRN_ON_POLLING
        ProcSubTermTransData();
#endif
    }

    return ErrRet( sRetVal );
}


short SubRecvImgVer( void )
{
    short sRetVal = SUCCESS;
    byte abMainTermVer[5] = { 0, };
    byte abMainVer[4] = { 0, };

    memcpy( abMainVer, MAIN_RELEASE_VER, 4 ); // 승차기 버전 Copy

    if ( boolIsRespVerNeedByOldProtocol == TRUE )  //구  PROTOCOL로 V CMD 수신 시
    {
        // ISAMID쓰는 곳이 없다.
        //BCD2ASC(  stRecvPkt.abData, gpstSharedInfo->abMainISAMID, 4 );
        //승차 sniper sam id
        BCD2ASC(  &stRecvPkt.abData[5], gpstSharedInfo->abMainPSAMID, 8 );
        DebugOutlnASC( "gpstSharedInfo->abMainPSAMID=>",
                        gpstSharedInfo->abMainPSAMID, 16 );

        //단말기 아이디
        memcpy(  gpstSharedInfo->abMainTermID, &stRecvPkt.abData[13], 9 );
        DebugOutlnASC( "gpstSharedInfo->abMainTermID=>",
                        gpstSharedInfo->abMainTermID, 9 );

        memcpy( abMainTermVer, &stRecvPkt.abData[22], 4 );
#ifdef TEST_DOWN_AND_ROLLBACK
        PrintlnASC( "abMainTermVer=>", &stRecvPkt.abData[22], 4 );
#endif    
    }
    else    // 신 protocol로 수신 시
    {
        //승차 sniper sam id
        memcpy(  gpstSharedInfo->abMainPSAMID, stRecvPkt.abData, 16 );
        //단말기 아이디
        memcpy(  gpstSharedInfo->abMainTermID, &stRecvPkt.abData[16], 9 );
    }

    return ErrRet( sRetVal );
}

short SubRecvSubTermID( void )
{
    short sRetVal = SUCCESS;
    int fdSubID;
    byte abWriteTermID[10] = { 0, };
    byte abRecvTermID[10] = { 0, };
    byte abSubIDBuff[28] = { 0, };

    memcpy( gpstSharedInfo->abSubTermID[gbSubTermNo-1], stRecvPkt.abData, 9 );

    if ( ( fdSubID = open( SUBTERM_ID_FILENAME, O_WRONLY | O_CREAT | O_TRUNC , OPENMODE )
         ) < 0 )
    {
        DebugOut( "Error!!! open() failed\n" );
        return -1;
    }

    abSubIDBuff[0] = '1';
    memcpy( &abSubIDBuff[gbSubTermNo*9-8],
            gpstSharedInfo->abSubTermID[gbSubTermNo-1], 9 );

    write( fdSubID, abSubIDBuff, 28 );
    close( fdSubID );

    memcpy( abWriteTermID, gpstSharedInfo->abSubTermID[gbSubTermNo-1], 9 );

    //0323부터
    if ( CheckWriteEEPROM( abWriteTermID, abRecvTermID ) == 0 )
    {
        DisplayDWORDInUpFND( GetDWORDFromASC( &abRecvTermID[3], 6 ) );
		Buzzer( 2, 1000000 );			// 1초 간격으로 2회 부저 출력
        sleep(2);
        DisplayDWORDInUpFND( 0 );
    }

    return ErrRet( sRetVal );
}

short SubRecvPoll( void )
{
    short sRetVal = SUCCESS;

    sRetVal = SetStationToSubTermial();

    if ( sRetVal < 0 )
    {
    	printf( "[SubRecvPoll]SetStationToSubTermial Error 발생 %02x",
				sRetVal );
        sRetVal = ERR_MAINSUB_COMM_SUBSTATION_SET;
    }

    return ErrRet( sRetVal );
}


short SubRecvACK( void )
{
    short sRetVal = SUCCESS;

    switch( bCurrCmd )
    {
        case MAIN_SUB_COMM_GET_TRANS_CONT :   // Polling 응답으로 BLSearch 수신
            DebugOut( "[8.거래내역처리완료]" );
            return ErrRet( sRetVal );

        case ACK :   // Polling 응답으로 PLSearch 수신
            DebugOut( "[8.BLPL요청/거래내역없어 단순폴링 완료]" );
            return ErrRet( sRetVal );

        case MAIN_SUB_COMM_SUB_TERM_IMG_VER :
            DebugOut( "\r\n[처리완료]  Command V 정상처리\r\n" );
            return ErrRet( sRetVal );

        case MAIN_SUB_COMM_GET_SUB_TERM_ID :
            DebugOut( "\r\n[처리완료]  Command G 정상처리\r\n" );
            return ErrRet( sRetVal );

        case MAIN_SUB_COMM_SUB_TERM_VOICE_VER :
            DebugOut( "\r\n[처리완료]  Command H 정상처리\r\n" );
            return ErrRet( sRetVal );

        case MAIN_SUB_COMM_REGIST_KEYSET :
            DebugOut( "\r\n[처리완료]  Command K 정상처리\r\n" );
            return ErrRet( sRetVal );
    }

    return ErrRet( sRetVal );

}


short SubRecvNAK( void )
{
    short sRetVal = SUCCESS;

    switch( bCurrCmd )
    {
        case MAIN_SUB_COMM_REQ_BL_SEARCH :   // Polling 응답으로 BLSearch 수신
            DebugOut( "[8.BL요청결과 NAK]" );
            sRetVal = SubRecvBLCheckResult();
            return ErrRet( sRetVal );

        case MAIN_SUB_COMM_REQ_PL_SEARCH :   // Polling 응답으로 PLSearch 수신
            DebugOut( "[9.PL요청결과 NAK]" );
            sRetVal = SubRecvPLCheckResult();
            return ErrRet( sRetVal );

        case MAIN_SUB_COMM_GET_TRANS_CONT :   // Polling 응답으로 TransFile 수신
            DebugOut( "\r\n 거래내역 파일을 승차기가 수신은 했지만 파싱에러\r\n" );
            return ErrRet( sRetVal );

        case ACK :   // Polling 응답으로 TransFile 수신
            DebugOut( "\r\n거래내역 파일 없어서 ACK보냈는데 NAK가 옴
                      ReckACK에서 에러 \r\n" );
            return ErrRet( sRetVal );
    }

    return ErrRet( sRetVal );
}

short SubRecvBLCheckResult( void )
{
    short sRetVal = SUCCESS;
    byte pcCmdData[40] = { 0, };
    TRANS_INFO stCardErrInfo;
	short PLCheckStatus =0;

   /*
	* BL의 결과는 2byte로 수신-승차기에서 에러가 날경우 에러코드를 수신하기 위해
	*/
    if ( stRecvPkt.bCmd == MAIN_SUB_COMM_REQ_BL_SEARCH )
    {
        if ( stRecvPkt.nDataSize > 3 )
        {
            memset( (byte *)&stCardErrInfo, 0, sizeof( TRANS_INFO ) );
	     	memcpy( &stCardErrInfo, &stRecvPkt.abData[3], sizeof( TRANS_INFO ) ); 
            AddCardErrLog( SUCCESS, &stCardErrInfo );
        }

		/*
		* 체크 성공여부 복사
		*/
		memcpy( &PLCheckStatus, stRecvPkt.abData, 2);
		
		if ( PLCheckStatus != SUCCESS )
		{
			// 완료 + 결과  pcCmdData = |8|성공여부(2byte)|result(1byte)|
			memcpy( pcCmdData, "8", 1);	        
	        memcpy( &pcCmdData[1],&PLCheckStatus, 2 );
			memcpy( &pcCmdData[3],&stRecvPkt.abData[2], 1 );
	        SetSharedDataforCmd( MAIN_SUB_COMM_REQ_BL_SEARCH, pcCmdData, 4 );			
		}
		else
		{
    		// 완료 + 결과  pcCmdData = |4|성공여부(2byte)|result(1byte)|
	        memcpy( pcCmdData, "4", 1 );
	        memcpy( &pcCmdData[1],&PLCheckStatus, 2 );
			memcpy( &pcCmdData[3],&stRecvPkt.abData[2], 1 );
	        SetSharedDataforCmd( MAIN_SUB_COMM_REQ_BL_SEARCH, pcCmdData, 4 );
		}
    }
    else if ( stRecvPkt.bCmd == NAK )
    {
		memcpy( pcCmdData, "8", 1);
        pcCmdData[1] = NAK;
		pcCmdData[2] = NAK;			
        SetSharedDataforCmd( MAIN_SUB_COMM_REQ_BL_SEARCH, pcCmdData, 4 );
    }

    return ErrRet( sRetVal );
}


short SubRecvPLCheckResult( void )
{
    byte pcCmdData[40] = { 0, };
    TRANS_INFO stCardErrInfo;
	short CheckStatus = 0;
	
    if ( stRecvPkt.bCmd == MAIN_SUB_COMM_REQ_PL_SEARCH )
    {
        if ( stRecvPkt.nDataSize > 3 )
        {
            memset( (byte *)&stCardErrInfo, 0, sizeof( TRANS_INFO ) );
	     	memcpy( &stCardErrInfo, &stRecvPkt.abData[3], sizeof( TRANS_INFO ) ); 
            AddCardErrLog( SUCCESS, &stCardErrInfo );
        }
		/*
		* 체크 성공여부 복사
		*/
		memcpy( &CheckStatus, stRecvPkt.abData, 2);
		
		if ( CheckStatus != SUCCESS )
		{
			// 완료 + 결과  pcCmdData = |8|성공여부(2byte)|result(1byte)|
			memcpy( pcCmdData, "8", 1);	        
	        memcpy( &pcCmdData[1],&CheckStatus, 2 );
			memcpy( &pcCmdData[3],&stRecvPkt.abData[2], 1 );
	        SetSharedDataforCmd( MAIN_SUB_COMM_REQ_PL_SEARCH, pcCmdData, 4 );
			
		}
		else
		{
    		// 완료 + 결과  pcCmdData = |4|성공여부(2byte)|result(1byte)|
	        memcpy( pcCmdData, "4", 1 );
	        memcpy( &pcCmdData[1],&CheckStatus, 2 );
			memcpy( &pcCmdData[3],&stRecvPkt.abData[2], 1 );
	        SetSharedDataforCmd( MAIN_SUB_COMM_REQ_PL_SEARCH, pcCmdData, 4 );
		}
    }
    else if ( stRecvPkt.bCmd == NAK )
    {
        memcpy( pcCmdData, "8", 1);
        pcCmdData[1] = NAK;
		pcCmdData[2] = NAK;			
        SetSharedDataforCmd( MAIN_SUB_COMM_REQ_PL_SEARCH, pcCmdData, 4 );
    }
    return 0;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       SendTDFile2MainTerm                                      *
*                                                                              *
*  DESCRIPTION:       Send subterminal's TD File to Mainterminal when bus      *
*                     running status is not drive                              *
*                                                                              *
*  INPUT PARAMETERS:  int nDevNo - subterminal device number like 1,2,3         *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공                                      *
*                                                                              *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
short SendTDFile2MainTerm( int nDevNo )
{
    int sRetVal = SUCCESS;
    byte abTmpFileName[30] = { 0, };
    int fdCheck = 0;

    memcpy(abTmpFileName, stRecvPkt.abData, stRecvPkt.nDataSize);

#ifdef TEST_DAESA
    printf( "\r\n 승차기가 전송요구한 거래내역파일명 : [%s]\r\n", abTmpFileName );
#endif

	printf( "[SendTDFile2MainTerm] 하차대사용거래내역파일 승차로 전송 : [%s] ",
		abTmpFileName );
 
    fdCheck = open( SUB_TRANS_SEND_SUCC, O_RDWR | O_CREAT |O_TRUNC, OPENMODE );
    close( fdCheck );

    bCurrCmd = MAIN_SUB_COMM_GET_TRANS_FILE;
    sRetVal = SendFile( nDevNo, abTmpFileName );

    if ( sRetVal < 0 )
    {
#ifdef TEST_DAESA
        printf( "\r\n [SendTDFile]거래내역 화일 전송 실패 : [%d]\r\n", sRetVal );
#endif
        if ( sRetVal == ERR_MAINSUB_COMM_FILE_NOT_FOUND )
        {
            DebugOut( "\r\n [SendTDFile] 거래내역 화일 없음" );
        }
    }
    else
    {
#ifdef TEST_DAESA
        printf( "\r\n[SendTDFile] 거래내역 화일 전송 성공 \r\n" );
#endif
        unlink( abTmpFileName );
    }

    system( "rm *.trn" );
    unlink( SUB_TRANS_SEND_SUCC );

#ifdef TEST_DAESA
    printf( "하차기의 모든 trn파일삭제\n" );
#endif

    return ErrRet( sRetVal );

}
/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       ImgFileRecvFromMainTermOldProtocol                       *		       
*                                                                              *
*  DESCRIPTION:       03xx버전이 사용하는 구프로토콜로 하차프로그램 파일을     *
*                     수신한다.                                                *
*                   														   *
*																			   *
*  INPUT PARAMETERS:  void					                                   *
*                    												           *
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
*                    위한 변수를 원래 stRecvPkt구조체 내의 abData변수를 사용   *
*                    하지 않고 abRecvData로 별도 선언하여 사용하고 있다.       *
*                                                                              *
*                                                                              *
*******************************************************************************/
short ImgFileRecvFromMainTermOldProtocol( void )
{
	int retVal;
	RS485_FILE_HEADER_PKT_OLD stfileHeaderPkt;
	
	char tmp_filename[13]= { 0, };
	char org_filename[13]= { 0, };

	memset( &stfileHeaderPkt, 0x00, sizeof( RS485_FILE_HEADER_PKT_OLD ));
	
	memcpy(stfileHeaderPkt.achFileName, abRecvData + 1, ( stRecvPkt.nDataSize) - 3);
	
	printf("\r\n rs485SubDResp rs485PktSend  before :\r\n");
		
	memcpy(tmp_filename,stfileHeaderPkt.achFileName,12);	
	memcpy(org_filename,stfileHeaderPkt.achFileName,12);	
	memcpy(&tmp_filename[9],"tmp",3);
	
	retVal = RecvFileOldProtocol(gbSubTermNo, tmp_filename);
	if (retVal < 0)
	{
		printf("\r\n rs485SubDResp rs485Download Fail : [%d]\r\n",retVal);	
		unlink(tmp_filename);		
	} 
	else 
	{
		rename(tmp_filename,org_filename);
//0330 2005.2.28  실행프로그램 정상여부 Check 
		unlink("bus200");
		system("cp bus100 bus200");
		system("sync");
        unlink( STATUS_FLAG_FILE );

        ClosePeripheralDevices();
		system("reset");
		exit(1);
	}
	return retVal;
}



/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       RecvFileOldProtocol			                           *		       
*                                                                              *
*  DESCRIPTION:       03xx버전이 사용하는 구프로토콜로 하차프로그램 파일을     *
*                     수신한다.                                                *
*                   														   *
*																			   *
*  INPUT PARAMETERS:  int nDevNo - 단말기 번호                                 *
*                     char* pchFileName - 수신할 하차기프로그램 파일           *
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
*                    위한 변수를 원래 stRecvPkt구조체 내의 abData변수를 사용   *
*                    하지 않고 abRecvData로 별도 선언하여 사용하고 있다.       *
*                                                                              *
*                                                                              *
*******************************************************************************/
short RecvFileOldProtocol( int nDevNo, char* pchFileName )
{
    int fdFile;
    short sRetVal = SUCCESS;

   /*
    * 수신된 파일을 저장할 파일 생성/열기
    */
    fdFile = open( pchFileName, O_WRONLY | O_CREAT | O_TRUNC, OPENMODE );

    if ( fdFile < 0 )
    {
    	printf( " %s file open fail\n", pchFileName);
        close( fdFile );
        return -1;
    }
   /*
    * 패킷수신 및 저장 Loop 
    */
    while( 1 )
    {
	   /*
	    * 패킷수신
	    */
        sRetVal = RecvPkt( 8000, nDevNo );

        if ( sRetVal < 0 )
        {
            printf( "/r/n file recv Error: [1] /r/n" );
            break;
        }

	   /* 
	    * 패킷수신시 ACK응답을 위한 데이터 생성 
	    */
        stSendPkt.bCmd = 'D';
        stSendPkt.bDevNo = nDevNo + '0';
        stSendPkt.nDataSize = 1;
		stSendPkt.abData[0] = ACK;
	   /* 
	    * ACK전송 
	    */
        sRetVal = SendPkt();
        if ( sRetVal < 0 )
        {
            DebugOut( "/r/n SendPkt Error  : [2] /r/n" );
            break;
        }

	   /* 
	    *  EOT수신여부 체크 
	    */
        if ( ( stRecvPkt.nDataSize == 1 ) && ( abRecvData[0] == EOT ) )
        {
			printf("구프로토콜로 펌웨어 다운로드 완료(승차기는 구프로토콜 사용버전 ");
			break;
        }
	    /* 
	     *  패킷수신데이터 파일에 저장 
	     */
        write( fdFile, abRecvData, stRecvPkt.nDataSize );
		/* 
	     *  다음 패킷수신시 시퀀스 체크를 위해 현재 시퀀스를 저장
	     */

    }

    close( fdFile );

    return sRetVal;
}



short ImgFileRecvFromMainTerm( void )
{
    int sRetVal =0;
    char achRecvData[30] = { 0, };
    char achTmpFileName[13] = { 0, };
    char achOrgFileName[13] = { 0, };

    memcpy( achRecvData, stRecvPkt.abData, stRecvPkt.nDataSize );
    achRecvData[stRecvPkt.nDataSize] = 0x00;

    DebugOut( "\r\n 하차기로 받을 하차기 실행프로그램명 :[%s]\n", achRecvData );

    memcpy( achTmpFileName, achRecvData, strlen( achRecvData ) );
    memcpy( achOrgFileName, achRecvData, strlen( achRecvData ) );
    memcpy( &achTmpFileName[9], "tmp", 3 );

	printf( "[ImgFileRecvFromMainTerm] 승차수신 : [%s] ", achTmpFileName );

    sRetVal = RecvFile( gbSubTermNo, achTmpFileName );

    if ( sRetVal < 0 )
    {
        printf( "\r\n Image Download Fail : [%d]\r\n", sRetVal );
        unlink( achTmpFileName );

        CreateNAK( gbSubTermNo );
        sRetVal = SendPkt();

        if ( sRetVal < 0 )
        {
            return ErrRet( sRetVal );
        }
        else
        {
            DebugOut( "하차기 실행프로그램을 정상적으로 다운하지못해 NAK전송\n" );
        }

    }
    else
    {
        CreateACK( gbSubTermNo );
        sRetVal = SendPkt();

        if ( sRetVal < 0 )
        {
            return ErrRet( sRetVal );
        }
        else
        {
            DebugOut( "하차기 실행프로그램을 정상적으로 다운후 ACK전송 후 RESET\n" );
        }

        rename( achTmpFileName, achOrgFileName );

        //0330 2005.2.28  실행프로그램 정상여부 Check
        unlink( "bus200" );
        system( "cp bus100 bus200" );
        system( "sync" );
        unlink( STATUS_FLAG_FILE );

        ClosePeripheralDevices();
		printf("실행파일 다운로드 완료 - 리부팅합니다.\n");
        system( "reset" );
        exit(1);
    }

    return ErrRet( sRetVal );
}

short VoiceFileRecvFromMainTerm( void )
{
    short sRetVal = SUCCESS;
    int   fdVoiceApply = 0;
    int   fdVoiceVer = 0;
    int   fdVoiceFile = 0;
    char achRecvData[30] = { 0, };
    char achTmpFileName[13] = { 0, };
    char achOrgFileName[13] = { 0, };
    char achTmpVer[5] = { 0, };

    memcpy( achRecvData, stRecvPkt.abData, stRecvPkt.nDataSize );
    achRecvData[stRecvPkt.nDataSize] = 0x00;

    DebugOut( "\r\n fileName :[%s]", achRecvData );

    memcpy( achTmpFileName, achRecvData, strlen( achRecvData ) );
    memcpy( achOrgFileName, achRecvData, strlen( achRecvData ) );
    memcpy( &achTmpFileName[5], "tmp", 3 );

	printf( "[VoiceFileRecvFromMainTerm] 승차수신 : [%s] ", achTmpFileName );

    sRetVal = RecvFile( gbSubTermNo, achTmpFileName );
    if ( sRetVal < 0 )
    {
        printf( "\r\n rs485SubAResp rs485Download Fail : [%d]\r\n", sRetVal );
        unlink( achTmpFileName );

        CreateNAK( gbSubTermNo );
        sRetVal = SendPkt();
        if ( sRetVal < 0 )
        {
            return ErrRet( sRetVal );
        }
        else
        {
            DebugOut( "음성파일을 정상적으로 다운하지못해 NAK전송\n" );
        }
    }
    else
    {
        CreateACK( gbSubTermNo );
        sRetVal = SendPkt();
        if ( sRetVal < 0 )
        {
            return ErrRet( sRetVal );
        }
        else
        {
            printf( "음성파일을 정상적으로 다운후 ACK전송 후 RESET\n" );
        }

        unlink( achOrgFileName );
        rename( achTmpFileName, achOrgFileName );

        chmod( achOrgFileName, S_IRUSR|S_IWUSR );
		printf( "111\n" );
        gpstSharedInfo->bVoiceApplyStatus = 1; // 적용대기

        /* voiceapply.dat 파일을 생성 */
        if (( fdVoiceApply = open( VOICEAPPLY_FLAGFILE,
                                   O_WRONLY |O_CREAT|O_TRUNC,
                                   OPENMODE )
                                 ) < 0 )
        {
            printf( "Error!!! voiceapply.dat open() failed\n" );
            return -1;
        }

        if (( fdVoiceVer = open( VOICEAPPLY_VERSION,
                                 O_WRONLY |O_CREAT|O_TRUNC,
                                 OPENMODE )
                                ) < 0)
        {
            printf( "Error!!!  voicever.dat open() failed\n" );
            close( fdVoiceApply );
            unlink( VOICEAPPLY_VERSION );
            return -1;
        }

        if (( fdVoiceFile = open( VOICE0_FILE, O_RDONLY, OPENMODE )) < 0 )
        {
            printf( "Error!!! c_v0.dat open() failed\n" );
            close( fdVoiceApply );
            close( fdVoiceVer );
            unlink( VOICEAPPLY_VERSION );
            return -1;
        }

        /* c_v0.dat.dat의 끝 4byte(음성버젼)을 읽어 voicever.dat에 기록 */
		
        sRetVal = lseek( fdVoiceFile, -4, SEEK_END );
		if ( sRetVal == -1 )
			perror("Voice File read lseek error : ");            		
        sRetVal = read( fdVoiceFile, achTmpVer, 4 );
		if ( sRetVal == -1 )
			perror("Voice File read error : ");    
        sRetVal = write( fdVoiceVer, achTmpVer, 4 );
		if ( sRetVal == -1 )
			perror("Voice Ver Write read error : "); 		

        close( fdVoiceApply );
        close( fdVoiceVer );
        close( fdVoiceFile );

        /* 음성파일 다운로드 완료에 따른 하차기 재부팅           */
        /* 재부팅시 하차기에서는 voiceapply.dat이 존재할 경우에만
           chip에 음성파일을 Write한다                           */
        printf("음성파일 다운로드 완료 - 재부팅합니다\n");
        system( "reset" );
    }

    return sRetVal;

}


short ParameterFileRecvFromMainTerm( void )
{
    short sRetVal = SUCCESS;
    short sResult = SUCCESS;
    short sKeySetRetVal = SUCCESS;
    short sIDCenterRetVal = SUCCESS;
    char achTmpRecvFileName[30];
    char achTmpFileName[30];
    char achOrgFileName[30];

    byte bCmd = 0;
    char achCmdData[5] = { 0, };
    word wCmdDataSize = 0;

    byte abDebugCmdData[40];
    byte bDebugSharedMemoryCmd;
    word wDebugDataSize;

    while( stRecvPkt.bCmd != ETB )
    {
        memset( achTmpRecvFileName, 0, sizeof( achTmpRecvFileName ) );
        memset( achTmpFileName, 0, sizeof( achTmpFileName ) );
        memset( achOrgFileName, 0, sizeof( achOrgFileName ) );

        GetSharedCmd( &bCmd, achCmdData, &wCmdDataSize );
        if ( ( bCmd  == CMD_NEW_CONF_IMG ) &&
             ( achCmdData[0] == '0'  || achCmdData[0] == '9') )
        {
            ClearSharedCmdnData( CMD_NEW_CONF_IMG );
        }

        sRetVal = RecvPkt( 3000, gbSubTermNo ); // receive ack 도 처리
        if ( sRetVal < 0 )
        {
            return ErrRet( sRetVal );
        }

        memcpy( achTmpRecvFileName, stRecvPkt.abData, stRecvPkt.nDataSize );
        achTmpRecvFileName[stRecvPkt.nDataSize] = 0x00;

        DebugOut( "stRecvPkt.abData[0] %x\n", stRecvPkt.abData[0] );
        DebugOut( "stRecvPkt.nDataSize %d\n", stRecvPkt.nDataSize );
        DebugOutlnASC( " stRecvPkt.abData\n",
                       stRecvPkt.abData, stRecvPkt.nDataSize );
        DebugOut( "\r\n 하차기로 받을 파일명 :[%s]", achTmpRecvFileName );

        if ( stRecvPkt.bCmd == ETB )
        {
            // 파라미터파일 다운 성공에 따라 공유메모리를 이용해
            // 하차메인에 파라미터파일 RELOAD요청
            sResult = SetSharedCmdnData( CMD_NEW_CONF_IMG, "1100", 4 );

            if ( sResult < 0 )
            {
                printf( "Error!!하차메인에 파라미터파일 RELOAD요청실패//////\n" );
                memset( abDebugCmdData, 0x00, sizeof( abDebugCmdData ) );
                GetSharedCmd( &bDebugSharedMemoryCmd, abDebugCmdData,
                              &wDebugDataSize );
                printf( "머가 들어있냐?====SharedMemoryCmd = %c====\n",
                         bDebugSharedMemoryCmd );
            }
            else
            {
                printf( "[ParameterFileRecvFromMainTerm] 메인프로세스에 운영정보 로드 요청 성공\n" );
            }        
            break;
        }

        CreateACK( gbSubTermNo );
        sRetVal = SendPkt();

        if ( sRetVal < 0 )
        {
            return ErrRet( sRetVal );
        }

        memcpy( achTmpFileName, achTmpRecvFileName,
                strlen( achTmpRecvFileName ) );
        memcpy( achOrgFileName, achTmpRecvFileName,
                strlen( achTmpRecvFileName ) );

        DebugOut( "\r\n achOrgFileName : [%s]", achOrgFileName );

        memcpy( &achTmpFileName[strlen( achTmpFileName )-3], "tmp", 3 );

		printf( "[ParameterFileRecvFromMainTerm] 승차수신 : [%s] ",
			achTmpFileName );

        sRetVal = RecvFile( gbSubTermNo, achTmpFileName );
        if ( sRetVal < 0 )
        {
            printf( "[ParameterFileRecvFromMainTerm] 승차수신 실패 : [%s]\n",
				achTmpFileName );
            unlink( achTmpFileName );
        }
        else
        {
			sResult = CheckValidFile( GetDCSCommFileNoByFileName( &achOrgFileName[2] ), achTmpFileName );
			if ( sResult == SUCCESS )
			{
				printf( " -> [%u][%s]\n", GetDCSCommFileNoByFileName( &achOrgFileName[2] ), achOrgFileName );
	            rename( achTmpFileName, achOrgFileName );
			}
			else
			{
				printf( " -> 유효성체크 실패하여 삭제 [%u][%s]\n", GetDCSCommFileNoByFileName( &achOrgFileName[2] ), achOrgFileName );
				unlink( achTmpFileName );
			}

            if ( memcmp( achOrgFileName, PSAM_KEYSET_INFO_FILE, 12 ) == 0 )
            {
                // 초기화
                stSubTermPSAMAddResult.aboolIsKeySetAdded[gbSubTermNo-1] = FALSE;
                stSubTermPSAMAddResult.aboolIsIDCenterAdded[gbSubTermNo-1] = FALSE;

                sIDCenterRetVal = RegistOfflineID2PSAMbyEpurseIssuer();
                sKeySetRetVal = RegistOfflineKeyset2PSAMbyEpurseIssuer();

                if ( sIDCenterRetVal == SUCCESS )
                {
                    printf( "[ParameterFileRecvFromMainTerm] IDCENTER 등록 성공\n" );
                    stSubTermPSAMAddResult.aboolIsIDCenterAdded[gbSubTermNo-1] = TRUE;
                }
                else
                {
                    printf( "[ParameterFileRecvFromMainTerm] IDCENTER 등록 실패\n" );
                    stSubTermPSAMAddResult.aboolIsIDCenterAdded[gbSubTermNo-1] = FALSE;
                }

                if ( sKeySetRetVal == SUCCESS )
                {
                    printf( "[ParameterFileRecvFromMainTerm] KEYSET 등록 성공\n" );
                    stSubTermPSAMAddResult.aboolIsKeySetAdded[gbSubTermNo-1] = TRUE;
                }
                else
                {
                    printf( "[ParameterFileRecvFromMainTerm] KEYSET 등록 실패\n" );
                    stSubTermPSAMAddResult.aboolIsKeySetAdded[gbSubTermNo-1] = FALSE;
                }
            }
        }
    }

    return ErrRet( sRetVal );
}

short CheckReqBLPLSearch( bool *pboolIsBLPLCheckReq, byte *pbBLPLCmd )
{
    byte bCmd;
    byte pcCmdData[40];
    word wCmdDataSize;
    short sRetVal = SUCCESS;

    GetSharedCmd( &bCmd, pcCmdData, &wCmdDataSize );

#ifdef TEST_BLPL_CHECK
    printf( "[term_comm]recv blplreq from sharedmemory --> bCmd :
            %c, size : %d \n", bCmd, wCmdDataSize);
#endif

    if ( ( bCmd == MAIN_SUB_COMM_REQ_BL_SEARCH ) ||
         ( bCmd == MAIN_SUB_COMM_REQ_PL_SEARCH ) )
    {

        if ( pcCmdData[0] == '1' )
        {
            *pboolIsBLPLCheckReq = TRUE;
            *pbBLPLCmd = bCmd;
        }
        else
        {
            *pboolIsBLPLCheckReq = FALSE;
        }
    }
    else
    {
        *pboolIsBLPLCheckReq = FALSE;
    }

    return ErrRet( sRetVal );
}

// RTC Time 세팅 
// 정류장, 운행중여부, 승차단말기 id, 거래내역파일 세팅
short SetStationToSubTermial( void )
{
    short sRetVal = SUCCESS;
    byte abCurrTime[15] = { 0, };
    time_t tMainTermTime;

    memcpy( abCurrTime, stRecvPkt.abData , 14 );

    tMainTermTime = GetTimeTFromASCDtime( abCurrTime );
    SetRTCTime( tMainTermTime );

    memcpy( gpstSharedInfo->abNowStationID, stRecvPkt.abData + 14, 7 );
    memcpy( &( gpstSharedInfo->boolIsDriveNow) , stRecvPkt.abData + 21, 1 );
    memcpy( gpstSharedInfo->abMainTermID, stRecvPkt.abData + 22, 9 );
    memcpy( gpstSharedInfo->abTransFileName, stRecvPkt.abData + 31, 18 );
	gpstSharedInfo->gbGPSStatusFlag = stRecvPkt.abData[49];

    return ErrRet( sRetVal );
}



//******************************************************************************
//  OFFLINE ID CENTER 등록
//  Return : 0 = OK, 1 = NG
//******************************************************************************
short RegistOfflineID2PSAMbyEpurseIssuer( void )
{
    FILE *fdIDCenter;
    int nRecCnt;
    int nReadSize;
    int i;
    short sRetVal = -1;
    short sRet;
    byte abTmpBuff[90] = { 0, };
    byte abTmpSAMID[17] = { 0, };

    DebugOut( "\r\n<OFFLINE_IDCENTER> ID CENTER 등록\r\n " );

    fdIDCenter = fopen( EPURSE_ISSUER_REGIST_INFO_FILE, "rb" );
    if (fdIDCenter == NULL)
    {
        DebugOut( "\r\n<OFFLINE_IDCENTER> 파일 오픈 에러\n " );
        return ErrRet( ERR_MAINSUB_COMM_IDCENTER_FILE_OPEN );  // File not found
    }

    // 적용년월일 7자리
    if ( ( nReadSize = fread( abTmpBuff, 7, 1, fdIDCenter ) ) != 1 )
    {
        // Format 안맞음
        DebugOut( "\r\n<OFFLINE_IDCENTER> File Format 안맞음 1\n" );
        fclose( fdIDCenter );
        return ErrRet( ERR_MAINSUB_COMM_IDCENTER_FILE_APPLY_DATA );
    }

    // Record 건수 2자리
    if ( ( nReadSize = fread( abTmpBuff, 2, 1, fdIDCenter ) ) != 1 )
    {
        // Format 안맞음
        DebugOut( "\r\n<OFFLINE_IDCENTER> File Format 안맞음 2\n" );
        fclose( fdIDCenter );
        return ErrRet( ERR_MAINSUB_COMM_IDCENTER_FILE_RECORD_CNT );
    }

    nRecCnt = GetDWORDFromASC( abTmpBuff, 2 );
    DebugOut( "건수는=>[%d]", nRecCnt );

    if ( nRecCnt < 1 || nRecCnt > 99 )
    {
        // Record 건수가 없음
        DebugOut( "\r\n<OFFLINE_IDCENTER> Record 건수가 없음\n" );
        fclose( fdIDCenter );
        return ErrRet( ERR_MAINSUB_COMM_IDCENTER_FILE_RECORD_CNT );
    }
    else
    {
        DebugOut( "\r\n<OFFLINE_IDCENTER> Record 건수[%d]\n", nRecCnt );
    }

    // PSAM으로 다운로드
    for ( i = 0; i < nRecCnt; i++ )
    {
        nReadSize = fread( &stIDCenterData, sizeof(stIDCenterData), 1, fdIDCenter );

        memset( abTmpBuff, 0x00, sizeof( abTmpBuff ) );

        if ( nReadSize != 1 )
        {
            DebugOut( "Format 안맞음\n" );
            sRetVal = ERR_MAINSUB_COMM_IDCENTER_FILE_READ_OR_EOF;
            break; // Format 안맞음
        }

        memcpy( abTmpBuff, stIDCenterData.abSAMID, 8 );
        memcpy( &abTmpBuff[8], stIDCenterData.abEKV, 16 );
        memcpy( &abTmpBuff[24], stIDCenterData.abSign, 4 );

        BCD2ASC( abTmpBuff, abTmpSAMID, 8 );

        if ( gboolIsMainTerm == TRUE )
        {
#ifdef TEST_IDCENTER_KEYSET_REGIST
            PrintlnASC( "\n내 PSAMID : ", gpstSharedInfo->abMainPSAMID, 16 );
#endif

            if ( memcmp( abTmpSAMID, gpstSharedInfo->abMainPSAMID, 16 ) == 0 )
            {
#ifdef TEST_IDCENTER_KEYSET_REGIST
                printf( "<OFFLINE_IDCENTER> 승차 샘아이디와 같은 데이터 발견\n" );
#endif
            }
            else
            {
#ifdef TEST_IDCENTER_KEYSET_REGIST
                printf( "<OFFLINE_IDCENTER>SAM-ID [%s]\n", abTmpSAMID );
                printf( "<OFFLINE_IDCENTER>IDCENTER Version : [%02x] \n",
                        stIDCenterData.bIDCenter );
                printf( "<OFFLINE_IDCENTER>승차 샘아이디와 다른 데이터\n" );
#endif
                continue;
            }
        }
        else
        {
#ifdef TEST_IDCENTER_KEYSET_REGIST
            PrintlnASC( "\n내 PSAMID : ",
                        gpstSharedInfo->abSubPSAMID[gbSubTermNo-1], 16 );
#endif
            if ( memcmp( abTmpSAMID,
                         gpstSharedInfo->abSubPSAMID[gbSubTermNo-1],
                         16 ) == 0 )
            {
#ifdef TEST_IDCENTER_KEYSET_REGIST
                printf( "<OFFLINE_IDCENTER> 하차 샘아이디와 같은 데이터 발견\n" );
#endif
            }
            else
            {
#ifdef TEST_IDCENTER_KEYSET_REGIST
                printf( "<OFFLINE_IDCENTER>SAM-ID [%s]\n", abTmpSAMID );
                printf( "<OFFLINE_IDCENTER>IDCENTER Version : [%02x] \n",
                        stIDCenterData.bIDCenter );
                printf( "<OFFLINE_IDCENTER>하차 샘아이디와 다른 데이터\n" );
#endif
                continue;
            }
        }

        sRet = PSAMAddCenterOffline( stIDCenterData.bIDCenter, abTmpBuff );

#ifdef TEST_IDCENTER_KEYSET_REGIST
        printf( "<OFFLINE_IDCENTER> 실행결과 [%d] \n", sRet);
#endif
        if ( ( sRet == SUCCESS ) ||
             ( sRet == ERR_SAM_IF_PSAM_ENREGIST_IDCENTER ) )
        {
            sRetVal = SUCCESS;
//            printf( "IDCENTER 등록 완료 \n" );
        }
        else
        {
            sRetVal = ERR_MAINSUB_COMM_ADD_IDCENTER_OFFLINE;
        }

        break;

    }

    fclose( fdIDCenter );

    return ErrRet( sRetVal );
}


//******************************************************************************
//  OFFLINE KEYSET DOWNLOAD
//  Return : 0 = OK, 1 = NG
//******************************************************************************
short RegistOfflineKeyset2PSAMbyEpurseIssuer( void )
{
    short   sRetVal = -1;
    short   sRet;
    FILE    *fdKeyset;
    int     nRecCnt;
    int     nReadSize;
    int     i;
    byte    abTmpBuff[90] = { 0, };
    byte    abTmpSAMID[17] ={ 0, };

    DebugOut( "\r\n<OFFLINE_KEYSET> KEYSET 등록\r\n " );

    fdKeyset = fopen( PSAM_KEYSET_INFO_FILE, "rb" );
    if ( fdKeyset == NULL )
    {
        DebugOut( "\r\n<OFFLINE_IDCENTER> 파일 오픈 에러 \n" );
        return ErrRet( ERR_MAINSUB_COMM_KEYSET_FILE_OPEN);
    }

    // 적용년월일 7자리
    if ( ( nReadSize = fread( abTmpBuff, 7, 1, fdKeyset ) ) != 1 )
    {
        // Format 안맞음
        DebugOut( "\r\n<OFFLINE_KEYSET> 적용년월일 7자리 // Format 안맞음\n" );
        fclose( fdKeyset );
        return ErrRet( ERR_MAINSUB_COMM_KEYSET_FILE_APPLY_DATA );
    }

    // Record 건수 2자리
    if ( ( nReadSize = fread( abTmpBuff, 2, 1, fdKeyset ) ) != 1 )
    {
        // Format 안맞음
        DebugOut( "\r\n<OFFLINE_KEYSET> Record 건수 2자리 // Format 안맞음\n" );
        fclose( fdKeyset );
        return ErrRet( ERR_MAINSUB_COMM_KEYSET_FILE_RECORD_CNT );
    }

    nRecCnt = GetDWORDFromASC( abTmpBuff, 2 );
    DebugOut( "건수는 =>%d", nRecCnt );

    if ( nRecCnt < 1 || nRecCnt > 99 )
    {
        // Record 건수가 없음
        DebugOut( "\r\n<OFFLINE_KEYSET> Record 건수가 없음\n" );
        fclose( fdKeyset );
        return ErrRet( ERR_MAINSUB_COMM_KEYSET_FILE_RECORD_CNT );
    }
    else
    {
        DebugOut( "\r\n<OFFLINE_KEYSET> Record 건수[%d]\n", nRecCnt );
    }


    // SAM으로 다운로드
    for ( i = 0; i < nRecCnt; i++ )
    {
        nReadSize = fread( &stKeySetData, sizeof( stKeySetData ), 1, fdKeyset );

        memset( abTmpBuff, 0x00, sizeof( abTmpBuff ) );

        if ( nReadSize != 1 )
        {
            sRetVal = ERR_MAINSUB_COMM_KEYSET_FILE_READ_OR_EOF;
            break; // Format 안맞음
        }

        memcpy( abTmpBuff, stKeySetData.abSAMID, 8 );
        abTmpBuff[8] =  stKeySetData.bSortKey;
        memcpy( &abTmpBuff[9], stKeySetData.abVK, 4 );
        memcpy( &abTmpBuff[13], stKeySetData.abEKV, 64 );
        memcpy( &abTmpBuff[77], stKeySetData.abSign, 4 );

        BCD2ASC( abTmpBuff, abTmpSAMID, 8 );

        if( gboolIsMainTerm == TRUE )
        {
#ifdef TEST_IDCENTER_KEYSET_REGIST
            PrintlnASC( "\n내 PSAMID : ",
                                   gpstSharedInfo->abMainPSAMID, 16 );
#endif
            if ( memcmp( abTmpSAMID, gpstSharedInfo->abMainPSAMID, 16 ) == 0 )
            {
#ifdef TEST_IDCENTER_KEYSET_REGIST
               printf( "<OFFLINE_KEYSET>  승차 샘아이디와 같은 데이터 발견 \n" );
#endif
            }
            else
            {
#ifdef TEST_IDCENTER_KEYSET_REGIST
                printf( "<OFFLINE_KEYSET>PSAM-ID [%s]\n", abTmpSAMID );
                printf( "<OFFLINE_KEYSET>KEYSET idcenter : [%02x] \n",
                        stKeySetData.bIDCenter );
                printf( "<OFFLINE_KEYSET>승차 샘아이디와 다른 데이터\n" );
#endif
                continue;
            }
        }
        else
        {
#ifdef TEST_IDCENTER_KEYSET_REGIST
            PrintlnASC( "\n내 PSAMID : ",
                        gpstSharedInfo->abSubPSAMID[gbSubTermNo-1], 16 );
#endif
            if ( memcmp( abTmpSAMID,
                         gpstSharedInfo->abSubPSAMID[gbSubTermNo-1],16
                       ) == 0 )
            {
#ifdef TEST_IDCENTER_KEYSET_REGIST
                printf( "<OFFLINE_KEYSET> 하차 샘아이디 같음 \n" );
#endif
            }
            else
            {
#ifdef TEST_IDCENTER_KEYSET_REGIST
                printf( "<OFFLINE_KEYSET>SAM-ID [%s]\n", abTmpSAMID );
                printf( "<OFFLINE_KEYSET>KEYSET idcenter : [%02x] \n",
                        stKeySetData.bIDCenter );
                printf( "<OFFLINE_KEYSET>하차 샘아이디 틀림\n" );
#endif
                continue;
            }
        }

        sRet = PSAMAddKeySetOffline( stKeySetData.bIDCenter, abTmpBuff );
#ifdef TEST_IDCENTER_KEYSET_REGIST
        printf( "\r\n<OFFLINE_KEYSET> 실행결과 [%d] \n", sRet);
#endif
        if ( ( sRet == SUCCESS ) ||
             ( sRet == ERR_SAM_IF_PSAM_ENREGIST_IDCENTER ) )
        {
            sRetVal = SUCCESS;
//            printf( "keyset 등록 완료 \n" );
        }
        else
        {
            sRetVal = ERR_MAINSUB_COMM_ADD_IDCENTER_OFFLINE;
        }
        break;

    }

    fclose( fdKeyset );

    return sRetVal;
}



















































