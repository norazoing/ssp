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
*  PROGRAM ID :     keypad_proc_mainComm.c                                     *
*                                                                              *
*  DESCRIPTION:     This program comm. with Driver & Main Proc./ Comm Proc.    *
*                                                                              *
*  ENTRY POINT:     short GetImgFileApplyDatenVer( byte* pbFileVer,            *
*						byte* pbFileApplyDate )                                *
*					short SetTermIDLeapPwd( byte* pbRecvBuf, word wRecvLen )   *
*					short ResetupReq( byte* pbRecvBuf, word wRecvLen )         *
*					short KpdSetTime( byte* pbRecvBuf, word wRecvLen )         *
*					short KpdStartStopReq( byte* pbRecvBuf, word wRecvLen )    *
*					short KpdExtra( byte* pbRecvBuf, word wRecvLen )           *
*					short CancelMultiEnt( byte* pbRecvBuf, word wRecvLen )     *
*					short KpdBLCheck( byte* pbRecvBuf, word wRecvLen )         *
*					short KpdPLCheck( byte* pbRecvBuf, word wRecvLen )         *
*					short MultiEnt( byte* pbRecvBuf, word wRecvLen )           *
*					short StationCorrectReq( byte* pbRecvBuf, word wRecvLen )  *
*					short CashEntReceiptPrintReq( byte* pbRecvBuf,             *
*                   	word wRecvLen )                                        *
*					short SubTermIDSetReq( byte* pbRecvBuf, word wRecvLen )    *
*					short ServerIPSetting( byte* pbRecvBuf, word wRecvLen )    *
*					short TermInfoPrintingReq2Main( void )                     *
*					short CtrlKpdSharedCmd( byte bCmd, char* pcCmdData,        *
*                   	word wCmdDataSize, word wTimeOutSec )                  *
*					void CheckLancardSignal( void )                            *
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
* ---------- ---------------------------- ------------------------------------ *
* 2005/08/13 Solution Team  Gwan Yul Kim  Initial Release                      *
*                                                                              *
*******************************************************************************/

/*******************************************************************************
*  Inclusion of Header Files                                                   *
*******************************************************************************/
#define _GNU_SOURCE
#include "../system/bus_type.h"
#include "keypad_proc_mainComm.h"

extern KPDCOMM_POOL_DATA gstKpdPool;

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      GetImgFileApplyDatenVer                                  *
*                                                                              *
*  DESCRIPTION:       This program reads ApplyDate & Image version in Img File *
*                                                                              *
*  INPUT PARAMETERS:  byte* pbFileVer                                          *
*                     byte* pbFileApplyDate                                    *
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
short GetImgFileApplyDatenVer( byte* pbFileVer, byte* pbFileApplyDate )
{
	short sRetVal = 0;
	int fdKpdImg = 0;
	int nSeekPos = 0;

	byte abFileVerLocal[5];
	byte abFileApplyDateLocal[15];

	sRetVal = access( KPD_IMAGE_FILE, F_OK );
	if ( sRetVal == SUCCESS )
	{
		fdKpdImg = open( KPD_IMAGE_FILE, O_RDONLY );
		if ( fdKpdImg < 0 )
		{
			return ErrRet( ERR_KEYPAD_IMAGEFILE_OPEN );
		}

		memset( abFileVerLocal, 0x00, sizeof(abFileVerLocal) );
		memset( abFileApplyDateLocal, 0x00, sizeof(abFileApplyDateLocal) );

		nSeekPos = lseek( fdKpdImg, 
						  (-1)*(FILE_DATE_SIZE + FILE_VERSION_SIZE), 
						  SEEK_END );
		read( fdKpdImg, abFileApplyDateLocal, FILE_DATE_SIZE );
		nSeekPos= lseek( fdKpdImg, (-1)*FILE_VERSION_SIZE, SEEK_END );
		read( fdKpdImg, abFileVerLocal, FILE_VERSION_SIZE );
		close( fdKpdImg );

		memcpy( pbFileVer, abFileVerLocal, FILE_VERSION_SIZE );
		memcpy( pbFileApplyDate, abFileApplyDateLocal,	FILE_DATE_SIZE );
	}

	return sRetVal;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SetTermIDLeapPwd                                         *
*                                                                              *
*  DESCRIPTION:       Set TerminalID & Leap Password & Request reload to Comm  *
*                                                                              *
*  INPUT PARAMETERS:  byte* pbRecvBuf                                          *
*                     word wRecvLen                                            *
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
short SetTermIDLeapPwd( byte* pbRecvBuf, word wRecvLen )
{
	short sResult = SUCCESS;
	int fdTCLeapFile = 0;
	byte abWriteTermID[20] = { 0, };
	byte abRecvTermID[20] = { 0, };
	byte abSendBuf[41] = { 0, };

	PrintlnASC( "[SetTermIDLeapPwd] 단말기ID  : ", pbRecvBuf + 1, 9 );
	PrintlnASC( "[SetTermIDLeapPwd] LEAP PSWD : ", pbRecvBuf + 10, 20 );

	fdTCLeapFile = open( TC_LEAP_FILE, O_WRONLY | O_CREAT | O_TRUNC, OPENMODE );
	if ( fdTCLeapFile < 0 )
	{
		printf( "[SetTermIDLeapPwd] tc_leap.dat 파일 초기화 OPEN 실패\n" );
		return ErrRet( ERR_KEYPAD_TC_LEAP_FILE_OPEN );
	}

	memset( abWriteTermID, 0x00, sizeof(abWriteTermID) );
	memset( abRecvTermID, 0x00, sizeof(abRecvTermID) );

	write( fdTCLeapFile, pbRecvBuf + 1, wRecvLen - 1 );
	close( fdTCLeapFile );

	memcpy( abWriteTermID, pbRecvBuf + 1, 9 );

	if ( CheckWriteEEPROM( abWriteTermID, abRecvTermID ) == SUCCESS )
	{
		Buzzer( 2, 1000000 );		// 1초 간격으로 2회 부저 출력

		DisplayDWORDInUpFND( GetDWORDFromASC(abRecvTermID + 3, 6) );
		sleep ( 2 );

		DisplayDWORDInUpFND( 0 );
	}

	memset( abSendBuf, 0x00, sizeof(abSendBuf) );
	abSendBuf[0] = CMD_REQUEST;
	abSendBuf[1] = CMD_REQUEST;
	abSendBuf[2] = CMD_SUCCESS_RES;

	sResult = CtrlKpdSharedCmd( CMD_PARMS_RESET, abSendBuf, 3, -1 );
	if ( sResult == SUCCESS )
	{
		printf( "[SetTermIDLeapPwd] 처리 성공\n" );
	}
	else
	{
		printf( "[SetTermIDLeapPwd] 처리 실패\n" );
	}

	return sResult;
}

short SetTermIDLeapPwd0118( byte *pbRecvBuf, word wRecvLen )
{
	short sResult = SUCCESS;
	int fdTCLeapFile = 0;
	byte abReadMainTermID[9] = {0, };
	byte abWriteMainTermID[9] = {0, };
	byte abLEAPPSWD[32] = {0, };
	byte abSendBuf[41] = {0, };

	memcpy( abWriteMainTermID, pbRecvBuf + 1, sizeof( abWriteMainTermID ) );
	memcpy( abLEAPPSWD, pbRecvBuf + 1 + sizeof( abWriteMainTermID ),
		wRecvLen - 1 - sizeof( abWriteMainTermID ) );

	printf		( "[SetTermIDLeapPwd0118] wRecvLen          : %d\n", wRecvLen );
	PrintlnASC	( "[SetTermIDLeapPwd0118] abWriteMainTermID : ", abWriteMainTermID, sizeof( abWriteMainTermID ) );
	PrintlnASC	( "[SetTermIDLeapPwd0118] abLEAPPSWD        : ", abLEAPPSWD, wRecvLen - 1 - sizeof( abWriteMainTermID ) );

	sResult = OpenEEPROM();
	if ( sResult != SUCCESS )
	{
		printf( "[SetTermIDLeapPwd0118] EEPROM OPEN 실패\n" );
		return ErrRet( ERR_DEVICE_EEPROM_OPEN );
	}

	// EEPROM에서 단말기ID를 읽음
	sResult = ReadEEPROM( abReadMainTermID );
	if ( sResult != SUCCESS	)
	{
		printf( "[SetTermIDLeapPwd0118] EEPROM으로부터 단말기ID READ 실패\n" );
		CloseEEPROM();
		return -1;
	}

	PrintlnASC	( "[SetTermIDLeapPwd0118] abReadMainTermID  : ", abReadMainTermID, sizeof( abReadMainTermID ) );

	// EEPROM으로부터 읽은 단말기ID가 유효하다면
	if ( memcmp( abReadMainTermID, "14", 2 ) == 0 &&
		 IsDigitASC( abReadMainTermID, sizeof( abReadMainTermID ) ) )
	{
		// TC_LEAP_FILE에 WRITE하기 위해 복사
		memcpy( abWriteMainTermID, abReadMainTermID, sizeof( abWriteMainTermID ) );
	}
	// EEPROM으로부터 읽은 단말기ID가 유효하지 않다면
	else
	{
		// 운전자조작기를 통해 입력된 단말기ID를 EEPROM에 WRITE
		WriteEEPROM( abWriteMainTermID );
	}

	CloseEEPROM();

	fdTCLeapFile = open( TC_LEAP_FILE, O_WRONLY | O_CREAT | O_TRUNC, OPENMODE );
	if ( fdTCLeapFile < 0 )
	{
		printf( "[SetTermIDLeapPwd0118] TC_LEAP_FILE open() failed\n" );
		return ErrRet( ERR_KEYPAD_TC_LEAP_FILE_OPEN );
	}

	PrintlnASC( "[SetTermIDLeapPwd0118] abWriteMainTermID : ",
		abWriteMainTermID, sizeof( abWriteMainTermID ) );

	write( fdTCLeapFile, abWriteMainTermID, sizeof( abWriteMainTermID ) );
	write( fdTCLeapFile, abLEAPPSWD, wRecvLen - 1 - sizeof( abWriteMainTermID ) );
	close( fdTCLeapFile );

	Buzzer( 2, 1000000 );					// 1초 간격으로 2회 부저 출력
	DisplayDWORDInUpFND( GetDWORDFromASC( abWriteMainTermID + 3, 6 ) );
	sleep ( 2 );
	DisplayDWORDInUpFND( 0 );

	memset( abSendBuf, 0x00, sizeof(abSendBuf) );
	abSendBuf[0] = CMD_REQUEST;
	abSendBuf[1] = CMD_REQUEST;
	abSendBuf[2] = CMD_SUCCESS_RES;

	sResult = CtrlKpdSharedCmd( CMD_PARMS_RESET, abSendBuf, 3, -1 );
	if ( sResult == SUCCESS )
	{
		printf( "[SetTermIDLeapPwd0118] 처리 성공\n" );
	}
	else
	{
		printf( "[SetTermIDLeapPwd0118] 처리 실패\n" );
	}

	return sResult;
}

short SetMainTermID( byte *abRecvBuf, word wRecvLen )
{
	short sResult = SUCCESS;
	int nResult = 0;
	byte abBuf[32] = {0, };
	byte abReadMainTermID[9] = {0, };
	byte abWriteMainTermID[9] = {0, };
	byte abSendBuf[41] = { 0, };
	FILE *fdFile = NULL;

	memcpy( abWriteMainTermID, abRecvBuf + 1, sizeof( abWriteMainTermID ) );

	PrintlnASC( "[SetMainTermID] abWriteMainTermID : ", abWriteMainTermID, sizeof( abWriteMainTermID ) );

	// EEPROM에 승차단말기ID를 WRITE
	sResult = CheckWriteEEPROM( abWriteMainTermID, abReadMainTermID );
	if ( sResult != SUCCESS )
	{
		printf( "[SetMainTermID] CheckWriteEEPROM() 실패\n" );
		return -1;
	}

	// TC_LEAP_FILE 파일의 승차단말기ID 부분을 REPLACE
	if ( IsExistFile( TC_LEAP_FILE ) )
	{
		fdFile = fopen( TC_LEAP_FILE, "rb+" );
		if ( fdFile == NULL )
		{
			sResult = -1;
			goto FINALLY;
		}

		nResult = fread( abBuf, 29, 1, fdFile );
		if ( nResult != 1 )
		{
			sResult = -1;
			goto FINALLY;
		}

		memcpy( abBuf, abWriteMainTermID, sizeof( abWriteMainTermID ) );

		fseek( fdFile, 0, SEEK_SET );

		nResult = fwrite( abBuf, 29, 1, fdFile );
		if ( nResult != 1 )
		{
			sResult = -1;
			goto FINALLY;
		}
		
		FINALLY:

		if ( fdFile != NULL )
		{
			fflush( fdFile );
			fclose( fdFile );
		}

		if ( sResult != SUCCESS )
		{
			return sResult;
		}
	}

	Buzzer( 2, 1000000 );			// 1초 간격으로 2회 부저 출력				

	DisplayDWORDInUpFND( GetDWORDFromASC( abWriteMainTermID + 3, 6 ) );
	sleep ( 2 );
	DisplayDWORDInUpFND( 0 );

	memset( abSendBuf, 0x00, sizeof(abSendBuf) );
	abSendBuf[0] = CMD_REQUEST;
	abSendBuf[1] = CMD_REQUEST;
	abSendBuf[2] = CMD_SUCCESS_RES;

	sResult = CtrlKpdSharedCmd( CMD_PARMS_RESET, abSendBuf, 3, -1 );
	if ( sResult == SUCCESS )
	{
		printf( "[SetMainTermID] 처리 성공\n" );
	}
	else
	{
		printf( "[SetMainTermID] 처리 실패\n" );
	}

	return sResult;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      ResetupReq                                               *
*                                                                              *
*  DESCRIPTION:       Request Resetup to CommProc                              *
*                                                                              *
*  INPUT PARAMETERS:  byte* pbRecvBuf                                          *
*                     word wRecvLen                                            *
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
short ResetupReq( byte* pbRecvBuf, word wRecvLen )
{
	int sRetVal = -1;
	byte bSendCode = 0;
	int nIndex = 0;
	int fdSetupFile = 0;
	int fdSetupBackupFile = 0;

	PrintlnASC( "[ResetupReq] 차량ID : ", pbRecvBuf + 1, 9 );
	PrintlnASC( "[ResetupReq] 집계IP : ", pbRecvBuf + 10, 12 );

	fdSetupFile = open( SETUP_FILE, O_WRONLY | O_CREAT |O_TRUNC, OPENMODE );
	if ( fdSetupFile < 0 )
	{
		printf( "[ResetupReq] setup.dat 파일 초기화 OPEN 실패\n" );
		return ErrRet( ERR_KEYPAD_SETUP_FILE_OPEN );
	}

	fdSetupBackupFile = open( SETUP_BACKUP_FILE,
							O_WRONLY | O_CREAT | O_TRUNC, OPENMODE );
	if ( fdSetupBackupFile < 0 )
	{
		printf( "[ResetupReq] setup.backup 파일 초기화 OPEN 실패\n" );
		close( fdSetupFile );
		return ErrRet( ERR_KEYPAD_SETUP_BACKUP_FILE_OPEN );
	}

	write( fdSetupFile, pbRecvBuf + 1, wRecvLen - 1 );
	write( fdSetupBackupFile, pbRecvBuf + 1, wRecvLen - 1 );

	close( fdSetupFile );
	close( fdSetupBackupFile );

	/* 
	 * TERMCOMM Process에게 재설치를 요청함
	 */
	memset( pbRecvBuf, 0x00, sizeof(pbRecvBuf) );
	pbRecvBuf[0] = CMD_REQUEST;
	sRetVal = CtrlKpdSharedCmd( CMD_RESETUP, pbRecvBuf, 1, -1 );
	if ( sRetVal != SUCCESS )
	{
		printf( "[ResetupReq] 재설치 요청 결과 실패 \n" );
		DisplayASCInDownFND( FND_ERR_GET_PSAM_ID );
		Buzzer( 5, 50000 );
		VoiceOut( VOICE_CHECK_TERM );
		sleep( 1 );
		DisplayASCInDownFND( FND_READY_MSG );
		return -1;
	}

	/* 
	 * 재설치결과 운전자조작기 Image를 재수신했을경우 조작기에 전송함
	 */
	if ( pbRecvBuf[4] == CMD_REQUEST )
	{
		bSendCode = NAK;
		SendKpd( &bSendCode, 1 );
		usleep( 100000 );
		gpstSharedInfo->boolIsKpdImgRecv = TRUE;
	}

	/* 
	 * 재설치후 Main Process에게 운영정보 재로딩을 요청함
	 */
	for ( nIndex=0; nIndex<4; nIndex++ )
	{
		pbRecvBuf[nIndex] = CMD_REQUEST;
	}
	pbRecvBuf[4] = CMD_SUCCESS_RES;
	sRetVal = CtrlKpdSharedCmd( CMD_NEW_CONF_IMG, pbRecvBuf, 5, -1 );
	if ( sRetVal != SUCCESS )
	{
		printf( "[ResetupReq] 운영정보 재로딩 요청 결과 실패 \n" );
		DisplayASCInDownFND( FND_ERR_GET_PSAM_ID );
		Buzzer( 5, 50000 );
		VoiceOut( VOICE_CHECK_TERM );
		sleep( 1 );
		DisplayASCInDownFND( FND_READY_MSG );
		return -1;
	}

	DisplayASCInDownFND( FND_INIT_MSG );
	Buzzer( 3, 1000000 );

	printf( "[ResetupReq] 처리 성공\n" );
	
	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      KpdSetTime                                               *
*                                                                              *
*  DESCRIPTION:       Set RTC Time received From Driver                        *
*                                                                              *
*  INPUT PARAMETERS:  byte* pbRecvBuf                                          *
*                     word wRecvLen                                            *
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
short KpdSetTime( byte* pbRecvBuf, word wRecvLen )
{
	time_t tTime = 0;
//	byte abDateTime[20] = { 0, };

	PrintlnASC( "[KpdSetTime] 변경시간 : ", pbRecvBuf + 1, 14 );
	tTime = GetTimeTFromASCDtime( pbRecvBuf + 1 );
	SetRTCTime( tTime );
//	GetRTCTime( &tTime );
//	TimeT2ASCDtime( tTime, abDateTime );

	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      KpdStartStopReq                                          *
*                                                                              *
*  DESCRIPTION:       Start or Stop Driving                                    *
*                                                                              *
*  INPUT PARAMETERS:  byte* pbRecvBuf                                          *
*                     word wRecvLen                                            *
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
short KpdStartStopReq( byte* pbRecvBuf, word wRecvLen )
{
	return CtrlKpdSharedCmd( CMD_START_STOP, pbRecvBuf, wRecvLen, -1 );
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      KpdExtra                                                 *
*                                                                              *
*  DESCRIPTION:       Process Extra received from Driver                       *
*                                                                              *
*  INPUT PARAMETERS:  byte* pbRecvBuf                                          *
*                     word wRecvLen                                            *
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
short KpdExtra( byte* pbRecvBuf, word wRecvLen )
{
	if ( wRecvLen != 1 )
	{
		return ErrRet( ERR_KEYPAD_DRIVER_CMD_LENGTH );
	}

	if ( (gpstSharedInfo->boolIsDriveNow != TRUE) ||
		 (gpstSharedInfo->boolIsReadyToDrive != TRUE) )
	{
		return ErrRet( ERR_KEYPAD_IS_DRIVE_NOW );
	}

	/* 1 : 할증 */
	if ( pbRecvBuf[1] == '1')
	{
		gstKpdPool.abDriverDisplayData[0] &= 0xCF;
		gstKpdPool.abDriverDisplayData[0] |= 0x10;
	}
	/* 2 : 할증 취소 */
	else if ( pbRecvBuf[1] == '2')
	{
		gstKpdPool.abDriverDisplayData[0] &= 0xCF;
		gstKpdPool.abDriverDisplayData[0] |= 0x20;
	}
	else
	{
		return ErrRet( ERR_KEYPAD_DRIVER_CMD_DATA );
	}

	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      CancelMultiEnt                                           *
*                                                                              *
*  DESCRIPTION:       Cancel Multi On										   *
*                                                                              *
*  INPUT PARAMETERS:  byte* pbRecvBuf                                          *
*                     word wRecvLen                                            *
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
short CancelMultiEnt( byte* pbRecvBuf, word wRecvLen )
{
	return CtrlKpdSharedCmd( CMD_CANCEL_MULTI_GETON, pbRecvBuf, wRecvLen, 10 );
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      KpdBLCheck                                               *
*                                                                              *
*  DESCRIPTION:       Check BL 		         								   *
*                                                                              *
*  INPUT PARAMETERS:  byte* pbRecvBuf                                          *
*                     word wRecvLen                                            *
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
short KpdBLCheck( byte* pbRecvBuf, word wRecvLen )
{
	short sRetVal = SUCCESS;
	byte abPrefix[7] = { 0, };
	byte abCardNum[10] = { 0, };
	byte abRespCode = 0;

	KPDCMD_COMM_PKT	stSendPacket;

	memset( &stSendPacket, 0x00, sizeof( KPDCMD_COMM_PKT ) );

	abRespCode = ACK;
	SendKpd( &abRespCode, 1 );
	memcpy( abPrefix, pbRecvBuf + 1, 6 );
	memcpy( abCardNum, pbRecvBuf + 7, 9 );

	sRetVal = SearchBLinBus( NULL, abPrefix, GetDWORDFromASC( abCardNum, 9 ),
		&abRespCode);
	if ( sRetVal == SUCCESS && abRespCode == 0 )
	{
		stSendPacket.abData[0] = 0x30;
	}
	else
	{
		stSendPacket.abData[0] = 0x31;
	}

	stSendPacket.bCmd = KPDCMD_BL_CHECK;
	stSendPacket.nDataSize = 1;

	sRetVal = SendKpdPkt( &stSendPacket );

	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      KpdPLCheck                                               *
*                                                                              *
*  DESCRIPTION:       Check PL 		         								   *
*                                                                              *
*  INPUT PARAMETERS:  byte* pbRecvBuf                                          *
*                     word wRecvLen                                            *
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
short KpdPLCheck( byte* pbRecvBuf, word wRecvLen )
{
	short sRetVal = 0;

	byte abCardNo[31] = { 0, };
	byte abRespCode = 0;
	dword dwCardAlias = 0;

	KPDCMD_COMM_PKT	 stSendPacket;
	
	memset( abCardNo, 0x00, sizeof(abCardNo) );
	memset( &stSendPacket, 0x00, sizeof(KPDCMD_COMM_PKT) );
	
	abRespCode = ACK;
	SendKpd( &abRespCode, 1 );
		
	memset( abCardNo, 0x00, sizeof(abCardNo) );
	memcpy( abCardNo, pbRecvBuf + 1, 10 );		
	dwCardAlias = GetDWORDFromASC( abCardNo, 10 );		
	sRetVal = SearchPL(dwCardAlias, &abRespCode);

	// 후불카드 alias의 경우 1비트 shift
	if ( dwCardAlias >= POSTPAY_START_ALIAS &&
		 dwCardAlias <= POSTPAY_END_ALIAS )
	{
		abRespCode = abRespCode << 1;
	}

	switch ( abRespCode )
	{
		case 0 :
			memcpy( stSendPacket.abData, "00", 2 );
			break;
		case 1 :
			memcpy( stSendPacket.abData, "01", 2 );
			break;
		case 2 :
			memcpy( stSendPacket.abData, "10", 2 );
			break;
		case 3 :
			memcpy( stSendPacket.abData, "11", 2 );
			break;
	}

	stSendPacket.bCmd = KPDCMD_PL_CHECK;
	stSendPacket.nDataSize = 2;

	usleep(1000);
	sRetVal = SendKpdPkt( &stSendPacket );

	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      MultiEnt                                                 *
*                                                                              *
*  DESCRIPTION:       Set Muti GetOn Structure								   *
*                                                                              *
*  INPUT PARAMETERS:  byte* pbRecvBuf                                          *
*                     word wRecvLen                                            *
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
short MultiEnt( byte* pbRecvBuf, word wRecvLen )
{
	return CtrlKpdSharedCmd( CMD_MULTI_GETON, pbRecvBuf, wRecvLen, 10 );
}

short CityTourTicketInput( byte* pbRecvBuf, word wRecvLen )
{
	return CtrlKpdSharedCmd( CMD_INPUT_CITY_TOUR_BUS_TICKET, pbRecvBuf,
		wRecvLen, 10 );
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      StationCorrectReq                                        *
*                                                                              *
*  DESCRIPTION:       Move StationID +1 or -1								   *
*                                                                              *
*  INPUT PARAMETERS:  byte* pbRecvBuf                                          *
*                     word wRecvLen                                            *
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
short StationCorrectReq( byte* pbRecvBuf, word wRecvLen )
{
	return CtrlKpdSharedCmd(CMD_STATION_CORRECT, pbRecvBuf, wRecvLen, 10);
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      CashEntReceiptPrintReq                                   *
*                                                                              *
*  DESCRIPTION:       Request Printing of Cash GetOn Receipts				   *
*                                                                              *
*  INPUT PARAMETERS:  byte* pbRecvBuf                                          *
*                     word wRecvLen                                            *
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
short CashEntReceiptPrintReq( byte* pbRecvBuf, word wRecvLen )
{
	byte achSendData[12] = { 0, };

	achSendData[0] = pbRecvBuf[0];
	achSendData[1] = KPDCMD_CASHENT_RECEIPT_PRINT;
	memcpy( achSendData + 2, pbRecvBuf + 1, wRecvLen - 1 );
	return CtrlKpdSharedCmd(CMD_PRINT, achSendData, strlen(achSendData), 10);
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SubTermIDSetReq                                          *
*                                                                              *
*  DESCRIPTION:       Request Setting of New SubTermID       				   *
*                                                                              *
*  INPUT PARAMETERS:  byte* pbRecvBuf                                          *
*                     word wRecvLen                                            *
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
short SubTermIDSetReq( byte* pbRecvBuf, word wRecvLen )
{
	short sResult = SUCCESS;
	int fdSubID = 0;
	byte i = 0;
	byte bSubTermCnt = 0;
	// 하차단말기 미존재시 하차단말기ID파일(subid.dat)에 해당 단말기 자리에는
	// '0'문자로 채워 기록한다.
	byte abSubTermIDBuf[SUBTERM_ID_COUNT_SIZE] = {0, };

	memset( abSubTermIDBuf, '0', sizeof( abSubTermIDBuf ) );

	fdSubID = open( SUBTERM_ID_FILENAME, O_WRONLY | O_CREAT | O_TRUNC,
		OPENMODE );
	if ( fdSubID < 0 )
	{
		printf( "[SubTermIDSetReq] 'subid.dat' 파일 OPEN 오류\n" );
		return -1;
	}

	bSubTermCnt = GetDECFromASCNo( pbRecvBuf[1] );

	abSubTermIDBuf[0] = pbRecvBuf[1];
	for ( i = 0; i < 3; i++ )
	{
		if ( i < bSubTermCnt )
		{
			memcpy( &abSubTermIDBuf[1 + ( i * SUBTERM_ID_SIZE )],
				&pbRecvBuf[2 + ( i * SUBTERM_ID_SIZE )], SUBTERM_ID_SIZE );
		}
	}

	write( fdSubID, abSubTermIDBuf, sizeof( abSubTermIDBuf ) );
	close( fdSubID );

	// 신규로 입력된 하차단말기개수가 0개이면 굳이 승하차통신을 시도할 필요가
	// 없다.
	if ( bSubTermCnt != 0 )
	{
		sResult = CtrlKpdSharedCmd( CMD_SUBID_SET, pbRecvBuf, wRecvLen, 30 );
	}

	if ( sResult == SUCCESS )
	{
		printf( "[SubTermIDSetReq] 처리 성공\n" );
	}
	else
	{
		printf( "[SubTermIDSetReq] 처리 실패\n" );
	}

	return sResult;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      ServerIPSetting                                          *
*                                                                              *
*  DESCRIPTION:       Sets DCS Server IP                                       *
*                                                                              *
*  INPUT PARAMETERS:  byte* pbRecvBuf                                          *
*                     word wRecvLen                                            *
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
short ServerIPSetting( byte* pbRecvBuf, word wRecvLen )
{
	short sResult = SUCCESS;
	char achBuf[41] = { 0, };
	int fdSetupFile = 0;

	memset( achBuf, 0x00, sizeof(achBuf) );

	fdSetupFile = open( SETUP_FILE, O_RDWR );
	if ( fdSetupFile < 0 )
	{
		DebugOut( "Error!!! open setup.dat failed\n" );
		return ErrRet( ERR_KEYPAD_SETUP_FILE_OPEN );
	}

	read( fdSetupFile, achBuf, MAINTERM_ID_SIZE + SERVER_IP_SIZE );
	memcpy( achBuf + MAINTERM_ID_SIZE, pbRecvBuf + 1, wRecvLen - 1 );
	lseek( fdSetupFile, 0, SEEK_SET );
	write( fdSetupFile, achBuf, MAINTERM_ID_SIZE + SERVER_IP_SIZE );
	close( fdSetupFile );

	memset( achBuf, 0x00, sizeof(achBuf) );
	achBuf[0] = CMD_REQUEST;
	achBuf[1] = CMD_SUCCESS_RES;
	achBuf[2] = CMD_REQUEST;

	sResult = CtrlKpdSharedCmd( CMD_PARMS_RESET, achBuf, 3, -1 );
	if ( sResult == SUCCESS )
	{
		printf( "[ServerIPSetting] 처리 성공\n" );
	}
	else
	{
		printf( "[ServerIPSetting] 처리 실패\n" );
	}

	return sResult;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      TermInfoPrintingReq2Main                                 *
*                                                                              *
*  DESCRIPTION:       This program requests TermInfo Printing to main process  *
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
short TermInfoPrintingReq2Main( void )
{
	short sRetVal = 0;
	int nIndex = 0;
	byte abKpdVer[5] = { 0, };
	char achSendData[41] = { 0, };

	for ( nIndex = 0; nIndex < 3; nIndex++ )
	{
		if ( IsAllZero( gpstSharedInfo->abKpdVer,
				sizeof( gpstSharedInfo->abKpdVer ) ) == FALSE &&
			IsAllASCZero( gpstSharedInfo->abKpdVer,
				sizeof( gpstSharedInfo->abKpdVer ) ) == FALSE &&
			IsAllZero( gpstSharedInfo->abDriverOperatorID,
				sizeof( gpstSharedInfo->abDriverOperatorID ) ) == FALSE &&
			IsAllASCZero( gpstSharedInfo->abDriverOperatorID,
				sizeof( gpstSharedInfo->abDriverOperatorID ) ) == FALSE )
		{
			break;
		}
		ReceiveImgVerfromKpd( abKpdVer, gpstSharedInfo->abDriverOperatorID );
	}

	achSendData[0] = 0x31;
	achSendData[1] = KPDCMD_TERMINFO_PRINT;

	sRetVal = CtrlKpdSharedCmd( CMD_PRINT, achSendData, 2, 10 );

	return sRetVal;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      CtrlKpdSharedCmd                                         *
*                                                                              *
*  DESCRIPTION:       Set Shared Command with Main & Comm Process			   *
*                     and Receive Execution Result.                            *
*                                                                              *
*  INPUT PARAMETERS:  byte bCmd                                                *
*                     char* pcCmdData                                          *
*                     word wCmdDataSize                                        *
*                     word wTimeOutSec                                         *
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
short CtrlKpdSharedCmd( byte bCmd, char* pcCmdData, word wCmdDataSize,
	int nTimeOutSec )
{
	short sReturn = SUCCESS;
	time_t tStartTime = 0;
	time_t tEndTime = 0;

	byte bRecvCmd = 0;
	char achRecvData[41] = { 0, };
	word wRecvDataSize = 0;
	byte bRequestResult = 0;

	printf( "[CtrlKpdSharedCmd] 타임아웃 시간 : %d sec\n", nTimeOutSec );

	if ( gpstSharedInfo->boolIsReadyToDrive != TRUE ||
		 gpstSharedInfo->boolIsKpdLock == TRUE )
	{
		ErrRet( ERR_KEYPAD_DRIVER_CMD_LOSS );
	}
	
	tStartTime = gpstSharedInfo->tTermTime;

	/* 
	 * MAIN/TERMCOMM Process에게 Driver Command를 전달함
	 */
	while ( SetSharedCmdnData(bCmd, pcCmdData, wCmdDataSize) != SUCCESS )
	{
		printf( "[CtrlKpdSharedCmd] SetSharedCmdnData() 실패 : [%02x] [%c]\n", bCmd, bCmd );

		tEndTime = gpstSharedInfo->tTermTime;
		if ( nTimeOutSec != -1 &&
			 abs(tEndTime - tStartTime) > nTimeOutSec )
		{
			printf( "[CtrlKpdSharedCmd] SetSharedCmdnData() 타임아웃\n" );
			ClearSharedCmdnData( bCmd );
			return ErrRet( ERR_KEYPAD_DRIVER_CMD_TIMEOVER );
		}

		usleep( 10000 );
	}

	/* 
	 * MAIN/TERMCOMM Process에서 Command실행결과를 Return받음
	 */
	bRequestResult = 0;							// 초기화
	while ( bRequestResult != CMD_SUCCESS_RES &&
			bRequestResult != CMD_FAIL_RES )
	{
		tEndTime = gpstSharedInfo->tTermTime;
		if ( nTimeOutSec != -1 &&
			 abs(tEndTime - tStartTime) > nTimeOutSec )
		{
			printf( "[CtrlKpdSharedCmd] GetSharedCmd() 타임아웃\n" );
			ClearSharedCmdnData( bCmd );
			return ErrRet( ERR_KEYPAD_DRIVER_CMD_TIMEOVER );
		}

		bRecvCmd = '0';
		wRecvDataSize = 0;
		memset( achRecvData, 0x00, sizeof( achRecvData ) );

		sReturn = GetSharedCmd( &bRecvCmd, achRecvData, &wRecvDataSize );		
		if ( sReturn == SUCCESS )
		{
			if ( bRecvCmd == bCmd )
			{
				bRequestResult = achRecvData[0];
			}
		}

		if ( gpstSharedInfo->boolIsKpdLock == TRUE )
		{
			KpdCommProcPolling( &bRecvCmd, achRecvData );
		}

		usleep( 10000 );
	}

	ClearSharedCmdnData( bCmd );

	if ( bRequestResult == CMD_FAIL_RES )
	{
		printf( "[CtrlKpdSharedCmd] GetSharedCmd() 실패 응답\n" );
		return ERR_KEYPAD_DRIVER_CMD_FAIL_RES;
	}

	printf( "[CtrlKpdSharedCmd] GetSharedCmd() 성공 응답\n" );

	return SUCCESS;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SCRead                                                   *
*                                                                              *
*  DESCRIPTION:       This program reads Smart Card, checks validation of card *
*                     and saves Common Structure.                              *
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
void CheckLancardSignal( void )
{
	FILE* fileStatus = NULL;
	char* pchBuf = NULL;
	int nBufSize = 128;
	int nIndex = 0;
	char achLanStrength[16] = { 0, };
	char achLanQuality[16] = { 0, };
	int nReadByte = 0;
	int nMaxLine = 0;
	int nStrength = 0;
	int nQuality = 0;

	fileStatus = fopen( LANCARD_SIGNAL_FILE, "r" );
	if ( fileStatus == NULL)
	{
		return;
	}

	pchBuf = (byte*)malloc( nBufSize );
	for( nIndex = 0; nIndex < 3 ; nIndex++ )
	{
		memset( pchBuf, 0x00, sizeof(pchBuf) );
		nReadByte = getline( &pchBuf, &nBufSize, fileStatus );
		if ( nReadByte <= 0 )
		{
			break;
		}

		if ( strstr(pchBuf, "Signal Strength:") )
		{
			nMaxLine  = strlen(pchBuf);
			nStrength = GetINTFromASC( pchBuf + 17, (nMaxLine -17) -1 );
			sprintf( achLanStrength, "%03d", nStrength );
			memset( gpstSharedInfo->abLanCardStrength, 0x00,
					sizeof(gpstSharedInfo->abLanCardStrength) );
			memcpy( gpstSharedInfo->abLanCardStrength, achLanStrength,
					sizeof(gpstSharedInfo->abLanCardStrength) );
		}
		
		if ( strstr(pchBuf, "Signal Quality:") )
		{
			nMaxLine  = strlen(pchBuf);
			nQuality = GetINTFromASC( pchBuf + 16, (nMaxLine -16) -1 );
			sprintf( achLanQuality, "%03d", nQuality );
			memset( gpstSharedInfo->abLanCardQuality, 0x00,
					sizeof(gpstSharedInfo->abLanCardQuality) );
			memcpy( gpstSharedInfo->abLanCardQuality, achLanQuality,
					sizeof(gpstSharedInfo->abLanCardQuality) );
		}		
	}

	fclose(fileStatus);
	free( pchBuf );
}

