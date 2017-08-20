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
*  PROGRAM ID :       file_mgt.c                                               *
*                                                                              *
*  DESCRIPTION:       이 프로그램은 파일을 관리하기 위한 파일번호와 파일명을 제공 *
*						하거나 파일의 Lock과 Unlock을 제어한다.                 *
*                                                                              *
*  ENTRY POINT: 	short GetFileNo( char *pchFileName );                      *
*					short GetDownFileIndex( char* pchCmd );                    *
*					void GetRecvFileName( USER_PKT_MSG*      pstRecvUsrPktMsg, *
*					                      PKT_HEADER_INFO*   pstPktHeaderInfo, *
*					                      char*  			 achRecvFileName );*
*					int LockFile( int fd );									   *
*					int UnlockFile( int fd ); 								   *
*																			   *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: None                                                     *
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
* 2005/09/03 Solution Team Mi Hyun Noh  Initial Release                        *
*                                                                              *
*******************************************************************************/

#ifndef _FILE_MGT_H_
#define _FILE_MGT_H_

/*******************************************************************************
*  Inclusion of Header Files                                                   *
*******************************************************************************/
#include "../system/bus_type.h"
#include "../comm/dcs_comm.h"

/*******************************************************************************
*  structure of downloadinfo.dat                                               *
*******************************************************************************/
typedef struct
{
	char 	achFileName[30];		// file name
	byte 	abFileVer[7];			// file version(BCD)
	char	chDownStatus;			// download status flag
	byte	abDownSize[4];			// download size
} __attribute__((packed)) DOWN_FILE_INFO;


/*******************************************************************************
*  structure of downfilelink.dat                                               *
*******************************************************************************/
typedef struct 	
{
	char	chFileNo;			// order No.of fileTable chain
	byte	abDownFileVer[7];		// download file version
	byte	abDownFilePktNo[4];		// packet No. of download file
	byte	abDownFileSize[4];		// download file size
} __attribute__((packed)) FILE_RELAY_RECV_INFO;


/*******************************************************************************
*  structure of install.dat                                                    *
*******************************************************************************/
typedef struct
{
	char achTermApplVer[5];				// 단말기 S/W 버전
	char achMainTermID[9];				// 단말기ID
	char achTranspBizrID[9];			// 교통사업자ID
	char achVehicleID[9];				// 차량ID
	byte abTermSetupDtime[7];			// 설치일자
	char abTranspMethodCode[3];			// 교통수단코드
	char achDeviceClassCode[2];			// 장비구분코드
	char achDCSIPAddr[12];				// 집계 IP
	char achIPAddrClassCode[2];			// IP주소 구분자
	char achLocalIPAddr[12];			// DHCP 할당 IP
	char achPSAMPort[4];				// PSAM PORT
	char achSubTermPSAMExistYN[3];		// 하차단말기 PSAM 존재여부
	char achFlashMemFreeSize [3];		// FLASH MEMORY Free Size
	char achPGLoaderVer[4];				// PGLOADER 버전
	byte abMainISAMID[4];				// 승차단말기 ISAM ID
	byte abSubISAMID[4];				// 하차단말기 ISAM ID
	byte abBLCRC[4];					// BL CRC32
	byte abMifPrepayPLCRC[4];			// 구선불PL CRC32
	byte abPostpayPLCRC[4];				// 구후불PL CRC32
	byte abAICRC[4];					// AI(신선불PL) CRC32
	byte abPSAMVer[2];					// PSAM 버전
	byte bSAMCnt;						// PSAM ID 개수
	byte abSamIDList[16*4];				// PSAM ID
} __attribute__((packed)) INSTALL_INFO;



/*******************************************************************************
*  structure of LEAP_PASSWD - tc_leap.dat                                      *
*******************************************************************************/
typedef struct
{
	byte 	abMainTermID[9];          	// main terminal ID
	byte 	abLEAPPasswd[20];       	// Leap password		
} __attribute__((packed)) LEAP_PASSWD; 


/*******************************************************************************
*  structure of COMM_INFO - setup.dat                                          *
*******************************************************************************/
typedef struct
{
	byte 	abVehicleID[9];          	// vehicle ID
	byte 	abDCSIPAddr[12];       		// DCS server IP (ASCII type)	
	
} __attribute__((packed)) COMM_INFO; 

typedef struct
{
	byte abApplyDtime[7];			// apply date
	byte bApplySeqNo;			// apply sequence number
	byte abRecordCnt[3];			// record count
} __attribute__((packed)) TEMP_PREPAY_ISSUER_INFO_HEADER;

typedef struct
{
	byte abPrepayIssuerID[7];    // prepay issuer ID
	byte bAssocCode;	     // terminal association class code
	byte bXferDisYoungCardApply; // transfer discount/young card apply Y/N
} __attribute__((packed)) TEMP_PREPAY_ISSUER_INFO;

typedef struct
{
	byte abApplyDtime[7];			// apply date (BCD)
	word wRecordCnt;			// record count
} __attribute__((packed)) TEMP_POSTPAY_ISSUER_INFO_HEADER;

typedef struct
{
	byte abPrefixNo[3];			// Prefix No. (BCD)
	word wCompCode;				// compress code
} __attribute__((packed)) TEMP_POSTPAY_ISSUER_INFO;

typedef struct
{
	byte abApplyDtime[14];			// apply date (ASC)
	byte abRecordCnt[4];			// record count (ASC)
	byte abCrLf[2];					// special character
} __attribute__((packed)) TEMP_POSTPAY_ISSUER_VALID_PERIOD_INFO_HEADER;

typedef struct
{
	byte abPrefixNo[6];				// Prefix No. (ASC)
	byte abIssuerID[7];				// card issuer ID
	byte abExpiryDate[6];			// card expiry date (YYYYMM)
	byte abCrLf[2];					// special character
} __attribute__((packed)) TEMP_POSTPAY_ISSUER_VALID_PERIOD_INFO;

typedef struct
{
	byte abApplyDtime[7];			// apply date
	byte abXferApplyStartTime[4];		// transfer apply start time
	byte abXferApplyEndTime[4];		// transfer apply end time
	byte bHolidayClassCode;			// holiday class code
	byte abXferEnableTime[3];		// transfer enable time
	byte abXferEnableCnt[2];		// transfer enable count
	byte abCrLf[2];				// special character
} __attribute__((packed)) TEMP_XFER_APPLY_INFO;

typedef struct
{
	byte abApplyDtime[7];			// apply date (BCD)
	byte bApplySeqNo;			// apply sequence number
	byte abRecordCnt[3];			// record count  (BCD)
} __attribute__((packed)) TEMP_DIS_EXTRA_INFO_HEADER;

typedef struct
{
	byte bTranspCardClassCode;		
		// transportation card class code
	byte abDisExtraTypeID[6];		// discount or extra type ID
	byte bPositiveNegativeClass;		// positive or negative class
	byte bDisExtraApplyStandardCode;	
		// discount or extra apply standard code
	byte abDisExtraRate[6];			// discount/extra rate
	byte abDisExtraAmt[3];			// discount/extra  amount
	byte abCrLf[2];				// special character
} __attribute__((packed)) TEMP_DIS_EXTRA_INFO;

typedef struct
{
	byte abApplyDtime[7];			// apply date
	byte abRecordCnt[3];			// record count
} __attribute__((packed)) TEMP_HOLIDAY_INFO_HEADER;

typedef struct
{
	byte abApplyDtime[7];			// apply  date
	byte bApplySeqNo;			// apply  sequence number
	byte abTranspMethodCode[3];		// transportation method code
	byte abSingleFare[3];			// single fare
	byte abBaseFare[3];			// base fare
	byte abBaseDist[3];			// base distance
	byte abAddedFare[3];			// added fare
	byte abAddedDist[3];			// added distance
	byte abOuterCityAddedDist[3];		// outer city added distance
	byte abOuterCityAddedDistFare[3];	// outer city added distancefare
	byte abAdultCashEntFare[3];		// adult cash enter fare
	byte abChildCashEntFare[3];		// child cash enter fare
	byte abYoungCashEntFare[3];		// youngcash enter fare
	byte abPermitErrDist[3];		// permit error distance
	byte abPermitErrRate[6];		// permit error rate 
	struct
	{
		byte abSourceFare[3];		// source fare
		byte abChangeFare[3];		// change fare
	} __attribute__((packed)) FARE_INFO[100];
	byte abCrLf[2];				// special character
} __attribute__((packed)) TEMP_NEW_FARE_INFO;

/*******************************************************************************
* structure of TEMP_STATION_INFO_HEADER - c_st_inf.dat                         *
*******************************************************************************/
typedef struct
{
	byte abApplyDtime[7];			// apply date (BCD)
	byte bApplySeqNo;			// apply sequence number
	byte abRouteID[8];			// bus route  ID        
	byte abTranspMethodName[16];		// bus route name         
	byte abRecordCnt[3];			// bus station count  
} __attribute__((packed)) TEMP_STATION_INFO_HEADER;

typedef struct
{
	byte abStationID[7];			// bus station  ID
	byte bCityInOutClassCode;		// city in or out class  code
	byte abStationOrder[3];			// station order 
	byte abStationName[16];			// bus station name
	byte abStationLongitude[10];		// bus station  longitude
	byte abStationLatitude[9];		// bus station  latitude 
	byte abOffset[3];			// offset
	byte abDistFromFirstStation[3];		// first station distance (BCD)
	byte abStationApproachAngle[3];		// station approach angle 
} __attribute__((packed)) TEMP_STATION_INFO;


/*******************************************************************************
* structure of TEMP_VEHICLE_PARM - op_par.dat                                  *
*******************************************************************************/
typedef struct
{
	byte	abApplyDtime[14];      		// apply  date    
	byte    abTranspBizrID[9];            	
		// transportation Business company ID
	byte    abBusBizOfficeID[2];          	// bus business office  ID
	byte    abVehicleID [9];              	// vehicle ID      
	byte    abVehicleNo [20];             	// vehicle No.     
	byte    abRouteID [8];                	// route ID      
	byte    abDriverID [6];               	// driver ID    
	byte    abTranspMethodCode[3];       	// transportation method code 
	byte    abBizNo[10];                  	// business No.   
	byte    abTranspBizrNm[20];           	
		// transportation Business company name 
	byte    abAddr[103];                  	// address         
	byte    abRepreNm[10];                	// representative name     
} __attribute__((packed)) TEMP_VEHICLE_PARM;


/*******************************************************************************
*  structure of ROUTE_PARM - li_par.dat                              	       *
*******************************************************************************/
typedef struct
{
	byte	abApplyDtime[14];                 // apply  date                    
	byte    abRouteID[8];                     // route  ID                      
	byte    abTranspMethodCode[3];            // transportation method code                 
	byte    abXferDisApplyFreq[2];            
		// transfer discount  apply frequency           
	byte    abXferApplyTime [3];              // transfer apply  time                
	byte    abTermGroupCode[2];               // terminal group  code              
	byte    abSubTermCommIntv[3];             
		// Sub terminal and communication  interval       
	byte    abCardProcTimeIntv [3];           
		// card process  time  interval          
	byte    abUpdateBLValidPeriod [2];        // update  B/L valid period             
	byte    abUpdatePLValidPeriod[2];         // update  P/L valid period             
	byte    abUpdateAIValidPeriod[2];         // update  AI valid period              
	byte    bDriverCardUseYN;                 // driver carduser Y/N           
	byte    abChargeInfoSendFreq[2];          
		// charge  info send frequency           
	byte    bRunKindCode;                     // run kind code                
	byte    bGyeonggiIncheonRangeFareInputWay;       
		// Gyeonggi Incheon range fare input  way 
	byte    abLEAPPasswd[10];                 // LEAP password                    
	byte    bECardUseYN;                      // postpay e_card use Y/N         

} __attribute__((packed)) TEMP_ROUTE_PARM;

/*******************************************************************************
*  structure of TEMP_EPURSE_ISSUER_REGIST_INFO - idcenter.dat                  *
*******************************************************************************/
typedef struct { 
	byte	abApplyDtime[7];   		// apply date    
	byte    abRecordCnt [2];     		// record count    
} __attribute__((packed))TEMP_EPURSE_ISSUER_REGIST_INFO_HEADER;

typedef struct { // tot 35
	hex		passwid_alg_cd;	  	// algorithm id
	hex		keyset_ver;		// keyset version  issuer id
	bcd		sam_id[8];		// sam id
	hex		ekv[16];		// EKV
	hex		sign[4];		// Sign
} __attribute__((packed))TEMP_EPURSE_ISSUER_REGIST_INFO;


/*******************************************************************************
*  structure of TEMP_PSAM_KEYSET_INFO - keyset.dat                             *
*******************************************************************************/
typedef struct { 
	byte	abApplyDtime[7];   		// apply date    
	byte    abRecordCnt [2];     	// record count    
} __attribute__((packed))TEMP_PSAM_KEYSET_INFO_HEADER;

typedef struct {
	hex		passwd_alg_cd;		// 1 password algorithm 
	hex		keyset_ver;		// 1 keyset version 
	bcd		sam_id[8]; 		// 8 SAM ID
	hex		sort_key;		// sort key (key kind )
	hex		vk[4];			// vk (key version )
	hex		ekv[64];		
		// ekv electronic purse Issuer keyset 
	hex		sign[4];		// sign
} __attribute__((packed))TEMP_PSAM_KEYSET_INFO;


/*******************************************************************************
*  structure of VER_INFO - c_ve_inf.dat                              	       *
*******************************************************************************/
typedef struct
{
	byte	abVerInfoCreateDtime[14];// version info creation date            
	byte    abMasterBLVer[14];       // fixed B/Lversion                  
	dword   dwMasterBLPkt;           // fixed B/L Pkt                
	byte    abUpdateBLVer[14];       // update B/Lversion                  
	dword   dwUpdateBLPkt;           // update B/L Pkt                
	byte    abHotBLVer[14];          // HOTB/Lversion                   
	dword   dwHotBLPkt;              // HOTB/L Pkt                 
	byte    abMasterPrepayPLVer[14]; // fixed P/L version (prepay )           
	dword   dwMasterPrepayPLPkt;     // fixed P/L(prepay ) Pkt          
	byte    abMasterPostpayPLVer[14];// fixed P/L version (postpay)           
	dword   dwMasterPostpayPLPkt;    // fixed P/L (postpay) Pkt         
	byte    abUpdatePLVer[14];       // update PL version                   
	dword   dwUpdatePLPkt;           // update PL Pkt                 
	byte    abHotPLVer[14];          // HOT P/Lversion                   
	dword   dwHotPLVerPkt;           // HOT P/L Pkt                 
	byte    abMasterAIVer[14];       // fixed AI version (new transportation )          
	dword   dwMasterAIPkt;           // fixed AI(new transportation ) Pkt         
	byte    abUpdateAIVer[14];       // update AI version (new transportation )          
	dword   dwUpdateAIPkt;           // update AI(new transportation ) Pkt         
	byte    abHotAIVer[14];          // HOT AI version (new transportation )           
	dword   dwHotAIPkt;              // HOT AI(new transportation ) Pkt          
	byte    abVehicleParmVer[14];    // run vehicle parameter version         
	dword   dwVehicleParmPkt;        // run vehicle parameter  Pkt       
	byte    abNextVehicleParmVer[14];// run vehicle parameter next version     
	dword   dwNextVehicleParmPkt;    // run vehicle parameter next  Pkt   
	byte    abRouteParmVer[14];      // route  parameter version           
	dword   dwRouteParmPkt;          // route  parameter  Pkt         
	byte    abNextRouteParmVer[14];  // route  parameter next version       
	dword   dwNextRouteParmPkt;      // route  parameter next  Pkt     
	byte    abBasicFareInfoVer[14];  // basic fare info version             
	dword   dwBasicFareInfoPkt;       // basic fare info  Pkt           
	byte    abNextBasicFareInfoVer[14];// basic fare info next version         
	dword   dwNextBasicFareInfoPkt;  // basic fare info next  Pkt       
	byte    abOldFareVer [14];       // old single fare                  
	dword   dwOldFarePkt;            // old single fare Pkt             
	byte    abNextOldFareVer[14];    // old single fare next version           
	dword   dwNextOldFarePkt;        // old single fare next  Pkt         
	byte    abOldAirFareVer[14];     // old airport  fare(A~C)             
	dword   dwOldAirFarePkt;         // old airport  fare(A~C) Pkt        
	byte    abNextOldAirFareVer[14]; // old airport  fare(A~C)next version      
	dword   dwNextOldAirFarePkt;     // old airport  fare(A~C)next version  Pkt
	byte    abOldRangeFareVer[14];   // old range  fare                  
	dword   dwOldRangeFarePkt;       // old range  fare Pkt             
	byte    abNextOldRangeFareVer[14];// old range  fare next version           
	dword   dwNextOldRangeFarePkt;   // old range  fare next  Pkt         
	byte    abBusStationInfoVer[14]; // bus station interval info version         
	dword   dwBusStationInfoPkt;     // bus station interval info  Pkt       
	byte    abNextBusStationInfoVer[14];// bus station info next version     
	dword   dwNextBusStationInfoPkt; // bus station interval info next  Pkt   
	byte    abPrepayIssuerInfoVer[14];// prepay issuer info version           
	dword   dwPrepayIssuerInfoPkt;   // prepay issuer info  Pkt         
	byte    abNextPrepayIssuerInfoVer[14];// prepay issuer info next version       
	dword   dwNextPrepayIssuerInfoPkt;// prepay issuer info next  Pkt     
	byte    abPostpayIssuerInfoVer[14];// postpay issuer info version           
	dword   dwPostpayIssuerInfoPkt;   // postpay issuer info  Pkt         
	byte    abNextPostpayIssuerInfoVer[14];// postpay issuer info next version       
	dword   dwNextPostpayIssuerInfoPkt;// postpay issuer info next  Pkt     
	byte    abDisExtraInfoVer[14];    // discount or extra info version             
	dword   dwDisExtraInfoPkt;        // discount or extra info  Pkt           
	byte    abNextDisExtraInfoVer[14];// discount or extra info next version         
	dword   dwNextDisExtraInfoPkt;    // discount or extra info next  Pkt       
	byte    abHolidayInfoVer[14];     // holiday info version                 
	dword   dwHolidayInfoPkt;         // holiday info  Pkt               
	byte    abNextHolidayInfoVer[14]; // holiday info next version             
	dword   dwNextHolidayInfoPkt;     // holiday info next  Pkt           
	byte    abDriverOperatorApplVer[14];// driver operator Program version      
	dword   dwDriverOperatorApplPkt;  // driver operator Program Pkt    
	byte    abNextDriverOperatorApplVer[14];// driver operator  next version  
	dword   dwNextDriverOperatorApplPkt;// driver operator Program next  Pkt
	byte    abMainTermApplVer[14];    // enter terminal Program           
	dword   dwMainTermApplPkt;        // enter terminal Program Pkt      
	byte    abNextMainTermApplVer[14];// enter terminal Program next version    
	dword   dwNextMainTermApplPkt;    // enter terminal Program next  Pkt  
	byte    abSubTermApplVer[14];     // sub terminal Program           
	dword   dwSubTermApplPkt;         // sub terminal Program Pkt      
	byte    abNextSubTermApplVer[14]; // sub terminal Program next version    
	dword   dwNextSubTermApplPkt;     // sub terminal Program next  Pkt  
	byte    abVoiceInfoVer[14];       // voice info version                 
	dword   dwVoiceInfoPkt;           // voice info version  Pkt           
	byte    abNextVoiceInfoVer[14];   // voice info next version             
	dword   dwNextVoiceInfoPkt;       // voice info next  Pkt           
	byte    abXferApplyRegulationVer[14];// transfer regulation version                 
	dword   dwXferApplyRegulationPkt; // transfer regulation  Pkt               
	byte    abNextXferApplyRegulationVer[14];// transfer regulation next version             
	dword   dwNextXferApplyRegulationPkt;// transfer regulation next  Pkt           
	byte    abEpurseIssuerRegistInfoVer[14];// Payment SAM purse issuer version          
	dword   dwEpurseIssuerRegistInfoPkt; // Payment SAM purse issuer  Pkt        
	byte    abNextEpurseIssuerRegistInfoVer[14];// PSAM issuer next version      
	dword   dwNextEpurseIssuerRegistInfoPkt;// PSAM issuer next  Pkt    
	byte    abPSAMKeysetVer[14];      // Payment SAM KeySet version            
	dword   dwPSAMKeysetPkt;          // Payment SAM KeySet  Pkt          
	byte    abNextPSAMKeysetVer[14];  // Payment SAM KeySet next version        
	dword   dwNextPSAMKeysetPkt;      // Payment SAM KeySet next  Pkt      
	byte    abAutoChargeKeysetVer[14];// auto-charge SAM KeySet version        
	dword   dwAutoChargeKeysetPkt;    // auto-charge SAM KeySet  Pkt      
	byte    abNextAutoChargeKeysetVer[14];// auto-charge SAM KeySet next version    
	dword   dwNextAutoChargeKeysetPkt;// auto-charge SAM KeySet next  Pkt  
	byte    abAutoChargeParmVer[14];  // auto-charge Parameterversion        
	dword   dwAutoChargeParmPkt;      // auto-charge Parameter Pkt      
	byte    abNextAutoChargeParmVer[14];// auto-charge Parameter next version    
	dword   dwNextAutoChargeParmPkt;  // auto-charge Parameter next  Pkt  
	byte    abXferApplyInfoVer[14];   // transfer apply version                 
	dword   dwXferApplyInfoPkt;       // transfer apply  Pkt               
	byte    abNextXferApplyInfoVer[14];// transfer apply next version             
	dword   dwNextXferApplyInfoPkt;   // transfer apply next  Pkt           
	byte    abIssuerValidPeriodInfoVer[14];// issuer valid period version           
	dword   dwIssuerValidPeriodInfoPkt;// issuer valid period  Pkt         
	byte    abNextIssuerValidPeriodInfoVer[14];// issuer valid period next ver       
	dword   dwNextIssuerValidPeriodInfoPkt;// issuer valid period next  Pkt     
	byte    abCrLf[2];                // special character                    
} __attribute__((packed)) VER_INFO;

  
/*******************************************************************************
*  Declaration of variables ( FILE NAME )                                      *
*******************************************************************************/
#define NEXT_VEHICLE_PARM_FILE              "n_op_par.dat"
#define NEXT_ROUTE_PARM_FILE                "n_li_par.dat"
#define TMP_VEHICLE_PARM_FILE               "tmp_c_op_par.dat"
#define TMP_NEXT_VEHICLE_PARM_FILE          "tmp_n_op_par.dat"
#define TMP_ROUTE_PARM_FILE                 "tmp_c_li_par.dat"
#define TMP_NEXT_ROUTE_PARM_FILE            "tmp_n_li_par.dat"
#define NEXT_PREPAY_ISSUER_INFO_FILE        "n_ap_inf.dat"
#define TMP_PREPAY_ISSUER_INFO_FILE         "tmp_c_ap_inf.dat"
#define TMP_NEXT_PREPAY_ISSUER_INFO_FILE    "tmp_n_ap_inf.dat"
#define NEXT_POSTPAY_ISSUER_INFO_FILE       "n_dp_inf.dat"
#define TMP_POSTPAY_ISSUER_INFO_FILE        "tmp_c_dp_inf.dat"
#define TMP_NEXT_POSTPAY_ISSUER_INFO_FILE   "tmp_n_dp_inf.dat"
#define NEXT_XFER_APPLY_INFO_FILE           "n_tr_inf.dat"
#define TMP_XFER_APPLY_INFO_FILE            "tmp_c_tr_inf.dat"
#define TMP_NEXT_XFER_APPLY_INFO_FILE       "tmp_n_tr_inf.dat"
#define NEXT_DIS_EXTRA_INFO_FILE            "n_de_inf.dat"
#define TMP_DIS_EXTRA_INFO_FILE             "tmp_c_de_inf.dat"
#define TMP_NEXT_DIS_EXTRA_INFO_FILE        "tmp_n_de_inf.dat"
#define NEXT_HOLIDAY_INFO_FILE              "n_ho_inf.dat"
#define TMP_HOLIDAY_INFO_FILE               "tmp_n_ho_inf.dat"
#define TMP_NEXT_HOLIDAY_INFO_FILE          "tmp_n_ho_inf.dat"
#define NEXT_NEW_FARE_INFO_FILE             "n_n_far.dat"
#define TMP_NEW_FARE_INFO_FILE              "tmp_c_n_far.dat"
#define TMP_NEXT_NEW_FARE_INFO_FILE         "tmp_n_n_far.dat"
#define NEXT_BUS_STATION_INFO_FILE          "n_st_inf.dat"
#define TMP_BUS_STATION_INFO_FILE           "tmp_c_st_inf.dat"
#define TMP_NEXT_BUS_STATION_INFO_FILE      "tmp_n_st_inf.dat"
#define NEXT_EPURSE_ISSUER_REGIST_INFO_FILE     	"n_idcenter.dat"
#define EPURSE_ISSUER_REGIST_INFO_FILE      		"c_idcenter.dat"
#define TMP_EPURSE_ISSUER_REGIST_INFO_FILE      	"tmp_c_idcenter.dat"
#define TMP_NEXT_EPURSE_ISSUER_REGIST_INFO_FILE 	"tmp_n_idcenter.dat"
#define NEXT_PSAM_KEYSET_INFO_FILE          		"n_keyset.dat"
#define PSAM_KEYSET_INFO_FILE               		"c_keyset.dat"
#define TMP_PSAM_KEYSET_INFO_FILE           		"tmp_c_keyset.dat"
#define TMP_NEXT_PSAM_KEYSET_INFO_FILE      		"tmp_n_keyset.dat"
#define NEXT_AUTO_CHARGE_SAM_KEYSET_INFO_FILE       "n_alsam_key.dat"
#define TMP_AUTO_CHARGE_SAM_KEYSET_INFO_FILE        "tmp_c_alsam_key.dat"
#define TMP_NEXT_AUTO_CHARGE_SAM_KEYSET_INFO_FILE   "tmp_n_alsam_key.dat"
#define NEXT_AUTO_CHARGE_PARM_INFO_FILE     "n_alsam_para.dat"
#define TMP_AUTO_CHARGE_PARM_INFO_FILE      "tmp_c_alsam_par.dat"
#define TMP_NEXT_AUTO_CHARGE_PARM_INFO_FILE "tmp_n_alsam_par.dat"

#define NEXT_KPD_IMAGE_FILE                 "n_dr_pro.dat"
#define TMP_KPD_IMAGE_FILE                  "tmp_c_dr_pro.dat"
#define TMP_NEXT_KPD_IMAGE_FILE             "tmp_n_dr_pro.dat"

#define NEXT_BUS_MAIN_IMAGE_FILE            "n_en_pro.dat"
#define TMP_BUS_MAIN_IMAGE_FILE             "tmp_c_en_pro.dat"
#define TMP_NEXT_BUS_MAIN_IMAGE_FILE        "tmp_n_en_pro.dat"

#define TMP_BUS_SUB_IMAGE_FILE              "tmp_c_ex_pro.dat"
#define NEXT_BUS_SUB_IMAGE_FILE             "n_ex_pro.dat"
#define TMP_NEXT_BUS_SUB_IMAGE_FILE         "tmp_n_ex_pro.dat"

#define NEXT_VOICE0_FILE                    "n_v0.dat"
#define TMP_VOICE0_FILE                     "tmp_c_v0.dat"
#define TMP_NEXT_VOICE0_FILE                "tmp_n_v0.dat"

#define UPDATE_BL_FILE                      "c_ch_bl.dat"
#define HOT_BL_FILE                         "c_ho_bl.dat"
#define DOWN_ING_HOT_BL_FILE                "tmp_c_ho_bl.dat"
#define HOT_PL_FILE                         "c_ho_pl.dat"
#define DOWN_ING_HOT_PL_FILE                "tmp_c_ho_pl.dat"
#define HOT_AI_FILE                         "c_ho_ai.dat"
#define DOWN_ING_HOT_AI_FILE                "tmp_c_ho_ai.dat"

#define UPDATE_BL_BEFORE_VER_FILE           "v_bl_be.dat"
#define UPDATE_PL_BEFORE_VER_FILE           "v_pl_be.dat"
#define UPDATE_AI_BEFORE_VER_FILE           "v_ai_be.dat"
#define ISSUER_VALID_PERIOD_INFO_FILE 		"c_cd_inf.dat"
#define AUTO_CHARGE_PARM_INFO_FILE          		"c_alsam_para.dat"
#define AUTO_CHARGE_SAM_KEYSET_INFO_FILE    		"c_alsam_key.dat"

#define VER_INFO_FILE                       "c_ve_inf.dat"
#define UPLOAD_VER_INFO_FILE                "version.trn"

#define RECONCILE_BACKUP_FILE               "reconcileinfo.dat.back"
#define RECONCILE_TMP_FILE                  "reconcileinfo.dat.tmp"
#define DOWN_FILE_BACKUP_FILE               "downloadinfo.dat.back"
#define MERGE_FLAG_FILE                     "bl_pl_ctrl.flg"


/*******************************************************************************
*  Declaration of Extern variables                                             *
*******************************************************************************/
extern COMM_INFO		gstCommInfo;

extern char* achDCSCommFile[][2];

extern bool boolIsApplyNextVer;		// 0 - cannot apply , 1 - can apply
extern bool boolIsApplyNextVerParm;
extern bool boolIsApplyNextVerVoice;
extern bool boolIsApplyNextVerAppl;
extern bool boolIsApplyNextVerDriverAppl;

#define DCS_COMM_FILE_CNT   		48

/*******************************************************************************
*  Declaration of function                                                     *
*******************************************************************************/
short GetFileNo( char *pchFileName );
short GetDownFileIndex( char* pchCmd );
void GetRecvFileName( USER_PKT_MSG*      pstRecvUsrPktMsg,
                      PKT_HEADER_INFO*   pstPktHeaderInfo,
                      char*  			 achRecvFileName );
int LockFile( int fd );
int UnlockFile( int fd );
short WriteCommSuccTime( void );
short GetCommSuccTimeDiff( char *pchDtime );
byte GetDCSCommFileNoByFileName( byte *abFileName );

#endif
