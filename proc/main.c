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
*  PROGRAM ID :       main.c                                                   *
*                                                                              *
*  DESCRIPTION:       This Program starts/ends Execution, initializes Term,    *
*                     checks basic datas and loops continually.                *
*                                                                              *
*  ENTRY POINT:       main ()               ** mandatory **                    *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: 1 - Program Exit                                         *
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
*    DATE                SE NAME              DESCRIPTION                      *
* ---------- ---------------------------- -------------------------------------*
* 2005/08/09 Solution Team Gwan Yul Kim  Initial Release                       *
*                                                                              *
*******************************************************************************/

/*******************************************************************************
*  Inclusion of System Header Files                                            *
*******************************************************************************/
#include <unistd.h>

/*******************************************************************************
*  Inclusion of User Header Files                                              *
*******************************************************************************/
#include "../system/bus_type.h"
#include "../system/bus_define.h"
#include "../system/device_interface.h"

#include "main.h"
#include "main_environ_init.h"
#include "main_load_criteriaInfo.h"
#include "main_process_busTerm.h"
#include "main_childProcess_manage.h"
#include "file_mgt.h"
#include "../comm/socket_comm.h"

/*******************************************************************************
*  Declaration of Global Variables                                             *
*******************************************************************************/
bool gboolIsMainTerm = FALSE;		// Div of MainTerm & SubTerm
PROC_SHARED_INFO *gpstSharedInfo = NULL;	// 공유메모리 포인터
byte gbSubTermCnt = 0;				// 하차단말기 갯수
MYTERM_INFO gstMyTermInfo;			// 프로그램이 구동되는 Terminal 정보
pthread_t nCommGPSThreadID = 0;

MULTI_ENT_INFO gstMultiEntInfo;			// 다인승정보
CITY_TOUR_BUS_TICKET_INFO gstCityTourBusTicketInfo;
										// 시티투어버스 승차권입력정보

// Message Queue for Printing
int gnMsgQueue = 0;

byte gabKernelVer[3];
byte gbSubTermNo;				// # of SubTerm

byte gabGPSStationID[7];				// GPS에서만 업데이트하는 정류장ID
int gnGPSStatus;						// GPS Status
time_t gtDriveStartDtime;				// 운행시작시간
bool gboolIsRunningGPSThread = FALSE;	// GPS쓰레드 실행 여부 (GPS로그 작성 관련)
word gwDriveCnt;						// 운행횟수
word gwGPSRecvRate;						// GPS 수신율
dword gdwDistInDrive;					// 운행중 이동한 거리
bool gboolIsEndStation = 0;				// 종점도착여부
dword gdwDelayTimeatEndStation = 0;		// 종점에 도착한뒤 delay time

/*******************************************************************************
*  Declaration of Module Variables                                             *
*******************************************************************************/
static bool StartVoiceMent( void );
static void RebootSystem( NEW_IMGNVOICE_RECV* pstNewImgNVoiceRecv );

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      main                                                     *
*                                                                              *
*  DESCRIPTION :      BUS프로그램 Main                                         *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: 1                                                        *
*                                                                              *
*  Author : GwanYul Kim													   	   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
int main( void )
{
	bool boolIsMentAlreadyOut = FALSE;
	NEW_IMGNVOICE_RECV stNewImgNVoiceRecv;

	memset( &stNewImgNVoiceRecv, 0x00, sizeof(NEW_IMGNVOICE_RECV) );
	stNewImgNVoiceRecv.boolIsReset = FALSE;

    /*
     *  Device Open
     */
	OpenDeviceFiles();

	InitLog();
	LogMain("단말기시작\n");

    /*
     *  프로그램이 구동되기위한 기본 환경사항 CHECK
     */
	CheckEnvironment();

    /*
     *   SAM 초기화
     */
	SAMInitialization();

    /*
     *  RF 초기화
     */
	RFInitialization();

    /*
     *  CHILD PROCESS CHECK
     */
	CreateChildProcess();

    /*
     *  집계에서 수신받은 기본정보 LOADING
     */
	OperParmNBasicInfo( &stNewImgNVoiceRecv );

	while( stNewImgNVoiceRecv.boolIsReset == FALSE )
	{
	    /*
	     *  프로그램시작메세지는 BOOTING후 1회만 출력함
	     */
		if ( boolIsMentAlreadyOut == FALSE )
		{
			if( StartVoiceMent() == TRUE )
			{
				boolIsMentAlreadyOut = TRUE;
			}
		}

		CheckNDisplayDynamicInfo();

	    /*
	     *  운행중: CARD처리. 비운행중: BLPL MERGE CHECK
	     */
		MainProc( &stNewImgNVoiceRecv );

	    /*
	     *  CHILD PROCESS CHECK
	     */
		CheckChildProcess();
	}

	KillChildProcess();

    /*
     *  집계PC에서 신규실행파일/음성정보수신시 재부팅함
     */
	RebootSystem( &stNewImgNVoiceRecv );

	exit( 1 );
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      StartVoiceMent                                           *
*                                                                              *
*  DESCRIPTION :      "운행을 시작합니다." 메세시출력                          *
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
static bool StartVoiceMent( void )
{
	bool boolIsStarted = FALSE;
	short sDateGap = 0;
	char achBuf[100] = { 0, };

	if ( gboolIsMainTerm == TRUE )
	{
	    /*
	     *  TermComm에서 운행준비될때까지 기다림
	     */
		if ( (gpstSharedInfo->boolIsReadyToDrive == TRUE) &&
			 (gpstSharedInfo->boolIsKpdLock != TRUE) &&
			 (gpstSharedInfo->boolIsKpdImgRecv == FALSE))
		{
			sleep(1);

			if( CheckMainTermBasicInfoFile() == SUCCESS )
			{
				/*
				 * 운영정보수신일짜가 4일 이상이면 수신요청음성메세지 출력
				 */
				memset( achBuf, 0x00, sizeof(achBuf) );
				sDateGap = GetCommSuccTimeDiff( achBuf );
				if( 4 <= sDateGap )
				{
					DebugOut("DCS 와 접속한지 [%d]일 \n", sDateGap );
					DisplayASCInUpFND( FND_ERR_RECEIVE_LATEST_INFO );
					DisplayASCInDownFND( achBuf );
					Buzzer( 5, 50000 );

					/*
					 * 최신 운행정보를 수신하여 주십시요.
					 */
					VoiceOut( VOICE_RECEIVE_LATEST_INFO );
					sleep( 3 );
				}

				if ( IsDriveNow() != SUCCESS )
				{
					sleep( 3 );
				}

				boolIsStarted = TRUE;
				VoiceMent2EndUser();
			}
		}
	}
	else
	{
	    /*
	     *  하차기에서는 바로 프로그램시작메세지를 출력함
	     */
	    boolIsStarted = TRUE;
		VoiceMent2EndUser();
	}

	return boolIsStarted;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      RebootSystem                                             *
*                                                                              *
*  DESCRIPTION :      시스템을 재부팅함                                        *
*                                                                              *
*  INPUT PARAMETERS:  NEW_IMGNVOICE_RECV* pstNewImgNVoiceRecv                  *
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
static void RebootSystem( NEW_IMGNVOICE_RECV* pstNewImgNVoiceRecv )
{
	byte abBuf[100] = { 0, };

    /*
     *  신규 실행프로그램을 집계PC에서 수신함
     */
	if ( pstNewImgNVoiceRecv->boolIsImgRecv == TRUE )
	{
		sprintf( abBuf, "cp %s/%s %s/%s",
			BUS_EXECUTE_DIR, BUS_EXECUTE_BACKUP_FILE,
			BUS_BACKUP_DIR, BUS_EXECUTE_BEFORE_BACKUP_FILE );
		system( abBuf );

		sprintf( abBuf, "%s/%s", BUS_EXECUTE_DIR, BUS_EXECUTE_BACKUP_FILE );
		unlink( abBuf );

		sprintf( abBuf, "cp %s/%s %s/%s",
			BUS_EXECUTE_DIR, BUS_EXECUTE_FILE,
			BUS_EXECUTE_DIR, BUS_EXECUTE_BACKUP_FILE );
		system( abBuf );
	}

	/*
	 * 운전자조작기 프로그램 송수신중에는 WAITING한다.
	 */
/*	while ( gpstSharedInfo->boolIsKpdImgRecv == TRUE )
	{
		usleep(100000);
	}	*/

    /*
     *  Main Process Exit Log 생성
     */
	LogWrite( ERR_SETUP_DRIVE_MAIN_EXIT );

    /*
     *  Delete Shared Memory Buffer
     */
	MemoryRemoval();

    /*
     *  Close Peripherial Device
     */
	ClosePeripheralDevices();

	unlink( STATUS_FLAG_FILE );
	system( "sync" );

	system( "reset" );
}

