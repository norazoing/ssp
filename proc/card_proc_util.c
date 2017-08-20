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
*  PROGRAM ID :       card_proc_util.c                                         *
*                                                                              *
*  DESCRIPTION:       카드처리시 필요한 유틸성 기능들을 제공한다.              *
*                                                                              *
*  ENTRY POINT:       bool IsValidPrepayIssuer( byte *abCardNo );              *
*                     bool IsValidPostpayCardNo( byte *abCardNo,               *
*                         byte bIssuerCode );                                  *
*                     bool IsSamsungAmexCard( byte *abCardNo );                *
*                     bool IsBCCardInvalidBIN( byte *abCardNo );               *
*                     bool IsValidPostpayIssuer( byte *abCardNo );             *
*                     bool IsValidIssuerValidPeriod( byte *abCardNo,           *
*                         byte *abExpiryDate );                                *
*                     bool IsValidMifPrepayIssueDate( byte *abCardNo,          *
*                         byte *abIssueDate );                                 *
*                     bool IsValidSCPrepayIssueDate( byte *abCardNo,           *
*                         time_t tIssueDate );                                 *
*                     short SearchCardErrLog( TRANS_INFO *pstTransInfo );      *
*                     void InitCardErrLog( void );                             *
*                     void AddCardErrLog( short sResult,                       *
*                         TRANS_INFO *pstTransInfo );                          *
*                     void DeleteCardErrLog( byte *abCardNo );                 *
*                     void PrintTransInfo( TRANS_INFO *pstTransInfo );         *
*                     void InitCardNoLog( void );                              *
*                     void AddCardNoLog( byte *abCardNo );                     *
*                     bool IsExistCardNoLog( byte *abCardNo );                 *
*                     bool IsValidSCPurseInfo(                                 *
*                         SC_EF_PURSE_INFO *pstPurseInfo );                    *
*                     void InitWatch( void );                                  *
*                     double StopWatch( void );                                *
*                     void PrintXferInfo(                                      *
*                         COMMON_XFER_DATA *pstCommonXferData );               *
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
* 2006/01/11 F/W Dev Team Boohyeon Jeon   Initial Release                      *
*                                                                              *
*******************************************************************************/

#include "../system/bus_type.h"
#include "card_proc.h"

#include "card_proc_util.h"

// 오류LOG 및 카드번호LOG 최대 갯수 정의 ///////////////////////////////////////
#define MAX_CARD_NO_LOG				10		// 카드번호LOG 최대 갯수

// 카드번호LOG 관련 전역변수 ///////////////////////////////////////////////////
static byte gabCardNoLog[MAX_CARD_NO_LOG][20];
static int gnCardNoLogCount = -1;

//------------------------------------------------------------------------------
// 시간 측정을 위한 변수 선언
static struct timeval stTime = {0L, 0L};
static int nWatchCount = 0;
static double dTotDuration = 0.0;

static short SearchErrLog( byte *abCardNo );
static void AddErrLog( TRANS_INFO *pstTransInfo );
static void UpdateErrLog( byte bIndex, TRANS_INFO *pstTransInfo );
static void PrintErrLog( void );

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       IsValidPrepayIssuer                                      *
*                                                                              *
*  DESCRIPTION:       선불발행사정보에 카드의 prefix가 존재하는지의 여부를     *
*                     리턴한다.                                                *
*                                                                              *
*  INPUT PARAMETERS:  abCardNo - 카드번호                                      *
*                                                                              *
*  RETURN/EXIT VALUE: TRUE - 발행사 존재                                       *
*                     FALSE - 발행사 미존재                                    *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-23                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
bool IsValidPrepayIssuer( byte *abCardNo )
{
	dword i = 0;					// 반복문에서 사용되는 임시 변수

	// 선불발행사정보의 로드에 실패한 경우 무조건 TRUE 리턴
	if ( gstPrepayIssuerInfoHeader.dwRecordCnt == 0 )
	{
		return TRUE;
	}

	for ( i = 0; i < gstPrepayIssuerInfoHeader.dwRecordCnt; i++ )
	{
		if ( memcmp( abCardNo, gpstPrepayIssuerInfo[i].abPrepayIssuerID, 7 )
			== 0 )
		{
			return TRUE;
		}
	}

	return FALSE;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       IsValidPostpayCardNo                                     *
*                                                                              *
*  DESCRIPTION:       유효한 카드번호인지의 여부를 리턴한다.                   *
*                                                                              *
*  INPUT PARAMETERS:  abCardNo - 카드번호                                      *
*                     bIssuerCode - 발행사코드                                 *
*                                                                              *
*  RETURN/EXIT VALUE: TRUE - 유효한 카드번호                                   *
*                     FALSE - 카드번호 오류                                    *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-23                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
bool IsValidPostpayCardNo( byte *abCardNo, byte bIssuerCode )
{
	bool boolResult = FALSE;

	if ( strlen( abCardNo ) < 15 )
		return FALSE;

	switch ( abCardNo[0] )
	{
		case '9':
			switch ( bIssuerCode )
			{
				case 0x00:
				case 0x02:
				case ISS_SS:
					if ( memcmp( abCardNo, "941009", 6 ) == 0 ||
						 memcmp( abCardNo, "941010", 6 ) == 0 )
					{
						boolResult = IsValidISOCardNo( abCardNo );
					}
					else
					{
						boolResult = IsValidSamsungLocalCardNo( abCardNo );
					}
					break;
				case ISS_LG:
					boolResult = IsValidLGLocalCardNo( abCardNo );
					break;
				default:
					boolResult = IsValidISOCardNo( abCardNo );
					break;
			}
			break;
		case '3':
			if ( IsSamsungAmexCard( abCardNo ) )
			{
				boolResult = IsValidAmexCardNo( abCardNo );
			}
			else
			{
				boolResult = IsValidISOCardNo( abCardNo );
			}
			break;
		default:
			boolResult = IsValidISOCardNo( abCardNo );
			break;
	}

	return boolResult;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       IsSamsungAmexCard                                        *
*                                                                              *
*  DESCRIPTION:       BIN코드를 이용하여 삼성AMEX카드 여부를 판별한다.         *
*                                                                              *
*  INPUT PARAMETERS:  abCardNo - 카드번호                                      *
*                                                                              *
*  RETURN/EXIT VALUE: TRUE - 삼성AMEX카드                                      *
*                     FALSE - 삼성AMEX카드 아님                                *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-23                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
bool IsSamsungAmexCard( byte *abCardNo )
{
	byte i = 0;
	const static byte bSamsungAmexCardBINCnt = 3;
	const static byte abSamsungAmexCardBIN[3][7] =
		{"376293", "379183", "379184"};

	for ( i = 0; i < bSamsungAmexCardBINCnt; i++ )
	{
		if ( memcmp( abSamsungAmexCardBIN[i], abCardNo, 6 ) == 0 )
		{
			return TRUE;
		}
	}

	return FALSE;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       IsBCCardInvalidBIN                                       *
*                                                                              *
*  DESCRIPTION:       BC 테스트카드 BIN코드 여부를 판별한다.                   *
*                     (해당 BIN코드이면서 alias번호에 문제가 있는 카드는       *
*                      사용불가 처리한다.)                                     *
*                                                                              *
*  INPUT PARAMETERS:  abCardNo - 카드번호                                      *
*                                                                              *
*  RETURN/EXIT VALUE: TRUE - BC 테스트카드 BIN코드                             *
*                     FALSE - BC 테스트카드 BIN코드 아님                       *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-23                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
bool IsBCCardInvalidBIN( byte *abCardNo )
{
	byte i = 0;
	const static byte bBCCardInvalidBINCnt = 11;
	const static byte abBCCardInvalidBIN[11][7] =
		{"455323", "490623", "537620", "537703", "941025", "942021", "942023",
		 "942025", "942031", "942032", "942033"};

	for ( i = 0; i < bBCCardInvalidBINCnt; i++ )
	{
		if ( memcmp( abBCCardInvalidBIN[i], abCardNo, 6 ) == 0 )
		{
			return TRUE;
		}
	}

	return FALSE;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       IsValidPostpayIssuer                                     *
*                                                                              *
*  DESCRIPTION:       후불발행사정보에 해당 카드의 prefix가 존재하는지의 여부를*
*                     리턴한다.                                                *
*                                                                              *
*  INPUT PARAMETERS:  abCardNo - 카드번호                                      *
*                                                                              *
*  RETURN/EXIT VALUE: TRUE - 후불발행사정보에 prefix가 존재함                  *
*                     FALSE - 후불발행사정보에 prefix가 존재하지 않음          *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-23                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
bool IsValidPostpayIssuer( byte *abCardNo )
{
	dword i = 0;

	// 후불발행사정보의 로드에 실패한 경우 무조건 TRUE 리턴
	if ( gstPostpayIssuerInfoHeader.wRecordCnt == 0 )
	{
		return TRUE;
	}

	for ( i = 0; i < gstPostpayIssuerInfoHeader.wRecordCnt; i++ )
	{
		if ( memcmp( gpstPostpayIssuerInfo[i].abPrefixNo, abCardNo, 6 ) == 0 )
		{
			return TRUE;
		}
	}
	return FALSE;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       IsValidIssuerValidPeriod                                 *
*                                                                              *
*  DESCRIPTION:       후불발행사 유효기간 체크를 수행한다.                     *
*                                                                              *
*  INPUT PARAMETERS:  abCardNo - 카드번호                                      *
*                     abExpiryDate - 유효기간 (YYYYMM)                         *
*                                                                              *
*  RETURN/EXIT VALUE: TRUE - 카드의 유효기간이 운영정보의 유효기간보다 작음    *
*                     FALSE - 카드의 유효기간이 운영정보의 유효기간보다 크거나 *
*                         같음                                                 *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-23                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
bool IsValidIssuerValidPeriod( byte *abCardNo, byte *abExpiryDate )
{
	dword i = 0;

	// 발행사유효기간정보의 로드에 실패한 경우 무조건 TRUE 리턴
	if ( gstIssuerValidPeriodInfoHeader.wRecordCnt == 0 )
	{
		return TRUE;
	}

	for ( i = 0; i < gstIssuerValidPeriodInfoHeader.wRecordCnt; i++ )
	{
		if ( memcmp( gpstIssuerValidPeriodInfo[i].abPrefixNo, abCardNo,
				sizeof( gpstIssuerValidPeriodInfo[i].abPrefixNo ) ) == 0 &&
			 memcmp( "000000", gpstIssuerValidPeriodInfo[i].abExpiryDate,
				sizeof( gpstIssuerValidPeriodInfo[i].abExpiryDate ) ) != 0 &&
			 memcmp( abExpiryDate,
			 	gpstIssuerValidPeriodInfo[i].abExpiryDate,
			 	sizeof( gpstIssuerValidPeriodInfo[i].abExpiryDate ) ) >= 0 )
		{
			return FALSE;
		}
	}
	return TRUE;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       IsValidMifPrepayIssueDate                                *
*                                                                              *
*  DESCRIPTION:       구선불카드 발급일 체크를 수행한다.                       *
*                                                                              *
*  INPUT PARAMETERS:  abCardNo - 카드번호                                      *
*                     abIssueDate - 발급일 (YYYYMMDD)                          *
*                                                                              *
*  RETURN/EXIT VALUE: TRUE - 구선불카드 발급일 체크 성공                       *
*                     FALSE - 구선불카드 발급일 체크 실패                      *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2006-05-03                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
bool IsValidMifPrepayIssueDate( byte *abCardNo, byte *abIssueDate )
{
	dword i = 0;
	byte abASCPrefix[7] = {0, };
	byte abCheckIssueDate[8] = {0, };

	// 발행사유효기간정보의 로드에 실패한 경우 무조건 TRUE 리턴
	if ( gstIssuerValidPeriodInfoHeader.wRecordCnt == 0 )
	{
		return TRUE;
	}

	if ( memcmp( abIssueDate, "00000000", 8 ) == 0 )
	{
		printf( "[IsValidMifPrepayIssueDate] 발급일이 '0'으로 설정\n" );
		return TRUE;
	}

	memcpy( abASCPrefix, abCardNo, sizeof( abASCPrefix ) );
	if ( abASCPrefix[0] == '0' )
	{
		abASCPrefix[0] = '1';
	}

	for ( i = 0; i < gstIssuerValidPeriodInfoHeader.wRecordCnt; i++ )
	{
		memcpy( &abCheckIssueDate[2], gpstIssuerValidPeriodInfo[i].abExpiryDate,
			sizeof( gpstIssuerValidPeriodInfo[i].abExpiryDate ) );
		if ( memcmp( gpstIssuerValidPeriodInfo[i].abExpiryDate, "70", 2 ) >= 0 )
		{
			abCheckIssueDate[0] = '1';
			abCheckIssueDate[1] = '9';
		}
		else
		{
			abCheckIssueDate[0] = '2';
			abCheckIssueDate[1] = '0';
		}

		if ( memcmp( "000000", gpstIssuerValidPeriodInfo[i].abPrefixNo,
				sizeof( gpstIssuerValidPeriodInfo[i].abPrefixNo ) ) == 0 &&
			 memcmp( abASCPrefix,
			 	gpstIssuerValidPeriodInfo[i].abIssuerID, 
			 	sizeof( gpstIssuerValidPeriodInfo[i].abIssuerID ) ) == 0 &&
			 memcmp( "000000", gpstIssuerValidPeriodInfo[i].abExpiryDate,
			 	sizeof( gpstIssuerValidPeriodInfo[i].abExpiryDate ) ) != 0 &&
			 memcmp( abIssueDate, abCheckIssueDate, sizeof( abCheckIssueDate ) ) >= 0 )
		{
			return FALSE;
		}
	}
	return TRUE;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       IsValidSCPrepayIssueDate                                 *
*                                                                              *
*  DESCRIPTION:       신선불카드 발급일 체크를 수행한다.                       *
*                                                                              *
*  INPUT PARAMETERS:  abCardNo - 카드번호                                      *
*                     tIssueDate - 발급일                                      *
*                                                                              *
*  RETURN/EXIT VALUE: TRUE - 신선불카드 발급일 체크 성공                       *
*                     FALSE - 신선불카드 발급일 체크 실패                      *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2006-05-03                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
bool IsValidSCPrepayIssueDate( byte *abCardNo, time_t tIssueDate )
{
	dword i = 0;
	byte abASCPrefix[7] = {0, };
	byte abCardIssueDate[8] = {0, };
	byte abCheckIssueDate[8] = {0, };

	// 발행사유효기간정보의 로드에 실패한 경우 무조건 TRUE 리턴
	if ( gstIssuerValidPeriodInfoHeader.wRecordCnt == 0 )
	{
		return TRUE;
	}

	TimeT2ASCDate( tIssueDate, abCardIssueDate );

	abASCPrefix[0] = '3';
	memcpy( &abASCPrefix[1], abCardNo, 6 );

	for ( i = 0; i < gstIssuerValidPeriodInfoHeader.wRecordCnt; i++ )
	{
		memcpy( &abCheckIssueDate[2], gpstIssuerValidPeriodInfo[i].abExpiryDate,
			sizeof( gpstIssuerValidPeriodInfo[i].abExpiryDate ) );
		if ( memcmp( gpstIssuerValidPeriodInfo[i].abExpiryDate, "70", 2 ) >= 0 )
		{
			abCheckIssueDate[0] = '1';
			abCheckIssueDate[1] = '9';
		}
		else
		{
			abCheckIssueDate[0] = '2';
			abCheckIssueDate[1] = '0';
		}

		if ( memcmp( "000000", gpstIssuerValidPeriodInfo[i].abPrefixNo,
				sizeof( gpstIssuerValidPeriodInfo[i].abPrefixNo ) ) == 0 &&
			 memcmp( abASCPrefix,
			 	gpstIssuerValidPeriodInfo[i].abIssuerID,
			 	sizeof( gpstIssuerValidPeriodInfo[i].abIssuerID ) ) == 0 &&
			 memcmp( "000000", gpstIssuerValidPeriodInfo[i].abExpiryDate,
			 	sizeof( gpstIssuerValidPeriodInfo[i].abExpiryDate ) ) != 0 &&
			 memcmp( abCardIssueDate, abCheckIssueDate, sizeof( abCardIssueDate ) ) >= 0 )
		{
			return FALSE;
		}
	}
	return TRUE;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SearchCardErrLog                                         *
*                                                                              *
*  DESCRIPTION:       Error List[링크드리스트]를 검색한 후 결과를 반환한다.    *
*		              Error List에 있는 카드일 경우 카드정보 구조체를 해당	   *
*          			  Error Log의 구조체로 바꿔준다.                           *
*					  Error List의 해당 Log 번호를 저장한다. (-> nCurrentLog)  *
*					  Log 번호는 1번부터 시작한다.                             *
*                                                                              *
*  INPUT PARAMETERS:  TRANS_INFO *pstTransInfo                                 *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - Error List에 없음 (정상 카드)                  *
*                     != 0 - Error Code (Error Log 기록 당시의 에러 코드)      *
*                                                                              *
*  Author : Kyoungryun Bae													   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short SearchCardErrLog( TRANS_INFO *pstTransInfo )
{
	short sIndex = 0;

	sIndex = SearchErrLog( pstTransInfo->abCardNo );

	// 오류로그에 존재하지 않음 ////////////////////////////////////////////////
	if ( sIndex == -1 )
	{
		return -1;
	}

	// 오류로그에 존재함 ///////////////////////////////////////////////////////
	memcpy( pstTransInfo,
		&gpstSharedInfo->astTransErrLog[( byte )sIndex].stTransInfo,
		sizeof( TRANS_INFO ) );

	return SUCCESS;
}

void InitCardErrLog( void )
{
	gpstSharedInfo->bTransErrLogPtr = 0;
	memset( gpstSharedInfo->astTransErrLog, 0x00,
		sizeof( gpstSharedInfo->astTransErrLog ) );
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      AddCardErrLog                                            *
*                                                                              *
*  DESCRIPTION:       카드거래처리 후 에러처리                                 *
*                                                                              *
*  INPUT PARAMETERS:  short sResult, TRANS_INFO *pstTransInfo                  *
*                                                                              *
*  RETURN/EXIT VALUE: N/A                                                      *
*                                                                              *
*  Author : MeeHyang Son													   *
*                                                                              *
*  DATE   : 2005-09-06 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
void AddCardErrLog( short sResult, TRANS_INFO *pstTransInfo )
{
	short sIndex = 0;

	if ( sResult != SUCCESS )
	{
		//카드 쓰기오류 카운트 증가
		pstTransInfo->bWriteErrCnt++;
		pstTransInfo->sErrCode = sResult;
	}

	if ( ( sIndex = SearchErrLog( pstTransInfo->abCardNo ) ) == -1 )
	{
		AddErrLog( pstTransInfo );
	}
	else
	{
		UpdateErrLog( ( byte )sIndex, pstTransInfo );
	}

	PrintErrLog();
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      DeleteCardErrLog                                         *
*                                                                              *
*  DESCRIPTION:       Error 링크드리스트의 특정 Node(->nCurrentLog) 삭제       *
*			          SearchErrLog()가 선행되서 nCurrentLog 값이               *
*			          설정되어 있지 않으면 에러 반환                           *
*                                                                              *
*  INPUT PARAMETERS:  N/A                                                      *
*                                                                              *
*  RETURN/EXIT VALUE: 0 - 정상적으로 삭제함                                    *
*				      1 - 삭제하려는 Log가 리스트에 없음                       *
*                                                                              *
*  Author : Kyoungryun Bae													   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
void DeleteCardErrLog( byte *abCardNo )
{
	short sIndex = 0;

	sIndex = SearchErrLog( abCardNo );
	if ( sIndex != -1 )
	{
		gpstSharedInfo->astTransErrLog[( byte )sIndex].boolIsDeleted = TRUE;
	}

	PrintErrLog();
}

static short SearchErrLog( byte *abCardNo )
{
	byte i = 0;

	for ( i = 0; i < MAX_ERROR_LOG; i++ )
	{
		if ( memcmp( gpstSharedInfo->astTransErrLog[i].stTransInfo.abCardNo,
			abCardNo, 20 ) == 0 &&
			 gpstSharedInfo->astTransErrLog[i].boolIsDeleted == FALSE )
		{
			DebugOut( "[SearchErrLog] 검색 BINGO!\n" );
			return i;
		}
	}

	DebugOut( "[SearchErrLog] 검색 실패!\n" );

	return -1;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      AddErrLog                                                *
*                                                                              *
*  DESCRIPTION:       Error List에 Log 추가                                    *
*                                                                              *
*  INPUT PARAMETERS:  TRANS_INFO *pstTransInfo                                 *
*                                                                              *
*  RETURN/EXIT VALUE: N/A                                                      *
*                                                                              *
*  Author : Kyoungryun Bae													   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static void AddErrLog( TRANS_INFO *pstTransInfo )
{
	DebugOutlnASC( "[AddErrLog] 카드번호 : ", pstTransInfo->abCardNo, 20 );
	DebugOut	 ( "[AddErrLog] 인덱스   : %u\n",
		gpstSharedInfo->bTransErrLogPtr );

	gpstSharedInfo->astTransErrLog[gpstSharedInfo->bTransErrLogPtr].boolIsDeleted = FALSE;
	memcpy(
		&gpstSharedInfo->astTransErrLog[gpstSharedInfo->bTransErrLogPtr].stTransInfo,
		pstTransInfo, sizeof( TRANS_INFO ) );
	gpstSharedInfo->bTransErrLogPtr = ( gpstSharedInfo->bTransErrLogPtr + 1 )
		% MAX_ERROR_LOG;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      UpdateErrLog                                             *
*                                                                              *
*  DESCRIPTION:       Error 링크드리스트의 특정 Node( ->nCurrentLog ) 값 변경    *
*                                                                              *
*  INPUT PARAMETERS:  TRANS_INFO *pstTransInfo                                 *
*                                                                              *
*			          SearchErrLog()가 선행되서 nCurrentLog 값이               *
*			          설정되어 있지 않으면 에러 반환                           *
*                                                                              *
*  INPUT PARAMETERS:  N/A                                                      *
*                                                                              *
*  RETURN/EXIT VALUE: 0 - 정상적으로 삭제함                                    *
*				      1 - 삭제하려는 Log가 리스트에 없음                       *
*                                                                              *
*  Author : Kyoungryun Bae													   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static void UpdateErrLog( byte bIndex, TRANS_INFO *pstTransInfo )
{
	DebugOutlnASC( "[UpdateErrLog] 카드번호 : ", pstTransInfo->abCardNo, 20 );
	DebugOut	 ( "[UpdateErrLog] 인덱스   : %u\n", bIndex );

	memcpy( &gpstSharedInfo->astTransErrLog[bIndex].stTransInfo, pstTransInfo,
		sizeof( TRANS_INFO ) );
}

static void PrintErrLog( void )
{
	byte i = 0;

	for ( i = 0; i < MAX_ERROR_LOG; i++ )
	{
		if ( gpstSharedInfo->astTransErrLog[i].boolIsDeleted )
		{
			DebugOut( "[%2u] 삭제 ", i );
		}
		else
		{
			DebugOut( "[%2u]      ", i );
		}
		DebugOutlnASC( "",
			gpstSharedInfo->astTransErrLog[i].stTransInfo.abCardNo, 20 );
	}
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       PrintTransInfo                                           *
*                                                                              *
*  DESCRIPTION:       카드정보구조체의 내용을 화면에 출력한다.                 *
*                                                                              *
*  INPUT PARAMETERS:  pstTransInfo - 카드정보 구조체                           *
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
void PrintTransInfo( TRANS_INFO *pstTransInfo )
{
	DebugOut		( "\n// 카드정보구조체 /////////////////////////////////" );
	DebugOut		( "////////////////////////////\n" );
	DebugOut		( "칩 시리얼 번호                        : %lu\n",
		pstTransInfo->dwChipSerialNo );
	DebugOut		( "카드사용자유형                        : %u\n",
		pstTransInfo->bPLUserType );
	DebugOut		( "카드유형                              : %u\n",
		pstTransInfo->bCardType );
	DebugOutlnASC	( "카드번호                              : ",
		pstTransInfo->abCardNo, 20 );
	DebugOut		( "alias 번호                            : %lu\n",
		pstTransInfo->dwAliasNo );

	DebugOut		( "신카드 알고리즘유형                   : %u\n",
		pstTransInfo->bSCAlgoriType );
	DebugOut		( "신카드 거래유형                       : %u\n",
		pstTransInfo->bSCTransType );
	DebugOut		( "신카드 개별거래수집키버전             : %u\n",
		pstTransInfo->bSCTransKeyVer );
	DebugOut		( "신카드 전자화폐사ID                   : %u\n",
		pstTransInfo->bSCEpurseIssuerID );
	DebugOutlnASC	( "신카드 전자지갑ID                     : ",
		pstTransInfo->abSCEpurseID, 16 );
	DebugOut		( "신카드 거래건수                       : %lu\n",
		pstTransInfo->dwSCTransCnt );

	DebugOutlnASC	( "PSAM ID                               : ",
		pstTransInfo->abPSAMID, 16 );
	DebugOut		( "PSAM 거래카운터                       : %lu\n",
		pstTransInfo->dwPSAMTransCnt );
	DebugOut		( "PSAM 총액거래수집카운터               : %lu\n",
		pstTransInfo->dwPSAMTotalTransCnt );
	DebugOut		( "PSAM 개별거래수집건수                 : %u\n",
		pstTransInfo->wPSAMIndvdTransCnt );
	DebugOut		( "PSAM 누적거래총액                     : %lu\n",
		pstTransInfo->dwPSAMAccTransAmt );
	DebugOutlnBCD	( "PSAM 서명                             : ",
		pstTransInfo->abPSAMSign, 4 );

	DebugOut		( "직전충전후카드잔액                    : %lu\n",
		pstTransInfo->dwBalAfterCharge );
	DebugOut		( "직전충전시카드거래건수                : %lu\n",
		pstTransInfo->dwChargeTransCnt );
	DebugOut		( "직전충전금액                          : %lu\n",
		pstTransInfo->dwChargeAmt );
	DebugOutlnASC	( "직전충전기LSAMID                      : ",
		pstTransInfo->abLSAMID, 16 );
	DebugOut		( "직전충전기LSAM거래일련번호            : %lu\n",
		pstTransInfo->dwLSAMTransCnt );
	DebugOut		( "직전충전거래유형                      : %u\n",
		pstTransInfo->bChargeTransType );
	DebugOutlnASC	( "구선불카드 직전충전승인번호           : ",
		pstTransInfo->abMifPrepayChargeAppvNop, 14 );
	DebugOutlnBCD	( "구선불카드 TCC                        : ",
		pstTransInfo->abMifPrepayTCC, 8 );

	DebugOut		( "승하차유형                            : %u\n",
		pstTransInfo->bEntExtType );
	DebugOut		( "구카드 단말기그룹코드                 : %u\n",
		pstTransInfo->bMifTermGroupCode );
	DebugOutlnTimeT	( "구카드 이용시간                       : ",
		pstTransInfo->tMifEntExtDtime );
	DebugOut		( "다인승여부                            : %s\n",
		GetBoolString( pstTransInfo->boolIsMultiEnt ) );
	DebugOut		( "추가승차여부                          : %s\n",
		GetBoolString( pstTransInfo->boolIsAddEnt ) );
	DebugOutlnBCD	( "다인승사용자유형                      : ",
		( byte * )pstTransInfo->abUserType, 3 );
	DebugOutlnBCD	( "다인승사용자수                        : ",
		( byte * )pstTransInfo->abUserCnt, 3 );
	DebugOutlnASC	( "다인승할인할증유형ID1                 : ",
		pstTransInfo->abDisExtraTypeID[0], 6 );
	DebugOutlnASC	( "다인승할인할증유형ID2                 : ",
		pstTransInfo->abDisExtraTypeID[1], 6 );
	DebugOutlnASC	( "다인승할인할증유형ID3                 : ",
		pstTransInfo->abDisExtraTypeID[2], 6 );
	DebugOut		( "사용거리                              : %lu\n",
		pstTransInfo->dwDist );
	DebugOut		( "비환승 사유                           : %u\n",
		pstTransInfo->bNonXferCause );
	DebugOut		( "후불카드이면서 월변경여부             : %s\n",
		GetBoolString( pstTransInfo->boolIsChangeMonth ) );
	DebugOutlnTimeT	( "관광권카드 최초 사용 일시             : ",
		pstTransInfo->tMifTourFirstUseDtime );
	DebugOut		( "관광권카드 유형                       : %u\n",
		pstTransInfo->wMifTourCardType );
	DebugOut		( "관광권카드 이번 승/하차시 증가 횟수   : %u\n",
		pstTransInfo->bMifTourUseCnt );

	DebugOut		( "\n-- 이전환승정보 -----------------------------------" );
	DebugOut		( "----------------------------\n" );
	PrintCommonXferInfo( 0, &pstTransInfo->stPrevXferInfo );
	DebugOut		( "\n-- 신규환승정보 -----------------------------------" );
	DebugOut		( "----------------------------\n" );
	PrintCommonXferInfo( 0, &pstTransInfo->stNewXferInfo );

	if ( pstTransInfo->bCardType == TRANS_CARDTYPE_MIF_PREPAY ||
		pstTransInfo->bCardType == TRANS_CARDTYPE_DEPOSIT )
	{
		DebugOut		( "\n-- 구선불카드 BLOCK18 -------------------------" );
		DebugOut		( "--------------------------------\n" );
		PrintMifPrepayBlock18( &pstTransInfo->stMifPrepayBlock18 );
	}
	if ( pstTransInfo->bCardType == TRANS_CARDTYPE_MIF_POSTPAY )
	{
		DebugOut		( "\n-- 구후불카드 구환승정보 ----------------------" );
		DebugOut		( "--------------------------------\n" );
		PrintMifPostpayOldXferData( &pstTransInfo->stOldXferInfo );
	}

	DebugOut		( "오류코드                              : %x\n",
		pstTransInfo->sErrCode );
	DebugOut		( "WRITE오류 발생 횟수                   : %u\n",
		pstTransInfo->bWriteErrCnt );
	DebugOut		( "/////////////////////////////////////////////////////" );
	DebugOut		( "//////////////////////////\n" );
	DebugOut		( "\n" );
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       InitCardNoLog                                            *
*                                                                              *
*  DESCRIPTION:       카드번호LOG를 초기화한다.                                *
*                                                                              *
*  INPUT PARAMETERS:  void                                                     *
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
void InitCardNoLog( void )
{
	gnCardNoLogCount = -1;
	memset( gabCardNoLog, 0, sizeof( gabCardNoLog ) );
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       AddCardNoLog                                             *
*                                                                              *
*  DESCRIPTION:       카드번호LOG에 카드번호를 추가한다.                       *
*                                                                              *
*  INPUT PARAMETERS:  abCardNo - 카드번호LOG에 추가하고자 하는 카드번호        *
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
void AddCardNoLog( byte *abCardNo )
{
	byte i = 0;

	if ( IsExistCardNoLog( abCardNo ) )
	{
		for ( i = 0; i < MAX_CARD_NO_LOG; i++ )
		{
			DebugOutlnASC( "[CARDNO_LOG] 카드번호 : ", gabCardNoLog[i], 20 );
		}
		return;
	}

	if ( gnCardNoLogCount == -1 ) {
		memset( gabCardNoLog, 0, sizeof( gabCardNoLog ) );
		gnCardNoLogCount = 0;
	}

	memcpy( gabCardNoLog[gnCardNoLogCount], abCardNo, 20 );
	gnCardNoLogCount = ( gnCardNoLogCount + 1 ) % MAX_CARD_NO_LOG;

	for ( i = 0; i < MAX_CARD_NO_LOG; i++ )
	{
		DebugOutlnASC( "[CARDNO_LOG] 카드번호 : ", gabCardNoLog[i], 20 );
	}
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       IsExistCardNoLog                                         *
*                                                                              *
*  DESCRIPTION:       카드번호LOG에 카드번호를 추가한다.                       *
*                                                                              *
*  INPUT PARAMETERS:  abCardNo - 카드번호LOG에 존재하는지 확인하고자 하는      *
*                         카드번호                                             *
*                                                                              *
*  RETURN/EXIT VALUE: TRUE - 입력된 카드번호가 카드번호LOG에 존재함            *
*                     FALSE - 입력된 카드번호가 카드번호LOG에 존재하지 않음    *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-25                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
bool IsExistCardNoLog( byte *abCardNo )
{

	int i = 0;

	if ( gnCardNoLogCount == -1 )
		return FALSE;

	for ( i = 0; i < MAX_CARD_NO_LOG; i++ )
		if ( memcmp( gabCardNoLog[i], abCardNo, 20 ) == 0 )
			return TRUE;

	return FALSE;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      IsValidSCPurseInfo                                       *
*                                                                              *
*  DESCRIPTION:       신카드 전자지갑 정보파일 체크                            *
*                                                                              *
*  INPUT PARAMETERS:  SC_EF_PURSE_INFO *pstPurseInfo                           *
*                                                                              *
*  RETURN/EXIT VALUE: TRUE/FALSE                                               *
*                                                                              *
*  Author : MeeHyang Son													   *
*                                                                              *
*  DATE   : 2005-11-17 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
bool IsValidSCPurseInfo( SC_EF_PURSE_INFO *pstPurseInfo )
{

	// 카드번호 확인
	if ( IsDigitASC( pstPurseInfo->abEpurseID, 16 ) != TRUE )
		return FALSE;

	// 카드소지자 구분코드 확인 ( 0x00 ~ 0x0F )
	if ( pstPurseInfo->bUserTypeCode > 0x0F )
		return FALSE;

	return TRUE;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      InitWatch                                                *
*                                                                              *
*  DESCRIPTION:       시간 측정을 위해 변수를 초기화                           *
*                                                                              *
*  INPUT PARAMETERS:  N/A                                                      *
*                                                                              *
*  RETURN/EXIT VALUE: N/A                                                      *
*                                                                              *
*  Author : Kyoungryun Bae													   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
void InitWatch( void )
{
	stTime.tv_sec = 0;
	stTime.tv_usec = 0;
	nWatchCount =0;
	dTotDuration = 0.0;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      StopWatch                                                *
*                                                                              *
*  DESCRIPTION:       InitWatch() 함수를 호출한 시점으로부터 경과된 시간 출력  *
*                                                                              *
*  INPUT PARAMETERS:  N/A                                                      *
*                                                                              *
*  RETURN/EXIT VALUE: double 경과시간                                          *
*                                                                              *
*  Author : Kyoungryun Bae													   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
double StopWatch( void )
{
	struct timeval tv;
	double dur = 0.0;

	gettimeofday( &tv, NULL );
	if ( stTime.tv_sec == 0 && stTime.tv_usec==0 ) {  /* first stop */
		DebugOut( "Start watch....\n" );
	}
	else {
		nWatchCount++;

		dur = ( tv.tv_sec - stTime.tv_sec ) +
			( ( tv.tv_usec - stTime.tv_usec ) / 1000000.0 );
		DebugOut( "[%d]....Time duration %f sec\n", nWatchCount, dur );

		dTotDuration += dur;
		DebugOut( "[%d]....Total Time duration %f sec\n", nWatchCount,
			dTotDuration );
	}
	stTime.tv_sec  = tv.tv_sec;
	stTime.tv_usec = tv.tv_usec;

	return dur;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       PrintXferInfo                                            *
*                                                                              *
*  DESCRIPTION:       입력된 환승정보를 콘솔에 간략한 형태로 출력한다.         *
*                                                                              *
*  INPUT PARAMETERS:  pstCommonXferData - 입력 공통환승정보                    *
*                                                                              *
*  RETURN/EXIT VALUE: void                                                     *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-11-17                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
void PrintXferInfo( COMMON_XFER_DATA *pstCommonXferData )
{
	printf( "승하차 구분\t\t: %9x", pstCommonXferData->bEntExtType );
	printf( "\t환승 누적 횟수\t\t: %9u", pstCommonXferData->bAccXferCnt );

	printf( "\n환승 일련번호\t\t: %9u", pstCommonXferData->bXferSeqNo );
	printf( "\t정류장(역) 코드\t\t:   " );
	PrintASC( pstCommonXferData->abStationID, 7 );

	printf( "\n교통 수단 코드\t\t: %9u", pstCommonXferData->wTranspMethodCode );
	printf( "\t이용 시간\t: " );
	PrintTimeT( pstCommonXferData->tEntExtDtime );

	printf( "\n환승내 누적 이동 거리\t: %9lu",
		pstCommonXferData->dwAccDistInXfer );
	printf( "\t환승내 누적 이용 금액\t: %9lu",
		pstCommonXferData->dwAccAmtInXfer );

	printf( "\n단말기ID\t\t: %09lu", pstCommonXferData->dwTermID );
	printf( "\t다인승 거래 내역\t: " );
	PrintBCD( ( byte * )pstCommonXferData->abMultiEntInfo, 6 );

	printf( "\n총 누적 승차 횟수\t: %9u", pstCommonXferData->wTotalAccEntCnt );
	printf( "\t총 누적 사용 금액\t: %9lu",
		pstCommonXferData->dwTotalAccUseAmt );

	printf( "\n소지자1 최대 기본요금\t: %9u",
		pstCommonXferData->awMaxBaseFare[0] );
	printf( "\t소지자2 최대 기본요금\t: %9u",
		pstCommonXferData->awMaxBaseFare[1] );

	printf( "\n소지자3 최대 기본요금\t: %9u",
		pstCommonXferData->awMaxBaseFare[2] );
	printf( "\t환승내 기본요금의 합\t: %9u",
		pstCommonXferData->wTotalBaseFareInXfer );

	printf( "\n직전 페널티 요금\t: %9u", pstCommonXferData->wPrevPenaltyFare );
	printf( "\t이전 미징수 금액\t: %9u",
		pstCommonXferData->wPrevUnchargedFare );

	printf( "\n요금\t\t\t: %9lu", pstCommonXferData->dwFare );
	printf( "\t잔액\t\t\t: %9lu", pstCommonXferData->dwBal );

	printf( "\n" );
}
