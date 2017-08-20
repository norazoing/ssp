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
*  PROGRAM ID :       write_trans_data.c                                       *
*                                                                              *
*  DESCRIPTION:       거래내역 기록을 위한 함수들을 제공한다.                  *
*                                                                              *
*  ENTRY POINT:       short WriteTransHeader( void );                          *
*                     void WriteTransData( TRANS_INFO *pstTransInfo );         *
*                     short WriteTransDataForCommProcess( byte *abTransTD );   *
*                     void WriteCashTransData( byte bUserType );               *
*                     short WriteTransTail( void );                            *
*                     short UpdateTransDCSRecvDtime( byte *abTransFileName );  *
*                     short RemakeTrans( void );                               *
*                                                                              *
*  INPUT FILES:       control.trn - 운행정보기록파일                           *
*                     YYYYMMDDhhmmss.trn - 승차거래내역파일                    *
*                     YYYYMMDDhhmmss.10 - 하차임시대사파일                     *
*                     YYYYMMDDhhmmss.20 - 하차임시대사파일                     *
*                     YYYYMMDDhhmmss.30 - 하차임시대사파일                     *
*                                                                              *
*  OUTPUT FILES:      control.trn - 운행정보기록파일                           *
*                     YYYYMMDDhhmmss.trn - 승차거래내역파일                    *
*                     YYYYMMDDhhmmss.trn - 하차대사거래내역파일                *
*                     alight_term_td.tmp - 하차거래내역파일                    *
*                     aboard_term_td.bak - 승차백업거래내역파일                *
*                     alight_term_td.bak - 하차백업거래내역파일                *
*                     temptd.dat - 임시대사파일                                *
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

/*******************************************************************************
*  Inclusion of Header Files                                                   *
*******************************************************************************/
#include "../system/bus_type.h"
#include "../system/device_interface.h"
#include "main_process_busTerm.h"
#include "file_mgt.h"
#include "card_proc.h"
#include "card_proc_util.h"
#include "trans_proc.h"

#include "write_trans_data.h"
#include "reconcile_file_mgt.h"

#define LENGTH_OF_TRANS_HEADER			"185"		// 거래내역헤더길이 문자열
#define LENGTH_OF_TRANS_DATA			"202"		// 거래내역데이터길이 문자열
#define LENGTH_OF_TRANS_TAIL			"161"		// 거래내역테일길이 문자열

#define RECORD_TYPE_TRANS_HEADER		'H'			// 거래내역헤더유형 문자
#define RECORD_TYPE_TRANS_DATA			'D'			// 거래내역데이터유형 문자
#define RECORD_TYPE_TRANS_TAIL			'T'			// 거래내역테일유형 문자

#define PENALTY_TYPE_NO_TAG_IN_EXT		"01"		// 페널티유형 - 하차미태그

#define CHECK_SAM_SIGN_FAIL				'1'			// SAM SIGN 체크 실패
#define CHECK_SAM_SIGN_PASS				'2'			// SAM SIGN 미체크

#define SUB_TRANS_DATA_NOT_YET_SEND		'0'			// 하차거래내역 미전송
#define SUB_TRANS_DATA_SEND_COMPELETED	'1'			// 하차거래내역 전송완료

/*******************************************************************************
*  Declaration of Structure                                                    *
*******************************************************************************/
/*
 * 거래내역 헤더 구조체
 */
typedef struct {
	byte	abTHRecLen[3];					// Recode 길이
	byte	bTHRecType;						// Record 구분
	byte	abTranspBizrID[9];				// 교통사업자 ID
	byte	abBizOfficeID[2];				// 영업소 ID
	byte	abRouteID[8];					// 버스 노선 ID
	byte	abTranspMethodCode[3];			// 교통수단코드
	byte	abVehicleID[9];					// 차량 ID
	byte	abTermID[9];					// 단말기 ID
	byte	abDriverID[6];					// 운전자 ID
	byte	abDCSRecvDtime[14];				// 집계시스템 수신일시
	byte	abStartStationID[7];			// 시작정류장 ID
	byte	abStartDtime[14];				// 운행 시작 일시
	byte	abTermUseSeqNo[4];				// 단말기사용집계일련번호
	byte	abFileName[90];					// File Name
	byte	abRecMAC[4];					// Record Mac
	byte	abCRLF[2];						// 개행 문자
}__attribute__( ( packed ) ) TRANS_TH;

/*
 * 거래내역 데이터 구조체
 */
typedef struct {
	byte	abTDRecLen[3];					// TD Record 길이
	byte	bTDRecType;						// TD Record 구분
	byte	bCardType;						// 카드유형
	byte	bEntExtType;					// 승하차유형
	byte	abCardNo[10];					// 카드번호/일회권ID
	dword	dwAliasNo;						// alias번호
	byte	abEntExtDtime[7];				// 승하차일시
	byte	bUserType;						// 사용자유형
	byte	abDisExtraTypeID1[6];			// 승객1 할인할증유형ID
	word	wUserCnt1;						// 승객수1
	byte	abDisExtraTypeID2[6];			// 승객2 할인할증유형ID
	word	wUserCnt2;						// 승객수2
	byte	abDisExtraTypeID3[6];			// 승객3 할인할증유형ID
	word	wUserCnt3;						// 승객수3
	byte	abPenaltyType[2];				// 패널티유형
	dword	dwPenaltyFare;					// 페널티요금
	byte	bSCAlgoriType;					// 알고리즘유형
	byte	bSCTransType;					// 거래유형
	byte	bSCTransKeyVer;					// 개별거래수집키버전
	byte	bSCEpurseIssuerID;				// 전자화폐사ID
	byte	abSCEpurseID[8];				// 전자지갑ID
	dword	dwSCTransCnt;					// 카드거래건수
	dword	dwBal;							// 카드잔액
	dword	dwFare;							// 요금
	byte	abPSAMID[8];					// SAM ID
	dword	dwPSAMTransCnt;					// SAM거래카운터
	dword	dwPSAMTotalTransCnt;			// SAM총액거래수집카운터
	word	wPSAMIndvdTransCnt;				// SAM개별거래수집건수
	dword	dwPSAMAccTransAmt;				// SAM누적거래총액
	byte	abPSAMSign[4];					// SAM서명
	byte	abXferSeqNo[3];					// 환승일련번호
	dword	dwPrevMaxBaseFare1;				// 승객1 이전거래 지불 최대기본요금
	dword	dwPrevMaxBaseFare2;				// 승객2 이전거래 지불 최대기본요금
	byte	abPrevMaxBaseFare3[4];			// 승객3 이전거래 지불 최대기본요금
	dword	dwTotalBaseFareInXfer;			// 환승내이용수단기본요금의합
	word	wXferCnt;						// 환승누적횟수
	dword	dwDist;							// 사용거리
	dword	dwAccDistInXfer;				// 환승내누적이동거리
	dword	dwAccUseAmtInXfer;				// 환승내누적이용금액
	dword	dwTotalAccAmt;					// 총누적사용금액
	byte	abStationID[4];					// 정류장ID
	dword	dwPrevPenaltyFare;				// 직전페널티요금
	dword	dwPrevEntFare;					// 하차시승차금액
	byte	abPrevStationID[4];				// 직전정류장ID
	byte	abPrevTranspMethodCode[2];		// 직전교통수단유형
	byte	abPrevEntExtDtime[7];			// 직전승하차일시
	dword	dwBalAfterCharge;				// 충전후카드잔액
	dword	dwChargeTransCnt;				// 충전시카드거래건수
	dword	dwChargeAmt;					// 충전금액
	byte	abLSAMID[8];					// 충전기SAM ID
	dword	dwLSAMTransCnt;					// 충전기SAM거래일련번호
	byte	bChargeTransType;				// 충전거래유형
	byte	abRecMAC[4];					// Record Mac
	byte	abVerifySignVal;				// SIGN값검증결과
	byte	abCRLF[2];						// 개행문자
}__attribute__( ( packed ) ) TRANS_TD;

/*
 * 거래내역 테일 구조체
 */
typedef struct {
	byte	abTTRecLen[3];					// Record 길이
	byte	bTTRecType;						// Record구분
	byte	abFileName[90];					// File Name
	byte	abTotalCnt[9];					// 총건수
	byte	abTotalAmt[10];					// 총금액
	byte	abEndDtime[14];					// 운행 종료 일시
	byte	abReturnStationID[7];			// 회차역 ID
	byte	abReturnDtime[14];				// 회차시각
	byte	abEndStationID[7];				// 종료정류장 ID
	byte	abRecMAC[4];					// Record MAC
	byte	abCRLF[2];						// 개행문자
}__attribute__( ( packed ) ) TRANS_TT;

/*******************************************************************************
*  Declaration of Module Functions                                             *
*******************************************************************************/
static short WriteTransDataInMainTerm( TRANS_TD *pstTransTD );
static short WriteTransDataInSubTerm( TRANS_TD *pstTransTD );
static void WriteRemakeTransDataInSubTerm( TRANS_TD *pstTransTD );
static short WriteBackupTransData( TRANS_TD *pstTransTD );
static void CreateTransFileName( byte *abTransFileName,
	time_t *ptStartDriveDtime );
static void CreateEndedTransFileName( byte *abEndedTransFileName,
	time_t *ptEndDriveDtime );
static short AppendTransFileToReconcileFile( time_t tNowDtime );
static short CopyTransHeaderAndTransData( byte *abSrcTransFileName,
	byte *abDesTransFileName, dword *pdwTotalCnt, dword *pdwTotalAmt );
static short RemakeTransData( byte *abMainTransFileName,
	byte *abSubTempRemakeFileName, byte *abTempRemakeFileName,
	dword *pdwTotalCnt, dword *pdwTotalAmt );
static short CopyTransTail( byte *abSrcTransFileName,
	byte *abDesTransFileName, dword dwTotalCnt, dword dwTotalAmt );
static word GetTermAggrSeqNo( void );
static word GetBackupTransDataSeqNo( void );
static word GetCashTransSeqNo( void );
static void SetControlTrans(CONTROL_TRANS *pstControlTrans,
	byte *abTransFileName);
static void SetTransHeader( TRANS_TH *pstTransTH, time_t tStartDriveDtime );
static void SetTransData( TRANS_INFO *pstTransInfo, TRANS_TD *pstTransTD );
static void SetCashTransData( TRANS_TD *pstTransTD, byte bUserType );
static void SetTransTail( TRANS_TT *pstTransTT, dword dwTotalCnt,
	dword dwTotalAmt, time_t tEndDriveDtime );
static short FileRenameTransFile( byte *abOldTransFileName,
	byte *abNewTransFileName );
static short FileReadControlTrans( CONTROL_TRANS *pstControlTrans );
static short FileReadTransHeader( TRANS_TH *pstTransTH, byte *abTransFileName );
static short FileReadTransDataWithFD( FILE *fdFile, TRANS_TD *pstTransTD,
	dword dwIndex );
static short FileReadTransDataWithoutTransHeaderWithFD( FILE *fdFile,
	TRANS_TD *pstTransTD, dword dwIndex );
static short FileReadTransTail( TRANS_TT *pstTransTT, byte *abTransFileName );
static short FileWriteControlTrans( CONTROL_TRANS *pstControlTrans );
static short FileWriteTransHeader( TRANS_TH *pstTransTH,
	byte *abTransFileName );
static short FileAppendTransData( TRANS_TD *pstTransTD, byte *abTransFileName );
static short FileAppendTransDataWithFD( FILE *fdFile, TRANS_TD *pstTransTD );
static short FileAppendSubTransData( byte *abSubTransTD,
	byte *abTransFileName );
static short FileAppendTransTail( TRANS_TT *pstTransTT, byte *abTransFileName );
static short FileUpdateControlTrans( dword dwFare );
static short FileUpdateTransHeaderAndAppendTransTail( TRANS_TT *pstTransTT,
	byte *abTransFileName );
static bool IsExistTransDataWithFD( FILE *fdFile, TRANS_TD *pstTransTD,
	dword dwTotCnt );

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       WriteTransHeader                                         *
*                                                                              *
*  DESCRIPTION:       운행정보파일과 승차거래내역파일을 생성하고 운행정보파일  *
*                     및 승차거래내역파일의 헤더의 내용을 WRITE한다.           *
*                                                                              *
*  INPUT PARAMETERS:  void                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공                                      *
*                     ERR_CARD_PROC_FILE_OPEN_CONTROL_TRANS                    *
*                         - 운행정보파일 OPEN 오류                             *
*                     ERR_CARD_PROC_FILE_WRITE_CONTROL_TRANS                   *
*                         - 운행정보파일 WRITE 오류                            *
*                     ERR_CARD_PROC_FILE_OPEN_MAIN_TRANS                       *
*                         - 승차거래내역파일 OPEN 오류                         *
*                     ERR_CARD_PROC_FILE_WRITE_MAIN_TRANS                      *
*                         - 승차거래내역파일 WRITE 오류                        *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-29                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
short WriteTransHeader( void )
{
	short sResult = SUCCESS;
	time_t tStartDriveDtime = 0;
	byte abTransFileName[19] = {0, };

	CONTROL_TRANS stControlTrans;
	TRANS_TH stTransTH;

	/*
	 * 카드 재태그 오류 횟수 초기화
	 * - 운행시작시 초기화되며, 운행종료시 거래내역파일의 테일에 기록된다.
	 */
	gwRetagCardCnt = 0;

	/*
	 * 운행중 거래내역파일명 설정
	 * - 거래내역파일명은 현재시간을 기반으로 "YYYYMMDDhhmmss.trn"의 형식으로
	 *	설정되며, 동일한 파일이 존재하는 경우 일정 시간을 sleep한 후
	 *	다시 파일명을 생성하도록 한다.
	 */
	CreateTransFileName( abTransFileName, &tStartDriveDtime );

	// 공유메모리에 운행중 거래내역파일명 설정
	memcpy( gpstSharedInfo->abTransFileName, abTransFileName,
		sizeof( gpstSharedInfo->abTransFileName ) );

	/*
	 * 운행정보기록구조체 설정
	 */
	SetControlTrans( &stControlTrans, abTransFileName );

	/*
	 * 거래내역헤더구조체 설정
	 */
	SetTransHeader( &stTransTH, tStartDriveDtime );

	/*
	 * 거래내역파일 접근 세마포어 ALLOC ****************************************
	 */
	SemaAlloc( SEMA_KEY_TRANS );

	/*
	 * 운행정보기록파일 WIRTE
	 */
	sResult = FileWriteControlTrans( &stControlTrans );
	if ( sResult != SUCCESS )
	{
		goto FINALLY;
	}

	/*
	 * 거래내역헤더 WIRTE
	 * - 운행정보기록파일 WRITE가 SUCCESS인 경우에만 수행한다.
	 */
	sResult = FileWriteTransHeader( &stTransTH, abTransFileName );
	if ( sResult != SUCCESS )
	{
		switch ( sResult )
		{
			case ERR_CARD_PROC_FILE_OPEN_TRANS:
				sResult = ERR_CARD_PROC_FILE_OPEN_MAIN_TRANS;
			case ERR_CARD_PROC_FILE_WRITE_TRANS:
				sResult = ERR_CARD_PROC_FILE_WRITE_MAIN_TRANS;
		}
		sResult = ERR_CARD_PROC_FILE_WRITE_MAIN_TRANS;
		goto FINALLY;
	}

	FINALLY:

	/*
	 * 두 파일 중 어느 하나의 WRITE라도 실패했으면 두 파일 모두를 삭제한다.
	 */
	if ( sResult != SUCCESS )
	{
		unlink( CONTROL_TRANS_FILE );
		unlink( abTransFileName );
	}

	/*
	 * 거래내역파일 접근 세마포어 FREE *****************************************
	 */
	SemaFree( SEMA_KEY_TRANS );

	if ( sResult != SUCCESS)
	{
		ErrRet( sResult );
		return sResult;
	}

	/*
	 * 운행시작시간 전역변수 설정
	 * - GPS 로그에 사용하기 위함
	 */
	gtDriveStartDtime = tStartDriveDtime;

	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       WriteTransData                                           *
*                                                                              *
*  DESCRIPTION:       거래내역데이터를 거래내역파일 및 백업거래내역파일에      *
*                     WRITE한다.                                               *
*                                                                              *
*  INPUT PARAMETERS:  pstTransInfo - 카드정보 구조체                           *
*                                                                              *
*  RETURN/EXIT VALUE: void                                                     *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-29                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
void WriteTransData( TRANS_INFO *pstTransInfo )
{
	short sResult = SUCCESS;
	bool boolIsSuccessWrite = TRUE;
	TRANS_TD stTransTD;

	/*
	 * 카드정보구조체의 내용을 이용하여 거래내역구조체 설정
	 */
	SetTransData( pstTransInfo, &stTransTD );

	/*
	 * 거래내역파일 접근 세마포어 ALLOC ****************************************
	 */
	SemaAlloc( SEMA_KEY_TRANS );

	/*
	 * 승차단말기의 경우
	 */
	if ( gboolIsMainTerm )
	{
		/*
		 * 승차거래내역파일에 기록
		 * - 운행정보기록파일의 UPDATE에 오류가 발생하더라도 오류를 발생하지
		 *   않는다.
		 */
		sResult = WriteTransDataInMainTerm( &stTransTD );
		if ( sResult != SUCCESS )
		{
			ErrRet( sResult );
			DebugOut( "[WriteTransData] 승차거래내역파일 기록 오류\n" );
		}
	}
	/*
	 * 하차단말기의 경우
	 */
	else
	{
		/*
		 * 하차거래내역파일에 기록
		 */
		sResult = WriteTransDataInSubTerm( &stTransTD );
		if ( sResult != SUCCESS )
		{
			ErrRet( sResult );
			DebugOut( "[WriteTransData] 하차거래내역파일 기록 오류\n" );
			boolIsSuccessWrite = FALSE;
		}

		/*
		 * 하차대사거래내역파일에 기록
		 */
		WriteRemakeTransDataInSubTerm( &stTransTD );
	}

	/*
	 * 승/하차단말기 공히 백업거래내역파일에 거래내역 기록
	 */
	sResult = WriteBackupTransData( &stTransTD );
	if ( sResult != SUCCESS )
	{
		ErrRet( sResult );
		DebugOut( "[WriteTransData] 백업 거래내역 기록 오류\n" );

		/*
		 * 하차단말기 거래내역 기록 오류이면서 백업 거래내역 기록 오류도
		 * 발생한 경우 -> FND에 119119 출력 후 단말기 정지
		 */
		if ( boolIsSuccessWrite == FALSE )
		{
			DisplayASCInUpFND( FND_ERR_MSG_WRITE_SUB_TRANS_DATA );
			SendKillAllProcSignal();
		}
	}

	/*
	 * 거래내역파일 접근 세마포어 FREE *****************************************
	 */
	SemaFree( SEMA_KEY_TRANS );
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       WriteTransDataForCommProcess                             *
*                                                                              *
*  DESCRIPTION:       승차의 통신프로세스가 하차 단말기로부터 전송된           *
*                     거래내역을 WRITE하기 위해 사용한다.                      *
*                                                                              *
*  INPUT PARAMETERS:  abTransTD - 202바이트로 구성된 거래내역데이터(TD)        *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공                                      *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-09-01                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
short WriteTransDataForCommProcess( byte *abTransTD )
{
	short sResult = SUCCESS;

	/*
	 * 거래내역파일 접근 세마포어 ALLOC ****************************************
	 */
	SemaAlloc( SEMA_KEY_TRANS );

	/*
	 * 승차단말기 거래내역 기록
	 */
	sResult = WriteTransDataInMainTerm( ( TRANS_TD * )abTransTD );
	if ( sResult != SUCCESS )
	{
		ErrRet( sResult );
	}

	/*
	 * 거래내역파일 접근 세마포어 FREE *****************************************
	 */
	SemaFree( SEMA_KEY_TRANS );

	return sResult;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       WriteTransDataInMainTerm                                 *
*                                                                              *
*  DESCRIPTION:       승차거래내역파일에 거래내역데이터를 WRITE하고            *
*                     운행정보파일을 업데이트한다.                             *
*                                                                              *
*  INPUT PARAMETERS:  pstTransTD - 거래내역데이터 구조체의 포인터              *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공                                      *
*                     ERR_CARD_PROC_FILE_OPEN_CONTROL_TRANS                    *
*                         - 운행정보파일 OPEN 실패                             *
*                     ERR_CARD_PROC_FILE_READ_CONTROL_TRANS                    *
*                         - 운행정보파일 READ 실패                             *
*                     ERR_CARD_PROC_FILE_WRITE_CONTROL_TRANS                   *
*                         - 운행정보파일 WRITE 실패                            *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-29                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static short WriteTransDataInMainTerm( TRANS_TD *pstTransTD )
{
	short sResult = SUCCESS;
	byte i = 0;

	/*
	 * 승차단말기 거래내역파일에 거래내역을 기록
	 * - 거래내역기록 중 실패하는 경우 단말기가 FND에 메시지 출력 후 중단된다.
	 */
	for ( i = 0; i < 3; i++ )
	{
		sResult = FileAppendTransData( pstTransTD,
			gpstSharedInfo->abTransFileName );
		if ( sResult == SUCCESS )
			break;
	}
	if ( sResult != SUCCESS )
	{
		switch ( sResult )
		{
			case ERR_CARD_PROC_FILE_OPEN_TRANS:
				sResult = ERR_CARD_PROC_FILE_OPEN_MAIN_TRANS;
				break;
			case ERR_CARD_PROC_FILE_WRITE_TRANS:
				sResult = ERR_CARD_PROC_FILE_WRITE_MAIN_TRANS;
				break;
			default:
				sResult = ERR_CARD_PROC_FILE_WRITE_MAIN_TRANS;
		}
		ErrRet( sResult );

		ctrl_event_info_write( TR_FILE_OPEN_ERROR_EVENT );
		DisplayASCInUpFND( FND_ERR_MSG_WRITE_MAIN_TRANS_DATA );
		Buzzer( 5, 50000 );
		VoiceOut( VOICE_CHECK_TERM );
		ErrRet( ERR_CARD_PROC_FILE_OPEN_MAIN_TRANS );
		SendKillAllProcSignal();
	}

	/*
	 * 운행정보기록파일을 업데이트
	 */
	sResult = FileUpdateControlTrans( pstTransTD->dwFare );

	return sResult;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       WriteTransDataInSubTerm                                  *
*                                                                              *
*  DESCRIPTION:       하차거래내역파일에 거래내역데이터를 WRITE한다.           *
*                                                                              *
*  INPUT PARAMETERS:  pstTransTD - 거래내역데이터 구조체의 포인터              *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공                                      *
*                     ERR_CARD_PROC_FILE_OPEN_SUB_TRANS                        *
*                         - 하차거래내역파일 OPEN 실패                         *
*                     ERR_CARD_PROC_FILE_WRITE_SUB_TRANS                       *
*                         - 하차거래내역파일 WRITE 실패                        *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-29                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static short WriteTransDataInSubTerm( TRANS_TD *pstTransTD )
{
	short sResult = SUCCESS;
	byte i = 0;
	byte abSubTransData[203] = {0, };

	// TD 복사
	memcpy( abSubTransData, ( byte * )pstTransTD, sizeof( TRANS_TD ) );
	// 미전송 상태로 설정
	abSubTransData[202] = SUB_TRANS_DATA_NOT_YET_SEND;

	for ( i = 0; i < 3; i++ )
	{
		sResult = FileAppendSubTransData( abSubTransData, SUB_TERM_TRANS_FILE );
		if ( sResult == SUCCESS )
		{
			break;
		}
	}
	if ( sResult != SUCCESS )
	{
		return sResult;
	}

	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       WriteRemakeTransDataInSubTerm                            *
*                                                                              *
*  DESCRIPTION:       하차대사거래내역파일에 거래내역데이터를 WRITE한다.       *
*                                                                              *
*  INPUT PARAMETERS:  pstTransTD - 거래내역데이터 구조체의 포인터              *
*                                                                              *
*  RETURN/EXIT VALUE: void                                                     *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-29                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static void WriteRemakeTransDataInSubTerm( TRANS_TD *pstTransTD )
{
	short sResult = SUCCESS;
	byte i = 0;

	/*
	 * 하차대사거래내역파일에 거래내역기록
	 * - 거래내역기록 중 실패하는 경우 단말기가 FND에 메시지 출력 후 중단된다.
	 */
	for ( i = 0; i < 3; i++ )
	{
		sResult = FileAppendTransData( pstTransTD,
			gpstSharedInfo->abTransFileName );
		if ( sResult == SUCCESS )
		{
			break;
		}
	}
	if ( sResult != SUCCESS )
	{
		switch ( sResult )
		{
			case ERR_CARD_PROC_FILE_OPEN_TRANS:
				sResult = ERR_CARD_PROC_FILE_OPEN_SUB_REMAKE;
				break;
			case ERR_CARD_PROC_FILE_WRITE_TRANS:
				sResult = ERR_CARD_PROC_FILE_WRITE_SUB_REMAKE;
				break;
			default:
				sResult = ERR_CARD_PROC_FILE_WRITE_SUB_REMAKE;
		}
		ErrRet( sResult );

		ctrl_event_info_write( TR_FILE_OPEN_ERROR_EVENT );
		DisplayASCInUpFND( FND_ERR_MSG_WRITE_SUB_TRANS_DATA );
		Buzzer( 5, 50000 );
		VoiceOut( VOICE_CHECK_TERM );
		SendKillAllProcSignal();
	}
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       WriteBackupTransData                                     *
*                                                                              *
*  DESCRIPTION:       승/하차백업거래내역파일에 거래내역데이터를 WRITE한다.    *
*                                                                              *
*  INPUT PARAMETERS:  pstTransTD - 거래내역데이터 구조체의 포인터              *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공                                      *
*                     ERR_CARD_PROC_FILE_OPEN_BACKUP_TRANS                     *
*                         - 백업거래내역파일 OPEN 오류                         *
*                     ERR_CARD_PROC_FILE_WRITE_BACKUP_TRANS                    *
*                         - 백업거래내역파일 WRITE 오류                        *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-29                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static short WriteBackupTransData( TRANS_TD *pstTransTD )
{
	short sResult = SUCCESS;
	FILE *fdFile = NULL;
	byte i = 0;
	int nResult = 0;
	word wSeqNo = 0;
	byte abFileName[256] = {0, };
	bool boolAlreadyExist = FALSE;

	/*
	 * 승/하차단말기 여부에 따라 백업거래내역파일명을 설정
	 */
	if ( gboolIsMainTerm == TRUE )
	{
		memcpy( abFileName, MAIN_TERM_BACKUP_TRANS_FILE,
			strlen( MAIN_TERM_BACKUP_TRANS_FILE ) );
	}
	else
	{
		memcpy( abFileName, SUB_TERM_BACKUP_TRANS_FILE,
			strlen( SUB_TERM_BACKUP_TRANS_FILE ) );
	}

	/*
	 * 해당 백업거래내역파일의 존재여부에 따라 서로 다른 모드로 파일을 OPEN
	 */
	boolAlreadyExist = IsExistFile( abFileName );

	for ( i = 0; i < 3; i++ )
	{
		// 같은 이름의 파일 존재함
		if ( boolAlreadyExist == TRUE )
		{
			fdFile = fopen( abFileName, "rb+" );
		}
		// 같은 이름의 파일 존재하지 않음
		else
		{
			fdFile = fopen( abFileName, "wb+" );
		}

		if ( fdFile != NULL )
		{
			break;
		}
	}
	if ( fdFile == NULL )
	{
		sResult = ERR_CARD_PROC_FILE_OPEN_BACKUP_TRANS;
		goto FINALLY;
	}

	/*
	 * 백업거래내역순번을 백업거래내역순번파일로부터 가져옴
	 */
	wSeqNo = GetBackupTransDataSeqNo();

	/*
	 * 거래내역을 백업거래내역파일에 WRITE
	 */
	fseek( fdFile, ( wSeqNo * sizeof( TRANS_TD ) ), SEEK_SET );
	for ( i = 0; i < 3; i++ )
	{
		nResult = fwrite( ( byte * )pstTransTD, sizeof( TRANS_TD ), 1, fdFile );
		if ( nResult == 1 )
			break;
	}
	if ( nResult != 1 )
	{
		sResult = ERR_CARD_PROC_FILE_WRITE_BACKUP_TRANS;
		goto FINALLY;
	}

	FINALLY:

	/*
	 * 파일 CLOSE
	 */
	if ( fdFile != NULL )
	{
		fflush( fdFile );
		fclose( fdFile );
	}

	if ( sResult != SUCCESS )
	{
		ctrl_event_info_write( TR_FILE_BACKUP_ERROR_EVENT );
		return sResult;
	}

	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       WriteCashTransData                                       *
*                                                                              *
*  DESCRIPTION:       승차거래내역파일에 입력된 사용자유형에 해당하는          *
*                     현금거래내역데이터를 생성하여 승차거래내역 파일 및       *
*                     승차 백업 거래내역 파일에 WRITE한다.                     *
*                                                                              *
*  INPUT PARAMETERS:  bUserType - 사용자유형                                   *
*                                                                              *
*  RETURN/EXIT VALUE: void                                                     *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-30                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
void WriteCashTransData( byte bUserType )
{
	short sResult = SUCCESS;
	TRANS_TD stTransTD;

	/*
	 * 거래내역파일 접근 세마포어 ALLOC ****************************************
	 */
	SemaAlloc( SEMA_KEY_TRANS );

	/*
	 * 현금거래내역데이터 설정
	 */
	SetCashTransData( &stTransTD, bUserType );

	/*
	 * 승차거래내역파일에 기록
	 */
	sResult = WriteTransDataInMainTerm( &stTransTD );
	if ( sResult != SUCCESS )
	{
		ErrRet( sResult );
	}

	/*
	 * 백업거래내역파일에 기록
	 */
	sResult = WriteBackupTransData( &stTransTD );
	if ( sResult != SUCCESS )
	{
		ErrRet( sResult );
	}

	/*
	 * 거래내역파일 접근 세마포어 FREE *****************************************
	 */
	SemaFree( SEMA_KEY_TRANS );
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       WriteTransTail                                           *
*                                                                              *
*  DESCRIPTION:       승차거래내역파일의 테일(TT)을 생성하고 파일명을          *
*                     변경한 후 집계로의 전송을 위해 레콘사일 리스트에         *
*                     등록한다.                                                *
*                                                                              *
*  INPUT PARAMETERS:  void                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행성공                                       *
*                     ERR_CARD_PROC_FILE_OPEN_CONTROL_TRANS                    *
*                         - 운행정보파일 OPEN 오류                             *
*                     ERR_CARD_PROC_FILE_READ_CONTROL_TRANS                    *
*                         - 운행정보파일 READ 오류                             *
*                     ERR_CARD_PROC_FILE_OPEN_MAIN_TRANS                       *
*                         - 승차거래내역파일 OPEN 오류                         *
*                     ERR_CARD_PROC_FILE_READ_MAIN_TRANS                       *
*                         - 승차거래내역파일 READ 오류                         *
*                     ERR_CARD_PROC_FILE_WRITE_MAIN_TRANS                      *
*                         - 승차거래내역파일 WRITE 오류                        *
*                     ERR_CARD_PROC_FILE_RENAME_MAIN_TRANS                     *
*                         - 승차거래내역파일 RENAME 오류                       *
*                     ERR_CARD_PROC_FILE_WRITE_RECONCILE_LIST                  *
*                         - RECONCILE 파일 WRITE 오류                          *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-29                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
short WriteTransTail( void )
{
	short sResult = SUCCESS;
	time_t tEndDriveDtime = 0;
	dword dwTotalCnt = 0;
	dword dwTotalAmt = 0;
	byte abEndedTransFileName[15] = {0, };
	CONTROL_TRANS stControlTrans;
	TRANS_TT stTransTT;

	/*
	 * 운행종료후 거래내역파일명 설정
	 * - 운행이 종료되면 "YYYYMMDDhhmmss.trn" 형식의 운행중 거래내역파일명을
	 *   "YYYYMMDDhhmmss" 형식의 운행종료후 거래내역파일명으로 RENAME한다.
	 *   현재 시간을 이용하여 운행종료후 거래내역파일명을 설정하고,
	 *   동일한 파일이 존재하는 경우 일정 시간을 sleep한 후 다시 파일명을
	 *   생성하도록 한다.
	 */
	CreateEndedTransFileName( abEndedTransFileName, &tEndDriveDtime );

	// 공유메모리에 운행종료후 거래내역파일명 설정
	memcpy( gpstSharedInfo->abEndedTransFileName, abEndedTransFileName,
		sizeof( gpstSharedInfo->abEndedTransFileName ) );

	/*
	 * 거래내역파일 접근 세마포어 ALLOC ****************************************
	 */
	SemaAlloc( SEMA_KEY_TRANS );

	/*
	 * 운행정보기록파일 READ
	 */
	sResult = FileReadControlTrans( &stControlTrans );
	if ( sResult != SUCCESS )
	{
		goto FINALLY;
	}

	/*
	 * 이번 운행의 거래내역 총건수 및 총금액 설정
	 */
	dwTotalCnt = GetDWORDFromASC( stControlTrans.abTotalCnt, 9 );
	dwTotalAmt = GetDWORDFromASC( stControlTrans.abTotalAmt, 10 );

	/*
	 * 거래내역테일 설정
	 * - 이번 운행의 거래내역 총건수 및 총금액, 그리고 운행종료시간을 이용하여
	 *   거래내역테일 구조체를 설정한다.
	 */
	SetTransTail( &stTransTT, dwTotalCnt, dwTotalAmt, tEndDriveDtime );

	/*
	 * 거래내역헤더 업데이트 및 거래내역테일 APPEND
	 * - 거래내역헤더의 파일명 필드를 업데이트한 후 거래내역테일을 APPEND한다.
	 */
	sResult = FileUpdateTransHeaderAndAppendTransTail( &stTransTT,
		gpstSharedInfo->abTransFileName );
	if ( sResult != SUCCESS )
	{
		switch ( sResult )
		{
			case ERR_CARD_PROC_FILE_OPEN_TRANS:
				sResult = ERR_CARD_PROC_FILE_OPEN_MAIN_TRANS;
				break;
			case ERR_CARD_PROC_FILE_READ_TRANS:
				sResult = ERR_CARD_PROC_FILE_READ_MAIN_TRANS;
				break;
			case ERR_CARD_PROC_FILE_WRITE_TRANS:
				sResult = ERR_CARD_PROC_FILE_WRITE_MAIN_TRANS;
				break;
		}
		sResult = ERR_CARD_PROC_FILE_WRITE_MAIN_TRANS;
		goto FINALLY;
	}

	/*
	 * 거래내역파일명 RENAME
	 * - 선수행한 거래내역헤더 업데이트 및 거래내역테일 APPEND 작업이 성공한
	 *   경우에만 수행한다.
	 * - 운행중 거래내역파일명의 거래내역파일을 운행종료후 거래내역파일명으로
	 *   RENAME한다.
	 */
	sResult = FileRenameTransFile( gpstSharedInfo->abTransFileName,
		gpstSharedInfo->abEndedTransFileName );
	if ( sResult != SUCCESS )
	{
		sResult = ERR_CARD_PROC_FILE_RENAME_MAIN_TRANS;
		goto FINALLY;
	}

	FINALLY:

	/*
	 * 거래내역파일 접근 세마포어 FREE *****************************************
	 */
	SemaFree( SEMA_KEY_TRANS );

	if ( sResult != SUCCESS )
	{
		return ErrRet( sResult );
	}

	/*
	 * RECONCILE 파일에 등록
	 */
	sResult = AppendTransFileToReconcileFile( tEndDriveDtime );
	if ( sResult != SUCCESS )
	{
		return ErrRet ( sResult );
	}

	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       CreateTransFileName                                      *
*                                                                              *
*  DESCRIPTION:       "YYYYMMDDhhmmss.trn" 형식의 운행중 거래내역파일명을      *
*                     생성한다. 현재 시간을 기반으로 생성되며, 동일한 파일명이 *
*                     존재하는 경우에는 일정시간을 sleep한 후 재생성한다.      *
*                                                                              *
*  INPUT PARAMETERS:  abTransFileName - 운행중 거래내역파일명을 리턴하기 위한  *
*                         byte 배열의 포인터                                   *
*                     ptStartDriveDtime - 운행시작시간을 리턴하기 위한 time_t  *
*                         타입의 포인터                                        *
*                                                                              *
*  RETURN/EXIT VALUE: void                                                     *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2006-03-14                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static void CreateTransFileName( byte *abTransFileName,
	time_t *ptStartDriveDtime )
{
	GetRTCTime( ptStartDriveDtime );
	while ( TRUE )
	{
		TimeT2ASCDtime( *ptStartDriveDtime, abTransFileName );
		memcpy( &abTransFileName[14], ".trn", 4 );
		abTransFileName[18] = '\0';			// 종료문자 삽입

		// 같은 이름의 파일 존재함
		if ( IsExistFile( abTransFileName ) )
		{
			ptStartDriveDtime++;
		}
		// 같은 이름의 파일 존재하지 않음
		else
		{
			break;
		}
	}
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       CreateEndedTransFileName                                 *
*                                                                              *
*  DESCRIPTION:       "YYYYMMDDhhmmss" 형식의 운행종료후 거래내역파일명을      *
*                     생성한다. 현재 시간을 기반으로 생성되며, 동일한 파일명이 *
*                     존재하는 경우에는 일정시간을 sleep한 후 재생성한다.      *
*                                                                              *
*  INPUT PARAMETERS:  abEndedTransFileName - 운행종료후 거래내역파일명을       *
*                         리턴하기 위한 byte 배열의 포인터                     *
*                     ptEndDriveDtime - 운행종료시간을 리턴하기 위한 time_t    *
*                         타입의 포인터                                        *
*                                                                              *
*  RETURN/EXIT VALUE: void                                                     *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2006-03-14                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static void CreateEndedTransFileName( byte *abEndedTransFileName,
	time_t *ptEndDriveDtime )
{
	GetRTCTime( ptEndDriveDtime );
	while ( TRUE )
	{
		TimeT2ASCDtime( *ptEndDriveDtime, abEndedTransFileName );
		abEndedTransFileName[14] = '\0';	// 종료문자 삽입

		// 같은 이름의 파일이 존재하면
		if ( IsExistFile( abEndedTransFileName ) )
		{
			ptEndDriveDtime++;
		}
		// 같은 이름의 파일 존재하지 않음
		else
		{
			break;
		}
	}
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       AppendTransFileToReconcileFile                           *
*                                                                              *
*  DESCRIPTION:       "YYYYMMDDhhmmss" 형식의 운행종료후 거래내역파일명을      *
*                     가진 거래내역파일을 RECONCILE 파일에 추가한다.           *
*                                                                              *
*  INPUT PARAMETERS:  tNowDtime - RECONCILE 파일 등록 시간                     *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공                                      *
*                     ERR_CARD_PROC_FILE_WRITE_RECONCILE_LIST                  *
*                         - RECONCILE 파일 WRITE 오류                          *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2006-03-14                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static short AppendTransFileToReconcileFile( time_t tNowDtime )
{
	short sResult = SUCCESS;
	byte i = 0;
	RECONCILE_DATA stReconcileData;

	memset( ( byte * )&stReconcileData, 0, sizeof( RECONCILE_DATA ) );

	// 집계PC로 전송할 파일명 - 종료문자까지 함께 복사
	memcpy( stReconcileData.achFileName, gpstSharedInfo->abEndedTransFileName,
		sizeof( gpstSharedInfo->abEndedTransFileName ) );

	// 전송할 파일상태 - TR파일 송신대기
	stReconcileData.bSendStatus = RECONCILE_SEND_WAIT;

	// 파일목록Write시간
	stReconcileData.tWriteDtime = tNowDtime;

	// RECONCILE 파일에 등록
	for ( i = 0; i < 3; i++ )
	{
		sResult = WriteReconcileFileList( &stReconcileData );
		if ( sResult == SUCCESS )
			break;
	}
	if ( sResult != SUCCESS )
	{
		return ERR_CARD_PROC_FILE_WRITE_RECONCILE_LIST;
	}

	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       UpdateTransDCSRecvDtime                                  *
*                                                                              *
*  DESCRIPTION:       집계통신모듈로부터 호출되어 승차거래내역파일의           *
*                     집계시스템수신일시를 현재시간으로 업데이트 한다.         *
*                                                                              *
*  INPUT PARAMETERS:  void                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공                                      *
*                     ERR_CARD_PROC_FILE_OPEN_MAIN_TRANS                       *
*                         - 승차거래내역파일 OPEN 오류                         *
*                     ERR_CARD_PROC_FILE_READ_MAIN_TRANS                       *
*                         - 승차거래내역파일 READ 오류                         *
*                     ERR_CARD_PROC_FILE_WRITE_MAIN_TRANS                      *
*                         - 승차거래내역파일 WRITE 오류                        *
*                                                                              *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-10                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
short UpdateTransDCSRecvDtime( byte *abTransFileName )
{
	short sResult = SUCCESS;
	int nResult = 0;
	dword dwCRC = 0;
	time_t tNowDtime = 0;
	byte i = 0;
	byte abASCNowDtime[14] = {0, };
	FILE *fdFile = NULL;
	TRANS_TH stTransTH;

	memset( ( byte * )&stTransTH, 0, sizeof( TRANS_TH ) );

	// RTC로부터 현재시간을 가져옴
	GetRTCTime( &tNowDtime );
	TimeT2ASCDtime( tNowDtime, abASCNowDtime );

	/*
	 * 거래내역파일 접근 세마포어 ALLOC ****************************************
	 */
	SemaAlloc( SEMA_KEY_TRANS );

	fdFile = fopen( abTransFileName, "rb+" );
	if ( fdFile == NULL )
	{
		sResult = ERR_CARD_PROC_FILE_OPEN_MAIN_TRANS;
		goto FINALLY;
	}

	fseek( fdFile, 0 ,SEEK_SET );

	nResult = fread( ( byte * )&stTransTH, sizeof( TRANS_TH ), 1, fdFile );
	if ( nResult != 1 )
	{
		sResult = ERR_CARD_PROC_FILE_READ_MAIN_TRANS;
		goto FINALLY;
	}

	// 거래내역헤더의 집계시스템 수신일시
	memcpy( stTransTH.abDCSRecvDtime, abASCNowDtime,
		sizeof( stTransTH.abDCSRecvDtime ) );

	// 거래내역헤더의 Record Mac
	dwCRC = MakeCRC32( ( byte * )&stTransTH, sizeof( TRANS_TH ) - 6 );
	memcpy( stTransTH.abRecMAC, ( byte * )&dwCRC,
		sizeof( stTransTH.abRecMAC ) );

	// 거래내역헤더 WRITE
	fseek( fdFile, 0 ,SEEK_SET );

	for ( i = 0; i < 3; i++ )
	{
		nResult = fwrite( ( byte * )&stTransTH, sizeof( TRANS_TH ), 1, fdFile );
		if ( nResult == 1 )
			break;
	}
	if ( nResult != 1 )
	{
		sResult = ERR_CARD_PROC_FILE_WRITE_MAIN_TRANS;
		goto FINALLY;
	}

	FINALLY:

	if ( fdFile != NULL )
	{
		fflush( fdFile );
		fclose( fdFile );
	}

	/*
	 * 거래내역파일 접근 세마포어 FREE *****************************************
	 */
	SemaFree( SEMA_KEY_TRANS );

	ErrRet( sResult );

	return sResult;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       RemakeTrans                                              *
*                                                                              *
*  DESCRIPTION:       하차로부터 전송된 하차임시대사파일과 승차거래내역파일    *
*                     사이의 대사작업을 수행한다.                              *
*                     먼저 임시대사파일( "temptd.dat" )에 승차거래내역파일의   *
*                     헤더 및 데이터를 복사한 후 각 하차임시대사파일의 TD가    *
*                     승차거래내역파일에 존재하는지의 여부를 체크한 후         *
*                     존재하지 않는다면 임시대사파일에 추가한다.               *
*                     그리고 마지막으로 임시대사파일에 TT를 추가한 후          *
*                     임시대사파일을 승차거래내역파일명으로 rename한다.        *
*                                                                              *
*  INPUT PARAMETERS:  void                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공                                      *
*                     ERR_CARD_PROC_FILE_OPEN_MAIN_TRANS                       *
*                         - 승차거래내역파일 OPEN 오류                         *
*                     ERR_CARD_PROC_FILE_READ_MAIN_TRANS                       *
*                         - 승차거래내역파일 READ 오류                         *
*                     ERR_CARD_PROC_FILE_OPEN_TEMP_REMAKE                      *
*                         - 임시대사파일 OPEN 오류                             *
*                     ERR_CARD_PROC_FILE_WRITE_TEMP_REMAKE                     *
*                         - 임시대사파일 WRITE 오류                            *
*                     ERR_CARD_PROC_FILE_OPEN_SUB_TEMP_REMAKE                  *
*                         - 하차임시대사파일 OPEN 오류                         *
*                     ERR_CARD_PROC_FILE_READ_SUB_TEMP_REMAKE                  *
*                         - 하차임시대사파일 READ 오류                         *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-10                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
short RemakeTrans( void )
{
	short sResult = SUCCESS;
	byte abRemakeTransFileName[3][18];		// 종료문자를 포함하는
											// 각 하차단말기의 대사파일명
											// YYYYMMDDhhmmss.#0
	dword i = 0;
	dword dwMainTDCnt = 0;
	int nResultSize = 0;

	dword dwTotalCnt = 0;
	dword dwTotalAmt = 0;

	// 하차단말기가 존재하지 않으면 SUCCESS 리턴 ///////////////////////////////
	if ( gbSubTermCnt == 0 )
		return SUCCESS;

	DebugOut( "[RemakeTrans] 거래내역파일명		 : %s\n",
		gpstSharedInfo->abTransFileName );
	DebugOut( "[RemakeTrans] 운행후거래내역파일명 : %s\n",
		gpstSharedInfo->abEndedTransFileName );

	/*
	 * 각 하차단말기로부터 승차단말기로 전송된 하차대사파일명을 설정
	 * - YYYYMMDDhhmmss.10
	 *   YYYYMMDDhhmmss.20
	 *   YYYYMMDDhhmmss.30
	 * - 하차단말기가 인지하고 있는 거래내역파일명은
	 *   운행중 거래내역파일명이므로, YYYYMMDDhhmmss 복사시
	 *   운행종료후거래내역파일명이 아닌 운행중 거래내역파일명을
	 *   복사하도록 한다.
	 */
	memset( abRemakeTransFileName, 0, sizeof( abRemakeTransFileName ) );
	for ( i = 0; i < gbSubTermCnt; i++ )
	{
		memcpy( abRemakeTransFileName[i], gpstSharedInfo->abTransFileName, 14 );
		abRemakeTransFileName[i][14] = '.';
		abRemakeTransFileName[i][15] = ( i + 1 ) + '0';
		abRemakeTransFileName[i][16] = '0';

		DebugOutlnASC( "[RemakeTrans] 하차대사파일명 : ",
			abRemakeTransFileName[i], 17 );
	}

	// 승차거래내역파일 TD 건수 확인 ///////////////////////////////////////////
	dwMainTDCnt = ( ( dword )GetFileSize( gpstSharedInfo->abEndedTransFileName )
		- sizeof( TRANS_TH ) - sizeof( TRANS_TT ) ) / sizeof( TRANS_TD );

	DebugOut( "[RemakeTrans] 거래내역파일건수 : %lu\n", dwMainTDCnt );

	/*
	 * 거래내역파일 접근 세마포어 ALLOC ****************************************
	 */
	SemaAlloc( SEMA_KEY_TRANS );

	/*
	 * 승차거래내역파일의 TH 및 TD를 임시대사파일로 복사
	 */
	sResult = CopyTransHeaderAndTransData( gpstSharedInfo->abEndedTransFileName,
		TEMP_REMAKE_FILE, &dwTotalCnt, &dwTotalAmt );
	if ( sResult != SUCCESS )
	{
		goto FINALLY;
	}

	// TD 복사 ( 하차임시대사파일 -> 임시대사파일 ) ////////////////////////////
	for ( i = 0; i < gbSubTermCnt; i++ )
	{
		DebugOutlnASC( "[RemakeTrans] 하차대사파일 처리시작 : ",
			abRemakeTransFileName[i], 17 );

		// 하차임시대사파일이 존재하지 않으면 continue /////////////////////////
		if ( !IsExistFile( abRemakeTransFileName[i] ) )
		{
			DebugOutlnASC( "[RemakeTrans] 하차대사파일 미존재 : ",
				abRemakeTransFileName[i], 17 );
			continue;
		}

		sResult = RemakeTransData( gpstSharedInfo->abEndedTransFileName,
			abRemakeTransFileName[i], TEMP_REMAKE_FILE,
			&dwTotalCnt, &dwTotalAmt );
		if ( sResult != SUCCESS )
		{
			goto FINALLY;
		}

		DebugOutlnASC( "[RemakeTrans] 하차대사파일 처리종료 : ",
			abRemakeTransFileName[i], 17 );
	}

	sResult = CopyTransTail( gpstSharedInfo->abEndedTransFileName,
		TEMP_REMAKE_FILE, dwTotalCnt, dwTotalAmt );
	if ( sResult != SUCCESS )
	{
		goto FINALLY;
	}

	// 임시대사파일을 승차거래내역파일로 RENAME ////////////////////////////////
	for ( i = 0; i < 3; i++ )
	{
		nResultSize = rename( TEMP_REMAKE_FILE,
			gpstSharedInfo->abEndedTransFileName );
		if ( nResultSize != -1 )
			break;
	}
	if ( nResultSize == -1 )
	{
		ctrl_event_info_write( TR_REMAKE_RENAME_ERROR_EVENT );
		sResult = ERR_CARD_PROC_FILE_RENAME_TEMP_REMAKE;
		goto FINALLY;
	}

	FINALLY:

	// 하차단말기대사파일 및 임시대사파일 DELETE ///////////////////////////////
	for ( i = 0; i < 3; i++ )
	{
		unlink( abRemakeTransFileName[i] );
	}

	unlink( TEMP_REMAKE_FILE );

	/*
	 * 거래내역파일 접근 세마포어 FREE *****************************************
	 */
	SemaFree( SEMA_KEY_TRANS );

	ErrRet( sResult );

	return sResult;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       CopyTransHeaderAndTransData                              *
*                                                                              *
*  DESCRIPTION:       승차거래내역파일의 헤더 및 데이터를 임시대사파일로       *
                      복사한다. 이 과정에서 총건수 및 총금액을 다시 계산한다.  *
*                                                                              *
*  INPUT PARAMETERS:  abMainTransFileName - 승차거래내역파일명                 *
*                     abSubTempRemakeFileName - 하차임시대사파일명             *
*                     abTempRemakeFileName - 임시대사파일명                    *
*                     pdwTotalCnt - 총건수 포인터                              *
*                     pdwTotalAmt - 총금액 포인터                              *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공                                      *
*                     ERR_CARD_PROC_FILE_OPEN_MAIN_TRANS                       *
*                         - 승차거래내역파일 OPEN 오류                         *
*                     ERR_CARD_PROC_FILE_READ_MAIN_TRANS                       *
*                         - 승차거래내역파일 READ 오류                         *
*                     ERR_CARD_PROC_FILE_OPEN_TEMP_REMAKE                      *
*                         - 임시대사파일 OPEN 오류                             *
*                     ERR_CARD_PROC_FILE_WRITE_TEMP_REMAKE                     *
*                         - 임시대사파일 WRITE 오류                            *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-29                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static short CopyTransHeaderAndTransData( byte *abSrcTransFileName,
	byte *abDesTransFileName, dword *pdwTotalCnt, dword *pdwTotalAmt )
{
	short sResult = SUCCESS;
	dword i = 0;
	dword dwSrcTDCnt = 0;
	FILE *fdSrcFile = NULL;
	FILE *fdDesFile = NULL;
	TRANS_TH stTransTH;
	TRANS_TD stTransTD;

	dwSrcTDCnt = ( ( dword )GetFileSize( abSrcTransFileName ) -
		sizeof( TRANS_TH ) - sizeof( TRANS_TT ) ) / sizeof( TRANS_TD );

	DebugOut( "[CopyTransHeaderAndTransData] 원본거래내역건수	  : %lu\n",
		dwSrcTDCnt );

	/*
	 * 거래내역헤더 COPY
	 */
	sResult = FileReadTransHeader( &stTransTH, abSrcTransFileName );
	if ( sResult != SUCCESS )
	{
		switch ( sResult )
		{
			case ERR_CARD_PROC_FILE_OPEN_TRANS:
				return ERR_CARD_PROC_FILE_OPEN_MAIN_TRANS;
			case ERR_CARD_PROC_FILE_READ_TRANS:
				return ERR_CARD_PROC_FILE_READ_MAIN_TRANS;
		}
		return ERR_CARD_PROC_FILE_READ_MAIN_TRANS;
	}
	sResult = FileWriteTransHeader( &stTransTH, abDesTransFileName );
	if ( sResult != SUCCESS )
	{
		switch ( sResult )
		{
			case ERR_CARD_PROC_FILE_OPEN_TRANS:
				return ERR_CARD_PROC_FILE_OPEN_TEMP_REMAKE;
			case ERR_CARD_PROC_FILE_WRITE_TRANS:
				return ERR_CARD_PROC_FILE_WRITE_TEMP_REMAKE;
		}
		return ERR_CARD_PROC_FILE_WRITE_TEMP_REMAKE;
	}

	/*
	 * 거래내역데이터 COPY
	 */
	fdSrcFile = fopen( abSrcTransFileName, "rb" );
	if ( fdSrcFile == NULL )
	{
		sResult = ERR_CARD_PROC_FILE_OPEN_MAIN_TRANS;
		goto FINALLY;
	}

	fdDesFile = fopen( abDesTransFileName, "ab" );
	if ( fdDesFile == NULL )
	{
		sResult = ERR_CARD_PROC_FILE_OPEN_TEMP_REMAKE;
		goto FINALLY;
	}

	for ( i = 0; i < dwSrcTDCnt; i++ )
	{
		sResult = FileReadTransDataWithFD( fdSrcFile, &stTransTD, i );
		if ( sResult != SUCCESS )
		{
			sResult = ERR_CARD_PROC_FILE_READ_MAIN_TRANS;
			goto FINALLY;
		}
		sResult = FileAppendTransDataWithFD( fdDesFile, &stTransTD );
		if ( sResult != SUCCESS )
		{
			sResult = ERR_CARD_PROC_FILE_WRITE_TEMP_REMAKE;
			goto FINALLY;
		}

		*pdwTotalCnt += 1;
		*pdwTotalAmt += stTransTD.dwFare;
	}

	FINALLY:

	if ( fdSrcFile != NULL )
	{
		fclose( fdSrcFile );
	}
	if ( fdDesFile != NULL )
	{
		fclose( fdDesFile );
	}

	return SUCCESS;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       RemakeTransData                                          *
*                                                                              *
*  DESCRIPTION:       승차거래내역파일과 하차임시대사파일간의 대사작업을       *
*                     수행하여 임시대사파일에 WRITE한다.                       *
*                     (하차임시대사파일에 존재하면서 승차거래내역파일에        *
*                      존재하지 않는 거래내역데이터를 임시대파일에 추가한다.)  *
*                                                                              *
*  INPUT PARAMETERS:  abMainTransFileName - 승차거래내역파일명                 *
*                     abSubTempRemakeFileName - 하차임시대사파일명             *
*                     abTempRemakeFileName - 임시대사파일명                    *
*                     pdwTotalCnt - 총건수 포인터                              *
*                     pdwTotalAmt - 총금액 포인터                              *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공                                      *
*                     ERR_CARD_PROC_FILE_OPEN_MAIN_TRANS                       *
*                         - 승차거래내역파일 OPEN 오류                         *
*                     ERR_CARD_PROC_FILE_OPEN_SUB_TEMP_REMAKE                  *
*                         - 하차임시대사파일 OPEN 오류                         *
*                     ERR_CARD_PROC_FILE_OPEN_TEMP_REMAKE                      *
*                         - 임시대사파일 OPEN 오류                             *
*                     ERR_CARD_PROC_FILE_READ_SUB_TEMP_REMAKE                  *
*                         - 하차임시대사파일 READ 오류                         *
*                     ERR_CARD_PROC_FILE_WRITE_TEMP_REMAKE                     *
*                         - 임시대사파일 WRITE 오류                            *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-29                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static short RemakeTransData( byte *abMainTransFileName,
	byte *abSubTempRemakeFileName, byte *abTempRemakeFileName,
	dword *pdwTotalCnt, dword *pdwTotalAmt )
{
	short sResult = SUCCESS;
	dword dwCnt = 0;
	dword dwSubRemakeFileSize = 0;
	dword dwMainTDCnt = 0;
	TRANS_TD stTransTD;

	FILE *fdMainTransFile = NULL;
	FILE *fdSubRemakeFile = NULL;
	FILE *fdTempRemakeFile = NULL;

	// 하차임시대사파일 SIZE 확인 //////////////////////////////////////////////
	dwSubRemakeFileSize = ( dword )GetFileSize( abSubTempRemakeFileName );

	if ( dwSubRemakeFileSize % sizeof( TRANS_TD ) != 0 )
	{
		DebugOut( "[DEBUG] 하차대사거래내역파일 크기 오류\n" );
		return SUCCESS;
	}

	dwMainTDCnt = ( ( dword )GetFileSize( abMainTransFileName ) -
		sizeof( TRANS_TH ) - sizeof( TRANS_TT ) ) / sizeof( TRANS_TD );

	fdMainTransFile = fopen( abMainTransFileName, "rb" );
	if ( fdMainTransFile == NULL )
	{
		sResult = ERR_CARD_PROC_FILE_OPEN_MAIN_TRANS;
		goto FINALLY;
	}

	fdSubRemakeFile = fopen( abSubTempRemakeFileName, "rb" );
	if ( fdSubRemakeFile == NULL )
	{
		sResult = ERR_CARD_PROC_FILE_OPEN_SUB_TEMP_REMAKE;
		goto FINALLY;
	}

	fdTempRemakeFile = fopen( abTempRemakeFileName, "ab" );
	if ( fdTempRemakeFile == NULL )
	{
		sResult = ERR_CARD_PROC_FILE_OPEN_TEMP_REMAKE;
		goto FINALLY;
	}

	// 하차대사거래내역파일 각 TD를 승차거래내역파일의 모든 TD와 ///////////////
	// 비교한 후 존재하지 않으면 임시대사파일에 추가
	for ( dwCnt = 0; dwCnt < dwSubRemakeFileSize / sizeof( TRANS_TD ); dwCnt++ )
	{
		sResult = FileReadTransDataWithoutTransHeaderWithFD( fdSubRemakeFile,
			&stTransTD, dwCnt );
		if ( sResult != SUCCESS )
		{
			sResult = ERR_CARD_PROC_FILE_READ_SUB_TEMP_REMAKE;
			goto FINALLY;
		}

		DebugOutlnBCD( "[RemakeTrans] \t하차대사파일 TD READ : ",
			stTransTD.abEntExtDtime, 7 );

		// 존재하지 않는 TD의 경우 임시대사파일에 추가한다.
		if ( IsExistTransDataWithFD( fdMainTransFile, &stTransTD, dwMainTDCnt )
			== FALSE )
		{
			DebugOut( "[RemakeTrans] \t존재하지 않는 TD이므로 추가함\n" );

			sResult = FileAppendTransDataWithFD( fdTempRemakeFile, &stTransTD );
			if ( sResult != SUCCESS )
			{
				sResult = ERR_CARD_PROC_FILE_WRITE_TEMP_REMAKE;
				goto FINALLY;
			}

			*pdwTotalCnt += 1;
			*pdwTotalAmt += stTransTD.dwFare;
		}
		else
		{
			DebugOut( "[RemakeTrans] \t이미 존재하는 TD이므로 추가하지" );
			DebugOut( " 않음\n" );
		}
	}

	FINALLY:

	if ( fdMainTransFile != NULL )
	{
		fclose( fdMainTransFile );
	}
	if ( fdSubRemakeFile != NULL )
	{
		fclose( fdSubRemakeFile );
	}
	if ( fdTempRemakeFile != NULL )
	{
		fclose( fdTempRemakeFile );
	}

	return sResult;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       CopyTransTail                                            *
*                                                                              *
*  DESCRIPTION:       승차거래내역파일의 거래내역테일을 임시대사파일로         *
*                     복사한다. 단, 총건수 및 총금액을 다시 설정한다.          *
*                                                                              *
*  INPUT PARAMETERS:  abSrcTransFileName - 승차거래내역파일명                  *
*                     abDesTransFileName - 임시대사파일명                      *
*                     dwTotalCnt - 총건수                                      *
*                     dwTotalAmt - 총금액                                      *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공                                      *
*                     ERR_CARD_PROC_FILE_OPEN_MAIN_TRANS                       *
*                         - 승차거래내역파일 OPEN 오류                         *
*                     ERR_CARD_PROC_FILE_READ_MAIN_TRANS                       *
*                         - 승차거래내역파일 READ 오류                         *
*                     ERR_CARD_PROC_FILE_OPEN_TEMP_REMAKE                      *
*                         - 임시대사파일 OPEN 오류                             *
*                     ERR_CARD_PROC_FILE_WRITE_TEMP_REMAKE                     *
*                         - 임시대사파일 WRITE 오류                            *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-29                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static short CopyTransTail( byte *abSrcTransFileName,
	byte *abDesTransFileName, dword dwTotalCnt, dword dwTotalAmt )
{
	short sResult = SUCCESS;
	dword dwCRC = 0;
	TRANS_TT stTransTT;

	// TT 복사 ( 승차거래내역파일 -> 임시대사파일 ) //////////////////////////////
	// 승차거래내역파일 READ
	sResult = FileReadTransTail( &stTransTT, abSrcTransFileName );
	if ( sResult != SUCCESS )
	{
		switch ( sResult )
		{
			case ERR_CARD_PROC_FILE_OPEN_TRANS:
				return ERR_CARD_PROC_FILE_OPEN_MAIN_TRANS;
			case ERR_CARD_PROC_FILE_READ_TRANS:
				return ERR_CARD_PROC_FILE_READ_MAIN_TRANS;
		}
		return ERR_CARD_PROC_FILE_READ_MAIN_TRANS;
	}

	// 총건수 다시 설정
	DWORD2ASCWithFillLeft0( dwTotalCnt, stTransTT.abTotalCnt, 9 );
	// 총금액 다시 설정
	DWORD2ASCWithFillLeft0( dwTotalAmt, stTransTT.abTotalAmt, 10 );
	// Record MAC 다시 설정
	dwCRC = MakeCRC32( ( byte * )&stTransTT, sizeof( TRANS_TT ) - 6 );
	memcpy( stTransTT.abRecMAC, ( byte * )&dwCRC,
		sizeof( stTransTT.abRecMAC ) );

	// 임시대사파일에 WRITE
	sResult = FileAppendTransTail( &stTransTT, abDesTransFileName );
	if ( sResult != SUCCESS )
	{
		switch ( sResult )
		{
			case ERR_CARD_PROC_FILE_OPEN_TRANS:
				return ERR_CARD_PROC_FILE_OPEN_TEMP_REMAKE;
			case ERR_CARD_PROC_FILE_WRITE_TRANS:
				return ERR_CARD_PROC_FILE_WRITE_TEMP_REMAKE;
		}
		return ERR_CARD_PROC_FILE_WRITE_TEMP_REMAKE;
	}

	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       GetTermAggrSeqNo                                         *
*                                                                              *
*  DESCRIPTION:       단말기사용집계일련번호를 "seqth.tmp" 파일로부터          *
*                     1 증가하여 가져온다.                                     *
*                                                                              *
*  INPUT PARAMETERS:  void                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: word - 단말기사용집계일련번호                            *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-29                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static word GetTermAggrSeqNo( void )
{
	int nResult = 0;
	int fdFile = 0;
	word wSeqNo = 0;
	byte abBuf[4] = {0, };

	/*
	 * 같은 이름의 파일이 이미 존재하는 경우 ***********************************
	 * - 파일로부터 일련번호를 읽어 1 증가하여 WRITE한 후 그 값을 리턴한다.
	 */
	if ( IsExistFile( TERM_AGGR_SEQ_NO_FILE ) )
	{
		fdFile = open( TERM_AGGR_SEQ_NO_FILE, O_RDWR | O_CREAT );
		if ( fdFile < 0 )
		{
			return 0;
		}

		nResult = read( fdFile, abBuf, sizeof( abBuf ) );
		// 정상 READ - READ한 size가 4
		if ( nResult == 4 )
		{
			wSeqNo = GetWORDFromASC( abBuf, 4 );
		}
		// 오류 READ - READ한 size가 4가 아님
		else
		{
			close( fdFile );
			return 0;
		}

		// 0000 ~ 9999 로테이션
		wSeqNo = ( wSeqNo + 1 ) % 10000;

		WORD2ASCWithFillLeft0( wSeqNo, abBuf, sizeof( abBuf ) );

		// 파일의 시작으로 포인터 이동 - 리턴값 무시
		lseek( fdFile, 0L, SEEK_SET );

		nResult = write( fdFile, abBuf, sizeof( abBuf ) );

		// 오류 WRITE 이면 - WRITE한 size가 4가 아니면
		if ( nResult != 4 )
		{
			close( fdFile );
			return 0;
		}

		close( fdFile );

		return wSeqNo;
	}
	/*
	 * 같은 이름의 파일이 존재하지 않는 경우 ***********************************
	 * - "0000"의 값을 파일에 WRITE한 후 0을 리턴한다.
	 */
	else
	{
		fdFile = open( TERM_AGGR_SEQ_NO_FILE, O_RDWR | O_CREAT );
		if ( fdFile < 0 )
		{
			return 0;
		}

		memset( abBuf, '0', sizeof( abBuf ) );

		nResult = write( fdFile, abBuf, sizeof( abBuf ) );

		// 오류 WRITE 이면 - WRITE한 size가 4가 아니면
		if ( nResult != 4 )
		{
			close( fdFile );
			return 0;
		}

		close( fdFile );

		return 0;
	}

	return 0;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       GetTermAggrSeqNo                                         *
*                                                                              *
*  DESCRIPTION:       백업거래내역순번을 "backseq.tmp" 파일로부터              *
*                     1 증가하여 가져온다.                                     *
*                                                                              *
*  INPUT PARAMETERS:  void                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: word - 백업거래내역순번                                  *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-29                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static word GetBackupTransDataSeqNo( void )
{
	int fdFile = 0;
	word wSeqNo = 0;
	int nResult = 0;
	byte abBuf[4] = {0, };

	// 같은 이름의 파일 존재함
	if ( IsExistFile( BACKUP_TRANS_SEQ_NO_FILE ) )
	{
		fdFile = open( BACKUP_TRANS_SEQ_NO_FILE, O_RDWR | O_CREAT );
		if ( fdFile < 0 )
		{
			return 0;
		}

		nResult = read( fdFile, abBuf, sizeof( abBuf ) );
		// 정상 READ
		if ( nResult == 4 )
		{
			wSeqNo = GetWORDFromASC( abBuf, 4 );
		}
		// 오류 READ
		else
		{
			close( fdFile );
			return 0;
		}

		wSeqNo = ( wSeqNo + 1 ) % 5000;

		WORD2ASCWithFillLeft0( wSeqNo, abBuf, sizeof( abBuf ) );

		// 리턴값을 무시한다.
		lseek( fdFile, 0L, SEEK_SET );

		nResult = write( fdFile, abBuf, sizeof( abBuf ) );
		if ( nResult != 4 )
		{
			close( fdFile );
			return 0;
		}

		close( fdFile );

		return wSeqNo;
	}
	// 같은 이름의 파일 존재하지 않음
	else
	{
		fdFile = open( BACKUP_TRANS_SEQ_NO_FILE, O_RDWR | O_CREAT );
		if ( fdFile < 0 )
		{
			return 0;
		}

		memset( abBuf, '0', sizeof( abBuf ) );

		nResult = write( fdFile, abBuf, sizeof( abBuf ) );
		if ( nResult != 4 )
		{
			close( fdFile );
			return 0;
		}

		close( fdFile );

		return 0;
	}

	return 0;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       GetCashTransSeqNo                                        *
*                                                                              *
*  DESCRIPTION:       일회권ID일련번호를 "ticketseq.tmp" 파일로부터            *
*                     1 증가하여 가져온다.                                     *
*                                                                              *
*  INPUT PARAMETERS:  void                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: word -  일회권ID일련번호                                 *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-30                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static word GetCashTransSeqNo( void )
{
	int fdFile = 0;
	word wSeqNo = 0;
	int nResult = 0;
	byte abBuf[5];

	// 같은 이름의 파일 존재함
	if ( IsExistFile( CASH_GETON_SEQ_NO_FILE ) )
	{
		fdFile = open( CASH_GETON_SEQ_NO_FILE, O_RDWR | O_CREAT );
		if ( fdFile < 0 )
		{
			return 1;
		}

		nResult = read( fdFile, abBuf, sizeof( abBuf ) );
		// 정상 READ
		if ( nResult == 5 )
		{
			wSeqNo = GetWORDFromASC( abBuf, 5 );
		}
		// 오류 READ
		else
		{
			close( fdFile );
			return 1;
		}

		wSeqNo = ( wSeqNo + 1 ) % 100000;

		WORD2ASCWithFillLeft0( wSeqNo, abBuf, sizeof( abBuf ) );

		// 리턴값을 무시한다.
		lseek( fdFile, 0L, SEEK_SET );

		nResult = write( fdFile, abBuf, sizeof( abBuf ) );
		if ( nResult != 5 )
		{
			close( fdFile );
			return 1;
		}

		close( fdFile );

		return wSeqNo;
	}
	// 같은 이름의 파일 존재하지 않음
	else
	{
		fdFile = open( CASH_GETON_SEQ_NO_FILE, O_RDWR | O_CREAT );
		if ( fdFile < 0 )
		{
			return 1;
		}

		// memset( abBuf, '0', sizeof( abBuf ) );
		memcpy( abBuf, "00001", sizeof( abBuf ) );

		nResult = write( fdFile, abBuf, sizeof( abBuf ) );
		if ( nResult != 5 )
		{
			close( fdFile );
			return 1;
		}

		close( fdFile );

		return 1;
	}

	return 1;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       SetControlTrans                                          *
*                                                                              *
*  DESCRIPTION:       운행정보파일 구조체를 초기화한다.                        *
*                                                                              *
*  INPUT PARAMETERS:  pstControlTrans - 운행정보파일 구조체 포인터             *
*                     abTransFileName - 거래내역파일명                         *
*                                                                              *
*  RETURN/EXIT VALUE: void                                                     *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2006-03-14                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static void SetControlTrans(CONTROL_TRANS *pstControlTrans,
	byte *abTransFileName)
{
	memset( ( byte * )pstControlTrans, 0, sizeof( CONTROL_TRANS ) );

	// 1. 운행구분
	pstControlTrans->bDriveNow = '1';

	// 2. 총건수
	memset( pstControlTrans->abTotalCnt, '0',
		sizeof( pstControlTrans->abTotalCnt ) );

	// 3. 총금액
	memset( pstControlTrans->abTotalAmt, '0',
		sizeof( pstControlTrans->abTotalAmt ) );

	// 4. TR파일명
	memcpy( pstControlTrans->abTRFileName, abTransFileName,
		sizeof( pstControlTrans->abTRFileName ) );
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       SetTransHeader                                           *
*                                                                              *
*  DESCRIPTION:       운행시작일시 등의 정보를 이용하여 거래내역헤더를         *
*                     설정한다.                                                *
*                                                                              *
*  INPUT PARAMETERS:  pstTransTH - 설정하고자 하는 거래내역헤더 구조체 포인터  *
*                     tStartDriveDtime - 운행시작일시                          *
*                                                                              *
*  RETURN/EXIT VALUE: void                                                     *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2006-03-14                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static void SetTransHeader( TRANS_TH *pstTransTH, time_t tStartDriveDtime )
{
	memset( ( byte * )pstTransTH, 0, sizeof( TRANS_TH ) );

	// 1. Recode 길이 - 185
	memcpy( pstTransTH->abTHRecLen, LENGTH_OF_TRANS_HEADER,
		sizeof( pstTransTH->abTHRecLen ) );

	// 2. Record 구분 - H
	pstTransTH->bTHRecType = RECORD_TYPE_TRANS_HEADER;

	// 3. 교통사업자 ID
	memcpy( pstTransTH->abTranspBizrID, gstVehicleParm.abTranspBizrID,
		sizeof( pstTransTH->abTranspBizrID ) );

	// 4. 영업소 ID
	memcpy( pstTransTH->abBizOfficeID, gstVehicleParm.abBusBizOfficeID,
		sizeof( pstTransTH->abBizOfficeID ) );

	// 5. 버스 노선 ID
	memcpy( pstTransTH->abRouteID, gstVehicleParm.abRouteID,
		sizeof( pstTransTH->abRouteID ) );

	// 6. 교통수단코드
	memcpy( pstTransTH->abTranspMethodCode, gstVehicleParm.abTranspMethodCode,
		sizeof( pstTransTH->abTranspMethodCode ) );

	// 7. 차량 ID
	memcpy( pstTransTH->abVehicleID, gstVehicleParm.abVehicleID,
		sizeof( pstTransTH->abVehicleID ) );

	// 8. 단말기 ID
	memcpy( pstTransTH->abTermID, gpstSharedInfo->abMainTermID,
		sizeof( pstTransTH->abTermID ) );

	// 9. 운전자 ID
	memcpy( pstTransTH->abDriverID, gstVehicleParm.abDriverID,
		sizeof( pstTransTH->abDriverID ) );

	// 10. 집계시스템 수신일시 - 집계시스템으로 전송시 다시 업데이트됨
	memset( pstTransTH->abDCSRecvDtime, '0',
		sizeof( pstTransTH->abDCSRecvDtime ) );

	// 11. 시작정류장 ID
	memcpy( pstTransTH->abStartStationID, gpstSharedInfo->abNowStationID,
		sizeof( pstTransTH->abStartStationID ) );

	// 12. 운행 시작 일시
	TimeT2ASCDtime( tStartDriveDtime, pstTransTH->abStartDtime );

	// 13. 단말기사용집계일련번호
	WORD2ASCWithFillLeft0( GetTermAggrSeqNo(), pstTransTH->abTermUseSeqNo,
		sizeof( pstTransTH->abTermUseSeqNo ) );

	// 14. File Name
	memset( pstTransTH->abFileName, 0, sizeof( pstTransTH->abFileName ) );

	// 15. Record Mac
	memset( pstTransTH->abRecMAC, '0', sizeof( pstTransTH->abRecMAC ) );

	// 16. 개행 문자
	pstTransTH->abCRLF[0] = 0x0D;
	pstTransTH->abCRLF[1] = 0x0A;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       SetTransData                                             *
*                                                                              *
*  DESCRIPTION:       카드정보구조체로부터 거래내역데이터를 조립한다.          *
*                                                                              *
*  INPUT PARAMETERS:  pstTransInfo - 카드정보 구조체의 포인터                  *
*                     pstTransTD - 거래내역데이터 구조체의 포인터              *
*                                                                              *
*  RETURN/EXIT VALUE: void                                                     *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-29                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static void SetTransData( TRANS_INFO *pstTransInfo, TRANS_TD *pstTransTD )
{
	byte abBuf[8] = {0, };
	dword dwCRC = 0;

	memset( ( byte * )pstTransTD, 0, sizeof( TRANS_TD ) );

	// 1. TD Record 길이
	memcpy( pstTransTD->abTDRecLen, LENGTH_OF_TRANS_DATA,
		sizeof( pstTransTD->abTDRecLen ) );

	// 2. TD Record 구분
	pstTransTD->bTDRecType = RECORD_TYPE_TRANS_DATA;

	// 3. 카드유형
	if ( pstTransInfo->bCardType == TRANS_CARDTYPE_MIF_TOUR )
	{
		pstTransTD->bCardType = TRANS_CARDTYPE_SC_PREPAY;
	}
	else
	{
		pstTransTD->bCardType = pstTransInfo->bCardType;
	}

	// 4. 승하차유형
	if ( IsEnt( pstTransInfo->stNewXferInfo.bEntExtType ) )
	{
		pstTransTD->bEntExtType = '0';				// 승차
	}
	else
	{
		pstTransTD->bEntExtType = '1';				// 하차
	}

	// 5. 카드번호/일회권ID
	ASC2BCD( pstTransInfo->abCardNo, pstTransTD->abCardNo,
		sizeof( pstTransInfo->abCardNo ) );

	// 6. alias번호
	pstTransTD->dwAliasNo = pstTransInfo->dwAliasNo;

	// 7. 승하차일시
	TimeT2BCDDtime( pstTransInfo->stNewXferInfo.tEntExtDtime,
		pstTransTD->abEntExtDtime );

	LogMain("거래1 : %lu\n", pstTransInfo->stNewXferInfo.tEntExtDtime);
	LogMain("거래2 : %02x%02x%02x%02x%02x%02x%02x\n",
		pstTransTD->abEntExtDtime[0],
		pstTransTD->abEntExtDtime[1],
		pstTransTD->abEntExtDtime[2],
		pstTransTD->abEntExtDtime[3],
		pstTransTD->abEntExtDtime[4],
		pstTransTD->abEntExtDtime[5],
		pstTransTD->abEntExtDtime[6]);

	// 8. 사용자유형 - 카드 사용자 유형을 넣어줌
	pstTransTD->bUserType = pstTransInfo->bCardUserType;

	// 9. 승객1 할인할증유형ID
	memcpy( pstTransTD->abDisExtraTypeID1, pstTransInfo->abDisExtraTypeID[0],
		sizeof( pstTransTD->abDisExtraTypeID1 ) );

	// 11. 승객2 할인할증유형ID
	memcpy( pstTransTD->abDisExtraTypeID2, pstTransInfo->abDisExtraTypeID[1],
		sizeof( pstTransTD->abDisExtraTypeID2 ) );

	// 13. 승객3 할인할증유형ID
	memcpy( pstTransTD->abDisExtraTypeID3, pstTransInfo->abDisExtraTypeID[2],
		sizeof( pstTransTD->abDisExtraTypeID3 ) );

	if ( IsEnt( pstTransInfo->stNewXferInfo.bEntExtType ) )
	{
		// 10. 승객수1
		pstTransTD->wUserCnt1 = pstTransInfo->abUserCnt[0];

		// 12. 승객수2
		pstTransTD->wUserCnt2 = pstTransInfo->abUserCnt[1];

		// 14. 승객수3
		pstTransTD->wUserCnt3 = pstTransInfo->abUserCnt[2];
	}
	else
	{
		// 10. 승객수1
		pstTransTD->wUserCnt1 =
			pstTransInfo->stPrevXferInfo.abMultiEntInfo[0][USER_CNT];

		// 12. 승객수2
		pstTransTD->wUserCnt2 =
			pstTransInfo->stPrevXferInfo.abMultiEntInfo[1][USER_CNT];

		// 14. 승객수3
		pstTransTD->wUserCnt3 =
			pstTransInfo->stPrevXferInfo.abMultiEntInfo[2][USER_CNT];
	}

	// 15. 패널티유형
	if ( pstTransInfo->stNewXferInfo.wPrevPenaltyFare != 0 )
	{
		memcpy( pstTransTD->abPenaltyType, PENALTY_TYPE_NO_TAG_IN_EXT,
			sizeof( pstTransTD->abPenaltyType ) );
	}
	else
	{
		memset( pstTransTD->abPenaltyType, 0,
			sizeof( pstTransTD->abPenaltyType ) );
	}

	// 16. 페널티요금
	pstTransTD->dwPenaltyFare = pstTransInfo->stNewXferInfo.wPrevPenaltyFare;

	// 17. 알고리즘유형
	pstTransTD->bSCAlgoriType = pstTransInfo->bSCAlgoriType;

	// 18. 거래유형
	pstTransTD->bSCTransType = pstTransInfo->bSCTransType;

	// 19. 개별거래수집키버전
	pstTransTD->bSCTransKeyVer = pstTransInfo->bSCTransKeyVer;

	// 20. 전자화폐사ID
	pstTransTD->bSCEpurseIssuerID = pstTransInfo->bSCEpurseIssuerID;

	// 21. 전자지갑ID ( 카드식별자 )
	//     1) 관광권카드 - 최초사용일시
	//     2) 구선불카드/예치금카드 - 구선불카드 직전충전승인번호
	//     3) 그 외 카드 - 전자지갑ID (구후불카드는 모두 0으로 설정)
	if ( pstTransInfo->bCardType == TRANS_CARDTYPE_MIF_TOUR )
	{
		if ( pstTransInfo->tMifTourFirstUseDtime == 0 )
		{
			TimeT2BCDDtime( pstTransInfo->stNewXferInfo.tEntExtDtime,
				pstTransTD->abSCEpurseID );
		}
		else
		{
			TimeT2BCDDtime( pstTransInfo->tMifTourFirstUseDtime,
				pstTransTD->abSCEpurseID );
		}
	}
	else if ( pstTransInfo->bCardType == TRANS_CARDTYPE_MIF_PREPAY ||
			  pstTransInfo->bCardType == TRANS_CARDTYPE_DEPOSIT )
	{
		ASC2BCD( pstTransInfo->abMifPrepayChargeAppvNop,
			pstTransTD->abSCEpurseID,
			sizeof( pstTransInfo->abMifPrepayChargeAppvNop ) );
	}
	else
	{
		ASC2BCD( pstTransInfo->abSCEpurseID, pstTransTD->abSCEpurseID,
			sizeof( pstTransInfo->abSCEpurseID ) );
	}

	// 22. 카드거래건수
	//     1) 시티투어버스에서 관광권카드 - 이전환승정보의 총누적사용횟수
	//     2) 구선불/구후불/예치금/관광권카드 - 신규환승정보의 총누적사용횟수
	//     3) 신선/후불카드 - 카드의 NTEP
	if ( IsCityTourBus( gstVehicleParm.wTranspMethodCode ) == TRUE &&
		 pstTransInfo->bCardType == TRANS_CARDTYPE_MIF_TOUR )
	{
		pstTransTD->dwSCTransCnt = pstTransInfo->stPrevXferInfo.wTotalAccEntCnt;
	}
	else if ( pstTransInfo->bCardType == TRANS_CARDTYPE_MIF_PREPAY ||
			  pstTransInfo->bCardType == TRANS_CARDTYPE_DEPOSIT ||
			  pstTransInfo->bCardType == TRANS_CARDTYPE_MIF_POSTPAY ||
			  pstTransInfo->bCardType == TRANS_CARDTYPE_MIF_TOUR )
	{
		pstTransTD->dwSCTransCnt = pstTransInfo->stNewXferInfo.wTotalAccEntCnt;
	}
	else
	{
		pstTransTD->dwSCTransCnt = pstTransInfo->dwSCTransCnt;
	}

	// 23. 카드잔액
	//     1) 관광권카드 - 거래 후 총누적사용횟수
	//     2) 예치금카드 - 0
	//     3) 그 외 카드 - 거래 후 잔액
	if ( pstTransInfo->bCardType == TRANS_CARDTYPE_MIF_TOUR )
	{
		pstTransTD->dwBal = pstTransInfo->stNewXferInfo.dwMifTourTotalAccUseCnt;
	}
	else if ( pstTransInfo->bCardType == TRANS_CARDTYPE_DEPOSIT )
	{
		pstTransTD->dwBal = 0;
	}
	else
	{
		pstTransTD->dwBal = pstTransInfo->stNewXferInfo.dwBal;
	}

	// 24. 요금
	pstTransTD->dwFare = pstTransInfo->stNewXferInfo.dwFare;

	// 25. SAM ID
	if ( pstTransInfo->bCardType == TRANS_CARDTYPE_MIF_PREPAY ||
		 pstTransInfo->bCardType == TRANS_CARDTYPE_DEPOSIT ||
		 pstTransInfo->bCardType == TRANS_CARDTYPE_MIF_POSTPAY )
	{
		if ( pstTransInfo->boolIsTCard == TRUE )
		{
			ASC2BCD( gstMyTermInfo.abPSAMID, pstTransTD->abPSAMID,
				sizeof( gstMyTermInfo.abPSAMID ) );
		}
		else
		{
			memset( abBuf, '0', sizeof( abBuf ) );
			memcpy( abBuf, gstMyTermInfo.abISAMID,
				sizeof( gstMyTermInfo.abISAMID ) );
			memset( pstTransTD->abPSAMID, 0, sizeof( pstTransTD->abPSAMID ) );
			ASC2BCD( abBuf, pstTransTD->abPSAMID, 8 );
		}
	}
	else
	{
		ASC2BCD( pstTransInfo->abPSAMID, pstTransTD->abPSAMID,
			sizeof( pstTransInfo->abPSAMID ) );
	}

	// 26. SAM거래카운터
	//     1) 관광권카드 - 이번 승/하차시 증가 횟수
	//     2) 구선불카드/예치금카드 - TCC 앞 4B
	//     3) 그 외 카드 - SAM거래카운터
	if ( pstTransInfo->bCardType == TRANS_CARDTYPE_MIF_TOUR )
	{
		pstTransTD->dwPSAMTransCnt = pstTransInfo->bMifTourUseCnt;
	}
	else if ( ( pstTransInfo->bCardType == TRANS_CARDTYPE_MIF_PREPAY ||
				pstTransInfo->bCardType == TRANS_CARDTYPE_DEPOSIT ) &&
			  pstTransInfo->boolIsTCard == FALSE )
	{
		memcpy( ( byte * )&pstTransTD->dwPSAMTransCnt,
			pstTransInfo->abMifPrepayTCC,
			sizeof( pstTransTD->dwPSAMTransCnt ) );
	}
	else
	{
		pstTransTD->dwPSAMTransCnt = pstTransInfo->dwPSAMTransCnt;
	}

	// 27. SAM총액거래수집카운터
	//     1) 관광권카드 - 관광권카드유형
	//     2) 구선불카드/예치금카드 - TCC 뒤 4B
	//     3) 그 외 카드 - SAM총액거래수집카운터
	if ( pstTransInfo->bCardType == TRANS_CARDTYPE_MIF_TOUR )
	{
		pstTransTD->dwPSAMTotalTransCnt = pstTransInfo->wMifTourCardType;
	}
	else if ( ( pstTransInfo->bCardType == TRANS_CARDTYPE_MIF_PREPAY ||
				pstTransInfo->bCardType == TRANS_CARDTYPE_DEPOSIT ) &&
			  pstTransInfo->boolIsTCard == FALSE )
	{
		memcpy( ( byte * )&pstTransTD->dwPSAMTotalTransCnt,
			&pstTransInfo->abMifPrepayTCC[4],
			sizeof( pstTransTD->dwPSAMTotalTransCnt ) );
	}
	else
	{
		pstTransTD->dwPSAMTotalTransCnt = pstTransInfo->dwPSAMTotalTransCnt;
	}

	// 28. SAM개별거래수집건수
	pstTransTD->wPSAMIndvdTransCnt = pstTransInfo->wPSAMIndvdTransCnt;

	// 29. SAM누적거래총액
	pstTransTD->dwPSAMAccTransAmt = pstTransInfo->dwPSAMAccTransAmt;

	// 30. SAM서명
	if ( pstTransInfo->boolIsTCard == TRUE )
	{
		memcpy( pstTransTD->abPSAMSign, pstTransInfo->abMifPrepayTCC,
			sizeof( pstTransTD->abPSAMSign ) );
	}
	else
	{
		pstTransTD->abPSAMSign[0] = pstTransInfo->abPSAMSign[3];
		pstTransTD->abPSAMSign[1] = pstTransInfo->abPSAMSign[2];
		pstTransTD->abPSAMSign[2] = pstTransInfo->abPSAMSign[1];
		pstTransTD->abPSAMSign[3] = pstTransInfo->abPSAMSign[0];
	}

	// 31. 환승일련번호
	//     1) 시티투어버스 - "300"
	//     2) 그 외 교통수단 - 환승일련번호
	if ( IsCityTourBus( gstVehicleParm.wTranspMethodCode ) == TRUE )
	{
		memcpy( pstTransTD->abXferSeqNo, "300",
			sizeof( pstTransTD->abXferSeqNo ) );
	}
	else
	{
		WORD2ASCWithFillLeft0( ( word )pstTransInfo->stNewXferInfo.bXferSeqNo,
			pstTransTD->abXferSeqNo, sizeof( pstTransTD->abXferSeqNo ) );
	}

	// 32. 승객1 이전거래시 지불된 최대기본요금
	pstTransTD->dwPrevMaxBaseFare1 =
		pstTransInfo->stPrevXferInfo.awMaxBaseFare[0];

	// 33. 거래전잔액 (승객2 이전거래시 지불된 최대기본요금)
	//     1) 관광권카드 - 거래 전 총누적사용횟수
	//     2) 예치금카드 - 0
	//     3) 그 외 카드 - 거래 전 잔액
	if ( pstTransInfo->bCardType == TRANS_CARDTYPE_MIF_TOUR )
	{
		pstTransTD->dwPrevMaxBaseFare2 =
			pstTransInfo->stPrevXferInfo.dwMifTourTotalAccUseCnt;
	}
	else if ( pstTransInfo->bCardType == TRANS_CARDTYPE_DEPOSIT )
	{
		pstTransTD->dwPrevMaxBaseFare2 = 0;
	}
	else
	{
		pstTransTD->dwPrevMaxBaseFare2 =
			pstTransInfo->stPrevXferInfo.dwBal;
	}

	// 34. 승객3 이전거래시 지불된 최대기본요금
	//	  - 4바이트를 나누어 다음의 정보를 기록하도록 전용하고 있음
	//		 [0] : 인천환승플래그
	//		 [1] : 상위니블 : GPS상태플래그, 하위니블 : 추가승차플래그
	//		 [2] : 카드복구로그
	//		 [3] : 환승미처리로그

	// [0] : 인천환승플래그
	if ( pstTransInfo->stNewXferInfo.bEntExtType == XFER_ENT_AFTER_INCHEON ||
		 pstTransInfo->stNewXferInfo.bEntExtType == XFER_EXT_AFTER_INCHEON )
	{
		pstTransTD->abPrevMaxBaseFare3[0] = 0x01;
	}
	// [1] 상위니블 : GPS상태플래그
	//		 - 0x00 : 정상
	//		 - 0x01 : 감도이상
	//		 - 0x02 : 수신단절
	//		 - 0x03 : 거리이상
	pstTransTD->abPrevMaxBaseFare3[1] = gpstSharedInfo->gbGPSStatusFlag << 4;
	
	// [1] 하위니블 : 추가승차/정류장보정/하차시간보정 플래그
	//        7 6 5 4 3 2 1 0
	//        -------   | | |
	//        GPS상태   | | 추가승차 플래그 (승차단말기에서만 발생)
	//                  | 정류장보정 플래그 (승차단말기에서만 발생)
	//                  하차시간보정 플래그

	// 하차시간이 보정된 경우
	if ( pstTransInfo->boolIsAdjustExtDtime == TRUE )
	{
		pstTransTD->abPrevMaxBaseFare3[1] |= 0x04;		// 0000 0100
	}

	// 승차단말기의 경우
	if ( gboolIsMainTerm == TRUE )
	{
		// 정류장이 보정된 경우
		if ( memcmp( gpstSharedInfo->abNowStationID, gabGPSStationID,
				sizeof( gpstSharedInfo->abNowStationID ) ) != 0 )
		{
			pstTransTD->abPrevMaxBaseFare3[1] |= 0x02;	// 0000 0010
		}

		// 추가승차의 경우
		if ( pstTransInfo->boolIsAddEnt == TRUE )
		{
			pstTransTD->abPrevMaxBaseFare3[1] |= 0x01;	// 0000 0001
		}
	}
	// [2] : 카드복구로그
	pstTransTD->abPrevMaxBaseFare3[2] = pstTransInfo->bWriteErrCnt;
	// [3] : 환승미처리로그
	pstTransTD->abPrevMaxBaseFare3[3] = pstTransInfo->bNonXferCause;

	// 35. 환승내이용수단기본요금의합
	pstTransTD->dwTotalBaseFareInXfer =
		pstTransInfo->stNewXferInfo.wTotalBaseFareInXfer;

	// 36. 환승누적횟수
	pstTransTD->wXferCnt = pstTransInfo->stNewXferInfo.bAccXferCnt;

	// 37. 사용거리
	pstTransTD->dwDist = pstTransInfo->dwDist;

	// 38. 환승내누적이동거리
	pstTransTD->dwAccDistInXfer = pstTransInfo->stNewXferInfo.dwAccDistInXfer;

	// 39. 환승내누적이용금액
	pstTransTD->dwAccUseAmtInXfer = pstTransInfo->stNewXferInfo.dwAccAmtInXfer;

	// 40. 총누적사용금액
	pstTransTD->dwTotalAccAmt = pstTransInfo->stNewXferInfo.dwTotalAccUseAmt;

	// 41. 정류장ID
	memset( abBuf, '0', sizeof( abBuf ) );
	memcpy( abBuf, pstTransInfo->stNewXferInfo.abStationID,
		sizeof( pstTransInfo->stNewXferInfo.abStationID ) );
	ASC2BCD( abBuf, pstTransTD->abStationID, 8 );

	// 42. 직전페널티요금
	pstTransTD->dwPrevPenaltyFare =
		pstTransInfo->stPrevXferInfo.wPrevPenaltyFare;

	// 43. 하차시승차금액
	if ( IsEnt( pstTransInfo->stNewXferInfo.bEntExtType ) )
	{
		pstTransTD->dwPrevEntFare = 0;
	}
	else
	{
		pstTransTD->dwPrevEntFare = pstTransInfo->stPrevXferInfo.dwFare;
	}

	// 44. 직전정류장ID
	memset( abBuf, '0', sizeof( abBuf ) );
	memcpy( abBuf, pstTransInfo->stPrevXferInfo.abStationID,
		sizeof( pstTransInfo->stPrevXferInfo.abStationID ) );
	ASC2BCD( abBuf, pstTransTD->abPrevStationID, 8 );

	// 45. 직전교통수단유형
	memset( abBuf, '0', sizeof( abBuf ) );
	// 인천환승 페널티가 발생한 경우
	if ( !IsSeoulTransp( pstTransInfo->stPrevXferInfo.wTranspMethodCode ) &&
		 pstTransInfo->stPrevXferInfo.wIncheonPenaltyPrevTMCode != 0 &&
		 pstTransInfo->stNewXferInfo.wPrevPenaltyFare != 0 )
	{
		WORD2ASCWithFillLeft0(
			pstTransInfo->stPrevXferInfo.wIncheonPenaltyPrevTMCode, abBuf, 3 );
	}
	// 인천환승 페널티가 발생하지 않은 경우
	else
	{
		WORD2ASCWithFillLeft0( pstTransInfo->stPrevXferInfo.wTranspMethodCode,
			abBuf, 3 );
	}
	ASC2BCD( abBuf, pstTransTD->abPrevTranspMethodCode, 4 );

	// 46. 직전승하차일시
	// 인천환승 페널티가 발생한 경우
	if ( !IsSeoulTransp( pstTransInfo->stPrevXferInfo.wTranspMethodCode ) &&
		 pstTransInfo->stPrevXferInfo.wIncheonPenaltyPrevTMCode != 0 &&
		 pstTransInfo->stNewXferInfo.wPrevPenaltyFare != 0 )
	{
		TimeT2BCDDtime( pstTransInfo->stPrevXferInfo.tIncheonPenaltyPrevDtime,
			pstTransTD->abPrevEntExtDtime );
	}
	// 인천환승 페널티가 발생하지 않은 경우
	else
	{
		TimeT2BCDDtime( pstTransInfo->stPrevXferInfo.tEntExtDtime,
			pstTransTD->abPrevEntExtDtime );
	}

	// 47. 충전후카드잔액
	pstTransTD->dwBalAfterCharge = pstTransInfo->dwBalAfterCharge;

	// 48. 충전시카드거래건수
	pstTransTD->dwChargeTransCnt = pstTransInfo->dwChargeTransCnt;

	// 49. 충전금액
	pstTransTD->dwChargeAmt = pstTransInfo->dwChargeAmt;

	// 50. 충전기SAM ID
	ASC2BCD( pstTransInfo->abLSAMID, pstTransTD->abLSAMID,
		sizeof( pstTransInfo->abLSAMID ) );

	// 51. 충전기SAM거래일련번호
	pstTransTD->dwLSAMTransCnt = pstTransInfo->dwLSAMTransCnt;

	// 52. 충전거래유형
	pstTransTD->bChargeTransType = pstTransInfo->bChargeTransType;

	// 53. Record Mac
	dwCRC = MakeCRC32( ( byte * )pstTransTD, sizeof( TRANS_TD ) - 7 );
	memcpy( pstTransTD->abRecMAC, ( byte * )&dwCRC,
		sizeof( pstTransTD->abRecMAC ) );

	// 54. SIGN값검증결과
	switch ( pstTransInfo->bCardType )
	{
		case TRANS_CARDTYPE_MIF_PREPAY:
		case TRANS_CARDTYPE_MIF_POSTPAY:
		case TRANS_CARDTYPE_DEPOSIT:
			pstTransTD->abVerifySignVal = CHECK_SAM_SIGN_PASS;
			break;
		default:
			pstTransTD->abVerifySignVal = CHECK_SAM_SIGN_FAIL;
	}

	// 55. 개행문자
	pstTransTD->abCRLF[0] = 0x0D;
	pstTransTD->abCRLF[1] = 0x0A;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       SetCashTransData                                         *
*                                                                              *
*  DESCRIPTION:       사용자유형 정보를 이용하여 현금 거래내역의               *
*                     거래내역구조체를 조립한다.                               *
*                                                                              *
*  INPUT PARAMETERS:  pstTransTD - 설정하고자 하는 거래내역데이터 구조체 포인터*
*                     bUserType - 현금 사용자 유형                             *
*                                                                              *
*  RETURN/EXIT VALUE: void                                                     *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2006-03-14                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static void SetCashTransData( TRANS_TD *pstTransTD, byte bUserType )
{
	word wSeqNo = 0;
	dword dwCRC = 0;
	time_t tNowDtime = 0;
	byte abBuf[8] = {0, };
	byte abASCNowDtime[14] = {0, };
	byte abTempASCCardNo[20] = {0, };

	GetRTCTime( &tNowDtime );

	memset( ( byte * )pstTransTD, 0, sizeof( TRANS_TD ) );

	// 1. TD Record 길이
	memcpy( pstTransTD->abTDRecLen, LENGTH_OF_TRANS_DATA,
		sizeof( pstTransTD->abTDRecLen ) );

	// 2. TD Record 구분
	pstTransTD->bTDRecType = RECORD_TYPE_TRANS_DATA;

	// 3. 카드유형
	pstTransTD->bCardType = TRANS_CARDTYPE_CASH;

	// 4. 승하차유형
	pstTransTD->bEntExtType = '0';				// 승차

	// 5. 카드번호/일회권ID
	//     - 9B : 승차단말기ID
	//     - 6B : 현재시간 (hhmmss)
	//     - 5B : 일회권ID 일련번호
	memset( abTempASCCardNo, 0, sizeof( abTempASCCardNo ) );
	wSeqNo = GetCashTransSeqNo();
	TimeT2ASCDtime( tNowDtime, abASCNowDtime );
	memcpy( abTempASCCardNo, gpstSharedInfo->abMainTermID, 9 );
	memcpy( &abTempASCCardNo[9], &abASCNowDtime[8], 6 );
	WORD2ASCWithFillLeft0( wSeqNo, &abTempASCCardNo[15], 5 );
	ASC2BCD( abTempASCCardNo, pstTransTD->abCardNo, 20 );

	// 6. alias번호

	// 7. 승하차일시
	TimeT2BCDDtime( tNowDtime, pstTransTD->abEntExtDtime );

	// 8. 사용자유형
	pstTransTD->bUserType = bUserType;

	// 9. 승객1 할인할증유형ID
	CreateDisExtraTypeID( pstTransTD->bCardType, bUserType, tNowDtime, FALSE,
		pstTransTD->abDisExtraTypeID1 );

	// 10. 승객수1
	pstTransTD->wUserCnt1 = 1;

	// 11. 승객2 할인할증유형ID

	// 12. 승객수2

	// 13. 승객3 할인할증유형ID

	// 14. 승객수3

	// 15. 패널티유형

	// 16. 페널티요금

	// 17. 알고리즘유형

	// 18. 거래유형

	// 19. 개별거래수집키버전

	// 20. 전자화폐사ID

	// 21. 전자지갑ID

	// 22. 카드거래건수

	// 23. 카드잔액

	// 24. 요금
	pstTransTD->dwFare = GetBaseFare( pstTransTD->bCardType, bUserType,
		gstVehicleParm.wTranspMethodCode );

	// 25. SAM ID

	// 26. SAM거래카운터

	// 27. SAM총액거래수집카운터

	// 28. SAM개별거래수집건수

	// 29. SAM누적거래총액

	// 30. SAM서명

	// 31. 환승일련번호
	memcpy( pstTransTD->abXferSeqNo, "001", 3 );

	// 32. 승객1 이전거래시 지불된 최대기본요금

	// 33. 승객2 이전거래시 지불된 최대기본요금 - RFU

	// 34. 승객3 이전거래시 지불된 최대기본요금 - 거래로그

	// 35. 환승내이용수단기본요금의합

	// 36. 환승누적횟수

	// 37. 사용거리

	// 38. 환승내누적이동거리

	// 39. 환승내누적이용금액

	// 40. 총누적사용금액

	// 41. 정류장ID
	memset( abBuf, '0', sizeof( abBuf ) );
	memcpy( abBuf, gpstSharedInfo->abNowStationID,
		sizeof( gpstSharedInfo->abNowStationID ) );
	ASC2BCD( abBuf, pstTransTD->abStationID, 8 );

	// 42. 직전페널티요금

	// 43. 하차시승차금액

	// 44. 직전정류장ID

	// 45. 직전교통수단유형

	// 46. 직전승하차일시

	// 47. 충전후카드잔액

	// 48. 충전시카드거래건수

	// 49. 충전금액

	// 50. 충전기SAM ID

	// 51. 충전기SAM거래일련번호

	// 52. 충전거래유형

	// 53. Record Mac
	dwCRC = MakeCRC32( ( byte * )pstTransTD, sizeof( TRANS_TD ) - 7 );
	memcpy( pstTransTD->abRecMAC, ( byte * )&dwCRC,
		sizeof( pstTransTD->abRecMAC ) );

	// 54. SIGN값검증결과
	switch ( pstTransTD->bCardType )
	{
		case TRANS_CARDTYPE_MIF_PREPAY:
		case TRANS_CARDTYPE_MIF_POSTPAY:
		case TRANS_CARDTYPE_DEPOSIT:
		case TRANS_CARDTYPE_CASH:
		case TRANS_CARDTYPE_STUDENT_TOKEN:
			pstTransTD->abVerifySignVal = CHECK_SAM_SIGN_PASS;
			break;
		default:
			pstTransTD->abVerifySignVal = CHECK_SAM_SIGN_FAIL;
	}

	// 55. 개행문자
	pstTransTD->abCRLF[0] = 0x0D;
	pstTransTD->abCRLF[1] = 0x0A;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       SetTransTail                                             *
*                                                                              *
*  DESCRIPTION:       총건수 및 총금액, 운행종료일시 등의 정보를 이용하여      *
*                     거래내역테일을 설정한다.                                 *
*                                                                              *
*  INPUT PARAMETERS:  pstTransTT - 설정하고자 하는 거래내역테일 구조체 포인터  *
*                     dwTotalCnt - 총건수                                      *
*                     dwTotalAmt - 총금액                                      *
*                     tEndDriveDtime - 운행종료일시                            *
*                                                                              *
*  RETURN/EXIT VALUE: void                                                     *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2006-03-14                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static void SetTransTail( TRANS_TT *pstTransTT, dword dwTotalCnt,
	dword dwTotalAmt, time_t tEndDriveDtime )
{
	dword dwCRC = 0;
	byte abBuf[7] = {0, };
	byte abASCDtime[14] = {0, };

	memset( ( byte * )pstTransTT, 0, sizeof( TRANS_TT ) );

	TimeT2ASCDtime( tEndDriveDtime, abASCDtime );

	// 1. Record 길이
	memcpy( pstTransTT->abTTRecLen, LENGTH_OF_TRANS_TAIL,
		sizeof( pstTransTT->abTTRecLen ) );

	// 2. Record구분
	pstTransTT->bTTRecType = RECORD_TYPE_TRANS_TAIL;

	// 3. File Name
	memcpy( &pstTransTT->abFileName[0], "B001.0", 6 );		// 연계항목명
	memcpy( &pstTransTT->abFileName[6], ".", 1 );
	memcpy( &pstTransTT->abFileName[7], gstVehicleParm.abTranspBizrID,
		sizeof( gstVehicleParm.abTranspBizrID ) );
															// 교통사업자ID
	memcpy( &pstTransTT->abFileName[16], ".", 1 );
	memcpy( &pstTransTT->abFileName[17], gpstSharedInfo->abMainTermID,
		sizeof( gpstSharedInfo->abMainTermID ) );
															// 단말기ID
	memcpy( &pstTransTT->abFileName[26], ".", 1 );
	memcpy( &pstTransTT->abFileName[27], gpstSharedInfo->abTransFileName, 14 );
															// 생성일시
	memcpy( &pstTransTT->abFileName[41], ".", 1 );
	memcpy( &pstTransTT->abFileName[42], "C", 1 );			// 구분자
	memset( &pstTransTT->abFileName[43], 0, 48 );

	// 4. 총건수
	DWORD2ASCWithFillLeft0( dwTotalCnt, pstTransTT->abTotalCnt, 9 );

	// 5. 총금액
	DWORD2ASCWithFillLeft0( dwTotalAmt, pstTransTT->abTotalAmt, 10 );

	// 6. 운행 종료 일시
	memcpy( pstTransTT->abEndDtime, abASCDtime,
		sizeof( pstTransTT->abEndDtime ) );

	// 7. 회차역 ID
	// 정류장인식률 3자리
	WORD2ASCWithFillLeft0( gwGPSRecvRate, abBuf, 3 );
	memcpy( &pstTransTT->abReturnStationID[0], abBuf, 3 );
	// '카드를 다시 대주세요' 발생 횟수 4자리
	WORD2ASCWithFillLeft0( gwRetagCardCnt, abBuf, 4 );
	memcpy( &pstTransTT->abReturnStationID[3], abBuf, 4 );

	// 8. 회차시각
	memset( pstTransTT->abReturnDtime, '0',
		sizeof( pstTransTT->abEndStationID ) );
	// 운행횟수 3자리
	WORD2ASCWithFillLeft0( gwDriveCnt, abBuf, 3 );
	memcpy( &pstTransTT->abReturnDtime[0], abBuf, 3 );
	// 운행거리 7자리
	DWORD2ASCWithFillLeft0( gdwDistInDrive, abBuf, 7 );
	memcpy( &pstTransTT->abReturnDtime[3], abBuf, 7 );
	// 프로그램 버전 4자리
	memcpy( &pstTransTT->abReturnDtime[10], MAIN_RELEASE_VER, 4 );

	// 9. 종료정류장 ID
	memcpy( pstTransTT->abEndStationID, gpstSharedInfo->abNowStationID,
		sizeof( pstTransTT->abEndStationID ) );

	// 10. Record MAC
	dwCRC = MakeCRC32( ( byte * )pstTransTT, sizeof( TRANS_TT ) - 6 );
	memcpy( pstTransTT->abRecMAC, ( byte * )&dwCRC,
		sizeof( pstTransTT->abRecMAC ) );

	// 11. 개행문자
	pstTransTT->abCRLF[0] = 0x0D;
	pstTransTT->abCRLF[1] = 0x0A;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       FileRenameTransFile                                      *
*                                                                              *
*  DESCRIPTION:       거래내역파일명을 RENAME한다.                             *
*                                                                              *
*  INPUT PARAMETERS:  abOldTransFileName - RENAME하고자 하는 거래내역파일명    *
*                     abNewTransFileName - RENAME 결과 거래내역파일명          *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공                                      *
*                     ERR_CARD_PROC_FILE_RENAME_TRANS                          *
*                         - 거래내역파일명 RENAME 오류                         *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2006-03-14                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static short FileRenameTransFile( byte *abOldTransFileName,
	byte *abNewTransFileName )
{
	byte i = 0;
	int nResult = 0;

	for ( i = 0; i < 3; i++ )
	{
		/*
		 * rename의 리턴값 -> 0: 성공, -1: 실패
		 */
		nResult = rename( abOldTransFileName, abNewTransFileName );
		if ( nResult == 0 )
			break;
	}
	if ( nResult != 0 )
	{
		return ERR_CARD_PROC_FILE_RENAME_TRANS;
	}

	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       FileReadControlTrans                                     *
*                                                                              *
*  DESCRIPTION:       운행정보파일을 읽어 그 결과를 CONTROL_TRANS 구조체       *
*                     형식으로 리턴한다.                                       *
*                                                                              *
*  INPUT PARAMETERS:  pstControlTrans - 운행정보파일의 내용을 담아 리턴할      *
*                         CONTROL_TRANS 구조체의 포인터                        *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공                                      *
*                     ERR_CARD_PROC_FILE_OPEN_CONTROL_TRANS                    *
*                         - 운행정보파일 OPEN 오류                             *
*                     ERR_CARD_PROC_FILE_READ_CONTROL_TRANS                    *
*                         - 운행정보파일 READ 오류                             *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2006-03-14                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static short FileReadControlTrans( CONTROL_TRANS *pstControlTrans )
{
	short sResult = SUCCESS;
	int nResult = 0;
	FILE *fdFile = NULL;

	memset( ( byte * )pstControlTrans, 0, sizeof( CONTROL_TRANS ) );

	fdFile = fopen( CONTROL_TRANS_FILE, "rb+" );
	if ( fdFile == NULL )
	{
		sResult = ERR_CARD_PROC_FILE_OPEN_CONTROL_TRANS;
		goto FINALLY;
	}

	nResult = fread( ( byte * )pstControlTrans, sizeof( CONTROL_TRANS ), 1,
		fdFile );
	if ( nResult != 1 )
	{
		sResult = ERR_CARD_PROC_FILE_READ_CONTROL_TRANS;
		goto FINALLY;
	}

	FINALLY:

	if ( fdFile != NULL )
	{
		fclose( fdFile );
	}

	return sResult;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       FileReadTransHeader                                      *
*                                                                              *
*  DESCRIPTION:       거래내역파일로부터 거래내역헤더를 읽어 TRANS_TH 구조체
                      형식으로 리턴한다.                                       *
*                                                                              *
*  INPUT PARAMETERS:  pstTransTH - 거래내역파일로부터 READ한 TRNAS_TH 구조체   *
*                     abTransFileName - 거래내역파일명                         *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공                                      *
*                     ERR_CARD_PROC_FILE_OPEN_TRANS                            *
*                         - 거래내역파일 OPEN 오류                             *
*                     ERR_CARD_PROC_FILE_READ_TRANS                            *
*                         - 거래내역파일 READ 오류                             *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2006-03-14                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static short FileReadTransHeader( TRANS_TH *pstTransTH, byte *abTransFileName )
{
	short sResult = SUCCESS;
	int nResult = 0;
	FILE *fdFile = NULL;

	fdFile = fopen( abTransFileName, "rb" );
	if ( fdFile == NULL )
	{
		sResult = ERR_CARD_PROC_FILE_OPEN_TRANS;
		goto FINALLY;
	}

	nResult = fread( ( byte * )pstTransTH, sizeof( TRANS_TH ), 1, fdFile );
	if ( nResult != 1 )
	{
		sResult = ERR_CARD_PROC_FILE_READ_TRANS;
		goto FINALLY;
	}

	FINALLY:

	if ( fdFile != NULL )
	{
		fclose( fdFile );
	}

	return sResult;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       FileReadTransDataWithFD                                  *
*                                                                              *
*  DESCRIPTION:       거래내역파일로부터 인덱스에 해당하는                     *
*                     거래내역데이터를 읽어 TRANS_TD 구조체 형식으로 리턴한다. *
*                                                                              *
*  INPUT PARAMETERS:  fdFile - 거래내역파일의 파일디스크립터                   *
*                     pstTransTD - 거래내역파일로부터 READ한 TRNAS_TD 구조체   *
*                     dwIndex - READ하고자 하는 거래내역데이터의 인덱스        *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공                                      *
*                     ERR_CARD_PROC_FILE_READ_TRANS                            *
*                         - 거래내역파일 READ 오류                             *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2006-03-14                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static short FileReadTransDataWithFD( FILE *fdFile, TRANS_TD *pstTransTD,
	dword dwIndex )
{
	int nResult = 0;

	fseek( fdFile, ( sizeof( TRANS_TH ) + sizeof( TRANS_TD ) * dwIndex ),
		SEEK_SET );

	nResult = fread( ( byte * )pstTransTD, sizeof( TRANS_TD ), 1, fdFile );
	if ( nResult != 1 )
	{
		return ERR_CARD_PROC_FILE_READ_TRANS;
	}

	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       FileReadTransDataWithoutTransHeaderWithFD                *
*                                                                              *
*  DESCRIPTION:       거래내역헤더가 없는 거래내역파일로부터 인덱스에 해당하는 *
*                     거래내역데이터를 읽어 TRANS_TD 구조체 형식으로 리턴한다. *
*                                                                              *
*  INPUT PARAMETERS:  fdFile                                                   *
*                         - 거래내역헤더가 없는 거래내역파일의 파일디스크립터  *
*                     pstTransTD - 거래내역파일로부터 READ한 TRNAS_TD 구조체   *
*                     dwIndex - READ하고자 하는 거래내역데이터의 인덱스        *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공                                      *
*                     ERR_CARD_PROC_FILE_READ_TRANS                            *
*                         - 거래내역파일 READ 오류                             *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2006-03-14                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static short FileReadTransDataWithoutTransHeaderWithFD( FILE *fdFile,
	TRANS_TD *pstTransTD, dword dwIndex )
{
	int nResult = 0;

	fseek( fdFile, ( sizeof( TRANS_TD ) * dwIndex ), SEEK_SET );

	nResult = fread( ( byte * )pstTransTD, sizeof( TRANS_TD ), 1, fdFile );
	if ( nResult != 1 )
	{
		return ERR_CARD_PROC_FILE_READ_TRANS;
	}

	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       FileReadTransTail                                        *
*                                                                              *
*  DESCRIPTION:       거래내역파일에서 거래내역테일을 읽어 TRANS_TT 구조체     *
*                     형식으로 리턴한다.                                       *
*                                                                              *
*  INPUT PARAMETERS:  pstTransTT - 거래내역파일로부터 READ한 TRANS_TT 구조체   *
*                     abTransFileName - 거래내역파일명                         *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공                                      *
*                     ERR_CARD_PROC_FILE_OPEN_TRANS                            *
*                         - 거래내역파일 OPEN 오류                             *
*                     ERR_CARD_PROC_FILE_READ_TRANS                            *
*                         - 거래내역파일 READ 오류                             *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2006-03-14                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static short FileReadTransTail( TRANS_TT *pstTransTT, byte *abTransFileName )
{
	short sResult = SUCCESS;
	int nResult = 0;
	FILE *fdFile = NULL;

	fdFile = fopen( abTransFileName, "rb" );
	if ( fdFile == NULL )
	{
		sResult = ERR_CARD_PROC_FILE_OPEN_TRANS;
		goto FINALLY;
	}

	fseek( fdFile, ( -1 * sizeof( TRANS_TT ) ), SEEK_END );

	nResult = fread( ( byte * )pstTransTT, sizeof( TRANS_TT ), 1, fdFile );
	if ( nResult != 1 )
	{
		sResult = ERR_CARD_PROC_FILE_READ_TRANS;
		goto FINALLY;
	}

	FINALLY:

	if ( fdFile != NULL )
	{
		fclose( fdFile );
	}

	return sResult;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       FileWriteControlTrans                                    *
*                                                                              *
*  DESCRIPTION:       운행정보파일에 CONTROL_TRANS 구조체의 내용을 신규로      *
*                     WRITE한다. (존재한다면) 기존의 운행정보파일은 삭제된다.  *
*                                                                              *
*  INPUT PARAMETERS:  pstControlTrans - 운행정보파일에 WRITE할 CONTROL_TRANS   *
*                         구조체                                               *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공                                      *
*                     ERR_CARD_PROC_FILE_OPEN_CONTROL_TRANS                    *
*                         - 운행정보파일 OPEN 오류                             *
*                     ERR_CARD_PROC_FILE_WRITE_CONTROL_TRANS                   *
*                         - 운행정보파일 WRITE 오류                            *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2006-03-14                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static short FileWriteControlTrans( CONTROL_TRANS *pstControlTrans )
{
	short sResult = SUCCESS;
	int nResult = 0;
	FILE *fdFile = NULL;

	fdFile = fopen( CONTROL_TRANS_FILE, "wb" );
	if ( fdFile == NULL )
	{
		sResult = ERR_CARD_PROC_FILE_OPEN_CONTROL_TRANS;
		goto FINALLY;
	}

	nResult = fwrite( ( byte * )pstControlTrans, sizeof( CONTROL_TRANS ), 1,
		fdFile );
	if ( nResult != 1 )
	{
		sResult = ERR_CARD_PROC_FILE_WRITE_CONTROL_TRANS;
		goto FINALLY;
	}

	nResult = fflush( fdFile );
	if ( nResult != SUCCESS )
	{
		sResult = ERR_CARD_PROC_FILE_WRITE_CONTROL_TRANS;
		goto FINALLY;
	}

	FINALLY:

	if ( fdFile != NULL )
	{
		fclose( fdFile );
	}

	return sResult;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       FileWriteTransHeader                                     *
*                                                                              *
*  DESCRIPTION:       거래내역헤더를 주어진 이름의 거래내역파일에 기록한다.    *
*                     단, 파일은 신규로 생성된다.                              *
*                     append한다.                                              *
*                                                                              *
*  INPUT PARAMETERS:  pstTransTH - 거래내역헤더                                *
*                     abTransFileName - 거래내역파일명                         *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공                                      *
*                     ERR_CARD_PROC_FILE_OPEN_TRANS                            *
*                         - 거래내역파일 OPEN 오류                             *
*                     ERR_CARD_PROC_FILE_WRITE_TRANS                           *
*                         - 거래내역파일 WRITE 오류                            *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2006-03-14                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static short FileWriteTransHeader( TRANS_TH *pstTransTH, byte *abTransFileName )
{
	short sResult = SUCCESS;
	int nResult = 0;
	FILE *fdFile = NULL;

	fdFile = fopen( abTransFileName, "wb" );
	if ( fdFile == NULL )
	{
		sResult = ERR_CARD_PROC_FILE_OPEN_TRANS;
		goto FINALLY;
	}

	nResult = fwrite( ( byte * )pstTransTH, sizeof( TRANS_TH ), 1, fdFile );
	if ( nResult != 1 )
	{
		sResult = ERR_CARD_PROC_FILE_WRITE_TRANS;
		goto FINALLY;
	}

	nResult = fflush( fdFile );
	if ( nResult != SUCCESS )
	{
		sResult = ERR_CARD_PROC_FILE_WRITE_TRANS;
		goto FINALLY;
	}

	FINALLY:

	if ( fdFile != NULL )
	{
		fclose( fdFile );
	}

	return sResult;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       FileAppendTransData                                      *
*                                                                              *
*  DESCRIPTION:       거래내역데이터를 주어진 이름의 거래내역파일에            *
*                     append한다.                                              *
*                                                                              *
*  INPUT PARAMETERS:  pstTransTD - 거래내역데이터                              *
*                     abTransFileName - 거래내역파일명                         *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공                                      *
*                     ERR_CARD_PROC_FILE_OPEN_TRANS                            *
*                         - 거래내역파일 OPEN 오류                             *
*                     ERR_CARD_PROC_FILE_WRITE_TRANS                           *
*                         - 거래내역파일 WRITE 오류                            *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2006-03-14                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static short FileAppendTransData( TRANS_TD *pstTransTD, byte *abTransFileName )
{
	short sResult = SUCCESS;
	int nResult = 0;
	FILE *fdFile = NULL;

	fdFile = fopen( abTransFileName, "ab" );
	if ( fdFile == NULL )
	{
		sResult = ERR_CARD_PROC_FILE_OPEN_TRANS;
		goto FINALLY;
	}

	nResult = fwrite( ( byte * )pstTransTD, sizeof( TRANS_TD ), 1, fdFile );
	if ( nResult != 1 )
	{
		sResult = ERR_CARD_PROC_FILE_WRITE_TRANS;
		goto FINALLY;
	}

	nResult = fflush( fdFile );
	if ( nResult != SUCCESS )
	{
		sResult = ERR_CARD_PROC_FILE_WRITE_TRANS;
		goto FINALLY;
	}

	FINALLY:

	if ( fdFile != NULL )
	{
		fclose( fdFile );
	}

	return sResult;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       FileAppendTransDataWithFD                                *
*                                                                              *
*  DESCRIPTION:       거래내역데이터를 주어진 파일디스크립터에 기록한다. 단,   *
*                     파일 포인터의 이동은 하지 않으므로 함수명처럼 append를   *
*                     보장하기 위해서는 파일포인터가 파일의 가장 마지막에      *
*                     있다는 조건이 만족되어야 한다.                           *
*                                                                              *
*  INPUT PARAMETERS:  fdFile - 거래내역파일의 파일디스크립터                   *
*                     pstTransTD - 거래내역데이터                              *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공                                      *
*                     ERR_CARD_PROC_FILE_WRITE_TRANS                           *
*                         - 거래내역파일 WRITE 오류                            *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2006-03-14                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static short FileAppendTransDataWithFD( FILE *fdFile, TRANS_TD *pstTransTD )
{
	int nResult = 0;

	nResult = fwrite( ( byte * )pstTransTD, sizeof( TRANS_TD ), 1, fdFile );
	if ( nResult != 1 )
	{
		return ERR_CARD_PROC_FILE_WRITE_TRANS;
	}

	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       FileAppendSubTransData                                   *
*                                                                              *
*  DESCRIPTION:       203B로 구성된 거래내역데이터 (거래내역데이터 202B +      *
*                     전송여부 1B)를 거래내역파일에 APPEND한다.                *
*                                                                              *
*  INPUT PARAMETERS:  abSubTransTD - 거래내역파일에 APPEND할                   *
*                         거래내역데이터 (203B 바이트 배열)                    *
*                     abTransFileName - 거래내역파일명                         *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공                                      *
*                     ERR_CARD_PROC_FILE_OPEN_SUB_TRANS                        *
*                         - 하차거래내역파일 OPEN 오류                         *
*                     ERR_CARD_PROC_FILE_WRITE_SUB_TRANS                       *
*                         - 하차거래내역파일 WRITE 오류                        *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2006-03-14                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static short FileAppendSubTransData( byte *abSubTransTD, byte *abTransFileName )
{
	short sResult = SUCCESS;
	int nResult = 0;
	FILE *fdFile = NULL;

	fdFile = fopen( abTransFileName, "ab" );
	if ( fdFile == NULL )
	{
		sResult = ERR_CARD_PROC_FILE_OPEN_SUB_TRANS;
		goto FINALLY;
	}

	nResult = fwrite( abSubTransTD, sizeof( TRANS_TD ) + 1, 1, fdFile );
	if ( nResult != 1 )
	{
		sResult = ERR_CARD_PROC_FILE_WRITE_SUB_TRANS;
		goto FINALLY;
	}

	nResult = fflush( fdFile );
	if ( nResult != SUCCESS )
	{
		sResult = ERR_CARD_PROC_FILE_WRITE_SUB_TRANS;
		goto FINALLY;
	}

	FINALLY:

	if ( fdFile != NULL )
	{
		fclose( fdFile );
	}

	return sResult;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       FileAppendTransTail                                      *
*                                                                              *
*  DESCRIPTION:       거래내역파일에 거래내역테일을 APPEND한다.                *
*                                                                              *
*  INPUT PARAMETERS:  pstTransTT - 거래내역파일에 APPEND할 거래내역테일구조체  *
*                     abTransFileName - 거래내역파일명                         *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공                                      *
*                     ERR_CARD_PROC_FILE_OPEN_TRANS                            *
*                         - 거래내역파일 OPEN 오류                             *
*                     ERR_CARD_PROC_FILE_WRITE_TRANS                           *
*                         - 거래내역파일 WRITE 오류                            *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2006-03-14                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static short FileAppendTransTail( TRANS_TT *pstTransTT, byte *abTransFileName )
{
	short sResult = SUCCESS;
	int nResult = 0;
	FILE *fdFile = NULL;

	fdFile = fopen( abTransFileName, "ab" );
	if ( fdFile == NULL )
	{
		sResult = ERR_CARD_PROC_FILE_OPEN_TRANS;
		goto FINALLY;
	}

	nResult = fwrite( ( byte * )pstTransTT, sizeof( TRANS_TT ), 1, fdFile );
	if ( nResult != 1 )
	{
		sResult = ERR_CARD_PROC_FILE_WRITE_TRANS;
		goto FINALLY;
	}

	nResult = fflush( fdFile );
	if ( nResult != SUCCESS )
	{
		sResult = ERR_CARD_PROC_FILE_WRITE_TRANS;
		goto FINALLY;
	}

	FINALLY:

	if ( fdFile != NULL )
	{
		fclose( fdFile );
	}

	return sResult;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       FileUpdateControlTrans                                   *
*                                                                              *
*  DESCRIPTION:       운행정보파일을 읽어 총건수 및 총금액을 업데이트한 후     *
*                     다시 WRITE한다.                                          *
*                                                                              *
*  INPUT PARAMETERS:  dwFare - 총금액에 더할 금액                              *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공                                      *
*                     ERR_CARD_PROC_FILE_OPEN_CONTROL_TRANS                    *
*                         - 운행정보파일 OPEN 오류                             *
*                     ERR_CARD_PROC_FILE_READ_CONTROL_TRANS                    *
*                         - 운행정보파일 READ 오류                             *
*                     ERR_CARD_PROC_FILE_WRITE_CONTROL_TRANS                   *
*                         - 운행정보파일 WRITE 오류                            *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2006-03-14                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static short FileUpdateControlTrans( dword dwFare )
{
	short sResult = SUCCESS;
	int nResult = 0;
	dword dwTotalCnt = 0;
	dword dwTotalAmt = 0;
	FILE *fdFile = NULL;
	CONTROL_TRANS stControlTrans;

	fdFile = fopen( CONTROL_TRANS_FILE, "rb+" );
	if ( fdFile == NULL )
	{
		sResult = ERR_CARD_PROC_FILE_OPEN_CONTROL_TRANS;
		goto FINALLY;
	}

	nResult = fread( ( byte * )&stControlTrans, sizeof( CONTROL_TRANS ), 1,
		fdFile );
	if ( nResult != 1 )
	{
		sResult = ERR_CARD_PROC_FILE_READ_CONTROL_TRANS;
		goto FINALLY;
	}

	dwTotalCnt = GetDWORDFromASC( stControlTrans.abTotalCnt,
		sizeof( stControlTrans.abTotalCnt ) );
	dwTotalAmt = GetDWORDFromASC( stControlTrans.abTotalAmt,
		sizeof( stControlTrans.abTotalAmt ) );

	dwTotalCnt++;
	dwTotalAmt += dwFare;

	DWORD2ASCWithFillLeft0( dwTotalCnt, stControlTrans.abTotalCnt,
		sizeof( stControlTrans.abTotalCnt ) );
	DWORD2ASCWithFillLeft0( dwTotalAmt, stControlTrans.abTotalAmt,
		sizeof( stControlTrans.abTotalAmt ) );

	fseek( fdFile, 0 ,SEEK_SET );

	nResult = fwrite( ( byte * )&stControlTrans, sizeof( CONTROL_TRANS ), 1,
		fdFile );
	if ( nResult != 1 )
	{
		sResult = ERR_CARD_PROC_FILE_WRITE_CONTROL_TRANS;
		goto FINALLY;
	}

	nResult = fflush( fdFile );
	if ( nResult != SUCCESS )
	{
		sResult = ERR_CARD_PROC_FILE_WRITE_CONTROL_TRANS;
		goto FINALLY;
	}

	FINALLY:

	if ( fdFile != NULL )
	{
		fclose( fdFile );
	}

	return sResult;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       FileUpdateTransHeaderAndAppendTransTail                  *
*                                                                              *
*  DESCRIPTION:       거래내역파일의 헤더에 거래내역파일명을 업데이트한 후,    *
*                     거래내역파일에 테일의 내용을 APPEND한다.                 *
*                                                                              *
*  INPUT PARAMETERS:  pstTransTT - 거래내역에 APPEND할 거래내역 테일의 내용을  *
*                         담고있는 TRANS_TT 구조체                             *
*                     abTransFileName - 거래내역파일명                         *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공                                      *
*                     ERR_CARD_PROC_FILE_OPEN_TRANS                            *
*                         - 거래내역파일 OPEN 오류                             *
*                     ERR_CARD_PROC_FILE_READ_TRANS                            *
*                         - 거래내역파일 READ 오류                             *
*                     ERR_CARD_PROC_FILE_WRITE_TRANS                           *
*                         - 거래내역파일 WRITE 오류                            *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2006-03-14                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static short FileUpdateTransHeaderAndAppendTransTail( TRANS_TT *pstTransTT,
	byte *abTransFileName )
{
	short sResult = SUCCESS;
	int nResult = 0;
	byte i = 0;
	dword dwCRC = 0;
	FILE *fdFile = NULL;
	TRANS_TH stTransTH;

	/*
	 * 거래내역파일 OPEN
	 */
	fdFile = fopen( abTransFileName, "rb+" );
	if ( fdFile == NULL )
	{
		sResult = ERR_CARD_PROC_FILE_OPEN_TRANS;
		goto FINALLY;
	}

	fseek( fdFile, 0 ,SEEK_SET );

	nResult = fread( ( byte * )&stTransTH, sizeof( TRANS_TH ), 1, fdFile );
	if ( nResult != 1 )
	{
		sResult = ERR_CARD_PROC_FILE_READ_TRANS;
		goto FINALLY;
	}

	/*
	 * 거래내역헤더에 거래내역파일명 복사 및 Record Mac 생성
	 */
	memcpy( stTransTH.abFileName, pstTransTT->abFileName,
		sizeof( stTransTH.abFileName ) );
	dwCRC = MakeCRC32( ( byte * )&stTransTH, sizeof( TRANS_TH ) - 6 );
	memcpy( stTransTH.abRecMAC, ( byte * )&dwCRC,
		sizeof( stTransTH.abRecMAC ) );

	/*
	 * 거래내역헤더 WRITE
	 */
	fseek( fdFile, 0 ,SEEK_SET );

	for ( i = 0; i < 3; i++ )
	{
		nResult = fwrite( ( byte * )&stTransTH, sizeof( TRANS_TH ), 1, fdFile );
		if ( nResult == 1 )
			break;
	}
	if ( nResult != 1 )
	{
		sResult = ERR_CARD_PROC_FILE_WRITE_TRANS;
		goto FINALLY;
	}

	/*
	 * 거래내역테일 APPEND
	 */
	fseek( fdFile, 0 ,SEEK_END );

	for ( i = 0; i < 3; i++ )
	{
		nResult = fwrite( ( byte * )pstTransTT, sizeof( TRANS_TT ), 1, fdFile );
		if ( nResult == 1 )
			break;
	}
	if ( nResult != 1 )
	{
		sResult = ERR_CARD_PROC_FILE_WRITE_TRANS;
		goto FINALLY;
	}

	nResult = fflush( fdFile );
	if ( nResult != SUCCESS )
	{
		sResult = ERR_CARD_PROC_FILE_WRITE_TRANS;
		goto FINALLY;
	}

	FINALLY:

	if ( fdFile != NULL )
	{
		fclose( fdFile );
	}

	return sResult;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       IsExistTransDataWithFD                                   *
*                                                                              *
*  DESCRIPTION:       거래내역헤더가 존재하는 거래내역파일에 입력한            *
*                     거래내역데이터가 존재하는지의 여부를 TRUE/FALSE로        *
*                     리턴한다. 단, 입력된 파일디스크립터의 파일포인터 위치는  *
*                     초기화하지 않으므로 주의한다.                            *
*                                                                              *
*  INPUT PARAMETERS:  fdFile - 거래내역헤더가 존재하는 거래내역파일의          *
*                         파일디스크립터                                       *
*                     pstTransTD - 거래내역파일에 존재하는지의 여부를 확인할   *
*                         거래내역데이터 구조체의 포인터                       *
*                     dwTotCnt - 거래내역파일의 거래내역데이터 개수            *
*                                                                              *
*  RETURN/EXIT VALUE: TRUE - 거래내역파일에 거래내역데이터가 존재              *
*                     FALSE - 거래내역파일에 거래내역데이터가 미존재           *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-03-15                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static bool IsExistTransDataWithFD( FILE *fdFile, TRANS_TD *pstTransTD,
	dword dwTotCnt )
{
	int nResult = 0;
	dword i = 0;
	TRANS_TD stTransTD;

	/*
	 * 거래내역파일의 거래내역데이터 개수가 0이면 무조건 FALSE 리턴
	 */
	if ( dwTotCnt == 0 )
	{
		return FALSE;
	}

	/*
	 * 거래내역데이터 개수만큼 반복
	 */
	for ( i = 0; i < dwTotCnt; i++ )
	{
		fseek( fdFile, ( sizeof( TRANS_TH ) + sizeof( TRANS_TD ) * i ),
			SEEK_SET );

		nResult = fread( ( byte * )&stTransTD, sizeof( TRANS_TD ), 1, fdFile );
		if ( nResult != 1 )
		{
			return FALSE;
		}

		/*
		 * 거래내역파일로부터 READ한 거래내역데이터와 파라미터로 입력된
		 * 거래내역데이터가 동일하면 TRUE 리턴
		 */
		if ( memcmp( ( byte * )&stTransTD, ( byte * )pstTransTD,
				sizeof( TRANS_TD ) ) == 0 )
		{
			return TRUE;
		}
	}

	/*
	 * 동일한 거래내역데이터를 발견하지 못한 경우 FALSE 리턴
	 */
	return FALSE;
}

