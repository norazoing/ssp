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
*  PROGRAM ID :       load_parameter_file_mgt.c                                *
*                                                                              *
*  DESCRIPTION:       이 프로그램은 파라미터 파일과 집계와 통신하기위한  	   *
*                       정보를 로드한다.                                       *
*                                                                              *
*  ENTRY POINT:     void InitOperParmInfo( void );							   *
*					short LoadOperParmInfo( void );							   *
*					short LoadVehicleParm( void );							   *
*					short LoadRouteParm( ROUTE_PARM *pstRouteParmInfo );	   *
*					short LoadInstallInfo( void );							   *
*					short GetLEAPPasswd( void );                               *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: None                                                     *
*                                                                              *
*  INPUT FILES:         c_op_par.dat                                           *
*                       c_li_par.dat                                           *
*                       c_ap_inf.dat                                           *
*                       c_dp_inf.dat                                           *
*                       c_cd_inf.dat                                           *
*                       c_tr_inf.dat                                           *
*                       c_de_inf.dat                                           *
*                       c_ho_inf.dat                                           *
*                       c_n_far.dat                                            *
*                       c_st_inf.dat                                           *
*                       setup.dat                                              *
*                       setup.backup                                           *
*                       tc_leap.dat                                            *
*                       tc_leap.backup                                         *
*                                                                              *
*  OUTPUT FILES:                                                               *
*                                                                              *
*  SPECIAL LOGIC:     None                                                     *
*                                                                              *
********************************************************************************
*                         MODIFICATION LOG                                     *
*                                                                              *
*    DATE                SE NAME                      DESCRIPTION              *
* ---------- ---------------------------- ------------------------------------ *
* 2005/09/03 Solution Team Mi Hyun Noh  Initial Release                        *
*                                                                              *
*******************************************************************************/

/*******************************************************************************
*  Inclusion of System Header Files                                            *
*******************************************************************************/
#include "load_parameter_file_mgt.h"
#include "trans_proc.h"

/*******************************************************************************
*  Declaration of variables                                                    *
*******************************************************************************/
COMM_INFO                       gstCommInfo;

#define     NEGATIVE_CLASS      'M'     // 할인할증에서 -
#define     IP_CLASS_LENGTH     3       // 한 클래스를 구성하는 길이

int  nTermIDLength = sizeof( gpstSharedInfo->abMainTermID );


/*******************************************************************************
*  Declaration of Extern variables                                             *
*******************************************************************************/
ROUTE_PARM                      gstRouteParm;	// 노선별파라미터 구조체
VEHICLE_PARM                    gstVehicleParm;	// 운행차량 파라미터 구조체
PREPAY_ISSUER_INFO_HEADER		gstPrepayIssuerInfoHeader;
PREPAY_ISSUER_INFO          	*gpstPrepayIssuerInfo = NULL;
POSTPAY_ISSUER_INFO_HEADER      gstPostpayIssuerInfoHeader;
POSTPAY_ISSUER_INFO             *gpstPostpayIssuerInfo = NULL;
ISSUER_VALID_PERIOD_INFO_HEADER	gstIssuerValidPeriodInfoHeader;
ISSUER_VALID_PERIOD_INFO		*gpstIssuerValidPeriodInfo = NULL;
XFER_APPLY_INFO_HEADER          gstXferApplyInfoHeader;
XFER_APPLY_INFO                 *gpstXferApplyInfo = NULL;
DIS_EXTRA_INFO_HEADER           gstDisExtraInfoHeader;
DIS_EXTRA_INFO                  *gpstDisExtraInfo = NULL;
HOLIDAY_INFO_HEADER             gstHolidayInfoHeader;
HOLIDAY_INFO                    *gpstHolidayInfo = NULL;
NEW_FARE_INFO                   gstNewFareInfo;
STATION_INFO_HEADER             gstStationInfoHeader;
STATION_INFO                    *gpstStationInfo = NULL;

byte    gabDCSIPAddr[16] 	= { 0, };     	// DCS server IP (xxx.xxx.xxx.xxx)
byte    gabLEAPPasswd[21]	= { 0, };		// LEAP password
byte    gabEndStationID[7] 	= { 0, };    	// 종점정류장ID

TEMP_STATION_INFO_HEADER        stTmpStationInfoHeader;
TEMP_STATION_INFO               stTmpStationInfo;

/*******************************************************************************
*  Declaration of Module Functions                                             *
*******************************************************************************/
static short LoadPrepayIssuerInfo( void );
static short LoadPostpayIssuerInfo( void );
static short LoadIssuerValidPeriodInfo( void );
static short LoadXferApplyInfo( void );
static short LoadDisExtraInfo( void );
static short LoadHolidayInfo( void );
static short LoadNewFareInfo( void );
static short LoadHardCodedNewFareInfo( void );

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      InitOperParmInfo                                         *
*                                                                              *
*  DESCRIPTION :      각종 파라미터 정보를 초기화 한다.					       *
*                                                                              *
*  INPUT PARAMETERS:  void                                                     *
*                                                                              *
*  RETURN/EXIT VALUE:     void                                                 *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-11-09                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
void InitOperParmInfo( void )
{

    memset( &gstPrepayIssuerInfoHeader,
            0,
            sizeof(gstPrepayIssuerInfoHeader));
    memset( &gstPostpayIssuerInfoHeader,
            0,
            sizeof(gstPostpayIssuerInfoHeader));
    memset( &gstIssuerValidPeriodInfoHeader,
            0,
            sizeof(gstIssuerValidPeriodInfoHeader));
    memset( &gstXferApplyInfoHeader,
            0,
            sizeof(gstXferApplyInfoHeader));
    memset( &gstDisExtraInfoHeader,
            0,
            sizeof(gstDisExtraInfoHeader));
    memset( &gstHolidayInfoHeader,
            0,
            sizeof(gstHolidayInfoHeader));
    memset( &gstNewFareInfo,
            0,
            sizeof(gstNewFareInfo));
    memset( &gstStationInfoHeader,
            0,
            sizeof(gstStationInfoHeader));
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      LoadOperParmInfo                                         *
*                                                                              *
*  DESCRIPTION :      각 파라미터 파일을 로드 한다.							   *
*                                                                              *
*  INPUT PARAMETERS:  void                                                     *
*                                                                              *
*  RETURN/EXIT VALUE:     SUCCESS                                              *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short LoadOperParmInfo( void )
{
    short sResult = SUCCESS;
    short sRetVal = SUCCESS;
	byte i = 0;

    /*
     * 차량정보 로드
     */
	for ( i = 0; i < 3; i++ )
	{
	    sResult  = LoadVehicleParm();
		if ( sResult == SUCCESS )
		{
			break;
		}
	}
	if ( sResult != SUCCESS )
	{
        /*
         *  차량정보 로드 실패 이벤트 남김
         */
        ctrl_event_info_write( EVENT_LOAD_ERR_VEHICLE_INFO );

		sRetVal = ERR_LOAD_PARM;
	}

    /*
     * 노선정보 로드
     */
	for ( i = 0; i < 3; i++ )
	{
	    sResult  = LoadRouteParm();
		if ( sResult == SUCCESS )
		{
			break;
		}
	}
	if ( sResult != SUCCESS )
	{
        /*
         *  노선정보 로드 실패 이벤트 남김
         */
        ctrl_event_info_write( EVENT_LOAD_ERR_ROUTE_INFO );

		sRetVal = ERR_LOAD_PARM;
	}

    /*
     * 신요금정보 로드
     */
	for ( i = 0; i < 3; i++ )
	{
	    sResult  = LoadNewFareInfo();
		if ( sResult == SUCCESS )
		{
			break;
		}
	}
    if ( sResult != SUCCESS )
    {
        /*
         *  신요금정보 로드 실패 이벤트 남김
         */
        ctrl_event_info_write( EVENT_LOAD_ERR_NEW_FARE_INFO );

		/*
		 * 하드코딩된 신요금정보를 로드
		 */
		LoadHardCodedNewFareInfo();

        sRetVal = ERR_LOAD_PARM;
    }

    /*
     * 정류장정보 로드
     */
	for ( i = 0; i < 3; i++ )
	{
	    sResult  = LoadStationInfo();
		if ( sResult == SUCCESS )
		{
			break;
		}
	}
	if ( sResult != SUCCESS )
	{
        /*
         *  정류장정보 로드 실패 이벤트 남김
         */
        ctrl_event_info_write( EVENT_LOAD_ERR_STATION_INFO );

		sRetVal = ERR_LOAD_PARM;
	}

    /*
     * 선불발행사정보 로드
     */
	for ( i = 0; i < 3; i++ )
	{
	    sResult  = LoadPrepayIssuerInfo();
		if ( sResult == SUCCESS )
		{
			break;
		}
	}
	if ( sResult != SUCCESS )
	{
        /*
         *  선불발행사정보 로드 실패 이벤트 남김
         */
        ctrl_event_info_write( EVENT_LOAD_ERR_PREPAY_ISSUER_INFO );

		sRetVal = ERR_LOAD_PARM;
	}

    /*
     * 후불발행사정보 로드
     */
	for ( i = 0; i < 3; i++ )
	{
	    sResult  = LoadPostpayIssuerInfo();
		if ( sResult == SUCCESS )
		{
			break;
		}
	}
	if ( sResult != SUCCESS )
	{
        /*
         *  후불발행사정보 로드 실패 이벤트 남김
         */
        ctrl_event_info_write( EVENT_LOAD_ERR_POSTPAY_ISSUER_INFO );

		sRetVal = ERR_LOAD_PARM;
	}

    /*
     * 발행사유효기간정보 로드
     */
	for ( i = 0; i < 3; i++ )
	{
	    sResult  = LoadIssuerValidPeriodInfo();
		if ( sResult == SUCCESS )
		{
			break;
		}
	}
	if ( sResult != SUCCESS )
	{
        /*
         *  발행사유효기간정보 로드 실패 이벤트 남김
         */
        ctrl_event_info_write( EVENT_LOAD_ERR_ISSUER_VALID_PERIOD_INFO );

		sRetVal = ERR_LOAD_PARM;
	}

    /*
     * 할인할증정보 로드
     */
	for ( i = 0; i < 3; i++ )
	{
	    sResult  = LoadDisExtraInfo();
		if ( sResult == SUCCESS )
		{
			break;
		}
	}
    if ( sResult != SUCCESS )
    {
        /*
         *  할인할증정보 로드 실패 이벤트 남김
         */
        ctrl_event_info_write( EVENT_LOAD_ERR_DIS_EXTRA_INFO );

        sRetVal = ERR_LOAD_PARM;
    }

    /*
     * 환승정보 로드
     */
	for ( i = 0; i < 3; i++ )
	{
	    sResult  = LoadHolidayInfo();
		if ( sResult == SUCCESS )
		{
			break;
		}
	}
    if ( sResult != SUCCESS )
	{
		/*
		 *  휴일정보 로드 실패 이벤트 남김
		 */
	    ctrl_event_info_write( EVENT_LOAD_ERR_HOLIDAY_INFO );

		sRetVal = ERR_LOAD_PARM;
	}

	/*
	 * 환승적용정보 로드
	 */
	for ( i = 0; i < 3; i++ )
	{
	    sResult  = LoadXferApplyInfo();
		if ( sResult == SUCCESS )
		{
			break;
		}
	}
	if ( sResult != SUCCESS )
	{
        /*
         *  환승적용정보 로드 실패 이벤트 남김
         */
        ctrl_event_info_write( EVENT_LOAD_ERR_XFER_APPLY_INFO );

		sRetVal = ERR_LOAD_PARM;
	}

	printf( "[LoadInfo] 모든 운영정보파일 로드 완료 ----------------------------------------\n" );

	return sRetVal;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      LoadVehicleParm                                          *
*                                                                              *
*  DESCRIPTION :      운행차량 파라미터 파일을 로드 한다.			               *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       ERR_FILE_OPEN | GetFileNo( VEHICLE_PARM_FILE )         *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short LoadVehicleParm( void )
{
	int nReadSize = 0;
	FILE *fdVehicleParm = NULL;
	TEMP_VEHICLE_PARM stTmpVehicleParm;

	memset( &stTmpVehicleParm, 0x00, sizeof( stTmpVehicleParm ) );
	memset( &gstVehicleParm, 0x00, sizeof( gstVehicleParm ) );

	/*
	 * 차량정보파일 OPEN
	 */
	fdVehicleParm = fopen( VEHICLE_PARM_FILE, "rb" );
	if ( fdVehicleParm == NULL )
	{
		printf( "[LoadInfo] 차량정보 로드 실패 (OPEN)\n" );
		return ErrRet( ERR_FILE_OPEN | GetFileNo( VEHICLE_PARM_FILE ) );
	}

	/*
	 * 차량정보파일 READ
	 */
	nReadSize = fread( &stTmpVehicleParm,
		sizeof( stTmpVehicleParm ),
		1,
		fdVehicleParm );

    fclose( fdVehicleParm );

	if ( nReadSize != 1 )
	{
		printf( "[LoadInfo] 차량정보 로드 실패 (레코드 READ)\n" );
		return ErrRet( ERR_FILE_READ | GetFileNo( VEHICLE_PARM_FILE ) );
	}

	/*
	 * 차량정보 설정
	 */
	// 1. 적용일시
	memcpy( gstVehicleParm.abApplyDtime, stTmpVehicleParm.abApplyDtime,
		sizeof( gstVehicleParm.abApplyDtime ) );

	// 2. 교통사업자 ID
	memcpy( gstVehicleParm.abTranspBizrID, stTmpVehicleParm.abTranspBizrID,
		sizeof( gstVehicleParm.abTranspBizrID ) );

	// 3. 버스영업소 ID
	memcpy( gstVehicleParm.abBusBizOfficeID, stTmpVehicleParm.abBusBizOfficeID,
		sizeof( gstVehicleParm.abBusBizOfficeID ) );

	// 4. 차량 ID
	memcpy( gstVehicleParm.abVehicleID, stTmpVehicleParm.abVehicleID,
		sizeof( gstVehicleParm.abVehicleID ) );

	// 5. 차량번호
	memcpy( gstVehicleParm.abVehicleNo, stTmpVehicleParm.abVehicleNo,
		sizeof( gstVehicleParm.abVehicleNo ) );

	// 6. 노선 ID
	memcpy( gstVehicleParm.abRouteID, stTmpVehicleParm.abRouteID,
		sizeof( gstVehicleParm.abRouteID ) );

	// 7. 운전자 ID
	memcpy( gstVehicleParm.abDriverID, stTmpVehicleParm.abDriverID,
		sizeof( gstVehicleParm.abDriverID ) );

	// 8. 교통수단코드 - byte 배열
	memcpy( gstVehicleParm.abTranspMethodCode,
		stTmpVehicleParm.abTranspMethodCode,
		sizeof( gstVehicleParm.abTranspMethodCode ) );

	// 9. 교통수단코드 - word
	gstVehicleParm.wTranspMethodCode =
		GetWORDFromASC( gstVehicleParm.abTranspMethodCode,
			sizeof( gstVehicleParm.abTranspMethodCode ) );

	// 10. 사업자번호
	memcpy( gstVehicleParm.abBizNo, stTmpVehicleParm.abBizNo,
		sizeof( gstVehicleParm.abBizNo ) );

	// 11. 교통사업자명
	memcpy( gstVehicleParm.abTranspBizrNm, stTmpVehicleParm.abTranspBizrNm,
		sizeof( gstVehicleParm.abTranspBizrNm ) );

	// 12. 주소
	memcpy( gstVehicleParm.abAddr, stTmpVehicleParm.abAddr,
		sizeof( gstVehicleParm.abAddr ) );

	// 13. 대표자명
	memcpy( gstVehicleParm.abRepreNm, stTmpVehicleParm.abRepreNm,
		sizeof( gstVehicleParm.abRepreNm ) );

	DebugOutlnASC	( "적용일시                              : ",
		gstVehicleParm.abApplyDtime, sizeof( stTmpVehicleParm.abApplyDtime ) );
	DebugOutlnASC	( "교통사업자 ID                         : ",
		gstVehicleParm.abTranspBizrID,
		sizeof( gstVehicleParm.abTranspBizrID ) );
	DebugOutlnASC	( "버스영업소 ID                         : ",
		gstVehicleParm.abBusBizOfficeID,
		sizeof( gstVehicleParm.abBusBizOfficeID ) );
	DebugOutlnASC	( "차량 ID                               : ",
		gstVehicleParm.abVehicleID, sizeof( gstVehicleParm.abVehicleID ) );
	DebugOutlnASC	( "차량번호                              : ",
		gstVehicleParm.abVehicleNo, sizeof( gstVehicleParm.abVehicleNo ) );
	DebugOutlnASC	( "노선 ID                               : ",
		gstVehicleParm.abRouteID, sizeof( gstVehicleParm.abRouteID ) );
	DebugOutlnASC	( "운전자 ID                             : ",
		gstVehicleParm.abDriverID, sizeof( gstVehicleParm.abDriverID ) );
	DebugOutlnASC	( "교통수단코드 - byte 배열              : ",
		gstVehicleParm.abTranspMethodCode,
		sizeof( gstVehicleParm.abTranspMethodCode ) );
	DebugOut		( "교통수단코드 - word                   : %u\n",
		gstVehicleParm.wTranspMethodCode );
	DebugOutlnASC	( "사업자번호                            : ",
		gstVehicleParm.abBizNo, sizeof( gstVehicleParm.abBizNo ) );
	DebugOutlnASC	( "교통사업자명                          : ",
		gstVehicleParm.abTranspBizrNm,
		sizeof( gstVehicleParm.abTranspBizrNm ) );
	DebugOutlnASC	( "주소                                  : ",
		gstVehicleParm.abAddr, sizeof( gstVehicleParm.abAddr ) );
	DebugOutlnASC	( "대표자명                              : ",
		gstVehicleParm.abRepreNm, sizeof( gstVehicleParm.abRepreNm ) );

	printf( "[LoadInfo] 차량정보           [%s] 로드 성공\n", VEHICLE_PARM_FILE );

    return SUCCESS;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      LoadRouteParm                                            *
*                                                                              *
*  DESCRIPTION :      노선별 파라미터 파일을 로드한다.		                   *
*                                                                              *
*  INPUT PARAMETERS:  ROUTE_PARM *pstRouteParmInfo                             *
*                       - 운전자 조작기에서만 사용되는 정보                      *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       ERR_FILE_OPEN | GetFileNo( ROUTE_PARM_FILE )           *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short LoadRouteParm( void )
{
	int i = 0;								// LEAP 패스워드 사이즈
	int nReadSize = 0;
	FILE *fdRouteParm = NULL;				// 파일 디스크립터
	TEMP_ROUTE_PARM	stTmpRouteParm;			// 노선별파라미터 구조체( read용 )

	memset( &stTmpRouteParm, 0x00, sizeof( TEMP_ROUTE_PARM ) );
	memset( &gstRouteParm, 0x00, sizeof( ROUTE_PARM ) );

	/*
	 * 노선정보파일 OPEN
	 */
	fdRouteParm = fopen( ROUTE_PARM_FILE, "rb" );
	if ( fdRouteParm == NULL )
	{
//		printf( "[LoadInfo] 노선정보 로드 실패 (OPEN)\n" );
		return ErrRet( ERR_FILE_OPEN | GetFileNo( ROUTE_PARM_FILE ) );
	}

	/*
	 * 노선정보파일 READ
	 */
	nReadSize = fread( &stTmpRouteParm,
		sizeof( stTmpRouteParm ),
		1,
		fdRouteParm );

    fclose( fdRouteParm );

	if ( nReadSize != 1 )
	{
		printf( "[LoadInfo] 노선정보 로드 실패 (레코드 READ)\n" );
		return ErrRet( ERR_FILE_READ | GetFileNo( ROUTE_PARM_FILE ) );
	}

	/*
	 * 노선정보 설정
	 */
	// 1. 적용일시
	memcpy( gstRouteParm.abApplyDtime, stTmpRouteParm.abApplyDtime,
		sizeof( gstRouteParm.abApplyDtime ) );

	// 2. 노선ID
	memcpy( gstRouteParm.abRouteID, stTmpRouteParm.abRouteID,
		sizeof( gstRouteParm.abRouteID ) );

	// 3. 교통수단코드 - byte 배열
	memcpy( gstRouteParm.abTranspMethodCode, stTmpRouteParm.abTranspMethodCode,
		sizeof( gstRouteParm.abTranspMethodCode ) );

	// 4. 교통수단코드 - word
	gstRouteParm.wTranspMethodCode =
		GetWORDFromASC( stTmpRouteParm.abTranspMethodCode,
			sizeof( stTmpRouteParm.abTranspMethodCode ) );

	// 5. 환승할인적용횟수 (미사용)
	memcpy( gstRouteParm.abXferDisApplyFreq, stTmpRouteParm.abXferDisApplyFreq,
		sizeof( gstRouteParm.abXferDisApplyFreq ) );

	// 6. 환승적용시간 (미사용)
	memcpy( gstRouteParm.abXferApplyTime, stTmpRouteParm.abXferApplyTime,
		sizeof( gstRouteParm.abXferApplyTime ) );

	// 7. 단말기그룹코드 (미사용)
	memcpy( gstRouteParm.abTermGroupCode, stTmpRouteParm.abTermGroupCode,
		sizeof( gstRouteParm.abTermGroupCode ) );

	// 8. 하차단말기와통신간격 (미사용)
	memcpy( gstRouteParm.abSubTermCommIntv, stTmpRouteParm.abSubTermCommIntv,
		sizeof( gstRouteParm.abSubTermCommIntv ) );

	// 9. 카드처리시간간격 (미사용)
	memcpy( gstRouteParm.abCardProcTimeIntv, stTmpRouteParm.abCardProcTimeIntv,
		sizeof( gstRouteParm.abCardProcTimeIntv ) );

	// 10. 변동BL유효기간 (미사용)
	memcpy( gstRouteParm.abUpdateBLValidPeriod,
		stTmpRouteParm.abUpdateBLValidPeriod,
		sizeof( gstRouteParm.abUpdateBLValidPeriod ) );

	// 11. 변동PL유효기간 (미사용)
	memcpy( gstRouteParm.abUpdatePLValidPeriod,
		stTmpRouteParm.abUpdatePLValidPeriod,
		sizeof( gstRouteParm.abUpdatePLValidPeriod ) );

	// 12. 변동AI유효기간 (미사용)
	memcpy( gstRouteParm.abUpdateAIValidPeriod,
		stTmpRouteParm.abUpdateAIValidPeriod,
		sizeof( stTmpRouteParm.abUpdateAIValidPeriod ) );

	// 13. 운전자카드사용유무 (미사용)
	gstRouteParm.bDriverCardUseYN = stTmpRouteParm.bDriverCardUseYN;

	// 14. 충전정보전송횟수 (미사용)
	memcpy( gstRouteParm.abChargeInfoSendFreq,
		stTmpRouteParm.abChargeInfoSendFreq,
		sizeof( gstRouteParm.abChargeInfoSendFreq ) );

	// 15. 운행종류코드 - 운전자조작기전송
	gstRouteParm.bRunKindCode = stTmpRouteParm.bRunKindCode;

	// 16. 경기인천구간요금입력방식 - 운전자조작기전송
	gstRouteParm.bGyeonggiIncheonRangeFareInputWay =
		stTmpRouteParm.bGyeonggiIncheonRangeFareInputWay;

	// 17. LEAP 암호 (미사용)
	//     - gpaLEAPPasswd에는 복사하지 않음
	memset( gstRouteParm.abLEAPPasswd, 0x00,
		sizeof( gstRouteParm.abLEAPPasswd ) );

	for ( i = 0 ; i < sizeof( stTmpRouteParm.abLEAPPasswd ) ; i++ )
	{
		if ( stTmpRouteParm.abLEAPPasswd[i] == 0xff )
		{
			break;
		}
	}

	BCD2ASC( stTmpRouteParm.abLEAPPasswd, gstRouteParm.abLEAPPasswd, i );

	// 18. 후불ecard 사용유무 (미사용)
	gstRouteParm.bECardUseYN = stTmpRouteParm.bECardUseYN;

	DebugOutlnASC	( "적용일시                              : ",
		gstRouteParm.abApplyDtime, sizeof( gstRouteParm.abApplyDtime ) );
	DebugOutlnASC	( "노선ID                                : ",
		gstRouteParm.abRouteID, sizeof( gstRouteParm.abRouteID ) );
	DebugOutlnASC	( "교통수단코드 - byte 배열              : ",
		gstRouteParm.abTranspMethodCode,
		sizeof( gstRouteParm.abTranspMethodCode ) );
	DebugOut     	( "교통수단코드 - word                   : %u\n",
		gstRouteParm.wTranspMethodCode );
	DebugOutlnASC	( "환승할인적용횟수 (미사용)             : ",
		gstRouteParm.abXferDisApplyFreq,
		sizeof( gstRouteParm.abXferDisApplyFreq ) );
	DebugOutlnASC	( "환승적용시간 (미사용)                 : ",
		gstRouteParm.abXferApplyTime,
		sizeof( gstRouteParm.abXferApplyTime ) );
	DebugOutlnASC	( "단말기그룹코드 (미사용)               : ",
		gstRouteParm.abTermGroupCode, sizeof( gstRouteParm.abTermGroupCode ) );
	DebugOutlnASC	( "하차단말기와통신간격 (미사용)         : ",
		gstRouteParm.abSubTermCommIntv,
		sizeof( gstRouteParm.abSubTermCommIntv ) );
    DebugOutlnASC	( "카드처리시간간격 (미사용)             : ",
		gstRouteParm.abCardProcTimeIntv,
		sizeof( gstRouteParm.abCardProcTimeIntv ) );
	DebugOutlnASC	( "변동BL유효기간 (미사용)               : ",
		gstRouteParm.abUpdateBLValidPeriod,
		sizeof( gstRouteParm.abUpdateBLValidPeriod ) );
	DebugOutlnASC	( "변동PL유효기간 (미사용)               : ",
		gstRouteParm.abUpdatePLValidPeriod,
		sizeof( gstRouteParm.abUpdatePLValidPeriod ) );
	DebugOutlnASC	( "변동AI유효기간 (미사용)               : ",
		gstRouteParm.abUpdateAIValidPeriod,
		sizeof( gstRouteParm.abUpdateAIValidPeriod ) );
	DebugOut		( "운전자카드사용유무 (미사용)           : %u\n",
		gstRouteParm.bDriverCardUseYN );
	DebugOutlnASC	( "충전정보전송횟수 (미사용)             : ",
		gstRouteParm.abChargeInfoSendFreq,
		sizeof( gstRouteParm.abChargeInfoSendFreq ) );
	DebugOut		( "운행종류코드 - 운전자조작기전송       : %u\n",
		gstRouteParm.bRunKindCode );
	DebugOut		( "경기인천구간요금입력방식 - 운조전송   : %u\n",
		gstRouteParm.bGyeonggiIncheonRangeFareInputWay );
    DebugOutlnASC	( "LEAP 암호 (미사용)                    : ",
		gstRouteParm.abLEAPPasswd, sizeof( gstRouteParm.abLEAPPasswd ) );
    DebugOut		( "후불ecard 사용유무 (미사용)           : %u\n",
		gstRouteParm.bECardUseYN );

    printf( "[LoadInfo] 노선정보           [%s] 로드 성공\n", ROUTE_PARM_FILE );

    return SUCCESS;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      LoadPrepayIssuerInfo                                     *
*                                                                              *
*  DESCRIPTION :      구선불발행사 정보 파일을 로드한다.					   *
*                                                                              *
*  INPUT PARAMETERS:  void                                                     *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS,                                               *
*                       ERR_FILE_OPEN | GetFileNo( PREPAY_ISSUER_INFO_FILE )   *
*                       ERR_FILE_READ | GetFileNo( PREPAY_ISSUER_INFO_FILE )   *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short LoadPrepayIssuerInfo( void )
{
	short	sResult		= SUCCESS;
	FILE	*fdInfo		= NULL;
	dword	i			= 0;
	int		nReadSize	= 0;

	PREPAY_ISSUER_INFO					*pstPrepayIssuerInfo = NULL;
	TEMP_PREPAY_ISSUER_INFO_HEADER		stInfoHeader;
	TEMP_PREPAY_ISSUER_INFO				stInfo;

	/*
	 * 선불발행사정보파일 OPEN
	 */
	fdInfo = fopen( PREPAY_ISSUER_INFO_FILE, "rb" );
	if ( fdInfo == NULL )
	{
		printf( "[LoadInfo] 선불발행사정보 로드 실패 (OPEN)\n" );
		sResult = ERR_FILE_OPEN | GetFileNo( PREPAY_ISSUER_INFO_FILE );
		goto FINALLY;
	}

	/*
	 * 선불발행사정보파일 헤더 READ
	 */
	nReadSize = fread( &stInfoHeader,
		sizeof( stInfoHeader ),
		1,
		fdInfo );
	if ( nReadSize != 1 )
	{
		printf( "[LoadInfo] 선불발행사정보 로드 실패 (헤더 READ)\n" );
		sResult = ERR_FILE_READ | GetFileNo( PREPAY_ISSUER_INFO_FILE );
		goto FINALLY;
	}

	/*
	 * 선불발행사정보 헤더 설정
	 */
	gstPrepayIssuerInfoHeader.tApplyDtime =
		GetTimeTFromBCDDtime( stInfoHeader.abApplyDtime );
	gstPrepayIssuerInfoHeader.bApplySeqNo = stInfoHeader.bApplySeqNo;
	gstPrepayIssuerInfoHeader.dwRecordCnt =
		GetDWORDFromASC( stInfoHeader.abRecordCnt,
			sizeof( stInfoHeader.abRecordCnt ) );

	DebugOutlnTimeT ( "적용일시                              : ",
		gstPrepayIssuerInfoHeader.tApplyDtime );
	DebugOut        ( "적용일련번호                          : %u\n",
		gstPrepayIssuerInfoHeader.bApplySeqNo );
	DebugOut        ( "레코드 건수                           : %lu\n",
		gstPrepayIssuerInfoHeader.dwRecordCnt );

	/*
	 * 선불발행사정보 메모리 공간 ALLOC
	 */
	pstPrepayIssuerInfo =
		( PREPAY_ISSUER_INFO * )malloc( sizeof( PREPAY_ISSUER_INFO ) *
			gstPrepayIssuerInfoHeader.dwRecordCnt );
	if ( pstPrepayIssuerInfo == NULL )
	{
		printf( "[LoadInfo] 선불발행사정보 로드 실패 (메모리공간 ALLOC)\n" );
		sResult = ERR_FILE_READ | GetFileNo( PREPAY_ISSUER_INFO_FILE );
		goto FINALLY;
	}

	/*
	 * 선불발행사정보 설정
	 */
	for ( i = 0; i < gstPrepayIssuerInfoHeader.dwRecordCnt; i++ )
	{
		nReadSize = fread( &stInfo, sizeof( stInfo ), 1, fdInfo );
		if ( nReadSize != 1 )
		{
			printf( "[LoadInfo] 선불발행사정보 로드 실패 (레코드 READ)\n" );
			sResult = ERR_FILE_READ | GetFileNo( PREPAY_ISSUER_INFO_FILE );
			goto FINALLY;
		}

		memcpy( pstPrepayIssuerInfo[i].abPrepayIssuerID,
			stInfo.abPrepayIssuerID,
			sizeof( pstPrepayIssuerInfo[i].abPrepayIssuerID ) );
		pstPrepayIssuerInfo[i].bAssocCode = stInfo.bAssocCode;
		pstPrepayIssuerInfo[i].bXferDisYoungCardApply =
			stInfo.bXferDisYoungCardApply;

		DebugOutlnASC   ( "선불 발행사 ID                        : ",
			pstPrepayIssuerInfo[i].abPrepayIssuerID,
			sizeof( pstPrepayIssuerInfo[i].abPrepayIssuerID ));
		DebugOut        ( "단말기 소속 조합 구분코드             : '%c'\n",
			pstPrepayIssuerInfo[i].bAssocCode );
		DebugOut        ( "환승할인/학생카드 적용유무            : '%c'\n",
			pstPrepayIssuerInfo[i].bXferDisYoungCardApply );
    }

	FINALLY:

	if ( fdInfo != NULL )
	{
		fclose( fdInfo );
	}

	if ( sResult == SUCCESS )
	{
		/*
		 * 기존 로드된 선불발행사정보는 FREE
		 */
	    free( gpstPrepayIssuerInfo );

		/*
		 * 새로 로드된 정보를 포인터에 설정
		 */
	    gpstPrepayIssuerInfo = pstPrepayIssuerInfo;

		printf( "[LoadInfo] 선불발행사정보     [%s] 로드 성공\n", PREPAY_ISSUER_INFO_FILE );
	}
	else
	{
		free( gpstPrepayIssuerInfo );
		free( pstPrepayIssuerInfo );
		memset( &gstPrepayIssuerInfoHeader, 0x00,
			sizeof( PREPAY_ISSUER_INFO_HEADER ) );
		gpstPrepayIssuerInfo = NULL;
	}

	return sResult;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      LoadPostpayIssuerInfo                                    *
*                                                                              *
*  DESCRIPTION :      후불발행사 정보 파일을 로드한다.					       *
*                                                                              *
*  INPUT PARAMETERS:  void                                                     *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS,                                               *
*                       ERR_FILE_OPEN | GetFileNo( POSTPAY_ISSUER_INFO_FILE )  *
*                       ERR_FILE_READ | GetFileNo( POSTPAY_ISSUER_INFO_FILE )  *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short LoadPostpayIssuerInfo( void )
{
	short	sResult		= SUCCESS;
	FILE	*fdInfo		= NULL;
	dword	i			= 0;
	int		nReadSize	= 0;

	POSTPAY_ISSUER_INFO				*pstPostpayIssuerInfo = NULL;
	TEMP_POSTPAY_ISSUER_INFO_HEADER	stInfoHeader;
	TEMP_POSTPAY_ISSUER_INFO		stInfo;

	/*
	 * 후불발행사정보파일 OPEN
	 */
	fdInfo = fopen( POSTPAY_ISSUER_INFO_FILE, "rb" );
	if ( fdInfo == NULL )
	{
		printf( "[LoadInfo] 후불발행사정보 로드 실패 (OPEN)\n" );
		sResult = ERR_FILE_OPEN | GetFileNo( POSTPAY_ISSUER_INFO_FILE );
		goto FINALLY;
	}

	/*
	 * 후불발행사정보파일 헤더 READ
	 */
	nReadSize = fread( &stInfoHeader,
		sizeof( stInfoHeader ),
		1,
		fdInfo );
	if ( nReadSize != 1 )
	{
		printf( "[LoadInfo] 후불발행사정보 로드 실패 (헤더 READ)\n" );
		sResult = ERR_FILE_READ | GetFileNo( POSTPAY_ISSUER_INFO_FILE );
		goto FINALLY;
	}

	/*
	 * 후불발행사정보 헤더 설정
	 */
	gstPostpayIssuerInfoHeader.tApplyDtime =
		GetTimeTFromBCDDtime( stInfoHeader.abApplyDtime );
	gstPostpayIssuerInfoHeader.wRecordCnt = stInfoHeader.wRecordCnt;

	DebugOutlnTimeT ( "적용일시                              : ",
		gstPostpayIssuerInfoHeader.tApplyDtime );
	DebugOut        ( "레코드 건수                           : %u\n",
		gstPostpayIssuerInfoHeader.wRecordCnt );

	/*
	 * 후불발행사정보 메모리 공간 ALLOC
	 */
	pstPostpayIssuerInfo =
		( POSTPAY_ISSUER_INFO *)malloc( sizeof( POSTPAY_ISSUER_INFO ) *
			gstPostpayIssuerInfoHeader.wRecordCnt );
	if ( pstPostpayIssuerInfo == NULL )
	{
		printf( "[LoadInfo] 후불발행사정보 로드 실패 (메모리공간 ALLOC)\n" );
		sResult = ERR_FILE_READ | GetFileNo( POSTPAY_ISSUER_INFO_FILE );
		goto FINALLY;
	}

	/*
	 * 후불발행사정보 설정
	 */
	for ( i = 0; i < gstPostpayIssuerInfoHeader.wRecordCnt; i++ )
	{
		nReadSize = fread( &stInfo, sizeof( stInfo ), 1, fdInfo );
		if ( nReadSize != 1 )
		{
			printf( "[LoadInfo] 후불발행사정보 로드 실패 (레코드 READ)\n" );
			sResult = ERR_FILE_READ | GetFileNo( POSTPAY_ISSUER_INFO_FILE );
			goto FINALLY;
		}

		BCD2ASC( stInfo.abPrefixNo, pstPostpayIssuerInfo[i].abPrefixNo,
			sizeof( stInfo.abPrefixNo ) );
		pstPostpayIssuerInfo[i].wCompCode = stInfo.wCompCode;

		DebugOutlnASC   ( "Prefix 번호                           : ",
			pstPostpayIssuerInfo[i].abPrefixNo,
			sizeof ( pstPostpayIssuerInfo[i].abPrefixNo ) );
		DebugOut		( "압축코드                              : %u\n",
			pstPostpayIssuerInfo[i].wCompCode );
	}

	FINALLY:

	if ( fdInfo != NULL )
	{
		fclose( fdInfo );
	}

	if ( sResult == SUCCESS )
	{
		/*
		 * 기존 로드된 후불발행사정보는 FREE
		 */
	    free( gpstPostpayIssuerInfo );

		/*
		 * 새로 로드된 정보를 포인터에 설정
		 */
	    gpstPostpayIssuerInfo = pstPostpayIssuerInfo;

		printf( "[LoadInfo] 후불발행사정보     [%s] 로드 성공\n", POSTPAY_ISSUER_INFO_FILE );
	}
	else
	{
		free( gpstPostpayIssuerInfo );
		free( pstPostpayIssuerInfo );
		memset( &gstPostpayIssuerInfoHeader, 0x00,
			sizeof( POSTPAY_ISSUER_INFO_HEADER ) );
		gpstPostpayIssuerInfo = NULL;
	}

	return sResult;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      LoadIssuerValidPeriodInfo                                *
*                                                                              *
*  DESCRIPTION :      후불발행서 유효기간 정보 파일을 로드한다.                *
*                                                                              *
*  INPUT PARAMETERS:  void                                                     *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS,                                               *
*          ERR_FILE_OPEN | GetFileNo( ISSUER_VALID_PERIOD_INFO_FILE )          *
*          ERR_FILE_READ | GetFileNo( ISSUER_VALID_PERIOD_INFO_FILE )          *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short LoadIssuerValidPeriodInfo( void )
{
	short	sResult		= SUCCESS;
	FILE	*fdInfo		= NULL;
	dword	i			= 0;
	int		nReadSize	= 0;

	ISSUER_VALID_PERIOD_INFO		*pstIssuerValidPeriodInfo = NULL;
	TEMP_POSTPAY_ISSUER_VALID_PERIOD_INFO_HEADER	stInfoHeader;
	TEMP_POSTPAY_ISSUER_VALID_PERIOD_INFO			stInfo;

	fdInfo = fopen( ISSUER_VALID_PERIOD_INFO_FILE, "rb" );
	if ( fdInfo == NULL )
	{
		printf( "[LoadInfo] 발행사유효기간정보 로드 실패 (OPEN)\n" );
		sResult =
			ERR_FILE_OPEN | GetFileNo( ISSUER_VALID_PERIOD_INFO_FILE );
		goto FINALLY;
	}

	nReadSize = fread( &stInfoHeader,
		sizeof( stInfoHeader ),
		1,
		fdInfo );
	if ( nReadSize != 1 )
	{
		printf( "[LoadInfo] 발행사유효기간정보 로드 실패 (헤더 READ)\n" );
		sResult =
			ERR_FILE_READ | GetFileNo( ISSUER_VALID_PERIOD_INFO_FILE );
		goto FINALLY;
	}

	gstIssuerValidPeriodInfoHeader.tApplyDtime =
		GetTimeTFromASCDtime( stInfoHeader.abApplyDtime );
	gstIssuerValidPeriodInfoHeader.wRecordCnt =
		GetWORDFromASC(stInfoHeader.abRecordCnt,
			sizeof( stInfoHeader.abRecordCnt ) );

	DebugOutlnTimeT ( "적용일시                              : ",
		gstIssuerValidPeriodInfoHeader.tApplyDtime );
	DebugOut        ( "레코드 건수                           : %u\n",
		gstIssuerValidPeriodInfoHeader.wRecordCnt );

	pstIssuerValidPeriodInfo =
		( ISSUER_VALID_PERIOD_INFO * )malloc(sizeof( ISSUER_VALID_PERIOD_INFO ) *
			gstIssuerValidPeriodInfoHeader.wRecordCnt );
	if ( pstIssuerValidPeriodInfo == NULL )
	{
		printf( "[LoadInfo] 발행사유효기간정보 로드 실패 (메모리공간 ALLOC)\n" );
		sResult =
			ERR_FILE_READ | GetFileNo( ISSUER_VALID_PERIOD_INFO_FILE );
		goto FINALLY;
	}

	for ( i = 0; i < gstIssuerValidPeriodInfoHeader.wRecordCnt; i++ )
	{
		nReadSize = fread( &stInfo, sizeof( stInfo ), 1, fdInfo );
		if ( nReadSize != 1 )
		{
			printf( "[LoadInfo] 발행사유효기간정보 로드 실패 (레코드 READ)\n" );
			sResult =
				ERR_FILE_READ | GetFileNo( ISSUER_VALID_PERIOD_INFO_FILE );
			goto FINALLY;
		}

		memcpy( pstIssuerValidPeriodInfo[i].abPrefixNo,
			stInfo.abPrefixNo,
			sizeof( pstIssuerValidPeriodInfo[i].abPrefixNo ) );
		memcpy( pstIssuerValidPeriodInfo[i].abIssuerID,
			stInfo.abIssuerID,
			sizeof( pstIssuerValidPeriodInfo[i].abIssuerID ) );
		memcpy( pstIssuerValidPeriodInfo[i].abExpiryDate,
			stInfo.abExpiryDate,
			sizeof( pstIssuerValidPeriodInfo[i].abExpiryDate ) );

		DebugOutlnASC	( "Prefix  번호                          : ",
			pstIssuerValidPeriodInfo[i].abPrefixNo,
			sizeof( pstIssuerValidPeriodInfo[i].abPrefixNo ) );
        DebugOutlnASC	( "발행사ID                              : ",
			pstIssuerValidPeriodInfo[i].abIssuerID,
			sizeof( pstIssuerValidPeriodInfo[i].abIssuerID ) );
        DebugOutlnASC	( "카드유효기간 (YYYYMM)                 : ",
			pstIssuerValidPeriodInfo[i].abExpiryDate,
			sizeof( pstIssuerValidPeriodInfo[i].abExpiryDate ) );
    }

	FINALLY:

	if ( fdInfo != NULL )
	{
		fclose( fdInfo );
	}

	if ( sResult == SUCCESS )
	{
		/*
		 * 기존 로드된 발행사유효기간정보는 FREE
		 */
	    free( gpstIssuerValidPeriodInfo );

		/*
		 * 새로 로드된 정보를 포인터에 설정
		 */
	    gpstIssuerValidPeriodInfo = pstIssuerValidPeriodInfo;

		printf( "[LoadInfo] 발행사유효기간정보 [%s] 로드 성공\n",
			ISSUER_VALID_PERIOD_INFO_FILE );
	}
	else
	{
		free( gpstIssuerValidPeriodInfo );
		free( pstIssuerValidPeriodInfo );
		memset( &gstIssuerValidPeriodInfoHeader, 0x00,
			sizeof( ISSUER_VALID_PERIOD_INFO_HEADER ) );
		gpstIssuerValidPeriodInfo = NULL;
	}

	return sResult;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       LoadXferApplyInfo                                        *
*                                                                              *
*  DESCRIPTION:       환승적용정보 파일을 로드한다. 단, 로드에 실패하는 경우   *
*                     무조건 환승적용정보건수를 0으로 만들고 포인터를          *
*                     초기화한다.                                              *
*                                                                              *
*  INPUT PARAMETERS:  없음                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS                                                  *
*                     ERR_FILE_OPEN | GetFileNo( XFER_APPLY_INFO_FILE )        *
*                     ERR_FILE_READ | GetFileNo( XFER_APPLY_INFO_FILE )        *
*                                                                              *
*  Author:            Mi Hyun Noh                                              *
*                                                                              *
*  DATE:              2005-09-03                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static short LoadXferApplyInfo( void )
{
	short	sResult		= SUCCESS;
	int		fdInfo		= 0;
    dword	i			= 0;
    int		nReadSize	= 0;

	struct stat				fileStatus;			// File Status 구조체
	XFER_APPLY_INFO			*pstXferApplyInfo = NULL;
	TEMP_XFER_APPLY_INFO	stInfo;

	/*
	 * 환승적용정보파일 OPEN
	 */
	fdInfo = open( XFER_APPLY_INFO_FILE, O_RDWR, OPENMODE );
	if ( fdInfo < 0 )
	{
	    printf( "[LoadInfo] 환승적용정보 로드 실패 (OPEN)\n" );
		sResult = ERR_FILE_OPEN | GetFileNo( XFER_APPLY_INFO_FILE );
		goto FINALLY;
	}

	/*
	 * 환승적용정보파일 fstat
	 */
    if ( fstat( fdInfo, &fileStatus ) != SUCCESS )
	{
	    printf( "[LoadInfo] 환승적용정보 로드 실패 (fstat)\n" );
		sResult = ERR_FILE_READ | GetFileNo( XFER_APPLY_INFO_FILE );
		goto FINALLY;
	}

	/*
	 * 환승적용정보파일 레코드 개수 산출
	 */
    gstXferApplyInfoHeader.dwRecordCnt = fileStatus.st_size / sizeof( stInfo );
    if ( gstXferApplyInfoHeader.dwRecordCnt == 0 )
	{
	    printf( "[LoadInfo] 환승적용정보 로드 실패 (건수 0)\n" );
		sResult = ERR_FILE_READ | GetFileNo( XFER_APPLY_INFO_FILE );
		goto FINALLY;
	}

	DebugOut		( "환승적용정보 건수                     : %lu\n",
		gstXferApplyInfoHeader.dwRecordCnt );

	/*
	 * 환승적용정보 메모리 공간 ALLOC
	 */
    pstXferApplyInfo =
    	( XFER_APPLY_INFO *)malloc( sizeof( XFER_APPLY_INFO ) *
    		gstXferApplyInfoHeader.dwRecordCnt );
	if ( pstXferApplyInfo == NULL )
	{
	    printf( "[LoadInfo] 환승적용정보 로드 실패 (메모리공간 ALLOC)\n" );
		sResult = ERR_FILE_READ | GetFileNo( XFER_APPLY_INFO_FILE );
		goto FINALLY;
	}

	/*
	 * 환승적용정보 설정
	 */
    for ( i = 0; i < gstXferApplyInfoHeader.dwRecordCnt; i++ )
	{
	    nReadSize = read( fdInfo, &stInfo, sizeof( stInfo ) );
	    if ( nReadSize < sizeof( stInfo ) )
		{
		    printf( "[LoadInfo] 환승적용정보 로드 실패 (레코드 READ)\n" );
			sResult = ERR_FILE_READ | GetFileNo( XFER_APPLY_INFO_FILE );
			goto FINALLY;
		}

	    pstXferApplyInfo[i].tApplyDtime =
			GetTimeTFromBCDDtime( stInfo.abApplyDtime );
	    pstXferApplyInfo[i].wXferApplyStartTime =
			GetWORDFromASC( stInfo.abXferApplyStartTime,
				sizeof( stInfo.abXferApplyStartTime ) );
	    pstXferApplyInfo[i].wXferApplyEndTime =
			GetWORDFromASC( stInfo.abXferApplyEndTime,
				sizeof( stInfo.abXferApplyEndTime ) );
	    pstXferApplyInfo[i].bHolidayClassCode = stInfo.bHolidayClassCode;
	    pstXferApplyInfo[i].dwXferEnableTime =
			GetDWORDFromASC( stInfo.abXferEnableTime,
				sizeof( stInfo.abXferEnableTime ) );
	    pstXferApplyInfo[i].wXferEnableCnt =
			GetWORDFromASC( stInfo.abXferEnableCnt,
				sizeof( stInfo.abXferEnableCnt ) );

	    DebugOutlnTimeT ( "적용일시                              : ",
			pstXferApplyInfo[i].tApplyDtime );
	    DebugOut		( "환승적용시작시간                      : %u\n",
			pstXferApplyInfo[i].wXferApplyStartTime );
        DebugOut		( "환승적용종료시간                      : %u\n",
			pstXferApplyInfo[i].wXferApplyEndTime );
        DebugOut		( "휴일구분코드                          : '%c'\n",
			pstXferApplyInfo[i].bHolidayClassCode );
        DebugOut		( "환승가능시간                          : %lu\n",
			pstXferApplyInfo[i].dwXferEnableTime );
        DebugOut		( "환승가능횟수                          : %u\n",
			pstXferApplyInfo[i].wXferEnableCnt );
    }

	FINALLY:

	if ( fdInfo >= 0 )
	{
		close( fdInfo );
	}

	if ( sResult == SUCCESS )
	{
		/*
		 * 기존 로드된 환승적용정보는 FREE
		 */
	    free( gpstXferApplyInfo );

		/*
		 * 새로 로드된 정보를 포인터에 설정
		 */
	    gpstXferApplyInfo = pstXferApplyInfo;

		printf( "[LoadInfo] 환승적용정보       [%s] 로드 성공\n", XFER_APPLY_INFO_FILE );
	}
	else
	{
		free( gpstXferApplyInfo );
		free( pstXferApplyInfo );
		memset( &gstXferApplyInfoHeader, 0x00,
			sizeof( XFER_APPLY_INFO_HEADER ) );
		gpstXferApplyInfo = NULL;
	}

    return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      LoadDisExtraInfo                                         *
*                                                                              *
*  DESCRIPTION :      할인할증 정보파일을 로드한다.							   *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       ERR_FILE_OPEN | GetFileNo( DIS_EXTRA_INFO_FILE )       *
*                       ERR_FILE_READ | GetFileNo( DIS_EXTRA_INFO_FILE )       *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short LoadDisExtraInfo( void )
{
	short	sResult		= SUCCESS;
	FILE	*fdInfo		= NULL;
	dword	i			= 0;
	int		nReadSize	= 0;

	DIS_EXTRA_INFO*				pstDisExtraInfo = NULL;
	TEMP_DIS_EXTRA_INFO_HEADER	stInfoHeader;
	TEMP_DIS_EXTRA_INFO			stInfo;

	fdInfo = fopen( DIS_EXTRA_INFO_FILE, "rb" );
	if ( fdInfo == NULL )
	{
		printf( "[LoadInfo] 할인할증정보 로드 실패 (OPEN)\n" );
		sResult = ERR_FILE_OPEN | GetFileNo( DIS_EXTRA_INFO_FILE );
		goto FINALLY;
	}

	nReadSize = fread( &stInfoHeader,
		sizeof( stInfoHeader ),
		1,
		fdInfo );
	if ( nReadSize != 1 )
	{
		printf( "[LoadInfo] 할인할증정보 로드 실패 (헤더 READ)\n" );
		sResult = ERR_FILE_READ | GetFileNo( DIS_EXTRA_INFO_FILE );
		goto FINALLY;
	}

	gstDisExtraInfoHeader.tApplyDtime =
		GetTimeTFromBCDDtime( stInfoHeader.abApplyDtime );
	gstDisExtraInfoHeader.bApplySeqNo = stInfoHeader.bApplySeqNo;
	gstDisExtraInfoHeader.dwRecordCnt =
		GetDWORDFromBCD( stInfoHeader.abRecordCnt,
			sizeof( stInfoHeader.abRecordCnt ) );

	DebugOutlnTimeT ( "적용일시                              : ",
		gstDisExtraInfoHeader.tApplyDtime );
	DebugOut        ( "적용일련번호                          : %u\n",
		gstDisExtraInfoHeader.bApplySeqNo );
	DebugOut        ( "레코드 건수                           : %lu\n",
		gstDisExtraInfoHeader.dwRecordCnt );

	pstDisExtraInfo = ( DIS_EXTRA_INFO * )malloc( sizeof( DIS_EXTRA_INFO ) *
		gstDisExtraInfoHeader.dwRecordCnt );
	if ( pstDisExtraInfo == NULL )
	{
		printf( "[LoadInfo] 할인할증정보 로드 실패 (메모리공간 ALLOC)\n" );
		sResult = ERR_FILE_READ | GetFileNo( DIS_EXTRA_INFO_FILE );
		goto FINALLY;
	}

	for ( i = 0; i < gstDisExtraInfoHeader.dwRecordCnt; i++ )
	{
		nReadSize = fread( &stInfo, sizeof( stInfo ), 1, fdInfo );
		if ( nReadSize != 1 )
		{
			printf( "[LoadInfo] 할인할증정보 로드 실패 (레코드 READ)\n" );
			sResult = ERR_FILE_READ | GetFileNo( DIS_EXTRA_INFO_FILE );
			goto FINALLY;
		}

		pstDisExtraInfo[i].bTranspCardClassCode = stInfo.bTranspCardClassCode;
		memcpy( pstDisExtraInfo[i].abDisExtraTypeID, stInfo.abDisExtraTypeID,
			sizeof( pstDisExtraInfo[i].abDisExtraTypeID ) );
		pstDisExtraInfo[i].bDisExtraApplyCode =
			stInfo.bDisExtraApplyStandardCode;
		pstDisExtraInfo[i].fDisExtraRate =
			(float)GetINTFromASC( &stInfo.abDisExtraRate[0], 3 );
		pstDisExtraInfo[i].fDisExtraRate +=
			( stInfo.abDisExtraRate[4] - ZERO ) * 0.1 +
			( stInfo.abDisExtraRate[5] - ZERO ) * 0.01;
		pstDisExtraInfo[i].nDisExtraAmt =
			GetDWORDFromBCD( stInfo.abDisExtraAmt,
				sizeof( stInfo.abDisExtraAmt ) );

		if ( stInfo.bPositiveNegativeClass == NEGATIVE_CLASS )
		{
			pstDisExtraInfo[i].fDisExtraRate *= -1;
			pstDisExtraInfo[i].nDisExtraAmt *= -1;
		}

		DebugOut		( "교통카드구분코드                      : '%c'\n",
			pstDisExtraInfo[i].bTranspCardClassCode );
		DebugOutlnASC   ( "할인할증유형ID                        : ",
			pstDisExtraInfo[i].abDisExtraTypeID,
			sizeof( pstDisExtraInfo[i].abDisExtraTypeID ) );
		DebugOut		( "할인할증적용기준코드                  : '%c'\n",
			pstDisExtraInfo[i].bDisExtraApplyCode );
		DebugOut		( "할인/할증률                           : %f\n",
			pstDisExtraInfo[i].fDisExtraRate );
		DebugOut		( "할인/할증 금액                        : %d\n",
			pstDisExtraInfo[i].nDisExtraAmt );
	}

	FINALLY:

	if ( fdInfo != NULL )
	{
		fclose( fdInfo );
	}

	if ( sResult == SUCCESS )
	{
		/*
		 * 기존 로드된 할인할증정보는 FREE
		 */
		free( gpstDisExtraInfo );

		/*
		 * 새로 로드된 정보를 포인터에 설정
		 */
		gpstDisExtraInfo = pstDisExtraInfo;

		printf( "[LoadInfo] 할인할증정보       [%s] 로드 성공\n", DIS_EXTRA_INFO_FILE );
	}
	else
	{
		free( gpstDisExtraInfo );
		free( pstDisExtraInfo );
		memset( &gstDisExtraInfoHeader, 0x00, sizeof( DIS_EXTRA_INFO_HEADER ) );
		gpstDisExtraInfo = NULL;
	}

	return sResult;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       LoadHolidayInfo                                          *
*                                                                              *
*  DESCRIPTION:       휴일정보 파일을 로드한다. 단, 로드에 실패하는 경우       *
*                     무조건 휴일정보건수를 0으로 만들고 포인터를 초기화한다.  *
*                                                                              *
*  INPUT PARAMETERS:  없음                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS                                                  *
*                     ERR_FILE_OPEN | GetFileNo( HOLIDAY_INFO_FILE )           *
*                     ERR_FILE_READ | GetFileNo( HOLIDAY_INFO_FILE )           *
*                                                                              *
*  Author:            Mi Hyun Noh                                              *
*                                                                              *
*  DATE:              2005-09-03                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static short LoadHolidayInfo( void )
{
	short	sResult		= SUCCESS;
	FILE	*fdInfo		= NULL;
	dword	i 			= 0;
	int		nReadSize   = 0;

	HOLIDAY_INFO 				*pstHolidayInfo = NULL;
	TEMP_HOLIDAY_INFO_HEADER	stInfoHeader;

	/*
	 * 휴일정보파일 OPEN
	 */
	fdInfo = fopen( HOLIDAY_INFO_FILE, "rb" );
	if ( fdInfo == NULL )
	{
		printf( "[LoadInfo] 휴일정보 로드 실패 (OPEN)\n" );
		sResult = ERR_FILE_OPEN | GetFileNo( HOLIDAY_INFO_FILE );
		goto FINALLY;
	}

	/*
	 * 휴일정보파일 헤더 READ
	 */
	nReadSize = fread( &stInfoHeader,
		sizeof( stInfoHeader ),
		1,
		fdInfo );
	if ( nReadSize != 1 )
	{
		printf( "[LoadInfo] 휴일정보 로드 실패 (헤더 READ)\n" );
		sResult = ERR_FILE_READ | GetFileNo( HOLIDAY_INFO_FILE );
		goto FINALLY;
	}

	/*
	 * 휴일정보 헤더구조체 설정
	 */
	gstHolidayInfoHeader.tApplyDtime =
		GetTimeTFromBCDDtime( stInfoHeader.abApplyDtime );
	gstHolidayInfoHeader.dwRecordCnt =
		GetDWORDFromASC( stInfoHeader.abRecordCnt,
			sizeof( stInfoHeader.abRecordCnt ) );

	DebugOutlnTimeT	( "휴일정보 적용일시                     : ",
		gstHolidayInfoHeader.tApplyDtime );
	DebugOut		( "휴일정보 건수                         : %lu\n",
		gstHolidayInfoHeader.dwRecordCnt );

	/*
	 * 휴일정보 메모리 공간 ALLOC
	 */
	pstHolidayInfo = ( HOLIDAY_INFO * )malloc( sizeof( HOLIDAY_INFO )
		* gstHolidayInfoHeader.dwRecordCnt );
	if ( pstHolidayInfo == NULL )
	{
		printf( "[LoadInfo] 휴일정보 로드 실패 (메모리공간 ALLOC)\n" );
		sResult = ERR_FILE_READ | GetFileNo( HOLIDAY_INFO_FILE );
		goto FINALLY;
	}

	/*
	 * 휴일정보 설정
	 */
	for ( i = 0; i < gstHolidayInfoHeader.dwRecordCnt; i++ )
	{
		nReadSize = fread( &pstHolidayInfo[i],
			sizeof( HOLIDAY_INFO ),
			1,
			fdInfo );
		if ( nReadSize != 1 )
		{
			printf( "[LoadInfo] 휴일정보 로드 실패 (레코드 READ)\n" );
			sResult = ERR_FILE_READ | GetFileNo( HOLIDAY_INFO_FILE );
			goto FINALLY;
		}

		DebugOutlnASC	( "휴일일자ID                            : ",
			pstHolidayInfo[i].abHolidayDate,
			sizeof( pstHolidayInfo[i].abHolidayDate ) );
		DebugOut		( "휴일구분코드                          : '%c'\n",
			pstHolidayInfo[i].bHolidayClassCode );
	}

	FINALLY:

	if ( fdInfo != NULL )
	{
		fclose( fdInfo );
	}

	if ( sResult == SUCCESS )
	{
		/*
		 * 기존 로드된 휴일정보는 FREE
		 */
		free( gpstHolidayInfo );

		/*
		 * 새로 로드된 정보를 포인터에 설정
		 */
		gpstHolidayInfo = pstHolidayInfo;

		printf( "[LoadInfo] 휴일정보           [%s] 로드 성공\n", HOLIDAY_INFO_FILE );
	}
	else
	{
		free( gpstHolidayInfo );
		free( pstHolidayInfo );
		memset( &gstHolidayInfoHeader, 0x00, sizeof( HOLIDAY_INFO_HEADER ) );
		gpstHolidayInfo = NULL;
	}

	return sResult;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      LoadNewFareInfo                                          *
*                                                                              *
*  DESCRIPTION :      요금정보 파일을 로드한다.					               *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       ERR_FILE_OPEN | GetFileNo( NEW_FARE_INFO_FILE )        *
*                       ERR_FILE_READ | GetFileNo( NEW_FARE_INFO_FILE )        *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short LoadNewFareInfo( void )
{
	FILE	*fdInfo		= NULL;
	int		nReadSize	= 0;

	TEMP_NEW_FARE_INFO	stInfo;

	/*
	 * 신요금정보파일 OPEN
	 */
	fdInfo = fopen( NEW_FARE_INFO_FILE, "rb" );
	if ( fdInfo == NULL )
	{
		printf( "[LoadInfo] 신요금정보 로드 실패 (OPEN)\n" );
		return ErrRet( ERR_FILE_OPEN | GetFileNo( NEW_FARE_INFO_FILE ) );
	}

	/*
	 * 신요금정보 레코드 READ
	 */
	nReadSize = fread( &stInfo, sizeof( stInfo ), 1, fdInfo );
	if ( nReadSize != 1 )
	{
		printf( "[LoadInfo] 신요금정보 로드 실패 (레코드 READ)\n" );
		fclose( fdInfo );
		return ErrRet( ERR_FILE_READ | GetFileNo( NEW_FARE_INFO_FILE ) );
	}

	fclose( fdInfo );

	gstNewFareInfo.tApplyDatetime = GetTimeTFromBCDDtime( stInfo.abApplyDtime );
	gstNewFareInfo.bApplySeqNo = stInfo.bApplySeqNo;
	gstNewFareInfo.wTranspMethodCode =
		GetWORDFromASC( stInfo.abTranspMethodCode,
			sizeof( stInfo.abTranspMethodCode ) );
	gstNewFareInfo.dwSingleFare =
		GetDWORDFromBCD( stInfo.abSingleFare, sizeof( stInfo.abSingleFare ) );
	gstNewFareInfo.dwBaseFare =
		GetDWORDFromBCD( stInfo.abBaseFare, sizeof( stInfo.abBaseFare ) );
	gstNewFareInfo.dwBaseDist =
		GetDWORDFromBCD( stInfo.abBaseDist, sizeof( stInfo.abBaseDist ) );
	gstNewFareInfo.dwAddedFare =
		GetDWORDFromBCD( stInfo.abAddedFare, sizeof( stInfo.abAddedFare ) );
	gstNewFareInfo.dwAddedDist =
		GetDWORDFromBCD( stInfo.abAddedDist, sizeof( stInfo.abAddedDist ) );
	gstNewFareInfo.dwOuterCityAddedDist =
		GetDWORDFromBCD( stInfo.abOuterCityAddedDist,
			sizeof( stInfo.abOuterCityAddedDist ) );
	gstNewFareInfo.dwOuterCityAddedDistFare =
		GetDWORDFromBCD( stInfo.abOuterCityAddedDistFare,
			sizeof( stInfo.abOuterCityAddedDistFare ) );
	gstNewFareInfo.dwAdultCashEntFare =
		GetDWORDFromBCD( stInfo.abAdultCashEntFare,
			sizeof( stInfo.abAdultCashEntFare ) );
	gstNewFareInfo.dwChildCashEntFare =
		GetDWORDFromBCD( stInfo.abChildCashEntFare,
			sizeof( stInfo.abChildCashEntFare ) );
	gstNewFareInfo.dwYoungCashEntFare =
		GetDWORDFromBCD( stInfo.abYoungCashEntFare,
			sizeof( stInfo.abYoungCashEntFare ) );
	gstNewFareInfo.dwPermitErrDist =
		GetDWORDFromBCD( stInfo.abPermitErrDist,
			sizeof( stInfo.abPermitErrDist ) );

	DebugOutlnTimeT	( "적용 일시                             : ",
		gstNewFareInfo.tApplyDatetime );
	DebugOut		( "적용 일련번호                         : %u\n",
		gstNewFareInfo.bApplySeqNo );
	DebugOut		( "교통수단코드                          : %u\n",
		gstNewFareInfo.wTranspMethodCode );
    DebugOut		( "단일요금 - 미사용                     : %lu\n",
		gstNewFareInfo.dwSingleFare );
    DebugOut		( "기본운임                              : %lu\n",
		gstNewFareInfo.dwBaseFare );
    DebugOut		( "기본거리                              : %lu\n",
		gstNewFareInfo.dwBaseDist );
    DebugOut		( "부가운임                              : %lu\n",
		gstNewFareInfo.dwAddedFare );
    DebugOut		( "부가거리                              : %lu\n",
		gstNewFareInfo.dwAddedDist );
    DebugOut		( "시외부가거리                          : %lu\n",
		gstNewFareInfo.dwOuterCityAddedDist );
    DebugOut		( "시외부가거리운임                      : %lu\n",
		gstNewFareInfo.dwOuterCityAddedDistFare );
    DebugOut		( "성인 현금승차요금                     : %lu\n",
		gstNewFareInfo.dwAdultCashEntFare );
    DebugOut		( "어린이 현금승차요금                   : %lu\n",
		gstNewFareInfo.dwChildCashEntFare );
    DebugOut		( "청소년 현금승차요금                   : %lu\n",
		gstNewFareInfo.dwYoungCashEntFare );
    DebugOut		( "허용오차거리 - 미사용                 : %lu\n",
		gstNewFareInfo.dwPermitErrDist );

	printf( "[LoadInfo] 신요금정보         [%s]  로드 성공\n", NEW_FARE_INFO_FILE );

	return SUCCESS;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      LoadStationInfo                                          *
*                                                                              *
*  DESCRIPTION :      버스 정류장 정보 파일을 로드한다.			               *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       ERR_FILE_READ | GetFileNo( BUS_STATION_INFO_FILE ) )   *
*                       ERR_FILE_READ | GetFileNo( BUS_STATION_INFO_FILE )     *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short LoadStationInfo( void )
{
	short	sResult			= SUCCESS;
	FILE*	fdInfo			= NULL;
	int		nRecordCntLoop	= 0;
	int		nReadSize		= 0;
	byte	abTempBuf[11]	= { 0, };

	STATION_INFO				*pstStationInfo = NULL;
	TEMP_STATION_INFO_HEADER	stTmpStationInfoHeader;
	TEMP_STATION_INFO			stTmpStationInfo;

	memset( &gstStationInfoHeader, 0x00, sizeof( gstStationInfoHeader ) );
	memset( &stTmpStationInfoHeader, 0x00, sizeof( stTmpStationInfoHeader ) );

	fdInfo = fopen( BUS_STATION_INFO_FILE,  "rb" );
	if ( fdInfo == NULL )
	{
		printf( "[LoadInfo] 정류장정보 로드 실패 (OPEN)\n" );
		sResult = ERR_FILE_OPEN | GetFileNo( BUS_STATION_INFO_FILE );
		goto FINALLY;
	}

	nReadSize = fread( &stTmpStationInfoHeader,
		sizeof( stTmpStationInfoHeader ),
		1,
		fdInfo );
	if ( nReadSize != 1 )
	{
		printf( "[LoadInfo] 정류장정보 로드 실패 (헤더 READ)\n" );
		sResult = ERR_FILE_READ | GetFileNo( BUS_STATION_INFO_FILE );
		goto FINALLY;
	}

	gstStationInfoHeader.tApplyDtime =
		GetTimeTFromBCDDtime( stTmpStationInfoHeader.abApplyDtime );
	gstStationInfoHeader.bApplySeqNo = stTmpStationInfoHeader.bApplySeqNo;
	memcpy( gstStationInfoHeader.abRouteID, stTmpStationInfoHeader.abRouteID,
		sizeof( gstStationInfoHeader.abRouteID ) );
	memcpy( gstStationInfoHeader.abTranspMethodName,
		stTmpStationInfoHeader.abTranspMethodName,
		sizeof( gstStationInfoHeader.abTranspMethodName ) );
	gstStationInfoHeader.dwRecordCnt =
		GetDWORDFromASC( stTmpStationInfoHeader.abRecordCnt,
			sizeof( stTmpStationInfoHeader.abRecordCnt ) );

	DebugOutlnTimeT ( "적용일시                              : ",
		gstStationInfoHeader.tApplyDtime );
	DebugOut        ( "적용일련번호                          : %u\n",
		gstStationInfoHeader.bApplySeqNo );
	DebugOutlnASC   ( "버스노선 ID                           : ",
		gstStationInfoHeader.abRouteID,
		sizeof( gstStationInfoHeader.abRouteID ) );
	DebugOutlnASC   ( "버스노선명                            : ",
		gstStationInfoHeader.abTranspMethodName,
		sizeof( gstStationInfoHeader.abTranspMethodName ) );
	DebugOut        ( "레코드 건수                           : %lu\n",
		gstStationInfoHeader.dwRecordCnt );
	DebugOut		( "\n" );

	pstStationInfo = ( STATION_INFO * )malloc( sizeof( STATION_INFO ) *
		gstStationInfoHeader.dwRecordCnt );
	if ( pstStationInfo == NULL )
	{
		printf( "[LoadInfo] 정류장정보 로드 실패 (메모리공간 ALLOC)\n" );
		sResult = ERR_FILE_READ | GetFileNo( BUS_STATION_INFO_FILE );
		goto FINALLY;
	}

	for ( nRecordCntLoop = 0;
		nRecordCntLoop < gstStationInfoHeader.dwRecordCnt; nRecordCntLoop++ )
	{
		nReadSize = fread( &stTmpStationInfo,
			sizeof( stTmpStationInfo ),
			1,
			fdInfo );
		if ( nReadSize != 1 )
		{
			printf( "[LoadInfo] 정류장정보 로드 실패 (레코드 READ)\n" );
			sResult = ERR_FILE_READ | GetFileNo( BUS_STATION_INFO_FILE );
			goto FINALLY;
		}

		// 마지막 정류장의 경우
		if ( nRecordCntLoop == gstStationInfoHeader.dwRecordCnt - 1  )
		{
			memcpy( gabEndStationID, stTmpStationInfo.abStationID,
				sizeof( gabEndStationID ) );
		}

		// 버스정류장 ID
		memcpy( pstStationInfo[nRecordCntLoop].abStationID,
			stTmpStationInfo.abStationID,
			sizeof( pstStationInfo[nRecordCntLoop].abStationID ) );

		// 시계내외구분 코드
		pstStationInfo[nRecordCntLoop].bCityInOutClassCode =
			stTmpStationInfo.bCityInOutClassCode;

		// 정류장순서
		pstStationInfo[nRecordCntLoop].wStationOrder =
			GetWORDFromASC( stTmpStationInfo.abStationOrder,
				sizeof( stTmpStationInfo.abStationOrder ) );

		// 버스정류장명
		memcpy( pstStationInfo[nRecordCntLoop].abStationName,
			stTmpStationInfo.abStationName,
			sizeof( pstStationInfo[nRecordCntLoop].abStationName ) );

		// 버스정류장 경도
		memset( abTempBuf, 0, sizeof( abTempBuf ) );
		memcpy( abTempBuf,
			stTmpStationInfo.abStationLongitude,
			sizeof( stTmpStationInfo.abStationLongitude ) );
		pstStationInfo[nRecordCntLoop].dStationLongitude = atof( abTempBuf );

		// 버스정류장 위도
		memset( abTempBuf, 0, sizeof( abTempBuf ) );
		memcpy( abTempBuf,
			stTmpStationInfo.abStationLatitude,
			sizeof( stTmpStationInfo.abStationLatitude ) );
		pstStationInfo[nRecordCntLoop].dStationLatitude = atof( abTempBuf );

		// offset
		pstStationInfo[nRecordCntLoop].wOffset =
		   GetWORDFromASC( stTmpStationInfo.abOffset,
			   sizeof( stTmpStationInfo.abOffset ) );

		// 첫정류장에서의 거리
		pstStationInfo[nRecordCntLoop].dwDistFromFirstStation =
			GetDWORDFromBCD( stTmpStationInfo.abDistFromFirstStation,
				 sizeof( stTmpStationInfo.abDistFromFirstStation ) );

		// 정류장 진입각
		pstStationInfo[nRecordCntLoop].wStationApproachAngle =
		   GetWORDFromASC( stTmpStationInfo.abStationApproachAngle,
			   sizeof( stTmpStationInfo.abStationApproachAngle ) );

		DebugOutlnASC	( "버스정류장 ID                         : ",
			pstStationInfo[nRecordCntLoop].abStationID,
			sizeof( pstStationInfo[nRecordCntLoop].abStationID ) );
		DebugOut		( "시계내외구분 코드                     : '%c'\n",
			pstStationInfo[nRecordCntLoop].bCityInOutClassCode );
		DebugOut		( "정류장순서                            : %u\n",
			pstStationInfo[nRecordCntLoop].wStationOrder );
		DebugOutlnASC	( "버스정류장명                          : ",
			pstStationInfo[nRecordCntLoop].abStationName,
			sizeof( pstStationInfo[nRecordCntLoop].abStationName ) );
		DebugOut		( "버스정류장 경도                       : %f\n",
			pstStationInfo[nRecordCntLoop].dStationLongitude );
		DebugOut		( "버스정류장 위도                       : %f\n",
			pstStationInfo[nRecordCntLoop].dStationLatitude );
		DebugOut		( "offset                                : %u\n",
			pstStationInfo[nRecordCntLoop].wOffset );
		DebugOut		( "첫정류장에서의 거리                   : %lu\n",
			pstStationInfo[nRecordCntLoop].dwDistFromFirstStation );
		DebugOut		( "정류장 진입각                         : %u\n",
			pstStationInfo[nRecordCntLoop].wStationApproachAngle );
	}

	FINALLY:

	if ( fdInfo != NULL )
	{
		fclose( fdInfo );
	}

	if ( sResult == SUCCESS )
	{
		/*
		 * 기존 로드된 정류장정보는 FREE
		 */
		free( gpstStationInfo );

		/*
		 * 새로 로드된 정보를 포인터에 설정
		 */
		gpstStationInfo = pstStationInfo;

		printf( "[LoadInfo] 정류장정보         [%s] 로드 성공\n", BUS_STATION_INFO_FILE );
	}
	else
	{
		free( gpstStationInfo );
		free( pstStationInfo );
		memset( &gstStationInfoHeader, 0x00, sizeof( STATION_INFO_HEADER ) );
		gpstStationInfo = NULL;
	}

	return sResult;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      LoadInstallInfo                                          *
*                                                                              *
*  DESCRIPTION :      setup.dat에 등록된 집계시스템 IP와 차량 ID를 로드한다.   *
*                                                                              *
*  INPUT PARAMETERS:  none                                                     *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       ERR_FILE_OPEN | GetFileNo( SETUP_BACKUP_FILE )         *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short LoadInstallInfo( void )
{
	FILE* fdSetup = NULL;
	int nReadSize = 0;
	int nIP1 = 0;		// IP first group
	int nIP2 = 0;		// IP second group
	int nIP3 = 0;		// IP third group
	int nIP4 = 0;		// IP fourth group

	fdSetup = fopen( SETUP_FILE, "rb" );
	if ( fdSetup == NULL )
	{
		fdSetup = fopen( SETUP_BACKUP_FILE, "rb" );
        if ( fdSetup == NULL )
        {
            return ErrRet( ERR_FILE_OPEN | GetFileNo( SETUP_BACKUP_FILE ) );
        }
	}

	nReadSize = fread( &gstCommInfo, sizeof( gstCommInfo ), 1, fdSetup );

	fclose(fdSetup);

	if ( nReadSize != 1 )
	{
		return ErrRet( ERR_FILE_READ | GetFileNo( SETUP_BACKUP_FILE ) );
	}

	nIP1 = GetINTFromASC( &gstCommInfo.abDCSIPAddr[0], IP_CLASS_LENGTH );
	nIP2 = GetINTFromASC( &gstCommInfo.abDCSIPAddr[3], IP_CLASS_LENGTH );
	nIP3 = GetINTFromASC( &gstCommInfo.abDCSIPAddr[6], IP_CLASS_LENGTH );
	nIP4 = GetINTFromASC( &gstCommInfo.abDCSIPAddr[9], IP_CLASS_LENGTH );

	memset( gabDCSIPAddr, 0x00, sizeof( gabDCSIPAddr ) );
	sprintf( gabDCSIPAddr, "%d.%d.%d.%d", nIP1, nIP2, nIP3, nIP4 );

	memcpy( gstVehicleParm.abVehicleID, gstCommInfo.abVehicleID,
		sizeof( gstVehicleParm.abVehicleID ) );

	DebugOutlnASC	( "차량ID                                : ",
		gstCommInfo.abVehicleID,
		sizeof( gstCommInfo.abVehicleID ) );
	DebugOutlnASC	( "집계서버IP                            : ",
		gstCommInfo.abDCSIPAddr,
		sizeof( gstCommInfo.abDCSIPAddr ) );
	DebugOutlnASC	( "집계서버IP                            : ",
		gabDCSIPAddr, sizeof( gabDCSIPAddr ) );

	printf( "[LoadInfo] 설치정보           [%s]    로드 성공\n", SETUP_FILE );

	return SUCCESS;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      GetLEAPPasswd                                            *
*                                                                              *
*  DESCRIPTION :      This program gets LEAP password.                         *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       ERR_FILE_OPEN | GetFileNo( TC_LEAP_FILE )              *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short GetLEAPPasswd( void )
{

    byte    abFileName[30] 	= { 0,};
    int     nLoopCnt    	= 0;
    int     nTmpLoop    	= 0;

    FILE    	*fdSetup;
	LEAP_PASSWD	stLeapPasswd;

    strcpy( abFileName, TC_LEAP_FILE );
    memset( &stLeapPasswd, 0x00, sizeof( stLeapPasswd ) );

    /*
     *  tc_leap.dat를 읽는데 실패하면 tc_leap.backup을 읽는다
     */
    do
    {
        fdSetup = fopen( abFileName, "rb" );

        if ( fdSetup != NULL )
        {

            if ( fread( &stLeapPasswd,
                        sizeof( stLeapPasswd ),
                        1,
                        fdSetup
                      ) == 1 )
            {
                for ( nTmpLoop = 0 ;
                      nTmpLoop < sizeof( stLeapPasswd.abLEAPPasswd ) ;
                      nTmpLoop++ )
                {
                    if ( stLeapPasswd.abLEAPPasswd[nTmpLoop] == 0xff )
                    {
                        break;
                    }
                }

                memset( gabLEAPPasswd, 0x00, sizeof( gabLEAPPasswd ) );
                memcpy( gabLEAPPasswd, stLeapPasswd.abLEAPPasswd, nTmpLoop );

                fclose( fdSetup );

                return SUCCESS;
            }

            fclose( fdSetup );

        }

        /*
         *  tc_leap.dat가 open 실패 시
         */
        memset( abFileName, 0x00, sizeof( abFileName ) );
        strncpy( abFileName,
                 TC_LEAP_BACKUP_FILE,
                 sizeof( TC_LEAP_BACKUP_FILE ) );
        nLoopCnt++;

    } while ( nLoopCnt < 2 );

    return ErrRet( ERR_FILE_OPEN | GetFileNo( TC_LEAP_FILE ) );
}

static short LoadHardCodedNewFareInfo( void )
{
	memset( &gstNewFareInfo, 0x00, sizeof( NEW_FARE_INFO ) );

	/*
	 *  교통수단코드
	 */
	gstNewFareInfo.wTranspMethodCode = gstVehicleParm.wTranspMethodCode;

	/*
	 *  단일요금 (미사용)
	 */
	gstNewFareInfo.dwSingleFare =
		GetHardCodedBaseFare( gstVehicleParm.wTranspMethodCode );

	/*
	 *  기본운임
	 */
	gstNewFareInfo.dwBaseFare =
		GetHardCodedBaseFare( gstVehicleParm.wTranspMethodCode );

	/*
	 *  기본거리
	 */
	gstNewFareInfo.dwBaseDist = 10000;

	/*
	 *  부가운임
	 */
	gstNewFareInfo.dwAddedFare = 100;

	/*
	 *  부가거리
	 */
	gstNewFareInfo.dwAddedDist = 5000;

	/*
	 *  시외부가거리 (미사용)
	 */
	gstNewFareInfo.dwOuterCityAddedDist = 0;

	/*
	 *  시외부가거리운임 (미사용)
	 */
	gstNewFareInfo.dwOuterCityAddedDistFare = 0;

	/*
	 *  성인 현금승차요금
	 */
	gstNewFareInfo.dwAdultCashEntFare = 0;

	/*
	 *  어린이 현금승차요금
	 */
	gstNewFareInfo.dwChildCashEntFare = 0;

	/*
	 *  청소년 현금승차요금
	 */
	gstNewFareInfo.dwYoungCashEntFare = 0;

	/*
	 *  허용오차거리 (미사용)
	 */
	gstNewFareInfo.dwPermitErrDist = 0;

	PrintlnTimeT	( "적용 일시                             : ",
		gstNewFareInfo.tApplyDatetime );
	printf			( "적용 일련번호                         : %u\n",
		gstNewFareInfo.bApplySeqNo );
	printf			( "교통수단코드                          : %u\n",
		gstNewFareInfo.wTranspMethodCode );
    printf			( "단일요금 - 미사용                     : %lu\n",
		gstNewFareInfo.dwSingleFare );
    printf			( "기본운임                              : %lu\n",
		gstNewFareInfo.dwBaseFare );
    printf			( "기본거리                              : %lu\n",
		gstNewFareInfo.dwBaseDist );
    printf			( "부가운임                              : %lu\n",
		gstNewFareInfo.dwAddedFare );
    printf			( "부가거리                              : %lu\n",
		gstNewFareInfo.dwAddedDist );
    printf			( "시외부가거리                          : %lu\n",
		gstNewFareInfo.dwOuterCityAddedDist );
    printf			( "시외부가거리운임                      : %lu\n",
		gstNewFareInfo.dwOuterCityAddedDistFare );
    printf			( "성인 현금승차요금                     : %lu\n",
		gstNewFareInfo.dwAdultCashEntFare );
    printf			( "어린이 현금승차요금                   : %lu\n",
		gstNewFareInfo.dwChildCashEntFare );
    printf			( "청소년 현금승차요금                   : %lu\n",
		gstNewFareInfo.dwYoungCashEntFare );
    printf			( "허용오차거리 - 미사용                 : %lu\n",
		gstNewFareInfo.dwPermitErrDist );

	printf( "[LoadInfo] 하드코딩된 신요금정보 로드\n" );

	return ErrRet( ERR_FILE_OPEN | GetFileNo( NEW_FARE_INFO_FILE ) );
}
