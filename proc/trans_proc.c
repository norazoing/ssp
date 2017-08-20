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
*  PROGRAM ID :       trans_proc.c                                             *
*                                                                              *
*  DESCRIPTION:       승하차유형판단 및 요금을 계산하고 신규환승정보를         *
*                     생성한다.                                                *
*                                                                              *
*  ENTRY POINT:       None                                                     *
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
* 2005/07/13 Solution Team  Boohyeon Jeon Initial Release                      *
*                                                                              *
*******************************************************************************/

/*******************************************************************************
*  Inclusion of Header Files                                                   *
*******************************************************************************/
#include "../system/bus_type.h"
#include "card_proc_util.h"
#include "card_proc.h"

#include "trans_proc.h"

/*******************************************************************************
*  Macro Definition                                                            *
*******************************************************************************/
/*
 * 요금 초기값 관련 매크로 정의
 */
#define MAX_FARE					100000			// 최대가능요금
#define SC_PREPAY_MAX_BAL			500000			// 신선불카드 최대가능잔액
#define MIF_PREPAY_MAX_BAL			70000			// 구선불카드 최대가능잔액
#define MIN_BAL						250				// 최소가능잔액
#define MAX_PREV_UNCHARGED_FARE		900				// 최대가능이전미징수금액
#define MAX_XFER_ENABLE_CNT			5				// 최대환승가능횟수
#define MAX_MIF_TOUR_DAILY_USE_CNT	20				// 관광권카드 최대일누적사용횟수

/*
 * 비환승사유 관련 매크로 정의
 * - 거래내역의 최대기본요금3의 1byte를 전용하여 기록함
 */
#define NOT_XFER_CAUSE_XFER_TIME_ELAPSED			0x01	// 시간초과
#define NOT_XFER_CAUSE_XFER_CNT_ELAPSED				0X02	// 환승횟수초과
#define NOT_XFER_CAUSE_INVALID_TRANSP_METHOD_CODE	0x03	// 교통수단틀림
#define NOT_XFER_CAUSE_DIFF_MIF_TERM_GROUP_CODE		0x04	// 그룹코드틀림
#define NOT_XFER_CAUSE_DIFF_MULTI_GET_ON_INFO		0x05	// 승객수다름
#define NOT_XFER_CAUSE_SAME_TERM					0x06	// 동일차량
#define NOT_XFER_CAUSE_NOT_TAG_IN_EXT				0x07	// 하차미태그
#define NOT_XFER_CAUSE_INVALID_XFER_APPLY_INFO		0x08	// 환승적용정보깨짐

/*******************************************************************************
*  Declaration of Module Functions                                             *
*******************************************************************************/
static short GetEntExtType( TRANS_INFO *pstTransInfo, byte *pbEntExtType );
static short ProcEnt( TRANS_INFO *pstTransInfo );
static bool IsXfer( TRANS_INFO *pstTransInfo );
static bool IsValidXferFromXferApplyInfo( time_t tPrevEntExtDatetime,
	time_t tNewEntExtDatetime, byte bAccXferCnt, byte *pbNonXferCause );
static bool IsValidXferFromMultiEnt( TRANS_INFO *pstTransInfo );
static short ProcExt( TRANS_INFO *pstTransInfo );
static dword CalcDist( byte *abEntStationID, byte *abExtStationID );
static dword CalcFare( byte bCardType, byte bUserType, word wTranspMethodCode,
	time_t tEntExtDtime, bool boolIsXfer, bool boolIsBaseFare );
static void GetDisExtraAmtRate( byte bCardType, byte bUserType,
	time_t tEntExtDtime, bool boolIsXfer, bool boolIsBaseFare,
	int *pnDisExtraAmt, float *pfDiscExtraRate );
static bool SearchDisExtraInfo( byte *abDisExtraTypeID, bool boolIsBaseFare,
	int *pnDisExtraAmt, float *pfDisExtraRate );
static short ProcCityTourEnt( TRANS_INFO *pstTransInfo );

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       TransProc                                                *
*                                                                              *
*  DESCRIPTION:       카드정보구조체의 포인터를 입력받아 이를 바탕으로 승하차  *
*                     유형을 판단하고 요금을 계산한다. 그리고 신규환승정보를   *
*                     생성하여 이를 다시 카드정보구조체에 설정한다.            *
*                                                                              *
*  INPUT PARAMETERS:  pstTransInfo - 카드정보구조체                            *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공                                      *
*                     ERR_CARD_PROC_NO_VOICE - 요금이 100,000원 이상인 경우    *
*                        (음성없이 비프음 3회 출력)                            *
*                     ERR_CARD_PROC_CANNOT_USE - 구선불카드이면서 잔액이       *
*                         500,000원 이상인 경우                                *
*                     ERR_CARD_PROC_TAG_IN_EXT - 최소재태그가능시간 미만인 경우*
*                     ERR_CARD_PROC_ALREADY_PROCESSED - 최소재승차가능시간     *
*                         이하인 경우                                          *
*                     ERR_CARD_PROC_INSUFFICIENT_BAL - 선불카드이면서 잔액이   *
*                         지불해야하는 요금보다 적은 경우                      *
*                     ERR_CARD_PROC_RETAG_CARD - 다인승정보의 승객수 합이 0    *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-23                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
short TransProc( TRANS_INFO *pstTransInfo )
{
	short sResult = SUCCESS;
	byte bEntExtType = 0;
	byte abNowDate[8] = {0, };

	/*
	 * 시티투어버스의 경우 승차처리
	 */
	if ( IsCityTourBus( gstVehicleParm.wTranspMethodCode ) == TRUE )
	{
		sResult = ProcCityTourEnt( pstTransInfo );
		return sResult;
	}

	// 신선불카드이면서 거래전 잔액이 500,000원 이상인 경우 ////////////////////
	// - 사용할 수 없는 카드입니다.
	if ( pstTransInfo->bCardType == TRANS_CARDTYPE_SC_PREPAY &&
		 pstTransInfo->stPrevXferInfo.dwBal > SC_PREPAY_MAX_BAL )
	{
		printf( "[TransProc] 신선불카드이면서 잔액이 500,000원 초과 - " );
		printf( "사용할 수 없는 카드입니다\n" );
		return ERR_CARD_PROC_CANNOT_USE;
	}

	// 구선불카드이면서 거래전 잔액이 70,000원 이상인 경우 /////////////////////
	// - 사용할 수 없는 카드입니다.
	if ( pstTransInfo->bCardType == TRANS_CARDTYPE_MIF_PREPAY &&
		 pstTransInfo->stPrevXferInfo.dwBal > MIF_PREPAY_MAX_BAL )
	{
		printf( "[TransProc] 구선불카드이면서 잔액이 70,000원 초과 - " );
		printf( "사용할 수 없는 카드입니다\n" );
		return ERR_CARD_PROC_CANNOT_USE;
	}

	/*
	 * 승하차유형판단
	 */
	sResult = GetEntExtType( pstTransInfo, &bEntExtType );
	if ( sResult != SUCCESS )
	{
		return sResult;
	}

	// 승차처리 ////////////////////////////////////////////////////////////////
	if ( bEntExtType == ENT )
	{
		// 관광권카드의 경우
		if ( pstTransInfo->bCardType == TRANS_CARDTYPE_MIF_TOUR )
		{
			// 최초사용일 이후 사용기간이 지난 카드의 경우
			// '카드 유효기간이 지났습니다' 음성 출력
			if ( IsValidMifTourExpiryDate( pstTransInfo->tMifTourFirstUseDtime,
					pstTransInfo->wMifTourCardType,
					pstTransInfo->stNewXferInfo.tEntExtDtime ) == FALSE )
			{
				printf( "[TransProc] 최초사용일시 이후 카드유형에 따른 " );
				printf( "사용기한을 초과하였으므로 카드사용이 불가함\n" );
				return ERR_CARD_PROC_EXPIRED_CARD;
			}

			// 만기일이 지난 카드의 경우
			// '카드 유효기간이 지났습니다' 음성 출력
			TimeT2ASCDate( pstTransInfo->stNewXferInfo.tEntExtDtime, abNowDate);
			if ( memcmp( abNowDate, pstTransInfo->abMifTourExpiryDate, 8 ) > 0 )
			{
				printf( "[TransProc] 관광권카드 만기일 이후 사용 오류\n" );
				return ERR_CARD_PROC_EXPIRED_CARD;
			}
		}

		// '승차'의 경우 다인승정보의 승객수의 합이 0이면
		// '카드를 다시 대주세요' 리턴
		if ( pstTransInfo->abUserCnt[0] +
			 pstTransInfo->abUserCnt[1] +
			 pstTransInfo->abUserCnt[2] == 0 )
		{
			printf( "[TransProc] 다인승정보의 승객수의 합이 0 - 카드를 다시 " );
			printf( "대주세요" );
			return ERR_CARD_PROC_RETAG_CARD;
		}

		// '승차'의 경우 추가재승차여부를 FALSE로 초기화함
		gstMultiEntInfo.boolIsAddEnt = FALSE;

		// 승차처리
		sResult = ProcEnt( pstTransInfo );
	}
	// 하차처리 ////////////////////////////////////////////////////////////////
	else
	{
		// 하차시 승/하차시간 역전현상이 발생하면
		// 승차시간에 1초를 더한 값을 설정
		if ( pstTransInfo->stNewXferInfo.tEntExtDtime <
				pstTransInfo->stPrevXferInfo.tEntExtDtime )
		{
			printf( "[TransProc] 승하차시간이 역전되어 보정함\n" );
			if ( abs( pstTransInfo->stPrevXferInfo.tEntExtDtime -
					pstTransInfo->stNewXferInfo.tEntExtDtime ) > 20 )
			{
				printf( "[TransProc] 역전된 승하차시간이 20초 이상이므로 " );
				printf( "하차시간보정 플래그 남김\n" );
				pstTransInfo->boolIsAdjustExtDtime = TRUE;
			}
			pstTransInfo->stNewXferInfo.tEntExtDtime =
				pstTransInfo->stPrevXferInfo.tEntExtDtime + 1;
		}
		
		// 하차처리
		sResult = ProcExt( pstTransInfo );
	}
	if ( sResult != SUCCESS )
	{
		return sResult;
	}

	// 요금이 100,000원 이상인 경우 - 음성없이 비프음 3회 출력 /////////////////
	if ( pstTransInfo->stNewXferInfo.dwFare > MAX_FARE )
	{
		printf( "[TransProc] 요금이 100,000원 이상 - 음성없이 비프음 3회 " );
		printf( "출력\n" );
		return ERR_CARD_PROC_NO_VOICE;
	}

	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       GetEntExtType                                            *
*                                                                              *
*  DESCRIPTION:       입력된 카드정보구조체를 이용하여 승하차유형을 판단한다.  *
*                                                                              *
*  INPUT PARAMETERS:  pstTransInfo - 카드정보구조체의 포인터                   *
*                     pbEntExtType - 승하차유형을 리턴하기 위한 바이트 포인터  *
*                                    [ENT / EXT]                               *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공                                      *
*                     ERR_CARD_PROC_TAG_IN_EXT - 최소재태그가능시간 미만인 경우*
*                     ERR_CARD_PROC_ALREADY_PROCESSED - 최소재승차가능시간     *
*                         이하인 경우                                          *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-23                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static short GetEntExtType( TRANS_INFO *pstTransInfo, byte *pbEntExtType )
{
	int nRetagSec = 0;				// 재태그시간 (초)

	int nMaxRidingSec = 0;			// 최대승차가능시간   (마을1시간/일반3시간)
	int nMinAbroadSec = 0;			// 최소재승차가능시간 (마을10분/일반20분)
	int nMinRetagSec = 0;			// 최소재태그가능시간

	bool gboolIsSameStation = FALSE;// 이전환승정보의 정류장과
									// 현재 정류장의동일 정류장 여부
	bool gboolIsSameTerm = FALSE;	// 이전환승정보의 터미널ID와 현재
									// 차량의 터미널ID가 동일한지의 여부

#ifdef TEST_ALWAYS_ENT
	return ENT;
#endif

	// 최대승차가능시간 및 최소재승차가능시간 설정 /////////////////////////////
	// 마을버스의 경우
	if ( IsVillageBus( gstVehicleParm.wTranspMethodCode ) )
	{
		nMaxRidingSec = 3600;		// 최대승차가능시간   1시간
		nMinAbroadSec = 600;		// 최소재승차가능시간 10분
	}
	// 일반버스의 경우
	else
	{
		nMaxRidingSec = 18000;		// 최대승차가능시간   5시간
		nMinAbroadSec = 1200;		// 최소재승차가능시간 20분
	}

	// 최소재태그가능시간 설정 /////////////////////////////////////////////////
	// 승차단말기 또는 하차단말기의 (마을 또는 광역)의 경우 5초
	if ( gboolIsMainTerm == TRUE ||
		 IsVillageBus( gstVehicleParm.wTranspMethodCode ) ||
		 IsWideAreaBus( gstVehicleParm.wTranspMethodCode ) )
	{
		nMinRetagSec = 5;
	}
	// 하차단말기 & 그외 교통수단의 경우 15초
	else
	{
		nMinRetagSec = 15;
	}

	// 이전환승정보의 정류장과 현재 정류장의 동일 정류장 여부 설정 /////////////
	if ( memcmp( gpstSharedInfo->abNowStationID,
		 pstTransInfo->stPrevXferInfo.abStationID,
		 sizeof( gpstSharedInfo->abNowStationID ) ) == 0 )
		gboolIsSameStation = TRUE;
	else
		gboolIsSameStation = FALSE;

	// 이전환승정보의 터미널ID와 현재 차량의 터미널ID가 동일한지 여부 결정 /////
	if ( pstTransInfo->stPrevXferInfo.dwTermID ==
			GetDWORDFromASC( gpstSharedInfo->abMainTermID,
		 		sizeof( gpstSharedInfo->abMainTermID ) ) )
	{
		gboolIsSameTerm = TRUE;
	}
	else
	{
		gboolIsSameTerm = FALSE;
	}

	// 재태그시간 설정 /////////////////////////////////////////////////////////
	nRetagSec = pstTransInfo->stNewXferInfo.tEntExtDtime -
				pstTransInfo->stPrevXferInfo.tEntExtDtime;

	// 재태그시간에 오류가 있는 경우 15초로 설정
	if ( nRetagSec < 0 )
		nRetagSec = 15;

	// 추가승차 여부 설정 //////////////////////////////////////////////////////
	// 1. 추가승차여부가 FALSE이고
	// 2. 이전환승정보의 승하차유형이 '승차'이고
	// 3. 다인승이 설정되어 있으며
	// 4. 이전환승정보의 단말기ID가 현 단말기ID와 동일하고
	// 5. 카드번호LOG에 카드번호가 존재하는 경우
	if ( !gstMultiEntInfo.boolIsAddEnt &&
		 IsEnt( pstTransInfo->stPrevXferInfo.bEntExtType ) &&
		 gstMultiEntInfo.boolIsMultiEnt &&
		 gboolIsSameTerm &&
		 IsExistCardNoLog( pstTransInfo->abCardNo ) )
	{
		gstMultiEntInfo.boolIsAddEnt = TRUE;
	}

	DebugOut( "\n" );
	DebugOut( "#############################################################" );
	DebugOut( "##################\n" );
	DebugOut( "# 승하차유형 판단\n" );
	DebugOut( "#\n" );
	DebugOut( "# 기본데이터\n" );
	DebugOut( "#     - 재태그시간                           : %5d (sec)\n",
		nRetagSec );
	DebugOut( "#\n" );
	DebugOut( "#     - 최대승차가능시간                     : %5d (sec)\n",
		nMaxRidingSec );
	DebugOut( "#     - 최소재승차가능시간                   : %5d (sec)\n",
		nMinAbroadSec );
	DebugOut( "#     - 최소재태그가능시간                   : %5d (sec)\n",
		nMinRetagSec );
	DebugOut( "#\n" );
	DebugOut( "#     - 추가재승차 여부                      : %s\n",
		GetBoolString( gstMultiEntInfo.boolIsAddEnt ) );
	DebugOut( "#     - 운전자조작기 입력을 통한 다인승 여부 : %s\n",
		GetBoolString( gstMultiEntInfo.boolIsMultiEnt ) );
	DebugOut( "#     - 동일 단말기 여부                     : %s\n",
		GetBoolString( gboolIsSameTerm ) );
	DebugOut( "#     - 동일 정류장 여부                     : %s\n",
		GetBoolString( gboolIsSameStation ) );
	DebugOut( "#\n" );
	DebugOut( "# 판단과정\n" );

	// 동일단말기 //////////////////////////////////////////////////////////////
	if ( gboolIsSameTerm == TRUE )
	{

		DebugOut( "#     - 동일단말기\n" );

		// 단말기그룹코드가 타시도 코드이면 '승차' /////////////////////////////
		if ( pstTransInfo->bMifTermGroupCode == 0x02 ||
			 pstTransInfo->bMifTermGroupCode == 0x03 ||
			 pstTransInfo->bMifTermGroupCode == 0x04 ||
			 pstTransInfo->bMifTermGroupCode == 0x06 ||
			 pstTransInfo->bMifTermGroupCode == 0x07 ||
			 ( pstTransInfo->bMifTermGroupCode == 0x05 &&
			   pstTransInfo->tMifEntExtDtime !=
				pstTransInfo->stPrevXferInfo.tEntExtDtime ) )
		{
			DebugOut( "#     - 구 단말기그룹코드가 타시도 코드\n" );
			DebugOut( "#     - 최종결과 : 승차\n" );

			*pbEntExtType = ENT;

			return SUCCESS;
		}

		// 이전 승하차유형이 '승차' ////////////////////////////////////////////
		if ( IsEnt( pstTransInfo->stPrevXferInfo.bEntExtType ) )
		{
			DebugOut( "#     - 이전 승하차유형이 '승차'\n" );

			// 재태그시간이 최대승차가능시간 이상 //////////////////////////////
			if ( nRetagSec >= nMaxRidingSec )
			{

				DebugOut( "#     - 재태그시간이 최대승차가능시간 이상\n" );
				DebugOut( "#     - 최종결과 : 승차\n" );

				*pbEntExtType = ENT;

				return SUCCESS;
			}
			// 재태그시간이 최대승차가능시간 미만 //////////////////////////////
			else
			{

				DebugOut( "#     - 재태그시간이 최대승차가능시간 미만\n" );

				// 추가재승차 //////////////////////////////////////////////////
				if ( gstMultiEntInfo.boolIsAddEnt )
				{
					DebugOut( "#     - 추가재승차\n" );
					DebugOut( "#     - 최종결과 : 하차\n" );

					*pbEntExtType = EXT;

					return SUCCESS;
				}
				// 다인승 //////////////////////////////////////////////////////
				else if ( gstMultiEntInfo.boolIsMultiEnt )
				{
					DebugOut( "#     - 다인승\n" );
					DebugOut( "#     - 최종결과 : 승차\n" );

					*pbEntExtType = ENT;

					return SUCCESS;
				}
				// 추가재승차도 다인승도 아님 //////////////////////////////////
				else
				{
					DebugOut( "#     - 추가재승차도 다인승도 아님\n" );

					// 최소재태그가능시간이상 또는 다른정류장 //////////////////
					if ( nRetagSec >= nMinRetagSec || !gboolIsSameStation )
					{
						DebugOut( "#     - 최소재태그가능시간이상 또는 " );
						DebugOut( "다른정류장\n" );
						DebugOut( "#     - 최종결과 : 하차\n" );

						*pbEntExtType = EXT;

						return SUCCESS;
					}
					else
					{
						DebugOut( "#     - 최소재태그가능시간미만\n" );
						DebugOut( "#     - 최종결과 : 내릴때카드를" );
						DebugOut( "대주세요\n" );

						return ERR_CARD_PROC_TAG_IN_EXT;
					}
				}
			}
		}
		// 이전 승하차유형이 '하차' ////////////////////////////////////////////
		else
		{

			DebugOut( "#     - 이전 승하차유형이 '하차'\n" );

			// 최소재승차가능시간이상 또는 다인승 또는 추가재승차 //////////////
			if ( nRetagSec >= nMinAbroadSec ||
				gstMultiEntInfo.boolIsMultiEnt ||
				gstMultiEntInfo.boolIsAddEnt )
			{

				DebugOut( "#     - 최소재승차가능시간이상 또는 다인승 " );
				DebugOut( "또는 하차 후 재승차\n" );
				DebugOut( "#     - 최종결과 : 승차\n" );

				*pbEntExtType = ENT;

				return SUCCESS;
			}
			// 최소재승차가능시간미만이면서 다인승아님 /////////////////////////
			else
			{

				DebugOut( "#     - 최소재승차가능시간미만이면서 " );
				DebugOut( "다인승아님\n" );
				DebugOut( "#     - 최종결과 : 이미처리된카드입니다\n" );

				return ERR_CARD_PROC_ALREADY_PROCESSED;
			}
		}
	}
	// 다른단말기 //////////////////////////////////////////////////////////////
	else
	{

		DebugOut( "#     - 다른단말기\n" );
		DebugOut( "#     - 최종결과 : 승차\n" );

		*pbEntExtType = ENT;

		return SUCCESS;
	}

	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       ProcEnt                                                  *
*                                                                              *
*  DESCRIPTION:       승차의 경우 요금을 계산하고 신규환승정보를 생성한다.     *
*                                                                              *
*  INPUT PARAMETERS:  pstTransInfo - 카드정보구조체의 포인터                   *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공                                      *
*                     ERR_CARD_PROC_INSUFFICIENT_BAL - 선불카드이면서 잔액이   *
*                         지불해야하는 요금보다 적은 경우                      *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-24                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static short ProcEnt( TRANS_INFO *pstTransInfo )
{
	int i = 0;
	bool boolIsPenalty = FALSE;
	bool boolIsXfer = FALSE;
	dword awMaxBaseFare[3] = {0, };

	DebugOut( "#\n" );
	DebugOut( "# 승차과정\n" );

	// 페널티 처리 /////////////////////////////////////////////////////////////
	// 서울 교통수단 페널티 발생 조건
	//     1) 이전에 사용한 교통수단이 서울 소속 &&
	//     2) 환승누적횟수가 0보다 큼 &&
	//     3) 이전 승하차유형이 승차
	if ( IsSeoulTransp( pstTransInfo->stPrevXferInfo.wTranspMethodCode ) &&
		 pstTransInfo->stPrevXferInfo.bAccXferCnt > 0 &&
		 IsEnt( pstTransInfo->stPrevXferInfo.bEntExtType ) )
	{
		dword dwSumOfPrevBaseFare = 0;		// 이전 교통수단 기본요금의 합

		// 페널티 계산을 위하여 이전 교통수단 기본요금의 합을 계산
		for ( i = 0; i < 3; i++ )
			dwSumOfPrevBaseFare += CalcFare( pstTransInfo->bCardType,
				pstTransInfo->stPrevXferInfo.abMultiEntInfo[i][USER_TYPE],
				pstTransInfo->stPrevXferInfo.wTranspMethodCode,
				pstTransInfo->stPrevXferInfo.tEntExtDtime,
				TRUE,
				TRUE ) *
				pstTransInfo->stPrevXferInfo.abMultiEntInfo[i][USER_CNT];

		// 이전 교통수단 기본요금의 합이 이전 교통수단에서 지불한 요금보다
		// 큰 경우 (할인 받은 금액이 존재하는 경우)
		// - 할인받은 금액을 신규환승정보의 직전페널티요금으로 설정
		if ( dwSumOfPrevBaseFare > pstTransInfo->stPrevXferInfo.dwFare )
			pstTransInfo->stNewXferInfo.wPrevPenaltyFare =
				dwSumOfPrevBaseFare - pstTransInfo->stPrevXferInfo.dwFare;
		// 할인받은 금액이 존재하지 않는 경우
		// - 신규환승정보의 직전페널티요금을 0으로 설정
		else
			pstTransInfo->stNewXferInfo.wPrevPenaltyFare = 0;

		// 신규환승정보의 요금에 직전페널티요금을 부가
		pstTransInfo->stNewXferInfo.dwFare +=
			pstTransInfo->stNewXferInfo.wPrevPenaltyFare;

		// 페널티 발생 여부를 TRUE로 설정
		boolIsPenalty = TRUE;

		DebugOut( "#     - 페널티 발생                          : %5u (원)\n",
			pstTransInfo->stNewXferInfo.wPrevPenaltyFare );
	}
	// 서울외 교통수단 페널티 발생 조건
	//     1) 이전에 사용한 교통수단이 서울 아님 &&
	//     2) 서울외 교통수단 페널티 교통수단코드에 0이 아닌 값이 설정
	else if ( !IsSeoulTransp( pstTransInfo->stPrevXferInfo.wTranspMethodCode )
		&& pstTransInfo->stPrevXferInfo.wIncheonPenaltyPrevTMCode != 0 )
	{
		dword dwSumOfPrevBaseFare = 0;		// 이전 교통수단 기본요금의 합

		// 페널티 계산을 위하여 이전 교통수단 기본요금의 합을 계산
		for ( i = 0; i < 3; i++ )
		{
			dwSumOfPrevBaseFare += CalcFare( pstTransInfo->bCardType,
				pstTransInfo->stPrevXferInfo.abMultiEntInfo[i][USER_TYPE],
				pstTransInfo->stPrevXferInfo.wIncheonPenaltyPrevTMCode,
				pstTransInfo->stPrevXferInfo.tIncheonPenaltyPrevDtime,
				TRUE,
				TRUE ) *
				pstTransInfo->stPrevXferInfo.abMultiEntInfo[i][USER_CNT];
		}

		// 이전 교통수단 기본요금의 합이 이전 교통수단에서 지불한 요금보다
		// 큰 경우 (할인 받은 금액이 존재하는 경우)
		// - 할인받은 금액을 신규환승정보의 직전페널티요금으로 설정
		if ( dwSumOfPrevBaseFare >
			 pstTransInfo->stPrevXferInfo.dwIncheonPenaltyPrevFare )
			pstTransInfo->stNewXferInfo.wPrevPenaltyFare =
				dwSumOfPrevBaseFare -
				pstTransInfo->stPrevXferInfo.dwIncheonPenaltyPrevFare;
		// 할인받은 금액이 존재하지 않는 경우
		// - 신규환승정보의 직전페널티요금을 0으로 설정
		else
			pstTransInfo->stNewXferInfo.wPrevPenaltyFare = 0;

		// 신규환승정보의 요금에 직전페널티요금을 부가
		pstTransInfo->stNewXferInfo.dwFare +=
			pstTransInfo->stNewXferInfo.wPrevPenaltyFare;

		// 페널티 발생 여부를 TRUE로 설정
		boolIsPenalty = TRUE;

		DebugOut( "#     - 페널티 발생 (서울외 환승)            : %5u (원)\n",
			pstTransInfo->stNewXferInfo.wPrevPenaltyFare );
	}

	// 구카드의 경우 승객2, 3 이전거래시 지불된 최대기본요금 복원 //////////////
	// (관광권카드는 다인승이 없음)
	if ( pstTransInfo->bCardType == TRANS_CARDTYPE_MIF_PREPAY ||
		 pstTransInfo->bCardType == TRANS_CARDTYPE_DEPOSIT ||
		 pstTransInfo->bCardType == TRANS_CARDTYPE_MIF_POSTPAY )
	{
		float afDiscountRate[3] = {0, };
		int nTempBaseFare = 0;
		int nDiscount = 0;

		for ( i = 0; i < 3; i++ )
	   		GetDisExtraAmtRate( pstTransInfo->bCardType,
	   			pstTransInfo->stPrevXferInfo.abMultiEntInfo[i][USER_TYPE],
	   			pstTransInfo->stPrevXferInfo.tEntExtDtime,
	   			TRUE,
	   			TRUE,
	   			&nDiscount,
	   			&afDiscountRate[i] );

		if ( pstTransInfo->stPrevXferInfo.abMultiEntInfo[0][USER_CNT] != 0 )
		{
			nTempBaseFare =
				( pstTransInfo->stPrevXferInfo.awMaxBaseFare[0] /
				 pstTransInfo->stPrevXferInfo.abMultiEntInfo[0][USER_CNT] ) /
				( 1 + afDiscountRate[0] / 100 );
			pstTransInfo->stPrevXferInfo.awMaxBaseFare[1] =
				( nTempBaseFare + ( int )( ( float )nTempBaseFare *
				( float )afDiscountRate[1] / 100.0 ) ) *
				pstTransInfo->stPrevXferInfo.abMultiEntInfo[1][USER_CNT];
			pstTransInfo->stPrevXferInfo.awMaxBaseFare[2] =
				( nTempBaseFare + ( int )( ( float )nTempBaseFare *
				( float )afDiscountRate[2] / 100.0 ) ) *
				pstTransInfo->stPrevXferInfo.abMultiEntInfo[2][USER_CNT];
		}

		DebugOut( "#     - 복원된 구카드 승객1 최대기본요금     : %5u (원)\n",
			pstTransInfo->stPrevXferInfo.awMaxBaseFare[0] );
		DebugOut( "#     - 복원된 구카드 승객2 최대기본요금     : %5u (원)\n",
			pstTransInfo->stPrevXferInfo.awMaxBaseFare[1] );
		DebugOut( "#     - 복원된 구카드 승객3 최대기본요금     : %5u (원)\n",
			pstTransInfo->stPrevXferInfo.awMaxBaseFare[2] );
	}

	// 환승여부 판단 ///////////////////////////////////////////////////////////
	if ( ( boolIsXfer = IsXfer( pstTransInfo ) ) )
	{
		DebugOut( "#     - 환승\n" );

		// 환승누적횟수 ////////////////////////////////////////////////////////
		pstTransInfo->stNewXferInfo.bAccXferCnt =
			pstTransInfo->stPrevXferInfo.bAccXferCnt + 1;

		// 환승일련번호 ////////////////////////////////////////////////////////
		pstTransInfo->stNewXferInfo.bXferSeqNo =
			pstTransInfo->stPrevXferInfo.bXferSeqNo;

		// 환승내누적이동거리 //////////////////////////////////////////////////
		pstTransInfo->stNewXferInfo.dwAccDistInXfer =
			pstTransInfo->stPrevXferInfo.dwAccDistInXfer;

		// 환승내누적이용금액 //////////////////////////////////////////////////
		pstTransInfo->stNewXferInfo.dwAccAmtInXfer =
			pstTransInfo->stPrevXferInfo.dwAccAmtInXfer;

		// 다인승거래내역 //////////////////////////////////////////////////////
		memcpy( pstTransInfo->stNewXferInfo.abMultiEntInfo,
			pstTransInfo->stPrevXferInfo.abMultiEntInfo,
			sizeof( pstTransInfo->stNewXferInfo.abMultiEntInfo ) );

		// 각 승객별로 기본요금계산 및 승객 할인할증유형ID 설정 ////////////////
		memset( awMaxBaseFare, 0, sizeof( awMaxBaseFare ) );
		for ( i = 0; i < 3 && pstTransInfo->abUserCnt[i] != 0; i++ )
		{
			// 요금계산 ////////////////////////////////////////////////////////
			awMaxBaseFare[i] = CalcFare( pstTransInfo->bCardType,
				pstTransInfo->abUserType[i],
				gstVehicleParm.wTranspMethodCode,
				pstTransInfo->stNewXferInfo.tEntExtDtime,
				TRUE,
				TRUE ) * pstTransInfo->abUserCnt[i];
			// 승객 할인할증유형ID /////////////////////////////////////////////
			CreateDisExtraTypeID( pstTransInfo->bCardType,
				pstTransInfo->abUserType[i],
				pstTransInfo->stNewXferInfo.tEntExtDtime,
				TRUE,
				pstTransInfo->abDisExtraTypeID[i] );
		}

		// 승객 이전거래시 지불된 최대기본요금 /////////////////////////////////
		for ( i = 0; i < 3; i++ )
		{
			if ( pstTransInfo->stPrevXferInfo.awMaxBaseFare[i] <
				awMaxBaseFare[i] )
			{
				pstTransInfo->stNewXferInfo.awMaxBaseFare[i] = awMaxBaseFare[i];
				pstTransInfo->stNewXferInfo.dwFare += awMaxBaseFare[i] -
					pstTransInfo->stPrevXferInfo.awMaxBaseFare[i];
			}
			else
			{
				pstTransInfo->stNewXferInfo.awMaxBaseFare[i] =
					pstTransInfo->stPrevXferInfo.awMaxBaseFare[i];
			}
		}

		// 미징수요금 //////////////////////////////////////////////////////////
		pstTransInfo->stNewXferInfo.wPrevUnchargedFare =
			pstTransInfo->stPrevXferInfo.wPrevUnchargedFare;

		// 환승내이용수단기본요금의합 //////////////////////////////////////////
		pstTransInfo->stNewXferInfo.wTotalBaseFareInXfer =
			pstTransInfo->stPrevXferInfo.wTotalBaseFareInXfer;

		// 교통수단/승하차유형 /////////////////////////////////////////////////
		// 직전 교통수단이 버스류일때
		if ( IsBus( pstTransInfo->stPrevXferInfo.wTranspMethodCode ) )
		{
			if ( pstTransInfo->stPrevXferInfo.bEntExtType ==
				 XFER_EXT_AFTER_INCHEON )
				pstTransInfo->stNewXferInfo.bEntExtType =
					XFER_ENT_AFTER_INCHEON;
			else
				pstTransInfo->stNewXferInfo.bEntExtType = XFER_ENT_AFTER_BUS;
		}
		// 직전 교통수단이 철도류일때
		else if ( IsSubway( pstTransInfo->stPrevXferInfo.wTranspMethodCode ) )
		{
			if ( pstTransInfo->stPrevXferInfo.bEntExtType ==
				 XFER_EXT_AFTER_INCHEON )
				pstTransInfo->stNewXferInfo.bEntExtType =
					XFER_ENT_AFTER_INCHEON;
			else
				pstTransInfo->stNewXferInfo.bEntExtType = XFER_ENT_AFTER_SUBWAY;
		}
		else
			pstTransInfo->stNewXferInfo.bEntExtType = XFER_ENT;
	}
	else
	{
		DebugOut( "#     - 환승아님 %u\n", pstTransInfo->bNonXferCause );

		// 환승누적횟수 ////////////////////////////////////////////////////////
		pstTransInfo->stNewXferInfo.bAccXferCnt = 0;

		// 환승일련번호 ////////////////////////////////////////////////////////
		// 환승일련번호는 1 ~ 99 까지의 값이 로테이션 됨
		// 단, 구선불카드의 경우 1 ~ 15 까지의 값이 로테이션 됨
		pstTransInfo->stNewXferInfo.bXferSeqNo =
			( pstTransInfo->stPrevXferInfo.bXferSeqNo + 1 ) % 100;
		if ( ( pstTransInfo->bCardType == TRANS_CARDTYPE_MIF_PREPAY ||
			   pstTransInfo->bCardType == TRANS_CARDTYPE_DEPOSIT ) &&
			 pstTransInfo->stNewXferInfo.bXferSeqNo >= 16 )
			pstTransInfo->stNewXferInfo.bXferSeqNo = 1;
		if ( pstTransInfo->stNewXferInfo.bXferSeqNo == 0 )
			pstTransInfo->stNewXferInfo.bXferSeqNo = 1;

		// 환승내누적이동거리 //////////////////////////////////////////////////
		pstTransInfo->stNewXferInfo.dwAccDistInXfer = 0;

		// 환승내누적이용금액 //////////////////////////////////////////////////
		pstTransInfo->stNewXferInfo.dwAccAmtInXfer = 0;

		// 다인승거래내역 //////////////////////////////////////////////////////
		for ( i = 0; i < 3; i++ )
		{
			pstTransInfo->stNewXferInfo.abMultiEntInfo[i][USER_TYPE] =
				pstTransInfo->abUserType[i];
			pstTransInfo->stNewXferInfo.abMultiEntInfo[i][USER_CNT] =
				pstTransInfo->abUserCnt[i];
		}

		// 각 승객별로 기본요금계산 및 승객 할인할증유형ID 설정 ////////////////
		memset( awMaxBaseFare, 0, sizeof( awMaxBaseFare ) );
		for ( i = 0; i < 3 && pstTransInfo->abUserCnt[i] != 0; i++ )
		{
			// 기본요금계산 ////////////////////////////////////////////////////
			awMaxBaseFare[i] = CalcFare( pstTransInfo->bCardType,
				pstTransInfo->abUserType[i],
				gstVehicleParm.wTranspMethodCode,
				pstTransInfo->stNewXferInfo.tEntExtDtime,
				FALSE,
				TRUE ) * pstTransInfo->abUserCnt[i];

			// 승객 할인할증유형ID /////////////////////////////////////////////
			CreateDisExtraTypeID( pstTransInfo->bCardType,
				pstTransInfo->abUserType[i],
				pstTransInfo->stNewXferInfo.tEntExtDtime,
				FALSE,
				pstTransInfo->abDisExtraTypeID[i] );

			DebugOut( "#     - 승객%d 기본요금 및 할인할증ID         : ", i );
			DebugOut( "%5ld (원), ", awMaxBaseFare[i] );
			DebugOutlnASC( "", pstTransInfo->abDisExtraTypeID[i], 6 );
		}

		// 승객 이전거래시 지불된 최대기본요금 /////////////////////////////////
		for ( i = 0; i < 3; i++ )
		{
			pstTransInfo->stNewXferInfo.awMaxBaseFare[i] = awMaxBaseFare[i];
			pstTransInfo->stNewXferInfo.dwFare += awMaxBaseFare[i];
		}

		// 미징수요금 //////////////////////////////////////////////////////////
		pstTransInfo->stNewXferInfo.wPrevUnchargedFare = 0;

		// 환승내이용수단기본요금의합 //////////////////////////////////////////
		pstTransInfo->stNewXferInfo.wTotalBaseFareInXfer = 0;

		// 교통수단/승하차유형 /////////////////////////////////////////////////
		// 직전 교통수단이 버스류일때
		if ( IsBus( pstTransInfo->stPrevXferInfo.wTranspMethodCode ) )
			pstTransInfo->stNewXferInfo.bEntExtType = XFER_ENT_AFTER_BUS;
		// 직전 교통수단이 지하철류일때
		else if ( IsSubway( pstTransInfo->stPrevXferInfo.wTranspMethodCode ) )
			pstTransInfo->stNewXferInfo.bEntExtType = XFER_ENT_AFTER_SUBWAY;
		else
			pstTransInfo->stNewXferInfo.bEntExtType = XFER_ENT;
	}

	// 요금 10원이하 절사 //////////////////////////////////////////////////////
	pstTransInfo->stNewXferInfo.dwFare =
		pstTransInfo->stNewXferInfo.dwFare / 10 * 10;

	// 환승내누적이용금액 //////////////////////////////////////////////////////
	pstTransInfo->stNewXferInfo.dwAccAmtInXfer +=
		pstTransInfo->stNewXferInfo.dwFare;

	// 환승내이용수단기본요금의합 //////////////////////////////////////////////
	pstTransInfo->stNewXferInfo.wTotalBaseFareInXfer += awMaxBaseFare[0] +
		awMaxBaseFare[1] +
		awMaxBaseFare[2] +
		pstTransInfo->stNewXferInfo.wPrevPenaltyFare;

	// 총누적사용금액 //////////////////////////////////////////////////////////
	pstTransInfo->stNewXferInfo.dwTotalAccUseAmt =
		pstTransInfo->stPrevXferInfo.dwTotalAccUseAmt +
		pstTransInfo->stNewXferInfo.dwFare;

	// 총누적승차횟수 //////////////////////////////////////////////////////////
	pstTransInfo->stNewXferInfo.wTotalAccEntCnt =
		pstTransInfo->stPrevXferInfo.wTotalAccEntCnt + 1;

	// 정류장ID ////////////////////////////////////////////////////////////////
	memcpy( pstTransInfo->stNewXferInfo.abStationID,
		gpstSharedInfo->abNowStationID, 7 );

	// 교통수단유형 ////////////////////////////////////////////////////////////
	pstTransInfo->stNewXferInfo.wTranspMethodCode =
		gstVehicleParm.wTranspMethodCode;

	// 터미널ID ////////////////////////////////////////////////////////////////
	pstTransInfo->stNewXferInfo.dwTermID =
		GetDWORDFromASC( gpstSharedInfo->abMainTermID,
		sizeof( gpstSharedInfo->abMainTermID ) );

	// 사용거리	( 단위:100M( 예:10400M ==> '0104' ) ////////////////////////////
	pstTransInfo->dwDist = 0;

	// 구선불카드 이전승하차구분의 하차여부 ////////////////////////////////////
	pstTransInfo->stNewXferInfo.boolIsPrevExt =
		!IsEnt( pstTransInfo->stPrevXferInfo.bEntExtType );

	// 관광권카드의 경우 카드잔액 설정 /////////////////////////////////////////
	if ( pstTransInfo->bCardType == TRANS_CARDTYPE_MIF_TOUR )
	{
		DebugOut( "#     - 관광권카드\n" );

		pstTransInfo->bMifTourUseCnt = 0;
		if ( boolIsPenalty == TRUE ) pstTransInfo->bMifTourUseCnt++;
		if ( boolIsXfer == FALSE ) pstTransInfo->bMifTourUseCnt++;

		DebugOut( "#     - 이번 승차에서의 관광권카드 사용횟수  : %u 회\n",
			pstTransInfo->bMifTourUseCnt );

		if ( IsSameDate( pstTransInfo->stPrevXferInfo.tEntExtDtime,
				pstTransInfo->stNewXferInfo.tEntExtDtime ) )
		{
			DebugOut( "#     - 동일일 -> 일누적사용횟수에 횟수를 합산\n" );

			pstTransInfo->stNewXferInfo.bMifTourDailyAccUseCnt =
				pstTransInfo->stPrevXferInfo.bMifTourDailyAccUseCnt +
				pstTransInfo->bMifTourUseCnt;
		}
		else
		{
			DebugOut( "#     - 다른일 -> 일누적사용횟수를 이번 횟수로 설정\n" );

			pstTransInfo->stNewXferInfo.bMifTourDailyAccUseCnt =
				pstTransInfo->bMifTourUseCnt;
		}

		if ( pstTransInfo->stNewXferInfo.bMifTourDailyAccUseCnt >
				MAX_MIF_TOUR_DAILY_USE_CNT )
		{
			DebugOut( "#     - 일누적사용횟수 초과\n" );
			return ERR_CARD_PROC_INSUFFICIENT_BAL;
		}

		// 총누적사용횟수 처리
		pstTransInfo->stNewXferInfo.dwMifTourTotalAccUseCnt =
			pstTransInfo->stPrevXferInfo.dwMifTourTotalAccUseCnt +
			pstTransInfo->bMifTourUseCnt;

	}
	// 선불카드의 경우 카드잔액 설정 ///////////////////////////////////////////
	else if ( pstTransInfo->bCardType == TRANS_CARDTYPE_MIF_PREPAY ||
		 pstTransInfo->bCardType == TRANS_CARDTYPE_SC_PREPAY )
	{

		dword dwTempBal = 0;

		DebugOut( "#     - 선불카드\n" );

		memcpy( &dwTempBal, &pstTransInfo->stPrevXferInfo.dwBal,
			sizeof( dwTempBal ) );

		DebugOut( "#     - 최종계산된 요금                      : %5lu (원)\n",
			pstTransInfo->stNewXferInfo.dwFare );
		DebugOut( "#     - 잔액                                 : %5lu (원)\n",
			dwTempBal );

		// 잔액부족 오류가 발생하는 경우 ///////////////////////////////////////
		// 1. 요금이 잔액보다 큰 경우
		// 2. 승차시 잔액이 250원 미만인 경우
		if ( ( int )pstTransInfo->stNewXferInfo.dwFare > dwTempBal ||
			 dwTempBal < MIN_BAL )
		{

			DebugOut( "#     - 잔액부족 오류\n" );

#ifndef TEST_NOT_CHECK_MIN_BAL
			return ERR_CARD_PROC_INSUFFICIENT_BAL;
#else
			pstTransInfo->stNewXferInfo.dwBal =
				pstTransInfo->stPrevXferInfo.dwBal -
				pstTransInfo->stNewXferInfo.dwFare;
#endif
		}
		else
		{

			DebugOut( "#     - 정상적으로 잔액 차감\n" );

			pstTransInfo->stNewXferInfo.dwBal =
				pstTransInfo->stPrevXferInfo.dwBal -
				pstTransInfo->stNewXferInfo.dwFare;
		}
	}
	// 후불카드 및 예치금카드의 경우 카드잔액 설정 /////////////////////////////
	else
	{

		DebugOut( "#     - 후불카드 또는 예치금카드\n" );

		if ( IsSameMonth( pstTransInfo->stPrevXferInfo.tEntExtDtime,
			 pstTransInfo->stNewXferInfo.tEntExtDtime ) )
		{

			DebugOut( "#     - 동일월 -> 잔액에 요금을 합산\n" );

			pstTransInfo->stNewXferInfo.dwBal =
				pstTransInfo->stPrevXferInfo.dwBal +
				pstTransInfo->stNewXferInfo.dwFare;
		}
		else
		{

			DebugOut( "#     - 다른월 -> 잔액을 요금으로 설정\n" );

			pstTransInfo->stNewXferInfo.dwBal =
				pstTransInfo->stNewXferInfo.dwFare;
			pstTransInfo->boolIsChangeMonth = TRUE;
		}
	}

	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       IsXfer                                                   *
*                                                                              *
*  DESCRIPTION:       이번 승차가 환승인지의 여부를 판별하여 리턴하고,         *
*                     환승불가사유코드를 설정한다.                             *
*                                                                              *
*  INPUT PARAMETERS:  pstTransInfo - 카드정보구조체의 포인터                   *
*                                                                              *
*  RETURN/EXIT VALUE: TRUE - 환승                                              *
*                     FALSE - 환승아님                                         *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-11                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static bool IsXfer( TRANS_INFO *pstTransInfo )
{
	// 시간초과 또는 환승횟수 초과이면 환승 X
	if ( !IsValidXferFromXferApplyInfo(
			pstTransInfo->stPrevXferInfo.tEntExtDtime,
			pstTransInfo->stNewXferInfo.tEntExtDtime,
			pstTransInfo->stPrevXferInfo.bAccXferCnt,
			&pstTransInfo->bNonXferCause ) )
	{
		return FALSE;
	}

	// 동일차량이면 환승 X
	if ( pstTransInfo->stPrevXferInfo.dwTermID ==
		 GetDWORDFromASC( gpstSharedInfo->abMainTermID, 9 ) )
	{
		pstTransInfo->bNonXferCause = NOT_XFER_CAUSE_SAME_TERM;
		return FALSE;
	}

	// 하차미태그면 환승 X
	if ( IsEnt( pstTransInfo->stPrevXferInfo.bEntExtType ) )
	{
		pstTransInfo->bNonXferCause = NOT_XFER_CAUSE_NOT_TAG_IN_EXT;
		return FALSE;
	}

	// 현재 또는 이전 교통수단코드가 '광역버스'이면 환승 X
	if ( IsWideAreaBus( gstVehicleParm.wTranspMethodCode ) ||
		 IsWideAreaBus( pstTransInfo->stPrevXferInfo.wTranspMethodCode ) )
	{
		pstTransInfo->bNonXferCause = NOT_XFER_CAUSE_INVALID_TRANSP_METHOD_CODE;
		return FALSE;
	}

	// 이전 교통수단코드가 ('서울버스' || '서울철도')가 아니면 환승 X
	if ( !IsSeoulBusSubway( pstTransInfo->stPrevXferInfo.wTranspMethodCode ) )
	{
		pstTransInfo->bNonXferCause = NOT_XFER_CAUSE_INVALID_TRANSP_METHOD_CODE;
		return FALSE;
	}

	// 단말기그룹코드가 타시도 코드이면 환승 X
	if ( pstTransInfo->bMifTermGroupCode == 0x02 ||
		 pstTransInfo->bMifTermGroupCode == 0x03 ||
		 pstTransInfo->bMifTermGroupCode == 0x04 ||
		 pstTransInfo->bMifTermGroupCode == 0x06 ||
		 pstTransInfo->bMifTermGroupCode == 0x07 )
	{
		pstTransInfo->bNonXferCause = NOT_XFER_CAUSE_DIFF_MIF_TERM_GROUP_CODE;
		return FALSE;
	}

	// 단말기그룹코드가 5이면서 이용시간이 다르면 환승 X
	if ( pstTransInfo->bMifTermGroupCode == 0x05 &&
		 pstTransInfo->tMifEntExtDtime !=
			pstTransInfo->stPrevXferInfo.tEntExtDtime )
	{
		pstTransInfo->bNonXferCause = NOT_XFER_CAUSE_DIFF_MIF_TERM_GROUP_CODE;
		return FALSE;
	}

	// 다인승 환승조건이 달라지면 환승 X
	if ( !IsValidXferFromMultiEnt( pstTransInfo ) )
	{
		pstTransInfo->bNonXferCause = NOT_XFER_CAUSE_DIFF_MULTI_GET_ON_INFO;
		return FALSE;
	}

	return TRUE;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       IsValidXferFromXferApplyInfo                             *
*                                                                              *
*  DESCRIPTION:       환승적용정보에 기반하여 이번 승차가 환승인지의 여부를    *
*                     판별하여 리턴하고, 환승불가사유코드를 설정한다.          *
*                                                                              *
*  INPUT PARAMETERS:  tPrevEntExtDatetime - 이전승하차일시                     *
*                     tNewEntExtDatetime - 신규승하차일시                      *
*                     bAccXferCnt - 환승누적횟수                               *
*                     pbNonXferCause - 환승불가사유코드를 위한 바이트 포인터   *
*                                                                              *
*  RETURN/EXIT VALUE: TRUE - 환승                                              *
*                     FALSE - 환승아님                                         *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-24                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static bool IsValidXferFromXferApplyInfo( time_t tPrevEntExtDatetime,
	time_t tNewEntExtDatetime, byte bAccXferCnt, byte *pbNonXferCause )
{
	byte abBuf[14] = {0, };
	word wNowTime = 0;
//	dword i = 0;
//	bool boolIsSuccessFind = FALSE;
//	bool boolIsChanged = FALSE;
//	byte bTodayCode = '1';
	int nRetagSec = 0;
	dword dwOverTime = 0;

//	static XFER_APPLY_INFO *gpstCurrentXferApplyInfo = NULL;

	TimeT2ASCDtime( tNewEntExtDatetime, abBuf );
	wNowTime = GetWORDFromASC( &abBuf[8], 4 );
/*
	if ( tNewEntExtDatetime != 0 )
	{
		for ( i = 0; i < gstHolidayInfoHeader.dwRecordCnt; i++ )
		{
			if ( memcmp( abBuf, gpstHolidayInfo[i].abHolidayDate,
					sizeof( gpstHolidayInfo[i].abHolidayDate ) ) == 0 )
			{
				bTodayCode = gpstHolidayInfo[i].bHolidayClassCode;
				break;
			}
		}
	}

	// 현 환승적용정보가 유효한지 검사 /////////////////////////////////////////
	if ( gpstCurrentXferApplyInfo == NULL )
		boolIsChanged = TRUE;
	else if ( ( gpstCurrentXferApplyInfo->wXferApplyStartTime <
			  gpstCurrentXferApplyInfo->wXferApplyEndTime ) &&
			 ( wNowTime < gpstCurrentXferApplyInfo->wXferApplyStartTime ||
			  wNowTime >= gpstCurrentXferApplyInfo->wXferApplyEndTime ) )
		boolIsChanged = TRUE;
	else if ( ( gpstCurrentXferApplyInfo->wXferApplyStartTime >
			  gpstCurrentXferApplyInfo->wXferApplyEndTime ) &&
			 ( wNowTime < gpstCurrentXferApplyInfo->wXferApplyStartTime &&
			  wNowTime >= gpstCurrentXferApplyInfo->wXferApplyEndTime ) )
		boolIsChanged = TRUE;

	// 환승적용정보가 변경된 경우 //////////////////////////////////////////////
	if ( boolIsChanged )
	{
		boolIsSuccessFind = FALSE;
		for ( i = 0; i < gstXferApplyInfoHeader.dwRecordCnt &&
				!boolIsSuccessFind; i++ )
		{
			if ( bTodayCode == gpstXferApplyInfo[i].bHolidayClassCode &&
				tNewEntExtDatetime > gpstXferApplyInfo[i].tApplyDtime )
			{
				gpstCurrentXferApplyInfo = &( gpstXferApplyInfo[i] );

				if ( ( ( gpstCurrentXferApplyInfo->wXferApplyStartTime <
					  gpstCurrentXferApplyInfo->wXferApplyEndTime ) &&
					 ( wNowTime >=
					  gpstCurrentXferApplyInfo->wXferApplyStartTime &&
					  wNowTime <
					  gpstCurrentXferApplyInfo->wXferApplyEndTime ) ) ||
					( ( gpstCurrentXferApplyInfo->wXferApplyStartTime >
					  gpstCurrentXferApplyInfo->wXferApplyEndTime ) &&
					 !( wNowTime <
					   gpstCurrentXferApplyInfo->wXferApplyStartTime &&
					   wNowTime >=
					   gpstCurrentXferApplyInfo->wXferApplyEndTime ) ) )
				{
					boolIsSuccessFind = TRUE;
				}
			}
		}

		// 조건에 맞는 환승적용정보가 존재하지 않으므로 무조건 첫탑승 처리
		if ( !boolIsSuccessFind )
		{
			// 환승적용정보 포인터 초기화
			gpstCurrentXferApplyInfo = NULL;

			// 비환승사유 설정
			*pbNonXferCause = NOT_XFER_CAUSE_INVALID_XFER_APPLY_INFO;

			// 환승가능시간 하드코딩
			if ( wNowTime >= 700 && wNowTime < 2100 )
				dwOverTime = 33 * 60;
			else
				dwOverTime = 66 * 60;

			// 재태그시간 계산
			nRetagSec = tNewEntExtDatetime - tPrevEntExtDatetime;

			// 재태그시간에 오류가 있는 경우 10초로 설정
			if ( nRetagSec < 0 )
				nRetagSec = 10;

			// 하드코딩된 환승가능시간과 환승가능횟수 비교하여 환승여부 판단
			if ( nRetagSec <= dwOverTime && bAccXferCnt < 4 )
				return TRUE;

			return FALSE;
		}
	}
*/
	// 재태그시간 계산
	nRetagSec = tNewEntExtDatetime - tPrevEntExtDatetime;

	// 재태그시간에 오류가 있는 경우 10초로 설정
	if ( nRetagSec < 0 )
		nRetagSec = 15;

	// 환승가능시간 하드코딩
	if ( wNowTime >= 700 && wNowTime < 2100 )
		dwOverTime = 33 * 60;
	else
		dwOverTime = 66 * 60;
	if ( nRetagSec > dwOverTime )
	{
		*pbNonXferCause = NOT_XFER_CAUSE_XFER_TIME_ELAPSED;
		return FALSE;
	}

	// 환승가능횟수 하드코딩
	if ( bAccXferCnt >= MAX_XFER_ENABLE_CNT - 1 )
	{
		*pbNonXferCause = NOT_XFER_CAUSE_XFER_CNT_ELAPSED;
		return FALSE;
	}

	return TRUE;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       IsValidXferFromMultiEnt                                  *
*                                                                              *
*  DESCRIPTION:       이전 다인승정보와 현재 다인승정보를 비교하여 환승여부를  *
*                     판별하여 리턴한다.                                       *
*                                                                              *
*  INPUT PARAMETERS:  pstTransInfo - 카드정보구조체                            *
*                                                                              *
*  RETURN/EXIT VALUE: TRUE - 환승                                              *
*                     FALSE - 환승아님                                         *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-24                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static bool IsValidXferFromMultiEnt( TRANS_INFO *pstTransInfo )
{
	bool boolResult = TRUE;
	byte i = 0;
	byte j = 0;

	// 다인승 환승조건 검사 ////////////////////////////////////////////////////
	for ( i = 0; i < 3  && boolResult; i++ )
	{
		boolResult = FALSE;
		for ( j = 0; j < 3; j ++ )
		{
			if ( pstTransInfo->abUserType[i] ==
				pstTransInfo->stPrevXferInfo.abMultiEntInfo[j][USER_TYPE] &&
				pstTransInfo->abUserCnt[i] ==
				pstTransInfo->stPrevXferInfo.abMultiEntInfo[j][USER_CNT] )
			{
				boolResult = TRUE;
			}
		}
	}

	for ( j = 0; j < 3 && boolResult; j++ )
	{
		boolResult = FALSE;
		for ( i = 0; i < 3; i++ )
		{
			if ( pstTransInfo->abUserType[i] ==
				pstTransInfo->stPrevXferInfo.abMultiEntInfo[j][USER_TYPE] &&
				pstTransInfo->abUserCnt[i] ==
				pstTransInfo->stPrevXferInfo.abMultiEntInfo[j][USER_CNT] )
			{
				boolResult = TRUE;
			}
		}
	}

	return boolResult;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       ProcExt                                                  *
*                                                                              *
*  DESCRIPTION:       하차의 경우 요금을 계산하고 신규환승정보를 생성한다.     *
*                                                                              *
*  INPUT PARAMETERS:  pstTransInfo - 카드정보구조체의 포인터                   *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공                                      *
*                     ERR_CARD_PROC_INSUFFICIENT_BAL - 선불카드이면서 잔액이   *
*                         지불해야하는 요금보다 적은 경우                      *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-25                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static short ProcExt( TRANS_INFO *pstTransInfo )
{

	int i = 0;
	int anDiscount[3] = {0, };		// 할인금액 배열
	float afDiscountRate[3] = {0.0, };
									// 할인율 배열
	bool boolIsXfer = FALSE;		// 환승여부
	dword dwDist = 0;				// 이동거리

	DebugOut( "#\n" );
	DebugOut( "# 하차과정\n" );

	// 이전미징수금액이 900보다 크거나 원단위가 0이 아니면 0으로 설정함
	if ( pstTransInfo->stPrevXferInfo.wPrevUnchargedFare >
			MAX_PREV_UNCHARGED_FARE ||
		pstTransInfo->stPrevXferInfo.wPrevUnchargedFare % 10 != 0 )
	{
		pstTransInfo->stPrevXferInfo.wPrevUnchargedFare = 0;
	}

	// 환승여부 판단
	if ( pstTransInfo->stPrevXferInfo.bAccXferCnt > 0 )
		boolIsXfer = TRUE;

	// 할인율 및 할인금액 계산 /////////////////////////////////////////////////
	for ( i = 0; i < 3 &&
		pstTransInfo->stPrevXferInfo.abMultiEntInfo[i][USER_CNT] != 0; i++ )
	{
		GetDisExtraAmtRate( pstTransInfo->bCardType,
				pstTransInfo->stPrevXferInfo.abMultiEntInfo[i][USER_TYPE],
				pstTransInfo->stPrevXferInfo.tEntExtDtime,
				boolIsXfer,
			 	FALSE,
			 	&anDiscount[i],
			 	&afDiscountRate[i] );
		CreateDisExtraTypeID( pstTransInfo->bCardType,
				pstTransInfo->stPrevXferInfo.abMultiEntInfo[i][USER_TYPE],
				pstTransInfo->stNewXferInfo.tEntExtDtime,
				boolIsXfer,
				pstTransInfo->abDisExtraTypeID[i] );
	}

	// 현재이동거리 계산 ///////////////////////////////////////////////////////
	dwDist = CalcDist( pstTransInfo->stPrevXferInfo.abStationID,
							gpstSharedInfo->abNowStationID );

	DebugOut( "#     - 현재이동거리                         : %5lu ( m )\n",
		dwDist );
	DebugOut( "#     - 환승내이동거리                       : %5lu ( m )\n",
		( pstTransInfo->stPrevXferInfo.dwAccDistInXfer + dwDist ) );

	// 사용거리 설정
	pstTransInfo->dwDist = dwDist;

	// 환승이면서 환승내이동거리가 기본거리를 초과하는 경우 ////////////////////
	if ( pstTransInfo->stPrevXferInfo.dwAccDistInXfer + dwDist >
		gstNewFareInfo.dwBaseDist && boolIsXfer )
	{

		int nUseDist = 0;			// 환승내누적이동거리에서 기본거리 초과량
		int nPrevDistUnit = 0;		// 이전단위거리
		int nNewDistUnit = 0;		// 신규단위거리

		DebugOut( "#     - 환승내이동거리가 기본거리를 초과\n" );

		if ( gstNewFareInfo.dwAddedDist != 0 )
		{
			// 이전단위거리 계산
			nUseDist = ( ( int )pstTransInfo->stPrevXferInfo.dwAccDistInXfer -
				( int )gstNewFareInfo.dwBaseDist ) - 1;
			nPrevDistUnit = nUseDist / ( int )gstNewFareInfo.dwAddedDist + 1;

			// 신규단위거리 계산
			// (우선 환승내누적이동거리와 신규이동거리를 모두 합친 거리로 계산)
			nNewDistUnit = ( (
				( int )pstTransInfo->stPrevXferInfo.dwAccDistInXfer +
				( int )dwDist - ( int )gstNewFareInfo.dwBaseDist ) - 1 ) /
				( int )gstNewFareInfo.dwAddedDist + 1;

			// 이전사용거리에서 기본거리를 뺀 거리가 0보다 작거나
			// 1회 환승이면서 ( 도시철도 및 인천환승 ) 후 승차가 아닌 경우
			// 이전단위거리를 0으로 설정
			if ( nUseDist < 0 ||
				( pstTransInfo->stPrevXferInfo.bAccXferCnt == 1 &&
				 ( pstTransInfo->stPrevXferInfo.bEntExtType !=
				   		XFER_ENT_AFTER_SUBWAY &&
				   pstTransInfo->stPrevXferInfo.bEntExtType !=
				   		XFER_ENT_AFTER_INCHEON ) ) )
			{
				nPrevDistUnit = 0;
			}
		}

		// ( 환승내누적이동거리를 합산했던 ) 신규단위거리에서 이전단위거리를 뺌
		nNewDistUnit -= nPrevDistUnit;

		DebugOut( "#     - 신규단위거리                         : %5d\n",
			nNewDistUnit );

		// 신규단위거리가 0보다 큰 경우
		if ( nNewDistUnit > 0 )
		{
			dword dwFare = 0;

			for ( i = 0; i < 3 &&
				pstTransInfo->stPrevXferInfo.abMultiEntInfo[i][USER_CNT] != 0;
				i++ )
			{

				dwFare = nNewDistUnit * gstNewFareInfo.dwAddedFare *
					pstTransInfo->stPrevXferInfo.abMultiEntInfo[i][USER_CNT];

				if ( afDiscountRate[i] != 0 )
					dwFare += dwFare * afDiscountRate[i] / 100;
				else
					dwFare += anDiscount[i];

				pstTransInfo->stNewXferInfo.dwFare += dwFare;
			}

			// 10원이하 절사
			pstTransInfo->stNewXferInfo.dwFare =
				pstTransInfo->stNewXferInfo.dwFare / 10 * 10;
		}
	}

	// 미징수요금을 이전 미징수요금 + 요금으로 설정
	pstTransInfo->stNewXferInfo.wPrevUnchargedFare =
		pstTransInfo->stPrevXferInfo.wPrevUnchargedFare +
		pstTransInfo->stNewXferInfo.dwFare;

	// 요금을 0으로 설정
	pstTransInfo->stNewXferInfo.dwFare = 0;

	// 미징수요금 발생 /////////////////////////////////////////////////////////
	if ( pstTransInfo->stPrevXferInfo.dwAccAmtInXfer +
		pstTransInfo->stNewXferInfo.wPrevUnchargedFare >
		pstTransInfo->stPrevXferInfo.wTotalBaseFareInXfer )
	{

		word wTempUnchargedFare = 0;		// 임시 미징수요금
		wTempUnchargedFare = pstTransInfo->stPrevXferInfo.dwAccAmtInXfer +
							 pstTransInfo->stNewXferInfo.wPrevUnchargedFare -
							 pstTransInfo->stPrevXferInfo.wTotalBaseFareInXfer;
		pstTransInfo->stNewXferInfo.dwFare =
			pstTransInfo->stNewXferInfo.wPrevUnchargedFare - wTempUnchargedFare;
		pstTransInfo->stNewXferInfo.wPrevUnchargedFare = wTempUnchargedFare;
	}
	// 미징수요금 미발생 ///////////////////////////////////////////////////////
	else
	{
		pstTransInfo->stNewXferInfo.dwFare =
			pstTransInfo->stNewXferInfo.wPrevUnchargedFare;
		pstTransInfo->stNewXferInfo.wPrevUnchargedFare = 0;
	}

	DebugOut( "#     - 환승내이용수단기본요금의합           : %5u (원)\n",
		pstTransInfo->stPrevXferInfo.wTotalBaseFareInXfer );
	DebugOut( "#     - 요금                                 : %5lu (원)\n",
		pstTransInfo->stNewXferInfo.dwFare );
	DebugOut( "#     - 미징수요금                           : %5u (원)\n",
		pstTransInfo->stNewXferInfo.wPrevUnchargedFare );

	// 교통수단/승하차유형 설정 ////////////////////////////////////////////////
	pstTransInfo->stNewXferInfo.bEntExtType =
		pstTransInfo->stPrevXferInfo.bEntExtType | XFER_EXT;

	// 환승누적횟수 ////////////////////////////////////////////////////////////
	pstTransInfo->stNewXferInfo.bAccXferCnt =
		pstTransInfo->stPrevXferInfo.bAccXferCnt;

	// 환승일련번호 ////////////////////////////////////////////////////////////
	pstTransInfo->stNewXferInfo.bXferSeqNo =
		pstTransInfo->stPrevXferInfo.bXferSeqNo;

	// 정류장ID ////////////////////////////////////////////////////////////////
	memcpy( pstTransInfo->stNewXferInfo.abStationID,
		gpstSharedInfo->abNowStationID, 7 );

	// 교통수단유형 ////////////////////////////////////////////////////////////
	pstTransInfo->stNewXferInfo.wTranspMethodCode =
		gstVehicleParm.wTranspMethodCode;

	// 환승내누적이동거리 //////////////////////////////////////////////////////
	pstTransInfo->stNewXferInfo.dwAccDistInXfer =
		pstTransInfo->stPrevXferInfo.dwAccDistInXfer + dwDist;

	// 환승내누적이용금액 //////////////////////////////////////////////////////
	pstTransInfo->stNewXferInfo.dwAccAmtInXfer =
		pstTransInfo->stPrevXferInfo.dwAccAmtInXfer +
		pstTransInfo->stNewXferInfo.dwFare;

	// 터미널ID ////////////////////////////////////////////////////////////////
	pstTransInfo->stNewXferInfo.dwTermID =
		GetDWORDFromASC( gpstSharedInfo->abMainTermID, 9 );

	// 다인승거래내역 //////////////////////////////////////////////////////////
	for ( i = 0; i < 3; i++ )
	{
		pstTransInfo->stNewXferInfo.abMultiEntInfo[i][USER_TYPE] =
			pstTransInfo->stPrevXferInfo.abMultiEntInfo[i][USER_TYPE];
		pstTransInfo->stNewXferInfo.abMultiEntInfo[i][USER_CNT] =
			pstTransInfo->stPrevXferInfo.abMultiEntInfo[i][USER_CNT];
	}

	// 총누적승차횟수 //////////////////////////////////////////////////////////
	pstTransInfo->stNewXferInfo.wTotalAccEntCnt =
		pstTransInfo->stPrevXferInfo.wTotalAccEntCnt;

	// 총누적사용금액 //////////////////////////////////////////////////////////
	pstTransInfo->stNewXferInfo.dwTotalAccUseAmt =
		pstTransInfo->stPrevXferInfo.dwTotalAccUseAmt +
		pstTransInfo->stNewXferInfo.dwFare;

	// 승객 이전거래시 지불된 최대기본요금 /////////////////////////////////////
	memcpy( pstTransInfo->stNewXferInfo.awMaxBaseFare,
		pstTransInfo->stPrevXferInfo.awMaxBaseFare,
		sizeof( pstTransInfo->stNewXferInfo.awMaxBaseFare ) );

	// 환승내이용수단기본요금의합 //////////////////////////////////////////////
	pstTransInfo->stNewXferInfo.wTotalBaseFareInXfer =
		pstTransInfo->stPrevXferInfo.wTotalBaseFareInXfer;

	// 페널티요금 //////////////////////////////////////////////////////////////
	pstTransInfo->stNewXferInfo.wPrevPenaltyFare = 0;

	// 구선불카드 이전승하차구분의 하차여부 ////////////////////////////////////
	pstTransInfo->stNewXferInfo.boolIsPrevExt =
		!IsEnt( pstTransInfo->stPrevXferInfo.bEntExtType );

	// 관광권카드의 경우 카드잔액 설정 /////////////////////////////////////////
	if ( pstTransInfo->bCardType == TRANS_CARDTYPE_MIF_TOUR )
	{
		DebugOut( "#     - 관광권카드\n" );

		pstTransInfo->bMifTourUseCnt = 0;

		// 일누적사용횟수
		if ( IsSameDate( pstTransInfo->stPrevXferInfo.tEntExtDtime,
				pstTransInfo->stNewXferInfo.tEntExtDtime ) )
		{
			DebugOut( "#     - 동일일 -> 일누적사용횟수를 동일하게 설정\n" );

			pstTransInfo->stNewXferInfo.bMifTourDailyAccUseCnt =
				pstTransInfo->stPrevXferInfo.bMifTourDailyAccUseCnt;
		}
		else
		{
			DebugOut( "#     - 다른일 -> 일누적사용횟수를 0으로 초기화\n" );

			pstTransInfo->stNewXferInfo.bMifTourDailyAccUseCnt =
				pstTransInfo->bMifTourUseCnt;
		}


		// 총누적사용횟수
		pstTransInfo->stNewXferInfo.dwMifTourTotalAccUseCnt =
			pstTransInfo->stPrevXferInfo.dwMifTourTotalAccUseCnt;

	}
	// 선불카드의 경우 카드잔액 설정 ///////////////////////////////////////////
	else if ( pstTransInfo->bCardType == TRANS_CARDTYPE_MIF_PREPAY ||
		pstTransInfo->bCardType == TRANS_CARDTYPE_SC_PREPAY )
	{
		dword dwTempBal = 0;

		DebugOut( "#     - 선불카드\n" );

		memcpy( &dwTempBal, &pstTransInfo->stPrevXferInfo.dwBal, 4 );

		DebugOut( "#     - 최종계산된 요금                      : %5lu (원)\n",
			pstTransInfo->stNewXferInfo.dwFare );
		DebugOut( "#     - 잔액                                 : %5lu (원)\n",
			dwTempBal );

		if ( ( int )pstTransInfo->stNewXferInfo.dwFare > dwTempBal )
		{

			DebugOut( "#     - 잔액부족 오류\n" );

			return ERR_CARD_PROC_INSUFFICIENT_BAL;
		}
		else
		{
			DebugOut( "#     - 정상적으로 잔액 차감\n" );

			pstTransInfo->stNewXferInfo.dwBal =
				pstTransInfo->stPrevXferInfo.dwBal -
				pstTransInfo->stNewXferInfo.dwFare;
		}
	}
	// 후불카드 및 예치금카드의 경우 카드잔액 설정 /////////////////////////////
	else
	{

		DebugOut( "#     - 후불카드 또는 예치금카드\n" );

		if ( IsSameMonth( pstTransInfo->stPrevXferInfo.tEntExtDtime,
			pstTransInfo->stNewXferInfo.tEntExtDtime ) )
		{

			DebugOut( "#     - 동일월 -> 잔액에 요금을 합산\n" );

			pstTransInfo->stNewXferInfo.dwBal =
				pstTransInfo->stPrevXferInfo.dwBal +
				pstTransInfo->stNewXferInfo.dwFare;
		}
		else
		{

			DebugOut( "#     - 다른월 -> 잔액을 요금으로 설정\n" );

			pstTransInfo->stNewXferInfo.dwBal =
				pstTransInfo->stNewXferInfo.dwFare;
			pstTransInfo->boolIsChangeMonth = TRUE;
		}
	}

	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       CalcDist                                                 *
*                                                                              *
*  DESCRIPTION:       양 정류장사이의 거리를 정류장정보로부터 계산하여         *
*                     리턴한다.                                                *
*                                                                              *
*  INPUT PARAMETERS:  abEntStationID - 승차정류장ID                            *
*                     abExtStationID - 하차정류장ID                            *
*                                                                              *
*  RETURN/EXIT VALUE: dword - 양 정류장사이의 거리                             *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-25                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static dword CalcDist( byte *abEntStationID, byte *abExtStationID )
{
	word i = 0;
	word j = 0;
	dword dwMaxDist = 0;
	dword dwEntDist = 0;
	dword dwExtDist = 0;
	int nDist = 0;
	dword dwAnotherDist = 0;
	bool boolIsSuccessFind = FALSE;

	// 정류장정보 로딩에 실패한 경우 거리는 0을 리턴
	if ( gstStationInfoHeader.dwRecordCnt == 0 )
	{
		return 0;
	}

	if ( abEntStationID == NULL || abExtStationID == NULL )
	{
		DebugOut( "[CalcDist] 거리계산시 정류장ID가 NULL로 입력됨\n" );
		ErrRet( ERR_CARD_PROC_STATION_ID );
		return 0;
	}

	dwMaxDist = gpstStationInfo[gstStationInfoHeader.dwRecordCnt - 1].
					dwDistFromFirstStation;

	boolIsSuccessFind = FALSE;
	for ( i = 0; i < gstStationInfoHeader.dwRecordCnt && !boolIsSuccessFind;
		i++ )
	{
		if ( memcmp( abEntStationID, gpstStationInfo[i].abStationID,
			 sizeof( gpstStationInfo[j].abStationID ) ) == 0 )
		{
			dwEntDist = gpstStationInfo[i].dwDistFromFirstStation;
			boolIsSuccessFind = TRUE;
		}
	}

	if ( !boolIsSuccessFind )
	{
		DebugOut( "[CalcDist] 거리계산시 승차정류장ID가 존재하지 않음\n" );
		ErrRet( ERR_CARD_PROC_STATION_ID );
		return 0;
	}

	boolIsSuccessFind = FALSE;
	for ( j = 0; j < gstStationInfoHeader.dwRecordCnt && !boolIsSuccessFind;
		j++ )
	{
		if ( memcmp( abExtStationID, gpstStationInfo[j].abStationID,
			 sizeof( gpstStationInfo[j].abStationID ) ) == 0 )
		{
			dwExtDist = gpstStationInfo[j].dwDistFromFirstStation;
			boolIsSuccessFind = TRUE;
		}
	}

	if ( !boolIsSuccessFind ) {
		DebugOut( "[CalcDist] 거리계산시 승차정류장ID가 존재하지 않음\n" );
		ErrRet( ERR_CARD_PROC_STATION_ID );
		return 0;
	}

	nDist = dwExtDist - dwEntDist;

	if ( nDist < 0 )
	{
		dwAnotherDist = dwMaxDist - dwEntDist + dwExtDist;	// 또다른 패스
		if ( ( -1 * nDist ) > dwAnotherDist )
		{
			nDist = dwAnotherDist;
			DebugOut( "#     - 종점경유 보정 (최대거리 : %lu, ", dwMaxDist );
			DebugOut( "승차거리 : %lu, 하차거리 : %lu)\n", dwEntDist,
				dwExtDist );
		}
		else
		{
			nDist *= -1;
			DebugOut( "#     - 역주행 보정   (최대거리 : %lu, ", dwMaxDist );
			DebugOut( "승차거리 : %lu, 하차거리 : %lu)\n", dwEntDist,
				dwExtDist );
		}
	}

	return nDist;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       CalcFare                                                 *
*                                                                              *
*  DESCRIPTION:       입력된 조건에 해당하는 기본요금 및 할인율/할인금액을     *
*                     가져와 요금을 계산하여 리턴한다.                         *
*                                                                              *
*  INPUT PARAMETERS:  bCardType - 카드유형                                     *
*                     bUserType - 사용자유형                                   *
*                     wTranspMethodCode - 교통수단코드                         *
*                     tEntExtDtime - 이용시간                                  *
*                     boolIsXfer - 환승여부                                    *
*                     boolIsBaseFare - 기본요금여부                            *
*                                                                              *
*  RETURN/EXIT VALUE: dword - 계산된 요금                                      *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-24                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static dword CalcFare( byte bCardType, byte bUserType, word wTranspMethodCode,
	time_t tEntExtDtime, bool boolIsXfer, bool boolIsBaseFare )
{
	dword dwFare = 0;
	int nDiscount = 0;
	float fDiscountRate = 0.0;

	// 기본요금을 가져옴 ///////////////////////////////////////////////////////
	dwFare = GetBaseFare( bCardType, bUserType, wTranspMethodCode );

	// 할인율 및 할인금액을 가져옴 /////////////////////////////////////////////
	GetDisExtraAmtRate( bCardType, bUserType, tEntExtDtime, boolIsXfer,
		boolIsBaseFare, &nDiscount, &fDiscountRate );

	// 광역버스이면 할인할증금액 사용
	if ( IsWideAreaBus( wTranspMethodCode ) )
		dwFare += nDiscount;
	// 광역버스가아니면 할인할증률 사용
	else
		dwFare += dwFare * fDiscountRate / 100;

	return dwFare;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       GetBaseFare                                              *
*                                                                              *
*  DESCRIPTION:       입력된 조건에 해당하는 기본요금을 리턴한다.              *
*                                                                              *
*  INPUT PARAMETERS:  bCardType - 카드유형                                     *
*                     bUserType - 사용자유형                                   *
*                     wTranspMethodCode - 교통수단코드                         *
*                                                                              *
*  RETURN/EXIT VALUE: dword - 기본요금                                         *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-24                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
dword GetBaseFare( byte bCardType, byte bUserType, word wTranspMethodCode )
{
	dword dwBaseFare = 0;				// 기본요금

	// 현금의 경우 /////////////////////////////////////////////////////////////
	if ( bCardType == TRANS_CARDTYPE_CASH ||
		 bCardType == TRANS_CARDTYPE_STUDENT_TOKEN )
	{
		switch ( bUserType )
		{
			case USERTYPE_ADULT:
			case USERTYPE_TEST:
				dwBaseFare = gstNewFareInfo.dwAdultCashEntFare;
				break;
			case USERTYPE_YOUNG:
				dwBaseFare = gstNewFareInfo.dwYoungCashEntFare;
				break;
			case USERTYPE_CHILD:
				dwBaseFare = gstNewFareInfo.dwChildCashEntFare;
				break;
		}
	}
	// 교통수단유형이 현재차량의 교통수단유형과 동일한 경우 ////////////////////
	else if ( wTranspMethodCode == gstVehicleParm.wTranspMethodCode )
	{
		dwBaseFare = gstNewFareInfo.dwBaseFare;

#ifdef TEST_TRANS_0_WON
		dwBaseFare = 0;
#endif

#ifdef TEST_TRANS_10_WON
		dwBaseFare = 10;
#endif
	}
	// 그 외의 경우 ////////////////////////////////////////////////////////////
	else
	{
		dwBaseFare = GetHardCodedBaseFare( wTranspMethodCode );
	}

	return dwBaseFare;
}

dword GetHardCodedBaseFare( word wTranspMethodCode )
{
	dword dwBaseFare = 0;

	switch ( wTranspMethodCode )
	{
		case 101:		dwBaseFare = 800;		break;
		case 102:		dwBaseFare = 800;		break;
		case 103:		dwBaseFare = 800;		break;
		case 104:		dwBaseFare = 500;		break;
		case 105:		dwBaseFare = 500;		break;
		case 110:		dwBaseFare = 800;		break;
		case 115:		dwBaseFare = 800;		break;
		case 120:		dwBaseFare = 800;		break;
		case 121:		dwBaseFare = 500;		break;
		case 130:		dwBaseFare = 1400;		break;
		case 140:		dwBaseFare = 500;		break;
		case 200:		dwBaseFare = 800;		break;
		case 201:		dwBaseFare = 800;		break;
		case 202:		dwBaseFare = 800;		break;
		case 203:		dwBaseFare = 800;		break;
		case 204:		dwBaseFare = 800;		break;
		case 122:		dwBaseFare = 600;		break;
		case 131:		dwBaseFare = 1200;		break;
		case 151:		dwBaseFare = 600;		break;
		default:
			dwBaseFare = 0;
			DebugOut( "교통코드에 맞는 기초운임이 없음\n" );
	}

	return dwBaseFare;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       GetDisExtraAmtRate                                       *
*                                                                              *
*  DESCRIPTION:       입력된 조건에 해당하는 할인할증률/할인할증금액을         *
*                     가져온다.                                                *
*                                                                              *
*  INPUT PARAMETERS:  bCardType - 카드유형                                     *
*                     bUserType - 사용자유형                                   *
*                     tEntExtDtime - 이용시간                                  *
*                     boolIsXfer - 환승여부                                    *
*                     boolIsBaseFare - 기본요금여부                            *
*                     pnDisExtraAmt - 할인할증금액을 가져오기 위한 포인터      *
*                     pfDiscExtraRate - 할인할증률을 가져오기 위한 포인터      *
*                                                                              *
*  RETURN/EXIT VALUE: void                                                     *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-24                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static void GetDisExtraAmtRate( byte bCardType, byte bUserType,
	time_t tEntExtDtime, bool boolIsXfer, bool boolIsBaseFare,
	int *pnDisExtraAmt, float *pfDiscExtraRate )
{
	bool boolSearchSuccess = FALSE;
	byte abDisExtraTypeID[7] = {0, };

	*pnDisExtraAmt = 0;
	*pfDiscExtraRate = 0.0;

	// 신광역버스의 경우 ///////////////////////////////////////////////////////
	if ( gstVehicleParm.wTranspMethodCode == 131 )
	{
		switch ( bUserType )
		{
			case USERTYPE_ADULT:
				*pnDisExtraAmt = 0;
				break;
			case USERTYPE_YOUNG:
				*pnDisExtraAmt = -200;
				break;
			case USERTYPE_CHILD:
				*pnDisExtraAmt = -300;
				break;
			default:
				*pnDisExtraAmt = 0;
				break;
		}
		*pfDiscExtraRate = 0.0;
		return;
	}

	// 할인할증유형ID를 가져옴 /////////////////////////////////////////////////
	CreateDisExtraTypeID( bCardType,
					bUserType,
					tEntExtDtime,
					boolIsXfer,
					abDisExtraTypeID );

	// 할인할증정보에서 할인율 및 할인액 검색 //////////////////////////////////
	boolSearchSuccess = SearchDisExtraInfo( abDisExtraTypeID,
		boolIsBaseFare, pnDisExtraAmt, pfDiscExtraRate );

	// 검색에 실패한 경우 하드코딩된 값으로 할인율 및 할인금액 설정 ////////////
	if ( !boolSearchSuccess )
	{
		switch ( bUserType )
		{
			case USERTYPE_YOUNG:
				*pnDisExtraAmt = -280;
				*pfDiscExtraRate = -20.0;
				break;
			case USERTYPE_CHILD:
				*pnDisExtraAmt = -400;
				*pfDiscExtraRate = -50.0;
				break;
			case USERTYPE_ADULT:
			default:
				*pnDisExtraAmt = 0;
				*pfDiscExtraRate = 0.0;
				break;
		}
	}
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       CreateDisExtraTypeID                                     *
*                                                                              *
*  DESCRIPTION:       입력된 조건에 해당하는 할인할증유형ID를 생성한다.        *
*                                                                              *
*  INPUT PARAMETERS:  bCardType - 카드유형                                     *
*                     bUserType - 사용자유형                                   *
*                     tEntExtDtime - 이용시간                                  *
*                     boolIsXfer - 환승여부                                    *
*                     abDisExtraTypeID - 할인할증유형ID를 가져오기 위한 포인터 *
*                                                                              *
*  RETURN/EXIT VALUE: void                                                     *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-24                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
void CreateDisExtraTypeID( byte bCardType, byte bUserType,
	time_t tEntExtDtime, bool boolIsXfer, byte *abDisExtraTypeID )
{
	int i = 0;
	byte hour = 0;
	byte abEntExtDate[8] = {0, };

	abDisExtraTypeID[0] = '1';
	if ( tEntExtDtime != 0 )
	{
		TimeT2ASCDate( tEntExtDtime, abEntExtDate );
		for ( i = 0; i < gstHolidayInfoHeader.dwRecordCnt; i++ )
		{
			if ( memcmp( abEntExtDate, gpstHolidayInfo[i].abHolidayDate,
					sizeof( gpstHolidayInfo[i].abHolidayDate ) ) == 0 )
			{
				abDisExtraTypeID[0] = gpstHolidayInfo[i].bHolidayClassCode;
				break;
			}
		}
	}

	hour = GetHourFromTimeT( tEntExtDtime );
	if ( hour == 0 ) hour = 24;	// 00시는 사용안하기로 해서 추가
	abDisExtraTypeID[1] = hour / 10 + '0';
	abDisExtraTypeID[2] = hour % 10 + '0';

	if ( boolIsXfer )
		abDisExtraTypeID[3] = '1';
	else
		abDisExtraTypeID[3] = '0';

	if ( bCardType == TRANS_CARDTYPE_MIF_PREPAY ||
		 bCardType == TRANS_CARDTYPE_DEPOSIT ||
		 bCardType == TRANS_CARDTYPE_MIF_POSTPAY )
	{
		abDisExtraTypeID[1] = '0';
		abDisExtraTypeID[2] = '0';
	}

	if ( bCardType == TRANS_CARDTYPE_CASH ||
		 bCardType == TRANS_CARDTYPE_STUDENT_TOKEN )
	{
		abDisExtraTypeID[1] = '0';
		abDisExtraTypeID[2] = '0';
		abDisExtraTypeID[3] = '0';
	}

	abDisExtraTypeID[4] = bUserType / 10 + '0';
	abDisExtraTypeID[5] = bUserType % 10 + '0';
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       SearchDisExtraInfo                                       *
*                                                                              *
*  DESCRIPTION:       할인할증정보를 검색하여 할인할증금액과 할인할증률을      *
*                     리턴한다.                                                *
*                                                                              *
*  INPUT PARAMETERS:  abDisExtraTypeID - 할인할증유형ID                        *
*                     boolIsBaseFare - 기본요금여부                            *
*                     pnDisExtraAmt - 할인할증금액을 가져오기 위한 포인터      *
*                     pfDisExtraRate - 할인할증률을 가져오기 위한 포인터       *
*                                                                              *
*  RETURN/EXIT VALUE: TRUE - 검색성공                                          *
*                     FALSE - 검색실패                                         *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-24                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static bool SearchDisExtraInfo( byte *abDisExtraTypeID, bool boolIsBaseFare,
	int *pnDisExtraAmt, float *pfDisExtraRate )
{
	dword i = 0;
	byte bBaseAdd = '0';

	*pnDisExtraAmt = 0;
	*pfDisExtraRate = 0.0;

	// 할인할증정보의 로드에 실패한 경우 무조건 FALSE 리턴
	if ( gstDisExtraInfoHeader.dwRecordCnt == 0 )
	{
		return FALSE;
	}

	if ( boolIsBaseFare == TRUE )
		bBaseAdd = '1';
	else
		bBaseAdd = '2';

	// 할인할증유형ID 및 할인할증적용기준코드(기본요금/추가요금 필드)가 같은 ///
	// 할인할증정보를 검색 /////////////////////////////////////////////////////
	for ( i = 0; i < gstDisExtraInfoHeader.dwRecordCnt; i++ )
		if ( memcmp( abDisExtraTypeID, gpstDisExtraInfo[i].abDisExtraTypeID, 6 )
			== 0 && gpstDisExtraInfo[i].bDisExtraApplyCode == bBaseAdd )
		{
			*pnDisExtraAmt = gpstDisExtraInfo[i].nDisExtraAmt;
			*pfDisExtraRate = gpstDisExtraInfo[i].fDisExtraRate;
			return TRUE;
		}

	// 위의 검색에서 실패하는 경우 /////////////////////////////////////////////
	// 기본요금/추가요금 필드가 default( '0' ) 값을 가진 할인할증정보를 검색 ///
	for ( i = 0; i < gstDisExtraInfoHeader.dwRecordCnt; i++ )
		if ( memcmp( abDisExtraTypeID, gpstDisExtraInfo[i].abDisExtraTypeID,
			 sizeof( gpstDisExtraInfo[i].abDisExtraTypeID ) ) == 0 &&
			 gpstDisExtraInfo[i].bDisExtraApplyCode == '0' )
		{
			*pnDisExtraAmt = gpstDisExtraInfo[i].nDisExtraAmt;
			*pfDisExtraRate = gpstDisExtraInfo[i].fDisExtraRate;
			return TRUE;
		}

	return FALSE;
}

static short ProcCityTourEnt( TRANS_INFO *pstTransInfo )
{
	byte i = 0;
	byte abNowDate[8] = {0, };

	DebugOut( "\n" );
	DebugOut( "#############################################################" );
	DebugOut( "##################\n" );
	DebugOut( "# 시티투어버스 처리\n" );
	DebugOut( "#\n" );

	// 교통수단/승하차유형 /////////////////////////////////////////////////////
	pstTransInfo->stNewXferInfo.bEntExtType = XFER_ENT;

	// 정류장ID ////////////////////////////////////////////////////////////////
	memcpy( pstTransInfo->stNewXferInfo.abStationID,
		gpstSharedInfo->abNowStationID, 7 );

	// 다인승거래내역 //////////////////////////////////////////////////////////
	for ( i = 0; i < 3; i++ )
	{
		pstTransInfo->stNewXferInfo.abMultiEntInfo[i][USER_TYPE] =
			pstTransInfo->abUserType[i];
		pstTransInfo->stNewXferInfo.abMultiEntInfo[i][USER_CNT] =
			pstTransInfo->abUserCnt[i];
	}

	// 각 승객별로 기본요금계산 및 승객 할인할증유형ID 설정 ////////////////////
	for ( i = 0; i < 3 && pstTransInfo->abUserCnt[i] != 0; i++ )
	{
		// 승객 할인할증유형ID /////////////////////////////////////////////////
		CreateDisExtraTypeID( pstTransInfo->bCardType,
			pstTransInfo->abUserType[i],
			pstTransInfo->stNewXferInfo.tEntExtDtime,
			TRUE,
			pstTransInfo->abDisExtraTypeID[i] );
	}

	// 신선불카드의 경우
	if ( pstTransInfo->bCardType == TRANS_CARDTYPE_SC_PREPAY )
	{
		DebugOut( "# 신선불카드\n" );

		// 요금 ////////////////////////////////////////////////////////////////
		pstTransInfo->stNewXferInfo.dwFare =
			gstCityTourBusTicketInfo.dwTicketAmt;

		DebugOut( "#     - 최종계산된 요금                      : %5lu (원)\n",
			pstTransInfo->stNewXferInfo.dwFare );
		DebugOut( "#     - 잔액                                 : %5lu (원)\n",
			pstTransInfo->stPrevXferInfo.dwBal );

		if ( pstTransInfo->stNewXferInfo.dwFare >
			 pstTransInfo->stPrevXferInfo.dwBal )
		{

			DebugOut( "#     - 잔액부족 오류\n" );

			return ERR_CARD_PROC_INSUFFICIENT_BAL;
		}
		else
		{

			DebugOut( "#     - 정상적으로 잔액 차감\n" );

			// 잔액 ////////////////////////////////////////////////////////////
			pstTransInfo->stNewXferInfo.dwBal =
				pstTransInfo->stPrevXferInfo.dwBal -
				pstTransInfo->stNewXferInfo.dwFare;
		}
	}
	// 관광권카드의 경우
	else
	{
		DebugOut( "# 관광권카드\n" );

		// 최초사용일 이후 사용기간이 지난 카드의 경우
		// '카드 유효기간이 지났습니다' 음성 출력
		if ( IsValidMifTourExpiryDate( pstTransInfo->tMifTourFirstUseDtime,
				pstTransInfo->wMifTourCardType,
				pstTransInfo->stNewXferInfo.tEntExtDtime ) == FALSE )
		{
			printf( "[TransProc] 최초사용일시 이후 카드유형에 따른 " );
			printf( "사용기한을 초과하였으므로 카드사용이 불가함\n" );
			return ERR_CARD_PROC_EXPIRED_CARD;
		}

		// 만기일이 지난 카드의 경우
		// '카드 유효기간이 지났습니다' 음성 출력
		TimeT2ASCDate( pstTransInfo->stNewXferInfo.tEntExtDtime, abNowDate);
		if ( memcmp( abNowDate, pstTransInfo->abMifTourExpiryDate, 8 ) > 0 )
		{
			printf( "[TransProc] 관광권카드 만기일 이후 사용 오류\n" );
			return ERR_CARD_PROC_EXPIRED_CARD;
		}

		pstTransInfo->bMifTourUseCnt = 0;

		// 일누적사용횟수 //////////////////////////////////////////////////////
		if ( IsSameDate( pstTransInfo->stPrevXferInfo.tEntExtDtime,
				pstTransInfo->stNewXferInfo.tEntExtDtime ) )
		{
			DebugOut( "#     - 동일일 -> 일누적사용횟수에 횟수를 합산\n" );

			pstTransInfo->stNewXferInfo.bMifTourDailyAccUseCnt =
				pstTransInfo->stPrevXferInfo.bMifTourDailyAccUseCnt;
		}
		else
		{
			DebugOut( "#     - 다른일 -> 일누적사용횟수를 이번 횟수로 설정\n" );

			pstTransInfo->stNewXferInfo.bMifTourDailyAccUseCnt =
				pstTransInfo->bMifTourUseCnt;
		}

		// 총누적사용횟수 //////////////////////////////////////////////////////
		pstTransInfo->stNewXferInfo.dwMifTourTotalAccUseCnt =
			pstTransInfo->stPrevXferInfo.dwMifTourTotalAccUseCnt;
	}

	return SUCCESS;
}

