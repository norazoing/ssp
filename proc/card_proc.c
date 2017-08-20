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
*  PROGRAM ID :       card_proc.c                                              *
*                                                                              *
*  DESCRIPTION:       카드의 태그여부 체크에서부터 처리결과 출력까지 카드처리와*
*                     관련된 모든 사항을 처리한다.                             *
*                                                                              *
*  ENTRY POINT:       void CardProc( void )                                    *
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
* 2005/07/13 Solution Team  Boohyeon Jeon Initial Release                      *
*                                                                              *
*******************************************************************************/

#include "../system/bus_type.h"
#include "../system/device_interface.h"
#include "main.h"
#include "trans_proc.h"
#include "card_proc_mif_prepay.h"
#include "card_proc_mif_postpay.h"
#include "card_proc_mif_tour.h"
#include "card_proc_sc.h"
#include "card_proc_util.h"
#include "write_trans_data.h"

#include "card_proc.h"

#define KEYPAD_PRINT_0_WON				"000001"

/*******************************************************************************
*  Declaration of Function Prototypes                                          *
*******************************************************************************/
static void DisplayResult( TRANS_INFO *pstTransInfo, short sErrCode );
static void DisplaySuccess( TRANS_INFO *pstTransInfo );
static void DisplayError( TRANS_INFO *pstTransInfo, short sErrCode );

/*******************************************************************************
*  Declaration of Global Valiables                                             *
*******************************************************************************/
word gwRetagCardCnt = 0;				// 카드 재태그 오류 횟수

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       CardProc                                                 *
*                                                                              *
*  DESCRIPTION:       카드의 태그여부 체크에서부터 처리결과 출력까지 카드처리와*
*                     관련된 모든 사항을 처리한다.                             *
*                                                                              *
*  INPUT PARAMETERS:  void                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: void                                                     *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-10                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
void CardProc( void )
{
	short	sResult		= SUCCESS;	// 함수 실행 결과
	byte	bCardType	= 0;		// 카드 유형
	time_t	tNowDtime	= 0;		// 현재 시간
	TRANS_INFO stTransInfo;			// 카드정보구조체

	static int nTempCnt = 0;

	// 카드정보구조체 초기화
	memset( ( byte * )&stTransInfo, 0, sizeof( TRANS_INFO ) );

	// 신규환승정보의 이용 시간에 현재시간 설정
	GetRTCTime( &tNowDtime );
	stTransInfo.stNewXferInfo.tEntExtDtime = tNowDtime;

#ifdef TEST_PRINT_CARD_PROC_TIME
	InitWatch();
	StopWatch();
#endif

	// 카드유형판별 ////////////////////////////////////////////////////////////
	sResult = GetCardType( &bCardType, &stTransInfo.dwChipSerialNo );
	if (sResult == ERROR_CARD_NOTAG)
	{
		if (gboolIsMainTerm)
		{
			if (nTempCnt == 0)
			{
				DisplayDWORDInDownFND( 0 );
			}
	
			if (nTempCnt > 15)
			{
				DisplayASCInDownFND("");
				nTempCnt = 0;
			}
			else
			{
				nTempCnt++;
			}
		}
		return;
	}

	if ( sResult != SUCCESS )
	{
		switch ( sResult )
		{
			case ERROR_CARD_NOTAG:
				return;
			case ERROR_NOT_ONECARD:
				sResult = ERR_CARD_PROC_NOT_ONE_CARD;
				break;
			case ERROR_CANNOT_USE:
				sResult = ERR_CARD_PROC_CANNOT_USE;
				break;
			default:
				sResult = ERR_CARD_PROC_RETAG_CARD;
				break;
		}
		goto DISPLAY;
	}

#ifdef TEST_PRINT_CARD_PROC_TIME
	printf( "[CardProc] 카드유형 판별 : %f sec\n", StopWatch() );
#endif

	// 카드처리가 시작되면 운전자조작기에서 다인승 설정이 불가능하도록 플래그
	// 변수를 TRUE로 설정
	gpstSharedInfo->boolIsCardProc = TRUE;

	// 카드READ ////////////////////////////////////////////////////////////////
	switch ( bCardType )
	{
		case ( CARDTYPE_MIF | CARDTYPE_PREPAY ):
			sResult = MifPrepayRead( &stTransInfo );
			break;
		case ( CARDTYPE_MIF | CARDTYPE_POSTPAY ):
			sResult = MifPostpayRead( &stTransInfo );
			break;
		case ( CARDTYPE_MIF | CARDTYPE_TOUR ):
			sResult = MifTourRead( &stTransInfo );
			break;
		case CARDTYPE_SC:
			sResult = SCRead( &stTransInfo );
			break;
		default:
			sResult = ERR_CARD_PROC_CANNOT_USE;
			goto DISPLAY;
	}

#if defined( RELEASEMODE ) && defined( TEST_PRINT_CARD_PROC_INFO )
	if ( strlen( stTransInfo.abCardNo ) != 0 &&
		 sResult != ERR_CARD_PROC_NOT_APPROV )
	{
		PrintlnASC( "\n\n## 카드번호 : ", stTransInfo.abCardNo,
			sizeof( stTransInfo.abCardNo ) );
		printf( "\n## 이전환승정보 ##\n" );
		PrintXferInfo( &stTransInfo.stPrevXferInfo );
	}
#endif
	if ( sResult == ERR_CARD_PROC_LOG )
	{
	    printf( "[CardProc] 오류LOG의 카드정보를 재처리함\n" );
	    goto WRITE;
	}
	if ( sResult != SUCCESS )
	{
		goto DISPLAY;
	}

	// 교통수단별 사용불가카드 처리 ////////////////////////////////////////////
	// 시티투어버스
	if ( IsCityTourBus( gstVehicleParm.wTranspMethodCode ) == TRUE )
	{
		// 시티투어버스이면서 신선불/관광권카드가 아닌 경우
		// '사용할 수 없는 카드입니다' 음성 출력
		if ( stTransInfo.bCardType != TRANS_CARDTYPE_SC_PREPAY &&
			 stTransInfo.bCardType != TRANS_CARDTYPE_MIF_TOUR )
		{
			printf( "[CardProc] 시티투어버스이면서 신선불/관광권카드가 아닌 " );
			printf( "경우 '사용할 수 없는 카드입니다' 음성 출력\n" );
			sResult = ERR_CARD_PROC_CANNOT_USE;
			goto DISPLAY;
		}

		// 시티투어버스이면서 신선불카드인데도 요금입력이 되지 않은 경우
		// '승차권을 선택해 주세요' 음성 출력
		if ( stTransInfo.bCardType == TRANS_CARDTYPE_SC_PREPAY &&
			 gstCityTourBusTicketInfo.boolIsTicketInput == FALSE )
		{
			printf( "[CardProc] 시티투어버스이면서 신선불카드인데도 " );
			printf( "요금입력이 되지 않은 경우 '승차권을 선택해 주세요' " );
			printf( "음성 출력\n" );
			sResult = ERR_CARD_PROC_INPUT_TICKET;
			goto DISPLAY;
		}

		// 시티투어버스이면서 요금입력이 된 상태로 관광권카드가 태그되는 경우
		// '시티패스입니다' 음성 출력
		if ( stTransInfo.bCardType == TRANS_CARDTYPE_MIF_TOUR &&
			 gstCityTourBusTicketInfo.boolIsTicketInput == TRUE )
		{
			printf( "[CardProc] 시티투어버스이면서 요금입력이 된 상태로 " );
			printf( "관광권카드가 태그되는 경우 '시티패스입니다' " );
			printf( "음성 출력\n" );
			sResult = ERR_CARD_PROC_CITY_PASS_CARD;
			goto DISPLAY;
		}

	}
	// 광역버스
	else if ( IsWideAreaBus( gstVehicleParm.wTranspMethodCode ) == TRUE )
	{
		// 광역버스이면서 관광권카드인 경우
		// '사용할 수 없는 카드입니다' 음성 출력
		if ( stTransInfo.bCardType == TRANS_CARDTYPE_MIF_TOUR )
		{
			printf( "[CardProc] 광역버스이면서 관광권카드인 경우 " );
			printf( "'사용할 수 없는 카드입니다' 음성 출력\n" );
			sResult = ERR_CARD_PROC_CANNOT_USE;
			goto DISPLAY;
		}
	}

	// 다인승정보 설정 /////////////////////////////////////////////////////////
	// 시티투어버스
	if ( IsCityTourBus( gstVehicleParm.wTranspMethodCode ) == TRUE )
	{
		if ( gstCityTourBusTicketInfo.boolIsTicketInput == TRUE )
		{
			// 다인승정보 복사
			memcpy( stTransInfo.abUserType, gstCityTourBusTicketInfo.abUserType,
				sizeof( gstCityTourBusTicketInfo.abUserType ) );
			memcpy( stTransInfo.abUserCnt, gstCityTourBusTicketInfo.abUserCnt,
				sizeof( gstCityTourBusTicketInfo.abUserCnt ) );
		}
		else
		{
			// PL사용자유형을 설정하고, 사용자 수를 1명으로 설정
			stTransInfo.abUserType[0] = stTransInfo.bPLUserType;
			stTransInfo.abUserCnt[0] = 1;
		}
	}
	else
	{
		// PL사용자유형은 카드READ에서 가져오므로 카드READ 완료 후 설정함
		// 관광권카드의 경우
		if ( stTransInfo.bCardType == TRANS_CARDTYPE_MIF_TOUR )
		{
			if ( gstMultiEntInfo.boolIsMultiEnt == TRUE &&
				 ( gstMultiEntInfo.abUserCnt[0] +
				   gstMultiEntInfo.abUserCnt[1] +
				   gstMultiEntInfo.abUserCnt[2] ) != 1 )
			{
				printf( "[CardProc] 관광권카드의 경우 2인 이상의 다인승이 " );
				printf( "설정되었으므로 '다인승이 불가능한 카드입니다' 음성 " );
				printf( "출력\n" );
				sResult = ERR_CARD_PROC_CANNOT_MULTI_ENT;
				goto DISPLAY;
			}
			// 승객정보 초기화
			memset( stTransInfo.abUserType, 0, sizeof( stTransInfo.abUserType ) );
			memset( stTransInfo.abUserCnt, 0, sizeof( stTransInfo.abUserCnt ) );

			stTransInfo.abUserType[0] = stTransInfo.bPLUserType;
			stTransInfo.abUserCnt[0] = 1;
		}
		// 다인승입력의 경우
		else if ( gstMultiEntInfo.boolIsMultiEnt == TRUE )
		{
			// 다인승정보 복사
			memcpy( stTransInfo.abUserType, gstMultiEntInfo.abUserType,
				sizeof( stTransInfo.abUserType ) );
			memcpy( stTransInfo.abUserCnt, gstMultiEntInfo.abUserCnt,
				sizeof( stTransInfo.abUserCnt ) );
		}
		// 다인승미입력의 경우
		else
		{
			// PL사용자유형을 설정하고, 사용자 수를 1명으로 설정
			stTransInfo.abUserType[0] = stTransInfo.bPLUserType;
			stTransInfo.abUserCnt[0] = 1;
		}
	}

	PrintTransInfo( &stTransInfo );

	// 거래처리 ////////////////////////////////////////////////////////////////
	sResult = TransProc( &stTransInfo );

	// 추가승차여부 백업 ///////////////////////////////////////////////////////
	// 1. WRITE시 오류가 발생하여 다시 태그하면 백업된 추가승차여부 플래그를
	//    사용함 ( 공유메모리에도 백업한 내용을 리스토어함 )
	// 2. TD WRITE시 추가승차여부 플래그 설정에 사용함
	//    ( 하차/승차 페어중 하차의 경우에만 )
	stTransInfo.boolIsAddEnt = gstMultiEntInfo.boolIsAddEnt;

	// 다인승여부 백업 /////////////////////////////////////////////////////////
	stTransInfo.boolIsMultiEnt = gstMultiEntInfo.boolIsMultiEnt;
	if ( sResult != SUCCESS )
	{
		goto DISPLAY;
	}

	WRITE:

#ifdef TEST_WRITE_SLEEP
	printf( "TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT\n" );
	printf( "WRITE START\n" );
	sleep( 2 );
#endif

	// 카드WRITE ///////////////////////////////////////////////////////////////
	switch ( bCardType )
	{
		case ( CARDTYPE_MIF | CARDTYPE_PREPAY ):
			sResult = MifPrepayWrite( &stTransInfo );
			break;
		case ( CARDTYPE_MIF | CARDTYPE_POSTPAY ):
			sResult = MifPostpayWrite( &stTransInfo );
			break;
		case ( CARDTYPE_MIF | CARDTYPE_TOUR ):
			sResult = MifTourWrite( &stTransInfo );
			break;
		case CARDTYPE_SC:
			sResult = SCWrite( &stTransInfo );
			break;
		default:
			sResult = ERR_CARD_PROC_CANNOT_USE;
			goto DISPLAY;
	}

#if defined( RELEASEMODE ) && defined( TEST_PRINT_CARD_PROC_INFO )
	printf( "\n## 신규환승정보 ##\n" );
	PrintXferInfo( &stTransInfo.stNewXferInfo );
#endif

	PrintTransInfo( &stTransInfo );
	if ( sResult != SUCCESS )
	{
		goto DISPLAY;
	}

#ifdef TEST_PRINT_CARD_PROC_TIME
	printf( "[CardProc] 카드처리 완료 : %f sec\n\n", StopWatch() );
#endif

	// 추가승차여부/다인승여부/다인승정보 전역변수 복구 ////////////////////////
	// 0. WRITE오류로 인하여 복구된 경우
	if ( stTransInfo.bWriteErrCnt > 0 )
	{
		// 1. 백업된 추가승차여부가 TRUE이며
		// 2. 백업된 다인승여부가 TRUE이고
		// 2. 추가승차로 인한 ( 하차|승차 ) 페어 중 하차를 복구하는 경우
		// -> 추가승차여부 복원 및 다인승정보 입력 시간 갱신
		if ( stTransInfo.boolIsAddEnt &&
			 stTransInfo.boolIsMultiEnt &&
			 !IsEnt( stTransInfo.stNewXferInfo.bEntExtType ) )
		{
			DebugOut( "[CardProc] 추가승차여부 전역변수를 복구함\n" );

			gstMultiEntInfo.boolIsAddEnt = TRUE;		// 추가승차여부

			// main()으로부터의 타임아웃으로 인한 다인승정보 초기화를 방지하기
			// 위하여 다인승정보 입력 시간을 현재시간으로 설정
			gstMultiEntInfo.tInputDatetime = gpstSharedInfo->tTermTime;
		}

		// 1. 백업된 다인승여부가 TRUE인 경우
		// -> 다인승여부/다인승정보 복원
		if ( stTransInfo.boolIsMultiEnt )
		{
			byte i = 0;

			DebugOut( "[CardProc] 다인승여부/다인승정보 관련 전역변수를 " );
			DebugOut( "복구함\n" );

			gstMultiEntInfo.boolIsMultiEnt = TRUE;		// 다인승여부

			// 다인승정보
			for ( i = 0; i < 3; i++ )
			{
				gstMultiEntInfo.abUserType[i] = stTransInfo.abUserType[i];
				gstMultiEntInfo.abUserCnt[i] = stTransInfo.abUserCnt[i];
			}
		}
	}

	// 거래내역기록 ////////////////////////////////////////////////////////////
	WriteTransData( &stTransInfo );
LogMain("거래내역성공\n");
	// 카드번호LOG에 카드번호 추가 /////////////////////////////////////////////
	AddCardNoLog( stTransInfo.abCardNo );

	DISPLAY:

	// 카드처리가 완료되면 운전자조작기에서 다인승 설정이 가능하도록 플래그
	// 변수를 FALSE로 설정
	gpstSharedInfo->boolIsCardProc = FALSE;

	// 처리결과출력 ////////////////////////////////////////////////////////////
	DisplayResult( &stTransInfo, sResult );

	// 시티투어버스 승차권입력정보를 초기화함 //////////////////////////////////
	memset( &gstCityTourBusTicketInfo, 0x00,
		sizeof( CITY_TOUR_BUS_TICKET_INFO) );
	// 추가승차가 아닌 경우 다인승정보를 클리어함 //////////////////////////////
	// stTransInfo 변수는 소멸되므로 굳이 클리어할 필요 없음
	if ( gstMultiEntInfo.boolIsAddEnt == FALSE )
	{
		memset( &gstMultiEntInfo, 0x00, sizeof( MULTI_ENT_INFO ) );
		DisplayDWORDInUpFND( 0 );
 		DisplayDWORDInDownFND( 0 );
	}
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       DisplayResult                                            *
*                                                                              *
*  DESCRIPTION:       카드처리의 모든 과정이 완료되었으면, 처리결과를          *
*                     음성과 부저, FND로 출력한다.                             *
*                                                                              *
*  INPUT PARAMETERS:  pstTransInfo - 카드정보구조체의 포인터                   *
*                     sErrCode - 오류코드                                      *
*                                                                              *
*  RETURN/EXIT VALUE: void                                                     *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-25                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static void DisplayResult( TRANS_INFO *pstTransInfo, short sErrCode )
{
	if ( sErrCode == SUCCESS )
		DisplaySuccess( pstTransInfo );
	else
		DisplayError( pstTransInfo, sErrCode );
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       DisplaySuccess                                           *
*                                                                              *
*  DESCRIPTION:       카드처리가 정상적으로 완료된 경우 처리결과를             *
*                     음성과 부저, FND로 출력한다.                             *
*                                                                              *
*  INPUT PARAMETERS:  pstTransInfo - 카드정보구조체의 포인터                   *
*                                                                              *
*  RETURN/EXIT VALUE: void                                                     *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-25                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static void DisplaySuccess( TRANS_INFO *pstTransInfo )
{
	byte bTempUserType = 0;
	word wTotalUserCnt = 0;
	byte abBuf[6] = {0, };

	// 추가승차의 경우 처리결과를 출력하지 않음
	if ( pstTransInfo->boolIsAddEnt &&
		!IsEnt( pstTransInfo->stNewXferInfo.bEntExtType ) )
		return;

	// 운전자조작기에 처리금액 등을 출력함 /////////////////////////////////////
	if ( pstTransInfo->stNewXferInfo.dwFare != 0 )
	{
		DWORD2ASCWithFillLeft0( pstTransInfo->stNewXferInfo.dwFare, abBuf,
			sizeof( abBuf ) );
		ASC2BCD( abBuf, gpstSharedInfo->abTotalFare, sizeof( abBuf ) );
	}
	else
	{
		/*
		 * 운전자조작기에 0원을 출력하기 위해서는 "000001"을 전송한다.
		 */
		ASC2BCD( KEYPAD_PRINT_0_WON, gpstSharedInfo->abTotalFare,
			strlen( KEYPAD_PRINT_0_WON ) );
	}

	wTotalUserCnt =
		pstTransInfo->abUserCnt[0] +
		pstTransInfo->abUserCnt[1] +
		pstTransInfo->abUserCnt[2];

	// O LED ON
	OLEDOn();

	if ( pstTransInfo->bCardType == TRANS_CARDTYPE_MIF_TOUR )
	{
	 	DisplayDWORDInUpFND( pstTransInfo->bMifTourUseCnt );
		DisplayDWORDInDownFND(
			pstTransInfo->stNewXferInfo.bMifTourDailyAccUseCnt );
	}
	else
	{
	 	DisplayDWORDInUpFND( pstTransInfo->stNewXferInfo.dwFare );
		DisplayDWORDInDownFND( pstTransInfo->stNewXferInfo.dwBal );
	}

	// 테스트카드 //////////////////////////////////////////////////////////////
	if ( pstTransInfo->bPLUserType == USERTYPE_TEST )
	{
		DebugOut( "[DisplaySuccess] 테스트카드\n" );
		if ( gboolIsMainTerm &&
			 IsEnt( pstTransInfo->stNewXferInfo.bEntExtType ) )
		{
			VoiceOut( VOICE_TEST_CARD );
		}
		Buzzer( 1, 50000 );
	}
	// 다인승 승차//////////////////////////////////////////////////////////////
	else if ( pstTransInfo->boolIsMultiEnt && wTotalUserCnt > 1 &&
			  IsEnt( pstTransInfo->stNewXferInfo.bEntExtType ) )
	{
		DebugOut( "[DisplaySuccess] 다인승 승차\n" );

		// 환승이면
		if ( pstTransInfo->stNewXferInfo.bAccXferCnt == 0 )
		{
			// 다인승 환승이 아니면
			Buzzer( 1, 50000 );
			VoiceOut( VOICE_MULTI_ENT );		// 다인승입니다.
		}
		else
		{
			Buzzer( 1, 50000 );
			VoiceOut( VOICE_XFER );				// 환승입니다.
		}
	}
	// 하차 ////////////////////////////////////////////////////////////////////
	else if ( !IsEnt( pstTransInfo->stNewXferInfo.bEntExtType ) )
	{
		word wTotalExtUserCnt = 0;

		DebugOut( "[DisplaySuccess] 하차\n" );

		wTotalExtUserCnt =
			pstTransInfo->stNewXferInfo.abMultiEntInfo[0][USER_CNT] +
			pstTransInfo->stNewXferInfo.abMultiEntInfo[1][USER_CNT] +
			pstTransInfo->stNewXferInfo.abMultiEntInfo[2][USER_CNT];

		if ( wTotalExtUserCnt != 1 )
		{
			Buzzer( 1, 50000 );
		}
		else
		{
			switch ( pstTransInfo->stNewXferInfo.abMultiEntInfo[0][USER_TYPE] )
			{
				case USERTYPE_CHILD:
				case USERTYPE_STUDENT:
				case USERTYPE_YOUNG:
					Buzzer( 2, 50000 );
					break;
				default:
					Buzzer( 1, 50000 );
			}
		}
	}
	// 환승 승차 ///////////////////////////////////////////////////////////////
	else if ( pstTransInfo->stNewXferInfo.bAccXferCnt > 0 )
	{
		DebugOut( "[DisplaySuccess] 환승\n" );

		if ( pstTransInfo->boolIsMultiEnt )
			bTempUserType = pstTransInfo->abUserType[0];
		else
			bTempUserType = pstTransInfo->bPLUserType;

		VoiceOut( VOICE_XFER );					// 환승입니다

		// 사용자유형이 (어린이 || 학생 || 청소년)이면 부저 2번 출력
		if ( bTempUserType == USERTYPE_CHILD ||
			 bTempUserType == USERTYPE_STUDENT ||
			 bTempUserType == USERTYPE_YOUNG )
			Buzzer( 2, 50000 );
		else
			Buzzer( 1, 50000 );
	}
	// 승차 ////////////////////////////////////////////////////////////////////
	else
	{
		DebugOut( "[DisplaySuccess] 승차\n" );

		// 다인승으로 사용자유형을 입력한 경우
		if ( pstTransInfo->boolIsMultiEnt )
		{
			bTempUserType = pstTransInfo->abUserType[0];
		}
		// 다인승 입력이 아닌 경우
		else
		{
			bTempUserType = pstTransInfo->bPLUserType;
		}

		// 음성 없음
		// 단, 시티투어버스에서 관광권 사용시 'Thank you' 음성 출력
		if ( IsCityTourBus( gstVehicleParm.wTranspMethodCode ) == TRUE &&
			 pstTransInfo->bCardType == TRANS_CARDTYPE_MIF_TOUR )
		{
			VoiceOut( VOICE_THANK_YOU );		// Thank you
		}

		// 사용자유형이 (어린이 || 학생 || 청소년)이면 부저 2번 출력
		if ( bTempUserType == USERTYPE_CHILD ||
			 bTempUserType == USERTYPE_STUDENT ||
			 bTempUserType == USERTYPE_YOUNG )
			Buzzer( 2, 50000 );
		else
			Buzzer( 1, 50000 );
	}

#ifndef TEST_NOT_SLEEP_DURING_DISPLAY
	usleep( 1000000 );
#endif

	// O LED off
	OLEDOff();

 	// FND 초기화 (0, 0)
 	DisplayDWORDInUpFND( 0 );
 	DisplayDWORDInDownFND( 0 );
LogMain("성공출력\n");
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       DisplayError                                             *
*                                                                              *
*  DESCRIPTION:       카드처리에서 오류가 발생한 경우 처리결과를               *
*                     음성과 부저, FND로 출력한다.                             *
*                                                                              *
*  INPUT PARAMETERS:  pstTransInfo - 카드정보구조체의 포인터                   *
*                     sErrCode - 오류코드                                      *
*                                                                              *
*  RETURN/EXIT VALUE: void                                                     *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-25                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static void DisplayError( TRANS_INFO *pstTransInfo, short sErrCode )
{
	XLEDOn();

	Buzzer( 3, 80000 );

	switch ( sErrCode )
	{
		case ERR_CARD_PROC_NOT_ONE_CARD:
			VoiceOut( VOICE_TAG_ONE_CARD );		// 카드를 한장만 대주십시오.
			break;
		case ERR_CARD_PROC_CANNOT_USE:
			VoiceOut( VOICE_INVALID_CARD );		// 사용할 수 없는 카드입니다.
			break;
		case ERR_CARD_PROC_RETAG_CARD:
			// 여기서 기록된 카운트는 TT에 기록됨
			gwRetagCardCnt++;
			VoiceOut( VOICE_RETAG_CARD );		// 카드를 다시 대주세요.
			break;
		case ERR_CARD_PROC_INSUFFICIENT_BAL:
			VoiceOut( VOICE_INSUFFICIENT_BAL );	// 잔액이 부족합니다.
			break;
		case ERR_CARD_PROC_ALREADY_PROCESSED:
			VoiceOut( VOICE_ALREADY_PROCESSED );
												// 이미 처리되었습니다.
			break;
		case ERR_CARD_PROC_EXPIRED_CARD:
			VoiceOut( VOICE_EXPIRED_CARD );		// 카드 유효기간이 지났습니다.
			break;
		case ERR_CARD_PROC_TAG_IN_EXT:
			// 0329까지는 voicever.dat 파일이 존재하지 않아
			// gstMyTermInfo.abVoiceVer 변수의 모든 바이트가 0x00으로 설정됨
			// '내릴때 카드를 대주세요' 음성은 0330 버전부터 추가됨
			if ( gstMyTermInfo.abVoiceVer[0] == 0x00 )
			{
				VoiceOut( VOICE_ALREADY_PROCESSED );
												// 이미 처리되었습니다.
			}
			else
			{
				VoiceOut( VOICE_TAG_IN_EXT );	// 내릴때 카드를 대주세요.
			}
			break;
		case ERR_CARD_PROC_NOT_APPROV:
			VoiceOut( VOICE_NOT_APPROV );		// 미승인 카드입니다.
			break;
		case ERR_CARD_PROC_CANNOT_MULTI_ENT:
			VoiceOut( VOICE_CANNOT_MULTI_ENT_CARD );
												// 다인승이 불가능한 카드입니다.
			break;
		case ERR_CARD_PROC_INPUT_TICKET:
			VoiceOut( VOICE_INPUT_TICKET_INFO );
												// 승차권을 선택해 주세요.
			break;
		case ERR_CARD_PROC_CITY_PASS_CARD:
			VoiceOut( VOICE_CITY_PASS_CARD );	// 시티패스입니다.
			sleep( 1 );
			VoiceOut( VOICE_RETAG_CARD );		// 카드를 다시 대주세요.
			break;
		case ERR_CARD_PROC_NO_VOICE:
			// 음성없이 비프음만 3회 출력
			break;
		default:
			// 여기서 기록된 카운트는 TT에 기록됨
			gwRetagCardCnt++;
			VoiceOut( VOICE_RETAG_CARD );
			break;
	}
#ifndef TEST_NOT_SLEEP_DURING_DISPLAY
	usleep( 1000000 );
#endif

	XLEDOff();
LogMain("오류출력\n");
}

