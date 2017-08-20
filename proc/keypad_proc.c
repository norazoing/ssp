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
*  PROGRAM ID :		keypad_proc.h                                              *
*                                                                              *
*  DESCRIPTION:		This program comm. with Driver & Main Proc./ Comm Proc.    *
*                                                                              *
*  ENTRY POINT:		void CommKpd( void )                                       *
* 					int ASC2HEX(byte *abSrcASC,byte *abDesHEX,byte bLengthSrc) *
* 					short KpdCommProcPolling( byte* bCmd, byte* pbRecvBuf )    *
* 					short ReceiveImgVerfromKpd( byte* pbKpdVer )               *
* 					short SendKpd( byte* pbSendData, word wSendSize )          *
* 					short SendKpdPkt( KPDCMD_COMM_PKT* pstSendData )           *
* 					short SendNewImgProcess( char* pchKpdImgVer )              *
* 					short SendNewImgAfterVerCheck( void )                      *
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
*  Inclusion of Header Files                                                   *
*******************************************************************************/
#define _GNU_SOURCE
#include "../system/bus_type.h"
#include "keypad_proc.h"
#include "keypad_proc_mainComm.h"
#include "load_parameter_file_mgt.h"

/*******************************************************************************
*  Declaration of Global Variables                                             *
*******************************************************************************/
KPDCOMM_POOL_DATA gstKpdPool;
static bool gboolIsSendFare = FALSE;

/*******************************************************************************
*  Declaration of Module Global Variables                                      *
*******************************************************************************/
static int gfdKpdDevice = 0;

/*******************************************************************************
*  Declaration of Static Functions                                             *
*******************************************************************************/
static short CommKpdParseRecvData( byte bCmd, byte* pbRecvBuf, word wRecvLen );
static bool CheckUpdateYN( void );
static short RecvKpdPkt( KPDCMD_COMM_PKT* pstRecvData, int nTimeOut );
static short RecvKpd( byte* pbRecvData, word wRecvSize, int nTimeOut );
static short OpenKpd( dword wBaudrate );
static void CloseKpd( void );

static short kpdInitEnvironment(void);
static bool getKpdImageVersion( void );
static short kpdImageUpdatebyMain( void );
static short KpdMainProc( void );
static int ASC2HEX( byte *abSrcASC, byte *abDesHEX, byte bLengthSrc );

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      CommKpd                                                  *
*                                                                              *
*  DESCRIPTION:       Keypad Comm. Process Main .                              *
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
void CommKpd( void )
{
	short sRetVal = SUCCESS;
	bool boolIsLoadRouteParm = FALSE;
	static time_t tTime = 0;

	if ( tTime == 0 )
	{
		TimerStart( &tTime );
	}

	/*
	 * 사용하는 변수 초기화
	*/
	kpdInitEnvironment();
	
	while ( TRUE )
	{
		/*
		 * 노선별 파라미터 정보를 가져온다.
		 */
		if ( boolIsLoadRouteParm == FALSE )
		{
			if ( LoadRouteParm() == SUCCESS )
			{
				boolIsLoadRouteParm = TRUE;
			}
		}

		/*
		 * 운전자조작기 버전을 아직 가져오지 못한 경우에는 매 while문마다
		 * 버전 가져오기를 시도한다.
		 * - gpstSharedInfo->abKpdVer 변수는 0x00으로 초기화되므로
		 *   해당 변수의 모든 값이 0x00 이거나 '0'이면 아직 운전자조작기 버전을
		 *   가져오지 못한 것으로 추정
		 */
		if( IsAllZero( gpstSharedInfo->abKpdVer,
				sizeof( gpstSharedInfo->abKpdVer ) ) == TRUE ||
			IsAllASCZero( gpstSharedInfo->abKpdVer,
				sizeof( gpstSharedInfo->abKpdVer ) ) == TRUE )
		{
			getKpdImageVersion();
		}
		
		sRetVal = OpenKpd( 9600 );
		if ( sRetVal == SUCCESS )
		{
			/*
			 * 수신한 Image를 조작기에 전송함
			 */
			kpdImageUpdatebyMain();

			/*
			 * 조작기와의 통신 main부분
			 */
			KpdMainProc();
			CloseKpd();
		}

		/*
		 * LANCARD Signal Check
		 */
		if ( gpstSharedInfo->boolIsDriveNow == FALSE )
		{
			if ( TimerCheck( 10, tTime ) == 0 )
			{
				TimerStart( &tTime );
				CheckLancardSignal();
			}
		}

		usleep( 40000 );					// 40 ms sleep

		// 요금을 운전자조작기에 출력한 경우
		if ( gboolIsSendFare == TRUE )
		{
			gboolIsSendFare = FALSE;
			usleep( 670000 );				// 670 ms sleep
		}
	}
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      kpdInitEnvironment                                       *
*                                                                              *
*  DESCRIPTION:       Keypad Process구동시 데이타 초기화함                     *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS                                                  *
*                     - RTC Error                                              *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short kpdInitEnvironment(void)
{
	short sRetVal = SUCCESS;
	int	fdKpd = 0;
	byte abBuf[3] = {0, };
	byte abKpdImgVer[5] = {0, };

	memset( &gstKpdPool, 0x00, sizeof( KPDCOMM_POOL_DATA ) );
	memset( &gstRouteParm, 0x00, sizeof( TEMP_ROUTE_PARM ) );

	/*
	 * Polling 구조체 초기화
	 */
	gstKpdPool.bCommType = '0';
	gstKpdPool.abDriverDisplayData[0] = 0x25;
	gstKpdPool.abDriverDisplayData[1] = 0x00;
	memset( gstKpdPool.abLanCardStrength, 0,
		sizeof( gstKpdPool.abLanCardStrength ) );
	memset( gstKpdPool.abLanCardQuality, 0,
		sizeof( gstKpdPool.abLanCardQuality ) );

	/*
	 * 다운로드중 단말기가 재부팅되었을경우에는 운전자조작기 이미지를
	 * 운전자조작기에 전송
	 */
	memset( abBuf, 0x00, sizeof( abBuf ) );
	fdKpd = open( KPDAPPLY_FLAGFILE, O_RDONLY );
	if ( fdKpd >= 0 )
	{
		read( fdKpd, (void *)abBuf, 1 );
		close( fdKpd );
	}

	if ( abBuf[0] == '0' || abBuf[0] == '1' )
	{
		DebugOut( "================UPFlag In==============\n" );

		DisplayCommUpDownMsg( 1, 1 );

		if ( SUCCESS == OpenKpd(115200) )
		{
			memset( abKpdImgVer, 0x00, sizeof(abKpdImgVer) );
			
			gpstSharedInfo->boolIsKpdImgRecv = TRUE;
			sRetVal = SendNewImgProcess( abKpdImgVer );
			gpstSharedInfo->boolIsKpdImgRecv = FALSE;
			
			CloseKpd();

			DisplayCommUpDownMsg( 2, 1 );
		}
	}

	/*
	 * 첫시작시에는 운전자조작기의 버전을 확인후 프로그램 UpDate를 위함
	 */	
	gpstSharedInfo->boolIsKpdImgRecv = TRUE;
	
	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      getKpdImageVersion                                       *
*                                                                              *
*  DESCRIPTION:       운전자조작기에서 IMAGE VERISON을 획득함                  *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS                                                  *
*                     - Error                                                  *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static bool getKpdImageVersion( void )
{
	short sRetVal = 0;
	bool boolIsGetSuccess = FALSE;
	int nIdx = 0;
	
	/*
	 * 운전자조작기의 ID를 가져오기 위함
	 */
	OpenKpd( 9600 );
	for ( nIdx = 0; nIdx < 3; nIdx++ )
	{
		sRetVal =  ReceiveImgVerfromKpd( gpstSharedInfo->abKpdVer,
			gpstSharedInfo->abDriverOperatorID );
		if ( sRetVal == SUCCESS )
		{
			break;
		}
		usleep( 50000 );		// 0.5초 sleep
	}
	CloseKpd();

	if( sRetVal == SUCCESS )
		boolIsGetSuccess = TRUE;
	
	return boolIsGetSuccess;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      kpdImageUpdatebyMain                                     *
*                                                                              *
*  DESCRIPTION:       DCS에서 다운로드 받은 Driver Image를 조작기에 전송함     *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS                                                  *
*                     - Error                                                  *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short kpdImageUpdatebyMain( void )
{
	short sRetVal = 0;

	if ( gpstSharedInfo->boolIsKpdImgRecv == TRUE )
	{
		DebugOut( "KpdImg Download Start....\n" );

		/*
		 * KEYPAD IMAGE를 운전자조작기에 전송.
		*/
		sRetVal = SendNewImgAfterVerCheck();
		if( sRetVal == SUCCESS )
		{
			gpstSharedInfo->boolIsKpdImgRecv = FALSE;
		}
	}

	return sRetVal;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      KpdMainProc                                              *
*                                                                              *
*  DESCRIPTION:       운전자조작기와의 통신처리하는 Main함수                   *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS                                                  *
*                     - Error                                                  *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short KpdMainProc( void )
{
	short sRetVal = 0;
	byte bRecvCmd = 0;
	byte bSendCode = 0;
	bool boolIsSend2Kpd = 0;
	/*
	 * 운전자조작기와 통신하는 변수
	 */
	byte abRecvBuf[1024] = { 0, };

	bRecvCmd = 0;
	memset( abRecvBuf, 0x00, sizeof(abRecvBuf) );

	/*
	 * 조작기에 Polling
	 */
	sRetVal = KpdCommProcPolling( &bRecvCmd, abRecvBuf );
	boolIsSend2Kpd = FALSE;
	if ( KPDCMD_RECV(bRecvCmd) )
	{
		if ( gpstSharedInfo->boolIsKpdLock == TRUE )
		{
			bSendCode = NAK;
			SendKpd( &bSendCode, 1 );
		}
		else
		{
			boolIsSend2Kpd = TRUE;
		}
	}

	if ( boolIsSend2Kpd == TRUE )
	{
		if ( (bRecvCmd == KPDCMD_START_STOP) ||
			 (bRecvCmd == KPDCMD_SUBID_SET) )
		{
			bSendCode = ACK;
			SendKpd( &bSendCode, 1 );
			boolIsSend2Kpd = FALSE;
		}

		/*
		 * 운전자조작기에서 수신된 COMMAND Parsing
		*/
		sRetVal = CommKpdParseRecvData( bRecvCmd,
										abRecvBuf,
										strlen(abRecvBuf) );
		if( sRetVal >= 0 )
		{
			if ( boolIsSend2Kpd == TRUE )
			{
				bSendCode = ACK;
				SendKpd( &bSendCode, 1 );
				boolIsSend2Kpd = FALSE;
			}
		}
		else
		{
			if ( bRecvCmd != KPDCMD_SUBID_SET )
			{
				bSendCode = NAK;
				SendKpd( &bSendCode, 1 );
				boolIsSend2Kpd = FALSE;
			}
		}
	}

	if ( sRetVal == SUCCESS )
	{
		memset( gstKpdPool.abTotalFare, 0, 3 );
	}

	return SUCCESS;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SendNewImgAfterVerCheck                                  *
*                                                                              *
*  DESCRIPTION:       This program compares Kpd Image version & Send New Image *
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
short SendNewImgAfterVerCheck( void )
{
	short sRetVal = SUCCESS;
	char achKpdImgVer[5];
	bool boolIsUpdate = FALSE;

	KPDCMD_COMM_PKT	 stSendPacket;
	KPDCMD_COMM_PKT	 stRecvPacket;

	memset( &stSendPacket, 0x00, sizeof(KPDCMD_COMM_PKT) );
	memset( &stRecvPacket, 0x00, sizeof(KPDCMD_COMM_PKT) );

	/*
	 * 신 Image를 전송할것인지의 결정후 실제로 전송함
	*/
	boolIsUpdate = CheckUpdateYN();
	if ( boolIsUpdate == TRUE )
	{
		CloseKpd();
		sRetVal = OpenKpd( 115200 );
		if ( sRetVal >= 0 )
		{
			DisplayCommUpDownMsg( 1, 1 );
			memset( achKpdImgVer, 0x00, sizeof(achKpdImgVer) );
			sRetVal = SendNewImgProcess( achKpdImgVer );

			CloseKpd();
			DisplayCommUpDownMsg( 2, 1 );
		}

		OpenKpd( 9600 );
	}

	return sRetVal;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      KpdCommProcPolling                                       *
*                                                                              *
*  DESCRIPTION:       This program polls to Driver & receives Cmd 			   *
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
short KpdCommProcPolling( byte* bCmd, byte* pbRecvBuf )
{
	short sRetVal = 0;
	byte bRecvCode = 0;
	time_t tNowTime = 0;
	byte abNowTime[15] = { 0, };
//	byte abTotalFare[10] = { 0, };
	bool boolTemp = 0x00;
	static dword dwLEDCnt = 0;
	static byte bIsCityTourBus = 0;			// 시티투어버스여부
											// '0' : 시티투어버스 아님
											// '1' : 시티투어버스

	KPDCMD_COMM_PKT	 stSendPacket;
	KPDCMD_COMM_PKT	 stRecvPacket;

	memset( &stSendPacket, 0x00, sizeof(KPDCMD_COMM_PKT) );
	memset( &gstKpdPool, 0x00, sizeof(KPDCOMM_POOL_DATA) );

	// 시티투어버스여부 설정 - 최초 1회만 수행됨
	if ( bIsCityTourBus == 0 )
	{
		if ( gstVehicleParm.wTranspMethodCode == 132 )
		{
			bIsCityTourBus = '1';
		}
		else
		{
			bIsCityTourBus = '0';
		}
	}

	gstKpdPool.bRunKindCode = gstRouteParm.bRunKindCode;
	gstKpdPool.bGyeonggiIncheonRangeFareInputWay =
		gstRouteParm.bGyeonggiIncheonRangeFareInputWay;
	gstKpdPool.bDCSComm = gpstSharedInfo->boolIsKpdLock + 0x30;	// DCS통신중여부

	if ( IsAllZero( gpstSharedInfo->abTotalFare, 3 ) == FALSE )
	{
//		printf("[KpdCommProcPolling] 요금을 운전자조작기로 전송함\n");
		memcpy( gstKpdPool.abTotalFare, gpstSharedInfo->abTotalFare,
			sizeof(gpstSharedInfo->abTotalFare) );
		gboolIsSendFare = TRUE;
	}
	else
	{
		memset( gstKpdPool.abTotalFare, 0x00, sizeof( gstKpdPool.abTotalFare ) );
	}

/* 2005-11-24 0338버전에서는 적용하지않음
	if ( gpstSharedInfo->boolIsSetStationfromKpd == TRUE )
	{
		memcpy( gstKpdPool.abNowStationID, gpstSharedInfo->abKeyStationID,
				sizeof(gpstSharedInfo->abKeyStationID) );
		memcpy( gstKpdPool.abNowStationName, gpstSharedInfo->abKeyStationName,
				sizeof(gpstSharedInfo->abKeyStationName) );
	}
	else
	{
		memcpy( gstKpdPool.abNowStationID, gpstSharedInfo->abNowStationID,
				sizeof(gpstSharedInfo->abNowStationID) );
		memcpy( gstKpdPool.abNowStationName, gpstSharedInfo->abNowStationName,
				sizeof(gpstSharedInfo->abNowStationName) );
	}	*/

	memcpy( gstKpdPool.abNowStationID, gpstSharedInfo->abNowStationID,
			sizeof(gpstSharedInfo->abNowStationID) );
	memcpy( gstKpdPool.abNowStationName, gpstSharedInfo->abNowStationName,
			sizeof(gpstSharedInfo->abNowStationName) );

	memset( abNowTime, 0x00, sizeof(abNowTime) );
	tNowTime = gpstSharedInfo->tTermTime;
	if ( tNowTime <= 0 )
	{
		GetRTCTime( &tNowTime );
	}

	TimeT2ASCDtime( tNowTime, abNowTime );
	memcpy( gstKpdPool.abRTCTime, abNowTime, 14 );

	boolTemp = gpstSharedInfo->boolIsDriveNow;
	if ( boolTemp == TRUE )
	{
		/*
		 * 운행
		*/
		gstKpdPool.bDrivenEnterType = 0x80;

		boolTemp = gpstSharedInfo->boolIsGpsValid;
		if ( boolTemp != GPS_DATA_VALID )
		{
			dwLEDCnt++;
			if ( (dwLEDCnt % 10) == 0 )
			{
				gstKpdPool.bDrivenEnterType |= 0x08;
				dwLEDCnt = 0;
			}
		}
	}
	else
	{
		/*
		 * 운행아님
		*/
		gstKpdPool.bDrivenEnterType = 0x00;
		dwLEDCnt++;
		if ( (memcmp(gpstSharedInfo->abKpdVer, "0114", 4) > 0) &&
			(dwLEDCnt % 5) == 0 )
		{
			memcpy(gstKpdPool.abLanCardStrength,
				gpstSharedInfo->abLanCardStrength, 3);
			memcpy(gstKpdPool.abLanCardQuality,
				gpstSharedInfo->abLanCardQuality, 3);
			dwLEDCnt = 0;
		}
	}

	switch( gpstSharedInfo->bCardUserType )
	{
		case '1' :
			gstKpdPool.bDrivenEnterType |= 0x40;
			break;
		case '2' :
			gstKpdPool.bDrivenEnterType |= 0x20;
			break;
		case '3' :
			gstKpdPool.bDrivenEnterType |= 0x10;
			break;
		default:
			gstKpdPool.bDrivenEnterType &= 0x8f;
			break;
	}

	/*
	 * 환승여부
	*/
	boolTemp = gpstSharedInfo->boolIsXfer;
	if ( boolTemp == TRUE )
	{
		gstKpdPool.bDrivenEnterType |= 0x08;
	}
/*
	// GPS가 음영지역일경우에 운전자조작기에 DISPLAY함
	boolTemp = gpstSharedInfo->boolIsGpsValid;	// 무효한 GPS DATA
	if ( boolTemp != GPS_DATA_VALID )
	{
		dwLEDCnt++;
		switch( dwLEDCnt )
		{
			case 1 :
				gstKpdPool.bDrivenEnterType |= 0x40;
				break;
			case 2 :
				gstKpdPool.bDrivenEnterType |= 0x20;
				break;
			case 3 :
				gstKpdPool.bDrivenEnterType |= 0x10;
				break;
			case 4 :
				gstKpdPool.bDrivenEnterType |= 0x04;
				break;
			case 5 :
				gstKpdPool.bDrivenEnterType |= 0x08;
				break;
			default:
				gstKpdPool.bDrivenEnterType &= 0x8f;
				dwLEDCnt = 0;
				break;
		}
	}
*/

//	memcpy( abTotalFare, gpstSharedInfo->abTotalFare,
//		sizeof(gpstSharedInfo->abTotalFare) );

	// 시티투어버스여부
	gstKpdPool.bIsCityTourBus = bIsCityTourBus;

	stSendPacket.bCmd = 'P';
	memset( stSendPacket.abData, 0x00, sizeof(stSendPacket.abData) );
	memcpy( stSendPacket.abData, &gstKpdPool, sizeof(KPDCOMM_POOL_DATA) );
	stSendPacket.nDataSize = sizeof(KPDCOMM_POOL_DATA);

	gpstSharedInfo->boolIsXfer = 0x00;
	memset( gpstSharedInfo->abTotalFare, 0x00,
			sizeof(gpstSharedInfo->abTotalFare) );

	sRetVal = SendKpdPkt( &stSendPacket );
	if ( sRetVal != SUCCESS )
	{
		return sRetVal;
	}

	memset( &stRecvPacket, 0x00, sizeof(KPDCMD_COMM_PKT) );
	sRetVal = RecvKpdPkt( &stRecvPacket, 1000 );
	if ( (stRecvPacket.nDataSize == -1) &&
		 ((stRecvPacket.abData[0] == ACK) || (stRecvPacket.abData[0] == NAK)) )
	{
		return SUCCESS;
	}

	boolTemp = gpstSharedInfo->boolIsKpdLock;
	if ( boolTemp == TRUE )
	{
		bRecvCode = NAK;
		SendKpd( &bRecvCode, 1);

		return SUCCESS;
	}

	if ( (stRecvPacket.bCmd == KPDCMD_SUBID_SET) ||
		 (stRecvPacket.bCmd == KPDCMD_START_STOP) )
	{
		bRecvCode = ACK;
		SendKpd( &bRecvCode, 1);
	}

	*bCmd = stRecvPacket.bCmd;
	memcpy( pbRecvBuf, stRecvPacket.abData, stRecvPacket.nDataSize );

	return sRetVal;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      CommKpdParseRecvData                                     *
*                                                                              *
*  DESCRIPTION:       This program parses Cmd recevied from Driver             *
*                                                                              *
*  INPUT PARAMETERS:  byte* bCmd                                               *
*                     byte* pbRecvBuf                                          *
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
static short CommKpdParseRecvData( byte bCmd, byte* pbRecvBuf, word wRecvLen )
{
	short sRetVal = SUCCESS;
	int nIndex = 0;

	for ( nIndex = wRecvLen; nIndex > 0; nIndex-- )
	{
		pbRecvBuf[nIndex] = pbRecvBuf[nIndex - 1];
	}
	pbRecvBuf[0] = CMD_REQUEST;
	wRecvLen++;

	switch ( bCmd )
	{
		/*
		 * 승차단말기ID 및 LEAP패스워드 설정 - 운전자조작기 0118
		 */
		case KPDCMD_SET_TERMID_LEAPPWD_0118 :
			if ( gpstSharedInfo->boolIsDriveNow == TRUE ||
				 gpstSharedInfo->boolIsKpdLock == TRUE )
			{
				sRetVal = ERR_KEYPAD_IS_DRIVE_NOW;
			}
			else
			{
				sRetVal = SetTermIDLeapPwd0118( pbRecvBuf, wRecvLen );
			}
			break;

		/*
		 * 승차단말기ID 설정 - 운전자조작기 0118
		 */
		case KPDCMD_MAINID_SET :
			if ( gpstSharedInfo->boolIsReadyToDrive != TRUE ||
				 gpstSharedInfo->boolIsDriveNow == TRUE )
			{
				sRetVal = ERR_KEYPAD_IS_DRIVE_NOW;
			}
			else
			{
				sRetVal = SetMainTermID( pbRecvBuf, wRecvLen );
			}
			break;

		/*
		 * 승차단말기ID 및 LEAP패스워드 설정
		 */
		case KPDCMD_SET_TERMID_LEAPPWD :
			if ( gpstSharedInfo->boolIsDriveNow == TRUE ||
				 gpstSharedInfo->boolIsKpdLock == TRUE )
			{
				sRetVal = ERR_KEYPAD_IS_DRIVE_NOW;
			}
			else
			{
				sRetVal = SetTermIDLeapPwd( pbRecvBuf, wRecvLen );
			}
			break;

		/*
		 * 설치등록
		 */
		case KPDCMD_SETUP :
			if ( gpstSharedInfo->boolIsDriveNow == TRUE ||
				 gpstSharedInfo->boolIsKpdLock == TRUE ||
				 gpstSharedInfo->boolIsBLPLMergeNow == TRUE )
			{
				printf( "[CommKpdParseRecvData] 운행중 [%s] / 운전자조작기잠금 [%s] / BL/PL머지중 [%s] 이므로 처리 안됨\n",
					GetBoolString( gpstSharedInfo->boolIsDriveNow ),
					GetBoolString( gpstSharedInfo->boolIsKpdLock ),
					GetBoolString( gpstSharedInfo->boolIsBLPLMergeNow ) );
				sRetVal = ERR_KEYPAD_IS_DRIVE_NOW;
			}
			else
			{
				sRetVal = ResetupReq( pbRecvBuf, wRecvLen );
			}
			break;

		/*
		 * 시간 설정
		 */
		case KPDCMD_SET_TIME :
			return KpdSetTime( pbRecvBuf, wRecvLen );

		/*
		 * 시작, 종료
		 */
		case KPDCMD_START_STOP :
			if ( (gpstSharedInfo->boolIsReadyToDrive != TRUE) ||
				 (gpstSharedInfo->boolIsCardProc == TRUE) ||
				 (gpstSharedInfo->boolIsKpdLock == TRUE) )
			{
				sRetVal = ERR_KEYPAD_IS_DRIVE_NOW;
			}
			else if( wRecvLen == (DRIVER_ID_SIZE + 1) )
			{
				sRetVal = KpdStartStopReq( pbRecvBuf, wRecvLen );
			}
			else
			{
				sRetVal = ERR_KEYPAD_DRIVER_CMD_LENGTH;
			}
			break;

		/*
		 * 할증, 할증 취소
		*/
		case KPDCMD_EXTRA :
			return KpdExtra( pbRecvBuf, wRecvLen );

		/* 다인승 / 인천공항 요금 / 시외요금 */
//		case 0x16 :
//			return set0x16( pbRecvBuf, wRecvLen );

		/*
		 * 다인승취소
		*/
		case KPDCMD_CANCEL_MULTI_GETON :
			if ( (gpstSharedInfo->boolIsReadyToDrive != TRUE) ||
				 (gpstSharedInfo->boolIsCardProc == TRUE) ||
				 (gpstSharedInfo->boolIsDriveNow != TRUE) )
			{
				sRetVal = ERR_KEYPAD_IS_DRIVE_NOW;
			}
			else if( wRecvLen == (CANCEL_MULTI_GETON_SIZE + 1) )
			{
				sRetVal = CancelMultiEnt( pbRecvBuf, wRecvLen );
			}
			else
			{
				sRetVal = ERR_KEYPAD_DRIVER_CMD_LENGTH;
			}
			break;

		/*
		 * BL Checking
		*/
		case KPDCMD_BL_CHECK :
			sRetVal = KpdBLCheck( pbRecvBuf, wRecvLen );
			break;

		/*
		 * PL Checking
		*/
		case KPDCMD_PL_CHECK :
			sRetVal = KpdPLCheck( pbRecvBuf, wRecvLen );
			break;
		/*
		 * 다인승
		*/
		case KPDCMD_MULTI_GETON :
			if ( (gpstSharedInfo->boolIsReadyToDrive != TRUE) ||
				 (gpstSharedInfo->boolIsCardProc == TRUE) ||
				 (gpstSharedInfo->boolIsDriveNow != TRUE) )
			{
				sRetVal = ERR_KEYPAD_IS_DRIVE_NOW;
			}
			else if( wRecvLen == (RECEIPT_ALLCOUNT_SIZE + 1) )
			{
				sRetVal = MultiEnt( pbRecvBuf, wRecvLen );
			}
			else
			{
				sRetVal = ERR_KEYPAD_DRIVER_CMD_LENGTH;
			}
			break;

		/*
		 * 정류장 보정
		*/
		case KPDCMD_STATION_CORRECT :
			if ( (gpstSharedInfo->boolIsReadyToDrive != TRUE) ||
				 (gpstSharedInfo->boolIsCardProc == TRUE) ||
				 (gpstSharedInfo->boolIsDriveNow != TRUE) )
			{
				sRetVal = ERR_KEYPAD_IS_DRIVE_NOW;
			}
			else if( wRecvLen == (STATION_ID_CORRECT_SIZE + 1) )
			{
				sRetVal = StationCorrectReq( pbRecvBuf, wRecvLen );
			}
			else
			{
				sRetVal = ERR_KEYPAD_DRIVER_CMD_LENGTH;
			}
			break;
		/*
		 * 현금승차 영수증 출력
		*/
		case KPDCMD_CASHENT_RECEIPT_PRINT :
			if( (gpstSharedInfo->boolIsReadyToDrive != TRUE) ||
				(gpstSharedInfo->boolIsDriveNow != TRUE) )
			{
				sRetVal = ERR_KEYPAD_IS_DRIVE_NOW;
			}
			else if( wRecvLen == (RECEIPT_ALLCOUNT_SIZE + 1) )
			{
				sRetVal = CashEntReceiptPrintReq( pbRecvBuf, wRecvLen );
			}
			else
			{
				sRetVal = ERR_KEYPAD_DRIVER_CMD_LENGTH;
			}
			break;

		/*
		 * 하차단말기 ID SETTING
		*/
		case KPDCMD_SUBID_SET :
			if ( gpstSharedInfo->boolIsReadyToDrive != TRUE ||
				 gpstSharedInfo->boolIsDriveNow == TRUE )
			{
				sRetVal = ERR_KEYPAD_IS_DRIVE_NOW;
			}
			else if ( wRecvLen == SUBTERM_ID_COUNT_SIZE + 1 )
			{
				sRetVal = SubTermIDSetReq( pbRecvBuf, wRecvLen );
			}
			else
			{
				sRetVal = ERR_KEYPAD_DRIVER_CMD_LENGTH;
			}
			break;
		/*
		 * DCS SERVER IP SETTING
		 */
		case KPDCMD_SERVERIP_SET :
			if( (gpstSharedInfo->boolIsReadyToDrive != TRUE) ||
				(gpstSharedInfo->boolIsDriveNow == TRUE) )
			{
				sRetVal = ERR_KEYPAD_IS_DRIVE_NOW;
			}
			else if ( gpstSharedInfo->boolIsKpdLock == TRUE )
			{
				sRetVal = ERR_KEYPAD_IS_DCS_COMM;
			}
			else if( wRecvLen == (SERVER_IP_SIZE + 1) )
			{
				sRetVal = ServerIPSetting( pbRecvBuf, wRecvLen );
			}
			else
			{
				sRetVal = ERR_KEYPAD_DRIVER_CMD_LENGTH;
			}
			break;
		/*
		 * 승하차단말기정보및 운영정보 버전출력
		*/
		case KPDCMD_TERMINFO_PRINT :
			sRetVal = TermInfoPrintingReq2Main();
			break;

/* 2005-11-24 0338버전에서는 적용하지 않음
		case KPDCMD_CONFIRM_STATION_CORRECT	:
			pbRecvBuf[1] = 0x30;
			return KpdConfirmCancelStationCorrect( pbRecvBuf, 2 );
			break;

		case KPDCMD_CANCEL_STATION_CORRECT	:
			pbRecvBuf[1] = 0x31;
			return KpdConfirmCancelStationCorrect( pbRecvBuf, 2 );
			break;	*/
		case KPDCMD_CITY_TOUR_BUS_TICKET:
			if ( ( gpstSharedInfo->boolIsReadyToDrive != TRUE ) ||
				 ( gpstSharedInfo->boolIsCardProc == TRUE ) ||
				 ( gpstSharedInfo->boolIsDriveNow != TRUE ) )
			{
				printf( "[CommKpdParseRecvData] 시티투어티켓정보입력 - " );
				printf( "운행중아님\n" );
				sRetVal = ERR_KEYPAD_IS_DRIVE_NOW;
			}
			else if( wRecvLen == 8 )
			{
				sRetVal = CityTourTicketInput( pbRecvBuf, wRecvLen );
			}
			else
			{
				printf( "[CommKpdParseRecvData] 시티투어티켓정보입력 - " );
				printf( "레코드길이오류 : %d\n", wRecvLen );
				sRetVal = ERR_KEYPAD_DRIVER_CMD_LENGTH;
			}
			break;
		default :
			sRetVal = SUCCESS;
	}

	return sRetVal;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      CheckUpdateYN                                            *
*                                                                              *
*  DESCRIPTION:       This program checks whether or not Sending Keypad Image  *
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
static bool CheckUpdateYN( void )
{
	short sRetVal = 0;
	byte abKpdVer[5] = {0, };
	byte abFileVer[5] = {0, };
	byte abFileApplyDate[15] = { 0, };

	KPDCMD_COMM_PKT	 stSendPacket;
	KPDCMD_COMM_PKT	 stRecvPacket;

	sRetVal = GetImgFileApplyDatenVer( abFileVer, abFileApplyDate );

	/*
	 * UPDATE 할 운전자조작기 파일이 없으면 다음을 진행할필요없다.
	 */
	if( sRetVal != SUCCESS ) 
	{
		return FALSE;
	}
	
	// 무조건 1번 운전자조작기의 버전을 가져올 필요 있다.
	sRetVal = ReceiveImgVerfromKpd( abKpdVer,
		gpstSharedInfo->abDriverOperatorID );

	PrintlnASC( "[CheckUpdateYN] 이미지파일버전   : ", abFileVer, 4 );
	PrintlnASC( "[CheckUpdateYN] 운전자조작기버전 : ", abKpdVer, 4 );

	// 다음의 경우에 FALSE (운전자조작기 이미지 전송 안함) 리턴
	// 1. 버전 가져오기 함수 실행 결과 오류 발생
	// 2. 가져온 운전자조작기 버전이 모두 0x00
	// 3. 업데이트할 파일의 버전보다 운전자조작기의 버전이 낮거나 같은 경우
	if ( sRetVal != SUCCESS ||
		 IsAllZero( abKpdVer, 4 ) == TRUE ||
		 memcmp( abFileVer, abKpdVer, 4 ) <= 0 )
	{
		printf( "[CheckUpdateYN] 운전자조작기를 업데이트 하지 않음\n" );
		return FALSE;
	}

	/*
	 * Update 요청 주고받기
	 */
	memset( &stSendPacket, 0x00, sizeof(KPDCMD_COMM_PKT) );
	memset( &stRecvPacket, 0x00, sizeof(KPDCMD_COMM_PKT) );
	stSendPacket.bCmd = 'D';
	stSendPacket.nDataSize = 1;
	if ( SUCCESS != SendKpdPkt(&stSendPacket) ) return FALSE;
	if ( SUCCESS != RecvKpdPkt(&stRecvPacket, 3000) ) return FALSE;

	if ( (stRecvPacket.nDataSize == -1) &&
		(stRecvPacket.abData[0] == ACK) )
	{
		DebugOut("<SendNewImgAfterVerCheck>ACK Received\n");
	}
	else
	{
		DebugOut("<SendNewImgAfterVerCheck>NAK Received\n");
		return FALSE;
	}

	return TRUE;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      ReceiveImgVerfromKpd                                     *
*                                                                              *
*  DESCRIPTION:       This program receives Image Version from Keypad          *
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
short ReceiveImgVerfromKpd( byte *pbKpdVer, byte *abDriverOperatorID )
{
	short sRetVal = 0;
	byte bRecvCode = 0;

	KPDCMD_COMM_PKT	 stSendPacket;
	KPDCMD_COMM_PKT	 stRecvPacket;

	memset( &stSendPacket, 0x00, sizeof(KPDCMD_COMM_PKT) );
	memset( &stRecvPacket, 0x00, sizeof(KPDCMD_COMM_PKT) );

	stSendPacket.bCmd = 'V';
	stSendPacket.nDataSize = 1;
	sRetVal = SendKpdPkt( &stSendPacket );
	if ( sRetVal != SUCCESS )
	{
		printf( "[ReceiveImgVerfromKpd] SendKpdPkt() 실패\n" );
		return sRetVal;
	}

	memset( &stRecvPacket, 0x00, sizeof(KPDCMD_COMM_PKT) );
	sRetVal = RecvKpdPkt( &stRecvPacket, 3000 );
	if ( sRetVal != SUCCESS )
	{
		DebugOut( "[ReceiveImgVerfromKpd] RecvKpdPkt() 실패\n" );
		return sRetVal;
	}

	/*
	 * ACK 수신했을때
	 */
	if ( (stRecvPacket.nDataSize == -1) && (stRecvPacket.abData[0] == ACK) )
	{
		DebugOut( "[ReceiveImgVerfromKpd] ACK Received\n" );

		memset( &stRecvPacket, 0x00, sizeof(KPDCMD_COMM_PKT) );
		RecvKpdPkt( &stRecvPacket, 3000 );
		if ( stRecvPacket.bCmd == 'v' )
		{
			memset( pbKpdVer, 0x00, sizeof(pbKpdVer) );
			memcpy( pbKpdVer, stRecvPacket.abData, 4 );
			memcpy( gpstSharedInfo->abKpdVer, pbKpdVer,
				sizeof(gpstSharedInfo->abKpdVer) );

			PrintlnASC( "[ReceiveImgVerfromKpd] KPD 버전 : ", pbKpdVer, 4 );

			if ( stRecvPacket.nDataSize == 13 )
			{
				if ( IsAllZero( &stRecvPacket.abData[4], 9 ) == TRUE ||
					 IsAllASCZero( &stRecvPacket.abData[4], 9 ) == TRUE )
				{
					memset( abDriverOperatorID, 0x00, 9 );
				}
				else
				{
					memcpy( abDriverOperatorID, &stRecvPacket.abData[4], 9 );
				}
				PrintlnASC( "[ReceiveImgVerfromKpd] KPD ID   : ", abDriverOperatorID, 9 );
			}
			else
			{
				DebugOut( "[ReceiveImgVerfromKpd] KPD 버전 0118이 아닌 것으로 추정되며, 따라서 KPD ID 가져오지 못함\n" );
			}

			bRecvCode = ACK;
			SendKpd( &bRecvCode , 1 );
		}
		else
		{
			DebugOut( "[ReceiveImgVerfromKpd] 'v' 커맨드가 아님\n" );
			return ErrRet( ERR_KEYPAD_VERSION_RECEIVE );
		}

		return SUCCESS;
	}

	return ErrRet( ERR_KEYPAD_VERSION_RECEIVE );
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SendNewImgProcess                                        *
*                                                                              *
*  DESCRIPTION:       This program sends New Image to Keypad                   *
*                                                                              *
*  INPUT PARAMETERS:  char* pchKpdImgVer                                       *
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
short SendNewImgProcess( char* pchKpdImgVer )
{
	short sRetVal = SUCCESS;

	byte abSendBuf[128] = {0, };
	byte abRecvBuf[128] = {0, };

	char* pchFileBuf = NULL;

	int nReadByte = 0;
	int nSendByte = 0;
	int nIndex = 0;

	int fdKpdImgFlag = 0;
	FILE* fileKpdImg = NULL;
	size_t sizeFileBufSize = 128;

	printf( "[SendNewImgProcess] 운전자조작기 이미지 전송 시작\n" );

	sRetVal = RecvKpd( abRecvBuf, 1, 5000 );
	if ( sRetVal != SUCCESS )
	{
		printf( "[SendNewImgProcess] 운전자조작기 'N' 수신 실패\n" );
		return sRetVal;
	}

	if ( abRecvBuf[0] != 'N' )
	{
		printf( "[SendNewImgProcess] 운전자조작기 수신 바이트가 'N'이 아님\n" );
		return ErrRet( ERR_KEYPAD_CMD );
	}

	strcpy( abSendBuf, "Ddwn" );
	sRetVal = SendKpd( abSendBuf, 4 );
	if ( sRetVal != SUCCESS )
	{
		printf( "[SendNewImgProcess] 운전자조작기 'Ddwn' 수신 실패\n" );
		return sRetVal;
	}

	for ( nIndex = 0; nIndex < 10; nIndex++ )
	{
		memset( abRecvBuf, 0, sizeof( abRecvBuf ) );
		sRetVal = RecvKpd( abRecvBuf, 2, 1000 );
		if ( sRetVal == SUCCESS &&
			 abRecvBuf[0] == 0 &&
			 abRecvBuf[1] == 0 )
		{
			DebugOut( "[SendNewImgProcess] 운전자조작기 'Ddwn' 응답 수신\n" );
			break;
		}
	}
	// 운전자조작기 'Ddwn' 응답 수신에 실패한 경우
	if ( nIndex >= 10 )
	{
		printf( "[SendNewImgProcess] 운전자조작기 'Ddwn' 응답 수신 실패\n" );
		return ErrRet( ERR_KEYPAD_ACK );
	}

	fdKpdImgFlag = open( KPDAPPLY_FLAGFILE, O_WRONLY | O_CREAT | O_TRUNC,
		OPENMODE );
	if ( fdKpdImgFlag < 0 )
	{
		printf( "[SendNewImgProcess] 'driverdn.cfg' open 실패\n" );
		ErrRet( ERR_KEYPAD_FLAGFILE_OPEN );
	}
	else
	{
		flock( fdKpdImgFlag, LOCK_EX );
		abSendBuf[0] = '0';
		write( fdKpdImgFlag, abSendBuf, 1 );
		flock( fdKpdImgFlag, LOCK_UN );
		close( fdKpdImgFlag );
	}

	// Open & Read & Send New KeypadImage File
	fileKpdImg = fopen( KPD_IMAGE_FILE, "r" );
	if ( fileKpdImg == NULL )
	{
		printf( "[SendNewImgProcess] 'c_dr_pro.dat' open 실패\n" );
		return ErrRet( ERR_KEYPAD_IMAGEFILE_OPEN );
	}

	flock( (int)fileKpdImg, LOCK_EX );

	DebugOut( "nIndex=%d...\n", nIndex );

	pchFileBuf = (byte*)malloc( sizeFileBufSize );
	while ( TRUE )
	{
		memset( pchFileBuf, 0, sizeof(pchFileBuf) );
		memset( abSendBuf, 0, sizeof(abSendBuf) );

		nReadByte = getline( &pchFileBuf, &sizeFileBufSize, fileKpdImg );
		if ( nReadByte < 0 )
		{
			printf( "[SendNewImgProcess] getline() 실패\n" );
			sRetVal = ERR_KEYPAD_IMAGEFILE_READ;
			break;
		}

		if ( pchFileBuf[1] == '0' )
		{
			continue;
		}
		else if ( pchFileBuf[1] == '9' )
		{
			abSendBuf[0] = 'E';
			sRetVal = SendKpd( abSendBuf, 1 );
			if ( sRetVal != SUCCESS )
			{
				break;
			}
		}
		else
		{
			nSendByte = ASC2HEX( pchFileBuf + 2, abSendBuf + 1, nReadByte - 4 );
			abSendBuf[0] = 'B';
			sRetVal = SendKpd( abSendBuf, nSendByte + 1 );
			if ( sRetVal != SUCCESS )
			{
				break;
			}
			DebugOut( "<KPD>nIndex=%d, ImageData=%s.\n", nIndex++, abSendBuf );
		}

		sRetVal = RecvKpd( abRecvBuf, 1, 1000 );
		if ( sRetVal != SUCCESS )
		{
			break;
		}

		if ( abRecvBuf[0] != 0x00 )
		{
			printf( "[SendNewImgProcess] 운전자조작기 ACK 오류\n" );
			sRetVal = ERR_KEYPAD_ACK;
			break;
		}

		if ( pchFileBuf[1] == '9' )
		{
			sRetVal = SUCCESS;
			break;
		}
	}

	free( pchFileBuf );
	flock(fdKpdImgFlag, LOCK_UN);
	fclose( fileKpdImg );

	if ( sRetVal == SUCCESS )
	{
		unlink( KPDAPPLY_FLAGFILE );
		return SUCCESS;
	}

	if ( sRetVal != SUCCESS )
	{
		printf( "[SendNewImgProcess] 운전자조작기 이미지 전송 실패 (%x)\n",
			sRetVal );
	}
	else
	{
		printf( "[SendNewImgProcess] 운전자조작기 이미지 전송 완료\n" );
	}

	return ErrRet( sRetVal );
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SendKpdPkt                                               *
*                                                                              *
*  DESCRIPTION:       Send Packets to Keypad                                   *
*                                                                              *
*  INPUT PARAMETERS:  KPDCMD_COMM_PKT* pstSendData                             *
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
short SendKpdPkt( KPDCMD_COMM_PKT* pstSendData )
{
	short sRetVal = 0;
	int nIndex = 0;

	byte* pbSendBuf = NULL;
	word wSendBufLength = 0;
	byte bBCC = 0;

	wSendBufLength = pstSendData->nDataSize + 5;
	pbSendBuf = (byte*)malloc( wSendBufLength + 1 );
	memset( pbSendBuf, 0x00, wSendBufLength + 1 );
	pbSendBuf[0] = STX;
	pbSendBuf[1] = pstSendData->nDataSize + 1;
	pbSendBuf[2] = pstSendData->bCmd;
	memcpy( pbSendBuf + 3, pstSendData->abData, pstSendData->nDataSize );
	pbSendBuf[wSendBufLength - 2] = ETX;
	for ( nIndex = 0; nIndex < wSendBufLength - 1 ; nIndex++ )
	{
		bBCC ^= pbSendBuf[nIndex];
	}
	pbSendBuf[wSendBufLength - 1] = bBCC;

	sRetVal = SendKpd( pbSendBuf, wSendBufLength );
	if ( sRetVal != SUCCESS )
	{
		free( pbSendBuf );
		return ErrRet( sRetVal );
	}

	free( pbSendBuf );

	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      RecvKpdPkt                                               *
*                                                                              *
*  DESCRIPTION:       Receive Packets to Keypad                                *
*                                                                              *
*  INPUT PARAMETERS:  KPDCMD_COMM_PKT* pstRecvData                             *
*                     int nTimeOut                                             *
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
static short RecvKpdPkt( KPDCMD_COMM_PKT* pstRecvData, int nTimeOut )
{
	short sRetVal = 0;

	byte bTempBuf[2];
	byte* pbRecvBuf = NULL;
	byte bBCC = 0;

	int nRecvLength = 0;
	int nIndex = 0;

	memset( bTempBuf, 0x00, sizeof(bTempBuf) );
	sRetVal = RecvKpd( bTempBuf, 1, nTimeOut );
	if ( sRetVal != SUCCESS )
	{
		return ErrRet( sRetVal );
	}

	if ( (bTempBuf[0] == ACK) || (bTempBuf[0] == NAK) )
	{
		pstRecvData->abData[0] = bTempBuf[0];
		pstRecvData->nDataSize = -1;
		return SUCCESS;
	}

	sRetVal = RecvKpd( bTempBuf + 1, 1, nTimeOut );
	if ( sRetVal != SUCCESS )
	{
		return ErrRet( sRetVal );
	}

	nRecvLength = bTempBuf[1] + 4;
	pbRecvBuf = (byte*)malloc( nRecvLength );
	memcpy( pbRecvBuf, bTempBuf, 2 );

	sRetVal = RecvKpd( pbRecvBuf + 2, nRecvLength - 2, nTimeOut );
	if ( sRetVal != SUCCESS )
	{
		free( pbRecvBuf );
		return ErrRet( sRetVal );
	}

	if ( pbRecvBuf[0] != STX )
	{
		free( pbRecvBuf );
		return ErrRet( ERR_KEYPAD_STX );
	}

	if ( pbRecvBuf[nRecvLength - 2] != ETX )
	{
		free( pbRecvBuf );
		return ErrRet( ERR_KEYPAD_ETX );
	}

	for ( nIndex = 0; nIndex < nRecvLength - 1 ; nIndex++ )
	{
		bBCC ^= pbRecvBuf[nIndex];
	}

	pstRecvData->bCmd = pbRecvBuf[2];
	pstRecvData->nDataSize = nRecvLength - 5;
	memcpy( pstRecvData->abData, pbRecvBuf + 3, pstRecvData->nDataSize );

	free( pbRecvBuf );

	return SUCCESS;
}



/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SendKpd                                                  *
*                                                                              *
*  DESCRIPTION:       Send byte to Keypad                                      *
*                                                                              *
*  INPUT PARAMETERS:  byte* pbSendData                                         *
*                     word wSendSize                                           *
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
short SendKpd( byte* pbSendData, word wSendSize )
{
	short sRetVal = 0;
	static char achMsg[50];

	tcflush( gfdKpdDevice, TCIOFLUSH );
	sRetVal = write( gfdKpdDevice, pbSendData, wSendSize );
	if ( sRetVal < wSendSize )
	{
		DebugOut( achMsg, "Error!!! Keypad Write() failed\n" );
		DebugOutASC( achMsg, strlen(achMsg) );

		return ErrRet( ERR_KEYPAD_WRITE );
	}

	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      RecvKpd                                                  *
*                                                                              *
*  DESCRIPTION:       Receive byte from Keypad                                 *
*                                                                              *
*  INPUT PARAMETERS:  byte* pbRecvData                                         *
*                     word wRecvSize                                           *
*                     int nTimeOut                                             *
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
static short RecvKpd( byte* pbRecvData, word wRecvSize, int nTimeOut )
{
	int nSelectRet = 0;
	int nReadByte = 0;
	int nReadTotalByte = 0;

	byte* pbRecvDataPtr = NULL;

	fd_set stSocket;
	struct timeval stTimeOut;

	pbRecvDataPtr = pbRecvData;

	stTimeOut.tv_sec = nTimeOut / 1000;
	stTimeOut.tv_usec = (nTimeOut % 1000) * 1000;

	FD_ZERO( &stSocket );
	FD_SET( gfdKpdDevice, &stSocket );
	while ( 1 )
	{
		nSelectRet = select(gfdKpdDevice+1, &stSocket, NULL, NULL, &stTimeOut);
		switch ( nSelectRet )
		{
			case -1 :
				DebugOut( "Error!!! select() failed\n" );
				return ErrRet( ERR_KEYPAD_SELECT );
			case 0 :
				DebugOut( "Error!!!  Keypad Timeout\n" );
				return ErrRet( ERR_KEYPAD_TIMEOUT );
		}

		nReadByte = read(gfdKpdDevice, pbRecvDataPtr, wRecvSize-nReadTotalByte);
		if ( nReadByte < 0 )
		{
			DebugOut( "Error!!! read() failed\n" );
			return ErrRet( ERR_KEYPAD_READ );
		}

		nReadTotalByte += nReadByte;
		pbRecvDataPtr += nReadByte;
		if ( nReadTotalByte == wRecvSize )
		{
			break;
		}
	}

	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      OpenKpd                                                  *
*                                                                              *
*  DESCRIPTION:       Open Keypad Communication Channel						   *
*                                                                              *
*  INPUT PARAMETERS:  dword wBaudrate                                          *
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
static short OpenKpd( dword wBaudrate )
{
	struct termios stOptions;

	gfdKpdDevice = open( "/dev/ttyE0", O_RDWR | O_NOCTTY );
	if( gfdKpdDevice < 0 )
	{
		DebugOut( "/dev/ttyE0 open error\n" );
		return ErrRet( ERR_KEYPAD_OPEN );
	}

	tcgetattr( gfdKpdDevice, &stOptions );

	memset( &stOptions, 0, sizeof(struct termios) );

	stOptions.c_cflag = CS8 | CLOCAL | CREAD;

	switch( wBaudrate )
	{
		case 9600 :
			stOptions.c_cflag |= B9600;
			break;
		case 115200 :
			stOptions.c_cflag |= B115200;
			break;
	}

	/*
	 * 20; 20*0.1 = 2sec rx time out
	*/
	stOptions.c_cc[VTIME] = 10;
	stOptions.c_cc[VMIN] = 1;


	tcflush( gfdKpdDevice, TCIFLUSH );
	tcsetattr( gfdKpdDevice, TCSANOW , &stOptions );

	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      CloseKpd                                                 *
*                                                                              *
*  DESCRIPTION:       Open Keypad Communication Channel						   *
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
static void CloseKpd( void )
{
	close( gfdKpdDevice );
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      ASC2HEX                                                  *
*                                                                              *
*  DESCRIPTION:       Changes ASC Data to HEX                    			   *
*                                                                              *
*  INPUT PARAMETERS:  byte *abSrcASC                                           *
*                     byte *abDesHEX                                           *
*                     byte bLengthSrc                                          *
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
static int ASC2HEX( byte *abSrcASC, byte *abDesHEX, byte bLengthSrc )
{
	byte* pbSourASC = abSrcASC;
	byte* pbDestHEX = abDesHEX;
	byte bTemp;

	int nIndex = 0;
	while ( nIndex < bLengthSrc )
	{
		bTemp = 0;
		if (*pbSourASC >= '0' && *pbSourASC <= '9')
		{
			bTemp = *pbSourASC - '0';
		}
		else if (*pbSourASC >= 'A' && *pbSourASC <= 'F')
		{
			bTemp = *pbSourASC - 'A' + 10;
		}
		else if (*pbSourASC >= 'a' && *pbSourASC <= 'f')
		{
			bTemp = *pbSourASC - 'a' + 10;
		}

		if (nIndex % 2 == 0)
		{
			*pbDestHEX = bTemp << 4;
		}
		else
		{
			*pbDestHEX |= bTemp;
			pbDestHEX++;
		}

		nIndex++;
		pbSourASC++;
	}

	return ( ((nIndex - 1) / 2) + 1 );
}

