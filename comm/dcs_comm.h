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
*  PROGRAM ID :       dcs_comm.h	                                           *
*                                                                              *
*  DESCRIPTION:       집계시스템과의 통신을 위한 함수들을 제공한다.              *
*                                                                              *
*  ENTRY POINT:     void InitPkt( USER_PKT_MSG* stUserMsg );				   *
*					short SendUsrPkt2DCS( 	int fdSock,						   *
*                             				int nTimeOut,					   *
*                             				int nRetryCnt,					   *
*                             				USER_PKT_MSG* stUsrPktMsg );	   *
*					short RecvUsrPktFromDCS( int fdSock,					   *
*                                			 int nTimeOut,					   *
*                                			 int nRetryCnt,	 				   *
*                                			 USER_PKT_MSG* stUsrPktMsg );	   *
*					int OpenSession( char *pchIP, char *pchPort );			   *
*					short CloseSession( int fdSock );	  					   *
*					short AuthDCS( 	int fdSock,								   *
*                      				char* pchSessionCd,						   *
*                      				USER_PKT_MSG* pstSendUsrPktMsg,	 		   *
*                      				USER_PKT_MSG* pstRecvUsrPktMsg );		   *
*					short SendNRecvUsrPkt2DCS( int fdSock,					   *
*                          					   USER_PKT_MSG* pstSendUsrPktMsg, *
*                          					   USER_PKT_MSG* pstRecvUsrPktMsg )*
*					short SendRS( int fdSock, USER_PKT_MSG* pstSendUsrPktMsg );*
*					short SendEOS( int fdSock, USER_PKT_MSG* pstSendUsrPktMsg )*
*					short SendEOF( int fdSock, USER_PKT_MSG* pstSendUsrPktMsg )*
*					short RecvRS( int fdSock, USER_PKT_MSG* pstRecvUsrPktMsg );*
*					short RecvACK( int fdSock, USER_PKT_MSG* pstRecvUsrPktMsg )*
*					short GetDownFileIndex( char* pchCmd );						   *
*					void GetRecvFileName( USER_PKT_MSG* pstRecvUsrPktMsg,	   *
*                             			  PKT_HEADER_INFO*   pstPktHeaderInfo, *
*                             			  char*  achRecvFileName );			   *
*					short ReSetupTerm( void );								   *
*					short SetupTerm( void );								   *
*					void *DCSComm( void *arg );								   *
*                                                                              *
*  INPUT FILES:     None                                                       *
*                                                                              *
*  OUTPUT FILES:      c_op_par.dat - 운행차량파라미터 파일                      *
*                                                                              *
*  SPECIAL LOGIC:     None                                                     *
*                                                                              *
********************************************************************************
*                         MODIFICATION LOG                                     *
*                                                                              *
*    DATE                SE NAME                      DESCRIPTION              *
* ---------- ---------------------------- ------------------------------------ *
* 2006/03/27 F/W Dev Team Mi Hyun Noh  Initial Release                         *
*                                                                              *
*******************************************************************************/


#ifndef _DCS_COMM_H_
#define _DCS_COMM_H_

/*******************************************************************************
*  Inclusion of Header Files                                                   *
*******************************************************************************/
#include "../system/bus_type.h"

/*******************************************************************************
*  structure of PKT_MSG                                                        *
*******************************************************************************/
typedef struct
{
	byte	bSTX;               	// communication STX          
	char    achPktSize[4];          // communication packet size         
	char    chEncryptionYN;         // communication data encryption Y/N 
	byte    abConnSeqNo[4];         // communication sequence             
	char    achConnCmd[5];          // communication command         
	char    achDataSize[4];         // data Size            
	byte*   pbRealSendRecvData;     // Real send/receive data       
	byte    bETX;                   // etx                 
	byte    bBCC;                   // bcc                 
} __attribute__((packed)) PKT_MSG; 


/*******************************************************************************
*  structure of USER_PKT_MSG                                                   *
*******************************************************************************/
typedef struct
{
	char	chEncryptionYN;       	// communication data encryption Y/N 
	char    achConnCmd [5];         // communication command         
	long    lDataSize;              // dataSize            
	byte*   pbRealSendRecvData;     // send/receive DATA         
	int     nMaxDataSize;           // data size           
} __attribute__((packed)) USER_PKT_MSG; 


/*******************************************************************************
*  structure of DCS_FILE_NAME                                                *
*******************************************************************************/
typedef struct
{
	byte 	abDCSFileNameHeader[7];
	byte 	abTranspBizrID[9];            	// 교통사업자 ID
	byte	bSeperator1;					// 구분자 (.)
	byte 	abMainTermID[9];				// 승차단말기 ID
	byte	bSeperator2;					// 구분자 (.)
	byte 	abFileName[14];					// 버스 단말기내 파일명 
	byte 	abDCSFileNameTail[9];			// [0]-. [1]-sendStauts 나머지-space
} __attribute__((packed)) DCS_FILE_NAME; 


/*******************************************************************************
*  structure of PKT_HEADER_INFO                                                *
*******************************************************************************/
typedef struct
{
	byte	bNextSendFileExistYN;		// File Transfer Y/N      
	byte    abVehicleID[9];        		// terminal  ID               
	byte    bDataType;              	// Data Type   '0' : file '1' : message       
	byte    bNewVerYN;             		// File Version Type  0 - curr 1 - next
	DCS_FILE_NAME    stDCSFileName;  	// send file name              
	byte    abFileVer[7];           		// send file version             
	byte    abTotalPktCnt[4];       		// total packet count                 
	byte    abFileSize[4];          		// count new  file size             
	byte    abFileCheckSum[2];      		// fileCheckSum            
	byte    abFirstPktNo[4];        		// first packet No. of chain  file
	char    achSendDtime[14];       		// send time                 
} __attribute__((packed)) PKT_HEADER_INFO; 


/*******************************************************************************
*  structure of RS_COMMAND                                              	   *
*******************************************************************************/
typedef struct
{
	byte	abMainTermID[9];			// 승차단말기 ID   
	/*
	 *  Sender에서 Setting하여 송신한 Packet Seq. NO임 
	 *  (단,EOS/EOT/EOS의 경우에는 Sender는 0으로 Setting함)            
	 */
	byte    abRecvSeqNo[4];          	
	byte    abRecvStatusCode[4];        // 수신상태 코드 정상-'0000'
	/*
	 *	Status Code가 "0000"이면 본 항목은 Data에서 생략됨  
	 */
	byte    abRecvStatusMsg[40];            
} __attribute__((packed)) RS_COMMAND; 


/*******************************************************************************
*  structure of CONN_FILE_NAME                                            	   *
*******************************************************************************/
typedef struct
{
	char	achFileNameHeader[7];		// "B001.0."
	byte	abTranspBizrID[9];
	char	chClass1;					// '.'
	byte	abMainTermID[9];
	char	chClass2;					// '.'
	char 	achFileName[14];
	char	achSpace[9];				// 0x20
//	char	chZero;						// 0x30
} __attribute__((packed)) CONN_FILE_NAME; 

/*******************************************************************************
*  structure of RECONCILE_DTAIL_RESULT                                     	   *
*******************************************************************************/
typedef struct
{   
   CONN_FILE_NAME 	stReconFileName;
   char 			chReconResult;
} __attribute__((packed)) RECONCILE_DTAIL_RESULT; 

/*******************************************************************************
*  structure of RECONCILE_RESULT_PKT                                       	   *
*******************************************************************************/
typedef struct
{
   char						achReconDataCnt[2];
   RECONCILE_DTAIL_RESULT 	stReconDtailResultList[20];
} __attribute__((packed)) RECONCILE_RESULT_PKT; 


/*******************************************************************************
*  structure of VER_INFO_FOR_DOWN                                              *
*******************************************************************************/
typedef struct
{
	char	bSystemClassCode;			// system class code   
	char    achVehicleID[9];          	// vehicle ID               
	char    bPktClassCode;             	// 다음 패킷 존재 여부 1-없음      
	char    achVerInfoCnt[4];           // version information count 
	char    chOldNewClassCode;  		// File Version Type  0 - curr 1 - next            
//	byte   	abVerInfo[960];             	// version information        
	char   	achDriverOperatorApplCmd[5];    //driver operator Program cmd
	char	achDriverOperatorApplVer[7];	// (BCD)
	dword	dwDriverOperatorApplFileSize;
	dword   dwDriverOperatorApplPktSize;
	char   	achMainTermApplCmd[5];     	//main terminal Program cmd
	char	achMainTermApplVer[7];		// (BCD)
	dword	dwMainTermApplFileSize;
	dword   dwMainTermApplPktSize;
	char   	achSubTermApplCmd[5];     	//sub terminal Program cmd
	char	achSubTermApplVer[7];		// (BCD)
	dword	dwSubTermApplFileSize;
	dword   dwSubTermApplPktSize;
	char   	achAppl4Cmd[5];     		//Program4 cmd
	char	achAppl4Ver[7];				// (BCD)
	dword	dwAppl4FileSize;
	dword   dwAppl4PktSize;
	char   	achAppl5Cmd[5];     		//Program5 cmd
	char	achAppl5Ver[7];				// (BCD)
	dword	dwAppl5FileSize;
	dword   dwAppl5PktSize;
	char   	achAppl6Cmd[5];     		//Program6 cmd
	char	achAppl6Ver[7];				// (BCD)
	dword	dwAppl6FileSize;
	dword   dwAppl6PktSize;
	char   	achAppl7Cmd[5];     		//Program7 cmd
	char	achAppl7Ver[7];				// (BCD)
	dword	 dwAppl7FileSize;
	dword   dwAppl7PktSize;
	char   	achAppl8Cmd[5];     		//Program8 cmd
	char	achAppl8Ver[7];				// (BCD)
	dword	dwAppl8FileSize;
	dword   dwAppl8PktSize;
	char   	achAppl9Cmd[5];     		//Program9 cmd
	char	achAppl9Ver[7];				// (BCD)
	dword	dwAppl9FileSize;
	dword   dwAppl9PktSize;
	char   	achAppl10Cmd[5];     		//Program10 cmd
	char	achAppl10Ver[7];			// (BCD)
	dword	dwAppl10FileSize;
	dword   dwAppl10PktSize;
	char   	achVoice0Cmd[5];     		//voice 0 cmd
	char	achVoice0InfoVer[7];		// (BCD)
	dword	dwVoice0FileSize;
	dword   dwVoice0PktSize;
	char   	achVoice1Cmd[5];     		//voice 1 cmd
	char	achVoice1InfoVer[7];		// (BCD)
	dword	dwVoice1FileSize;
	dword   dwVoice1PktSize;
	char   	achVoice2Cmd[5];     		//voice 2 cmd
	char	achVoice2InfoVer[7];		// (BCD)
	dword	dwVoice2FileSize;
	dword   dwVoice2PktSize;
	char   	achVoice3Cmd[5];     		//voice 3 cmd
	char	achVoice3InfoVer[7];		// (BCD)
	dword	dwVoice3FileSize;
	dword   dwVoice3PktSize;
	char   	achVoice4Cmd[5];     		//voice 4 cmd
	char	achVoice4InfoVer[7];		// (BCD)
	dword	dwVoice4FileSize;
	dword   dwVoice4PktSize;
	char   	achVoice5Cmd[5];     		//voice 5 cmd
	char	achVoice5InfoVer[7];		// (BCD)
	dword	dwVoice5FileSize;
	dword   dwVoice5PktSize;
	char   	achVoice6Cmd[5];     		//voice 6 cmd
	char	achVoice6InfoVer[7];		// (BCD)
	dword	dwVoice6FileSize;
	dword   dwVoice6PktSize;
	char   	achVoice7Cmd[5];     		//voice 7 cmd
	char	achVoice7InfoVer[7];		// (BCD)
	dword	dwVoice7FileSize;
	dword   dwVoice7PktSize;
	char   	achVoice8Cmd[5];     		//voice 8 cmd
	char	achVoice8InfoVer[7];		// (BCD)
	dword	dwVoice8FileSize;
	dword   dwVoice8PktSize;
	char   	achVoice9Cmd[5];     		//voice 9 cmd
	char	achVoice9InfoVer[7];		// (BCD)
	dword	dwVoice9FileSize;
	dword   dwVoice9PktSize;
	char   	achVehicleParmCmd[5];     	//vehicle parameter version cmd
	char	achVehicleParmVer[7];		// (BCD)
	dword	dwVehicleParmFileSize;
	dword   dwVehicleParmPktSize;
	char   	achRouteParmCmd[5];     	//route parameter version cmd
	char	achRouteParmVer[7];			// (BCD)
	dword	dwRouteParmFileSize;
	dword   dwRouteParmPktSize;
	char   	achBasicFareInfoCmd[5];     //basic fare information version cmd
	char	achBasicFareInfoVer[7];		// (BCD)
	dword	dwBasicFareInfoFileSize;
	dword   dwBasicFareInfoPktSize;
	char   	achOldFareCmd[5];     		//old single fare cmd
	char	achOldFareVer[7];			// (BCD)
	dword	dwOldFareFileSize;
	dword   dwOldFarePktSize;
	char   	achOldAirFareCmd[5];     	//old airport fare cmd
	char	achOldAirFareVer[7];		// (BCD)
	dword	dwOldAirFareFileSize;
	dword   dwOldAirFarePktSize;
	char   	achOldRangeFareCmd[5];     	//old old range fare cmd
	char	achOldRangeFareVer[7];		// (BCD)
	dword	dwOldRangeFareFileSize;
	dword   dwOldRangeFarePktSize;
	char   	achBusStationInfCmd[5];     //bus station information version cmd
	char	achBusStationInfoVer[7];	// (BCD)
	dword	dwBusStationInfFileSize;
	dword   dwBusStationInfoPktSize;
	char   	achPrepayIssuerInfoCmd[5];  //prepay issuer information version cmd
	char	achPrepayIssuerInfoVer[7];	// (BCD)
	dword	dwPrepayIssuerInfoFileSize;
	dword   dwPrepayIssuerInfoPktSize;
	char   	achPostpayIssuerInfoCmd[5]; //postpay issuer information version cmd
	char	achPostpayIssuerInfoVer[7];	// (BCD)
	dword	dwPostpayIssuerInfoFileSize;
	dword   dwPostpayIssuerInfoPktSize;
	char   	achDisExtraInfoCmd[5];     	//discount/extra informationversion cmd
	char	achDisExtraInfoVer[7];		// (BCD)
	dword	dwDisExtraInfoFileSize;
	dword   dwDisExtraInfoPktSize;
	char   	achHolidayInfoCmd[5];     	//holiday informationversion cmd
	char	achHolidayInfoVer[7];		// (BCD)
	dword	dwHolidayInfoFileSize;
	dword   dwHolidayInfoPktSize;
	char   	achXferApplyRegulationCmd[5]; 	//transfer apply regulation version
	char	achXferApplyRegulationVer[7];	// (BCD)
	dword	dwXferApplyRegulationFileSize;
	dword   dwXferApplyRegulationPktSize;
	char   	achEpurseIssuerRegistInfoCmd[5];// issuer regulation version cmd
	char	achEpurseIssuerRegistInfoVer[7];// (BCD)
	dword	dwEpurseIssuerRegistInfoFileSize;
	dword   dwEpurseIssuerRegistInfoPktSize;
	char   	achPSAMKeysetCmd[5];     		//payment SAM KeySet version cmd
	char	achPSAMKeysetVer[7];			// (BCD)
	dword	dwPSAMKeysetFileSize;
	dword   dwPSAMKeysetPktSize;
	char   	achAutoChargeKeyseCmd[5];     	//auto-charge SAM KeySet version cmd
	char	achAutoChargeKeysetVer[7];		// (BCD)
	dword	dwAutoChargeKeyseFileSize;
	dword   dwAutoChargeKeysetPktSize;
	char   	achAutoChargeParmCmd[5];     	//auto-charge  parameter version cmd
	char	achAutoChargeParmVer[7];		// (BCD)
	dword	dwAutoChargeParmFileSize;
	dword   dwAutoChargeParmPktSize;
	char   	achXferApplyInfoCmd[5];     	//transfer apply version cmd
	char	achXferApplyInfoVer[7];			// (BCD)
	dword	dwXferApplyInfoFileSize;
	dword   dwXferApplyInfoPktSize;
	char   	achIssuerValidPeriodInfoCmd[5]; //issuer valid period version  cmd
	char	achIssuerValidPeriodInfoVer[7];	// (BCD)
	dword	dwIssuerValidPeriodInfoFileSize;
	dword   dwIssuerValidPeriodInfoPktSize;
	char   	achMasterBLCmd[5];    	 		//fixed B/L version cmd
	char	achMasterBLVer[7];				// (BCD)
	dword	dwMasterBLFileSize;
	dword   dwMasterBLPktSize;
	char   	achUpdateBLCmd[5];     			//upadte B/L version cmd
	char	achUpdateBLVer[7];				// (BCD)
	dword	dwUpdateBLFileSize;
	dword   dwUpdateBLPktSize;
	char   	achHotBLCmd[5];     			//HOT B/L version cmd
	char	achHotBLVer[7];					// (BCD)
	dword	dwHotBLFileSize;
	dword   dwHotBLPktSize;
	char   	achMasterPrepayPLCmd[5];     	//fixed P/L version(prepay) cmd
	char	achMasterPrepayPLVer[7];		// (BCD)
	dword	dwMasterPrepayPLFileSize;
	dword   dwMasterPrepayPLPktSize;
	char   	achMasterPostpayPLCmd[5];     	//fixed P/L version(postpay) cmd
	char	achMasterPostpayPLVer[7];		// (BCD)
	dword	dwMasterPostpayPLFileSize;
	dword   dwMasterPostpayPLPktSize;
	char   	achUpdatePLCmd[5];     			//upadte PL version cmd
	char	achUpdatePLVer[7];				// (BCD)
	dword	dwUpdatePLFileSize;
	dword   dwUpdatePLPktSize;
	char   	achHotPLCmd[5];     			//HOT P/L version cmd
	char	achHotPLVer[7];					// (BCD)
	dword	dwHotPLFileSize;
	dword   dwHotPLPktSize;					// (BCD)Pkt
	char   	achMasterAICmd[5];     			//	master AI version cmd
	char	achMasterAIVer[7];				// (BCD)
	dword	dwMasterAIFileSize;
	dword   dwMasterAIPktSize;
	char   	achUpdateAICmd[5];     			//upadte AI version cmd
	char	achUpdateAIVer[7];				// (BCD)
	dword	dwUpdateAIFileSize;
	dword   dwUpdateAIPktSize;
	char   	achHotAICmd[5];     			//HOT AI version 
	char	achHotAIVer[7];					// (BCD)
	dword	dwHotAIFileSize;
	dword   dwHotAIPktSize;
} __attribute__((packed)) VER_INFO_FOR_DOWN; 

/*******************************************************************************
*  Declaration of variables                                                    *
*******************************************************************************/
#define EOF_CMD                         "EOF00"
#define EOT_CMD                         "EOT00"
#define EOS_CMD                         "EOS00"
#define SERVER_COMM_PORT            	"20000"     // 집계통신에 사용될 port
#define ENCRYPTION                  	'1'
#define NOT_ENCRYPTION              	'0'
#define TIMEOUT                     	10
#define JOB_SESSION_CODE_LEN    		2
#define COMMAND_LENGTH              	5
#define VER_INFO_LENGTH             	14

/*******************************************************************************
*  Declaration of Module Functions                                             *
*******************************************************************************/
void InitPkt( USER_PKT_MSG* stUserMsg );
short SendUsrPkt2DCS( int fdSock,
                             int nTimeOut,
                             int nRetryCnt,
                             USER_PKT_MSG* stUsrPktMsg );
short RecvUsrPktFromDCS( int fdSock,
                                int nTimeOut,
                                int nRetryCnt,
                                USER_PKT_MSG* stUsrPktMsg );

int OpenSession( char *pchIP, char *pchPort );
short CloseSession( int fdSock );
short AuthDCS( int fdSock,
                      char* pchSessionCd,
                      USER_PKT_MSG* stSendUsrPktMsg,
                      USER_PKT_MSG* stRecvUsrPktMsg );

short SendNRecvUsrPkt2DCS( int fdSock,
                          USER_PKT_MSG* stSendUsrPktMsg,
                          USER_PKT_MSG* stRecvUsrPktMsg );

short SendRS( int fdSock, USER_PKT_MSG* pstSendUsrPktMsg );
short SendEOS( int fdSock, USER_PKT_MSG* pstSendUsrPktMsg );
short SendEOF( int fdSock, USER_PKT_MSG* pstSendUsrPktMsg );
short RecvRS( int fdSock, USER_PKT_MSG* pstRecvUsrPktMsg );
short RecvACK( int fdSock, USER_PKT_MSG* pstRecvUsrPktMsg );

short ReSetupTerm( void );
short SetupTerm( void );
void *DCSComm( void *arg );

#endif
