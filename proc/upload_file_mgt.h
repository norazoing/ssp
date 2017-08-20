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
*  PROGRAM ID :       upload_file_mgt.h                                       *
*                                                                              *
*  DESCRIPTION:       이 프로그램은 단말기 설치정보와 버전정보의 생성 기능을 	   *
*						제공한다.       										   *
*                                                                              *
*  ENTRY POINT:       CreateInstallInfoFile                                    *
*                     SetUploadVerInfo                                         *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: None                                                     *
*                                                                              *
*  INPUT FILES:       None			                                           *
*                                                                              *
*  OUTPUT FILES:      install.dat                                              *
*                     version.trn                                              *
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
#ifndef _UPLOAD_FILE_MGT_H_
#define _UPLOAD_FILE_MGT_H_

/*******************************************************************************
*  Inclusion of System Header Files                                            *
*******************************************************************************/
#include "file_mgt.h"

/*******************************************************************************
*  structure of UPLOAD_VER_INFO - version.trn                                  *
*******************************************************************************/
typedef struct				       
{
	byte	abMainTermApplVerName[5];		// 승차단말기 F/W 버전
	byte	abVerInfoCreateDtime[7];		// 버전정보생성일시
	byte	abSubTerm1ApplVerName[5];		// 하차단말기1 F/W 버전
	byte	abSubTerm2ApplVerName[5];		// 하차단말기2 F/W 버전
	byte	abSubTerm3ApplVerName[5];		// 하차단말기3 F/W 버전
	byte	abDriverOperatorApplVerName[5];	// 운전자조자작기 F/W 버전
	byte	abMainTermKernelVerName[2];		// 승차단말기 커널 버전
	byte	abTranspBizrID[9];				// 교통사업자 ID
	byte	abBusBizOfficeID[2];			// 영업소 ID
	byte	abRouteID[4];					// 노선 ID
	byte	abVehicleID[9];					// 차량 ID
	byte	abMainTermID[9];				// 승차단말기 ID
	byte	bSubTermCnt;					// 하차단말기 개수
	byte	abSubTerm1ID[9];				// 하차단말기1 ID
	byte	abSubTerm2ID[9];				// 하차단말기2 ID
	byte	abSubTerm3ID[9];				// 하차단말기3 ID
	byte	abDriverOperatorID[9];			// 운전자조작기 ID
	byte	abMainTermSAMID[16];			// 승차단말기 PSAM ID
	byte	abSubTerm1SAMID[16];			// 하차단말기1 PSAM ID
	byte	abSubTerm2SAMID[16];			// 하차단말기2 PSAM ID
	byte	abSubTerm3SAMID[16];			// 하차단말기3 PSAM ID
	byte	abDCSIPAddr[12];				// 집계서버 IP
	byte	abTranspMethodCode[3];			// 교통수단코드            
	byte	abMasterBLSize[5];				// 고정BL 크기
	byte	abMasterPrepayPLSize[5];		// 구선불고정PL 크기
	byte	abMasterPostpayPLSize[5];		// 후불고정PL 크기
	byte	abMasterAISize[5];				// 신선불고정PL 크기
	byte	abMasterBLVer[7];				// 고정BL 일시버전
	byte	abUpdateBLVer[7];				// 변동BL 일시버전
	byte	abHotBLVer[7];					// 핫BL 일시버전
	byte	abMasterPrepayPLVer[7];			// 구선불고정PL 일시버전
	byte    abMasterPostpayPLVer[7];          
		// fixed  p/l version (postpay)           
	byte    abUpdatePLVer[7];                 
		// update  pl version                   
	byte    abHotPLVer[7];                    
		// hot p/l version                   
	byte    abMasterAIVer[7];                 
		// fixed  ai version  (new transportation )         
	byte    abUpdateAIVer[7];                 
		// update  ai version (new transportation )          
	byte    abHotAIVer[7];                    
		// hot ai version  i(new transportation )         
	byte    abVehicleParmVer[7];              
		// run vehicle parameter  version         
	byte    abRouteParmVer[7];                
		// bus terminalroute  parameter  version  
	byte    abBasicFareInfoVer[7];            
		// basic fareinformation  version              
	byte    abBusStationInfoVer[7];           
		// bus station interval info  version          
	byte    abPrepayIssuerInfoVer[7];         
		// prepay issuer info  version          
	byte    abPostpayIssuerInfoVer[7];        
		// postpay issuer  info  version          
	byte    abDisExtraInfoVer[7];             
		// discount or extra info  version              
	byte    abHolidayInfoVer[7];              
		// holiday info  version                  
	byte    abXferApplyRegulationVer[7];      
		// transfer apply  regulation version              
	byte    abXferApplyVer[7];      	  
		// transfer apply  version                  
	byte    abMainTermApplVer[7];             
		// enter  S/W version                   
	byte    abSubTerm1ApplVer[7];             
		// sub  S/W version   1               
	byte    abSubTerm2ApplVer [7];            
		// sub  S/W version   2               
	byte    abSubTerm3ApplVer [7];            
		// sub  S/W version   3               
	byte    abDriverOperatorApplVer[7];       
		// driver operator  S/W version            
	byte    abEpurseIssuerRegistInfoVer[7];   
		// purse issuer registration info  version    
	byte    abPSAMKeysetVer[7];               
		// Payment  sam ketset version           
	byte    abAutoChargeKeysetVer[7];         
		// auto- charge  sam ketset          
	byte    abAutoChargeParmVer[7];           
		// auto- charge  parameter version        
	byte    abPostpayIssuerChkVer[7];         
		// postpay issuer  check version          
	byte    abCrLf[2];                        
		// special character                      
}__attribute__((packed))  UPLOAD_VER_INFO;   

/*******************************************************************************
*  Declaration of Module Functions                                             *
*******************************************************************************/
short CreateInstallInfoFile( void );
short SetUploadVerInfo( void );

#endif