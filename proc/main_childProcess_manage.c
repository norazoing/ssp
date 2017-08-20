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
*  PROGRAM ID :       main_childProcess_manage.c                               *
*                                                                              *
*  DESCRIPTION:       MAIN외에 CHILD PROCESS를 CHECK한다.                      *
*                                                                              *
*  ENTRY POINT:     void CreateChildProcess(void);                             *
*                   void CheckChildProcess(void);                              *
*                   void KillChildProcess( void );                             *
*                   void KillGPSThread( void );								   *
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
#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>

/*******************************************************************************
*  Inclusion of User Header Files                                              *
*******************************************************************************/
#include "../system/device_interface.h"
#include "../system/gps.h"
#include "main_childProcess_manage.h"
#include "printer_proc.h"
#include "keypad_proc.h"
#include "main.h"
#include "../comm/term_comm.h"

/*******************************************************************************
*  Declaration of Module Functions                                             *
*******************************************************************************/
static void CreateCommPrinterProc( void );
static void CreateCommKpdProc( void );
static void CreateTermCommProc( void );
static void ForkProcess( pid_t *pidProcessID );

/*******************************************************************************
*  Declaration of Module Variables                                             *
*******************************************************************************/
extern int gpsloopcheck;

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      CreateChildProcess                                       *
*                                                                              *
*  DESCRIPTION :      GPS/ PRINTER/ KEYPAD ChildProcess를 생성함               *
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
void CreateChildProcess(void)
{	
	if ( gboolIsMainTerm == TRUE )	
	{	    
		/*
	     *  GPS프로세스생성
	     */
		CheckGPSThread( );

	    /*
	     *  일회용영수증출력 프로세스 생성
	     */
		CreateCommPrinterProc();
		
	    /*
	     *  운전자조작기 통신 프로세스 생성
	     */
		CreateCommKpdProc();
	}
	
    /*
     *  승하차 통신프로세스 생성
     */
	CreateTermCommProc();
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      CheckChildProcess                                        *
*                                                                              *
*  DESCRIPTION :      Child Process를 Check하고, 미존재시 재성성함             *
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
void CheckChildProcess(void)
{
	static time_t tTime = 0;

	if ( tTime == 0 )
	{
		TimerStart( &tTime );
	}

	if ( TimerCheck( 10, tTime ) == 0 )
	{
		TimerStart( &tTime );
		if ( gboolIsMainTerm == TRUE )	
		{
			if ( gpstSharedInfo->boolIsDriveNow == TRUE &&
				 nCommGPSThreadID != 0 &&
				 gpsloopcheck == 3 )
			{
				printf( "[CheckChildProcess] GPS쓰레드가 HANG이 걸린것으로 판단\n" );
				ctrl_event_info_write( EVENT_GPS_THREAD_REFRESH );
				KillGPSThread();
				gnGPSStatus = 44;
			}
			CheckGPSThread();
			CreateCommPrinterProc();
			CreateCommKpdProc();

			gpsloopcheck = 3;
		}

		CreateTermCommProc();
	}
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      KillChildProcess                                         *
*                                                                              *
*  DESCRIPTION :      Child Process를 Kill함                                   *
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
void KillChildProcess( void )
{
	int nIndex = 0;
	byte abProcessPath[60] = { 0, };
	pid_t nProcssID[3] = { 0, };
	
	nProcssID[0] = gpstSharedInfo->nCommProcProcessID;
	nProcssID[1] = gpstSharedInfo->nCommDriverProcessID;
	nProcssID[2] = gpstSharedInfo->nCommPrinterProcessID;
	
	// sigaction. signal. killpg. kill
	KillGPSThread();				

	for ( nIndex=0; nIndex<3; nIndex++ )
	{
		memset ( abProcessPath, 0x00, sizeof(abProcessPath) );
 		sprintf( abProcessPath, "/proc/%d/exe", nProcssID[nIndex] );
		
		if ( SUCCESS == access(abProcessPath, F_OK) )
			kill( nProcssID[nIndex], SIGINT );
	}
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      KillGPSThread                                            *
*                                                                              *
*  DESCRIPTION :      GPS THREAD를 KILL                                        *
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
void KillGPSThread( void )
{
	if ( gboolIsMainTerm == TRUE && nCommGPSThreadID != 0 )
	{
		printf( "[KillGPSThread] GPS쓰레드 종료\n" );
		gboolIsRunningGPSThread = FALSE;
		pthread_cancel( nCommGPSThreadID );
		pthread_join( nCommGPSThreadID, NULL );
	}
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      CheckGPSThread                                           *
*                                                                              *
*  DESCRIPTION :      GPS 통신프로세스를 생성후 통신함수를 호출                *
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
void CheckGPSThread( void )
{	
	short sRetValue = 0;
	byte abProcessPath[60] = { 0, };
	
	memset ( abProcessPath, 0x00, sizeof(abProcessPath) );
	sprintf( abProcessPath, "/proc/%d/exe",
				gpstSharedInfo->nCommGPSProcessID );
	sRetValue = access( abProcessPath, F_OK );
	if ( (sRetValue != SUCCESS) &&
		 (gpstSharedInfo->boolIsReadyToDrive == TRUE) &&
		 (gpstSharedInfo->boolIsDriveNow == TRUE) )
	{
		printf("[CheckGPSThread] GPS쓰레드가 존재하지 않아 다시 생성함\n");
		CreateGPSThread();
	}
}

void CreateGPSThread( void )
{
	printf( "[CreateGPSThread] GPS쓰레드 생성\n" );
	gpsloopcheck = 4;
	gboolIsRunningGPSThread = TRUE;
	nCommGPSThreadID = 0;
	pthread_create( &nCommGPSThreadID, NULL, gpsMain, NULL );
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      CreateCommPrinterProc                                    *
*                                                                              *
*  DESCRIPTION :      PRINTER 통신프로세스를 생성후 통신함수를 호출            *
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
static void CreateCommPrinterProc( void )
{
	short sRetValue = 0;
	pid_t pidProcessID = 0;
	byte abProcessPath[60] = { 0, };
	
	memset ( abProcessPath, 0x00, sizeof(abProcessPath) );
	sprintf( abProcessPath, "/proc/%d/exe",
	gpstSharedInfo->nCommPrinterProcessID );
	sRetValue = access( abProcessPath, F_OK );
	if ( sRetValue != SUCCESS )
	{
		pidProcessID = 0;
		ForkProcess( &pidProcessID );
		if ( pidProcessID == 0 )
		{
			gpstSharedInfo->nCommPrinterProcessID = getpid();
			CommPrinter();

			exit( 1 );
		}
	}	
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      CreateCommKpdProc                                        *
*                                                                              *
*  DESCRIPTION :      KEYPAD 통신프로세스를 생성후 통신함수를 호출             *
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
static void CreateCommKpdProc( void )
{
	short sRetValue = 0;
	pid_t pidProcessID = 0;
	byte abProcessPath[60] = { 0, };
	
	memset ( abProcessPath, 0x00, sizeof(abProcessPath) );
	sprintf( abProcessPath, "/proc/%d/exe",
	gpstSharedInfo->nCommDriverProcessID );
	sRetValue = access( abProcessPath, F_OK );
	if ( sRetValue != SUCCESS )
	{
		pidProcessID = 0;
		ForkProcess( &pidProcessID );
		if ( pidProcessID == 0 )
		{
			gpstSharedInfo->nCommDriverProcessID = getpid();
			CommKpd();
			exit( 1 );
		}
	}
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      CreateTermCommProc                                       *
*                                                                              *
*  DESCRIPTION :      승하차 통신프로세스를 생성후 통신함수를 호출             *
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
static void CreateTermCommProc( void )
{
	short sRetValue = 0;
	pid_t pidProcessID = 0;
	byte abProcessPath[60] = { 0, };
			
	// Check CommProcess Between Terms
	memset ( abProcessPath, 0x00, sizeof(abProcessPath) );
	sprintf( abProcessPath, "/proc/%d/exe",
	gpstSharedInfo->nCommProcProcessID );
	sRetValue = access( abProcessPath, F_OK );
	if ( sRetValue != SUCCESS )
	{
		pidProcessID = 0;
		ForkProcess( &pidProcessID );
		if ( pidProcessID == 0 )
		{
			gpstSharedInfo->nCommProcProcessID = getpid();
			CommProc();

			exit( 1 );
		}
	}
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      ForkProcess                                              *
*                                                                              *
*  DESCRIPTION :      신규 프로세스를 Fork함                                   *
*                                                                              *
*  INPUT PARAMETERS:  pid_t *pidProcessID                                      *
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
static void ForkProcess( pid_t *pidProcessID )
{
	while( 1 )
	{
        if ( (*pidProcessID = fork() ) < 0 )
        {
            printf( "[ForkProcess] fork 실패\n" );
            DisplayASCInUpFND( FND_ERR_SYSTEM_MEMORY );
            DisplayASCInDownFND( FND_ERR_SYSTEM_MEMORY );
            Buzzer( 5, 50000 );
            VoiceOut( VOICE_CHECK_TERM );          // Term Check Msg

            sleep( 3 );
        }
        else
        {
        	break;
        }
	}	// End of While
}
