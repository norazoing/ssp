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
*  PROGRAM ID :       main_Load_CriteriaInfo.c                                 *
*                                                                              *
*  DESCRIPTION:       MAIN 프로그램을 구동하기위한 기본정보 Load.              *
*                                                                              *
*  ENTRY POINT:     short OperParmNBasicInfo (void)                            *
*                   short VoiceVerLoad( void )								   *
*                                                                              *
*  INPUT FILES:     None                                                       *
*                                                                              *
*  OUTPUT FILES:    None                                                       *
*                                                                              *
*  SPECIAL LOGIC:   None                                                       *
*                                                                              *
********************************************************************************
*                         MODIFICATION LOG                                     *
*                                                                              *
*    DATE                SE NAME                      DESCRIPTION              *
* ---------- ---------------------------- ------------------------------------ *
* 2006/03/27 F/W Dev Team GwanYul Kim  Initial Release                         *
*                                                                              *
*******************************************************************************/

/*******************************************************************************
*  Inclusion of System Header Files                                            *
*******************************************************************************/
#include <sys/stat.h>
#include <fcntl.h> 

/*******************************************************************************
*  Inclusion of User Header Files                                              *
*******************************************************************************/
#include "main_load_criteriaInfo.h"
#include "Load_parameter_file_mgt.h"
#include "main_process_busTerm.h"
#include "main_environ_init.h"
#include "main.h"
#include "Blpl_proc.h"
#include "../system/device_interface.h"
#include "../system/gps.h"

/*******************************************************************************
*  Declaration of Module Functions                                             *
*******************************************************************************/
static short CheckNRecvVehicleParmFile(  byte* pabCmdSharedData  );
static bool IsRTCOK( void );

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      OpenDeviceFiles                                          *
*                                                                              *
*  DESCRIPTION :      주변기기 DEVICE OPEN                                     *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: void                                                     *
*                                                                              *
*  Author  : Gwan Yul Kim                                                      *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short OperParmNBasicInfo ( NEW_IMGNVOICE_RECV* pstNewImgNVoiceRecv )
{
	short sRetVal = 0;
	byte abCmdSharedData[41] = { 0, };
	time_t tNowDtime = 0;
	
	VoiceVerLoad();
	if ( gboolIsMainTerm == TRUE )
	{
		IsRTCOK();
		
		sRetVal = CheckNRecvVehicleParmFile( abCmdSharedData );
		if ( sRetVal < 0 ) return sRetVal;
		
		/*
		 * 신운영정보, 신F/W수신, DRIVER IMAGE수신 체크
		 */
		abCmdSharedData[0] = 0x00;	
		CheckNewCriteriaInfoRecv( abCmdSharedData, 4, pstNewImgNVoiceRecv );
	}		
	else
	{
		if ( SUCCESS != access(VEHICLE_PARM_FILE, F_OK) )
		{
			return ErrRet( ERR_FILE_OPEN | GetFileNo(VEHICLE_PARM_FILE) );
		}
	}
	
	/*
	 * 초기 기준정보변수 초기화
	*/
	InitOperParmInfo();
	
	/*
	 * DCS에서 Para수신후 ConfInfo Reloading
	*/
	LoadOperParmInfo();

	if ( gboolIsMainTerm == TRUE )
	{
		/*
		 * 차량번호를 화면에 DISPLAY
		 */
		DisplayVehicleID();	
		
		/*
		 * 필요시 BLPL Merge실행
		 */
		if ( gpstSharedInfo->boolIsBLPLMergeNow != TRUE )
		{
			BLPLMerge();			
			sleep( 1 );
		}
	}

	// 승차단말기의 경우에만 체크
	if ( gboolIsMainTerm == TRUE )
	{
		// 운행중 부팅이면 "8001" 알람코드를 서버로 전송
		if ( IsExistFile( CONTROL_TRANS_FILE ) == TRUE )
		{
			printf( "[OperParmNBasicInfo] 운행중 부팅\n" );
			ctrl_event_info_write( EVENT_BOOTING_DURING_DRIVING );
		}

		// RTC 시간이 하드코딩한 시간을 벗어나면 "8003" 알람코드를 서버로 전송
		GetRTCTime( &tNowDtime );
		if ( tNowDtime < GetTimeTFromASCDtime( "20060101000000" ) ||
			 tNowDtime > GetTimeTFromASCDtime( "20100101000000" ) )
		{
			printf( "[OperParmNBasicInfo] RTC 시간 오류\n" );
			ctrl_event_info_write( EVENT_RTC_ERROR );
		}
	}

	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      CheckNRecvVehicleParmFile                                *
*                                                                              *
*  DESCRIPTION :      TermComm Process에게 Setup을 요청함                      *
*                                                                              *
*  INPUT PARAMETERS:  SUCCESS                                                  *
*                     - FAIL                                                   *
*                                                                              *
*  RETURN/EXIT VALUE: void                                                     *
*                                                                              *
*  Author  : Gwan Yul Kim                                                      *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short CheckNRecvVehicleParmFile( byte* pabCmdSharedData )
{
	short sRetVal = 0;
	byte bCmd = 0;
	byte abCmdSharedData[41] = { 0, };
	word wCmdDataSize = 0;
	byte bRequestResult = 0;
		
	memset( abCmdSharedData, 0x00, sizeof(abCmdSharedData) );
	abCmdSharedData[0] = CMD_REQUEST;	
	SetSharedCmdnDataLooping( CMD_SETUP, abCmdSharedData, 1 );
	
	bRequestResult = 0;
	while ( bRequestResult != CMD_SUCCESS_RES && 
			bRequestResult != CMD_FAIL_RES )
	{
		bCmd = '0';
		memset( abCmdSharedData, 0x00, sizeof(abCmdSharedData) );
		wCmdDataSize = 0;

		sRetVal = GetSharedCmd( &bCmd, abCmdSharedData, &wCmdDataSize );		
		if ( sRetVal == SUCCESS )
		{
			if ( bCmd == CMD_SETUP )
			{
				bRequestResult = abCmdSharedData[0];
			}
		}

		usleep( 1000000 );					// 1초 SLEEP
	}

	ClearSharedCmdnData( CMD_SETUP );

	if ( bRequestResult == CMD_FAIL_RES )
	{
		return ( ERR_FILE_OPEN | GetFileNo( VEHICLE_PARM_FILE ) );
	}

	memcpy( pabCmdSharedData, &abCmdSharedData[1], 40 );
		
	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      IsRTCOK                                                  *
*                                                                              *
*  DESCRIPTION :      단말기 RTC정상 CHECK함                                   *
*                                                                              *
*  INPUT PARAMETERS:  SUCCESS                                                  *
*                     - FAIL                                                   *
*                                                                              *
*  RETURN/EXIT VALUE: void                                                     *
*                                                                              *
*  Author  : Gwan Yul Kim                                                      *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static bool IsRTCOK( void )
{
	short sRetValue = 0;
	time_t tNowTime = 0;

	while( TRUE )
	{
		sRetValue = GetRTCTime( &tNowTime );
		if ( sRetValue < 2 )
		{
			if ( tNowTime > GetTimeTFromASCDtime("20050901000101") )
			{
				DebugOut( "\n====<LoadingInitConfInfo>RTC OK.====\n" );
				/*
				 * RTC OK
				*/
				break;
			}
		}

		DebugOut( "\n====<MAIN>RTC비정상. 보정대기중.====\n" );
		DisplayASCInUpFND( FND_INIT_MSG );
		DisplayASCInDownFND( FND_INIT_MSG );
		Buzzer( 5, 50000 );

		/* 
		 * Msg of Term Time Checking
		*/
		VoiceOut( VOICE_CHECK_TERM_TIME );
		if ( nCommGPSThreadID != 0 )
		{
			/*
			 * 2Minute of GPS Setting
			*/
			sleep( 2 );
		}
		else
		{
			GpsTimeCheck();
			sleep( 3 );
		}
	}

	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      VoiceVerLoad                                             *
*                                                                              *
*  DESCRIPTION:       Reads MainTerm/ SubTerm VoiceFile Version.               *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: 0 - success                                              *
*                     (-) - fail                                               *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short VoiceVerLoad( void )
{
	short sRetVal = 0 ;
	int fdVoiceVer = -1;
	int nIndex = 0;

	char achVoiceVer[5];

	memset( achVoiceVer,0x00,sizeof(achVoiceVer) );

	sRetVal = access( VOICEAPPLY_VERSION, F_OK );
	if ( sRetVal != SUCCESS )
	{
		DebugOut( "\r\n 음성 버전 파일이 없음\r\n" );
		return ErrRet( ERR_FILE_OPEN );
	}

	fdVoiceVer = open( VOICEAPPLY_VERSION, O_RDONLY );
	if ( fdVoiceVer < 0 )
	{
		return ErrRet( ERR_FILE_OPEN );
	}

	memset( achVoiceVer, 0x00, sizeof(achVoiceVer) );
	sRetVal = read( fdVoiceVer, achVoiceVer, 4 );
	close( fdVoiceVer );
	if ( sRetVal < SUCCESS )
	{
		DebugOut( "\r\n 음성 버전 파일 read Fail\r\n" );
		return ErrRet( ERR_FILE_OPEN );
	}

	if ( gboolIsMainTerm == TRUE )
	{
		memcpy( gpstSharedInfo->abMainVoiceVer, achVoiceVer,
			sizeof(gpstSharedInfo->abMainVoiceVer) );
	}
	else
	{
		for ( nIndex = 0; nIndex < gbSubTermCnt; nIndex++ )
		{
			memcpy( gpstSharedInfo->abSubVoiceVer[nIndex], achVoiceVer,
			sizeof(gpstSharedInfo->abSubVoiceVer[nIndex]) );
		}
	}
	
	/*
	 * My Voice File Version
	*/
	memcpy( gstMyTermInfo.abVoiceVer, achVoiceVer, 4 );

	return SUCCESS;
}

