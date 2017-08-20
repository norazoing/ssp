#include "../system/bus_type.h"
#include "card_proc.h"
#include "card_proc_util.h"
#include "blpl_proc.h"
#include "trans_proc.h"

// 구선불카드 카드번호에 포함된 사용자 유형 define /////////////////////////////
#define CARD_USER_ADULT				'0'		// 일반
#define CARD_USER_STUDENT			'1'		// 학생
#define CARD_USER_CHILD				'2'		// 어린이
#define CARD_USER_TEST				'5'		// 테스트

// block6 holder type의 사용자 유형 define /////////////////////////////////////
#define HOLDER_ADULT				0		// 일반
#define HOLDER_NEW_STUDENT			1		// 신규학생
#define HOLDER_REGISTERED_STUDENT	2		// 등록학생
#define HOLDER_UNIVERSITY_STUDENT	3		// 대학생

static void MifPrepaySetBasicCardInfo( TRANS_INFO *pstTransInfo,
	MIF_PREPAY_SECTOR1 *pstMifPrepaySector1,
	MIF_PREPAY_SECTOR2 *pstMifPrepaySector2 );
static short MifPrepayCheckValidCard( byte *abCardNo, byte *abIssueDate );
static short MifPrepayCheckPL( TRANS_INFO *pstTransInfo,
	MIF_PREPAY_BLOCK6 *pstMifPrepayBlock6,
	MIF_PREPAY_BLOCK16 *pstMifPrepayBlock16 );
static void MifPrepaySetCardInfoStruct( TRANS_INFO *pstTransInfo,
	MIF_PREPAY_SECTOR1 *pstMifPrepaySector1,
	MIF_PREPAY_SECTOR2 *pstMifPrepaySector2,
	MIF_PREPAY_SECTOR3 *pstMifPrepaySector3,
	MIF_PREPAY_SECTOR4 *pstMifPrepaySector4,
	COMMON_XFER_DATA *pstCommonXferData );
static void MifPrepayBuildISAMTransReq( TRANS_INFO *pstTransInfo,
	ISAM_TRANS_REQ *pstISAMTransReq );
static void MifPrepayBuildISAMMakeTCC( TRANS_INFO *pstTransInfo,
	ISAM_MAKE_TCC *pstISAMMakeTCC );
static void MifPrepayBuildOldXferInfo( TRANS_INFO *pstTransInfo );
static void MifPrepayBuildISAMTransReq( TRANS_INFO *pstTransInfo,
	ISAM_TRANS_REQ *pstISAMTransReq );
static void MifPrepayBuildISAMMakeTCC( TRANS_INFO *pstTransInfo,
	ISAM_MAKE_TCC *pstISAMMakeTCC );
static void MifPrepayBuildOldXferInfo( TRANS_INFO *pstTransInfo );

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       MifPrepayRead                                            *
*                                                                              *
*  DESCRIPTION:       구선불카드를 READ하고 유효성체크 및 PL체크를 수행한 후   *
*                     카드정보구조체를 조립한다.                               *
*                                                                              *
*  INPUT PARAMETERS:  pstTransInfo - 카드정보구조체의 포인터                   *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공                                      *
*                     ERR_CARD_PROC_RETAG_CARD - 카드를 다시 대주세요.         *
*                     ERR_CARD_PROC_CANNOT_USE - 사용할 수 없는 카드입니다.    *
*                     ERR_CARD_PROC_LOG - 에러로그에 존재하는 카드의 경우      *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-23                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
short MifPrepayRead( TRANS_INFO *pstTransInfo )
{
	short sResult = 0;

	MIF_PREPAY_SECTOR1 stMifPrepaySector1;
	MIF_PREPAY_SECTOR2 stMifPrepaySector2;
	MIF_PREPAY_SECTOR3 stMifPrepaySector3;
	MIF_PREPAY_SECTOR4 stMifPrepaySector4;
	COMMON_XFER_DATA stCommonXferData;
byte abTempCardNo[21] = {0, };
	// 구선불카드 기본정보READ 및 SAM체크 //////////////////////////////////////
	sResult = MifPrepayReadBasicInfo( pstTransInfo->dwChipSerialNo,
		&stMifPrepaySector1,
		&stMifPrepaySector2,
		&stMifPrepaySector3 );
	if ( sResult != SUCCESS )
	{
		printf( "[MifPrepayRead] 구선불카드 기본정보READ 및 SAM체크 ");
		printf( "실패\n" );
		switch ( sResult )
		{
			case ERR_PAY_LIB_MIF_PREPAY_READ_SECTOR1:
			case ERR_PAY_LIB_MIF_PREPAY_READ_SECTOR2:
			case ERR_PAY_LIB_MIF_PREPAY_READ_SECTOR3:
			case ERR_PAY_LIB_MIF_PREPAY_INVALID_BAL_BLOCK:
			case ERR_PAY_LIB_ISAM_SET_CARD_CSN:
			case ERR_PAY_LIB_ISAM_RECOVER_CASE1:
			case ERR_PAY_LIB_ISAM_RECOVER_CASE2:
			case ERR_PAY_LIB_ISAM_RECOVER_CASE3:
			case ERR_PAY_LIB_ISAM_RECOVER_CASE4:
			case ERR_PAY_LIB_ISAM_GET_CARD_INFO:
			case ERR_PAY_LIB_ISAM_CHK_TCC:
			case ERR_PAY_LIB_ISAM_CHK_CICC:
			case ERR_PAY_LIB_ISAM_CHK_CII:
			default:
				return ERR_CARD_PROC_RETAG_CARD;
		}
	}

	// 구선불카드 기본정보 설정 ////////////////////////////////////////////////
	MifPrepaySetBasicCardInfo( pstTransInfo, &stMifPrepaySector1,
		&stMifPrepaySector2 );
memcpy(abTempCardNo, pstTransInfo->abCardNo, 20);
LogMain("구선 : %s\n", abTempCardNo);
	// 테스트카드의 경우 사용불가 처리 /////////////////////////////////////////
	if ( pstTransInfo->abCardNo[9] == CARD_USER_TEST )
	{
#ifndef TEST_NOT_CHECK_TEST_CARD
			printf( "[MifPrepayRead] 테스트 카드의 경우 사용불가 처리\n" );
			return ERR_CARD_PROC_CANNOT_USE;	// '사용할 수 없는 카드입니다'
#endif
	}

	// 구선불카드 유효성 체크 //////////////////////////////////////////////////
	sResult = MifPrepayCheckValidCard( pstTransInfo->abCardNo,
		stMifPrepaySector2.stMifPrepayBlock8.abIssueDate );
	if ( sResult != SUCCESS )
	{
		return sResult;
	}

	// 구선불카드 PL체크 ///////////////////////////////////////////////////////
	sResult = MifPrepayCheckPL( pstTransInfo,
		&stMifPrepaySector1.stMifPrepayBlock6,
		&stMifPrepaySector4.stMifPrepayBlock16 );
	if ( sResult != SUCCESS )
	{
		printf( "[MifPrepayRead] PL체크 오류\n" );
		switch ( sResult )
		{
			case ERR_CARD_PROC_MIF_PREPAY_PL_ALIAS:
			case ERR_CARD_PROC_MIF_PREPAY_NL_CARD:
			case ERR_CARD_PROC_MIF_PREPAY_PL_CHECK:
				return ERR_CARD_PROC_NOT_APPROV;
			default:
				return ERR_CARD_PROC_RETAG_CARD;
		}
	}

	// 오류LOG리스트에 존재하는 카드인지 확인 //////////////////////////////////
	// (하차단말기의 경우 BL/PL 체크를 통해 승차단말기의 오류내역을 가져오므로,
	//  이 작업은 반드시 BL/PL 체크 다음에 존재하여야 함)
	sResult = SearchCardErrLog( pstTransInfo );
	if ( sResult == SUCCESS )
	{
		DebugOut( "[MifPrepayRead] 오류LOG리스트에 존재하는 카드\n" );
		return ERR_CARD_PROC_LOG;
	}

	// 구선불카드 환승정보 READ ////////////////////////////////////////////////
	sResult = MifPrepayReadXferInfo( pstTransInfo->dwChipSerialNo,
		&stCommonXferData );
	if ( sResult != SUCCESS )
	{
		printf( "[MifPrepayRead] 구선불카드 환승정보 READ 오류\n" );
		return ERR_CARD_PROC_RETAG_CARD;
	}

	// 구선불카드 구환승정보 READ //////////////////////////////////////////////
	sResult = MifPrepayReadOldXferInfo( pstTransInfo->dwChipSerialNo,
		&stMifPrepaySector4 );
	if ( sResult != SUCCESS )
	{
		printf( "[MifPrepayRead] 구선불카드 구환승정보 READ 오류\n" );
		return ERR_CARD_PROC_RETAG_CARD;
	}

	// 카드정보구조체 조립 /////////////////////////////////////////////////////
	MifPrepaySetCardInfoStruct( pstTransInfo,
		&stMifPrepaySector1, &stMifPrepaySector2, &stMifPrepaySector3,
		&stMifPrepaySector4, &stCommonXferData );

	return SUCCESS;
}

static void MifPrepaySetBasicCardInfo( TRANS_INFO *pstTransInfo,
	MIF_PREPAY_SECTOR1 *pstMifPrepaySector1,
	MIF_PREPAY_SECTOR2 *pstMifPrepaySector2 )
{
	// 카드번호 설정 ///////////////////////////////////////////////////////////
	memcpy( pstTransInfo->abCardNo,
		pstMifPrepaySector2->stMifPrepayBlock8.abCardNo,
		sizeof( pstTransInfo->abCardNo ) );

#ifdef TEST_CARDTYPE_DEPOSIT
	if ( memcmp( pstTransInfo->abCardNo, TEST_CARDTYPE_DEPOSIT, 20 ) == 0 )
		memcpy( pstTransInfo->abCardNo, "0000280", 7 );
#endif

	// alias번호 설정 //////////////////////////////////////////////////////////
#ifdef TEST_MIF_PREPAY_ALIAS_0
	pstTransInfo->dwAliasNo = 0;
#else
	pstTransInfo->dwAliasNo = pstMifPrepaySector1->stMifPrepayBlock6.dwAliasNo;
#endif

	// 카드에 기록된 사용자유형 설정 ///////////////////////////////////////////
	// 0 : 일반
	// 1 : 신규학생
	// 2 : 등록학생
	// 3 : 대학생, 학원생
	if ( pstMifPrepaySector1->stMifPrepayBlock6.bStudentCardType ==
			HOLDER_ADULT ||
		pstMifPrepaySector1->stMifPrepayBlock6.bStudentCardType ==
			HOLDER_UNIVERSITY_STUDENT )
	{
		pstTransInfo->bCardUserType = USERTYPE_ADULT;
	}
	else
	{
		pstTransInfo->bCardUserType = USERTYPE_YOUNG;
	}

	// 카드유형설정 ////////////////////////////////////////////////////////////
	// 카드번호가 "0000280"로 시작되는 경우 '예치금카드'로 설정
	if ( memcmp( "0000280", pstTransInfo->abCardNo, 7 ) == 0 )
	{
		pstTransInfo->bCardType = TRANS_CARDTYPE_DEPOSIT;
	}
	// 그 외의 경우 구선불카드로 설정
	else
	{
		pstTransInfo->bCardType = TRANS_CARDTYPE_MIF_PREPAY;
	}
}

static short MifPrepayCheckValidCard( byte *abCardNo, byte *abIssueDate )
{
	bool boolResult = FALSE;

	// 구선불카드 카드번호 체크 ////////////////////////////////////////////////
	boolResult = IsValidMifPrepayCardNo( abCardNo );
	if ( !boolResult )
	{
		printf( "[MifPrepayRead] 구선불카드 카드번호 체크 오류\n" );
		return ERR_CARD_PROC_RETAG_CARD;		// '카드를 다시 대주세요'
	}

	// 구선불카드 발행사 체크 //////////////////////////////////////////////////
	boolResult = IsValidPrepayIssuer( abCardNo );
	if ( !boolResult )
	{
		printf( "[MifPrepayRead] 구선불카드 발행사 체크 오류\n" );
		return ERR_CARD_PROC_NOT_APPROV;		// '미승인 카드입니다'
	}

	// 구선불카드 발급일 체크 //////////////////////////////////////////////////
	boolResult = IsValidMifPrepayIssueDate( abCardNo, abIssueDate );
	if ( !boolResult )
	{
		printf( "[MifPrepayRead] 구선불카드 발급일 체크 오류\n" );
		return ERR_CARD_PROC_CANNOT_USE;		// '사용할 수 없는 카드입니다'
	}

	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       MifPrepayCheckPL                                         *
*                                                                              *
*  DESCRIPTION:       PL체크를 수행하여 NL카드여부와 사용자유형을 가져온다.    *
*                                                                              *
*  INPUT PARAMETERS:  pstTransInfo - 카드정보 구조체 포인터                    *
*                     pstMifPrepayBlock6 - 구선불카드 BLOCK6 구조체 포인터     *
*                     pstMifPrepayBlock16 - 구선불카드 BLOCK16 구조체 포인터   *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공                                      *
*                     ERR_CARD_PROC_MIF_PREPAY_PL_ALIAS - alias번호 오류       *
*                     ERR_CARD_PROC_MIF_PREPAY_PL_CHECK - PL체크 호출 오류     *
*                     ERR_CARD_PROC_MIF_PREPAY_NL_CARD - NL카드                *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-23                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
// 1. 구선불카드의 경우 PL값은 2비트로 구성되며, 앞 1비트는 NL(0)/PL(1) 여부,
//    뒤 1비트는 일반(0)/청소년(1) 여부의 의미를 가진다.
// 2. NL/PL 판단시 다음과 같은 예외 상황들을 고려하여야 한다.
// 	- 경기예치금카드(prefix : "0000280")의 경우 NL/PL 비트를 무시하고 무조건
// 	  PL이다.
// 	- alias번호가 1 ~ 30,000,000 의 범위에 있는 경우 NL/PL 비트를 사용한다.
// 	- alias번호가 1 ~ 30,000,000 의 범위 외에 있는 경우 일반으로 처리한다.
// 2. 일반/청소년 판단시 다음과 같은 예외 상황들을 고려하여야 한다.
// 	- KSCC 보급형카드( prefix : "10007", "10008" )는 일반/청소년 비트를 그대로
// 	  적용한다.
// 	- 그 외 카드의 경우 카드번호에 기록된 사용자유형(카드 10번째 자리)이
// 	  테스트(5)이면 테스트카드이다.
// 	- 그 외 카드의 경우 카드번호에 기록된 사용자유형(카드 10번째 자리)이
// 	  청소년(1)이 아니면 일반/청소년 비트를 무시하고 무조건 일반이다.
// 	- 그 외 카드의 경우 카드번호에 기록된 사용자유형(카드 10번째 자리)이
// 	  청소년(1)이면 일반/청소년 비트가 청소년이거나 BLOCK6의 소지자구분이
// 	  신규학생이면 청소년이고 그 외의 경우 일반이다.
static short MifPrepayCheckPL( TRANS_INFO *pstTransInfo,
	MIF_PREPAY_BLOCK6 *pstMifPrepayBlock6,
	MIF_PREPAY_BLOCK16 *pstMifPrepayBlock16 )
{
	short sResult = 0;
	byte bPLValue = 0;

	bool boolIsPL = FALSE;
	bool boolIsYoung = FALSE;

#ifdef TEST_NOT_CHECK_BLPL
	pstTransInfo->bPLUserType = USERTYPE_ADULT;
	return SUCCESS;
#endif

	// BLOCK6의 BCC가 유효하지 않거나 //////////////////////////////////////////
	// 카드에 기록된 alias 번호가 범위를 벗어나는 경우
	if ( pstMifPrepayBlock6->boolIsValidBCC == FALSE ||
		 pstTransInfo->dwAliasNo < 1 ||
		 pstTransInfo->dwAliasNo > 30000000 )
	{
		// 발행사가 '10007' 또는 '10008'이면 무조건 NL
		if ( memcmp( pstTransInfo->abCardNo, "10007", 5 ) == 0 ||
			 memcmp( pstTransInfo->abCardNo, "10008", 5 ) == 0 )
		{
			return ERR_CARD_PROC_MIF_PREPAY_NL_CARD;
		}
		// 그 외의 경우에는 무조건 일반
		else
		{
			pstTransInfo->bPLUserType = USERTYPE_ADULT;
			return SUCCESS;
		}
	}

	// PL 체크 /////////////////////////////////////////////////////////////////
	sResult = SearchPLinBus( pstTransInfo->abCardNo, pstTransInfo->dwAliasNo,
		&bPLValue );

	// PL 체크 호출 자체가 실패한 경우 /////////////////////////////////////////
	// - 주로 하차단말기에서 승차단말기와의 통신 과정 중 발생 추정
	if ( sResult == ERR_PL_FILE_OPEN_MASTER_MIF_PREPAY_PL )
	{
		printf( "[MifPrepayCheckPL] 구선불PL이 미존재\n" );
		return ErrRet( ERR_CARD_PROC_MIF_PREPAY_PL_CHECK );
	}
	else if ( sResult != SUCCESS )
	{
		printf( "[MifPrepayCheckPL] 기타 알 수 없는 이유로 구선불PL체크 " );
		printf( "실패\n" );
		return ErrRet( ERR_CARD_PROC_RETAG_CARD );
	}
	else
	{
		if ( memcmp( pstTransInfo->abCardNo, "10007", 5 ) != 0 &&
			 memcmp( pstTransInfo->abCardNo, "10008", 5 ) != 0 )
		{
			DebugOut( "[MifPrepayCheckPL] eB / KSCC 카드가 아니므로 무조건 " );
			DebugOut( "PL처리\n" );
			bPLValue |= 2;
		}

		// PL 체크 결과가 0, 1, 2, 3이 아니면 '카드를 다시 대주세요' 리턴
		if ( bPLValue >= 4 )
		{
			DebugOut( "[MifPrepayCheckPL] 구선불카드 PL체크 결과가 " );
			DebugOut( "0, 1, 2, 3이 아님\n" );
			return ERR_CARD_PROC_MIF_PREPAY_PL_CHECK;
		}

		if ( ( bPLValue & 0x02 ) == 0x02 )
			boolIsPL = TRUE;

		if ( ( bPLValue & 0x01 ) == 0x01 )
			boolIsYoung = TRUE;
	}

	// PL 사용자유형 설정 //////////////////////////////////////////////////////

	// KSCC 보급형카드의 경우 PL에 기록된 일반/청소년 비트의 값에 따라
	// PL 사용자유형 설정
	if ( memcmp( pstTransInfo->abCardNo, "10007", 5 ) == 0 ||
		 memcmp( pstTransInfo->abCardNo, "10008", 5 ) == 0 )
	{
		if ( boolIsYoung )
		{
			pstTransInfo->bPLUserType = USERTYPE_YOUNG;
		}
		else
		{
			pstTransInfo->bPLUserType = USERTYPE_ADULT;
		}
	}
	// 그 외 카드의 경우 카드에 기록된 사용자유형이 최우선 판단 기준이 됨
	else
	{
		// 카드번호의 사용자유형이 '0'이면 무조건 일반
		if ( pstTransInfo->abCardNo[9] == CARD_USER_ADULT )
		{
			pstTransInfo->bPLUserType = USERTYPE_ADULT;
		}
		// 카드번호의 사용자유형이 '1'이면서
		// BLOCK6의 사용자유형이 '신규학생'이면 무조건 청소년
		else if ( pstTransInfo->abCardNo[9] == CARD_USER_STUDENT &&
				 pstMifPrepayBlock6->bStudentCardType == HOLDER_NEW_STUDENT )
		{
			pstTransInfo->bPLUserType = USERTYPE_YOUNG;
		}
		// 그 외의 경우 PL의 값으로 판단
		else
		{
			if ( boolIsYoung )
			{
				pstTransInfo->bPLUserType = USERTYPE_YOUNG;
			}
			else
			{
				pstTransInfo->bPLUserType = USERTYPE_ADULT;
			}
		}
	}

	// NL/PL 여부 판단 /////////////////////////////////////////////////////////
	if ( memcmp( "0000280", pstTransInfo->abCardNo, 7 ) != 0 && !boolIsPL )
	{
		return ERR_CARD_PROC_MIF_PREPAY_NL_CARD;
	}

	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       MifPrepaySetCardInfoStruct                               *
*                                                                              *
*  DESCRIPTION:       카드로부터 읽은 정보를 이용하여 카드정보 구조체를        *
*                     조립한다.                                                *
*                                                                              *
*  INPUT PARAMETERS:  pstTransInfo - 카드정보 구조체 포인터                    *
*                     pstMifPrepaySector1 - 구선불카드 SECTOR1 READ 결과       *
*                     pstMifPrepaySector2 - 구선불카드 SECTOR2 READ 결과       *
*                     pstMifPrepaySector3 - 구선불카드 SECTOR3 READ 결과       *
*                     pstMifPrepaySector4 - 구선불카드 SECTOR4 READ 결과       *
*                     pstCommonXferData - 환승정보 READ 결과                   *
*                                                                              *
*  RETURN/EXIT VALUE: void                                                     *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-23                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static void MifPrepaySetCardInfoStruct( TRANS_INFO *pstTransInfo,
	MIF_PREPAY_SECTOR1 *pstMifPrepaySector1,
	MIF_PREPAY_SECTOR2 *pstMifPrepaySector2,
	MIF_PREPAY_SECTOR3 *pstMifPrepaySector3,
	MIF_PREPAY_SECTOR4 *pstMifPrepaySector4,
	COMMON_XFER_DATA *pstCommonXferData )
{
	byte abBuf[14];

	// T구선불카드 여부
	pstTransInfo->boolIsTCard =
		pstMifPrepaySector1->stMifPrepayBlock6.boolIsTCard;

	// 직전충전후카드잔액 - 구선불카드
	pstTransInfo->dwBalAfterCharge =
		pstMifPrepaySector4->stMifPrepayBlock17.dwBalAfterCharge;

	// 직전충전시카드거래건수 - 구선불카드
	// ( "년월일"을 Long 4자리 "yyyymmdd"로 기록 )
	memset( abBuf, 0, sizeof( abBuf ) );
	TimeT2ASCDtime( pstMifPrepaySector4->stMifPrepayBlock16.tChargeDtime,
		abBuf );
	pstTransInfo->dwChargeTransCnt = GetDWORDFromASC( abBuf, 8 );

	// 직전충전금액 - 구선불카드
	pstTransInfo->dwChargeAmt =
		pstMifPrepaySector4->stMifPrepayBlock16.dwChargeAmt;

	// 직전충전기SAMID - 구선불카드
	memset( pstTransInfo->abLSAMID, '0', sizeof( pstTransInfo->abLSAMID ) );
	if ( pstTransInfo->boolIsTCard == TRUE )
	{
		pstTransInfo->abLSAMID[0] = '1';
	}
	else
	{
		pstTransInfo->abLSAMID[0] = '0';
	}
	memcpy( &pstTransInfo->abLSAMID[1],
		pstMifPrepaySector4->stMifPrepayBlock16.abOLSAMID,
		sizeof( pstMifPrepaySector4->stMifPrepayBlock16.abOLSAMID ) );

	// 직전충전기SAM거래일련번호 - 구선불카드
	// 	( "시분초"를 Long 4자리 "hhmmss"로 기록 )
	pstTransInfo->dwLSAMTransCnt = GetDWORDFromASC( &abBuf[8], 6 );

	// 구선불카드 직전충전승인번호
	memcpy( pstTransInfo->abMifPrepayChargeAppvNop,
		pstMifPrepaySector4->stMifPrepayBlock17.abAppvNo,
		sizeof( pstMifPrepaySector4->stMifPrepayBlock17.abAppvNo ) );
	memcpy( &pstTransInfo->abMifPrepayChargeAppvNop[10],
		pstMifPrepaySector4->stMifPrepayBlock18.abChargeAppvNo,
		sizeof( pstMifPrepaySector4->stMifPrepayBlock18.abChargeAppvNo ) );

	// 구카드 단말기그룹코드 ( 구환승영역 기록 )
	pstTransInfo->bMifTermGroupCode =
		pstMifPrepaySector4->stMifPrepayBlock18.bLastUseTranspCode;

	// 구카드 이용시간 ( 구환승영역기록 )
	pstTransInfo->tMifEntExtDtime =
		pstMifPrepaySector4->stMifPrepayBlock18.tLastExtDtime;

	// 이전환승정보
	memcpy( &pstTransInfo->stPrevXferInfo, pstCommonXferData,
		sizeof( COMMON_XFER_DATA ) );

	// 구선불카드 BLOCK18
	memcpy( &pstTransInfo->stMifPrepayBlock18,
		&pstMifPrepaySector4->stMifPrepayBlock18,
		sizeof( MIF_PREPAY_BLOCK18 ) );

	// 이전환승정보 - 잔액
	if ( pstTransInfo->bCardType == TRANS_CARDTYPE_DEPOSIT )
	{
		pstTransInfo->stPrevXferInfo.dwBal =
			pstMifPrepaySector4->stMifPrepayBlock18.dwDepositCardBal;
	}
	else
	{
		pstTransInfo->stPrevXferInfo.dwBal = pstMifPrepaySector2->dwBalBlock9;
	}
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       MifPrepayWrite                                           *
*                                                                              *
*  DESCRIPTION:       생성된 신규환승정보를 카드에 WRITE하고 요금을 카드의     *
*                     잔액으로부터 DECREMENT한다.                              *
*                                                                              *
*  INPUT PARAMETERS:  pstTransInfo - 카드정보 구조체                           *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공                                      *
*                     ERR_CARD_PROC_RETAG_CARD - 카드를 다시 대주세요.         *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-25                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
short MifPrepayWrite( TRANS_INFO *pstTransInfo )
{
	short sResult = SUCCESS;
	ISAM_TRANS_REQ stISAMTransReq;
	ISAM_MAKE_TCC stISAMMakeTCC;

	// ISAM_TRANS_REQ 조립 /////////////////////////////////////////////////////
	MifPrepayBuildISAMTransReq( pstTransInfo, &stISAMTransReq );

	// ISAM_MAKE_TCC 조립 //////////////////////////////////////////////////////
	MifPrepayBuildISAMMakeTCC( pstTransInfo, &stISAMMakeTCC );

	// 오류가 발생하여 재처리하면서 예치금카드가 아닌 경우 /////////////////////
	// 예치금카드의 경우 잔액차감 중 오류에 대한 고려 없이 처음부터 다시
	// 시도하면 됨
	if ( pstTransInfo->bWriteErrCnt > 0 &&
		 pstTransInfo->bCardType != TRANS_CARDTYPE_DEPOSIT )
	{
		MIF_PREPAY_SECTOR2 stMifPrepaySector2;

		DebugOut( "[MifPrepayWrite] 구선불카드이면서 오류가 발생한 적이 " );
		DebugOut( "있음\n" );

		sResult = MifPrepayReadSector2( pstTransInfo->dwChipSerialNo,
			&stMifPrepaySector2 );
		if ( sResult != SUCCESS )
		{
			return ERR_CARD_PROC_RETAG_CARD;
		}

		// 현재 카드로부터 읽은 잔액이 이전환승정보의 잔액과 같지 않은 경우
		// 이미 구선불카드 WRITE의 모든 액션이 완료되었으며,
		// BLOCK9에 대한 DECREMENT 후 잔액 비교시 오류가 발생하였다고
		// 미루어 추정할 수 있다. 따라서 이러한 경우는 바로 TCC를 생성한 후
		// SUCCESS를 리턴한다.
		// 단, PSAM v2 카드는 TCC를 재생성하지 않는다.
		if ( stMifPrepaySector2.dwBalBlock9 !=
			 pstTransInfo->stPrevXferInfo.dwBal )
		{
			if ( pstTransInfo->boolIsTCard == TRUE )
			{
				DebugOut( "[MifPrepayWrite] T구선불카드의 경우 TCC를 \n");
				DebugOut( "재생성 않음\n");
			}
			else
			{
				byte abTempLastTransInfo[16] = {0, };

				DebugOut( "[MifPrepayWrite] 잔액이 정상적이므로 TCC만 생성\n" );

				// ISAMMakeTCC 수행 이전에 반드시 ISAMTransReq가 수행되어야 함
				ISAMTransReq( &stISAMTransReq, abTempLastTransInfo );

				// ISAM TCC 생성 ///////////////////////////////////////////////
				sResult = ISAMMakeTCC( &stISAMMakeTCC,
					pstTransInfo->abMifPrepayTCC );
				if ( sResult != SUCCESS )
				{
					printf( "[MifPrepayWrite] ISAMMakeTCC() 오류\n" );
					AddCardErrLog( sResult, pstTransInfo );
					return ERR_CARD_PROC_RETAG_CARD;
				}
			}

			// 재처리된 카드의 경우 오류LOG로부터 해당 카드정보를 삭제 /////////
			if ( pstTransInfo->sErrCode != SUCCESS )
			{
				DeleteCardErrLog( pstTransInfo->abCardNo );
			}

			return SUCCESS;
		}
		else
		{
			DebugOut( "[MifPrepayWrite] 잔액이 디크리먼트 이전이므로 " );
			DebugOut( "처음부터 다시 실행함\n" );
		}
	}

	// 오류가 발생하였던 예치금카드의 경우 디버그 메시지 출력
	if ( pstTransInfo->bWriteErrCnt > 0 &&
		 pstTransInfo->bCardType == TRANS_CARDTYPE_DEPOSIT )
	{
		DebugOut( "[MifPrepayWrite] 예치금카드의 경우 처음부터 다시 실행함\n" );
	}

	// 구선불카드 구환승정보 변환 //////////////////////////////////////////////
	MifPrepayBuildOldXferInfo( pstTransInfo );

	// 구선불카드 구환승정보WRITE //////////////////////////////////////////////
	sResult = MifPrepayWriteOldXferInfo( pstTransInfo->dwChipSerialNo,
		&pstTransInfo->stMifPrepayBlock18 );
	if ( sResult != SUCCESS )
	{
		printf( "[MifPrepayWrite] 구선불카드 구환승정보WRITE 오류\n" );
		AddCardErrLog( sResult, pstTransInfo );
		return ERR_CARD_PROC_RETAG_CARD;
	}

	// 구선불카드 환승정보WRITE ////////////////////////////////////////////////
	sResult = MifPrepayWriteXferInfo( pstTransInfo->dwChipSerialNo,
		&pstTransInfo->stNewXferInfo );
	if ( sResult != SUCCESS )
	{
		printf( "[MifPrepayWrite] 구선불카드 환승정보WRITE 오류\n" );
		AddCardErrLog( sResult, pstTransInfo );
		return ERR_CARD_PROC_RETAG_CARD;
	}

#ifdef TEST_WRITE_SLEEP
	printf( "TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT\n" );
	sleep( 2 );
#endif

	// 구선불카드 잔액 DECREMENT ///////////////////////////////////////////////
	// 예치금카드의 경우 0을 DECREMENT
	if ( pstTransInfo->bCardType == TRANS_CARDTYPE_DEPOSIT )
	{
		sResult = MifPrepayDecrementBalance( pstTransInfo->dwChipSerialNo,
			pstTransInfo->boolIsTCard,
			0, 0xFFFFFFFF, &stISAMTransReq, &stISAMMakeTCC,
			gstMyTermInfo.abPSAMID, pstTransInfo->abMifPrepayTCC );
	}
	// 일반구선불카드의 경우
	else
	{
		sResult = MifPrepayDecrementBalance( pstTransInfo->dwChipSerialNo,
			pstTransInfo->boolIsTCard,
			pstTransInfo->stNewXferInfo.dwFare,
			pstTransInfo->stNewXferInfo.dwBal,
			&stISAMTransReq, &stISAMMakeTCC,
			gstMyTermInfo.abPSAMID, pstTransInfo->abMifPrepayTCC );
	}
	if ( sResult != SUCCESS )
	{
		printf( "[MifPrepayWrite] 구선불카드 잔액 DECREMENT 오류\n" );
		AddCardErrLog( sResult, pstTransInfo );
		return ERR_CARD_PROC_RETAG_CARD;
	}

	// 재처리된 카드의 경우 오류LOG로부터 해당 카드정보를 삭제 /////////////////
	if ( pstTransInfo->sErrCode != SUCCESS )
	{
		DeleteCardErrLog( pstTransInfo->abCardNo );
	}

	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       MifPrepayBuildISAMTransReq                               *
*                                                                              *
*  DESCRIPTION:       카드정보 구조체와 전역변수의 내용을 이용하여 ISAM의      *
*                     TRANS REQ 명령어 호출에 필요한 구조체를 조립한다.        *
*                                                                              *
*  INPUT PARAMETERS:  pstTransInfo - 카드정보 구조체                           *
*                     pstISAMTransReq - ISAM TRANS REQ 명령어 호출에 필요한    *
*                         구조체                                               *
*                                                                              *
*  RETURN/EXIT VALUE: void                                                     *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-09-22                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static void MifPrepayBuildISAMTransReq( TRANS_INFO *pstTransInfo,
	ISAM_TRANS_REQ *pstISAMTransReq )
{
	memset( ( byte * )pstISAMTransReq, 0, sizeof( ISAM_TRANS_REQ ) );

	// 승하차시간
	pstISAMTransReq->tEntExtDtime = pstTransInfo->stNewXferInfo.tEntExtDtime;

	// 교통수단유형
	pstISAMTransReq->bTranspType = 1;				// 1: 버스, 2: 철도

	// 단말기ID
	memcpy( pstISAMTransReq->abTermID, gstMyTermInfo.abISAMID,
		sizeof( pstISAMTransReq->abTermID ) );

	// 승하차유형
	if ( IsEnt( pstTransInfo->stNewXferInfo.bEntExtType ) )
		pstISAMTransReq->bEntExtType = 1;			// 1: 승차, 0: 하차
	else
		pstISAMTransReq->bEntExtType = 0;

	// 정류장ID
	pstISAMTransReq->wStationID =
		GetWORDFromASC( pstTransInfo->stNewXferInfo.abStationID, 7 );

	// 요금
	// - 예치금카드의 경우 0원
	if ( pstTransInfo->bCardType == TRANS_CARDTYPE_DEPOSIT )
	{
		pstISAMTransReq->dwFare = 0;
	}
	// - 일반 구선불카드의 경우
	else
	{
		pstISAMTransReq->dwFare = pstTransInfo->stNewXferInfo.dwFare;
	}
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       MifPrepayBuildISAMMakeTCC                                *
*                                                                              *
*  DESCRIPTION:       카드정보 구조체와 전역변수의 내용을 이용하여 ISAM의      *
*                     MAKE TCC 명령어 호출에 필요한 구조체를 조립한다.         *
*                                                                              *
*  INPUT PARAMETERS:  pstTransInfo - 카드정보 구조체                           *
*                     pstISAMMakeTCC - ISAM MAKE TCC 명령어 호출에 필요한      *
*                         구조체                                               *
*                                                                              *
*  RETURN/EXIT VALUE: void                                                     *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-09-22                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static void MifPrepayBuildISAMMakeTCC( TRANS_INFO *pstTransInfo,
	ISAM_MAKE_TCC *pstISAMMakeTCC )
{
	memset( ( byte * )pstISAMMakeTCC, 0, sizeof( ISAM_MAKE_TCC ) );

	// 승하차시간
	pstISAMMakeTCC->tEntExtDtime = pstTransInfo->stNewXferInfo.tEntExtDtime;

	// ISAM ID
	pstISAMMakeTCC->dwISAMID = GetDWORDFromASC( gstMyTermInfo.abISAMID, 7 );

	// 카드번호
	memcpy( pstISAMMakeTCC->abCardNo, pstTransInfo->abCardNo,
		sizeof( pstISAMMakeTCC->abCardNo ) );

	// 요금
	if ( pstTransInfo->bCardType == TRANS_CARDTYPE_DEPOSIT )
	{
		pstISAMMakeTCC->dwFare = 0;
	}
	// - 일반 구선불카드의 경우
	else
	{
		pstISAMMakeTCC->dwFare = pstTransInfo->stNewXferInfo.dwFare;
	}
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       MifPrepayBuildOldXferInfo                                *
*                                                                              *
*  DESCRIPTION:       카드정보 구조체와 전역변수의 내용을 이용하여             *
*                     카드에 WRITE할 구선불카드 구환승영역의 내용을 조립한다.  *
*                                                                              *
*  INPUT PARAMETERS:  pstTransInfo - 카드정보 구조체                           *
*                                                                              *
*  RETURN/EXIT VALUE: void                                                     *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-09-22                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static void MifPrepayBuildOldXferInfo( TRANS_INFO *pstTransInfo )
{
	// 최종사용교통수단코드
	pstTransInfo->stMifPrepayBlock18.bLastUseTranspCode = 1;
												// 0: 철도, 1: 버스

	// 최종하차단말기SAM ID
	if ( pstTransInfo->boolIsTCard == TRUE )
	{
		memcpy( pstTransInfo->stMifPrepayBlock18.abLastISAMID,
			&gstMyTermInfo.abPSAMID[9],
			sizeof( pstTransInfo->stMifPrepayBlock18.abLastISAMID ) );
	}
	else
	{
		memcpy( pstTransInfo->stMifPrepayBlock18.abLastISAMID,
			gstMyTermInfo.abISAMID,
			sizeof( pstTransInfo->stMifPrepayBlock18.abLastISAMID ) );
	}

	// 최종하차일시
	pstTransInfo->stMifPrepayBlock18.tLastExtDtime =
		pstTransInfo->stNewXferInfo.tEntExtDtime;

	// 환승횟수 - 그대로 유지
	// TODO : FLAG.xfer

	// 최종충전데이터전송횟수 - 그대로 유지

	// 카드사용순차번호 - 1 증가
	pstTransInfo->stMifPrepayBlock18.bCardUseSeqNo++;
	if ( pstTransInfo->stMifPrepayBlock18.bCardUseSeqNo >= 0xFF )
		pstTransInfo->stMifPrepayBlock18.bCardUseSeqNo = 0x01;

	// 충전승인번호 - 그대로 유지

	// 환승할인처리적용FLAG - 우선 FALSE로 설정
	// TODO : FLAG.xfer
	pstTransInfo->stMifPrepayBlock18.boolIsXferDis = FALSE;

	// 예치금카드누적금액
	if ( pstTransInfo->bCardType == TRANS_CARDTYPE_DEPOSIT )
	{
		pstTransInfo->stMifPrepayBlock18.dwDepositCardBal =
			pstTransInfo->stNewXferInfo.dwBal;
	}
	// 예치금카드누적금액 - 예치금카드 이외의 경우 그대로 유지
}
