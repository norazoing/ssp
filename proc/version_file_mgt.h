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
*  PROGRAM ID :       version_file_mgt.h                                       *
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

#ifndef _VERSION_FILE_MGT_H_
#define _VERSION_FILE_MGT_H_

/*******************************************************************************
*  Inclusion of System Header Files                                            *
*******************************************************************************/
#include "file_mgt.h"

/*******************************************************************************
*  structure of VER_INFO - c_ve_inf.dat                              	       *
*******************************************************************************/
typedef struct
{
	byte	abVerInfoCreateDtime [7]; // version info creation date (BCD)         
	byte    abMasterBLVer[7];         // fixed B/Lversion   (BCD)           
	dword   dwMasterBLPkt;            // fixed B/L Pkt                
	byte    abUpdateBLVer[7];         // update B/Lversion   (BCD)          
	dword   dwUpdateBLPkt;            // update B/L Pkt                
	byte    abHotBLVer[7];            // HOTB/Lversion    (BCD)           
	dword   dwHotBLPkt;               // HOTB/L Pkt                 
	byte    abMasterPrepayPLVer [7];  // fixed P/L version (prepay )           
	dword   dwMasterPrepayPLPkt;      // fixed P/L (prepay ) Pkt         
	byte    abMasterPostpayPLVer[7];  // fixed P/L version (postpay)           
	dword   dwMasterPostpayPLPkt;     // fixed P/L (postpay) Pkt         
	byte    abUpdatePLVer[7];         // update PL version                   
	dword   dwUpdatePLPkt;            // update PL Pkt                 
	byte    abHotPLVer[7];            // HOT P/L version                   
	dword   dwHotPLVerPkt;            // HOT P/L Pkt                 
	byte    abMasterAIVer[7];         // fixed AI version (new transportation )          
	dword   dwMasterAIPkt;            // fixed AI (new transportation ) Pkt        
	byte    abUpdateAIVer[7];         // update AI version (new transportation )          
	dword   dwUpdateAIPkt;            // update AI (new transportation ) Pkt        
	byte    abHotAIVer[7];            // HOT AI version (new transportation )           
	dword   dwHotAIPkt;               // HOT AI (new transportation ) Pkt         
	byte    abVehicleParmVer[7];      // run vehicle parameter version         
	dword   dwVehicleParmPkt;         // run vehicle parameter  Pkt       
	byte    abNextVehicleParmVer[7];  // run vehicle parameter next version     
	dword   dwNextVehicleParmPkt;     // run vehicle parameter next  Pkt   
	byte    abRouteParmVer[7];        // route parameter version           
	dword   dwRouteParmPkt;           // route  parameter  Pkt         
	byte    abNextRouteParmVer[7];    // route  parameter next version       
	dword   dwNextRouteParmPkt;       // route  parameter next  Pkt     
	byte    abBasicFareInfoVer[7];    // basic fare info version             
	dword   dwBasicFareInfoPkt;       // basic fare info  Pkt           
	byte    abNextBasicFareInfoVer[7];// basic fare info next version         
	dword   dwNextBasicFareInfoPkt;   // basic fare info next  Pkt       
	byte    abOldFareVer[7];          // old single fare                  
	dword   dwOldFarePkt;             // old single fare Pkt             
	byte    abNextOldFareVer[7];      // old single fare next version           
	dword   dwNextOldFarePkt;         // old single fare next  Pkt         
	byte    abOldAirFareVer[7];       // old airport  fare(A~C)             
	dword   dwOldAirFarePkt;          // old airport  fare(A~C) Pkt        
	byte    abNextOldAirFareVer[7];   // old airport  fare(A~C)next version      
	dword   dwNextOldAirFarePkt;      // old airport  fare(A~C)next  Pkt    
	byte    abOldRangeFareVer [7];    // old range  fare                  
	dword   dwOldRangeFarePkt;        // old range  fare Pkt             
	byte    abNextOldRangeFareVer[7]; // old range  fare next version           
	dword   dwNextOldRangeFarePkt;    // old range  fare next  Pkt         
	byte    abBusStationInfoVer[7];   // bus station info version         
	dword   dwBusStationInfoPkt;      // bus station info  Pkt       
	byte    abNextBusStationInfoVer[7];// bus station info next version     
	dword   dwNextBusStationInfoPkt;  // bus station info next Pkt   
	byte    abPrepayIssuerInfoVer[7]; // prepay issuer info version           
	dword   dwPrepayIssuerInfoPkt;    // prepay issuer info  Pkt         
	byte    abNextPrepayIssuerInfoVer[7];// prepay issuer info next version       
	dword   dwNextPrepayIssuerInfoPkt;// prepay issuer infor next Pkt     
	byte    abPostpayIssuerInfoVer[7];// postpay issuer info version           
	dword   dwPostpayIssuerInfoPkt;   // postpay issuer info  Pkt         
	byte    abNextPostpayIssuerInfoVer[7];// postpay issuer info next version       
	dword   dwNextPostpayIssuerInfoPkt;// postpay issuer info next Pkt     
	byte    abDisExtraInfoVer [7];    // discount or extra info version             
	dword   dwDisExtraInfoPkt;        // discount or extra info  Pkt           
	byte    abNextDisExtraInfoVer[7]; // discount or extra info next version         
	dword   dwNextDisExtraInfoPkt;    // discount or extra info next  Pkt       
	byte    abHolidayInfoVer [7];     // holiday info version                 
	dword   dwHolidayInfoPkt;         // holiday info  Pkt               
	byte    abNextHolidayInfoVer[7];  // holiday info next version             
	dword   dwNextHolidayInfoPkt;     // holiday info next  Pkt           
	byte    abDriverOperatorApplVer [7];// driver operator Program version      
	dword   dwDriverOperatorApplPkt;  // driver operator Program Pkt    
	byte    abNextDriverOperatorApplVer[7];// driver operator next version  
	dword   dwNextDriverOperatorApplPkt;// driver operator Program next  Pkt
	byte    abMainTermApplVer[7];    // enter terminal Program           
	dword   dwMainTermApplPkt;       // enter terminal Program Pkt      
	byte    abNextMainTermApplVer[7];// enter terminal Program next version    
	dword   dwNextMainTermApplPkt;   // enter terminal Program next  Pkt  
	byte    abSubTermApplVer [7];    // sub terminal Program           
	dword   dwSubTermApplPkt;        // sub terminal Program Pkt      
	byte    abNextSubTermApplVer[7]; // sub terminal Program next version    
	dword   dwNextSubTermApplPkt;    // sub terminal Program next  Pkt  
	byte    abVoiceInfoVer [7];      // voice info version                 
	dword   dwVoiceInfoPkt;          // voice info version  Pkt           
	byte    abNextVoiceInfoVer[7];   // voice info next version             
	dword   dwNextVoiceInfoPkt;      // voice info next  Pkt           
	byte    abXferApplyRegulationVer[7];// transfer apply regulation version             
	dword   dwXferApplyRegulationPkt;// transfer apply regulation  Pkt           
	byte    abNextXferApplyRegulationVer[7];// transfer apply regulation version             
	dword   dwNextXferApplyRegulationPkt;// transfer apply regulation  Pkt           
	byte    abEpurseIssuerRegistInfoVer[7];// Issuer registration info  version     
	dword   dwEpurseIssuerRegistInfoPkt;// Issuer registration info  Pkt    
	byte    abNextEpurseIssuerRegistInfoVer[7];//Issuer regi info next version 
	dword   dwNextEpurseIssuerRegistInfoPkt;// Issuer regi info next Pkt
	byte    abPSAMKeysetVer[7];      // Payment SAM KeySet version            
	dword   dwPSAMKeysetPkt;         // Payment SAM KeySet  Pkt          
	byte    abNextPSAMKeysetVer[7];  // Payment SAM KeySet next version        
	dword   dwNextPSAMKeysetPkt;     // Payment SAM KeySet next  Pkt      
	byte    abAutoChargeKeysetVer[7];// auto-charge SAM KeySet version        
	dword   dwAutoChargeKeysetPkt;   // auto-charge SAM KeySet  Pkt      
	byte    abNextAutoChargeKeysetVer[7];// auto-charge SAM KeySet next        
	dword   dwNextAutoChargeKeysetPkt;// auto-charge SAM KeySet next  Pkt  
	byte    abAutoChargeParmVer [7]; // auto-charge Parameter version        
	dword   dwAutoChargeParmPkt;     // auto-charge Parameter Pkt      
	byte    abNextAutoChargeParmVer[7];// auto-charge Parameter next version    
	dword   dwNextAutoChargeParmPkt; // auto-charge Parameter next  Pkt  
	byte    abXferApplyInfoVer [7];  // transfer apply version                 
	dword   dwXferApplyInfoPkt;      // transfer apply  Pkt               
	byte    abNextXferApplyInfoVer[7];// transfer apply next version             
	dword   dwNextXferApplyInfoPkt;  // transfer apply next  Pkt           
	byte    abIssuerValidPeriodInfoVer[7];// issuer valid period version           
	dword   dwIssuerValidPeriodInfoPkt;// issuer valid period  Pkt         
	byte    abNextIssuerValidPeriodInfoVer[7];// issuer valid period next ver     
	dword   dwNextIssuerValidPeriodInfoPkt;// issuer valid period next  Pkt     
	byte    abCrLf[2];               // special character                    
} __attribute__((packed)) TEMP_VER_INFO;

extern VER_INFO			stVerInfo;		// 버전정보 구조체

/*******************************************************************************
*  Declaration of Module Functions                                             *
*******************************************************************************/
short LoadVerInfo( VER_INFO* pstVerInfo );
short UpdateVerFile( void );
short SaveVerFile( void );
void InitVerInfo( void );

#endif

