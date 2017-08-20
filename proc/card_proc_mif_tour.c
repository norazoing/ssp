#include "../system/bus_type.h"
#include "card_proc.h"
#include "card_proc_util.h"
#include "blpl_proc.h"
#include "trans_proc.h"

static void MifTourSetBasicCardInfo( TRANS_INFO *pstTransInfo,
	MIF_TOUR_SECTOR1 *pstMifTourSector1,
	MIF_TOUR_BLOCK16 *pstMifTourBlock16 );
static short MifTourCheckValidCard( TRANS_INFO *pstTransInfo,
	byte *abIssueDate );

short MifTourRead( TRANS_INFO *pstTransInfo )
{
	short sResult = SUCCESS;
	MIF_TOUR_SECTOR1 stMifTourSector1;
	MIF_TOUR_BLOCK16 stMifTourBlock16;

	// 관광권카드 기본정보READ 및 체크 /////////////////////////////////////////
	sResult = MifTourReadBasicInfo( pstTransInfo->dwChipSerialNo,
		&stMifTourSector1,
		&stMifTourBlock16 );
	if ( sResult != SUCCESS )
	{
		printf( "[MifTourRead] MifTourReadBasicInfo() 실패\n" );
		return ERR_CARD_PROC_RETAG_CARD;
	}

	// 관광권카드 기본정보 설정 ////////////////////////////////////////////////
	MifTourSetBasicCardInfo( pstTransInfo,
		&stMifTourSector1,
		&stMifTourBlock16 );

	// 관광권카드 유효성 체크 //////////////////////////////////////////////////
	sResult = MifTourCheckValidCard( pstTransInfo,
		stMifTourSector1.abIssueDate );
	if ( sResult != SUCCESS )
	{
		return sResult;
	}

	// 오류LOG리스트에 존재하는 카드인지 확인 //////////////////////////////////
	// (하차단말기의 경우 BL/PL 체크를 통해 승차단말기의 오류내역을 가져오므로,
	//  이 작업은 반드시 BL/PL 체크 다음에 존재하여야 함)
	sResult = SearchCardErrLog( pstTransInfo );
	if ( sResult == SUCCESS )
	{
		DebugOut( "[MifTourRead] 오류LOG리스트에 존재하는 카드\n" );
		return ERR_CARD_PROC_LOG;
	}

	// 관광권카드 환승정보 READ ////////////////////////////////////////////////
	sResult =  MifTourReadXferInfo( pstTransInfo->dwChipSerialNo,
		&pstTransInfo->stPrevXferInfo );
	if ( sResult != SUCCESS )
	{
		printf( "[MifTourRead] 관광권카드 환승정보 READ 오류\n" );
		return ERR_CARD_PROC_RETAG_CARD;
	}

	return SUCCESS;
}

static void MifTourSetBasicCardInfo( TRANS_INFO *pstTransInfo,
	MIF_TOUR_SECTOR1 *pstMifTourSector1,
	MIF_TOUR_BLOCK16 *pstMifTourBlock16 )
{
	// 카드번호 설정 ///////////////////////////////////////////////////////////
	memset( pstTransInfo->abCardNo, 'F', sizeof( pstTransInfo->abCardNo ) );
	memcpy( pstTransInfo->abCardNo, pstMifTourSector1->abEpurseID,
		sizeof( pstMifTourSector1->abEpurseID ) );

	// alias번호 설정 //////////////////////////////////////////////////////////
	pstTransInfo->dwAliasNo = pstMifTourSector1->dwCardUserCertiID;

	// 카드에 기록된 사용자 유형 ///////////////////////////////////////////////
	pstTransInfo->bCardUserType = pstMifTourSector1->bUserTypeCode;

	// 카드 유형 ///////////////////////////////////////////////////////////////
	// 거래내역에 기록되는 카드 유형은 아니며, 단지 처리 중 구분을 위하여 설정
	pstTransInfo->bCardType = TRANS_CARDTYPE_MIF_TOUR;

	// 관광권카드 최초 사용 일시 ///////////////////////////////////////////////
	pstTransInfo->tMifTourFirstUseDtime = pstMifTourBlock16->tFirstUseDtime;

	// 관광권카드 만기일 ///////////////////////////////////////////////////////
	memcpy( pstTransInfo->abMifTourExpiryDate, pstMifTourSector1->abExpiryDate,
		sizeof( pstTransInfo->abMifTourExpiryDate ) );

	// 관광권카드 유형 /////////////////////////////////////////////////////////
	pstTransInfo->wMifTourCardType = pstMifTourSector1->wTourCardType;

}

static short MifTourCheckValidCard( TRANS_INFO *pstTransInfo,
	byte *abIssueDate )
{
	bool boolResult = FALSE;
	short sResult = SUCCESS;
	byte bPLValue = 0;
	byte abCardIssuerNo[7] = {0, };

#ifndef TEST_NOT_CHECK_ISSUER
	abCardIssuerNo[0] = '3';
	memcpy( &abCardIssuerNo[1], pstTransInfo->abCardNo, 6 );
	boolResult = IsValidPrepayIssuer( abCardIssuerNo );
	if ( boolResult == FALSE )
	{
		printf( "[MifTourCheckValidCard] 관광권카드 발행사 체크 오류\n" );
		return ERR_CARD_PROC_NOT_APPROV;
	}
#endif

	boolResult = IsValidSCPrepayCardNo( pstTransInfo->abCardNo );
	if ( !boolResult )
	{
		printf( "[MifTourCheckValidCard] 관광권카드 카드번호 오류\n" );
		return ERR_CARD_PROC_NOT_APPROV;
	}

	if ( pstTransInfo->tMifTourFirstUseDtime != 0 &&
		 pstTransInfo->stNewXferInfo.tEntExtDtime <
			pstTransInfo->tMifTourFirstUseDtime )
	{
		printf( "[MifTourCheckValidCard] 관광권카드 최초사용일시 이전 오류\n" );
		return ERR_CARD_PROC_CANNOT_USE;
	}

#ifdef TEST_NOT_CHECK_PL
	pstTransInfo->bPLUserType = pstTransInfo->bCardUserType;
	return SUCCESS;
#endif

	// PL 체크 /////////////////////////////////////////////////////////////////
	sResult = SearchPLinBus( pstTransInfo->abCardNo, pstTransInfo->dwAliasNo,
		&bPLValue );
	if ( sResult == ERR_PL_FILE_OPEN_MASTER_AI )
	{
		printf( "[MifTourCheckValidCard] 신선불PL이 미존재\n" );
		switch ( pstTransInfo->bCardUserType )
		{
			// 카드의 사용자 구분 코드가 어린이인 경우 어린이로 함
			case USERTYPE_CHILD:
				pstTransInfo->bPLUserType = USERTYPE_CHILD;
				break;
			// 카드 사용자 구분 코드가 청소년/학생인 경우는 청소년 요금임
			case USERTYPE_STUDENT:
			case USERTYPE_YOUNG:
				pstTransInfo->bPLUserType = USERTYPE_YOUNG;
				break;
			case USERTYPE_TEST:
				pstTransInfo->bPLUserType = USERTYPE_TEST;
				break;
			default:
				pstTransInfo->bPLUserType = USERTYPE_ADULT;
				break;
		}
		return SUCCESS;
	}
	if ( sResult != SUCCESS )
	{
		printf( "[MifTourCheckValidCard] 기타 알 수 없는 이유로 " );
		printf( "PL 체크 실패\n" );
		return ERR_CARD_PROC_RETAG_CARD;
	}

	if ( bPLValue == 0 )
	{
		printf( "[MifTourCheckValidCard] NL인 카드\n" );
		return ERR_CARD_PROC_NOT_APPROV;
	}
	else if ( bPLValue > 3 )
	{
		printf( "[MifTourCheckValidCard] PL 체크 결과가 3보다 큼\n" );
		return ERR_CARD_PROC_RETAG_CARD;
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
		case USERTYPE_CHILD:
			switch ( bPLValue )
			{
				case 1:		// P/L 비트에서 청소년 요금
					pstTransInfo->bPLUserType = USERTYPE_YOUNG;
					break;
				case 2:		// P/L 비트에서 어린이 요금
					pstTransInfo->bPLUserType = USERTYPE_CHILD;
					break;
				default:	// P/L 비트에서 일반 요금
					pstTransInfo->bPLUserType = USERTYPE_ADULT;
					break;
			}
			break;
		// 카드 사용자 구분 코드가 청소년/학생인 경우는
		// PL체크 결과값이 일반인경우는 일반요금이고 나머지는 청소년 요금임
		case USERTYPE_STUDENT:
		case USERTYPE_YOUNG:
			if ( bPLValue == 3 )
			{
				pstTransInfo->bPLUserType = USERTYPE_ADULT;
			}
			else
			{
				pstTransInfo->bPLUserType = USERTYPE_YOUNG;
			}
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

short MifTourWrite( TRANS_INFO *pstTransInfo )
{
	short sResult = SUCCESS;

	pstTransInfo->stNewXferInfo.bMifPostpayRecSeqNo =
		( pstTransInfo->stPrevXferInfo.bMifPostpayRecSeqNo + 1 ) % 256;

	pstTransInfo->stNewXferInfo.bMifPostpayReadXferSectorNo =
		pstTransInfo->stPrevXferInfo.bMifPostpayReadXferSectorNo;

	// 관광권카드 최초사용정보 WRITE ///////////////////////////////////////////
	if ( pstTransInfo->tMifTourFirstUseDtime == 0 )
	{
		sResult = MifTourWriteFirstUseInfo( pstTransInfo->dwChipSerialNo,
			pstTransInfo->abCardNo, pstTransInfo->stNewXferInfo.tEntExtDtime,
			gpstSharedInfo->abMainTermID );
		if ( sResult != SUCCESS )
		{
			printf( "[MifTourWrite] 관광권카드 최초사용정보 WRITE 오류\n" );
			AddCardErrLog( sResult, pstTransInfo );
			return ERR_CARD_PROC_RETAG_CARD;
		}
	}

	// 관광권카드 환승정보WRITE ////////////////////////////////////////////////
	// 시티투어버스의 경우 환승정보를 WRITE하지 않음
	if ( IsCityTourBus( gstVehicleParm.wTranspMethodCode ) == FALSE )
	{
		sResult = MifTourWriteXferInfo( pstTransInfo->dwChipSerialNo,
			&pstTransInfo->stNewXferInfo );
		if ( sResult != SUCCESS )
		{
			printf( "[MifTourWrite] 관광권카드 환승정보WRITE 오류\n" );
			AddCardErrLog( sResult, pstTransInfo );
			return ERR_CARD_PROC_RETAG_CARD;
		}
	}

	// 재처리된 카드의 경우 오류LOG로부터 해당 카드정보를 삭제 /////////////////
	if ( pstTransInfo->sErrCode != SUCCESS )
	{
		DeleteCardErrLog( pstTransInfo->abCardNo );
	}

	return SUCCESS;
}

