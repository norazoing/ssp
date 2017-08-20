#include "../system/bus_type.h"
#include "card_proc.h"
#include "card_proc_util.h"
#include "blpl_proc.h"
#include "trans_proc.h"

// BL / PL 체크시 발생 오류 ////////////////////////////////////////////////////
#define CHECK_BL					0		// BL체크 구후불카드
#define CHECK_PL					1		// PL체크 구후불카드
#define CHECK_PL_READ_SECTOR5_ERR	2		// PL체크 구후불카드임에도
											// SECTOR5 READ에 실패하는 경우
#define CHECK_PL_ALIAS_ERR			3		// PL체크 구후불카드이지만
											// alias번호가 범위를 벗어나는 경우
#define CHECK_PL_CICC_ERR			4		// CICC코드 ISAM체크 오류

static void MifPostpaySetBasicCardInfo( TRANS_INFO *pstTransInfo,
	MIF_POSTPAY_SECTOR0 *pstMifPostpaySector0,
	MIF_POSTPAY_SECTOR12 *pstMifPostpaySector12 );
static short MifPostpayCheckValidCard( TRANS_INFO *pstTransInfo,
	byte bIssuerCode, byte *abExpiryDate );
static short MifPostpayCheckBLPL( TRANS_INFO *pstTransInfo, byte bIssuerCode );
static byte MifPostpayGetBLPLType( dword dwChipSerialNo, byte bIssuerCode,
	dword *pdwAliasNo, byte *pbSavedPLValue );
static void MifPostpaySetCardInfoStruct( TRANS_INFO *pstTransInfo,
	MIF_POSTPAY_OLD_XFER_DATA *pstMifPostpayOldXferData,
	COMMON_XFER_DATA *pstCommonXferData );
static short MifPostpayIsBLCard( byte *abCardNo, byte bIssuerCode,
	byte *pbChkBLResult );
static void MifPostpayBuildOldXferInfo( TRANS_INFO *pstTransInfo );


/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       MifPostpayRead                                           *
*                                                                              *
*  DESCRIPTION:       구후불카드를 READ하고 유효성체크 및 PL/BL체크를 수행한 후*
*                     카드정보구조체를 조립한다.                               *
*                                                                              *
*  INPUT PARAMETERS:  pstTransInfo - 카드정보구조체의 포인터                   *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공                                      *
*                     ERR_CARD_PROC_RETAG_CARD - 카드를 다시 대주십시오.       *
*                     ERR_CARD_PROC_CANNOT_USE - 사용할 수 없는 카드입니다.    *
*                     ERR_CARD_PROC_EXPIRED_CARD - 카드 유효기간이 지났습니다. *
*                     ERR_CARD_PROC_NOT_APPROV - 미승인 카드입니다.            *
*                     ERR_CARD_PROC_LOG - 에러로그에 존재하는 카드의 경우      *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-23                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
short MifPostpayRead( TRANS_INFO *pstTransInfo )
{
	short sResult = SUCCESS;

	MIF_POSTPAY_SECTOR0 stMifPostpaySector0;
	MIF_POSTPAY_SECTOR12 stMifPostpaySector12;
	MIF_POSTPAY_OLD_XFER_DATA stMifPostpayOldXferData;
	COMMON_XFER_DATA stCommonXferData;
byte abTempCardNo[21] = {0, };
	// 구후불카드 기본정보READ 및 SAM체크 //////////////////////////////////////
	sResult = MifPostpayReadBasicInfo( pstTransInfo->dwChipSerialNo,
		&stMifPostpaySector0, &stMifPostpaySector12 );
	if ( sResult != SUCCESS )
	{
		printf( "[MifPostpayRead] 구후불카드 기본정보READ 및 SAM체크 오류\n" );
		return ERR_CARD_PROC_RETAG_CARD;		// '카드를 다시 대주세요'
	}

	// 구후불카드 기본정보 설정 ////////////////////////////////////////////////
	MifPostpaySetBasicCardInfo( pstTransInfo, &stMifPostpaySector0,
		&stMifPostpaySector12 );
memcpy(abTempCardNo, pstTransInfo->abCardNo, 20);
LogMain("구후 : %s\n", abTempCardNo);
	// 테스트카드의 경우 사용불가 처리 /////////////////////////////////////////
	if ( stMifPostpaySector0.bCardType == '9' )
	{
		// 테스트카드의 경우 사용불가 처리
#ifndef TEST_NOT_CHECK_TEST_CARD
		printf( "[MifPostpayRead] 테스트 카드의 경우 사용불가 처리\n" );
		return ERR_CARD_PROC_CANNOT_USE;		// '사용할 수 없는 카드입니다'
#endif
	}

	// 구후불카드 유효성 체크 //////////////////////////////////////////////////
	if ( IsAllZero( stMifPostpaySector12.abMifPostpayBlockBinary48,
			sizeof( stMifPostpaySector12.abMifPostpayBlockBinary48 ) ) ||
		 IsAllFF( stMifPostpaySector12.abMifPostpayBlockBinary48,
		 	sizeof( stMifPostpaySector12.abMifPostpayBlockBinary48 ) ) )
	{
		printf( "[MifPostpayRead] BLOCK48 내용 오류 (모두 0x00 or 0xFF)\n" );
		return ERR_CARD_PROC_CANNOT_USE;		// '사용할 수 없는 카드입니다'
	}

	sResult = MifPostpayCheckValidCard( pstTransInfo,
		stMifPostpaySector0.bIssuerCode,
		stMifPostpaySector12.abExpiryDate );
	if ( sResult != SUCCESS )
	{
		switch ( sResult )
		{
			case ERR_CARD_PROC_MIF_POSTPAY_INVALID_CARD_NO:
			case ERR_CARD_PROC_MIF_POSTPAY_INVALID_ISSUER_VALID_PERIOD:
				return ERR_CARD_PROC_CANNOT_USE;
												// '사용할 수 없는 카드입니다'
			case ERR_CARD_PROC_MIF_POSTPAY_EXPIRE:
				return ERR_CARD_PROC_EXPIRED_CARD;
												// '카드 유효기간이 지났습니다'
			case ERR_CARD_PROC_MIF_POSTPAY_INVALID_ISSUER:
				return ERR_CARD_PROC_NOT_APPROV;
												// '미승인 카드입니다'
			default:
				return ERR_CARD_PROC_RETAG_CARD;
												// '카드를 다시 대주세요'
		}
	}

#ifndef TEST_NOT_CHECK_BLPL
	// 구후불카드 BL/PL 체크 ///////////////////////////////////////////////////
	sResult = MifPostpayCheckBLPL( pstTransInfo,
		stMifPostpaySector0.bIssuerCode );
	if ( sResult != SUCCESS )
	{
		printf( "[MifPostpayRead] 구후불카드 BL/PL 체크 오류\n" );
		switch ( sResult )
		{
			case ERR_CARD_PROC_NOT_APPROV:
			case ERR_CARD_PROC_CANNOT_USE:
			case ERR_CARD_PROC_RETAG_CARD:
				return sResult;
			default:
				return ERR_CARD_PROC_RETAG_CARD;
												// '카드를 다시 대주세요'
		}
	}
#endif

	// 오류LOG리스트에 존재하는 카드인지 확인 //////////////////////////////////
	// (하차단말기의 경우 BL/PL 체크를 통해 승차단말기의 오류내역을 가져오므로,
	//  이 작업은 반드시 BL/PL 체크 다음에 존재하여야 함)
	sResult = SearchCardErrLog( pstTransInfo );
	if ( sResult == SUCCESS )
	{
		DebugOut( "[MifPostpayRead] 오류LOG리스트에 존재하는 카드\n" );
		return ERR_CARD_PROC_LOG;
	}

	// 구후불카드 환승정보 READ ////////////////////////////////////////////////
	sResult =  MifPostpayReadXferInfo( pstTransInfo->dwChipSerialNo,
		stMifPostpaySector0.bIssuerCode, &stCommonXferData );
	if ( sResult != SUCCESS )
	{
		printf( "[MifPostpayRead] 구후불카드 환승정보 READ 오류\n" );
		return ERR_CARD_PROC_RETAG_CARD;		// '카드를 다시 대주세요'
	}

	// 구후불카드 구환승정보 READ //////////////////////////////////////////////
	sResult =  MifPostpayReadOldXferInfo( pstTransInfo->dwChipSerialNo,
		&stMifPostpayOldXferData );
	if ( sResult != SUCCESS )
	{
		printf( "[MifPostpayRead] 구후불카드 구환승정보 READ 오류\n" );
		return ERR_CARD_PROC_RETAG_CARD;		// '카드를 다시 대주세요'
	}

	// 카드정보구조체 조립 /////////////////////////////////////////////////////
	MifPostpaySetCardInfoStruct( pstTransInfo, &stMifPostpayOldXferData,
		&stCommonXferData );

	return SUCCESS;
}

static void MifPostpaySetBasicCardInfo( TRANS_INFO *pstTransInfo,
	MIF_POSTPAY_SECTOR0 *pstMifPostpaySector0,
	MIF_POSTPAY_SECTOR12 *pstMifPostpaySector12 )
{
	// 카드종류를 구후불카드로 설정 ////////////////////////////////////////////
	pstTransInfo->bCardType = TRANS_CARDTYPE_MIF_POSTPAY;

	// 카드번호 설정 ///////////////////////////////////////////////////////////
	memset( pstTransInfo->abCardNo, 'F', sizeof( pstTransInfo->abCardNo ) );
	// 삼성Amex카드의 경우 카드번호가 15자리
	if ( pstMifPostpaySector0->bIssuerCode == ISS_SS &&
		pstMifPostpaySector12->abCardNo[0] == '3' )
	{
		memcpy( pstTransInfo->abCardNo, pstMifPostpaySector12->abCardNo, 15 );
	}
	// 그외 카드의 경우 카드번호가 16자리
	else
	{
		memcpy( pstTransInfo->abCardNo, pstMifPostpaySector12->abCardNo, 16 );
	}

	// 사용자유형 설정 /////////////////////////////////////////////////////////
	pstTransInfo->bPLUserType = USERTYPE_ADULT;
	pstTransInfo->bCardUserType = USERTYPE_ADULT;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       MifPostpayCheckValidCard                                 *
*                                                                              *
*  DESCRIPTION:       구후불카드 유효성 체크를 수행한다.                       *
*                     (카드번호 체크, 유효기간 체크, prefix 체크)              *
*                                                                              *
*  INPUT PARAMETERS:  pstTransInfo - 카드정보구조체의 포인터                   *
*                     bIssuerCode - 발행사코드                                 *
*                     abExpiryDate - 유효기간 (YYYYMM)                         *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공                                      *
*                     ERR_CARD_PROC_MIF_POSTPAY_INVALID_CARD_NO - 카드번호 오류*
*                     ERR_CARD_PROC_MIF_POSTPAY_EXPIRE - 유효기간 오류         *
*                     ERR_CARD_PROC_MIF_POSTPAY_INVALID_ISSUER - prefix 오류   *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-23                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static short MifPostpayCheckValidCard( TRANS_INFO *pstTransInfo,
	byte bIssuerCode, byte *abExpiryDate )
{
	if ( !IsValidPostpayCardNo( pstTransInfo->abCardNo, bIssuerCode ) )
	{
		printf( "[MifPostpayCheckValidCard] 카드번호 오류\n" );
		return ERR_CARD_PROC_MIF_POSTPAY_INVALID_CARD_NO;
	}

#ifndef TEST_NOT_CHECK_EXPIRY_DATE
	if ( !IsValidExpiryDate( abExpiryDate,
		pstTransInfo->stNewXferInfo.tEntExtDtime ) )
	{
		printf( "[MifPostpayCheckValidCard] 유효기간 오류\n" );
		return ERR_CARD_PROC_MIF_POSTPAY_EXPIRE;
	}
#endif

	if ( !IsValidPostpayIssuer( pstTransInfo->abCardNo ) )
	{
		printf( "[MifPostpayCheckValidCard] 후불발행사 체크 오류\n" );
		return ERR_CARD_PROC_MIF_POSTPAY_INVALID_ISSUER;
	}

	if ( !IsValidIssuerValidPeriod( pstTransInfo->abCardNo,
		abExpiryDate ) )
	{
		printf( "[MifPostpayCheckValidCard] 후불발행사유효기간 체크 오류\n" );
		return ERR_CARD_PROC_MIF_POSTPAY_INVALID_ISSUER_VALID_PERIOD;
	}

	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       MifPostpayCheckBLPL                                      *
*                                                                              *
*  DESCRIPTION:       구후불카드 BL / PL 체크를 수행한다.                      *
*                                                                              *
*  INPUT PARAMETERS:  pstTransInfo - 카드정보구조체의 포인터                   *
*                     bIssuerCode - 발행사코드                                 *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공                                      *
*                     ERR_CARD_PROC_CANNOT_USE - BC카드 일부 prefix의 경우     *
*                     ERR_CARD_PROC_NOT_APPROV - NL카드                        *
*                     ERR_CARD_PROC_RETAG_CARD - 기타 이유로 재태그 요청       *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-23                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static short MifPostpayCheckBLPL( TRANS_INFO *pstTransInfo, byte bIssuerCode )
{
	short sResult = SUCCESS;
	byte bBLPLType = 0;
	byte bResult = 0;
	byte bSavedPLValue = 0;
	dword dwTempAliasNo = 0;

	bBLPLType = MifPostpayGetBLPLType( pstTransInfo->dwChipSerialNo,
		bIssuerCode, &dwTempAliasNo, &bSavedPLValue );
	pstTransInfo->dwAliasNo = dwTempAliasNo;

	switch ( bBLPLType )
	{
		case CHECK_BL:
		case CHECK_PL_ALIAS_ERR:
			DebugOut( "[MifPostpayCheckBLPL] BL체크카드\n" );
			if ( bIssuerCode == ISS_LG )
			{
				printf( "[MifPostpayCheckBLPL] 엘지카드이므로 NL 리턴\n" );
				return ERR_CARD_PROC_NOT_APPROV;
			}
			if ( IsBCCardInvalidBIN( pstTransInfo->abCardNo ) )
			{
				printf( "[MifPostpayCheckBLPL] BC카드 사용불가 " );
				printf( "BIN이므로 '사용할 수 없는 카드입니다' 음성 출력\n" );
				return ERR_CARD_PROC_CANNOT_USE;
			}

			sResult =
				MifPostpayIsBLCard( pstTransInfo->abCardNo, bIssuerCode,
					&bResult );
			if ( sResult != SUCCESS )
			{
				return sResult;
			}

			// BL 체크 결과가 1 이면 '미승인 카드입니다' 리턴
			if ( bResult == 1 )
				return ERR_CARD_PROC_NOT_APPROV;

			// BL 체크 결과가 0, 1이 아니면 '카드를 다시 대주세요' 리턴
			if ( bResult >= 2 )
			{
				printf( "[MifPostpayCheckBLPL] 구후불카드 BL체크 결과가 ");
				printf( "0, 1이 아님\n" );
				return ERR_CARD_PROC_RETAG_CARD;
			}

			break;
		case CHECK_PL:
			DebugOut( "[MifPostpayCheckBLPL] PL체크카드\n" );
			sResult = SearchPLinBus( pstTransInfo->abCardNo, pstTransInfo->dwAliasNo,
				&bResult );
			if ( sResult != SUCCESS )
			{
				// 구후불카드 PL체크 호출 자체가 실패한 경우 카드에 저장되어
				// 있던 이전 성공 PL체크 결과를 사용한다.
				bResult = bSavedPLValue;
			}
			else
			{
				DebugOut( "[MifPostpayCheckBLPL] PL 체크 결과 : %u\n",
					bResult );
				DebugOut( "[MifPostpayCheckBLPL] 이전 PL 결과 : %u\n",
					bSavedPLValue );

				// PL 체크 결과와 BLOCK21에 저장된 이전 PL 결과가 동일 /////////
				if ( bResult == bSavedPLValue )
				{
					DebugOut( "[MifPostpayCheckBLPL] PL 체크 결과와 " );
					DebugOut( "BLOCK21에 저장된 이전 PL 결과가 동일하므로 " );
					DebugOut( "굳이 BLOCK21에 새로운 PL 체크 결과를 " );
					DebugOut( "WRITE하지 않음\n" );
				}
				// PL 체크 결과와 BLOCK21에 저장된 이전 PL 결과가 다름 /////////
				else
				{
					MIF_POSTPAY_BLOCK21 stMifPostpayBlock21;

					DebugOut( "[MifPostpayCheckBLPL] PL 체크 결과와 " );
					DebugOut( "BLOCK21에 저장된 이전 PL 결과가 다르므로 " );
					DebugOut( "BLOCK21에 새로운 PL 체크 결과를 WRITE함\n" );

					// 구후불카드 PL체크에 성공한 경우 그 결과를 BLOCK21에
					// 저장한다.
					memset( &stMifPostpayBlock21, 0,
						sizeof( MIF_POSTPAY_BLOCK21 ) );
					stMifPostpayBlock21.bSavedPLValue = bResult;
					// WRITE 결과는 무시한다. - 성공해도 그만, 실패해도 그만
					MifPostpayWriteBlock21( pstTransInfo->dwChipSerialNo,
						&stMifPostpayBlock21 );
				}
			}

			// PL 체크 결과가 0 이면 '미승인 카드입니다' 리턴
			if ( bResult == 0 )
				return ERR_CARD_PROC_NOT_APPROV;

			// PL 체크 결과가 0, 1이 아니면 '카드를 다시 대주세요' 리턴
			if ( bResult >= 2 )
			{
				printf( "[MifPostpayCheckBLPL] 구후불카드 PL체크 결과가 " );
				printf( "0, 1이 아님\n" );
				return ERR_CARD_PROC_RETAG_CARD;
			}

			break;
		case CHECK_PL_READ_SECTOR5_ERR:
			printf( "[MifPostpayCheckBLPL] PL SECTOR5 READ 오류\n" );
			return ERR_CARD_PROC_RETAG_CARD;
			break;
		case CHECK_PL_CICC_ERR:
			printf( "[MifPostpayCheckBLPL] PL CICC 코드 오류\n" );
			return ERR_CARD_PROC_RETAG_CARD;
			break;
	}

	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       MifPostpayGetBLPLType                                    *
*                                                                              *
*  DESCRIPTION:       구후불카드 BL을 체크할지 PL을 체크할지 판단하여 리턴한다.*
*                                                                              *
*  INPUT PARAMETERS:  dwChipSerialNo - 칩시리얼번호                            *
*                     pdwAliasNo - alias번호를 리턴하기 위한 포인터            *
*                     bIssuerCode - 발행사코드                                 *
*                     pbSavedPLValue - 카드에 저장된 PL체크 결과               *
*                                                                              *
*  RETURN/EXIT VALUE: CHECK_BL - BL 체크 카드                                  *
*                     CHECK_PL - PL 체크 카드                                  *
*                     CHECK_PL_READ_SECTOR5_ERR - PL 체크 카드임에도 불구하고  *
*                         SECTOR5 READ에 실패하는 경우                         *
*                     CHECK_PL_ALIAS_ERR - PL 체크 카드이면서 alias 범위 오류  *
*                     CHECK_PL_CICC_ERR - PL 체크 카드이면서 CICC 체크 오류    *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-23                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static byte MifPostpayGetBLPLType( dword dwChipSerialNo, byte bIssuerCode,
	dword *pdwAliasNo, byte *pbSavedPLValue )
{
	short sResult = SUCCESS;
	MIF_POSTPAY_SECTOR5 stMifPostpaySector5;

	if ( bIssuerCode == ISS_KM )
	{
		memset( ( byte * )pdwAliasNo, 0, sizeof( dword ) );
		return CHECK_BL;
	}

	sResult = MifPostpayReadSector5( dwChipSerialNo, &stMifPostpaySector5 );
	if ( sResult != SUCCESS )
	{
		return CHECK_PL_READ_SECTOR5_ERR;
	}

#ifdef TEST_MIF_POSTPAY_BLOCK20_ALL_ZERO
	memset( stMifPostpaySector5.abMifPostpayBlockBinary20, 0,
		sizeof( stMifPostpaySector5.abMifPostpayBlockBinary20 ) );
#endif

	// SECTOR5가 인텍키로 정상적으로 발급되지 않은 경우
	// 0xffffffffffff 등의 키로 READ가 가능한 경우가 있을 수 있음.
	// 하지만 이 경우 SECTOR5내의 각 BLOCK의 값이
	// 모두 0x00 이거나 0xff 일 것으로 추정되므로 이에 대한 처리를 수행함.
	if ( IsAllZero( stMifPostpaySector5.abMifPostpayBlockBinary20,
			sizeof( stMifPostpaySector5.abMifPostpayBlockBinary20 ) ) ||
		 IsAllFF( stMifPostpaySector5.abMifPostpayBlockBinary20,
		 	sizeof( stMifPostpaySector5.abMifPostpayBlockBinary20 ) ) )
	{
		return CHECK_BL;
	}

	sResult = ISAMSetCardCSN( dwChipSerialNo );
	if ( sResult != SUCCESS )
	{
		return CHECK_PL_CICC_ERR;
	}

	sResult = ISAMCheckCICC( stMifPostpaySector5.abMifPostpayBlockBinary20 );
	if ( sResult != SUCCESS )
	{
		return CHECK_PL_CICC_ERR;
	}

#ifdef TEST_MIF_POSTPAY_INVALID_ALIAS
	return CHECK_PL_ALIAS_ERR;
#endif
	// ALIAS번호가 구후불카드 범위( 30000001 ~ 90000000 )외인 경우 오류 처리
	if ( stMifPostpaySector5.dwAliasNo < 30000001 ||
		 stMifPostpaySector5.dwAliasNo > 90000000 )
	{
		return CHECK_PL_ALIAS_ERR;
	}

	memcpy( pdwAliasNo, &stMifPostpaySector5.dwAliasNo, sizeof( dword ) );
	*pbSavedPLValue = stMifPostpaySector5.stMifPostpayBlock21.bSavedPLValue;

	return CHECK_PL;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       MifPostpaySetCardInfoStruct                              *
*                                                                              *
*  DESCRIPTION:       카드로부터 읽은 정보를 이용하여 카드정보 구조체를        *
*                     조립한다.                                                *
*                                                                              *
*  INPUT PARAMETERS:  pstTransInfo - 카드정보 구조체 포인터                    *
*                     pstMifPostpayOldXferData - 구후불카드 구환승정보         *
*                         READ 결과                                            *
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
static void MifPostpaySetCardInfoStruct( TRANS_INFO *pstTransInfo,
	MIF_POSTPAY_OLD_XFER_DATA *pstMifPostpayOldXferData,
	COMMON_XFER_DATA *pstCommonXferData )
{
	// 구카드 단말기그룹코드 ( 구환승영역 기록 )
	pstTransInfo->bMifTermGroupCode = pstMifPostpayOldXferData->bTranspTypeCode;

	// 구카드 이용시간 ( 구환승영역기록 )
	pstTransInfo->tMifEntExtDtime = pstMifPostpayOldXferData->tUseDtime;

	// 이전환승정보
	memcpy( &pstTransInfo->stPrevXferInfo, pstCommonXferData,
		sizeof( COMMON_XFER_DATA ) );

	// 구후불카드 구환승정보
	memcpy( &pstTransInfo->stOldXferInfo, pstMifPostpayOldXferData,
		sizeof( MIF_POSTPAY_OLD_XFER_DATA ) );
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       MifPostpayIsBLCard                                       *
*                                                                              *
*  DESCRIPTION:       BL 체크를 수행하고 결과를 리턴한다.                      *
*                                                                              *
*  INPUT PARAMETERS:  abCardNo - 카드번호                                      *
*                     bIssuerCode - 발행사코드                                 *
*                     pbChkBLResult - BL체크 결과를 가져오기 위한 포인터       *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공                                      *
*                     NOT SUCCESS - SearchBL() 함수가 리턴하는 오류            *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-23                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static short MifPostpayIsBLCard( byte *abCardNo, byte bIssuerCode,
	byte *pbChkBLResult )
{
	short sResult = SUCCESS;
	dword dwCardNum = 0;
	byte abPrefix[6] = {0, };
	byte abTempBuf[9] = {0, };

	memcpy( abPrefix, abCardNo, sizeof( abPrefix ) );

	// 삼성AMEX카드 ////////////////////////////////////////////////////////////
	if ( IsSamsungAmexCard( abCardNo ) )
	{
		dwCardNum = GetDWORDFromASC( &abCardNo[6], 8 );
	}
	// 삼성로컬카드 ////////////////////////////////////////////////////////////
	else if ( abCardNo[0] == '9' &&
		( bIssuerCode == 0x00 || bIssuerCode == 0x02 ||
		  bIssuerCode == ISS_SS ) )
	{
		memcpy( abTempBuf, &abCardNo[6], 7 );
		memcpy( &abTempBuf[7], &abCardNo[14], 2 );
		dwCardNum = GetDWORDFromASC( abTempBuf, 9 );
	}
	// 그외 카드 ///////////////////////////////////////////////////////////////
	else
	{
		dwCardNum = GetDWORDFromASC( &abCardNo[6], 9 );
	}

	sResult = SearchBLinBus( abCardNo, abPrefix, dwCardNum, pbChkBLResult );
	if ( sResult == ERR_BL_MASTER_BL_FILE_NOT_EXIST )
	{
		printf( "[MifPostpayIsBLCard] 후불BL이 미존재\n" );
		return ERR_CARD_PROC_NOT_APPROV;
	}
	if ( sResult != SUCCESS )
	{
		printf( "[MifPostpayIsBLCard] 기타 알 수 없는 이유로 후불BL체크 " );
		printf( "실패\n" );
		return ERR_CARD_PROC_RETAG_CARD;
	}

	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       MifPostpayWrite                                          *
*                                                                              *
*  DESCRIPTION:       생성된 신규환승정보를 카드에 WRITE한다.                  *
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
short MifPostpayWrite( TRANS_INFO *pstTransInfo )
{
	short sResult = SUCCESS;

	pstTransInfo->stNewXferInfo.bMifPostpayRecSeqNo =
		( pstTransInfo->stPrevXferInfo.bMifPostpayRecSeqNo + 1 ) % 256;

	pstTransInfo->stNewXferInfo.bMifPostpayReadXferSectorNo =
		pstTransInfo->stPrevXferInfo.bMifPostpayReadXferSectorNo;

	// 구후불카드 구환승정보 설정 //////////////////////////////////////////////
	MifPostpayBuildOldXferInfo( pstTransInfo );

	// 구후불카드 환승정보WRITE ////////////////////////////////////////////////
	sResult = MifPostpayWriteXferInfo( pstTransInfo->dwChipSerialNo,
		&pstTransInfo->stNewXferInfo );
	if ( sResult != SUCCESS )
	{
		printf( "[MifPostpayWrite] 구후불카드 환승정보WRITE 오류\n" );
		AddCardErrLog( sResult, pstTransInfo );
		return ERR_CARD_PROC_RETAG_CARD;
	}

#ifdef TEST_WRITE_SLEEP
	printf( "TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT\n" );
	sleep( 2 );
#endif

	// 구후불카드 구환승정보WRITE //////////////////////////////////////////////
	sResult = MifPostpayWriteOldXferInfo( pstTransInfo->dwChipSerialNo,
		&pstTransInfo->stOldXferInfo );
	if ( sResult != SUCCESS )
	{
		printf( "[MifPostpayWrite] 구후불카드 구환승정보WRITE 오류\n" );
		AddCardErrLog( sResult, pstTransInfo );
		return ERR_CARD_PROC_RETAG_CARD;
	}

	// 재처리된 카드의 경우 오류LOG로부터 해당 카드정보를 삭제 /////////////////
	if ( pstTransInfo->sErrCode != SUCCESS )
		DeleteCardErrLog( pstTransInfo->abCardNo );

	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       MifPostpayBuildOldXferInfo                               *
*                                                                              *
*  DESCRIPTION:       카드정보 구조체와 전역변수의 내용을 이용하여             *
*                     카드에 WRITE할 구후불카드 구환승영역의 내용을 조립한다.  *
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
static void MifPostpayBuildOldXferInfo( TRANS_INFO *pstTransInfo )
{
	// 2. 이용역코드
	// TODO : 이용역코드 설정 방법 확인
	memset( pstTransInfo->stOldXferInfo.abUseStationID, 0xFF,
		sizeof( pstTransInfo->stOldXferInfo.abUseStationID ) );

	// 3. 승하차구분
	if ( IsEnt( pstTransInfo->stNewXferInfo.bEntExtType ) )
	{
		pstTransInfo->stOldXferInfo.bEntExtType = 0;	// 승차 0, 하차 1
	}
	else
	{
		pstTransInfo->stOldXferInfo.bEntExtType = 1;	// 승차 0, 하차 1
	}
	// 4. 이용수단구분
	pstTransInfo->stOldXferInfo.bTranspTypeCode = 1;	// 철도 0, 버스 1

	// 5. Format
	pstTransInfo->stOldXferInfo.bFormat = 1;

	// 6. 구역
	pstTransInfo->stOldXferInfo.bRegion = 0;

	// 7. 버스로부터환승구분
	if ( pstTransInfo->stNewXferInfo.bAccXferCnt > 0 )
	{
		pstTransInfo->stOldXferInfo.bXfer = 1;
	}
	else
	{
		pstTransInfo->stOldXferInfo.bXfer = 0;
	}

	// 8. 월사용누적금액
	if ( !IsSameMonth( pstTransInfo->stOldXferInfo.tUseDtime,
		pstTransInfo->stNewXferInfo.tEntExtDtime ) )
	{
		pstTransInfo->stOldXferInfo.dwBal = 0;
	}
	pstTransInfo->stOldXferInfo.dwBal += pstTransInfo->stNewXferInfo.dwFare;

	// 9. 사용횟수
	pstTransInfo->stOldXferInfo.wUseCnt =
		( pstTransInfo->stOldXferInfo.wUseCnt + 1 ) % 0xFFFF;

	// 10. 게이트번호
	pstTransInfo->stOldXferInfo.bGateNo = 0;

	// 11. 정산여부
	pstTransInfo->stOldXferInfo.bSettm = 0;

	// 1. 이용시간
	pstTransInfo->stOldXferInfo.tUseDtime =
		pstTransInfo->stNewXferInfo.tEntExtDtime;
}

