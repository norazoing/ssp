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
*  PROGRAM ID :       main_environ_init.c                                      *
*                                                                              *
*  DESCRIPTION:       MAIN 프로그램을 구동하기위한 주변장치를 check한다.       *
*                                                                              *
*  ENTRY POINT:     void OpenDeviceFiles (void);                               *
*                   void CheckEnvironment( void );                             *
*                   bool SAMInitialization (void);                             *
*                   void RFInitialization(void);                               *
*                   void VoiceMent2EndUser(void);                              *
*                   void MemoryRemoval( void );                                *
*                   void ClosePeripheralDevices ( void );                      *
*                   void CreateSharedMemNMsgQueue( void );                     * 
*                   void CheckFreeMemory( void );                              *
*                   short IsDriveNow( void );								   *
*                   void DisplayVehicleID( void )                              *
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
*  Inclusion of User Header Files                                              *
*******************************************************************************/
#define _GNU_SOURCE
#include "../system/bus_type.h"
#include "../system/device_interface.h"
#include "write_trans_data.h"
#include "file_mgt.h"
#include "../comm/term_comm.h"
#include "../comm/term_comm_mainterm.h"
#include "../comm/socket_comm.h"
#include "main_childProcess_manage.h"
#include "../system/gps.h"

#include "main_environ_init.h"

/*******************************************************************************
*  Declaration of Global Variables                                             *
*******************************************************************************/
bool gboolIsDisplaySwitchOn = FALSE;		// FND에 정보가 Display여부

/*******************************************************************************
*  Declaration of Module Functions                                             *
*******************************************************************************/
static void EnvironmentInit( void );
static bool MainGetSAMPort(void);
static bool MainGetSAMID( void );
static void SystemErrorMsg2EndUser(byte* pbFirstLineMsg, byte* pbSecondLineMsg);
static void DetermineTermType(void);
static void DisplayImgVer( void );
static void DisplayKernelRamdiskVer( void );
static short ApplyVoiceData2Flash( void );
static void CheckSubTransSendCompl( void );
static word CheckSubTransSendFailCount( void );
static void MainTermIDLoad( void );
static void SubTermIDCountLoad( void );

/*******************************************************************************
*  Declaration of Module Variables                                             *
*******************************************************************************/
static int gnMsgQueueID = 0;
static int gnSharedMemID = 0;

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
void OpenDeviceFiles(void)
{	
    /*
     *  RTC Device Open
     */
  	if ( OpenRTC() != SUCCESS )
		DebugOut( "RTC open error1\n" );
	
    /*
     *  Voice Device Open
     */
	if ( OpenVoice() != SUCCESS )
		DebugOut( "Voice open error1\n" );

    /*
     *  FND Device Open
     */
	if ( OpenBuzz() != SUCCESS )
		DebugOut( "buzz open error\n" );
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      CheckEnvironment                                         *
*                                                                              *
*  DESCRIPTION:      Main Program이 구동되기위한 주변기기들을 CHECK한다.       *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: None                                                     *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
void CheckEnvironment( void )
{
	word wUnsendCnt = 0;
	char acUnsendCnt[10] = { 0, };

	/*
	 * 3개의 Process가 사용하는 공유메모리생성
	*/
	CreateSharedMemNMsgQueue();	
	
	/*
	 * 승하차 단말기 결정
	*/
	DetermineTermType();
	
	/*
	 * 환경변수 초기화
	*/
	EnvironmentInit();	
	
	/*
	 * KERNEL과 RAMDIS 버전을 화면에 보여줌
	*/
	DisplayKernelRamdiskVer();
	
	sleep( 2 );

	if ( GetDWORDFromASC(gabKernelVer, 2) > 23 )
	{
		while( SUCCESS != CommOpen(DEV_MAINSUB) )
		{
			DisplayASCInUpFND( FND_ERR_MAIN_SUB_COMM_PORT );
			DisplayASCInDownFND( FND_ERR_MAIN_SUB_COMM_PORT );

			if ( gboolIsMainTerm == TRUE )
			{
				printf( "[CheckEnvironment] CommOpen() 실패\n" );
				Buzzer( 5, 50000 );
				/*
				 * 단말기를 점검 하시기 바랍니다.
				*/
				VoiceOut( VOICE_CHECK_TERM );
				sleep( 3 );
			}
		}

		CloseMainSubComm();
	}	
	
	/*
	 * BUS IMAGE버전을 화면에 보여줌
	*/
	DisplayImgVer();	
	
	if ( gboolIsMainTerm == TRUE )
	{
		CheckFreeMemory();		
		MainTermIDLoad();
	}
	else
	{
		CheckSubTransSendCompl();
		wUnsendCnt = CheckSubTransSendFailCount();
		if ( wUnsendCnt > 9 )
		{
			DebugOut ( "<CheckTerm>SubTerm Unsend Count[%d] \r\n ", wUnsendCnt );
			sprintf( acUnsendCnt,"%6d", wUnsendCnt );

			SystemErrorMsg2EndUser (FND_ERR_SUBTERM_TRN_EXIST, acUnsendCnt );
		}
	}
	
	SubTermIDCountLoad();
		
	if ( ApplyVoiceData2Flash() != SUCCESS )
	{
		ErrLogWrite( ERR_SETUP_DRIVE_VOICEDATA_2_FLASH );
	}
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SAMInitialization                                        *
*                                                                              *
*  DESCRIPTION:      SAM의 통신PORT초기화하며 ID를 구함                        *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: None                                                     *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
bool SAMInitialization (void)
{	
	bool boolIsInitSucc = FALSE;

	boolIsInitSucc = MainGetSAMPort();
	if ( boolIsInitSucc == TRUE )
	{
		boolIsInitSucc = MainGetSAMID();
	}

	return boolIsInitSucc;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      MemoryRemoval                                            *
*                                                                              *
*  DESCRIPTION:      Return of Shared Memory                                   *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: None                                                     *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                             RFInitialization                                 *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/	
void RFInitialization(void)
{
	// RF Open
	while( TRUE )
	{
		if( InitRFComm() == SUCCESS ) break;
		
		SystemErrorMsg2EndUser( FND_ERR_RF_INIT, FND_ERR_RF_INIT );
		sleep( 5 );
	}
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      VoiceMent2EndUser                                        *
*                                                                              *
*  DESCRIPTION:      운행시작메세지 출력                                       *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: None                                                     *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
void VoiceMent2EndUser(void)
{
	bool boolIsDriveNow = 0x00;

	boolIsDriveNow = gpstSharedInfo->boolIsDriveNow;
	if ( (gboolIsMainTerm == TRUE) && (boolIsDriveNow == TRUE) )
	{
		VoiceOut( VOICE_START_DRIVE ); // Msg. of Driving Start. 0327
		sleep(2);
	}

	if ( gboolIsMainTerm == TRUE )
	{
		Buzzer( 3, 1000000 );			// 1초 간격으로 3회 부저 출력
	}
	else
	{
		WriteBuzz( "1000", 300 );
		sleep( 1 );
		WriteBuzz( "1000", 300 );
	}

	DisplayDWORDInUpFND( 0 );
	usleep( 20000 );
	DisplayASCInDownFND( FND_READY_MSG );
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      EnvironmentInit                                          *
*                                                                              *
*  DESCRIPTION:      프로그램시작시 변구초기화                                   *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: None                                                     *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static void EnvironmentInit( void )
{
	time_t tNowTime = 0;
	byte abBuf[100] = { 0, };
	
	gpstSharedInfo->boolIsGpsValid = GPS_DATA_VALID;

	memset( &gstMultiEntInfo, 0x00, sizeof( MULTI_ENT_INFO ) );
	memset( &gstCityTourBusTicketInfo, 0x00,
		sizeof( CITY_TOUR_BUS_TICKET_INFO) );
	memset( &gstMyTermInfo, 0x00, sizeof( MYTERM_INFO ) );

	sprintf( abBuf, "chmod 755 %s/* ", BUS_EXECUTE_DIR );		
	system( abBuf );
	
	unlink( STATUS_FLAG_FILE );
	if ( gboolIsMainTerm == TRUE )
	{
		sprintf( abBuf, "rm %s/2*.tmp", BUS_EXECUTE_DIR );
		system( abBuf );
		sprintf( abBuf, "rm %s/2*.*0*", BUS_EXECUTE_DIR );
		system( abBuf );

		memcpy( gpstSharedInfo->abMainVer, MAIN_RELEASE_VER, 
			sizeof(MAIN_RELEASE_VER) );
	}
	else
	{
		memcpy( gpstSharedInfo->abSubVer[gbSubTermNo-1], MAIN_RELEASE_VER, 
			sizeof(MAIN_RELEASE_VER) );
	}
			
    GetRTCTime( &tNowTime );
    gpstSharedInfo->tTermTime = tNowTime;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      MemoryRemoval                                            *
*                                                                              *
*  DESCRIPTION:      Return of Shared Memory                                   *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: None                                                     *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
void MemoryRemoval( void )
{
	struct shmid_ds stShmID;

	memset( &stShmID, 0x00, sizeof(struct shmid_ds) );
	semctl( gnSharedMemID, 0, IPC_RMID, &stShmID );

	/*
	* 메세지큐 반납
	*/
	msgctl( gnMsgQueue, IPC_RMID, NULL );
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      ClosePeripheralDevices                                   *
*                                                                              *
*  DESCRIPTION:      Close All Peripherial Deivces                             *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: None                                                     *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
void ClosePeripheralDevices ( void )
{
	CloseRTC();
	CloseVoice();
	CloseBuzz();
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      MainGetSAMPort                                           *
*                                                                              *
*  DESCRIPTION :      SAM PORT를 획득함                                        *
*                                                                              *
*  INPUT PARAMETERS:  NONE                                                     *
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
static bool MainGetSAMPort(void)
{
	short sRetVal = 0;
	int nISAMRet = 0;
	int nPSAMRet = 0;
	byte wUnsendCnt = 0;
	byte abFirstLineMsg[7] = { 0, };
	byte abSecondLineMsg[7] = { 0, };

	/*
	 * SAM이 단말기에 실제로 존재하는 PORT번호.
	 * SAM이 없으면 -1을 PORT번호에 지정함. 		 
	 */
	/* 
	 * UART PORT에 연결된 SIM TYPE SAM 존재여부 CHECK. 
	 */
	int anSIMUARTPortNo[]	= {0, 1, -1, -1};		
	
	/* 
	 * Sniper에 연결된 SIM TYPE SAM 존재여부 CHECK. 
	 */
	int anSIMSniperPortNo[] = {0, 1, -1, -1}; 
	
	/* 
	 * PLCC TYPE SAM 존재여부 CHECK
	 */	
	int anPLCCPortNo[]		= {5, -1, -1, -1};	
		
	while( TRUE )
	{	
		/* 
		 * Sam Initialization
		 */
		sRetVal = SAMInit(anSIMUARTPortNo, anSIMSniperPortNo, anPLCCPortNo);

		nISAMRet = GetSAMPort(ISAM);
		nPSAMRet = GetSAMPort(PSAM);
		if ( (nISAMRet != -1) && (nPSAMRet != -1) )
		{
			break;
		}

		wUnsendCnt++;
		if ( wUnsendCnt > 1 )
		{
			DebugOut( "SAMInitBus Error!\n" );
			if ( nISAMRet == -1 )
			{
				strcpy( abFirstLineMsg, FND_ERR_ISAM );
				strcpy( abSecondLineMsg, FND_ERR_ISAM );
			}
			else if ( nPSAMRet == -1 )
			{
				strcpy( abFirstLineMsg,  FND_ERR_PSAM );
				strcpy( abSecondLineMsg,  FND_ERR_PSAM );
			}

			SystemErrorMsg2EndUser (abFirstLineMsg, abSecondLineMsg );
		}
		sleep( 2 );
	}

	return TRUE;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      MainGetSAMID                                             *
*                                                                              *
*  DESCRIPTION :      SAMID를 SAM에서 획득함                                   *
*                                                                              *
*  INPUT PARAMETERS:  NONE                                                     *
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
static bool MainGetSAMID( void )
{
    short sRetVal = 0;
    FILE* filePSAMIDFlg = NULL;
    byte abBuf[65] = { 0, };	
	byte abCSAMID[20] = { 0, };
	byte abISAMID[20] = { 0, };
	byte abPSAMID[20] = { 0, };
	dword dwISAMChipType = 0;
	
	SAM_COMM_INFO stSAMCOMMINFO;

	strncpy( gpstSharedInfo->abNowStationName, FIRST_STATION_NAME,
			 sizeof(FIRST_STATION_NAME) );

	// Get SAM ID & PSAM Port
	memset( abCSAMID, 0x00, sizeof(abCSAMID) );
	memset( abISAMID, 0x00, sizeof(abISAMID) );
	memset( abPSAMID, 0x00, sizeof(abPSAMID) );

	CSAMGetSAMID( abCSAMID );
	ISAMGetSAMID( abISAMID );
	while( TRUE )
	{
		memset( abPSAMID, 0x00, sizeof(abPSAMID) );
		sRetVal = PSAMGetSAMID( abPSAMID );
		if ( (sRetVal == SUCCESS) && (strlen(abPSAMID) > 0) )
		{
			break;
		}

		printf( "[MainGetSAMID] PSAMGetSAMID() 실패\n" );
		DisplayASCInUpFND( FND_ERR_GET_PSAM_ID );
		DisplayASCInDownFND( FND_ERR_GET_PSAM_ID );
		Buzzer( 5, 50000 );
		VoiceOut( VOICE_CHECK_TERM );		// 단말기를 점검 하시기 바랍니다.
		sleep( 2 );
	}

	// SAM Port/Communication-Type
	memset( &stSAMCOMMINFO, 0x00, sizeof(SAM_COMM_INFO) );
	GetSAMInfo( PSAM, &stSAMCOMMINFO );
	if ( stSAMCOMMINFO.nCommType == SAM_COMM_SIM_SNIPER )
	{
		stSAMCOMMINFO.nPort += 2;
	}

	if ( gboolIsMainTerm == TRUE )
	{
		// PSAM ID READING From simid.flg FIle
	    sRetVal = access( PSAMID_FILE, F_OK );
	    if  ( sRetVal  == SUCCESS )
		{
			filePSAMIDFlg = fopen( PSAMID_FILE, "rb+");
	        if  ( filePSAMIDFlg != NULL )
	        {
	    		memset( abBuf, 0x00, sizeof(abBuf) );
	        	fread ( abBuf, sizeof(abBuf), 1, filePSAMIDFlg );
			    memcpy( gpstSharedInfo->abMainPSAMID,  
						abBuf + PSAMID_SIZE*0, PSAMID_SIZE );
			    memcpy( gpstSharedInfo->abSubPSAMID[0], 
						abBuf + PSAMID_SIZE*1, PSAMID_SIZE );
			    memcpy( gpstSharedInfo->abSubPSAMID[1], 
						abBuf + PSAMID_SIZE*2, PSAMID_SIZE );
			    memcpy( gpstSharedInfo->abSubPSAMID[2], 
						abBuf + PSAMID_SIZE*3, PSAMID_SIZE );
				fclose( filePSAMIDFlg );
	        }
	    }

        memcpy( gpstSharedInfo->abMainCSAMID, abCSAMID,
        	sizeof(gpstSharedInfo->abMainCSAMID) );
        memcpy( gpstSharedInfo->abMainISAMID, abISAMID,
        	sizeof(gpstSharedInfo->abMainISAMID) );
		memcpy( gpstSharedInfo->abMainPSAMID, abPSAMID,
			sizeof(gpstSharedInfo->abMainPSAMID) );
		gpstSharedInfo->bMainPSAMPort = stSAMCOMMINFO.nPort + '0';
		gpstSharedInfo->bMainPSAMVer = IsTPSAM();
	}
	else
	{
        memcpy( gpstSharedInfo->abSubCSAMID[gbSubTermNo - 1], abCSAMID,
        	sizeof(gpstSharedInfo->abSubCSAMID) );
        memcpy( gpstSharedInfo->abSubISAMID[gbSubTermNo - 1], abISAMID,
        	sizeof(gpstSharedInfo->abSubISAMID) );
        memcpy( gpstSharedInfo->abSubPSAMID[gbSubTermNo - 1], abPSAMID,
        	sizeof(gpstSharedInfo->abSubPSAMID) );
		gpstSharedInfo->abSubPSAMPort[gbSubTermNo - 1]
			= stSAMCOMMINFO.nPort + '0';
		gpstSharedInfo->abSubPSAMVer[gbSubTermNo - 1] = IsTPSAM();
	}

    memcpy(gstMyTermInfo.abCSAMID, abCSAMID, sizeof(gstMyTermInfo.abCSAMID));
    memcpy(gstMyTermInfo.abISAMID, abISAMID, sizeof(gstMyTermInfo.abISAMID));
	memcpy(gstMyTermInfo.abPSAMID, abPSAMID, sizeof(gstMyTermInfo.abPSAMID));
	gstMyTermInfo.bPSAMPort = stSAMCOMMINFO.nPort + '0';

	// ISAM TYPE 표시
	dwISAMChipType = 3232;
	if ( IsISAM6464Type() == TRUE )
	{
		dwISAMChipType = 6464;
	}

	memset( abBuf, 0x00, sizeof(abBuf) );
	memcpy( abBuf, gstMyTermInfo.abPSAMID + 10, 6 );
	DisplayDWORDInUpFND( dwISAMChipType );
	DisplayASCInDownFND( abBuf );

	return TRUE;
}	

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      CreateProcSharedDatas                                    *
*                                                                              *
*  DESCRIPTION:       Creation of Shared Memory                                *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: None                                                     *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
void CreateSharedMemNMsgQueue( void )
{
	int nRetVal = 0;
	int nIndex = 0;
	
	void *pvSharedMemory = NULL;

	while ( TRUE )
	{
		gnSharedMemID = shmget( SHARED_MEMORY_KEY, sizeof(PROC_SHARED_INFO), 
			0666|IPC_CREAT );
		if ( gnSharedMemID != -1 )
		{
			gnSharedMemID = 0;
			break;
		}
		else
		{
			perror( "polling shmget failed : " );
			DebugOut( "\nPoll_MemShare\n" );
			SystemErrorMsg2EndUser ( FND_ERR_SYSTEM_MEMORY, 
				FND_ERR_SYSTEM_MEMORY );
		}
	}

	while ( TRUE )
	{
		pvSharedMemory = shmat( gnSharedMemID, (void *)0, 0 );
		if ( pvSharedMemory != (void *)-1 )
		{
			break;
		}
		else
		{
			perror( "polling shmget failed : " );
			DebugOut( "\nPoll_MemShare\n " );
			SystemErrorMsg2EndUser ( FND_ERR_SYSTEM_MEMORY, 
				FND_ERR_SYSTEM_MEMORY );
		}
	}

	gpstSharedInfo = (PROC_SHARED_INFO *) pvSharedMemory;
	memset( gpstSharedInfo, 0x00, sizeof(PROC_SHARED_INFO) );

	/*
	 * Create Message Queue for Printing.
	*/ 
	while ( TRUE )
	{
		gnMsgQueueID = msgget( MESSAGE_QUEUE_PRINTER_KEY, IPC_CREAT | IPC_PERM );
		if ( gnMsgQueueID >= 0 )
		{
			break;
		}
		else
		{
			perror( "Message Queue failed : " );
			DebugOut( "Msgget gnMsgQueue error \n" );
			SystemErrorMsg2EndUser( FND_ERR_SYSTEM_MEMORY, FND_ERR_SYSTEM_MEMORY );
		}
	}

	while ( TRUE )
	{
		for ( nIndex = SEMA_KEY_TRANS; nIndex <= SEMA_KEY_SHARED_CMD; nIndex++ )
		{
			nRetVal = SemaCreate( nIndex );
			if( nRetVal < 0 )
			{
				break;	// Break For Loop
			}
		}

		if ( nRetVal >= 0 )
		{
			break;	// Break While Loop
		}
		else
		{
			perror( "SemaCreate failed : \n");
			perror( sys_errlist[errno] );
			DebugOut( "\n=====Poll_MemShare====\n" );
			SystemErrorMsg2EndUser( FND_ERR_SEMA_CREATE, FND_ERR_SEMA_CREATE );
		}
	}
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SystemErrorMsg2EndUser                                   *
*                                                                              *
*  DESCRIPTION :      에러메세지출력                                           *
*                                                                              *
*  INPUT PARAMETERS:  byte* pbFirstLineMsg, byte* pbSecondLineMsg              *
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
static void SystemErrorMsg2EndUser( byte* pbFirstLineMsg, byte* pbSecondLineMsg )
{
	byte abFirstLineMsg[6] = { 0, };
	byte abSecondLineMsg[6] = { 0, };
	
	memcpy( abFirstLineMsg, pbFirstLineMsg, sizeof(abFirstLineMsg) );
	memcpy( abSecondLineMsg, pbSecondLineMsg, sizeof(abSecondLineMsg) );
	
	DisplayASCInUpFND( abFirstLineMsg );
	DisplayASCInDownFND( pbSecondLineMsg );
	
	/*
	 * Buzzer5회출력
	*/
	Buzzer( 5, 50000 );
	
	/*
	 * 음성출력-"단말기를 점검해주시기 바랍니다."
	*/
	VoiceOut( VOICE_CHECK_TERM );	

	sleep( 3 );	
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      DetermineTermType                                        *
*                                                                              *
*  DESCRIPTION:       Div. of MainTerm & SubTerm                               *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: None                                                     *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static void DetermineTermType(void)
{
	short sRetVal = 0;
	
	gboolIsMainTerm = FALSE;
	gbSubTermNo = '0';

	sRetVal = OpenDipSwitch();
	if( sRetVal != SUCCESS )
	{
		DebugOut( "Dipsw open error\n" );
		return;
	}

	gbSubTermNo =  ReadDipSwitch ();			// Get Device #
	CloseDipSwitch ();

	if ( gbSubTermNo == 0x00 )
	{
		gboolIsMainTerm = TRUE;		// Terminal Type : Main
		DebugOut( "====DipSwitchType = MainTerm====\n" );
	}
	else
	{
		gboolIsMainTerm = FALSE;	// Terminal Type : Sub
		DebugOut( "====DipSwitchType = SubTerm====\n" );
		DebugOut( "====gbSubTermNo = %d.====\n", gbSubTermNo );
	}
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      DisplayImgVer                                            *
*                                                                              *
*  DESCRIPTION:       This program displays Image Version					   *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: None	                                                   *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static void DisplayImgVer( void )
{
	char acTemp[6];
	char acDisplayVer[6];

	memset( acDisplayVer, 0x00,6 );
	memcpy( acDisplayVer, MAIN_RELEASE_VER, 4 );

	memset( acTemp,0x00,6);
	memcpy( acTemp, acDisplayVer, 2 );
	acTemp[2] = '-';
	memcpy( acTemp + 3, acDisplayVer + 2, 2 );

	DisplayASCInUpFND(  "      " );
	DisplayASCInDownFND(  acTemp );
	sleep( 1 );

	DisplayASCInUpFND(  "      " );
	DisplayASCInDownFND(  "      " );

	XLEDOff();
}
/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      DisplayKernelRamdiskVer                                  *
*                                                                              *
*  DESCRIPTION:       This program displays Kernel & Ramdisk Version		   *
*                     and saves Common Structure.                              *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: None                                                     *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static void DisplayKernelRamdiskVer( void )
{
	int nIndex = 0;
	FILE *fdTemp = NULL;
	char acBuf[150];

	char acRamdiskVer[3];
	size_t sizeFileBufSize = 0;
	int nReadByte = 0;
	char *pcFileBuf = NULL;

	// Get Kernel Version
	memset( acBuf,0x00,sizeof(acBuf) );
	sprintf( acBuf,"cat %s", KERNEL_VERSION_FILE );

	fdTemp = popen( acBuf, "r" );
	fgets( acBuf, sizeof(acBuf), fdTemp );
	pclose( fdTemp );

	memset( gabKernelVer, 0x00, sizeof(gabKernelVer) );
	for( nIndex = 0; nIndex < sizeof(acBuf); nIndex++)
	{
		if ( acBuf[nIndex] == 0x23 )
		{
			memcpy( gabKernelVer, &acBuf[nIndex+1], 2 );
		}
	}

	// Get Ramdisk Version
	memset( acBuf,0x00,sizeof(acBuf) );
	memset( acRamdiskVer,0x00,sizeof(acRamdiskVer) );
	sizeFileBufSize = 128;
	pcFileBuf = (unsigned char*)malloc( sizeFileBufSize );

	fdTemp = fopen( RAMDISK_VERSION_FILE, "r" );

	while(1)
	{
		memset(pcFileBuf, 0, sizeFileBufSize);
		nReadByte = getline( &pcFileBuf, &sizeFileBufSize, fdTemp );
		if ( nReadByte < 0 )
		{
			break;
		}
		if ( strstr(pcFileBuf, "ramdisk-busin") )
		{
			for( nIndex = 0; nIndex < 50; nIndex++)
			{
				if( pcFileBuf[nIndex] == 0x6E )
				{
					if( pcFileBuf[nIndex+2] == 0x2E )
					{
						memcpy( acRamdiskVer, &pcFileBuf[nIndex+1],1 );
						break;
					}
					if( pcFileBuf[nIndex+3] == 0x2E )
					{
						memcpy( acRamdiskVer, &pcFileBuf[nIndex+2],2 );
						break;
					}
				}
			}
		}
		if ( strstr(pcFileBuf, "ramdisk-busout") )
		{
			for ( nIndex = 0; nIndex < 50; nIndex++ )
			{
				if ( pcFileBuf[nIndex] == 0x74 )
				{
					if( pcFileBuf[nIndex+2] == 0x2E )
					{
						memcpy( acRamdiskVer, &pcFileBuf[nIndex+1],1 );
						break;
					}
					if ( pcFileBuf[nIndex+3] == 0x2E )
					{
						memcpy( acRamdiskVer, &pcFileBuf[nIndex+2], 2 );
						break;
					}
				}
			}
		}

	}

	free( pcFileBuf );
	fclose( fdTemp );

	DisplayDWORDInUpFND( atoi(gabKernelVer) );
	usleep( 100000 );
	DisplayASCInDownFND( acRamdiskVer );
	sleep( 2 );
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      DisplaySubTermTime                                       *
*                                                                              *
*  DESCRIPTION:       SubTerm Time Display.                                    *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: TRUE - Drive Now                                         *
*                     FALSE - Don't Drive Now                                  *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
bool DisplaySubTermTime( void )
{
	static char acBeforeDateTime[15];
	byte abNowDateTime[15];

	memset( abNowDateTime,0x00,sizeof(abNowDateTime) );
	if ( gboolIsDisplaySwitchOn == TRUE )
	{
		usleep( 200000 );
		return TRUE;
	}

	if ( gboolIsMainTerm == FALSE )
	{
		TimeT2ASCDtime( gpstSharedInfo->tTermTime, abNowDateTime );
		if ( memcmp( acBeforeDateTime, abNowDateTime, 14 ) != 0 )
		{
			if ( gboolIsDisplaySwitchOn == TRUE )
			{
				return TRUE;
			}

			DisplayASCInDownFND( abNowDateTime + 8 );
			memcpy( acBeforeDateTime, abNowDateTime, 14 );
		}
		if ( gpstSharedInfo->boolIsDriveNow == TRUE )
		{
			return TRUE;
		}

		return FALSE;
	}

	return TRUE;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      CheckFreeMemory                                          *
*                                                                              *
*  DESCRIPTION:       This program checks Free Memoryh						   *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: None                                                     *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
void CheckFreeMemory( void )
{
	int nIndex = 0;
	long lFreeMemory = 0;
	int fdDiskCheckResult = 0;
	int nRetVal = 0;
	byte abBuf[150] = { 0, };
	byte abFileName[10][100] = {"tmp_fa_pl.dat",
								"tmp_fd_pl.dat",
								"tmp_fi_ai.dat",
								UPDATE_PL_FILE,
								"c_cd_pl.dat",
								UPDATE_AI_FILE,
								CHANGE_BL_FILE,
								TEMP_MASTER_BL_FILE,
								MASTER_PREPAY_PL_FILE,
								MASTER_POSTPAY_PL_FILE };

	/*
	 * memory_check
	*/
    lFreeMemory = MemoryCheck();
	if ( (lFreeMemory <= 5) && (lFreeMemory >= 0) )
	{
		printf( "[CheckFreeMemory] 1차 디스크 여유공간 체크 오류\n" );

		for(nIndex=0; nIndex<8; nIndex++ )
		{
			sprintf( abBuf, "%s/%s", BUS_EXECUTE_DIR, abFileName[nIndex] );			
			unlink ( abBuf );
			printf( "[CheckFreeMemory] 1차 파일 삭제 : [%s]\n", abBuf );
		}
		
		fdDiskCheckResult = open( FLASH_DATA_OVER_ONE, O_RDWR| O_CREAT |O_TRUNC );
		if ( fdDiskCheckResult > 0 )
		{
			close( fdDiskCheckResult );
		}
	}

    lFreeMemory = MemoryCheck();
	if ( (lFreeMemory <= 5) && (lFreeMemory >= 0) )
	{
		printf( "[CheckFreeMemory] 2차 디스크 여유공간 체크 오류\n" );

		for(nIndex=8; nIndex<10; nIndex++ )
		{
			sprintf( abBuf, "%s/%s", BUS_EXECUTE_DIR, abFileName[nIndex] );			
			unlink ( abBuf );
			printf( "[CheckFreeMemory] 2차 파일 삭제 : [%s]\n", abBuf );
		}		

		fdDiskCheckResult = open( FLASH_DATA_OVER_TWO, O_RDWR| O_CREAT |O_TRUNC );
		if ( fdDiskCheckResult > 0 )
		{
			close( fdDiskCheckResult );
		}
	}

	/*
	 * Program Loader에서도 같이 생성하므로 반드시 필요함
	 */
	nRetVal = access( FLASH_DATA_OVER_ONE, F_OK );
	if ( nRetVal == SUCCESS )
	{
	 	LogWrite( ERR_SETUP_DRIVE_MEMORY_SIZE_ONE );
		ctrl_event_info_write( MEMORY_SIZE_ONE_ERROR_EVENT );
		unlink( FLASH_DATA_OVER_ONE );
	}

	nRetVal = access( FLASH_DATA_OVER_TWO, F_OK );
	if ( nRetVal == SUCCESS )
	{
	 	LogWrite( ERR_SETUP_DRIVE_MEMORY_SIZE_TWO );
		ctrl_event_info_write( MEMORY_SIZE_TWO_ERROR_EVENT );
		unlink( FLASH_DATA_OVER_TWO );
	}

}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      WriteVoiceDataToFlash                                    *
*                                                                              *
*  DESCRIPTION:       Write Data in "c_v0.dat" to Flash						   *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: 0 - Success                                              *
*                     (-) - Error                                              *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short ApplyVoiceData2Flash( void )
{
	short sRetVal = 0;
	int fdVoiceFile = 0;
	int sRetValue1 = 0;
	int sRetValue2 = 0;
	int nVoiceFileSize = 0;
	struct stat stVoiceFileStatus;

	sRetValue1 = access( VOICE0_FILE, F_OK);
	sRetValue2 = access( VOICEAPPLY_FLAGFILE, F_OK);

	if ( (sRetValue1 != SUCCESS) || (sRetValue2 != SUCCESS) )
	{
		return SUCCESS;
	}

	fdVoiceFile = open( VOICE0_FILE, O_RDWR );
	if ( fstat( fdVoiceFile, &stVoiceFileStatus ) < 0 )
	{
		DebugOut("Error!!! fstat() failed \n");
		return ErrRet( ERR_FILE_OPEN | GetFileNo(VOICE0_FILE) );
	}
	close( fdVoiceFile );

	nVoiceFileSize = (int)stVoiceFileStatus.st_size - 18;

	fdVoiceFile = open( VOICE0_FILE, O_RDONLY );
	if( fdVoiceFile < 0 )
	{
		DebugOut("====File open error====\n");
		return ErrRet( ERR_FILE_OPEN | GetFileNo(VOICE0_FILE) );
	}

	DisplayASCInUpFND( FND_VOICEFILE_WRITE_TO_FLASH );
	Buzzer( 5, 50000 );

	gpstSharedInfo->bVoiceApplyStatus = 2;

	lseek( fdVoiceFile, 0L, SEEK_SET );

	sRetVal = WriteVoiceFile2Flash( &fdVoiceFile, nVoiceFileSize );
	close( fdVoiceFile );
	if ( sRetVal == SUCCESS )
	{
		sRetVal = unlink( VOICEAPPLY_FLAGFILE );
		if ( sRetVal < 0 )
		{
			DebugOut( "unlink Result = %d \n", sRetVal );
			sleep( 1 );
		}
	}

	gpstSharedInfo->bVoiceApplyStatus = 0;

	DisplayASCInUpFND( "     0" );
	DisplayASCInDownFND( "       " );

	DebugOut( "====<ApplyVoiceData2Flash>Completed.====\n" );

	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      CheckSubTransSendCompl                                   *
*                                                                              *
*  DESCRIPTION:       This program checks Send Completion to MainTerm          *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: 0 - Success                                              *
*                     (-) - Error                                              *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static void CheckSubTransSendCompl( void )
{
	int nRetVal = 0;

	nRetVal = access( SUB_TRANS_SEND_SUCC, F_OK );
	if ( nRetVal == SUCCESS )		// file exist
	{
		system("rm /mnt/mtd8/bus/*.trn");
	}
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      CheckSubTransSendFailCount                               *
*                                                                              *
*  DESCRIPTION:       Check UnSending Trans Data Count                         *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: UnSending Count                                          *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static word CheckSubTransSendFailCount( void )
{
	word wRetVal = 0;

	int fdSubTrans = 0;
	struct stat stFileStatus;
	long lFileSize = 0;
	int nReadByte = 0;
	byte abReadData[SUBTERM_TRANS_RECORD_SIZE + 2];

//	alightconvert();

	fdSubTrans = open( SUB_TERM_TRANS_FILE, O_RDONLY );

	memset( &stFileStatus, 0x00, sizeof(struct stat) );
	if ( fstat( fdSubTrans, &stFileStatus ) == SUCCESS )
	{
		lFileSize = stFileStatus.st_size;
	}

	if	( lFileSize == 0 )
	{
		if ( fdSubTrans > 0 )
		{
			close( fdSubTrans );
		}

		return SUCCESS;
	}

	lseek( fdSubTrans, 0L, SEEK_SET );

	while ( TRUE )
	{
		memset( abReadData, 0x00, sizeof(abReadData) );
		nReadByte = read(fdSubTrans, abReadData, SUBTERM_TRANS_RECORD_SIZE + 1);
		if( nReadByte <= 0 )
		{
			close( fdSubTrans );
			return wRetVal;
		}

		if( abReadData[SUBTERM_TRANS_RECORD_SIZE] == '0' )	// 기전송Record Skip
		{
			wRetVal++;
			continue;
		}
	}

	return wRetVal;
}



/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      DisplayVehicleID                                         *
*                                                                              *
*  DESCRIPTION:       Display Vehicle ID.                                      *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: None                                                     *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
void DisplayVehicleID( void )
{
	char acVehicleID[7] = { 0, };
	dword dwVehicleID = 0;
	char acNowDateTime[15] = { 0, };
	char acNowDate[9] = { 0, };
	time_t tNowTime = 0;

	memset( acVehicleID, 0x00, sizeof(acVehicleID) );
	memcpy( acVehicleID, gstVehicleParm.abVehicleID + 3, 6 );
	dwVehicleID = GetDWORDFromASC( acVehicleID, 6 );

	memset( acNowDateTime, 0x00, sizeof(acNowDateTime) );
	memset( acNowDate, 0x00, sizeof(acNowDate) );
	GetRTCTime( &tNowTime );
	TimeT2ASCDtime( tNowTime, acNowDateTime );
	memcpy( acNowDate, acNowDateTime, 8 );

	/*
	 * 차량번호
	 */
	DisplayDWORDInUpFND( dwVehicleID );

	/*
	 * 현재시간(YYMMDD)
	 */
	DisplayASCInDownFND( acNowDate + 2 );
	usleep(100000);

	XLEDOff();
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      MainTermIDLoad                                           *
*                                                                              *
*  DESCRIPTION:       Load MainTerm ID                                         *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: None                                                     *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static void MainTermIDLoad( void )
{
	short sResult = 0;
	int fdMainTermID = 0;
	bool boolIsReadSucc = FALSE;
	byte abMainTermID[30] = {0, };

	sResult = OpenEEPROM();
	if ( sResult != SUCCESS )
	{
		printf( "[MainTermIDLoad] EEPROM OPEN 실패\n" );
		goto READ_MAIN_TERM_ID_FROM_FILE;
	}

	// EEPROM에서 단말기ID를 읽음
	sResult = ReadEEPROM( abMainTermID );

	CloseEEPROM();

	if ( sResult != SUCCESS	)
	{
		printf( "[MainTermIDLoad] EEPROM으로부터 단말기ID READ 실패\n" );
		goto READ_MAIN_TERM_ID_FROM_FILE;
	}

	PrintlnASC( "[MainTermIDLoad] abMainTermID  : ", abMainTermID, 9 );

	// EEPROM으로부터 읽은 단말기ID가 유효하다면
	if ( memcmp( abMainTermID, "14", 2 ) == 0 &&
		 IsDigitASC( abMainTermID, 9 ) == TRUE )
	{
		printf( "[MainTermIDLoad] EEPROM으로부터 READ한 단말기ID가 유효함\n" );
		// 복사
		memcpy( gpstSharedInfo->abMainTermID, abMainTermID,
			sizeof(gpstSharedInfo->abMainTermID) );
	}
	// EEPROM으로부터 읽은 단말기ID가 유효하지 않다면
	else
	{
		printf( "[MainTermIDLoad] EEPROM으로부터 READ한 단말기ID가 유효하지 " );
		printf( "않음\n" );
		goto READ_MAIN_TERM_ID_FROM_FILE;
	}

	return;

	READ_MAIN_TERM_ID_FROM_FILE:

	printf( "[MainTermIDLoad] 파일로부터 단말기ID를 READ함\n" );

	fdMainTermID = open( TC_LEAP_FILE, O_RDONLY );
	if ( fdMainTermID > 0 )
	{
		sResult = read( fdMainTermID, abMainTermID, 29 );
		close( fdMainTermID );
		if ( sResult > 0 )
		{
			boolIsReadSucc = TRUE;
		}
	}

	if ( boolIsReadSucc == FALSE )
	{
		fdMainTermID = open( TC_LEAP_BACKUP_FILE, O_RDONLY );
		if ( fdMainTermID > 0 )
		{
			sResult = read( fdMainTermID, abMainTermID, 29 );
			close( fdMainTermID );
			if ( sResult > 0 )
			{
				boolIsReadSucc = TRUE;
			}
		}
	}

	if ( boolIsReadSucc == TRUE )
	{
		printf( "[MainTermIDLoad] 파일로부터 단말기ID를 성공적으로 READ함\n" );
		memcpy( gpstSharedInfo->abMainTermID, abMainTermID,
			sizeof(gpstSharedInfo->abMainTermID) );
	}
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SubTermIDCountLoad                                       *
*                                                                              *
*  DESCRIPTION:       Load & Display SubTerm ID                                *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: None                                                     *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static void SubTermIDCountLoad( void )
{
	byte abSubTermIDBuf[SUBTERM_ID_COUNT_SIZE + 1] = { 0, };
	int fdSubTermID = 0;
	struct stat stFileStatus;
	int nFileSize = 0;
	int nReadByte = 0;
	char acReadBuf[30] = { 0, };

	int nIndex = 0;

	memset( abSubTermIDBuf, 0x30, sizeof(abSubTermIDBuf) );
	abSubTermIDBuf[28] = 0x00;

	memset( &stFileStatus, 0x00, sizeof(struct stat) );
	memset( acReadBuf, 0x00, sizeof(acReadBuf) );

	// subid.dat 파일을 OPEN한다.
	fdSubTermID = open( SUBTERM_ID_FILENAME, O_RDONLY );

	// subid.dat 파일이 존재하는 경우 //////////////////////////////////////////
	if ( fdSubTermID > 0 )
	{
		if ( fstat(fdSubTermID, &stFileStatus ) == 0 )
		{
			nFileSize = stFileStatus.st_size;
		}

		nReadByte = read ( fdSubTermID, acReadBuf, SUBTERM_ID_COUNT_SIZE );

		DebugOut ( "=====nReadByte ==> %d [%02x]=====\n", nReadByte,nReadByte );

		if ( nReadByte != SUBTERM_ID_COUNT_SIZE )
		{
			if ( gboolIsMainTerm == FALSE )
			{
				gbSubTermCnt = 3;
			}
			else
			{
				gbSubTermCnt = 1;
			}
			abSubTermIDBuf[0] = (byte)gbSubTermCnt + 0x30;

			if ( nFileSize == 0 )
			{
				memset( abSubTermIDBuf + 1, 0x30, SUBTERM_ID_SIZE );
			}
			else
			{
				memcpy( abSubTermIDBuf + 1, acReadBuf, SUBTERM_ID_SIZE );
			}

			write( fdSubTermID, abSubTermIDBuf, SUBTERM_ID_COUNT_SIZE );
		}
		else
		{
			memcpy( abSubTermIDBuf, acReadBuf, SUBTERM_ID_COUNT_SIZE );
			gbSubTermCnt = (int)( abSubTermIDBuf[0] - 0x30 );
			if ( gboolIsMainTerm == FALSE )
			{
				gbSubTermCnt = 3;
			}
		}

	}
	// subid.dat 파일이 존재하지 않는 경우 /////////////////////////////////////
	else
	{
		fdSubTermID = open( SUBTERM_ID_FILENAME, O_RDONLY | O_CREAT | O_TRUNC );
		if ( gboolIsMainTerm == FALSE ) 
		{
			gbSubTermCnt = 3;
		}
		else
		{
			gbSubTermCnt = 1;
		}
		abSubTermIDBuf[0] = (byte)gbSubTermCnt + 0x30;
		write ( fdSubTermID, abSubTermIDBuf, SUBTERM_ID_COUNT_SIZE );
	}

	close ( fdSubTermID );

	for ( nIndex=0; nIndex<gbSubTermCnt; nIndex++)
	{
		memcpy( gpstSharedInfo->abSubTermID[nIndex],
				&abSubTermIDBuf[nIndex*SUBTERM_ID_SIZE+1], SUBTERM_ID_SIZE );
	}

	if ( gboolIsMainTerm == FALSE )
	{
		memset( abSubTermIDBuf, 0, sizeof(abSubTermIDBuf) );
		if ( OpenEEPROM() == SUCCESS )
		{
			if ( ReadEEPROM( abSubTermIDBuf ) == SUCCESS )
			{
				memcpy( gpstSharedInfo->abSubTermID[gbSubTermNo-1],
						abSubTermIDBuf,
						sizeof(gpstSharedInfo->abSubTermID[gbSubTermNo-1]) );
				PrintlnASC( "[SubTermIDCountLoad] 하차단말기ID : ",
					gpstSharedInfo->abSubTermID[gbSubTermNo-1],
					sizeof( gpstSharedInfo->abSubTermID[gbSubTermNo-1] ) );
			}

			CloseEEPROM();
		}

		DisplayDWORDInUpFND( 0 );
		memset( abSubTermIDBuf, 0x00, sizeof(abSubTermIDBuf) );
		memcpy( abSubTermIDBuf,
				&gpstSharedInfo->abSubTermID[gbSubTermNo-1][3], 6 );
		DisplayASCInDownFND( abSubTermIDBuf );
		sleep( 1 );

       return;
    }

}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      IsDriveNow                                               *
*                                                                              *
*  DESCRIPTION:       This program openss "control.trn" and defines Driving St.*
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: 0 - Is Not Drive Now                                     *
*					  1 - Is Drive Now										   *
*                     (-) - Error                                              *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short IsDriveNow( void )
{
	short sRetValue = 0;
	int fdTranSumFile = 0;
	char achTransFileName[20] = { 0, };
	char achDriveStartTime[15] = { 0, };
	byte abTransFileBackup[100] = { 0, };

	int nReadCnt = 0;
	int nIndex = 0;

	CONTROL_TRANS stTransSum;

	bool boolIsDriveNow = FALSE;

	sRetValue = CheckParmFilesExist();
	if ( sRetValue != SUCCESS )
	{
		return ErrRet( ERR_SETUP_DRIVE_DRIVE_START_STOP );
	}

	if ( gboolIsMainTerm == FALSE )
	{
		return SUCCESS;
	}

	sRetValue = access( CONTROL_TRANS_FILE, F_OK );
	if ( sRetValue == SUCCESS )
	{
		sprintf( abTransFileBackup, "cp %s %s", 
			CONTROL_TRANS_FILE, CONTROL_TRANS_BACKUP_FILE );
		for( nIndex = 0; nIndex < 3; nIndex++ )
		{
			fdTranSumFile = open( CONTROL_TRANS_FILE, O_RDONLY );
			if ( fdTranSumFile > 0 ) break;
		}

		if ( nIndex >= 3 )
		{
			DisplayASCInUpFND( FND_ERR_MSG_WRITE_MAIN_TRANS_DATA );
			Buzzer( 5, 50000 );
			VoiceOut( VOICE_CHECK_TERM );
			ctrl_event_info_write( CONTROLTRN_OPEN_ERROR_EVENT );
			system( abTransFileBackup );
			unlink( CONTROL_TRANS_FILE );
			boolIsDriveNow = FALSE;
		}

		memset( &stTransSum,0,sizeof(CONTROL_TRANS) );
		nReadCnt = read( fdTranSumFile, &stTransSum, sizeof(CONTROL_TRANS) );
		close( fdTranSumFile );

		memset( achTransFileName, 0x00, sizeof(achTransFileName) );
		if( nReadCnt > 0 )
		{
			memcpy( achTransFileName, stTransSum.abTRFileName,
				sizeof(stTransSum.abTRFileName) );
			memcpy( gpstSharedInfo->abTransFileName, stTransSum.abTRFileName,
				sizeof(stTransSum.abTRFileName) );
			gpstSharedInfo->abTransFileName[18] = '\0';
			boolIsDriveNow = TRUE;
		}

		sRetValue = access( achTransFileName, F_OK );
		if ( sRetValue != SUCCESS )
		{
			DisplayASCInUpFND( FND_ERR_MSG_WRITE_MAIN_TRANS_DATA );
			Buzzer( 5, 50000 );
			VoiceOut( VOICE_CHECK_TERM );

			ctrl_event_info_write( CONTROLTRN_NOTFOUND_ERROR_EVENT );
			system( abTransFileBackup );
			unlink( CONTROL_TRANS_FILE );
			memset( gpstSharedInfo->abTransFileName, 0x00,
				sizeof(gpstSharedInfo->abTransFileName) );
			boolIsDriveNow = FALSE;
			sleep( 2 );
		}

		/*
		 * 운행시작시간 전역변수 설정 - GPS로그에 사용하기 위함
		 */
		if( boolIsDriveNow == TRUE )
		{
			memset( achDriveStartTime, 0x00, sizeof(achDriveStartTime) );
			memcpy( achDriveStartTime, achTransFileName, 14 );
			gtDriveStartDtime = GetTimeTFromASCDtime(achDriveStartTime);

			CreateGPSThread();

			InitIP( NETWORK_DEVICE );

			gpstSharedInfo->boolIsDriveNow = TRUE;
			
		}
	}

	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      CheckParmFilesExist                                      *
*                                                                              *
*  DESCRIPTION:       This program checks Files needed for Card Proc.          *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: 0 - Success                                              *
*                     (-) - Error                                              *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short CheckParmFilesExist( void )
{
	short sRetVal = SUCCESS;
	byte  bIndex = 0;
	static byte  abCheckFile[8][50] = {	VEHICLE_PARM_FILE,
										PREPAY_ISSUER_INFO_FILE,
										ROUTE_PARM_FILE,
										BUS_STATION_INFO_FILE,
										DIS_EXTRA_INFO_FILE,
										HOLIDAY_INFO_FILE,
										XFER_APPLY_INFO_FILE,
										NEW_FARE_INFO_FILE
		};

	/*
	 * 승차기에서 존재해야하는 기본정보파일
	 */
	if ( gboolIsMainTerm == TRUE )
	{
		return CheckMainTermBasicInfoFile();
	}

	/*
	 * 하차기에서 존재해야하는 기본정보파일
	 */
	for( bIndex=0; bIndex<8; bIndex++ )
	{
		sRetVal = access(abCheckFile[bIndex], F_OK);
		if( sRetVal != SUCCESS ) break; 
	}
	
	if ( sRetVal != SUCCESS )
	{
		printf( "[CheckParmFilesExist] 하차단말기 필수 파일이 존재하지 않음\n" );
		DisplayASCInUpFND( FND_ERR_CRITERIA_INFOFILE_NO_EXIST );
		Buzzer( 5, 50000 );
		
		/*
		 * 단말기를 점검 하시기 바랍니다.
		 */
		VoiceOut( VOICE_CHECK_TERM );
	}

	return sRetVal;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      CheckMainTermBasicInfoFile                               *
*                                                                              *
*  DESCRIPTION:       승차단말기에서 기본으로 존재해야하는 파일 CHECK          *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: 0 - Success                                              *
*                     (-) - Error                                              *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short CheckMainTermBasicInfoFile( void )
{
	short sRetVal = SUCCESS;
	
	sRetVal = access( VEHICLE_PARM_FILE, F_OK );	
	if ( sRetVal == SUCCESS )
	{
		sRetVal = access( SETUP_FILE, F_OK );
		if ( sRetVal != SUCCESS )
		{
			sRetVal = access( SETUP_BACKUP_FILE, F_OK );
		}
	}

	if ( sRetVal == SUCCESS )
	{
		sRetVal = access( TC_LEAP_FILE, F_OK );
		if ( sRetVal != SUCCESS )
		{
			sRetVal = access( TC_LEAP_BACKUP_FILE, F_OK );
		}
	}
	
	return sRetVal;
}

bool IsExistMainTermBasicInfoFile( void )
{
	byte i = 0;
	static byte abBasicInfoFile[4][64] = {
		VEHICLE_PARM_FILE,				// 차량정보파일
		ROUTE_PARM_FILE,				// 노선정보파일
		NEW_FARE_INFO_FILE,				// 요금정보파일
		BUS_STATION_INFO_FILE };		// 정류장정보파일

	// 설치정보 존재유무 확인
	if ( IsExistFile( SETUP_FILE) == FALSE &&
		 IsExistFile( SETUP_BACKUP_FILE) == FALSE )
	{
		return FALSE;
	}

	// 설치정보 존재유무 확인
	if ( IsExistFile( TC_LEAP_FILE) == FALSE &&
		 IsExistFile( TC_LEAP_BACKUP_FILE) == FALSE )
	{
		return FALSE;
	}

	for ( i = 0; i < 4; i++ )
	{
		if ( IsExistFile( abBasicInfoFile[i] ) == FALSE )
		{
			return FALSE;
		}
	}

	return TRUE;
}

bool IsExistSubTermBasicInfoFile( void )
{
	byte i = 0;
	static byte abBasicInfoFile[8][64] = {
		VEHICLE_PARM_FILE,				// 차량정보파일
		ROUTE_PARM_FILE,				// 노선정보파일
		NEW_FARE_INFO_FILE,				// 요금정보파일
		BUS_STATION_INFO_FILE,			// 정류장정보파일
		PREPAY_ISSUER_INFO_FILE,		// 선불발행사정보파일
		DIS_EXTRA_INFO_FILE,			// 할인할증정보파일
		HOLIDAY_INFO_FILE,				// 휴일정보파일
		XFER_APPLY_INFO_FILE };			// 환승적용정보파일

	for ( i = 0; i < 8; i++ )
	{
		if ( IsExistFile( abBasicInfoFile[i] ) == FALSE )
		{
			return FALSE;
		}
	}

	return TRUE;
}



