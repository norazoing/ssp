#include "../system/bus_type.h"
#include "card_proc.h"
#include "card_proc_util.h"
#include "blpl_proc.h"
#include "trans_proc.h"

/*******************************************************************************
*  Declaration of Defines                                                      *
*******************************************************************************/

// 카드 오류 분류 정의 /////////////////////////////////////////////////////////
#define NORMAL						0		// 정상 거래수행
#define REPURCHASE					1		// 재거래수행
#define PREPAY_CANCEL				2		// 선불카드 취소거래 후 거래
#define POSTPAY_CANCEL				3		// 후불카드 취소거래 후 거래
#define REWRITE						4		// 환승영역 다시쓰기
#define ABNORMAL					5		// 비정상 거래
											// (SAM 변동 -> 거래내역 저장만 함)

static short SCReadPurseInfo( SC_EF_PURSE_INFO *pstPurseInfo );
static short SCReadPurseAndTrans( SC_EF_PURSE_INFO *pstPurseInfo,
	COMMON_XFER_DATA *pstTrans,
	SC_EF_PURSE *pstPurse,
	SC_EF_PURSE *pstPurseLoad );
static short SCReadVerifyPurse( SC_EF_PURSE *pstPurse,
	SC_EF_PURSE *pstPurseLoad );
static short SCReadPayPrev( SC_EF_PURSE *pstPurse, COMMON_XFER_DATA *pstTrans );
static short SCPrepayCheckValidCard( TRANS_INFO *pstTransInfo,
	time_t tCardIssueDate );
static short SCPostpayCheckValidCard( TRANS_INFO *pstTransInfo,
	time_t tCardExpriryDate );
static void SCComTransInfo( SC_EF_PURSE_INFO *pstPurseInfo,
	SC_EF_PURSE *pstPurse,
	SC_EF_PURSE *pstPurseLoad,
	COMMON_XFER_DATA *pstCommonXferData,
	TRANS_INFO *pstTransInfo );
static short SCNormalPayment( TRANS_INFO *pstTransInfo );
static short SCRePayment( byte bWriteType, TRANS_INFO *pstTransInfo );
static void SCDeComTransInfo( TRANS_INFO *pstTransInfo,
	SC_EF_PURSE_INFO *pstPurseInfo,
	COMMON_XFER_DATA *pstCommonXferData );
static void SCSavePSAMResult( short sResult,
	PSAM_RES_TRANS *pstPSAMResult,
	TRANS_INFO *pstTransInfo );

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SCRead                                                   *
*                                                                              *
*  DESCRIPTION:       신카드 READ                                              *
*                                                                              *
*  INPUT PARAMETERS:  TRANS_INFO *pstTransInfo                                 *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS                                                  *
*                     ERR_CARD_PROC_SCREAD_PURSEINFO                           *
*                     ERR_CARD_PROC_LOG                                        *
*                     ERR_CARD_PROC_SCREAD_TRANS                               *
*                     ERR_CARD_PROC_INSUFFICIENT_BAL                           *
*                     ERR_CARD_PROC_SCREAD_PURSE                               *
*                     ERR_CARD_PROC_SCREAD                                     *
*                                                                              *
*  Author : MeeHyang Son													   *
*                                                                              *
*  DATE   : 2005-08-24 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short SCRead( TRANS_INFO *pstTransInfo )
{
	// 함수 결과값 선언
	short 				sResult = 0;

	// 신카드의 전자지갑파일, 전자지갑정보파일,
	// 환승파일의 정보를 담는 구조체 선언
	SC_EF_PURSE_INFO	stPurseInfo;
	COMMON_XFER_DATA	stTrans;
	SC_EF_PURSE		    stPurse;
	SC_EF_PURSE		    stPurseLoad;
byte abTempCardNo[21] = {0, };
	// 변수 초기화
	memset( ( byte * )&stPurseInfo, 0, sizeof( SC_EF_PURSE_INFO ) );
	memset( ( byte * )&stTrans,	 0, sizeof( COMMON_XFER_DATA ) );
	memset( ( byte * )&stPurse,	 0, sizeof( SC_EF_PURSE ) );
	memset( ( byte * )&stPurseLoad,	 0, sizeof( SC_EF_PURSE ) );
	memset( pstTransInfo->abCardNo, 'F', sizeof( pstTransInfo->abCardNo ) );

	DebugOut( "[smh] SCRead 시작------------\n" );

	// Select DF - 교통카드의 AID로 전자지갑정보파일을 선택하여
	// 전자지갑정보파일을 가져옴
	sResult = SCReadPurseInfo( &stPurseInfo );

	if ( sResult != SUCCESS )
	{
		DebugOut( "SCReadPurseInfo Read Error! !\n" );
		return ErrRet( sResult );
	}

	// 전자지갑 정보 파일 변환
	// 카드 타입 변환
	memcpy( pstTransInfo->abCardNo, stPurseInfo.abEpurseID,
		sizeof( stPurseInfo.abEpurseID ) );
	pstTransInfo->dwAliasNo =
		GetDWORDFromASC( stPurseInfo.abCardUserCertiID,10 );
	if ( ( stPurseInfo.bCardType & 0xF0 ) == 0 )
		pstTransInfo->bCardType = TRANS_CARDTYPE_SC_PREPAY;
	else if ( ( stPurseInfo.bCardType & 0xF0 ) == 0x10 )
		pstTransInfo->bCardType = TRANS_CARDTYPE_SC_POSTPAY;
	pstTransInfo->bCardUserType	= stPurseInfo.bUserTypeCode;
memcpy(abTempCardNo, pstTransInfo->abCardNo, 20);
LogMain("신 : %s\n", abTempCardNo);
	DebugOut( "[SCREAD] 신카드 PL 체크를 수행함\n" );
	// 선불카드 체크
	if ( pstTransInfo->bCardType == TRANS_CARDTYPE_SC_PREPAY  )
		sResult = SCPrepayCheckValidCard( pstTransInfo,
			stPurseInfo.tIssueDate );
	// 후불카드 체크
	else if ( pstTransInfo->bCardType == TRANS_CARDTYPE_SC_POSTPAY )
		sResult = SCPostpayCheckValidCard( pstTransInfo,
										  stPurseInfo.tExpiryDate );

	if ( sResult != SUCCESS )
	{
		DebugOut( "[SC_READ] 카드 유효성 체크 오류\n" );
		switch ( sResult )
		{
			case ERR_CARD_PROC_MIF_POSTPAY_INVALID_ISSUER_VALID_PERIOD:
			case ERR_CARD_PROC_MIF_POSTPAY_INVALID_CARD_NO:
			case ERR_CARD_PROC_MIF_POSTPAY_INVALID_ISSUER:
				return ERR_CARD_PROC_CANNOT_USE;
			case ERR_CARD_PROC_MIF_POSTPAY_EXPIRE:
				return ERR_CARD_PROC_EXPIRED_CARD;
			case ERR_CARD_PROC_NOT_APPROV:
			case ERR_CARD_PROC_CANNOT_USE:
			case ERR_CARD_PROC_RETAG_CARD:
				return sResult;
			default:
				return ERR_CARD_PROC_RETAG_CARD;
		}
	}

	// 환승 및 거래 내역 읽기 및
	// 전자지갑과 환승 영역 이상 여부 확인
	sResult = SCReadPurseAndTrans( &stPurseInfo, &stTrans, &stPurse,
		&stPurseLoad );
	if ( sResult != SUCCESS )
	{
		if ( ( sResult == ERR_CARD_PROC_SCREAD_VERIFY_PURSE ) ||
			 ( sResult == ERR_CARD_PROC_SCREAD_PURSE_LOAD ) )
			sResult= ERR_CARD_PROC_CANNOT_USE;
		else if ( sResult == ERR_CARD_PROC_INSUFFICIENT_BAL )
		{
			sResult = ErrRet( ERR_CARD_PROC_INSUFFICIENT_BAL );
		}
		else
		{
			sResult = ErrRet( ERR_CARD_PROC_RETAG_CARD );
		}

		return ErrRet( sResult );
	}

	// 오류LOG리스트에 존재하는 카드인지 확인
	sResult = SearchCardErrLog( pstTransInfo );
	if ( sResult == SUCCESS )
	{
		return ErrRet( ERR_CARD_PROC_LOG ); // 오류 처리
	}


	// 환승정보 구조체 조립
	SCComTransInfo( &stPurseInfo, &stPurse, &stPurseLoad, &stTrans,
		pstTransInfo );

	PrintTransInfo( pstTransInfo );
	DebugOut( "[smh] SCRead 끝------------\n" );
	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SCReadPurseInfo                                          *
*                                                                              *
*  DESCRIPTION:       신카드 전자지갑 정보파일 읽기                            *
*                                                                              *
*  INPUT PARAMETERS:  SC_EF_PURSE_INFO *pstPurseInfo                           *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS                                                  *
*                     ERR_CARD_PROC_RETAG_CARD                                 *
*                     ERR_CARD_PROC_CANNOT_USE                                 *
*                                                                              *
*  Author : MeeHyang Son													   *
*                                                                              *
*  DATE   : 2005-11-17 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short SCReadPurseInfo( SC_EF_PURSE_INFO *pstPurseInfo )
{
	// 함수 결과값 선언
	short 	sResult = 0;


	// Select DF - 교통카드의 AID로 전자지갑정보파일을 선택하여 정보를 가져옴
	sResult = SCSelectFilePurseDF( pstPurseInfo );

	if ( sResult != SUCCESS )
	{
		printf( "[신카드 읽기] 전자지갑정보파일 읽기 실패 \n" );
		sResult = ERR_CARD_PROC_RETAG_CARD;
		return ErrRet( sResult );
	}

	// 전자지갑파일 유효성 확인
	if ( IsValidSCPurseInfo( pstPurseInfo ) != TRUE )
	{
		printf( "[신카드 읽기]전자지갑정보파일 부정합 [E-77] !!\n" );
		// 구 소스에서 77 에러라고 하는 깨진 카드 오류
		sResult = ERR_CARD_PROC_CANNOT_USE;
		return ErrRet( sResult );
	}

	// 버스 단말기에서 사용 가능한 카드 체크
	if ( ( ( pstPurseInfo->bCardType & 0xF0 ) != 0x10 ) 	//후불카드 아님
		 &&( ( pstPurseInfo->bCardType & 0xF0 ) != 0x00 ) )	//선불카드 아님
	{
		printf( "[신카드 읽기]교통카드 선불/후불 카드 아님 \n" );
		sResult = ERR_CARD_PROC_CANNOT_USE;
		return ErrRet( sResult );
	}

#ifndef TEST_NOT_CHECK_TEST_CARD
	if ( pstPurseInfo->bEpurseIssuerID   == 0x01 )		//테스트 카드
	{
		printf( "[신카드 읽기]테스트 카드임!\n" );
		sResult = ERR_CARD_PROC_CANNOT_USE;
		return ErrRet( sResult );
	}
#endif

	return sResult;

}



/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SCReadPurseAndTrans                                      *
*                                                                              *
*  DESCRIPTION:       신카드 환승 및 거래내역 읽기                             *
*                     환승정보및 전자지갑 거래기록을 판독하는 기능             *
*                                                                              *
*  INPUT PARAMETERS:  SC_EF_PURSE_INFO *pstPurseInfo                           *
*                     COMMON_XFER_DATA *pstTrans                               *
*                     SC_EF_PURSE *pstPurse                                    *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS                                                  *
*                     ERR_CARD_PROC_SCREAD_TRANS                               *
*                     ERR_CARD_PROC_SCREAD_INSUFF_FARE                         *
*                     ERR_CARD_PROC_SCREAD_PURSE                               *
*                     ERR_CARD_PROC_SCREAD_VERIFY_PURSE                        *
*                     ERR_CARD_PROC_SCREAD                                     *
*                                                                              *
*  Author : MeeHyang Son													   *
*                                                                              *
*  DATE   : 2005-08-24 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short SCReadPurseAndTrans( SC_EF_PURSE_INFO *pstPurseInfo,
	COMMON_XFER_DATA *pstTrans,
	SC_EF_PURSE *pstPurse,
	SC_EF_PURSE *pstPurseLoad )
{
	short sResult = 0;
	bool boolIsNewCard = FALSE;

	// 환승정보 읽기
	sResult = SCReadRecordTrans( 1, pstTrans );
	if ( sResult != SUCCESS )
	{
		// 거래기록이 없으면
		// 첫사용 카드인지를 판별하고
		// 첫사용카드이면 환승정보구조체를 초기화한다.
		if ( sResult == ERR_CARD_IF_SC_NO_REC ) // New Card
		{
			memset( ( byte * )pstTrans, 0x00, sizeof( COMMON_XFER_DATA ) );
			sResult = SUCCESS;
			boolIsNewCard = TRUE;
			DebugOut( "[SMH] 환승정보 없는 첫사용카드임.\n" );
		}
		else
		{
			printf( "[신카드 읽기] 환승정보 읽기 실패 ErrCode[%x]\n", sResult );
			sResult = ERR_CARD_PROC_SCREAD_TRANS;
			return ErrRet( sResult ); // ERROR_TAG_AGAIN
		}
	}
	else
		DebugOut( "[SMH] 환승정보 1건이라도 있는  카드임.\n" );

	// 전자지갑파일 읽기
	sResult = SCReadRecordPurse( 1, pstPurse );
	if ( sResult != SUCCESS )
	{
		// 거래기록이 없으면 후불은 성공처리
		// 후불이 아니면 첫사용 카드인지를 판별하여 잔액부족 리턴
		// 거래기록은 없는데 첫사용이 아니면 이상한 카드 판단
		if ( sResult == ERR_CARD_IF_SC_NO_REC ) // New Card
		{
			 //후불카드이면 성공처리
			if ( ( pstPurseInfo->bCardType & 0xF0 ) == 0x10 )
			{
				memset( ( byte * )pstPurse, 0x00, sizeof( SC_EF_PURSE ) );
				sResult = SUCCESS;
			}
			else
			{
				if ( boolIsNewCard == TRUE )
				{
					printf( "[신카드 읽기] 충전 안된 첫사용 카드 \n" );
					sResult = ERR_CARD_PROC_INSUFFICIENT_BAL;
					return ErrRet( sResult );
				}
				else
				{
					printf( "[신카드 읽기] 환승정보 존재 but " );
					printf( "전자기갑 파일 없는 이상한 카드!! \n" );
					sResult = ERR_CARD_PROC_SCREAD_PURSE;
					return ErrRet( sResult );
				}
			}
		}
		else
		{
			printf( "[신카드 읽기] 전자지갑파일읽기 실패 ErrCode[%x]\n",
				sResult );
			sResult = ERR_CARD_PROC_RETAG_CARD;
			return ErrRet( sResult );
		}
	}

	// 잔액 정보 저장
	pstTrans->dwBal  = pstPurse->dwEpurseBal;

	// 전자지갑파일 유효성 검증
	sResult = SCReadVerifyPurse( pstPurse, pstPurseLoad );
	if ( sResult != SUCCESS )
	{
		return ErrRet( sResult );
	}

	// 신포멧이( 0x0A ) 아니면 직전 거래금액을 읽어서 유추해야 함
	if ( pstTrans->boolSCIsNewTransFormat != TRUE )
	{
		// 직전거래를 읽어서 직전거래금액 저장
		sResult = SCReadPayPrev( pstPurse, pstTrans );
		if ( sResult != SUCCESS )
		{
			//sResult = ERR_CARD_PROC_SCREAD;
			printf( "[신카드 읽기] 직전거래금액읽기 실패 ErrCode[%x]\n",
				sResult );
			return ErrRet( sResult );
		}
	}
	return ( sResult );

}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SCReadVerifyPurse                                        *
*                                                                              *
*  DESCRIPTION:       전자지갑 거래기록이 올바르게 기록된 상태를 확인          *
*                                                                              *
*  INPUT PARAMETERS:  SC_EF_PURSE *pstPurse                                    *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS                                                  *
*                     ERR_CARD_PROC_SCREAD_LOAD_ERR                            *
*                     ERR_CARD_PROC_SCREAD_VERIFY_PURSE                        *
*                                                                              *
*  Author : MeeHyang Son													   *
*                                                                              *
*  DATE   : 2005-08-24 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short SCReadVerifyPurse( SC_EF_PURSE *pstPurse,
	SC_EF_PURSE *pstPurseLoad )
{
	short	sResult = 0;
	byte	bCount;				// Record #
	dword	dwCurBal;			// 최종 잔액
	dword	dwBefBal;			// Second Record의 잔액
	dword	dwCurUsedBal;		// First Record의 거래금액
	byte	bTRT;				// 거래구분
	dword	dwEPCounter;		// 거래번호
	SC_EF_PURSE stBefPurse;

	bTRT 		= pstPurse->bTransType;
	dwCurBal		= pstPurse->dwEpurseBal;
	dwEPCounter 	= pstPurse->dwEpurseTransSeqNo;
	dwCurUsedBal	= pstPurse->dwTransAmt;

	bCount  = 2;

	// 직전 및 직직전 충전거래내역도 거래기록에 추가함 ( 0338대로 하라고 함 )
	// 직전이 충전이면 안함
	if (  bTRT  == TRT_LOAD )
	{
		memcpy( pstPurseLoad, pstPurse, sizeof( SC_EF_PURSE ) );
	}

	sResult = SCReadRecordPurse( bCount, &stBefPurse );

	if ( sResult != SUCCESS )
	{
		if ( sResult == ERR_CARD_IF_SC_NO_REC )
		{
			if ( dwEPCounter < 4 )
			{
				sResult = SUCCESS;	// 정상( 거래가 0건임 )
				return sResult;
			}
			// 최초 충전시에 거래번호는 1로 시작되며,
			// 충전에러시에는 재시도시 거래번호가 증가됨.
			// 즉 최초 충전시에 에러가 연속 3회 이상 발생된 카드는 사용불가.
			else
			{
				printf( "[신카드 읽기] 충전최초 충전시에 에러가 연속 3회 " );
				printf( "이상 발생된 카드는 사용불가.\n" );
				return ERR_CARD_PROC_SCREAD_PURSE_LOAD;
			}
		}
		else
			{
				printf( "[신카드 읽기] 전자지갑파일 읽기 실패 \n " );
				return ERR_CARD_PROC_SCREAD_PURSE; // Read Error
			}
	}

	dwBefBal	= stBefPurse.dwEpurseBal;

	switch ( bTRT )
	{
//		case 0x00:	//  Not used( JCOP은 미사용카드일 경우 all 0값이 나옴 )
		case TRT_PURCHASE:		// 전자지갑 지불
		case TRT_UNLOAD:		// 전자지갑 환불
		case TRT_CANCELLOAD:	// 전자지갑 충전취소
			if ( ( dwCurBal + dwCurUsedBal ) != dwBefBal )// 잔액 비정상카드
				sResult = ErrRet( ERR_CARD_PROC_SCREAD_VERIFY_PURSE );
			break;

		case TRT_LOAD:			// 전자지갑 충전
		case TRT_AUTOLOAD:		// 자동충전
		case TRT_CANCEL:		// 마지막 구매 거래 취소
			if ( ( dwCurBal - dwCurUsedBal ) != dwBefBal )// 잔액 비정상카드
				sResult = ErrRet( ERR_CARD_PROC_SCREAD_VERIFY_PURSE );
			break;
		// 후불카드는 월초에 잔액이 Clear 되므로 Check하면 안됨.
		case TRT_PURCHASE_POST:	// 후불카드 거래
		case TRT_CANCEL_POST:	// 후불카드 거래 취소
			sResult = SUCCESS;
			break;
		default:
			sResult= ErrRet( ERR_CARD_PROC_SCREAD_VERIFY_PURSE );
			break;
	}

	if ( sResult == ERR_CARD_PROC_SCREAD_VERIFY_PURSE )
	{
		printf( "[신카드 읽기] 잔액 비정상 : 거래구분 [%02x]", bTRT );
		printf( " 현잔액 = %ld  직전거래금액 = %ld ", dwCurBal, dwCurUsedBal );
		printf( " 직전잔액 = %ld\n", dwBefBal );
	}
	else if ( (  bTRT  != TRT_LOAD ) && ( stBefPurse.bTransType == TRT_LOAD ) )
	{
		memcpy( pstPurseLoad, &stBefPurse, sizeof( SC_EF_PURSE ) );
	}

	return ( sResult );

}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SCReadPayPrev                                            *
*                                                                              *
*  DESCRIPTION:       전자지갑에서 직전 거래금액을 추출한다                    *
*                     환승정보에 직전 거래금액이 없을 경우,                    *
*                     자지갑 파일의 직전 거래가 충전/환불 등의 거래일 경우엔   *
*                     지불 내역이나올때까지 계속 레코드번호 증가하면서 읽음    *
*                                                                              *
*  INPUT PARAMETERS:  SC_EF_PURSE *pstPurse, COMMON_XFER_DATA *pstTrans        *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS                                                  *
*                     ERR_CARD_PROC_SCREAD_PAY_PREV                            *
*                                                                              *
*  Author : MeeHyang Son													   *
*                                                                              *
*  DATE   : 2005-08-24 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short SCReadPayPrev( SC_EF_PURSE *pstPurse, COMMON_XFER_DATA *pstTrans )
{
	byte 		bCount;
	bool		boolIsLoopEnd = FALSE;
	short 		sResult = 0;
	SC_EF_PURSE stPrevPurse;

	switch ( pstPurse->bTransType )
	{
		case 0x00:					// 거래기록 없음
		case TRT_PURCHASE:			// 전자지갑 지불
		case TRT_PURCHASE_POST:		// 후불카드 거래
			pstTrans->dwFare = pstPurse->dwTransAmt;
			sResult = SUCCESS;
			return ( sResult );
	}

	for ( bCount = 0x02; boolIsLoopEnd == FALSE; bCount++ )
	{
		memset( ( byte * )&stPrevPurse,	 0, sizeof( SC_EF_PURSE ) );
		sResult = SCReadRecordPurse( bCount, &stPrevPurse );

		if ( sResult == SUCCESS )
		{
			switch ( stPrevPurse.bTransType )
			{
				case TRT_PURCHASE:		// 전자지갑 지불
				case TRT_PURCHASE_POST:	// 후불카드 거래
					boolIsLoopEnd = TRUE;
					pstTrans->dwFare =  stPrevPurse.dwTransAmt;
			}
		}
		else
		{
			if ( sResult == ERR_CARD_IF_SC_NO_REC )  // Record 없음
			{
				pstTrans->dwFare =  0;
				boolIsLoopEnd = TRUE;
				sResult = SUCCESS;
			}
			else
			{
				boolIsLoopEnd = TRUE;
				sResult = ErrRet( ERR_CARD_PROC_SCREAD_PAY_PREV );
			}
		}
	}
	return ( sResult );
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SCPrepayCheckValidCard                                   *
*                                                                              *
*  DESCRIPTION:       신선불카드 유효성체크                                    *
*                                                                              *
*  INPUT PARAMETERS:  TRANS_INFO *pstTransInfo                                 *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS                                                  *
*                     ERR_CARD_PROC_SCREAD_PREPAY_CHECK                        *
*                     ERR_CARD_PROC_SCREAD_NL_CARD                             *
*                                                                              *
*  Author : MeeHyang Son													   *
*                                                                              *
*  DATE   : 2005-08-24 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short SCPrepayCheckValidCard( TRANS_INFO *pstTransInfo,
	time_t tCardIssueDate )
{
	short sResult = 0;
	byte  bPLValue = 0;
	byte  abCardIssuerNo[7] = {0, };
	bool  boolResult;

	// 신선불카드도 발행사 체크 0338부터 수행
	// 신선불카드는 구선불카드와 달리 발행사 코드가 카드번호 앞 6자리이므로
	// 앞에 '3'+ 카드번호 앞 6자리를 발행사 코드로 관리한다고 함...
	// 참고로 구선불카드는 20자리 중 앞 7자리가 발행사 코드임
	abCardIssuerNo[0] = '3';
	memcpy( &abCardIssuerNo[1], pstTransInfo->abCardNo, 6 );

	DebugOutlnASC( "신선불 발행사 ID ==> ", abCardIssuerNo, 7 );

	// 시티투어버스인 경우
	if ( IsCityTourBus( gstVehicleParm.wTranspMethodCode ) )
	{
		// 발행사ID가 '3101'이 아닌 경우 '미승인카드입니다' 음성 출력
		if ( memcmp( abCardIssuerNo, "3101", 4 ) != 0 )
		{
			printf( "[SCPrepayCheckValidCard] 관광권카드 발행사 '3101' 아님 " );
			printf( "오류\n" );
			return ERR_CARD_PROC_NOT_APPROV;
		}
	}

	boolResult = IsValidPrepayIssuer( abCardIssuerNo );
	if ( !boolResult )
	{
		printf( "[SCPrepayCheckValidCard] 신선불카드 발행사 체크 오류\n" );
		return ERR_CARD_PROC_NOT_APPROV;
	}

	boolResult = IsValidSCPrepayCardNo( pstTransInfo->abCardNo );
	if ( !boolResult )
	{
		printf( "[SCPrepayCheckValidCard] 신선불카드 카드번호 오류\n" );
		return ERR_CARD_PROC_NOT_APPROV;
	}

	boolResult = IsValidSCPrepayIssueDate( pstTransInfo->abCardNo,
		tCardIssueDate );
	if ( !boolResult )
	{
		printf( "[SCPrepayCheckValidCard] 신선불카드 발급일 체크 오류\n" );
		return ERR_CARD_PROC_CANNOT_USE;
	}

#ifndef TEST_NOT_CHECK_BLPL
	// PL 체크
	sResult = SearchPLinBus( pstTransInfo->abCardNo, pstTransInfo->dwAliasNo,
		&bPLValue );

	if ( sResult == ERR_PL_FILE_OPEN_MASTER_AI )
	{
		printf( "[SCPostpayCheckValidCard] 신선불PL이 미존재\n" );
		switch ( pstTransInfo->bCardUserType )
		{
			// 카드의 사용자 구분 코드가 어린이인 경우 어린이로 함
			case USERTYPE_CHILD :
				pstTransInfo->bPLUserType = USERTYPE_CHILD;
				break;
			// 카드 사용자 구분 코드가 청소년/학생인 경우는 청소년 요금임
			case USERTYPE_STUDENT:
				pstTransInfo->bPLUserType = USERTYPE_YOUNG;
				break;
			case USERTYPE_TEST :
				pstTransInfo->bPLUserType = USERTYPE_TEST;
				break;
			default :
				pstTransInfo->bPLUserType = USERTYPE_ADULT;
				break;
		}
		return SUCCESS;
	}
	if ( sResult != SUCCESS )
	{
		printf( "[SCPostpayCheckValidCard] 기타 알 수 없는 이유로 " );
		printf( "후불PL체크 실패\n" );
		return ErrRet( ERR_CARD_PROC_RETAG_CARD );
	}

	if ( bPLValue == 0 )
	{
		printf( "[SCPrepayCheckValidCard] NL인 카드입니다\n" );
		sResult = ERR_CARD_PROC_NOT_APPROV;
		return ErrRet( sResult );
	}
	else if ( bPLValue > 3 )
	{
		printf( "[SCPrepayCheckValidCard] 신선불카드 PL 체크 결과값 ==> [%x] ",
			bPLValue );
		printf( "PL 결과값 이상 함\n" );
		sResult = ERR_CARD_PROC_SCREAD_PREPAY_CHECK;
		return ErrRet( sResult );
	}

	// pstTransInfo->bPLUserType 에는 요금계산을 위해 사용자 구분코드로 사용함
	// 0338 소스에는 그렇지 않으나 요금 계산을 위해서는
	// NL이 아닌 카드에 대해서는 PL 결과값과 카드의 사용자구분 중
	// 요금이 많이 나오는 것으로 계산하도록 한다고 함
	// 현재 카드의 사용자 유형은 다양하게 구분이 되므로
	// 향후 경로/장애자/국가유공자 등의 요금이 다르게 형성된다면
	// 이부분 수정해야 함
	switch ( pstTransInfo->bCardUserType )
	{
		// 카드의 사용자 구분 코드가 어린이인 경우는 PL체크 결과값을 따름
		case USERTYPE_CHILD :
			switch ( bPLValue )
			{
				case 1 : // P/L 비트에서 청소년 요금
					pstTransInfo->bPLUserType = USERTYPE_YOUNG;
					break;
				case 2 : // P/L 비트에서 어린이 요금
					pstTransInfo->bPLUserType = USERTYPE_CHILD;
					break;
				case 3 : // P/L 비트에서 일반 요금
					pstTransInfo->bPLUserType = USERTYPE_ADULT;
					break;
			}
			break;
		// 카드 사용자 구분 코드가 청소년/학생인 경우는
		// PL체크 결과값이 일반인경우는 일반요금이고 나머지는 청소년 요금임
		case USERTYPE_STUDENT:
		case USERTYPE_YOUNG:
			if ( bPLValue == 3 )
				pstTransInfo->bPLUserType = USERTYPE_ADULT;
			else
				pstTransInfo->bPLUserType = USERTYPE_YOUNG;
			break;
		case USERTYPE_TEST :
			pstTransInfo->bPLUserType = USERTYPE_TEST;
			break;
		default :
			pstTransInfo->bPLUserType = USERTYPE_ADULT;
			break;
	}
#endif

	return ( sResult );
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SCPostpayCheckValidCard                                  *
*                                                                              *
*  DESCRIPTION:       신후불카드 유효성체크                                    *
*                                                                              *
*  INPUT PARAMETERS:  TRANS_INFO *pstTransInfo, byte * abCardExpriryDate       *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS                                                  *
*                     ERR_CARD_PROC_SCREAD_POSTPAY_CHECK                       *
*                     ERR_CARD_PROC_SCREAD_NL_CARD                             *
*                     ERR_CARD_PROC_SCREAD                                     *
*  Author : MeeHyang Son													   *
*                                                                              *
*  DATE   : 2005-08-24 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short SCPostpayCheckValidCard( TRANS_INFO *pstTransInfo,
	time_t tCardExpriryDate )
{
	short sResult = 0;
	byte abTempExpiryDate[8];
	byte abExpiryDate[6];
	byte bPLValue;
	time_t tNowDate;
	bool boolValidCardNo;


	// 카드번호 유효성 체크
	// 발행사 정보를 알수 없어서 그냥 적용
	if ( pstTransInfo->abCardNo[15] == 'F' )
		boolValidCardNo = IsValidAmexCardNo( pstTransInfo->abCardNo );
	else
		boolValidCardNo = IsValidISOCardNo( pstTransInfo->abCardNo );

	if ( boolValidCardNo != TRUE )
	{
		DebugOut( "[SCPostpayCheckValidCard] 카드번호 오류\n" );
		return ERR_CARD_PROC_MIF_POSTPAY_INVALID_CARD_NO;

	}

	TimeT2ASCDate( tCardExpriryDate, abTempExpiryDate );
	memcpy( abExpiryDate, abTempExpiryDate, sizeof( abExpiryDate ) );
	tNowDate = pstTransInfo->stNewXferInfo.tEntExtDtime;

#ifndef TEST_NOT_CHECK_EXPIRY_DATE
	// 카드 유효기간 체크
	if ( !IsValidExpiryDate( abExpiryDate, tNowDate ) )
	{
		DebugOut( "[SCPostpayCheckValidCard] 유효기간 오류\n" );
		return ErrRet( ERR_CARD_PROC_MIF_POSTPAY_EXPIRE );
	}
#endif

	// 발행사 체크
	if ( !IsValidPostpayIssuer( pstTransInfo->abCardNo ) )
	{
		DebugOut( "[SCPostpayCheckValidCard] prefix 오류\n" );
		return ErrRet( ERR_CARD_PROC_MIF_POSTPAY_INVALID_ISSUER );
	}

	// 발행사 유효기간 체크
	if ( !IsValidIssuerValidPeriod( pstTransInfo->abCardNo, abExpiryDate ) )
	{
		DebugOut( "[SCPostpayCheckValidCard] 후불발행사유효기간 체크 오류\n" );
		return ErrRet( ERR_CARD_PROC_MIF_POSTPAY_INVALID_ISSUER_VALID_PERIOD );
	}

#ifndef TEST_NOT_CHECK_BLPL
	// PL 체크
	sResult = SearchPLinBus( pstTransInfo->abCardNo, pstTransInfo->dwAliasNo,
		&bPLValue );

	// 통신 실패시에 신후불카드는 미승인 처리
	if ( sResult == ERR_PL_FILE_OPEN_MASTER_POSTPAY_PL )
	{
		printf( "[SCPostpayCheckValidCard] 후불PL이 미존재\n" );
		return ErrRet( ERR_CARD_PROC_NOT_APPROV );
	}
	if ( sResult != SUCCESS )
	{
		printf( "[SCPostpayCheckValidCard] 기타 알 수 없는 이유로 " );
		printf( "후불PL체크 실패\n" );
		return ErrRet( ERR_CARD_PROC_RETAG_CARD );
	}

	if ( bPLValue == 0 ) // NL 카드 처리
	{
		DebugOut( "[SCPostpayCheckValidCard] NL인 카드입니다\n" );
		sResult = ErrRet( ERR_CARD_PROC_NOT_APPROV );
	}
	else if ( bPLValue > 1 ) // PL체크값이 이상하면 오류 처리
	{
		DebugOut( "[SMH]신후불카드 PL 체크 결과값 ==> [%x]", bPLValue );
		DebugOut( " PL 결과값 이상 함\n" );
		sResult = ErrRet( ERR_CARD_PROC_SCREAD_POSTPAY_CHECK );
	}
	else
	{
		pstTransInfo->bPLUserType = USERTYPE_ADULT;

	}
#endif

	return ( sResult );
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SCComTransInfo                                           *
*                                                                              *
*  DESCRIPTION:       신카드 카드정보구조체 설정                               *
*                                                                              *
*  INPUT PARAMETERS:  SC_EF_PURSE_INFO *pstPurseInfo                           *
*                     SC_EF_PURSE *pstPurse                                    *
*                     COMMON_XFER_DATA *pstTrans                               *
*                     TRANS_INFO *pstTransInfo                                 *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS                                                  *
*                                                                              *
*  Author : MeeHyang Son													   *
*                                                                              *
*  DATE   : 2005-08-24 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static void SCComTransInfo( SC_EF_PURSE_INFO *pstPurseInfo,
	SC_EF_PURSE *pstPurse,
	SC_EF_PURSE *pstPurseLoad,
	COMMON_XFER_DATA *pstCommonXferData,
	TRANS_INFO *pstTransInfo )
{
	pstTransInfo->bSCAlgoriType			= pstPurseInfo->bAlgoriID;
	pstTransInfo->bSCTransKeyVer		= pstPurseInfo->bEpurseKeysetVer;
	pstTransInfo->bSCEpurseIssuerID		= pstPurseInfo->bEpurseIssuerID;
	memcpy( pstTransInfo->abSCEpurseID, pstPurseInfo->abEpurseID,
		sizeof( pstTransInfo->abSCEpurseID ) );

	pstTransInfo->dwSCTransCnt 			= pstPurse->dwEpurseTransSeqNo;

	// 환승 정보 파일 저장
	memcpy( &pstTransInfo->stPrevXferInfo, pstCommonXferData,
			sizeof( COMMON_XFER_DATA ) );

	// 직전 충전 관련 데이터 저장
	if ( pstPurseLoad->bTransType == TRT_LOAD )
	{
		pstTransInfo->dwBalAfterCharge = pstPurseLoad->dwEpurseBal;
		pstTransInfo->dwChargeTransCnt = pstPurseLoad->dwEpurseTransSeqNo;
		pstTransInfo->dwChargeAmt = pstPurseLoad->dwTransAmt;
		memcpy( pstTransInfo->abLSAMID, pstPurseLoad->abSAMID,
			sizeof( pstTransInfo->abLSAMID ) );
		pstTransInfo->dwLSAMTransCnt = pstPurseLoad->dwSAMTransSeqNo;
		pstTransInfo->bChargeTransType = pstPurseLoad->bTransType;
	}
	else
	{
		pstTransInfo->dwBalAfterCharge = 0UL;
		pstTransInfo->dwChargeTransCnt = 0UL;
		pstTransInfo->dwChargeAmt	   = 0UL;
		memset( pstTransInfo->abLSAMID, 0, sizeof( pstTransInfo->abLSAMID ) );
		pstTransInfo->dwLSAMTransCnt   = 0UL;
		pstTransInfo->bChargeTransType = 0x00;
	}
	pstTransInfo->sErrCode = 0x00;

}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SCWrite                                                  *
*                                                                              *
*  DESCRIPTION:       신카드 WRITE                                             *
*                                                                              *
*  INPUT PARAMETERS:  TRANS_INFO *pstTransInfo                                 *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS                                                  *
*                     ???                                                      *
*                                                                              *
*  Author : MeeHyang Son													   *
*                                                                              *
*  DATE   : 2005-08-24 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short SCWrite( TRANS_INFO *pstTransInfo )
{
	short	sResult = SUCCESS;	// 함수 리턴값
	byte	bWriteType = 0;		// 이전의 에러 종류를 이용하여
								// 거래/재시도/재거래/취소거래/환승다시쓰기변수

	SC_EF_PURSE_INFO	stPurseInfo;
	SC_EF_PURSE			stReadPurse;
	COMMON_XFER_DATA	stCommonXferData;

	memset( &stPurseInfo, 0x00, sizeof( SC_EF_PURSE_INFO ) );
	memset( &stCommonXferData, 0x00, sizeof( COMMON_XFER_DATA ) );

	DebugOut( "[smh] SCWrite 시작------------\n" );

	// 이전의 에러 종류를 이용하여 거래/재시도/재거래/취소거래/환승다시쓰기 수행
	switch ( pstTransInfo->sErrCode )
	{

		// 신카드 정상 거래
		case 0x00 :
		// 신카드 거래 Initialize 오류
		case ERR_PAY_LIB_SC_PREPAY_PURCHASE_INIT_CARD :
		case ERR_PAY_LIB_SC_PREPAY_PURCHASE_INIT_SAM :
		case ERR_PAY_LIB_SC_POSTPAY_PURCHASE_INIT_CARD :
		case ERR_PAY_LIB_SC_POSTPAY_PURCHASE_INIT_SAM :
			bWriteType = NORMAL;
			break;
			
		// 신카드 카드거래 오류 중 purchase 오류는 잔액 확인
		case ERR_PAY_LIB_SC_PREPAY_PURCHASE_CARD:
		case ERR_PAY_LIB_SC_POSTPAY_PURCHASE_GENERATE_TC:
		case ERR_PAY_LIB_VERIFY_PURSE:
		case ERR_PAY_LIB_SC_VERIFY_PURSE_TRANS_TYPE:
		case ERR_PAY_LIB_SC_VERIFY_PURSE_SAM_ID:
			// 카드의 잔액을 읽어봄
			sResult = SCReadRecordPurse( 1, &stReadPurse );
			if ( sResult != SUCCESS )
			{
				return ERR_CARD_PROC_RETAG_CARD;
			}
			// 이전 잔액과 카드 잔액이 같으면 -> 지불거래
			if ( stReadPurse.dwEpurseBal == pstTransInfo->stPrevXferInfo.dwBal )
				bWriteType = NORMAL;
			// 이전 잔액과 카드 잔액이 다르면 -> 재거래
			else
			{
				if ( memcmp( pstTransInfo->abPSAMID,
					gstMyTermInfo.abPSAMID, 16 ) != 0 )
					bWriteType = ABNORMAL;
				else
					bWriteType = REPURCHASE;
			}
			break;
		// 신선불 재거래 오류
		case ERR_PAY_LIB_SC_PREPAY_REPURCHASE_INIT_CARD:
		case ERR_PAY_LIB_SC_PREPAY_REPURCHASE_INIT_SAM:
		case ERR_PAY_LIB_SC_PREPAY_REPURCHASE_CARD:
		case ERR_PAY_LIB_SC_PREPAY_REPURCHASE_CREDIT_SAM:
		case ERR_PAY_LIB_SC_PREPAY_PURCHASE_CREDIT_SAM:
			if ( memcmp( pstTransInfo->abPSAMID,
				gstMyTermInfo.abPSAMID, 16 ) != 0 )
				bWriteType = ABNORMAL;
			else
				bWriteType = REPURCHASE;
			break;
		// 신후불 재거래 오류
		case ERR_PAY_LIB_SC_POSTPAY_CANCEL_INIT_CARD:
		case ERR_PAY_LIB_SC_POSTPAY_CANCEL_INIT_SAM:
		case ERR_PAY_LIB_SC_POSTPAY_CANCEL_GENERATE_TC:
		case ERR_PAY_LIB_SC_POSTPAY_CANCEL_VERIFY_TC:
		// 신후불 SAM 거래 오류
		case ERR_PAY_LIB_SC_POSTPAY_PURCHASE_VERIFY_TC:
			if ( memcmp( pstTransInfo->abPSAMID,
				gstMyTermInfo.abPSAMID, 16 ) != 0 )
				bWriteType = ABNORMAL;
			else
				bWriteType = POSTPAY_CANCEL;
			break;
		// 신선불 취소 오류
		case ERR_PAY_LIB_SC_PREPAY_CANCEL_INIT_CARD:
		case ERR_PAY_LIB_SC_PREPAY_CANCEL_INIT_SAM:
			if ( memcmp( pstTransInfo->abPSAMID,
				gstMyTermInfo.abPSAMID, 16 ) != 0 )
				bWriteType = ABNORMAL;
			else
				bWriteType = PREPAY_CANCEL;
			break;
		// 환승정보 쓰기 오류
		case ERR_PAY_LIB_GENERATE_MAC:
		case ERR_PAY_LIB_APPEND_TRANS_RECORD:
		case ERR_PAY_LIB_VERIFY_TRANS:
			bWriteType = REWRITE;
			break;
		case ERR_SAM_IF_PSAM_NO_SIGN3:
			bWriteType = ABNORMAL;			
			break;			
		default :
			bWriteType = ABNORMAL;	
			break;
	}

	switch ( bWriteType )
	{
		case NORMAL :
			sResult = SCNormalPayment( pstTransInfo );
			break;
		case REPURCHASE :
		case POSTPAY_CANCEL :
		case PREPAY_CANCEL :
			sResult = SCRePayment( bWriteType, pstTransInfo );
			break;
		case ABNORMAL:
		case REWRITE :
			// 시티투어버스의 경우 환승정보를 WRITE하지 않음
			if ( IsCityTourBus( gstVehicleParm.wTranspMethodCode ) == FALSE )
			{
				// 환승정보 구조체 분해
				SCDeComTransInfo( pstTransInfo, &stPurseInfo,
					&stCommonXferData );

				// 환승정보 APPEND
				sResult = SCAppendXferInfo( TRANS_REWRITE, &stPurseInfo,
					&stCommonXferData );
			}
			break;
	}

	DebugOut( "[smh] SCWrite 끝------------\n" );

	// 신카드 오류처리 백업을 위해 리스트에 추가
	if ( sResult != SUCCESS )
	{
		AddCardErrLog( sResult, pstTransInfo );
		return ErrRet( ERR_CARD_PROC_RETAG_CARD );
	}

	// 재처리된 카드의 경우 오류LOG로부터 해당 카드정보를 삭제 /////////////////
	else if ( pstTransInfo->sErrCode != SUCCESS )
		DeleteCardErrLog( pstTransInfo->abCardNo );

	return ( sResult );
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SCNormalPayment                                          *
*                                                                              *
*  DESCRIPTION:       신카드 지불거래( 정상적인 경우 )                         *
*                                                                              *
*  INPUT PARAMETERS:  TRANS_INFO *pstTransInfo                                 *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS                                                  *
*                     ???                                                      *
*                                                                              *
*  Author : MeeHyang Son													   *
*                                                                              *
*  DATE   : 2005-08-24 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short SCNormalPayment( TRANS_INFO *pstTransInfo )
{
	short sResult = 0;

	SC_EF_PURSE_INFO	stPurseInfo;
	COMMON_XFER_DATA	stCommonXferData;
	PSAM_RES_TRANS		stPSAMResult;
	dword dwTransAmt;

	memset( ( byte * )&stPurseInfo, 		0, sizeof( SC_EF_PURSE_INFO ) );
	memset( ( byte * )&stCommonXferData,	0, sizeof( COMMON_XFER_DATA ) );
	memset( ( byte * )&stPSAMResult,	 	0, sizeof( PSAM_RES_TRANS ) );

	dwTransAmt = pstTransInfo->stNewXferInfo.dwFare;
	// 신선불카드 지불거래
	if ( pstTransInfo->bCardType == TRANS_CARDTYPE_SC_PREPAY )
	{

		sResult = SCPrepayPurchase( dwTransAmt, &stPSAMResult );
		// SAM 거래 내역 저장
		SCSavePSAMResult( sResult, &stPSAMResult, pstTransInfo );
		if ( sResult != SUCCESS )
		{
			DebugOut( "SCPrepayPurchase 오류 : %x\n", sResult );
			return ( sResult );
		}
	}
	// 신후불카드 지불거래
	else if ( pstTransInfo->bCardType == TRANS_CARDTYPE_SC_POSTPAY )
	{
		sResult = SCPostpayPurchase( pstTransInfo->boolIsChangeMonth,
			dwTransAmt, &stPSAMResult );
		// SAM 거래 내역 저장
		SCSavePSAMResult( sResult, &stPSAMResult, pstTransInfo );
		if ( sResult != SUCCESS )
		{
			DebugOut( "SCPostpayPurchase 오류 : %x\n", sResult );
			return ErrRet( sResult );
		}
	}

#ifdef TEST_WRITE_SLEEP
	printf( "TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT\n" );
	sleep( 2 );
#endif

	// 시티투어버스의 경우 환승정보를 WRITE하지 않음
	if ( IsCityTourBus( gstVehicleParm.wTranspMethodCode ) == FALSE )
	{
		// 환승정보 구조체 분해
		SCDeComTransInfo( pstTransInfo, &stPurseInfo, &stCommonXferData );

		// 환승정보 APPEND
		sResult = SCAppendXferInfo( TRANS_NORMAL, &stPurseInfo,
			&stCommonXferData );
		if ( sResult != SUCCESS )
		{
			printf( "[SCNormalPayment] SCAppendXferInfo() 실패\n" );
			return ErrRet( sResult );
		}
	}
	
	return ( sResult );
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SCRePayment                                              *
*                                                                              *
*  DESCRIPTION:       신카드 재지불거래                                        *
*                                                                              *
*  INPUT PARAMETERS:  byte bWriteType :REPURCHASE/PREPAY_CANCEL/POSATPAY_CANCEL*
*						TRANS_INFO *pstTransInfo                               *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS                                                  *
*                     ???                                                      *
*                                                                              *
*  Author : MeeHyang Son													   *
*                                                                              *
*  DATE   : 2005-08-24 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short SCRePayment( byte bWriteType, TRANS_INFO *pstTransInfo )
{
	short sResult = 0;
	SC_EF_PURSE_INFO	stPurseInfo;
	COMMON_XFER_DATA	stCommonXferData;
	PSAM_RES_TRANS		stPSAMResult;
	dword dwTransAmt = 0;

	memset( ( byte * )&stPurseInfo, 		0, sizeof( SC_EF_PURSE_INFO ) );
	memset( ( byte * )&stCommonXferData,	0, sizeof( COMMON_XFER_DATA ) );
	memset( ( byte * )&stPSAMResult,	 	0, sizeof( PSAM_RES_TRANS ) );

	dwTransAmt = pstTransInfo->stNewXferInfo.dwFare;

	// 신선불카드 지불재거래
	if ( bWriteType == REPURCHASE &&
		 pstTransInfo->bCardType == TRANS_CARDTYPE_SC_PREPAY )
	{
		sResult = SCPrepayRepurchase( dwTransAmt, &stPSAMResult );
		// SAM 거래 내역 저장
		SCSavePSAMResult( sResult, &stPSAMResult, pstTransInfo );
		if ( ( sResult != SUCCESS ) && ( sResult !=ERR_SAM_IF_PSAM_NO_SIGN3 ) )
		{
			DebugOut( "SCPrepayRepurchase 오류 : %x\n", sResult );
			return ErrRet( sResult );
		}
		else 
			sResult = SUCCESS;			
	}
	// 신후불카드 지불재거래( 취소 -> 거래 )
	else if ( bWriteType == POSTPAY_CANCEL ||
			 ( bWriteType == REPURCHASE &&
			  pstTransInfo->bCardType == TRANS_CARDTYPE_SC_POSTPAY ) )
	{
		sResult = SCPostpayCancel( &stPSAMResult );
		if ( sResult != SUCCESS )
		{
			DebugOut( "SCPostpayCancel 오류 : %x\n", sResult );
			return ErrRet( sResult );
		}
		sResult = SCPostpayPurchase( pstTransInfo->boolIsChangeMonth,
			dwTransAmt, &stPSAMResult );
		// SAM 거래 내역 저장
		SCSavePSAMResult( sResult, &stPSAMResult, pstTransInfo );
		if ( sResult != SUCCESS )
		{
			DebugOut( "SCPostpayPurchase 오류 : %x\n", sResult );
			return ErrRet( sResult );
		}
	}
	// 신선불카드 취소 후 거래
	else if ( bWriteType == PREPAY_CANCEL )
	{
		sResult = SCPrepayCancel( dwTransAmt, &stPSAMResult );
		// SAM 거래 내역 저장
		SCSavePSAMResult( sResult, &stPSAMResult, pstTransInfo );
		if ( sResult != SUCCESS )
		{
			DebugOut( "SCPrepayCancel 오류 : %x\n", sResult );
			return ErrRet( sResult );
		}
		sResult = SCPrepayPurchase( dwTransAmt, &stPSAMResult );
		// SAM 거래 내역 저장
		SCSavePSAMResult( sResult, &stPSAMResult, pstTransInfo );
		if ( sResult != SUCCESS )
		{
			DebugOut( "SCPrepayPurchase 오류 : %x\n", sResult );
			return ErrRet( sResult );
		}
	}

	// 시티투어버스의 경우 환승정보를 WRITE하지 않음
	if ( IsCityTourBus( gstVehicleParm.wTranspMethodCode ) == FALSE )
	{
		// 환승정보 구조체 분해
		SCDeComTransInfo( pstTransInfo, &stPurseInfo, &stCommonXferData );

		// 환승정보 APPEND
		sResult = SCAppendXferInfo( TRANS_NORMAL, &stPurseInfo,
			&stCommonXferData );
		if ( sResult != SUCCESS )
		{
			printf( "[SCRePayment] SCAppendXferInfo() 실패\n" );
			return ErrRet( sResult );
		}
	}

	return ( sResult );
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SCDeComTransInfo                                         *
*                                                                              *
*  DESCRIPTION:       카드정보구조체를 신카드 구조체로 재조립                  *
*                                                                              *
*  INPUT PARAMETERS:  TRANS_INFO *pstTransInfo                                 *
*                     SC_EF_PURSE_INFO *pstPurseInfo                           *
*                     COMMON_XFER_DATA *pstTrans                               *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS                                                  *
*                                                                              *
*  Author : MeeHyang Son													   *
*                                                                              *
*  DATE   : 2005-08-24 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static void SCDeComTransInfo( TRANS_INFO *pstTransInfo,
	SC_EF_PURSE_INFO *pstPurseInfo,
	COMMON_XFER_DATA *pstCommonXferData )
{
	// ( 1 ) 전자지갑 정보로 파일 변환
	pstPurseInfo->bEpurseIssuerID = pstTransInfo->bSCEpurseIssuerID;
	memcpy( pstPurseInfo->abEpurseID, pstTransInfo->abSCEpurseID,
		sizeof( pstPurseInfo->abEpurseID ) );

	// ( 2 ) 환승 정보 파일 변환
	memcpy( pstCommonXferData, &pstTransInfo->stNewXferInfo,
			sizeof( COMMON_XFER_DATA ) );

	// 현재 환승정보에서 사용하지 않는 바이트임
	pstCommonXferData->wTotalAccEntCnt = 0;
	pstCommonXferData->dwTotalAccUseAmt = 0;

}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SCSavePSAMResult                                         *
*                                                                              *
*  DESCRIPTION:       신카드 PSAM응답을 카드정보구조체에 저장                  *
*                                                                              *
*  INPUT PARAMETERS:  PSAM_RES_TRANS *pstPSAMResult                            *
*                     TRANS_INFO *pstTransInfo                                 *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS                                                  *
*                                                                              *
*  Author : MeeHyang Son													   *
*                                                                              *
*  DATE   : 2005-08-24 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static void SCSavePSAMResult( short sResult,
	PSAM_RES_TRANS *pstPSAMResult,
	TRANS_INFO *pstTransInfo )
{

	if ( sResult == SUCCESS )
	{
		pstTransInfo->bSCAlgoriType			= pstPSAMResult->bAlgID;
		pstTransInfo->bSCTransKeyVer		= pstPSAMResult->bTransKeyVer;
		pstTransInfo->bSCEpurseIssuerID		= pstPSAMResult->bCenterID;
		memcpy( pstTransInfo->abSCEpurseID, pstPSAMResult->abEpurseID,
			sizeof( pstTransInfo->abSCEpurseID ) );
	}
	pstTransInfo->bSCTransType 			= pstPSAMResult->bTransType;
	pstTransInfo->dwSCTransCnt 			= pstPSAMResult->dwTransCnt;

	memcpy( pstTransInfo->abPSAMID, pstPSAMResult->abSAMID,
		sizeof( pstTransInfo->abPSAMID ) );
	pstTransInfo->dwPSAMTransCnt 		= pstPSAMResult->dwSAMTransCnt;

	pstTransInfo->dwPSAMTotalTransCnt 	= pstPSAMResult->dwSAMTotalTransCnt;
	pstTransInfo->wPSAMIndvdTransCnt 	= pstPSAMResult->wSAMIndvdTransCnt;
	pstTransInfo->dwPSAMAccTransAmt 	= pstPSAMResult->dwSAMAccTransAmt;
	memcpy( pstTransInfo->abPSAMSign, pstPSAMResult->abSAMSign,
		sizeof( pstTransInfo->abPSAMSign ) );
}

