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
*  PROGRAM ID :       printer_proc.c                                           *
*                                                                              *
*  DESCRIPTION:       This program processes Receipt Printing                  *
*                                                                              *
*  ENTRY POINT:       CommPrinter()               ** mandatory **              *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: None                                                     *
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
*  Declaration of Header Files                                                 *
*******************************************************************************/
#include "printer_proc.h"
#include "load_parameter_file_mgt.h"
#include "version_file_mgt.h"

/*******************************************************************************
*  Declaration of Global Variables                                             *
*******************************************************************************/
static int gfdPrinterDevice = 0;			// Printer Device File Descriptor

/*******************************************************************************
*  Declaration of Module Functions                                             *
*******************************************************************************/
static void PrintOutCashEnt( char* pchRecvData );
static void PrintOutTermInfo( void );
static short WritePrinter( char* pcData, word wDataSize );
static void PrintOutStr( byte *abTitle, byte *abStr, byte bStrSize );
static void PrintOutDWORD( byte *abTitle, dword dwInput );

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      CommPrinter                                              *
*                                                                              *
*  DESCRIPTION:       Printing Program Main                                    *
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
void CommPrinter( void )
{
	short sRetVal = 0;
	int	nReadSize = 0;
	MSGQUEUE_DATA stMsgQueueData;

	while ( 1 )
	{
		gnMsgQueue = msgget( 900000 + 1, IPC_CREAT | IPC_PERM );
		if ( gnMsgQueue >= 0 )
		{
			break;
		}
		else
		{
			perror( "Message Queue failed in CommPrinter : " );
			printf( "Msgget gnMsgQueue error \n" );
			DisplayASCInUpFND( "888888" );
			DisplayASCInDownFND( "888888" );
			Buzzer( 5, 50000 );
			VoiceOut( VOICE_CHECK_TERM );		// Msg of Checking Terminal.

			sleep( 3 );
		}
	}

	while ( 1 )
	{
		sRetVal = OpenPrinter( &gfdPrinterDevice, 38400 );
		if ( sRetVal >= 0 )
		{
			break;
		}
		else
		{
			printf( "\nPrinter Device Open Failed!!\n" );
			DisplayASCInUpFND( "888888" );
			DisplayASCInDownFND( "888888" );
			Buzzer( 5, 50000 );
			VoiceOut( VOICE_CHECK_TERM );		// Msg of Checking Terminal.

			sleep( 3 );
		}
	}

	// 2006-02-07 Gykim
	ClosePrinter( &gfdPrinterDevice );

	// Main에서 공유메모리 데이타를 채울수있는 시간제공
	sleep( 5 );

	// 메세지큐 획득
	memset( &stMsgQueueData, 0x00, sizeof(MSGQUEUE_DATA) );
	while ( 1 )
	{
		// 메세지큐 읽기
		nReadSize = msgrcv( gnMsgQueue,
							&stMsgQueueData,
							MSGQUEUE_MAX_DATA + 1,
							0,
							0 );
	    if ( nReadSize < 0 )
	    {
			perror( "msgrcv failed\n PrinterProcess Exit!! \n" );
			printf( "\nPrinter Device Open Failed!!\n" );
			DisplayASCInUpFND( "888888" );
			DisplayASCInDownFND( "888888" );
			Buzzer( 5, 50000 );
			VoiceOut( VOICE_CHECK_TERM );		// Msg of Checking Terminal.

			sleep( 3 );
		}
		else if ( nReadSize == 0 )
		{
	    	usleep( 500 );
	    	continue;
	    }
	    else
	    {
	    	if ( stMsgQueueData.lMsgType == PRINT_RECEIPT )
	    	{
	    		PrintOutCashEnt( stMsgQueueData.achMsgData );
				usleep( 1000000 );
	    	}
	    	else if ( stMsgQueueData.lMsgType == PRINT_TERM_INFO )
	    	{
	    		PrintOutTermInfo();
	    	}
	    	memset( &stMsgQueueData, 0x00, sizeof(MSGQUEUE_DATA) );
	    }
	}

	// 메세지큐 반납
	msgctl( gnMsgQueue, IPC_RMID, NULL );

	//ClosePrinter( &gfdPrinterDevice );
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      PrintOutCashEnt                                          *
*                                                                              *
*  DESCRIPTION:       Print Receipt of Cash GetOn.                             *
*                                                                              *
*  INPUT PARAMETERS:  char* pchRecvData                                        *
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
static void PrintOutCashEnt( char* pchRecvData )
{
	short sRetVal = 0;
	char achPrintBuf[255];
	char achDataBuf[255];

	MSGQUEUE_RECEIPT_DATA stMsgQueueReceipt;

	sRetVal = OpenPrinter( &gfdPrinterDevice, 38400 );

	memcpy( &stMsgQueueReceipt, pchRecvData, sizeof( MSGQUEUE_RECEIPT_DATA ) );

	sprintf( achPrintBuf,"%c%c일회 승차권(영수증)%c%c", 0x11, 0x13, 0x12, 0x0d );
	sRetVal = WritePrinter( achPrintBuf, strlen(achPrintBuf) );

	memset( achPrintBuf,0x00,sizeof(achPrintBuf) );
	sprintf( achPrintBuf, "    %c", 0x0d );
	sRetVal = WritePrinter( achPrintBuf, strlen(achPrintBuf) );

	memset( achPrintBuf, 0x00, sizeof(achPrintBuf) );
	memset( achDataBuf, 0x00, sizeof(achDataBuf) );
	memcpy( achDataBuf, stMsgQueueReceipt.abBizNo, 3 );
	achDataBuf[3] = '-';
	memcpy( achDataBuf + 4, stMsgQueueReceipt.abBizNo + 3, 2 );
	achDataBuf[6] = '-';
	memcpy( achDataBuf + 7, stMsgQueueReceipt.abBizNo + 5, 5 );
	sprintf( achPrintBuf, "사업자 번호   :  %s%c", achDataBuf, 0x0d );
	sRetVal = WritePrinter( achPrintBuf, strlen(achPrintBuf) );

	memset( achPrintBuf,0x00,sizeof(achPrintBuf) );
	memset( achDataBuf,0x00,sizeof(achDataBuf) );
	memcpy( achDataBuf, stMsgQueueReceipt.abDateTime, 4 );
	memcpy( achDataBuf + strlen(achDataBuf), "년", strlen("년") );
	memcpy( achDataBuf + strlen(achDataBuf),
			stMsgQueueReceipt.abDateTime + 4, 2 );
	memcpy( achDataBuf + strlen(achDataBuf), "월", strlen("월") );
	memcpy( achDataBuf + strlen(achDataBuf),
			stMsgQueueReceipt.abDateTime + 6, 2 );
	memcpy( achDataBuf + strlen(achDataBuf), "일", strlen("일") );
	sprintf( achPrintBuf, "발행 일자    :  %s%c", achDataBuf, 0x0d );
	sRetVal = WritePrinter( achPrintBuf, strlen(achPrintBuf) );

	memset( achPrintBuf,0x00,sizeof(achPrintBuf) );
	memset( achDataBuf,0x00,sizeof(achDataBuf) );
	memcpy( achDataBuf, stMsgQueueReceipt.abDateTime + 8, 2 );
	memcpy( achDataBuf + strlen(achDataBuf), " : ", strlen(" : ") );
	memcpy( achDataBuf + strlen(achDataBuf),
			stMsgQueueReceipt.abDateTime + 10, 2 );
	memcpy( achDataBuf + strlen(achDataBuf), " : ", strlen(" : ") );
	memcpy( achDataBuf + strlen(achDataBuf),
			stMsgQueueReceipt.abDateTime + 12, 2 );
	sprintf( achPrintBuf, "발행 시간    :  %s %c", achDataBuf, 0x0d );
	sRetVal = WritePrinter( achPrintBuf, strlen(achPrintBuf) );

	memset( achPrintBuf,0x00,sizeof(achPrintBuf) );
	memset( achDataBuf,0x00,sizeof(achDataBuf) );
	memcpy( achDataBuf, stMsgQueueReceipt.abVehicleNo,
		sizeof(stMsgQueueReceipt.abVehicleNo) );
	sprintf( achPrintBuf, "차량 번호    :  %s %c", achDataBuf, 0x0d );
	sRetVal = WritePrinter( achPrintBuf, strlen(achPrintBuf) );

	memset( achPrintBuf,0x00,sizeof(achPrintBuf) );
	memset( achDataBuf,0x00,sizeof(achDataBuf) );
	memcpy( achDataBuf, stMsgQueueReceipt.abTranspMethodCodeName,
		sizeof(stMsgQueueReceipt.abTranspMethodCodeName) );
	sprintf( achPrintBuf, "버스 타입    :  %s %c", achDataBuf, 0x0d );
	sRetVal = WritePrinter( achPrintBuf, strlen(achPrintBuf) );

	memset( achPrintBuf,0x00,sizeof(achPrintBuf) );
	memset( achDataBuf,0x00,sizeof(achDataBuf) );
	memcpy( achDataBuf, stMsgQueueReceipt.abUserTypeName,
		sizeof(stMsgQueueReceipt.abUserTypeName) );
	sprintf( achPrintBuf, "승객 구분    :  %s %c", achDataBuf, 0x0d );
	sRetVal = WritePrinter( achPrintBuf, strlen(achPrintBuf) );

	memset( achPrintBuf,0x00,sizeof(achPrintBuf) );
	memset( achDataBuf,0x00,sizeof(achDataBuf) );
	memcpy( achDataBuf, stMsgQueueReceipt.achBusStationName,
		sizeof(stMsgQueueReceipt.achBusStationName) );
	sprintf( achPrintBuf, "승 차 역    :  %s %c", achDataBuf, 0x0d );
	sRetVal = WritePrinter( achPrintBuf, strlen(achPrintBuf) );

	memset( achPrintBuf,0x00,sizeof(achPrintBuf) );
	memset( achDataBuf,0x00,sizeof(achDataBuf) );
	memcpy( achDataBuf, stMsgQueueReceipt.abFare,
			sizeof(stMsgQueueReceipt.abFare) );
	sprintf( achPrintBuf, "요    금    : %c%c %s원%c%c",
		0x11,0x13, achDataBuf, 0x12, 0x0d );
	sRetVal = WritePrinter( achPrintBuf, strlen(achPrintBuf) );

	memset( achPrintBuf,0x00,sizeof(achPrintBuf) );
	sprintf( achPrintBuf, "      이용해 주셔서 감사합니다.%c", 0x0d );
	sRetVal = WritePrinter( achPrintBuf, strlen(achPrintBuf) );

	memset( achPrintBuf,0x00,sizeof(achPrintBuf) );
	sprintf( achPrintBuf, "        한국 스마트카드 (주)%c%c%c",
			 0x0d, 0x14, 0x0d );
	sRetVal = WritePrinter( achPrintBuf, strlen(achPrintBuf) );

	ClosePrinter( &gfdPrinterDevice );
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      PrintOutTermInfo                                         *
*                                                                              *
*  DESCRIPTION:       Print Terminal Info to the Printer.                      *
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
static void PrintOutTermInfo( void )
{
	int nIndex = 0;
	int fdData = 0;
	long lFreeMemory = 0;
	char abDataBuf[256] = {0, };
	VER_INFO stVerInfo;
	COMM_INFO stServerIP;

	memset( &stVerInfo, 0x00, sizeof( VER_INFO ) );
	memset( &stServerIP, 0x20, sizeof( COMM_INFO ) );

	OpenPrinter( &gfdPrinterDevice, 38400 );

	// 차량정보 로드
	LoadVehicleParm();

	// 정류장정보 로드
	LoadStationInfo();

	// 버전정보 로드
	LoadVerInfo( &stVerInfo );

	// HEADER출력 //////////////////////////////////////////////////////////////
	sprintf( abDataBuf, "%c%c    단말기 정보 %c", 0x11, 0x13, 0x12 );
	PrintOutStr( abDataBuf, "", 0 );
	PrintOutStr( "    ", "", 0 );

	TimeT2ASCDtime( gpstSharedInfo->tTermTime, abDataBuf );
	PrintOutStr(	"현재일자   :  ",			abDataBuf, 14 );
	PrintOutStr(	"승차 ID    :  ",			gpstSharedInfo->abMainTermID,
		sizeof( gpstSharedInfo->abMainTermID ) );

	for ( nIndex = 0; nIndex < gbSubTermCnt; nIndex++ )
	{
		PrintOutStr( "하차 ID    :  ", gpstSharedInfo->abSubTermID[nIndex],
			sizeof( gpstSharedInfo->abSubTermID[nIndex] ) );
	}

	PrintOutStr(	"운전자조작기 ID  :  ",		gpstSharedInfo->abDriverOperatorID,
		sizeof( gpstSharedInfo->abDriverOperatorID ) );
	PrintOutStr(	"노선 ID    :  ",			gstVehicleParm.abRouteID,
		sizeof( gstVehicleParm.abRouteID ) );
	PrintOutStr(	"노선명    :  ",			gstStationInfoHeader.abTranspMethodName,
		sizeof( gstStationInfoHeader.abTranspMethodName ) );
	PrintOutStr(	"차량번호    :  ",			gstVehicleParm.abVehicleNo,
		sizeof( gstVehicleParm.abVehicleNo ) );
	PrintOutStr(	"교통사업자 ID    :  ",		gstVehicleParm.abTranspBizrID,
		sizeof( gstVehicleParm.abTranspBizrID ) );
	PrintOutStr(	"교통사업자명    :  ",		gstVehicleParm.abTranspBizrNm,
		sizeof( gstVehicleParm.abTranspBizrNm ) );
	PrintOutStr(	"교통수단코드    :  ",		gstVehicleParm.abTranspMethodCode,
		sizeof( gstVehicleParm.abTranspMethodCode ) );

	fdData = open( SETUP_FILE, O_RDONLY );
	if ( fdData < 0 )
	{
		printf("[PrintOutTermInfo] setup.dat 파일 OPEN 오류\n");
	}
	else
	{
		read( fdData, &stServerIP, sizeof(COMM_INFO) );
		close( fdData );
	}
	sprintf( abDataBuf, "%d.%d.%d.%d",
						GetINTFromASC(stServerIP.abDCSIPAddr, 3),
						GetINTFromASC(stServerIP.abDCSIPAddr + 3, 3),
						GetINTFromASC(stServerIP.abDCSIPAddr + 6, 3),
						GetINTFromASC(stServerIP.abDCSIPAddr + 9, 3) );
	PrintOutStr(	"서버 IP    :  ",			abDataBuf,
		strlen( abDataBuf ) );
	PrintOutStr(	"승차버전    :  ",			MAIN_RELEASE_VER, 4 );
	for ( nIndex = 0; nIndex < gbSubTermCnt; nIndex++ )
	{
		PrintOutStr( "하차버전    :  ", gpstSharedInfo->abSubVer[nIndex],
			sizeof( gpstSharedInfo->abSubVer[nIndex] ) );
	}

	PrintOutStr(	"운전자버전    :  ",		gpstSharedInfo->abKpdVer,
		sizeof( gpstSharedInfo->abKpdVer ) );
	PrintOutStr(	"승차SAMID    :  ",			gpstSharedInfo->abMainPSAMID,
		sizeof( gpstSharedInfo->abMainPSAMID ) );

	for ( nIndex = 0; nIndex < gbSubTermCnt; nIndex++ )
	{
		PrintOutStr( "하차SAMID    :  ", gpstSharedInfo->abSubPSAMID[nIndex],
			sizeof( gpstSharedInfo->abSubPSAMID[nIndex] ) );
	}

	PrintOutStr(	"커널버전    :  ",			gabKernelVer, 2 );

	lFreeMemory = MemoryCheck();
	if ( lFreeMemory < 0 )
	{
		lFreeMemory = 0;
	}
	sprintf( abDataBuf, "%ldM", lFreeMemory );
	PrintOutStr(	"남은용량    :  ",			abDataBuf,
		strlen( abDataBuf ) );

	// 필수운영정보 ////////////////////////////////////////////////////////////
	PrintOutStr( "+ 필수운영정보 -----------------------", "", 0 );

	PrintOutStr(	"차량정보 버전 :  ",		stVerInfo.abVehicleParmVer,
		sizeof( stVerInfo.abVehicleParmVer ) );
	PrintOutStr(	"노선정보 버전 :  ",		stVerInfo.abRouteParmVer,
		sizeof( stVerInfo.abRouteParmVer ) );
	PrintOutStr(	"요금정보 버전 :  ",		stVerInfo.abBasicFareInfoVer,
		sizeof( stVerInfo.abBasicFareInfoVer ) );
	PrintOutStr(	"정류장정보 버전 :  ",		stVerInfo.abBusStationInfoVer,
		sizeof( stVerInfo.abBusStationInfoVer ) );

	// 기타운영정보 ////////////////////////////////////////////////////////////
	PrintOutStr( "+ 기타운영정보 -----------------------", "", 0 );

	PrintOutStr(	"선불발행사 버전 :  ",		stVerInfo.abPrepayIssuerInfoVer,
		sizeof( stVerInfo.abPrepayIssuerInfoVer ) );
	PrintOutStr(	"후불발행사 버전 :  ",		stVerInfo.abPostpayIssuerInfoVer,
		sizeof( stVerInfo.abPostpayIssuerInfoVer ) );
	PrintOutStr(	"발행사유효기간 버전 :  ",	stVerInfo.abIssuerValidPeriodInfoVer,
		sizeof( stVerInfo.abIssuerValidPeriodInfoVer ) );
	PrintOutStr(	"할인할증정보 버전 :  ",	stVerInfo.abDisExtraInfoVer,
		sizeof( stVerInfo.abDisExtraInfoVer ) );
	PrintOutStr(	"휴일정보 버전 :  ",		stVerInfo.abHolidayInfoVer,
		sizeof( stVerInfo.abHolidayInfoVer ) );
	PrintOutStr(	"환승적용정보 버전 :  ",	stVerInfo.abXferApplyInfoVer,
		sizeof( stVerInfo.abXferApplyInfoVer ) );

	// BL/PL정보 ///////////////////////////////////////////////////////////////
	PrintOutStr( "+ BL/PL정보 --------------------------", "", 0 );

	PrintOutStr(	"고정 BL 버전 :  ",			stVerInfo.abMasterBLVer,
		sizeof( stVerInfo.abMasterBLVer ) );
	PrintOutDWORD(	"고정 BL size :  ",			GetFileSize("c_fi_bl.dat") );
	PrintOutStr(	"변동 BL 버전 :  ",			stVerInfo.abUpdateBLVer,
		sizeof( stVerInfo.abUpdateBLVer ) );

	PrintOutStr(	"고정 구선불PL 버전 :  ",	stVerInfo.abMasterPrepayPLVer,
		sizeof( stVerInfo.abMasterPrepayPLVer ) );
	PrintOutDWORD(	"고정 구선불PL size :  ",	GetFileSize("c_fa_pl.dat") );
	PrintOutStr(	"고정 후불PL 버전 :  ",		stVerInfo.abMasterPostpayPLVer,
		sizeof( stVerInfo.abMasterPostpayPLVer ) );
	PrintOutDWORD(	"고정 후불PL size :  ",		GetFileSize("c_fd_pl.dat") );
	PrintOutStr(	"고정 신선불PL 버전 :  ",	stVerInfo.abMasterAIVer,
		sizeof( stVerInfo.abMasterAIVer ) );
	PrintOutDWORD(	"고정 신선불PL size :  ",	GetFileSize("c_fi_ai.dat") );

	PrintOutStr(	"변동 PL 버전 :  ",			stVerInfo.abUpdatePLVer,
		sizeof( stVerInfo.abUpdatePLVer ) );
	PrintOutStr(	"변동 신선불PL 버전 :  ",	stVerInfo.abUpdateAIVer,
		sizeof( stVerInfo.abUpdateAIVer ) );

	// 기타 정보 ///////////////////////////////////////////////////////////////
	PrintOutStr( "+ ------------------------------------", "", 0 );

	PrintOutStr(	"승차음성 버전 :  ",		gpstSharedInfo->abMainVoiceVer,
		sizeof( gpstSharedInfo->abMainVoiceVer ) );
	for ( nIndex = 0; nIndex < gbSubTermCnt; nIndex++ )
	{
		PrintOutStr( "하차음성 버전 :  ", gpstSharedInfo->abSubVoiceVer[nIndex],
			sizeof( gpstSharedInfo->abSubVoiceVer[nIndex] ) );
	}

	fdData = open( PG_LOADER_VER_FILE, O_RDONLY );
	if ( fdData < 0 )
	{
		printf("[PrintOutTermInfo] pgver.dat 파일 OPEN 오류\n");
	}
	else
	{
		read( fdData, abDataBuf, 4 );
		close( fdData );
	}
	PrintOutStr(	"PgLoader 버전 :  ",		abDataBuf, 4 );

	// TAIL 출력 ///////////////////////////////////////////////////////////////
	sprintf( abDataBuf, "        한국 스마트카드 (주)%c%c", 0x0d, 0x14 );
	PrintOutStr( abDataBuf, "", 0 );

	ClosePrinter( &gfdPrinterDevice );
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      WritePrinter                                             *
*                                                                              *
*  DESCRIPTION:       This program writes pcData to Printer					   *
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
static short WritePrinter( char* pcData, word wDataSize )
{
	short sRetVal = 0;

	sRetVal = write( gfdPrinterDevice, pcData, wDataSize );

	return sRetVal;
}

static void PrintOutStr( byte *abTitle, byte *abStr, byte bStrSize )
{
	byte abPrintBuf[256] = {0, };
	byte abBuf[256] = {0, };

	usleep( 50000 );
	memcpy( abBuf, abStr, bStrSize );
	sprintf( abPrintBuf, "%s%s%c", abTitle, abBuf, 0x0d );
	printf( "%s\r\n", abPrintBuf );
	WritePrinter( abPrintBuf, strlen( abPrintBuf ) );
}

static void PrintOutDWORD( byte *abTitle, dword dwInput )
{
	byte abPrintBuf[256] = {0, };

	usleep( 50000 );
	sprintf( abPrintBuf, "%s%lu%c", abTitle, dwInput, 0x0d );
	printf( "%s\r\n", abPrintBuf );
	WritePrinter( abPrintBuf, strlen( abPrintBuf ) );
}

