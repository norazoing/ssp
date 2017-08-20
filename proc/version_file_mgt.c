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
*  PROGRAM ID :       version_file_mgt.c                                       *
*                                                                              *
*  DESCRIPTION:       이 프로그램은 버전정보파일의 CRUD의 기능의 함수를 제공한다. *
*                                                                              *
*  ENTRY POINT:     short LoadVerInfo( VER_INFO* pstVerInfo );                 *
*					short UpdateVerFile( void );                 			   *
*					short SaveVerFile( void );                 				   *
*					void InitVerInfo( void );								   *
*																			   *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: None                                                     *
*                                                                              *
*  INPUT FILES:       c_ve_inf.dat                                             *
*                                                                              *
*  OUTPUT FILES:      c_ve_inf.dat      - 단말기 내에서 사용하는 버전정보파일    *
*					  version.trn		- 집계시스템으로 업로드해줄 버전정보파일 *
*                     bl_pl_ctrl.flg    - BLPL을 다운여부를 세팅해주는 파일      *
*										  blpl_proc.c에서 참고함				   *
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
#include "version_file_mgt.h"
#include "version_mgt.h"
#include "download_file_mgt.h"
#include "main.h"
#include "check_valid_file_mgt.h"
#include "upload_file_mgt.h"
#include "main_process_busTerm.h"

/*******************************************************************************
*  Declaration of variables                                                    *
*******************************************************************************/
#define TEMP_VER_INFO_FILE          "c_ve_inf.tmp"

bool boolIsCurr  					= 0;	// 0 - current, 1 - new
bool boolIsApplyNextVer 			= 0;    // 0 - cannot apply, 1 - can apply
bool boolIsApplyNextVerParm 		= 0;
bool boolIsApplyNextVerVoice 		= 0;
bool boolIsApplyNextVerAppl 		= 0;
bool boolIsApplyNextVerDriverAppl 	= 0;

VER_INFO			stVerInfo;		// 버전정보 구조체

/*******************************************************************************
*  Declaration of Module Functions                                             *
*******************************************************************************/
static short UpdateVerStruct( char* pchFileName,
                              DOWN_FILE_INFO* stDownFileInfo,
                              int nDCSCommFileNo,
                              bool boolIsCurr );
                              
/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      LoadVerInfo                                          `   *
*                                                                              *
*  DESCRIPTION :      c_ve_inf.dat 파일의 버전정보를 로드한다.		           *
*                                                                              *
*  INPUT PARAMETERS:  VER_INFO* pstVerInfo                                     *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS	                                               *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short LoadVerInfo( VER_INFO* pstVerInfo )
{
    FILE*	fdFile;
    TEMP_VER_INFO 	stTmpVerInfo;			// 버전정보 구조체( read용 )

    fdFile = fopen( VER_INFO_FILE,  "rb" );

    if( fdFile == NULL )
    {
        DebugOut( "File Cannot Open [%s] \n",  VER_INFO_FILE );

        fdFile = fopen( VER_INFO_FILE,  "wb" );
        memset( &stTmpVerInfo, 0x00, sizeof( stTmpVerInfo ) );

        stTmpVerInfo.abCrLf[0] = CR;
        stTmpVerInfo.abCrLf[1] = LF;

        fwrite( &stTmpVerInfo,  sizeof( stTmpVerInfo ), 1,  fdFile );

       	memset( pstVerInfo, 0x00, sizeof( stVerInfo ) );

    }
    else
    {
        memset( &stTmpVerInfo, 0x00, sizeof( stTmpVerInfo ) );
        memset( &stVerInfo, 0x00, sizeof( stVerInfo ) );

        if(  fread( &stTmpVerInfo,
        			sizeof( stTmpVerInfo ),
        			1,
                    fdFile
                  ) != 0 )
        {
            BCD2ASC( (byte*)&stTmpVerInfo.abVerInfoCreateDtime,
                     stVerInfo.abVerInfoCreateDtime,
                     sizeof( stTmpVerInfo.abVerInfoCreateDtime ) );
                     // 0 version data creation date( BCD )
            BCD2ASC( stTmpVerInfo.abMasterBLVer,
                     stVerInfo.abMasterBLVer,
                     sizeof( stTmpVerInfo.abMasterBLVer ) );
                     // 7 master B/L version( BCD )
            stVerInfo.dwMasterBLPkt = stTmpVerInfo.dwMasterBLPkt;
            BCD2ASC( stTmpVerInfo.abUpdateBLVer,
                     stVerInfo.abUpdateBLVer,
                     sizeof( stTmpVerInfo.abUpdateBLVer ) );
                     // 14 update B/L version( BCD )
            stVerInfo.dwUpdateBLPkt = stTmpVerInfo.dwUpdateBLPkt;
            BCD2ASC( stTmpVerInfo.abHotBLVer,
                     stVerInfo.abHotBLVer,
                     sizeof( stTmpVerInfo.abHotBLVer ) );
                     // 21 HOT B/L version( BCD )
            stVerInfo.dwHotBLPkt = stTmpVerInfo.dwHotBLPkt;

            BCD2ASC( stTmpVerInfo.abMasterPrepayPLVer,
                     stVerInfo.abMasterPrepayPLVer,
                     sizeof( stTmpVerInfo.abMasterPrepayPLVer ) );
                     // 28 master P/L version ( prepay )( BCD )
            stVerInfo.dwMasterPrepayPLPkt = stTmpVerInfo.dwMasterPrepayPLPkt;
            BCD2ASC( stTmpVerInfo.abMasterPostpayPLVer,
                     stVerInfo.abMasterPostpayPLVer,
                     sizeof( stTmpVerInfo.abMasterPostpayPLVer ) );
                     // 35 master P/L version ( postpay )( BCD )
            stVerInfo.dwMasterPostpayPLPkt = stTmpVerInfo.dwMasterPostpayPLPkt;
            BCD2ASC( stTmpVerInfo.abUpdatePLVer,
                     stVerInfo.abUpdatePLVer,
                     sizeof( stTmpVerInfo.abUpdatePLVer ) );
                     // 42 update PL version( BCD )
            stVerInfo.dwUpdatePLPkt = stTmpVerInfo.dwUpdatePLPkt;
            BCD2ASC( stTmpVerInfo.abHotPLVer,
                     stVerInfo.abHotPLVer,
                     sizeof( stTmpVerInfo.abHotPLVer ) );
                     // 49 HOT P/L version( BCD )
            stVerInfo.dwHotPLVerPkt = stTmpVerInfo.dwHotPLVerPkt;

            BCD2ASC( stTmpVerInfo.abMasterAIVer,
                     stVerInfo.abMasterAIVer,
                     sizeof( stTmpVerInfo.abMasterAIVer ) );
                     // 56 master AI version ( new transportation )( BCD )
            stVerInfo.dwMasterAIPkt = stTmpVerInfo.dwMasterAIPkt;
            BCD2ASC( stTmpVerInfo.abUpdateAIVer,
                     stVerInfo.abUpdateAIVer,
                     sizeof( stTmpVerInfo.abUpdateAIVer ) );
                     // 63 update AI version ( new transportation ) ( BCD )
            stVerInfo.dwUpdateAIPkt = stTmpVerInfo.dwUpdateAIPkt;
            BCD2ASC( stTmpVerInfo.abHotAIVer,
                     stVerInfo.abHotAIVer,
                     sizeof( stTmpVerInfo.abHotAIVer ) );
                     // 70 HOT AI version ( new transportation )(BCD)
            stVerInfo.dwHotAIPkt = stTmpVerInfo.dwHotAIPkt;
            BCD2ASC( stTmpVerInfo.abVehicleParmVer,
                     stVerInfo.abVehicleParmVer,
                     sizeof( stTmpVerInfo.abVehicleParmVer ) );
                     // 77 vehicle parameter version( BCD )
            stVerInfo.dwVehicleParmPkt = stTmpVerInfo.dwVehicleParmPkt;

            BCD2ASC( stTmpVerInfo.abNextVehicleParmVer,
                     stVerInfo.abNextVehicleParmVer,
                     sizeof( stTmpVerInfo.abNextVehicleParmVer ) );
                     // 84 vehicle parameter version next version( BCD )
            stVerInfo.dwNextVehicleParmPkt = stTmpVerInfo.dwNextVehicleParmPkt;
            BCD2ASC( stTmpVerInfo.abRouteParmVer,
                     stVerInfo.abRouteParmVer,
                     sizeof( stTmpVerInfo.abRouteParmVer ) );
                     // 91 bus terminal route parameter version
            stVerInfo.dwRouteParmPkt = stTmpVerInfo.dwRouteParmPkt;
            BCD2ASC( stTmpVerInfo.abNextRouteParmVer,
                     stVerInfo.abNextRouteParmVer,
                     sizeof( stTmpVerInfo.abNextRouteParmVer ) );
                     // 98 route parameter version apply next version( BCD )
            stVerInfo.dwNextRouteParmPkt = stTmpVerInfo.dwNextRouteParmPkt;
            BCD2ASC( stTmpVerInfo.abBasicFareInfoVer,
                     stVerInfo.abBasicFareInfoVer,
                     sizeof( stTmpVerInfo.abBasicFareInfoVer ) );
                     // 105 base fare data version( BCD )
            stVerInfo.dwBasicFareInfoPkt = stTmpVerInfo.dwBasicFareInfoPkt;
            BCD2ASC( stTmpVerInfo.abNextBasicFareInfoVer,
                     stVerInfo.abNextBasicFareInfoVer,
                     sizeof( stTmpVerInfo.abNextBasicFareInfoVer ) );
                     // 112 base fare data version apply next  version( BCD )
            stVerInfo.dwNextBasicFareInfoPkt = 
            							stTmpVerInfo.dwNextBasicFareInfoPkt;

            BCD2ASC( stTmpVerInfo.abOldFareVer,
                     stVerInfo.abOldFareVer,
                     sizeof( stTmpVerInfo.abOldFareVer ) );
                     // 119 old single fare( BCD )
            stVerInfo.dwOldFarePkt = stTmpVerInfo.dwOldFarePkt;
            BCD2ASC( stTmpVerInfo.abNextOldFareVer,
                     stVerInfo.abNextOldFareVer,
                     sizeof( stTmpVerInfo.abNextOldFareVer ) );
                     // 126 old single fare apply next version( BCD )
            stVerInfo.dwNextOldFarePkt = stTmpVerInfo.dwNextOldFarePkt;
            BCD2ASC( stTmpVerInfo.abOldAirFareVer,
                     stVerInfo.abOldAirFareVer,
                     sizeof( stTmpVerInfo.abOldAirFareVer ) );
                     // 133 old airport fare( A~C )( BCD )
            stVerInfo.dwOldAirFarePkt = stTmpVerInfo.dwOldAirFarePkt;
            BCD2ASC( stTmpVerInfo.abNextOldAirFareVer,
                     stVerInfo.abNextOldAirFareVer,
                     sizeof( stTmpVerInfo.abNextOldAirFareVer ) );
                     // 140 old airport fare( A~C ) apply next version( BCD )
            stVerInfo.dwNextOldAirFarePkt = stTmpVerInfo.dwNextOldAirFarePkt;
            BCD2ASC( stTmpVerInfo.abOldRangeFareVer,
                     stVerInfo.abOldRangeFareVer,
                     sizeof( stTmpVerInfo.abOldRangeFareVer ) );
                     // 147 old fare( BCD )
            stVerInfo.dwOldRangeFarePkt = stTmpVerInfo.dwOldRangeFarePkt;
            BCD2ASC( stTmpVerInfo.abNextOldRangeFareVer,
                     stVerInfo.abNextOldRangeFareVer,
                     sizeof( stTmpVerInfo.abNextOldRangeFareVer ) );
                     // 154 old fare apply next version( BCD )
            stVerInfo.dwNextOldRangeFarePkt =
                                            stTmpVerInfo.dwNextOldRangeFarePkt;

            BCD2ASC( stTmpVerInfo.abBusStationInfoVer,
                     stVerInfo.abBusStationInfoVer,
                     sizeof( stTmpVerInfo.abBusStationInfoVer ) );
                     // 161 bus station data version( BCD )
            stVerInfo.dwBusStationInfoPkt = stTmpVerInfo.dwBusStationInfoPkt;
            BCD2ASC( stTmpVerInfo.abNextBusStationInfoVer,
                     stVerInfo.abNextBusStationInfoVer,
                     sizeof( stTmpVerInfo.abNextBusStationInfoVer ));
                     // 168 bus station data next version ( BCD )
            stVerInfo.dwNextBusStationInfoPkt
                                        = stTmpVerInfo.dwNextBusStationInfoPkt;
            BCD2ASC( stTmpVerInfo.abPrepayIssuerInfoVer,
                     stVerInfo.abPrepayIssuerInfoVer,
                     sizeof( stTmpVerInfo.abPrepayIssuerInfoVer ) );
                     // 175 prepay issuer data version( BCD )
            stVerInfo.dwPrepayIssuerInfoPkt
                                        = stTmpVerInfo.dwPrepayIssuerInfoPkt;
            BCD2ASC( stTmpVerInfo.abNextPrepayIssuerInfoVer,
                     stVerInfo.abNextPrepayIssuerInfoVer,
                     sizeof( stTmpVerInfo.abNextPrepayIssuerInfoVer ) );
                     // 182 prepay issuer data next version( BCD )
            stVerInfo.dwNextPrepayIssuerInfoPkt
                                    = stTmpVerInfo.dwNextPrepayIssuerInfoPkt;
            BCD2ASC( stTmpVerInfo.abPostpayIssuerInfoVer,
                     stVerInfo.abPostpayIssuerInfoVer,
                     sizeof( stTmpVerInfo.abPostpayIssuerInfoVer ) );
                     // 189 postpay issuer data version( BCD )
            stVerInfo.dwPostpayIssuerInfoPkt
                                        = stTmpVerInfo.dwPostpayIssuerInfoPkt;
            BCD2ASC( stTmpVerInfo.abNextPostpayIssuerInfoVer,
                     stVerInfo.abNextPostpayIssuerInfoVer,
                     sizeof( stTmpVerInfo.abNextPostpayIssuerInfoVer ) );
                     // 196 postpay issuer data version apply next
                     // version( BCD )
            stVerInfo.dwNextPostpayIssuerInfoPkt
                                    = stTmpVerInfo.dwNextPostpayIssuerInfoPkt;
            BCD2ASC( stTmpVerInfo.abDisExtraInfoVer,
                     stVerInfo.abDisExtraInfoVer,
                     sizeof( stTmpVerInfo.abDisExtraInfoVer ) );
                     // 203 discount/extra data version( BCD )
            stVerInfo.dwDisExtraInfoPkt = stTmpVerInfo.dwDisExtraInfoPkt;
            BCD2ASC( stTmpVerInfo.abNextDisExtraInfoVer,
                     stVerInfo.abNextDisExtraInfoVer,
                     sizeof( stTmpVerInfo.abNextDisExtraInfoVer ) );
                     // 210 discount/extra data version apply next version(BCD)
            stVerInfo.dwNextDisExtraInfoPkt 
            							= stTmpVerInfo.dwNextDisExtraInfoPkt;
            BCD2ASC( stTmpVerInfo.abHolidayInfoVer,
                     stVerInfo.abHolidayInfoVer,
                     sizeof( stTmpVerInfo.abHolidayInfoVer ) );
                     // 217 holiday data version( BCD )
            stVerInfo.dwHolidayInfoPkt = stTmpVerInfo.dwHolidayInfoPkt;
            BCD2ASC( stTmpVerInfo.abNextHolidayInfoVer,
                     stVerInfo.abNextHolidayInfoVer,
                     sizeof( stTmpVerInfo.abNextHolidayInfoVer ) );
                     // 224 holiday data version apply next version( BCD )
            stVerInfo.dwNextHolidayInfoPkt = stTmpVerInfo.dwNextHolidayInfoPkt;

            BCD2ASC( stTmpVerInfo.abDriverOperatorApplVer,
                     stVerInfo.abDriverOperatorApplVer,
                     sizeof( stTmpVerInfo.abDriverOperatorApplVer ) );
                     // 231 driver operator Program( BCD )
            stVerInfo.dwDriverOperatorApplPkt
                                        = stTmpVerInfo.dwDriverOperatorApplPkt;
            BCD2ASC( stTmpVerInfo.abNextDriverOperatorApplVer,
                     stVerInfo.abNextDriverOperatorApplVer,
                     sizeof( stTmpVerInfo.abNextDriverOperatorApplVer ) );
                     // 238 driver operator Program apply next Program( BCD )
            stVerInfo.dwNextDriverOperatorApplPkt
                                    = stTmpVerInfo.dwNextDriverOperatorApplPkt;
            BCD2ASC( stTmpVerInfo.abMainTermApplVer,
                     stVerInfo.abMainTermApplVer,
                     sizeof( stTmpVerInfo.abMainTermApplVer ) );
                     // 245 main termianl Program( BCD )
            stVerInfo.dwMainTermApplPkt = stTmpVerInfo.dwMainTermApplPkt;
            BCD2ASC( stTmpVerInfo.abNextMainTermApplVer,
                     stVerInfo.abNextMainTermApplVer,
                     sizeof( stTmpVerInfo.abNextMainTermApplVer ) );
                     // 252 main termianl Program apply
                     // next Program( BCD )
            stVerInfo.dwNextMainTermApplPkt =
                                            stTmpVerInfo.dwNextMainTermApplPkt;
            BCD2ASC( stTmpVerInfo.abSubTermApplVer,
                     stVerInfo.abSubTermApplVer,
                     sizeof( stTmpVerInfo.abSubTermApplVer ) );
                     // 259 sub terminal Program( BCD )
            stVerInfo.dwSubTermApplPkt = stTmpVerInfo.dwSubTermApplPkt;
            BCD2ASC( stTmpVerInfo.abNextSubTermApplVer,
                     stVerInfo.abNextSubTermApplVer,
                     sizeof( stTmpVerInfo.abNextSubTermApplVer ) );
                     // 266 sub terminal Program apply next Program( BCD )
            stVerInfo.dwNextSubTermApplPkt = stTmpVerInfo.dwNextSubTermApplPkt;
            BCD2ASC( stTmpVerInfo.abVoiceInfoVer,
                     stVerInfo.abVoiceInfoVer,
                     sizeof( stTmpVerInfo.abVoiceInfoVer ) );
                     // 273 voice data version( BCD )
            stVerInfo.dwVoiceInfoPkt = stTmpVerInfo.dwVoiceInfoPkt;
            BCD2ASC( stTmpVerInfo.abNextVoiceInfoVer,
                     stVerInfo.abNextVoiceInfoVer,
                     sizeof( stTmpVerInfo.abNextVoiceInfoVer ) );
                     // 280 voice data version apply next version ( BCD )
            stVerInfo.dwNextVoiceInfoPkt = stTmpVerInfo.dwNextVoiceInfoPkt;

            BCD2ASC( stTmpVerInfo.abXferApplyRegulationVer,
                     stVerInfo.abXferApplyRegulationVer,
                     sizeof( stTmpVerInfo.abXferApplyRegulationVer ) );
            stVerInfo.dwXferApplyRegulationPkt
                                    = stTmpVerInfo.dwXferApplyRegulationPkt;
            BCD2ASC( stTmpVerInfo.abNextXferApplyRegulationVer,
                     stVerInfo.abNextXferApplyRegulationVer,
                     sizeof( stTmpVerInfo.abNextXferApplyRegulationVer ) );
            stVerInfo.dwNextXferApplyRegulationPkt
                                    = stTmpVerInfo.dwNextXferApplyRegulationPkt;

            BCD2ASC( stTmpVerInfo.abEpurseIssuerRegistInfoVer,
                     stVerInfo.abEpurseIssuerRegistInfoVer,
                     sizeof( stTmpVerInfo.abEpurseIssuerRegistInfoVer ) );
            stVerInfo.dwEpurseIssuerRegistInfoPkt
                                    = stTmpVerInfo.dwEpurseIssuerRegistInfoPkt;
            BCD2ASC( stTmpVerInfo.abNextEpurseIssuerRegistInfoVer,
                     stVerInfo.abNextEpurseIssuerRegistInfoVer,
                     sizeof( stTmpVerInfo.abNextEpurseIssuerRegistInfoVer ) );
            stVerInfo.dwNextEpurseIssuerRegistInfoPkt
                                = stTmpVerInfo.dwNextEpurseIssuerRegistInfoPkt;

            BCD2ASC( stTmpVerInfo.abPSAMKeysetVer,
                     stVerInfo.abPSAMKeysetVer,
                     sizeof( stTmpVerInfo.abPSAMKeysetVer ) );
            stVerInfo.dwPSAMKeysetPkt = stTmpVerInfo.dwPSAMKeysetPkt;
            BCD2ASC( stTmpVerInfo.abNextPSAMKeysetVer,
                     stVerInfo.abNextPSAMKeysetVer,
                     sizeof( stTmpVerInfo.abNextPSAMKeysetVer ) );
            stVerInfo.dwNextPSAMKeysetPkt = stTmpVerInfo.dwNextPSAMKeysetPkt;

            BCD2ASC( stTmpVerInfo.abAutoChargeKeysetVer,
                     stVerInfo.abAutoChargeKeysetVer,
                     sizeof( stTmpVerInfo.abAutoChargeKeysetVer ) );
            stVerInfo.dwAutoChargeKeysetPkt =
                                            stTmpVerInfo.dwAutoChargeKeysetPkt;
            BCD2ASC( stTmpVerInfo.abNextAutoChargeKeysetVer,
                     stVerInfo.abNextAutoChargeKeysetVer,
                     sizeof( stTmpVerInfo.abNextAutoChargeKeysetVer ) );
            stVerInfo.dwNextAutoChargeKeysetPkt =
                                        stTmpVerInfo.dwNextAutoChargeKeysetPkt;

            BCD2ASC( stTmpVerInfo.abAutoChargeParmVer,
                     stVerInfo.abAutoChargeParmVer,
                     sizeof( stTmpVerInfo.abAutoChargeParmVer ) );
            stVerInfo.dwAutoChargeParmPkt = stTmpVerInfo.dwAutoChargeParmPkt;
            BCD2ASC( stTmpVerInfo.abNextAutoChargeParmVer,
                     stVerInfo.abNextAutoChargeParmVer,
                     sizeof( stTmpVerInfo.abNextAutoChargeParmVer ) );
            stVerInfo.dwNextAutoChargeParmPkt =
                                        stTmpVerInfo.dwNextAutoChargeParmPkt;

            BCD2ASC( stTmpVerInfo.abXferApplyInfoVer,
                     stVerInfo.abXferApplyInfoVer,
                     sizeof( stTmpVerInfo.abXferApplyInfoVer ) );
            stVerInfo.dwXferApplyInfoPkt = stTmpVerInfo.dwXferApplyInfoPkt;
            BCD2ASC( stTmpVerInfo.abNextXferApplyInfoVer,
                     stVerInfo.abNextXferApplyInfoVer,
                     sizeof( stTmpVerInfo.abNextXferApplyInfoVer ) );
            stVerInfo.dwNextXferApplyInfoPkt =
                                        stTmpVerInfo.dwNextXferApplyInfoPkt;

            BCD2ASC( stTmpVerInfo.abIssuerValidPeriodInfoVer,
                     stVerInfo.abIssuerValidPeriodInfoVer,
                     sizeof( stTmpVerInfo.abIssuerValidPeriodInfoVer ) );
            stVerInfo.dwIssuerValidPeriodInfoPkt =
                                    stTmpVerInfo.dwIssuerValidPeriodInfoPkt;
            BCD2ASC( stTmpVerInfo.abNextIssuerValidPeriodInfoVer,
                     stVerInfo.abNextIssuerValidPeriodInfoVer,
                     sizeof( stTmpVerInfo.abNextIssuerValidPeriodInfoVer ) );
            stVerInfo.dwNextIssuerValidPeriodInfoPkt =
                                    stTmpVerInfo.dwNextIssuerValidPeriodInfoPkt;

            if ( gpstSharedInfo->bCommCmd != CMD_RESETUP )
            {
            	/*
            	 *	고정 BLPL파일 존재여부 체크하여 버전 세팅
            	 */
                SetVerBLPLFileExistYN();
            }

			/*
			 *	이어받기 정보 세팅
			 */
            SetMasterBLPLVer();  // version of update chain function

			/*
			 *	pstVerInfo 데이터 setting
			 */
            memcpy( pstVerInfo, &stVerInfo, sizeof( VER_INFO ) );

        }
    } // if( fdFile == NULL )
    
#if 0
    DebugOutlnASC( "stVerInfo.abVerInfoCreateDtime",
                    stVerInfo.abVerInfoCreateDtime,
                    sizeof(stVerInfo.abVerInfoCreateDtime) );
    DebugOutlnASC( "stVerInfo.abMasterBLVer",
                    stVerInfo.abMasterBLVer, sizeof(stVerInfo.abMasterBLVer) );
    DebugOut( "stVerInfo.dwMasterBLPkt [%lu] \n",
             stVerInfo.dwMasterBLPkt );
    DebugOutlnASC( "stVerInfo.abUpdateBLVer",
                    stVerInfo.abUpdateBLVer, sizeof(stVerInfo.abUpdateBLVer) );
    DebugOut( "stVerInfo.dwUpdateBLPkt [%lu] \n",
                stVerInfo.dwUpdateBLPkt );
    DebugOutlnASC( "stVerInfo.abHotBLVer",
                    stVerInfo.abHotBLVer, sizeof(stVerInfo.abHotBLVer) );
    DebugOut( "stVerInfo.dwHotBLPkt [%lu] \n",
                stVerInfo.dwHotBLPkt );
    DebugOutlnASC( "stVerInfo.abMasterPrepayPLVer",
                    stVerInfo.abMasterPrepayPLVer,
                    sizeof(stVerInfo.abMasterPrepayPLVer) );
    DebugOut( "stVerInfo.dwMasterPrepayPLPkt  [%lu] \n",
                stVerInfo.dwMasterPrepayPLPkt );
    DebugOutlnASC( "stVerInfo.abMasterPostpayPLVer",
                    stVerInfo.abMasterPostpayPLVer,
                    sizeof(stVerInfo.abMasterPostpayPLVer) );
    DebugOut( "stVerInfo.dwMasterPostpayPLPkt  [%lu] \n",
                stVerInfo.dwMasterPostpayPLPkt );
    DebugOutlnASC( "stVerInfo.abUpdatePLVer",
                    stVerInfo.abUpdatePLVer, sizeof(stVerInfo.abUpdatePLVer) );
    DebugOut( "stVerInfo.dwUpdatePLPkt  [%lu] \n",
                stVerInfo.dwUpdatePLPkt );
    DebugOutlnASC( "stVerInfo.abHotPLVer",
                    stVerInfo.abHotPLVer, sizeof(stVerInfo.abHotPLVer) );
    DebugOut( "stVerInfo.dwHotPLVerPkt  [%lu] \n",
                stVerInfo.dwHotPLVerPkt );

    DebugOutlnASC( "stVerInfo.abMasterAIVer",
                    stVerInfo.abMasterAIVer, sizeof(stVerInfo.abMasterAIVer) );
    DebugOut( "stVerInfo.dwMasterAIPkt  [%lu] \n",
                stVerInfo.dwMasterAIPkt);
    DebugOutlnASC( "stVerInfo.abUpdateAIVer",
                    stVerInfo.abUpdateAIVer, sizeof(stVerInfo.abUpdateAIVer) );
    DebugOut( "stVerInfo.dwUpdateAIPkt  [%lu] \n",
                stVerInfo.dwUpdateAIPkt );
    DebugOutlnASC( "stVerInfo.abHotAIVer",
                    stVerInfo.abHotAIVer, sizeof(stVerInfo.abHotAIVer) );
    DebugOut( "stVerInfo.dwHotAIPkt  [%lu] \n", stVerInfo.dwHotAIPkt );
    DebugOutlnASC( "stVerInfo.abVehicleParmVer",
                    stVerInfo.abVehicleParmVer,
                    sizeof(stVerInfo.abVehicleParmVer) );
    DebugOut( "stVerInfo.dwVehicleParmPkt  [%lu] \n",
                stVerInfo.dwVehicleParmPkt );

    DebugOutlnASC( "stVerInfo.abNextVehicleParmVer",
                    stVerInfo.abNextVehicleParmVer,
                    sizeof(stVerInfo.abNextVehicleParmVer) );
    DebugOut( "stVerInfo.dwNextVehicleParmPkt  [%lu] \n",
                stVerInfo.dwNextVehicleParmPkt );
    DebugOutlnASC( "stVerInfo.abRouteParmVer",
                    stVerInfo.abRouteParmVer,
                    sizeof(stVerInfo.abRouteParmVer) );
    DebugOut( "stVerInfo.dwRouteParmPkt  [%lu] \n",
                stVerInfo.dwRouteParmPkt);
    DebugOutlnASC( "stVerInfo.abNextRouteParmVer",
                    stVerInfo.abNextRouteParmVer,
                    sizeof(stVerInfo.abNextRouteParmVer) );
    DebugOut( "stVerInfo.dwNextRouteParmPkt  [%lu] \n",
                stVerInfo.dwNextRouteParmPkt );
    DebugOutlnASC( "stVerInfo.abBasicFareInfoVer",
                    stVerInfo.abBasicFareInfoVer,
                    sizeof(stVerInfo.abBasicFareInfoVer) );
    DebugOut( "stVerInfo.dwBasicFareInfoPkt  [%lu] \n",
                stVerInfo.dwBasicFareInfoPkt );
    DebugOutlnASC( "stVerInfo.abNextBasicFareInfoVer",
                    stVerInfo.abNextBasicFareInfoVer,
                    sizeof(stVerInfo.abNextBasicFareInfoVer) );
    DebugOut( "stVerInfo.dwNextBasicFareInfoPkt  [%lu] \n",
                stVerInfo.dwNextBasicFareInfoPkt );

    DebugOutlnASC( "stVerInfo.abOldFareVer",
                    stVerInfo.abOldFareVer, sizeof(stVerInfo.abOldFareVer) );
    DebugOut( "stVerInfo.dwOldFarePkt [%lu] \n",
                stVerInfo.dwOldFarePkt );
    DebugOutlnASC( "stVerInfo.abNextOldFareVer",
                    stVerInfo.abNextOldFareVer,
                    sizeof(stVerInfo.abNextOldFareVer) );
    DebugOut( "stVerInfo.dwNextOldFarePkt [%lu] \n",
                stVerInfo.dwNextOldFarePkt );
    DebugOutlnASC( "stVerInfo.abOldAirFareVer",
                    stVerInfo.abOldAirFareVer,
                    sizeof(stVerInfo.abOldAirFareVer) );
    DebugOut( "stVerInfo.dwOldAirFarePkt [%lu] \n",
                stVerInfo.dwOldAirFarePkt );
    DebugOutlnASC( "stVerInfo.abNextOldAirFareVer",
                    stVerInfo.abNextOldAirFareVer,
                    sizeof(stVerInfo.abNextOldAirFareVer) );
    DebugOut( "stVerInfo.dwNextOldAirFarePkt [%lu] \n",
                stVerInfo.dwNextOldAirFarePkt );
    DebugOutlnASC( "stVerInfo.abOldRangeFareVer",
                    stVerInfo.abOldRangeFareVer,
                    sizeof(stVerInfo.abOldRangeFareVer) );
    DebugOut( "stVerInfo.dwOldRangeFarePkt [%lu] \n",
                stVerInfo.dwOldRangeFarePkt );
    DebugOutlnASC( "stVerInfo.abNextOldRangeFareVer",
                    stVerInfo.abNextOldRangeFareVer,
                    sizeof(stVerInfo.abNextOldRangeFareVer) );
    DebugOut( "stVerInfo.dwNextOldRangeFarePkt [%lu] \n",
                stVerInfo.dwNextOldRangeFarePkt );
    DebugOutlnASC( "stVerInfo.abBusStationInfoVer",
                    stVerInfo.abBusStationInfoVer,
                    sizeof(stVerInfo.abBusStationInfoVer) );
    DebugOut( "stVerInfo.dwBusStationInfoPkt [%lu] \n",
                stVerInfo.dwBusStationInfoPkt );
    DebugOutlnASC( "stVerInfo.abNextBusStationInfoVer",
                    stVerInfo.abNextBusStationInfoVer,
                    sizeof(stVerInfo.abNextBusStationInfoVer) );
    DebugOut( "stVerInfo.dwNextBusStationInfoPkt [%lu] \n",
                stVerInfo.dwNextBusStationInfoPkt );
    DebugOutlnASC( "stVerInfo.abPrepayIssuerInfoVer",
                    stVerInfo.abPrepayIssuerInfoVer,
                    sizeof(stVerInfo.abPrepayIssuerInfoVer) );
    DebugOut( "stVerInfo.dwPrepayIssuerInfoPkt [%lu] \n",
                stVerInfo.dwPrepayIssuerInfoPkt );
    DebugOutlnASC( "stVerInfo.abNextPrepayIssuerInfoVer",
                    stVerInfo.abNextPrepayIssuerInfoVer,
                    sizeof(stVerInfo.abNextPrepayIssuerInfoVer) );
    DebugOut( "stVerInfo.dwNextPrepayIssuerInfoPkt [%lu] \n",
                stVerInfo.dwNextPrepayIssuerInfoPkt );
    DebugOutlnASC( "stVerInfo.abPostpayIssuerInfoVer",
                    stVerInfo.abPostpayIssuerInfoVer,
                    sizeof(stVerInfo.abPostpayIssuerInfoVer) );
    DebugOut( "stVerInfo.dwPostpayIssuerInfoPkt [%lu] \n",
                stVerInfo.dwPostpayIssuerInfoPkt );
    DebugOutlnASC( "stVerInfo.abNextPostpayIssuerInfoVer",
                    stVerInfo.abNextPostpayIssuerInfoVer,
                    sizeof(stVerInfo.abNextPostpayIssuerInfoVer) );
    DebugOut( "stVerInfo.dwNextPostpayIssuerInfoPkt [%lu] \n",
                stVerInfo.dwNextPostpayIssuerInfoPkt );
    DebugOutlnASC( "stVerInfo.abDisExtraInfoVer",
                    stVerInfo.abDisExtraInfoVer,
                    sizeof(stVerInfo.abDisExtraInfoVer) );
    DebugOut( "stVerInfo.dwDisExtraInfoPkt [%lu] \n",
                stVerInfo.dwDisExtraInfoPkt );
    DebugOutlnASC( "stVerInfo.abNextDisExtraInfoVer",
                    stVerInfo.abNextDisExtraInfoVer,
                    sizeof(stVerInfo.abNextDisExtraInfoVer) );
    DebugOut( "stVerInfo.dwNextDisExtraInfoPkt [%lu] \n",
                stVerInfo.dwNextDisExtraInfoPkt );
    DebugOutlnASC( "stVerInfo.abHolidayInfoVer",
                    stVerInfo.abHolidayInfoVer,
                    sizeof(stVerInfo.abHolidayInfoVer) );
    DebugOut( "stVerInfo.dwHolidayInfoPkt [%lu] \n",
                stVerInfo.dwHolidayInfoPkt );
    DebugOutlnASC( "stVerInfo.abNextHolidayInfoVer",
                    stVerInfo.abNextHolidayInfoVer,
                    sizeof(stVerInfo.abNextHolidayInfoVer) );
    DebugOut( "stVerInfo.dwNextHolidayInfoPkt [%lu] \n",
                stVerInfo.dwNextHolidayInfoPkt );
    DebugOutlnASC( "stVerInfo.abDriverOperatorApplVer",
                    stVerInfo.abDriverOperatorApplVer,
                    sizeof(stVerInfo.abDriverOperatorApplVer) );
    DebugOut( "stVerInfo.dwDriverOperatorApplPkt [%lu] \n",
                stVerInfo.dwDriverOperatorApplPkt );
    DebugOutlnASC( "stVerInfo.abNextDriverOperatorApplVer",
                    stVerInfo.abNextDriverOperatorApplVer,
                    sizeof(stVerInfo.abNextDriverOperatorApplVer) );
    DebugOut( "stVerInfo.dwNextDriverOperatorApplPkt [%lu] \n",
                stVerInfo.dwNextDriverOperatorApplPkt );
    DebugOutlnASC( "stVerInfo.abMainTermApplVer",
                    stVerInfo.abMainTermApplVer,
                    sizeof(stVerInfo.abMainTermApplVer) );
    DebugOut( "stVerInfo.dwMainTermApplPkt [%lu] \n",
                stVerInfo.dwMainTermApplPkt );
    DebugOutlnASC( "stVerInfo.abNextMainTermApplVer",
                    stVerInfo.abNextMainTermApplVer,
                    sizeof(stVerInfo.abNextMainTermApplVer) );
    DebugOut( "stVerInfo.dwNextMainTermApplPkt [%lu] \n",
                stVerInfo.dwNextMainTermApplPkt );
    DebugOutlnASC( "stVerInfo.abSubTermApplVer",
                    stVerInfo.abSubTermApplVer,
                    sizeof(stVerInfo.abSubTermApplVer) );
    DebugOut( "stVerInfo.dwSubTermApplPkt [%lu] \n",
                stVerInfo.dwSubTermApplPkt );
    DebugOutlnASC( "stVerInfo.abNextSubTermApplVer,",
                    stVerInfo.abNextSubTermApplVer,
                    sizeof(stVerInfo.abNextSubTermApplVer ) );
    DebugOut( "stVerInfo.dwNextSubTermApplPkt [%lu] \n",
                stVerInfo.dwNextSubTermApplPkt );
    DebugOutlnASC( "stVerInfo.abVoiceInfoVer,",
                    stVerInfo.abVoiceInfoVer,
                    sizeof(stVerInfo.abVoiceInfoVer ) );
    DebugOut( "stVerInfo.dwVoiceInfoPkt [%lu] \n",
                stVerInfo.dwVoiceInfoPkt );
    DebugOutlnASC( "stVerInfo.abNextVoiceInfoVer,",
                    stVerInfo.abNextVoiceInfoVer,
                    sizeof(stVerInfo.abNextVoiceInfoVer ) );
    DebugOut( "stVerInfo.dwNextVoiceInfoPkt [%lu] \n",
                stVerInfo.dwNextVoiceInfoPkt );
    DebugOutlnASC( "stVerInfo.abXferApplyRegulationVer,",
                    stVerInfo.abXferApplyRegulationVer,
                    sizeof(stVerInfo.abXferApplyRegulationVer ) );
    DebugOut( "stVerInfo.dwXferApplyRegulationPkt [%lu] \n",
                stVerInfo.dwXferApplyRegulationPkt );
    DebugOutlnASC( "stVerInfo.abNextXferApplyRegulationVer,",
                    stVerInfo.abNextXferApplyRegulationVer,
                    sizeof(stVerInfo.abNextXferApplyRegulationVer ) );
    DebugOut( "stVerInfo.dwNextXferApplyRegulationPkt [%lu] \n",
                stVerInfo.dwNextXferApplyRegulationPkt );
    DebugOutlnASC( "stVerInfo.abEpurseIssuerRegistInfoVer,",
                    stVerInfo.abEpurseIssuerRegistInfoVer,
                    sizeof(stVerInfo.abEpurseIssuerRegistInfoVer ) );
    DebugOut( "stVerInfo.dwEpurseIssuerRegistInfoPkt [%lu] \n",
                stVerInfo.dwEpurseIssuerRegistInfoPkt);
    DebugOutlnASC( "stVerInfo.abNextEpurseIssuerRegistInfoVer,",
                    stVerInfo.abNextEpurseIssuerRegistInfoVer,
                    sizeof(stVerInfo.abNextEpurseIssuerRegistInfoVer ) );
    DebugOut( "stVerInfo.dwNextEpurseIssuerRegistInfoPkt [%lu] \n",
                stVerInfo.dwNextEpurseIssuerRegistInfoPkt);

    DebugOutlnASC( "stVerInfo.abPSAMKeysetVer,",
                    stVerInfo.abPSAMKeysetVer,
                    sizeof(stVerInfo.abPSAMKeysetVer ) );
    DebugOut( "stVerInfo.dwPSAMKeysetPkt [%lu] \n",
                stVerInfo.dwPSAMKeysetPkt);
    DebugOutlnASC( "stVerInfo.abNextPSAMKeysetVer,",
                    stVerInfo.abNextPSAMKeysetVer,
                    sizeof(stVerInfo.abNextPSAMKeysetVer ) );
    DebugOut( "stVerInfo.dwNextPSAMKeysetPkt [%lu] \n",
                stVerInfo.dwNextPSAMKeysetPkt);
    DebugOutlnASC( "stVerInfo.abAutoChargeKeysetVer,",
                    stVerInfo.abAutoChargeKeysetVer,
                    sizeof(stVerInfo.abAutoChargeKeysetVer ) );
    DebugOut( "stVerInfo.dwAutoChargeKeysetPkt [%lu] \n",
                stVerInfo.dwAutoChargeKeysetPkt);
    DebugOutlnASC( "stVerInfo.abNextAutoChargeKeysetVer,",
                    stVerInfo.abNextAutoChargeKeysetVer,
                    sizeof(stVerInfo.abNextAutoChargeKeysetVer ));
    DebugOut( "stVerInfo.dwNextAutoChargeKeysetPkt [%lu] \n",
                stVerInfo.dwNextAutoChargeKeysetPkt );
    DebugOutlnASC( "stVerInfo.abAutoChargeParmVer,",
                    stVerInfo.abAutoChargeParmVer,
                    sizeof(stVerInfo.abAutoChargeParmVer) );
    DebugOut( "stVerInfo.dwAutoChargeParmPkt [%lu] \n",
                stVerInfo.dwAutoChargeParmPkt);
    DebugOutlnASC( "stVerInfo.abNextAutoChargeParmVer,",
                    stVerInfo.abNextAutoChargeParmVer,
                    sizeof(stVerInfo.abNextAutoChargeParmVer) );
    DebugOut( "stVerInfo.dwNextAutoChargeParmPkt [%lu] \n",
                stVerInfo.dwNextAutoChargeParmPkt);
    DebugOutlnASC( "stVerInfo.abXferApplyInfoVer,",
                    stVerInfo.abXferApplyInfoVer,
                    sizeof(stVerInfo.abXferApplyInfoVer) );
    DebugOut( "stVerInfo.dwXferApplyInfoPkt [%lu] \n",
                stVerInfo.dwXferApplyInfoPkt);
    DebugOutlnASC( "stVerInfo.abNextXferApplyInfoVer,",
                    stVerInfo.abNextXferApplyInfoVer,
                    sizeof(stVerInfo.abNextXferApplyInfoVer) );
    DebugOut( "stVerInfo.dwNextXferApplyInfoPkt [%lu] \n",
                stVerInfo.dwNextXferApplyInfoPkt);
    DebugOutlnASC( "stVerInfo.abIssuerValidPeriodInfoVer,",
                    stVerInfo.abIssuerValidPeriodInfoVer,
                    sizeof(stVerInfo.abIssuerValidPeriodInfoVer) );
    DebugOut( "stVerInfo.dwIssuerValidPeriodInfoPkt [%lu] \n",
                stVerInfo.dwIssuerValidPeriodInfoPkt);
    DebugOutlnASC( "stVerInfo.abNextIssuerValidPeriodInfoVer,",
                    stVerInfo.abNextIssuerValidPeriodInfoVer,
                    sizeof(stVerInfo.abNextIssuerValidPeriodInfoVer) );
    DebugOut( "stVerInfo.dwNextIssuerValidPeriodInfoPkt [%lu] \n",
                stVerInfo.dwNextIssuerValidPeriodInfoPkt);
#endif

    if ( fdFile > 0 )
    {
        fclose( fdFile );
    }

    LogDCS( "[LoadVerInfo] 버전정보 로드\n" );

    return SUCCESS;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      UpdateVerFile                                            *
*                                                                              *
*  DESCRIPTION :      This program updates version file.                       *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE:   sReturnVal                                             *
*           ErrRet( sReturnVal )                                               *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short UpdateVerFile( void )
{
    short   sReturnVal 			= SUCCESS;
    int     nFileLoop   		= 0;
    int   	nCurrNext    		= 0;
    char    achDownFileName[50]	= { 0, };
    byte 	SharedMemoryCmd;
    char 	achCmdData[40] 		= { 0, };
    word 	wCmdDataSize;

    DOWN_FILE_INFO stDownFileInfo;

    DebugOut( "UpdateVerFile\n" );

	/*
	 *	BLPL 다운로드 정보 write
	 */
    WriteBLPLDownInfo();

	/*
	 *	셋업중이거나 재설치 중일 때 SAM정보 다운여부를 체크한다.
	 */
    GetSharedCmd( &SharedMemoryCmd, achCmdData, &wCmdDataSize );
    if ( SharedMemoryCmd != CMD_SETUP && SharedMemoryCmd != CMD_RESETUP )
    {
    	CheckSAMInfoDown();
    }

    for ( nCurrNext = CURR ; nCurrNext <= NEXT ; nCurrNext++ )
    {
		
        if ( nCurrNext == CURR )
        {
            memcpy( achDownFileName, "tmp_c_", 6 );
        }
        else
        {
            memcpy( achDownFileName, "tmp_n_", 6 );
        }


        for ( nFileLoop = 0 ; nFileLoop < DCS_COMM_FILE_CNT ; nFileLoop++ )
        {
        	/*
        	 *	파일명 생성
        	 */
            sprintf( &achDownFileName[6], "%s", achDCSCommFile[nFileLoop][1] );

			/*
			 *	다운로드 정보 목록 파일을 read
			 */
            sReturnVal = ReadDownFileList( achDownFileName, &stDownFileInfo );

			/*
			 *	다운로드 목록에 있으면 해당 파일의 유효성을 체크하여
			 *  유효한 파일이면 버전을 업데이트 한다.
			 */
            if ( sReturnVal == SUCCESS )
            {

                if ( CheckValidFileForDownload( nFileLoop, achDownFileName,
						&stDownFileInfo ) == SUCCESS )
                {
                    sReturnVal = UpdateVerStruct( achDownFileName,
                                                  &stDownFileInfo,
                                                  nFileLoop, nCurrNext );

					boolIsApplyNextVer = TRUE;

                    if ( nFileLoop == 0 )
                    {
                        boolIsApplyNextVerDriverAppl = TRUE;
						printf( "[UpdateVerFile] 운전자조작기F/W 버전적용\n" );
                    }
                    else if ( nFileLoop >= 1 && nFileLoop < 10 )
                    {
                        boolIsApplyNextVerAppl = TRUE;
						printf( "[UpdateVerFile] 승하차단말기F/W 버전적용\n" );
                    }
                    else if ( nFileLoop >= 10 && nFileLoop < 20 )
                    {
                        boolIsApplyNextVerVoice = TRUE;
						printf( "[UpdateVerFile] 음성정보 버전적용\n" );
                    }
                    else if ( nFileLoop >= 21 && nFileLoop < 38 )
                    {
                        boolIsApplyNextVerParm = TRUE;
						printf( "[UpdateVerFile] 운영정보 버전적용\n" );
                    }

                }

            }
        }
    }

	/*
	 *	버전정보 저장
	 */
    sReturnVal = SaveVerFile();

    return sReturnVal;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SaveVerFile                                              *
*                                                                              *
*  DESCRIPTION :      버전정보를 c_ve_inf.dat파일로 저장한다.                   *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*           ErrRet( ERR_FILE_WRITE | GetFileNo( VER_INFO_FILE ) )              *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short SaveVerFile( void )
{
    FILE*	fdFile;
    time_t 	tCurrTime 	= 0L;      // current time
    int     nWriteByte 	= 0;
    TEMP_VER_INFO    stTmpVerInfo;
	
    rename( VER_INFO_FILE, TEMP_VER_INFO_FILE );
	
    DebugOut( "SaveVerFile\n" );

    fdFile = fopen( VER_INFO_FILE,  "wb" );

    if( fdFile == NULL )
    {
        rename( TEMP_VER_INFO_FILE,  VER_INFO_FILE );
	 	fdFile = fopen( VER_INFO_FILE,  "wb" );
    }

    memset( &stTmpVerInfo, 0x00, sizeof( TEMP_VER_INFO ) );

    GetRTCTime( &tCurrTime );
    TimeT2BCDDtime( tCurrTime, stTmpVerInfo.abVerInfoCreateDtime );
                    // versiondata creation date( BCD )

    ASC2BCD( stVerInfo.abMasterBLVer,
             stTmpVerInfo.abMasterBLVer,
             sizeof( stVerInfo.abMasterBLVer ) );
             // 7 master B/L version( BCD )
    stTmpVerInfo.dwMasterBLPkt = stVerInfo.dwMasterBLPkt;
    ASC2BCD( stVerInfo.abUpdateBLVer,
             stTmpVerInfo.abUpdateBLVer,
             sizeof( stVerInfo.abUpdateBLVer ) );
             // 14 update B/L version( BCD )
    stTmpVerInfo.dwUpdateBLPkt = stVerInfo.dwUpdateBLPkt;
    ASC2BCD( stVerInfo.abHotBLVer,
             stTmpVerInfo.abHotBLVer,
             sizeof( stVerInfo.abHotBLVer ) );
             // 21 HOT B/L version( BCD )
    stTmpVerInfo.dwHotBLPkt = stVerInfo.dwHotBLPkt;

    ASC2BCD( stVerInfo.abMasterPrepayPLVer,
             stTmpVerInfo.abMasterPrepayPLVer,
             sizeof( stVerInfo.abMasterPrepayPLVer ) );
             // 28 master P/L version (  prepay  )( BCD )
    stTmpVerInfo.dwMasterPrepayPLPkt = stVerInfo.dwMasterPrepayPLPkt;
    ASC2BCD( stVerInfo.abMasterPostpayPLVer,
             stTmpVerInfo.abMasterPostpayPLVer,
             sizeof( stVerInfo.abMasterPostpayPLVer ) );
             // 35 master P/L version (  postpay )( BCD )
    stTmpVerInfo.dwMasterPostpayPLPkt = stVerInfo.dwMasterPostpayPLPkt;
    ASC2BCD( stVerInfo.abUpdatePLVer,
             stTmpVerInfo.abUpdatePLVer,
             sizeof( stVerInfo.abUpdatePLVer ) );
             // 42 update PL version( BCD )
    stTmpVerInfo.dwUpdatePLPkt = stVerInfo.dwUpdatePLPkt;
    ASC2BCD( stVerInfo.abHotPLVer,
             stTmpVerInfo.abHotPLVer,
             sizeof( stVerInfo.abHotPLVer ) );
             // 49 HOT P/L version( BCD )
    stTmpVerInfo.dwHotPLVerPkt = stVerInfo.dwHotPLVerPkt;

    ASC2BCD( stVerInfo.abMasterAIVer,
             stTmpVerInfo.abMasterAIVer,
             sizeof( stVerInfo.abMasterAIVer ) );
             // 56 master AI version (  newtransportation )( BCD )
    stTmpVerInfo.dwMasterAIPkt = stVerInfo.dwMasterAIPkt;
    ASC2BCD( stVerInfo.abUpdateAIVer,
             stTmpVerInfo.abUpdateAIVer,
             sizeof( stVerInfo.abUpdateAIVer ) );
             // 63 update AI version (  newtransportation )( BCD )
    stTmpVerInfo.dwUpdateAIPkt = stVerInfo.dwUpdateAIPkt;
    ASC2BCD( stVerInfo.abHotAIVer,
             stTmpVerInfo.abHotAIVer,
             sizeof( stVerInfo.abHotAIVer ) );
             // 70 HOT AI version (  newtransportation )( BCD )
    stTmpVerInfo.dwHotAIPkt = stVerInfo.dwHotAIPkt;
    ASC2BCD( stVerInfo.abVehicleParmVer,
             stTmpVerInfo.abVehicleParmVer,
             sizeof( stVerInfo.abVehicleParmVer ) );
             // 77  runvehicle parameter version( BCD )
    stTmpVerInfo.dwVehicleParmPkt = stVerInfo.dwVehicleParmPkt;

    ASC2BCD( stVerInfo.abNextVehicleParmVer,
             stTmpVerInfo.abNextVehicleParmVer,
             sizeof( stVerInfo.abNextVehicleParmVer ) );
             // 84  runvehicle parameter version apply next
             // version( BCD )
    stTmpVerInfo.dwNextVehicleParmPkt = stVerInfo.dwNextVehicleParmPkt;

    ASC2BCD( stVerInfo.abRouteParmVer,
             stTmpVerInfo.abRouteParmVer,
             sizeof( stVerInfo.abRouteParmVer ) );
             // 91  bus terminalrouteparameter version( BCD )
    stTmpVerInfo.dwRouteParmPkt = stVerInfo.dwRouteParmPkt;

    ASC2BCD( stVerInfo.abNextRouteParmVer,
             stTmpVerInfo.abNextRouteParmVer,
             sizeof( stVerInfo.abNextRouteParmVer ) );
             // 98  bus terminalrouteparameter version apply next version( BCD )
    stTmpVerInfo.dwNextRouteParmPkt = stVerInfo.dwNextRouteParmPkt;

    ASC2BCD( stVerInfo.abBasicFareInfoVer,
             stTmpVerInfo.abBasicFareInfoVer,
             sizeof( stVerInfo.abBasicFareInfoVer ) );
             // 105 basic faredata version( BCD )
    stTmpVerInfo.dwBasicFareInfoPkt = stVerInfo.dwBasicFareInfoPkt;

    ASC2BCD( stVerInfo.abNextBasicFareInfoVer,
             stTmpVerInfo.abNextBasicFareInfoVer,
             sizeof( stVerInfo.abNextBasicFareInfoVer ) );
             // 112 basic faredata version apply next version( BCD )
    stTmpVerInfo.dwNextBasicFareInfoPkt = stVerInfo.dwNextBasicFareInfoPkt;;

    ASC2BCD( stVerInfo.abOldFareVer,
             stTmpVerInfo.abOldFareVer,
             sizeof( stVerInfo.abOldFareVer ) );
             // 119  old singlefare( BCD )
    stTmpVerInfo.dwOldFarePkt = stVerInfo.dwOldFarePkt;

    ASC2BCD( stVerInfo.abNextOldFareVer,
             stTmpVerInfo.abNextOldFareVer,
             sizeof( stVerInfo.abNextOldFareVer ) );
             // 126  old singlefare apply next version( BCD )
    stTmpVerInfo.dwNextOldFarePkt = stVerInfo.dwNextOldFarePkt;

    ASC2BCD( stVerInfo.abOldAirFareVer,
             stTmpVerInfo.abOldAirFareVer,
             sizeof( stVerInfo.abOldAirFareVer ) );
             // 133  old airportfare( A range~C range )( BCD )
    stTmpVerInfo.dwOldAirFarePkt = stVerInfo.dwOldAirFarePkt;

    ASC2BCD( stVerInfo.abNextOldAirFareVer,
             stTmpVerInfo.abNextOldAirFareVer,
             sizeof( stVerInfo.abNextOldAirFareVer ) );
             // 140  old airportfare( A range~C range ) apply next version(BCD)
    stTmpVerInfo.dwNextOldAirFarePkt = stVerInfo.dwNextOldAirFarePkt;

    ASC2BCD( stVerInfo.abOldRangeFareVer,
             stTmpVerInfo.abOldRangeFareVer,
             sizeof( stVerInfo.abOldRangeFareVer ) );
             // 147  old  rangefare( BCD )
    stTmpVerInfo.dwOldRangeFarePkt = stVerInfo.dwOldRangeFarePkt;

    ASC2BCD( stVerInfo.abNextOldRangeFareVer,
             stTmpVerInfo.abNextOldRangeFareVer,
             sizeof( stVerInfo.abNextOldRangeFareVer ) );
             // 154  old  rangefare apply next version( BCD )
    stTmpVerInfo.dwNextOldRangeFarePkt = stVerInfo.dwNextOldRangeFarePkt;

    ASC2BCD( stVerInfo.abBusStationInfoVer,
             stTmpVerInfo.abBusStationInfoVer,
             sizeof( stVerInfo.abBusStationInfoVer ) );
             // 161  bus station간data version( BCD )
    stTmpVerInfo.dwBusStationInfoPkt = stVerInfo.dwBusStationInfoPkt;

    ASC2BCD( stVerInfo.abNextBusStationInfoVer,
             stTmpVerInfo.abNextBusStationInfoVer,
             sizeof( stVerInfo.abNextBusStationInfoVer ) );
             // 168  bus station간data version apply next version( BCD )
    stTmpVerInfo.dwNextBusStationInfoPkt = stVerInfo.dwNextBusStationInfoPkt;

    ASC2BCD( stVerInfo.abPrepayIssuerInfoVer,
             stTmpVerInfo.abPrepayIssuerInfoVer,
             sizeof( stVerInfo.abPrepayIssuerInfoVer ) );
             // 175  prepay  issuer data version( BCD )
    stTmpVerInfo.dwPrepayIssuerInfoPkt = stVerInfo.dwPrepayIssuerInfoPkt;

    ASC2BCD( stVerInfo.abNextPrepayIssuerInfoVer,
             stTmpVerInfo.abNextPrepayIssuerInfoVer,
             sizeof( stVerInfo.abNextPrepayIssuerInfoVer ) );
             // 182  prepay  issuer data version apply next version( BCD )
    stTmpVerInfo.dwNextPrepayIssuerInfoPkt
                       					 = stVerInfo.dwNextPrepayIssuerInfoPkt;

    ASC2BCD( stVerInfo.abPostpayIssuerInfoVer,
             stTmpVerInfo.abPostpayIssuerInfoVer,
             sizeof( stVerInfo.abPostpayIssuerInfoVer ) );
             // 189  postpay issuer data version( BCD )
    stTmpVerInfo.dwPostpayIssuerInfoPkt = stVerInfo.dwPostpayIssuerInfoPkt;

    ASC2BCD( stVerInfo.abNextPostpayIssuerInfoVer,
             stTmpVerInfo.abNextPostpayIssuerInfoVer,
             sizeof( stVerInfo.abNextPostpayIssuerInfoVer ) );
             // 196  postpay issuer data version apply next version( BCD )
    stTmpVerInfo.dwNextPostpayIssuerInfoPkt
                                		= stVerInfo.dwNextPostpayIssuerInfoPkt;

    ASC2BCD( stVerInfo.abDisExtraInfoVer,
             stTmpVerInfo.abDisExtraInfoVer,
             sizeof( stVerInfo.abDisExtraInfoVer ) );
             // 203  discount/extradata version( BCD )
    stTmpVerInfo.dwDisExtraInfoPkt = stVerInfo.dwDisExtraInfoPkt;

    ASC2BCD( stVerInfo.abNextDisExtraInfoVer,
             stTmpVerInfo.abNextDisExtraInfoVer,
             sizeof( stVerInfo.abNextDisExtraInfoVer ) );
             // 210  discount/extradata version apply next version( BCD )
    stTmpVerInfo.dwNextDisExtraInfoPkt = stVerInfo.dwNextDisExtraInfoPkt;

    ASC2BCD( stVerInfo.abHolidayInfoVer,
             stTmpVerInfo.abHolidayInfoVer,
             sizeof( stVerInfo.abHolidayInfoVer ) );
             // 217 holidaydata version( BCD )
    stTmpVerInfo.dwHolidayInfoPkt = stVerInfo.dwHolidayInfoPkt;

    ASC2BCD( stVerInfo.abNextHolidayInfoVer,
             stTmpVerInfo.abNextHolidayInfoVer,
             sizeof( stVerInfo.abNextHolidayInfoVer ) );
             // 224 holidaydata version apply next version( BCD )
    stTmpVerInfo.dwNextHolidayInfoPkt = stVerInfo.dwNextHolidayInfoPkt;

    ASC2BCD( stVerInfo.abDriverOperatorApplVer,
             stTmpVerInfo.abDriverOperatorApplVer,
             sizeof( stVerInfo.abDriverOperatorApplVer ) );
             // 231 driver operator Program( BCD )
    stTmpVerInfo.dwDriverOperatorApplPkt = stVerInfo.dwDriverOperatorApplPkt;

    ASC2BCD( stVerInfo.abNextDriverOperatorApplVer,
             stTmpVerInfo.abNextDriverOperatorApplVer,
             sizeof( stVerInfo.abNextDriverOperatorApplVer ) );
             // 238 driver operator Program apply next Program(BCD)
    stTmpVerInfo.dwNextDriverOperatorApplPkt
                        = stVerInfo.dwNextDriverOperatorApplPkt;

    ASC2BCD( stVerInfo.abMainTermApplVer,
             stTmpVerInfo.abMainTermApplVer,
             sizeof( stVerInfo.abMainTermApplVer ) );
             // 245 main  terminal Program( BCD )
    stTmpVerInfo.dwMainTermApplPkt = stVerInfo.dwMainTermApplPkt;

    ASC2BCD( stVerInfo.abNextMainTermApplVer,
             stTmpVerInfo.abNextMainTermApplVer,
             sizeof( stVerInfo.abNextMainTermApplVer ) );
             // 252 main  terminal Program apply next Program( BCD )
    stTmpVerInfo.dwNextMainTermApplPkt = stVerInfo.dwNextMainTermApplPkt;

    ASC2BCD( stVerInfo.abSubTermApplVer,
             stTmpVerInfo.abSubTermApplVer,
             sizeof( stVerInfo.abSubTermApplVer ) );
             // 259 sub  terminal Program( BCD )
    stTmpVerInfo.dwSubTermApplPkt = stVerInfo.dwSubTermApplPkt;

    ASC2BCD( stVerInfo.abNextSubTermApplVer,
             stTmpVerInfo.abNextSubTermApplVer,
             sizeof( stVerInfo.abNextSubTermApplVer ) );
             // 266 sub  terminal Program apply next Program( BCD )
    stTmpVerInfo.dwNextSubTermApplPkt = stVerInfo.dwNextSubTermApplPkt;

    ASC2BCD( stVerInfo.abVoiceInfoVer,
             stTmpVerInfo.abVoiceInfoVer,
             sizeof( stVerInfo.abVoiceInfoVer ) );
             // 273 voicedata version( BCD )
    stTmpVerInfo.dwVoiceInfoPkt = stVerInfo.dwVoiceInfoPkt;

    ASC2BCD( stVerInfo.abNextVoiceInfoVer,
             stTmpVerInfo.abNextVoiceInfoVer,
             sizeof( stVerInfo.abNextVoiceInfoVer ) );
             // 280 voicedata version apply next version( BCD )
    stTmpVerInfo.dwNextVoiceInfoPkt = stVerInfo.dwNextVoiceInfoPkt;

    ASC2BCD( stVerInfo.abXferApplyRegulationVer,
             stTmpVerInfo.abXferApplyRegulationVer,
             sizeof( stVerInfo.abXferApplyRegulationVer ) );
    stTmpVerInfo.dwXferApplyRegulationPkt = stVerInfo.dwXferApplyRegulationPkt;

    ASC2BCD( stVerInfo.abNextXferApplyRegulationVer,
             stTmpVerInfo.abNextXferApplyRegulationVer,
             sizeof( stVerInfo.abNextXferApplyRegulationVer ) );
    stTmpVerInfo.dwNextXferApplyRegulationPkt
                            		= stVerInfo.dwNextXferApplyRegulationPkt;

    ASC2BCD( stVerInfo.abEpurseIssuerRegistInfoVer,
             stTmpVerInfo.abEpurseIssuerRegistInfoVer,
             sizeof( stVerInfo.abEpurseIssuerRegistInfoVer ) );
    stTmpVerInfo.dwEpurseIssuerRegistInfoPkt
                            			= stVerInfo.dwEpurseIssuerRegistInfoPkt;

    ASC2BCD( stVerInfo.abNextEpurseIssuerRegistInfoVer,
             stTmpVerInfo.abNextEpurseIssuerRegistInfoVer,
             sizeof( stVerInfo.abNextEpurseIssuerRegistInfoVer ) );
    stTmpVerInfo.dwNextEpurseIssuerRegistInfoPkt
                            	= stVerInfo.dwNextEpurseIssuerRegistInfoPkt;

    ASC2BCD( stVerInfo.abPSAMKeysetVer,
             stTmpVerInfo.abPSAMKeysetVer,
             sizeof( stVerInfo.abPSAMKeysetVer ) );
    stTmpVerInfo.dwPSAMKeysetPkt    = stVerInfo.dwPSAMKeysetPkt;

    ASC2BCD( stVerInfo.abNextPSAMKeysetVer,
             stTmpVerInfo.abNextPSAMKeysetVer,
             sizeof( stVerInfo.abNextPSAMKeysetVer ) );
    stTmpVerInfo.dwNextPSAMKeysetPkt = stVerInfo.dwNextPSAMKeysetPkt;

    ASC2BCD( stVerInfo.abAutoChargeKeysetVer,
             stTmpVerInfo.abAutoChargeKeysetVer,
             sizeof( stVerInfo.abAutoChargeKeysetVer ) );
    stTmpVerInfo.dwAutoChargeKeysetPkt = stVerInfo.dwAutoChargeKeysetPkt;

    ASC2BCD( stVerInfo.abNextAutoChargeKeysetVer,
             stTmpVerInfo.abNextAutoChargeKeysetVer,
             sizeof( stVerInfo.abNextAutoChargeKeysetVer ) );
    stTmpVerInfo.dwNextAutoChargeKeysetPkt
                            			= stVerInfo.dwNextAutoChargeKeysetPkt;

    ASC2BCD( stVerInfo.abAutoChargeParmVer,
             stTmpVerInfo.abAutoChargeParmVer,
             sizeof( stVerInfo.abAutoChargeParmVer ) );
    stTmpVerInfo.dwAutoChargeParmPkt = stVerInfo.dwAutoChargeParmPkt;

    ASC2BCD( stVerInfo.abNextAutoChargeParmVer,
             stTmpVerInfo.abNextAutoChargeParmVer,
             sizeof( stVerInfo.abNextAutoChargeParmVer ) );
    stTmpVerInfo.dwNextAutoChargeParmPkt = stVerInfo.dwNextAutoChargeParmPkt;

    ASC2BCD( stVerInfo.abXferApplyInfoVer,
             stTmpVerInfo.abXferApplyInfoVer,
             sizeof( stVerInfo.abXferApplyInfoVer ) );
    stTmpVerInfo.dwXferApplyInfoPkt = stVerInfo.dwXferApplyInfoPkt;

    ASC2BCD( stVerInfo.abNextXferApplyInfoVer,
             stTmpVerInfo.abNextXferApplyInfoVer,
             sizeof( stVerInfo.abNextXferApplyInfoVer ) );
    stTmpVerInfo.dwNextXferApplyInfoPkt = stVerInfo.dwNextXferApplyInfoPkt;

    ASC2BCD( stVerInfo.abIssuerValidPeriodInfoVer,
             stTmpVerInfo.abIssuerValidPeriodInfoVer,
             sizeof( stVerInfo.abIssuerValidPeriodInfoVer ) );
    stTmpVerInfo.dwIssuerValidPeriodInfoPkt
                                    	= stVerInfo.dwIssuerValidPeriodInfoPkt;

    ASC2BCD( stVerInfo.abNextIssuerValidPeriodInfoVer,
             stTmpVerInfo.abNextIssuerValidPeriodInfoVer,
             sizeof( stVerInfo.abNextIssuerValidPeriodInfoVer ) );
    stTmpVerInfo.dwNextIssuerValidPeriodInfoPkt
                                	= stVerInfo.dwNextIssuerValidPeriodInfoPkt;

    stTmpVerInfo.abCrLf[0] = CR;
    stTmpVerInfo.abCrLf[1] = LF;

    if ( fdFile > 0 )
    {
	    nWriteByte = fwrite( (char*)&stTmpVerInfo,
	    					  sizeof( stTmpVerInfo ),
	    					  1,
	                          fdFile );

	    fclose( fdFile );
    }

    if ( nWriteByte  < 1 )
    {
        return ErrRet( ERR_FILE_WRITE | GetFileNo( VER_INFO_FILE ) );
    }

    return SUCCESS;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      UpdateVerStruct                                          *
*                                                                              *
*  DESCRIPTION :      다운로드 받은 파일의 버전을 버전정보 구조체에 update한다.*
*                                                                              *
*  INPUT PARAMETERS:  char* pchFileName, DOWN_FILE_INFO* stDownFileInfo,       *
              int nDCSCommFileNo, bool boolIsCurr                              *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS                                                  *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short UpdateVerStruct( char* pchFileName,
                              DOWN_FILE_INFO* stDownFileInfo,
                              int nDCSCommFileNo,
                              bool boolIsCurr )
{
    char  	achCurrFileName[30]	= { 0, };
    DOWN_FILE_INFO  stTmpDownFileInfo;

    int 	nSizeLength 	=  sizeof( stDownFileInfo->abDownSize );
    int 	nVerLength 		=  sizeof( stDownFileInfo->abFileVer );

	/*
	 *	다운로드가 완료되지 않았으면 skip
	 */
    if ( stDownFileInfo->chDownStatus != DOWN_COMPL )
    {
		PrintlnASC( "[UpdateVerStruct] 다운로드 완료되지 않음 : ",
			pchFileName, strlen( pchFileName ) );
        return SUCCESS;
    }

	if (  gpstSharedInfo->boolIsBLPLMergeNow == TRUE )
	{
		switch ( nDCSCommFileNo )
		{
			case 38:	// 고정 BL
			case 39:	// 변동 BL
				if ( gpstSharedInfo->boolIsBLMergeNow == TRUE )
				{
					printf( "[UpdateVerStruct] BL 머지중이므로 파일 버림\n" );
					ctrl_event_info_write( EVENT_UNLINK_BL_DURING_MERGE );
					unlink( pchFileName );
					DelDownFileList( pchFileName, &stTmpDownFileInfo );
					return SUCCESS;
				}
				break;
			case 41:	// 고정 구선불PL
			case 42:	// 고정 후불PL
			case 43:	// 변동 PL
				if ( gpstSharedInfo->boolIsMifPrepayPLMergeNow == TRUE ||
					 gpstSharedInfo->boolIsPostpayPLMergeNow == TRUE )
				{
					printf( "[UpdateVerStruct] 구선불/후불 PL 머지중이므로 파일 버림\n" );
					ctrl_event_info_write( EVENT_UNLINK_POSTPAY_PL_DURING_MERGE );
					unlink( pchFileName );
					DelDownFileList( pchFileName, &stTmpDownFileInfo );
					return SUCCESS;
				}
				break;
			case 45:	// 고정 신선불PL
			case 46:	// 변동 신선불PL
				if ( gpstSharedInfo->boolIsSCPrepayPLMergeNow == TRUE  )
				{
					printf( "[UpdateVerStruct] 신선불 PL 머지중이므로 파일 버림\n" );
					ctrl_event_info_write( EVENT_UNLINK_SC_PREPAY_PL_DURING_MERGE );
					unlink( pchFileName );
					DelDownFileList( pchFileName, &stTmpDownFileInfo );
					return SUCCESS;
				}
				break;
		}
	}
	
	/*
	 *	적용될 파일명으로 변경
	 */
    strcpy( achCurrFileName, &pchFileName[4] ); // tmp_~  remove
    unlink( achCurrFileName );
    rename( pchFileName, achCurrFileName );
	printf( "[UpdateVerStruct] 임시파일RENAME [%s] -> [%s]\n",
		pchFileName, achCurrFileName );
    DelDownFileList( pchFileName, &stTmpDownFileInfo );

    switch ( nDCSCommFileNo )
    {
        case 0:         	//  terminal Application 1:driver

            if ( boolIsCurr == CURR )
            {
                BCD2ASC( stDownFileInfo->abFileVer,
                         stVerInfo.abDriverOperatorApplVer,
                         nVerLength );
                memcpy( (char*)&stVerInfo.dwDriverOperatorApplPkt,
                        stDownFileInfo->abDownSize,
                        nSizeLength );
            }

            if ( boolIsCurr == NEXT )
            {
                BCD2ASC( stDownFileInfo->abFileVer,
                         stVerInfo.abNextDriverOperatorApplVer,
                         nVerLength );
                memcpy( (char*)&stVerInfo.dwNextDriverOperatorApplPkt,
                        stDownFileInfo->abDownSize,
                        nSizeLength );
            }

            break;
        case 1:         	//  terminal Application 2:main terminal

            if ( boolIsCurr == CURR )
            {
                BCD2ASC( stDownFileInfo->abFileVer,
                         stVerInfo.abMainTermApplVer,
                         nVerLength );
                DebugOutlnASC( "stVerInfo.abMainTermApplVer=>",
                                stVerInfo.abMainTermApplVer,
                                sizeof( stVerInfo.abMainTermApplVer ) );
                memcpy( (char*)&stVerInfo.dwMainTermApplPkt,
                        stDownFileInfo->abDownSize,
                        nSizeLength );
            }

            if ( boolIsCurr == NEXT )
            {
                BCD2ASC( stDownFileInfo->abFileVer,
                         stVerInfo.abNextMainTermApplVer,
                         nVerLength );
                DebugOutlnASC( "stVerInfo.abNextMainTermApplVer=>",
                                stVerInfo.abNextMainTermApplVer,
                                sizeof( stVerInfo.abNextMainTermApplVer ) );
                memcpy( (char*)&stVerInfo.dwNextMainTermApplPkt,
                        stDownFileInfo->abDownSize,
                        nSizeLength );
            }
            break;

        case 2:         	//  terminal Application 3:sub terminal

            if ( boolIsCurr == CURR )
            {
                BCD2ASC( stDownFileInfo->abFileVer,
                         stVerInfo.abSubTermApplVer,
                         nVerLength );
                DebugOutlnASC( "stVerInfo.abSubTermApplVer=>",
                                stVerInfo.abSubTermApplVer,
                                sizeof( stVerInfo.abSubTermApplVer ) );
                memcpy( (char*)&stVerInfo.dwSubTermApplPkt,
                        stDownFileInfo->abDownSize,
                        nSizeLength );
            }

            if( boolIsCurr == NEXT )
            {
                BCD2ASC( stDownFileInfo->abFileVer,
                         stVerInfo.abNextSubTermApplVer,
                         nVerLength );
                DebugOutlnASC( "stVerInfo.abNextSubTermApplVer=>",
                                stVerInfo.abNextSubTermApplVer,
                                sizeof( stVerInfo.abNextSubTermApplVer ) );
                memcpy( (char*)&stVerInfo.dwNextSubTermApplPkt,
                        stDownFileInfo->abDownSize,
                        nSizeLength );
            }
            break;

        case 3:         	//  terminal Application nSizeLength~10
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
            break;

        case 10:        	//  terminal voice data 1 :

            if ( boolIsCurr == CURR )
            {
                BCD2ASC( stDownFileInfo->abFileVer,
                         stVerInfo.abVoiceInfoVer,
                         nVerLength );
                memcpy( (char*)&stVerInfo.dwVoiceInfoPkt,
                        stDownFileInfo->abDownSize,
                        nSizeLength );
            }

            if ( boolIsCurr == NEXT )
            {
                BCD2ASC( stDownFileInfo->abFileVer,
                         stVerInfo.abNextVoiceInfoVer,
                         nVerLength );
                memcpy( (char*)&stVerInfo.dwNextVoiceInfoPkt,
                        stDownFileInfo->abDownSize,
                        nSizeLength );
            }
            break;

        case 11:        	//  terminal voice data 2 ~ 10 :
        case 12:
        case 13:
        case 14:
        case 15:
        case 16:
        case 17:
        case 18:
        case 19:
            break;

        case 20:        	//  run vehicle parameter

            if ( boolIsCurr == CURR )
            {
                BCD2ASC( stDownFileInfo->abFileVer,
                         stVerInfo.abVehicleParmVer,
                         nVerLength );
                memcpy( (char*)&stVerInfo.dwVehicleParmPkt,
                        stDownFileInfo->abDownSize,
                        nSizeLength );
            }

            if ( boolIsCurr == NEXT )
            {
                BCD2ASC( stDownFileInfo->abFileVer,
                         stVerInfo.abNextVehicleParmVer,
                         nVerLength );
                memcpy( (char*)&stVerInfo.dwNextVehicleParmPkt,
                        stDownFileInfo->abDownSize,
                        nSizeLength );
            }
            break;

        case 21:        	//  bus  terminal route parameter

            if ( boolIsCurr == CURR )
            {
                BCD2ASC( stDownFileInfo->abFileVer,
                         stVerInfo.abRouteParmVer,
                         nVerLength );
                DebugOutlnASC( "stVerInfo.abRouteParmVer=>\n" ,
                                stVerInfo.abRouteParmVer,
                                sizeof( stVerInfo.abRouteParmVer ) );
				
                memcpy( (char*)&stVerInfo.dwRouteParmPkt,
                        stDownFileInfo->abDownSize,
                        nSizeLength );
            }

            if ( boolIsCurr == NEXT )
            {
                BCD2ASC( stDownFileInfo->abFileVer,
                         stVerInfo.abNextRouteParmVer,
                         nVerLength );
                memcpy( (char*)&stVerInfo.dwNextRouteParmPkt,
                        stDownFileInfo->abDownSize,
                        nSizeLength );
            }
            break;

        case 22:        	//  new fare data

            if ( boolIsCurr == CURR )
            {

                BCD2ASC( stDownFileInfo->abFileVer,
                         stVerInfo.abBasicFareInfoVer,
                         nVerLength );
                DebugOutlnASC( "stVerInfo.abBasicFareInfoVer=>\n" ,
                                stVerInfo.abBasicFareInfoVer,
                                sizeof( stVerInfo.abBasicFareInfoVer ) );
                memcpy( (char*)&stVerInfo.dwBasicFareInfoPkt,
                        stDownFileInfo->abDownSize,
                        nSizeLength );
            }

            if ( boolIsCurr == NEXT )
            {
                BCD2ASC( stDownFileInfo->abFileVer,
                         stVerInfo.abNextBasicFareInfoVer,
                         nVerLength );
                memcpy( (char*)&stVerInfo.dwNextBasicFareInfoPkt,
                        stDownFileInfo->abDownSize,
                        nSizeLength );
            }
            break;

        case 23:        	//  old single fare data

            if ( boolIsCurr == CURR )
            {
                BCD2ASC( stDownFileInfo->abFileVer,
                         stVerInfo.abOldFareVer,
                         nVerLength );
                memcpy( (char*)&stVerInfo.dwOldFarePkt,
                        stDownFileInfo->abDownSize,
                        nSizeLength );
            }

            if ( boolIsCurr == NEXT )
            {
                BCD2ASC( stDownFileInfo->abFileVer,
                         stVerInfo.abNextOldFareVer,
                         nVerLength );
                memcpy( (char*)&stVerInfo.dwNextOldFarePkt,
                        stDownFileInfo->abDownSize,
                        nSizeLength );
            }
            break;

        case 24:        	//  old airport fare data

            if ( boolIsCurr == CURR )
            {
                BCD2ASC( stDownFileInfo->abFileVer,
                         stVerInfo.abOldAirFareVer,
                         nVerLength );
                memcpy( (char*)&stVerInfo.dwOldAirFarePkt,
                        stDownFileInfo->abDownSize,
                        nSizeLength );
            }

            if ( boolIsCurr == NEXT )
            {
                BCD2ASC( stDownFileInfo->abFileVer,
                         stVerInfo.abNextOldAirFareVer,
                         nVerLength );
                memcpy( (char*)&stVerInfo.dwNextOldAirFarePkt,
                        stDownFileInfo->abDownSize,
                        nSizeLength );
            }
            break;

        case 25:        	//  old  range fare data

            if ( boolIsCurr == CURR )
            {
                BCD2ASC( stDownFileInfo->abFileVer,
                         stVerInfo.abOldRangeFareVer,
                         nVerLength );
                memcpy( (char*)&stVerInfo.dwOldRangeFarePkt,
                        stDownFileInfo->abDownSize,
                        nSizeLength );
            }

            if ( boolIsCurr == NEXT )
            {
                BCD2ASC( stDownFileInfo->abFileVer,
                         stVerInfo.abNextOldRangeFareVer,
                         nVerLength );
                memcpy( (char*)&stVerInfo.dwNextOldRangeFarePkt,
                        stDownFileInfo->abDownSize,
                        nSizeLength );
            }
            break;

        case 26:        	//  station data

            if ( boolIsCurr == CURR )
            {
                BCD2ASC( stDownFileInfo->abFileVer,
                         stVerInfo.abBusStationInfoVer,
                         nVerLength );
                memcpy( (char*)&stVerInfo.dwBusStationInfoPkt,
                        stDownFileInfo->abDownSize,
                        nSizeLength );
            }

            if ( boolIsCurr == NEXT )
            {
                BCD2ASC( stDownFileInfo->abFileVer,
                         stVerInfo.abNextBusStationInfoVer,
                         nVerLength );
                memcpy( (char*)&stVerInfo.dwNextBusStationInfoPkt,
                        stDownFileInfo->abDownSize,
                        nSizeLength );
            }
            break;

        case 27:        	//  prepay  issuer data

            if ( boolIsCurr == CURR )
            {
                BCD2ASC( stDownFileInfo->abFileVer,
                         stVerInfo.abPrepayIssuerInfoVer,
                         nVerLength );
                memcpy( (char*)&stVerInfo.dwPrepayIssuerInfoPkt,
                        stDownFileInfo->abDownSize,
                        nSizeLength );
            }

            if ( boolIsCurr == NEXT )
            {
                BCD2ASC( stDownFileInfo->abFileVer,
                         stVerInfo.abNextPrepayIssuerInfoVer,
                         nVerLength );
                memcpy( (char*)&stVerInfo.dwNextPrepayIssuerInfoPkt,
                        stDownFileInfo->abDownSize,
                        nSizeLength );
            }
            break;

        case 28:        	//  postpay issuer data

            if ( boolIsCurr == CURR )
            {
                BCD2ASC( stDownFileInfo->abFileVer,
                         stVerInfo.abPostpayIssuerInfoVer,
                         nVerLength );
                memcpy( (char*)&stVerInfo.dwPostpayIssuerInfoPkt,
                        stDownFileInfo->abDownSize,
                        nSizeLength );
            }

            if ( boolIsCurr == NEXT )
            {
                BCD2ASC( stDownFileInfo->abFileVer,
                         stVerInfo.abNextPostpayIssuerInfoVer,
                         nVerLength );
                memcpy( (char*)&stVerInfo.dwNextPostpayIssuerInfoPkt,
                        stDownFileInfo->abDownSize,
                        nSizeLength );
            }
            break;

        case 29:        	//  discount /extra data

            if ( boolIsCurr == CURR )
            {
                BCD2ASC( stDownFileInfo->abFileVer,
                        stVerInfo.abDisExtraInfoVer,
                        nVerLength );
                memcpy( (char*)&stVerInfo.dwDisExtraInfoPkt,
                        stDownFileInfo->abDownSize,
                        nSizeLength );
            }

            if ( boolIsCurr == NEXT )
            {
                BCD2ASC( stDownFileInfo->abFileVer,
                         stVerInfo.abNextDisExtraInfoVer,
                         nVerLength );
                memcpy( (char*)&stVerInfo.dwNextDisExtraInfoPkt,
                        stDownFileInfo->abDownSize,
                        nSizeLength );
            }
            break;

        case 30:        	// holiday data

            if ( boolIsCurr == CURR )
            {
                BCD2ASC( stDownFileInfo->abFileVer,
                         stVerInfo.abHolidayInfoVer,
                         nVerLength );
                memcpy( (char*)&stVerInfo.dwHolidayInfoPkt,
                        stDownFileInfo->abDownSize,
                        nSizeLength );
            }

            if ( boolIsCurr == NEXT )
            {
                BCD2ASC( stDownFileInfo->abFileVer,
                        stVerInfo.abNextHolidayInfoVer,
                        nVerLength );
                memcpy( (char*)&stVerInfo.dwNextHolidayInfoPkt,
                        stDownFileInfo->abDownSize,
                        nSizeLength );
            }
            break;

        case 31:        	// transfer condition data

            if ( boolIsCurr == CURR )
            {
                BCD2ASC( stDownFileInfo->abFileVer,
                         stVerInfo.abXferApplyRegulationVer,
                         nVerLength );
                memcpy( (char*)&stVerInfo.dwXferApplyRegulationPkt,
                        stDownFileInfo->abDownSize,
                        nSizeLength );
            }

            if ( boolIsCurr == NEXT )
            {
                BCD2ASC( stDownFileInfo->abFileVer,
                         stVerInfo.abNextXferApplyRegulationVer,
                         nVerLength );
                memcpy( (char*)&stVerInfo.dwNextXferApplyRegulationPkt,
                        stDownFileInfo->abDownSize,
                        nSizeLength );
            }

            break;

        case 32:			//  Issuer registration   count

            if ( boolIsCurr == CURR )
            {
                BCD2ASC( stDownFileInfo->abFileVer,
                         stVerInfo.abEpurseIssuerRegistInfoVer,
                         nVerLength );
                memcpy( (char*)&stVerInfo.dwEpurseIssuerRegistInfoPkt,
                        stDownFileInfo->abDownSize,
                        nSizeLength );
            }

            if ( boolIsCurr == NEXT )
            {
                BCD2ASC( stDownFileInfo->abFileVer,
                         stVerInfo.abNextEpurseIssuerRegistInfoVer,
                         nVerLength );
                memcpy( (char*)&stVerInfo.dwNextEpurseIssuerRegistInfoPkt,
                        stDownFileInfo->abDownSize,
                        nSizeLength );
            }

            break;

        case 33:			//  paymentSAM Key Set data

            if ( boolIsCurr == CURR )
            {
                BCD2ASC( stDownFileInfo->abFileVer,
                         stVerInfo.abPSAMKeysetVer,
                         nVerLength );
                memcpy( (char*)&stVerInfo.dwPSAMKeysetPkt,
                        stDownFileInfo->abDownSize,
                        nSizeLength );
            }

            if ( boolIsCurr == NEXT )
            {
                BCD2ASC( stDownFileInfo->abFileVer,
                         stVerInfo.abNextPSAMKeysetVer,
                         nVerLength );
                memcpy( (char*)&stVerInfo.dwNextPSAMKeysetPkt,
                         stDownFileInfo->abDownSize,
                         nSizeLength );
            }

            break;

        case 34:			//  auto-charge SAM Key Set data

            if ( boolIsCurr == CURR )
            {
                BCD2ASC( stDownFileInfo->abFileVer,
                         stVerInfo.abAutoChargeKeysetVer,
                         nVerLength );
                memcpy( (char*)&stVerInfo.dwAutoChargeKeysetPkt,
                        stDownFileInfo->abDownSize,
                        nSizeLength );
            }

            if ( boolIsCurr == NEXT )
            {
                BCD2ASC( stDownFileInfo->abFileVer,
                         stVerInfo.abNextAutoChargeKeysetVer,
                         nVerLength );
                memcpy( (char*)&stVerInfo.dwNextAutoChargeKeysetPkt,
                        stDownFileInfo->abDownSize,
                        nSizeLength );
            }
            break;

        case 35:    		//  auto-charge parameter  data

            if ( boolIsCurr == CURR )
            {
                BCD2ASC( stDownFileInfo->abFileVer,
                         stVerInfo.abAutoChargeParmVer,
                         nVerLength );
                memcpy( (char*)&stVerInfo.dwAutoChargeParmPkt,
                        stDownFileInfo->abDownSize,
                        nSizeLength );
            }

            if ( boolIsCurr == NEXT )
            {
                BCD2ASC( stDownFileInfo->abFileVer,
                        stVerInfo.abNextAutoChargeParmVer,
                        nVerLength );
                memcpy( (char*)&stVerInfo.dwNextAutoChargeParmPkt,
                        stDownFileInfo->abDownSize,
                        nSizeLength );
            }
            break;

        case 36:			// transferapply

            if ( boolIsCurr == CURR )
            {
                BCD2ASC( stDownFileInfo->abFileVer,
                         stVerInfo.abXferApplyInfoVer,
                         nVerLength );
                memcpy( (char*)&stVerInfo.dwXferApplyInfoPkt,
                         stDownFileInfo->abDownSize,
                         nSizeLength );
            }

            if ( boolIsCurr == NEXT )
            {
                BCD2ASC( stDownFileInfo->abFileVer,
                         stVerInfo.abNextXferApplyInfoVer,
                         nVerLength );
                memcpy((char*)&stVerInfo.dwNextXferApplyInfoPkt,
                        stDownFileInfo->abDownSize,
                        nSizeLength );
            }
            break;

        case 37:			// valid period check

            if ( boolIsCurr == CURR )
            {
                BCD2ASC( stDownFileInfo->abFileVer,
                         stVerInfo.abIssuerValidPeriodInfoVer,
                         nVerLength );
                memcpy( (char*)&stVerInfo.dwIssuerValidPeriodInfoPkt,
                        stDownFileInfo->abDownSize,
                        nSizeLength );
            }

            if ( boolIsCurr == NEXT )
            {
                BCD2ASC( stDownFileInfo->abFileVer,
                        stVerInfo.abNextIssuerValidPeriodInfoVer,
                        nVerLength );
                memcpy( (char*)&stVerInfo.dwNextIssuerValidPeriodInfoPkt,
                        stDownFileInfo->abDownSize,
                        nSizeLength );
            }
            break;

        case 38:			//  current master B/L data
            BCD2ASC( stDownFileInfo->abFileVer,
                     stVerInfo.abMasterBLVer,
                     nVerLength );
            stVerInfo.dwMasterBLPkt = 0;
            break;

        case 39:			//  current update B/L data
            BCD2ASC( stDownFileInfo->abFileVer,
                     stVerInfo.abUpdateBLVer,
                     nVerLength );
            stVerInfo.dwUpdateBLPkt = 0 ;
            break;

        case 40:			//  current HOT B/L data
            BCD2ASC( stDownFileInfo->abFileVer,
                     stVerInfo.abHotBLVer,
                     nVerLength );
            stVerInfo.dwHotBLPkt = 0 ;
            break;

        case 41:			//  current master P/L data -  prepay
            BCD2ASC( stDownFileInfo->abFileVer,
                     stVerInfo.abMasterPrepayPLVer,
                     nVerLength );
            stVerInfo.dwMasterPrepayPLPkt = 0 ;
            break;

        case 42:			//  current master P/L data -  postpay
            BCD2ASC( stDownFileInfo->abFileVer,
                     stVerInfo.abMasterPostpayPLVer,
                     nVerLength );
            stVerInfo.dwMasterPostpayPLPkt = 0 ;
            break;

        case 43:			//  current update P/L data
            BCD2ASC( stDownFileInfo->abFileVer,
                     stVerInfo.abUpdatePLVer,
                     nVerLength );
            stVerInfo.dwUpdatePLPkt = 0 ;
            break;

        case 44:			//  current HOT P/L data
            BCD2ASC( stDownFileInfo->abFileVer,
                     stVerInfo.abHotPLVer,
                     nVerLength );
            stVerInfo.dwHotPLVerPkt = 0 ;
            break;

        case 45:			// master A/I data
            BCD2ASC( stDownFileInfo->abFileVer,
                     stVerInfo.abMasterAIVer,
                     nVerLength );
            stVerInfo.dwMasterAIPkt = 0 ;
            break;

        case 46:			// update A/I data
            BCD2ASC( stDownFileInfo->abFileVer,
                     stVerInfo.abUpdateAIVer,
                     nVerLength );
            stVerInfo.dwUpdateAIPkt = 0 ;
            break;

        case 47:			// HOT A/I data
            BCD2ASC( stDownFileInfo->abFileVer,
                     stVerInfo.abHotAIVer,
                     nVerLength );
            stVerInfo.dwHotAIPkt = 0 ;
            break;

    }

    return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      InitVerInfo                                              *
*                                                                              *
*  DESCRIPTION :      master BLPL과 update BLPL버전을 제외한 버전을 초기화한다.  *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: None                                                     *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
void InitVerInfo( void )
{
    byte abVerInfoCreateDtime[14] 	= { 0, };

    byte abMasterBLVer[14] 			= { 0, };
    byte abUpdateBLVer[14] 			= { 0, };
    byte abMasterPrepayPLVer[14] 	= { 0, };
    byte abMasterPostpayPLVer[14] 	= { 0, };
    byte abUpdatePLVer[14] 			= { 0, };
    byte abMasterAIVer[14] 			= { 0, };	// master AI version ( BCD )
    byte abUpdateAIVer[14] 			= { 0, };	// update AI version( BCD )

    dword dwMasterBLPkt 			= 0;
    dword dwUpdateBLPkt 			= 0;
    dword dwMasterPrepayPLPkt 		= 0;
    dword dwMasterPostpayPLPkt 		= 0;
    dword dwUpdatePLPkt 			= 0;
    dword dwMasterAIPkt 			= 0;
    dword dwUpdateAIPkt 			= 0;

    time_t  tCurrTime 				= 0L;
    int 	nVerLength  			= sizeof( stVerInfo.abMasterBLVer );

    /*
     *	BLPL version backup
     */
     
    // 7 master B/L version( BCD )
    memcpy( abMasterBLVer,  stVerInfo.abMasterBLVer, nVerLength );
    dwMasterBLPkt = stVerInfo.dwMasterBLPkt ;

	// 14 update B/L version( BCD )
    memcpy( abUpdateBLVer, stVerInfo.abUpdateBLVer, nVerLength );
    dwUpdateBLPkt = stVerInfo.dwUpdateBLPkt;

	// 28 master P/L version (  prepay  )( BCD )
    memcpy( abMasterPrepayPLVer, stVerInfo.abMasterPrepayPLVer, nVerLength );
    dwMasterPrepayPLPkt = stVerInfo.dwMasterPrepayPLPkt;

	// 35 master P/L version (  postpay )( BCD )
    memcpy( abMasterPostpayPLVer, stVerInfo.abMasterPostpayPLVer, nVerLength );
    dwMasterPostpayPLPkt = stVerInfo.dwMasterPostpayPLPkt;

	// 42 update PL version( BCD )
    memcpy( abUpdatePLVer, stVerInfo.abUpdatePLVer, nVerLength );
    dwUpdatePLPkt = stVerInfo.dwUpdatePLPkt ;

	// master AI version (  newtransportation )( BCD )
    memcpy( abMasterAIVer,  stVerInfo.abMasterAIVer, nVerLength );
    dwMasterAIPkt = stVerInfo.dwMasterAIPkt;

	// update AI version (  newtransportation )( BCD )
    memcpy( abUpdateAIVer, stVerInfo.abUpdateAIVer, nVerLength );
    dwUpdateAIPkt = stVerInfo.dwUpdateAIPkt ;

    /*
     *	init version structure
     */
    memset( &stVerInfo, 0, sizeof( stVerInfo ) );

    /*
     *	백업된 BLPL version을 structure에 setting
     */
     
    // 0 versiondata creation date( BCD )
    GetRTCTime( &tCurrTime );
    TimeT2BCDDtime( tCurrTime, abVerInfoCreateDtime );
    memcpy( stVerInfo.abVerInfoCreateDtime,
    		abVerInfoCreateDtime,
    		sizeof( stVerInfo.abVerInfoCreateDtime ) );
                
	// 7 master B/L version( BCD )
    memcpy( stVerInfo.abMasterBLVer, abMasterBLVer, nVerLength );
    stVerInfo.dwMasterBLPkt = dwMasterBLPkt;

	// 14 update B/L version( BCD )
    memcpy( stVerInfo.abUpdateBLVer, abUpdateBLVer, nVerLength );
    stVerInfo.dwUpdateBLPkt = dwUpdateBLPkt;

	// 28 master P/L version (  prepay  )( BCD )
    memcpy( stVerInfo.abMasterPrepayPLVer, abMasterPrepayPLVer, nVerLength );
    stVerInfo.dwMasterPrepayPLPkt = dwMasterPrepayPLPkt;

	// 35 master P/L version (  postpay )( BCD )
    memcpy( stVerInfo.abMasterPostpayPLVer,
    		abMasterPostpayPLVer,
            nVerLength ); 
    stVerInfo.dwMasterPostpayPLPkt = dwMasterPostpayPLPkt ;

	// 42 update P/L version (  postpay )( BCD )
    memcpy(  stVerInfo.abUpdatePLVer, abUpdatePLVer, nVerLength );
    stVerInfo.dwUpdatePLPkt = dwUpdatePLPkt;

	// master AI version (  newtransportation )( BCD )
    memcpy( stVerInfo.abMasterAIVer, abMasterAIVer, nVerLength );
    stVerInfo.dwMasterAIPkt = dwMasterAIPkt;

	// update AI version (  newtransportation )( BCD )
    memcpy( stVerInfo.abUpdateAIVer,  abUpdateAIVer, nVerLength );
    stVerInfo.dwUpdateAIPkt = dwUpdateAIPkt;

	/*
	 *	초기화한 버전정보 파일로 저장
	 */
    SaveVerFile();

	/*
	 *	업로드용 버전파일 생성
	 */
    if ( SetUploadVerInfo() != SUCCESS )
    {
       DebugOut( "\r\n upload Version File Creating Fail \r\n" );
    }

}
