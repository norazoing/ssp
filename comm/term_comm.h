
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
*  PROGRAM ID :       sc_read.c                                                *
*                                                                              *
*  DESCRIPTION:       This program reads Smart Card, checks validation of card *
*                     and saves Common Structure.                              *
*                                                                              *
*  ENTRY POINT:       SCRead()               ** optional **                    *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: 0 - Success                                              *
*                     (-) - Error                                              *
*                                                                              *
*  INPUT FILES:       Issure Info Files                                        *
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
* 2005/09/27 Solution Team  woolim        Initial Release                      *
* 2006/04/17 F/W Dev. Team  wangura       파일분리 및 추가 구조화              *
*                                                                              *
*******************************************************************************/
#ifndef _TERM_COMM_H
#define _TERM_COMM_H

/*******************************************************************************
*  Definition of Macro                                                         *
*******************************************************************************/
#define STX     0x02 /* Packet의 Start */
#define ETX     0x03 /* Packet의 End   */
#define EOT     0x04 /* Last Packet    */
#define ACK     0x06 /* Acknowledge    */
#define NAK     0x15 /* Nagative Acknowledge */
#define ETB     0x17 /* 파라미터 파일전체의 전송종료를 알리는 신호로 사용 */

#define MAIN_SUB_COMM_SUB_TERM_IMG_VER		'V'		/* Program Version */
#define MAIN_SUB_COMM_SUB_TERM_IMG_DOWN_OLD	'D'     
#define MAIN_SUB_COMM_SUB_TERM_IMG_DOWN		'd' 
#define MAIN_SUB_COMM_SUB_TERM_VOICE_VER	'H'		/* Voice Version */
#define MAIN_SUB_COMM_SUB_TERM_VOIC_DOWN	'A'		/* Voice Download */
#define MAIN_SUB_COMM_SUB_TERM_PARM_DOWN	'M'		/* Parameter Download */
#define MAIN_SUB_COMM_GET_SUB_TERM_ID		'G'		/* Sub Term ID */
#define MAIN_SUB_COMM_ASSGN_SUB_TERM_ID		'I'		/* Assign Sub Term ID */
#define MAIN_SUB_COMM_REQ_BL_SEARCH			'B'		/* req BL search */
#define MAIN_SUB_COMM_REQ_PL_SEARCH			'U'		/* req PL search */
#define MAIN_SUB_COMM_POLLING				'P'		/* Polling */
#define MAIN_SUB_COMM_GET_TRANS_CONT		'X'		/* get transaction contents */
#define MAIN_SUB_COMM_GET_TRANS_FILE		'T'		/* get transaction file */
#define MAIN_SUB_COMM_CHK_CRC				'C'		/* check CRC command - old ver*/
#define MAIN_SUB_COMM_REGIST_KEYSET			'K'		/* regist keyset */


#define MAX_TRIAL_COUNT_OPEN_UART   ( 3 )    /* UART Open시도 최대횟수 */
#define MAX_TIMEOUT_COUNT           ( 1000 ) /* TIMEOUT 발생 최대횟수  */

// 1012 -> 1011 마지막 byte가 왜곡되는 현상 발생
#define DOWN_DATA_SIZE              ( 1012 )
#define DOWN_DATA_SIZE_OLD          ( 1024 )


#define SUBTERM_USE_NEW_PROTOCOL 1 /* 하차기 신프로토콜 사용구분 Flag */
#define SUBTERM_USE_OLD_PROTOCOL 2 /* 하차기 구프로토콜 사용구분 Flag */

/*******************************************************************************
*  Definition of Structures                                                    *
*******************************************************************************/
typedef struct usr_packet
{
    byte bCmd;
    byte bDevNo;
    byte abData[DOWN_DATA_SIZE];
    int nDataSize;
    word wSeqNo;
} __attribute__((packed)) MAINSUB_COMM_USR_PACKET;


typedef struct rs485_file_header
{
    char achFileName[30];
    short sErr;
} __attribute__((packed)) RS485_FILE_HEADER_PKT;



typedef struct
{
    char achFileName[30];
} __attribute__((packed)) RS485_FILE_HEADER_PKT_OLD;


typedef struct {
    bool boolIsKeySetAdded;
    bool boolIsIDCenterAdded;
} __attribute__((packed)) MAINTERM_PSAM_ADD_RESULT;


typedef struct {
    bool aboolIsKeySetAdded[3];
    bool aboolIsIDCenterAdded[3];
} __attribute__((packed)) SUBTERM_PSAM_ADD_RESULT;

typedef struct
{
    byte  abFareInfoVer[14];                // 기초운임정보 버전
    byte  abPrepayIssuerInfoVer[14];        // 선불 발행사 정보 버전
    byte  abPostpayIssuerInfoVer[14];       // 후불 발행사 정보 버전
    byte  abDisExtraInfoVer[14];            // 할인할증정보 버전
    byte  abHolidayInfoVer[14];             // 휴일정보 버전
    byte  abXferApplyInfoVer[14];           // 환승적용 버전
    byte  abStationInfoVer[14];             // 정류장 정보 버전
    byte  abVehicleParmVer[14];             // 운행차량 파라미터 버전
    byte  abRouteParmVer[14];               // 버스단말기노선별파라미터 버전
}__attribute__((packed)) SUB_VERSION_INFO_LOAD; // 하차에서 보내줄 버전정보 파일세팅



/*******************************************************************************
*  Declaration of Global Variables inside this module                          *
*******************************************************************************/

extern MAINSUB_COMM_USR_PACKET stSendPkt; /* Send Data */
extern MAINSUB_COMM_USR_PACKET stRecvPkt; /* Receviced Data */

extern SUB_VERSION_INFO_LOAD stSubVerInfo;       /* SubTerminal Version Infomaiton */

/* Result of adding KeySet/IDCenter to mainterm PSAM */
//static MAINTERM_PSAM_ADD_RESULT stAddMainTermPSAM;

/* Result of adding KeySet/IDCenter to subterm PSAM */
extern SUBTERM_PSAM_ADD_RESULT  stSubTermPSAMAddResult;
extern bool boolIsRespVerNeedByOldProtocol;

/*******************************************************************************
*  Declaration of Function Prototype                                           *
*******************************************************************************/
void CommProc( void );
short CloseMainSubComm(void);

extern bool boolIsOldProtocol;  // old Protocol

/* old protocol에서 stSendPkt.abData대신 사용하기 위한 buffer */
extern byte abSendData[1024];  
/* old protocol에서 stRecvPkt.abData대신 사용하기 위한 buffer */
extern byte abRecvData[1048];  

extern bool boolIsMainSubCommFileDownIng;
/* 
 * 승하차 통신프로세스에서 현재 실행되고 있는 명령어 저장 변수
 */
extern byte bCurrCmd; 
/* 
 * 하차단말기 갯수 
 */
extern int nIndex;  


/* 승하차간에 공통으로 쓰이는 함수 */
short CloseMainSubComm( void );
short SendPkt( void );
short CreateNAK( int nDevNo );
short RecvPkt( int nTimeOut, int nDevNo );
short RecvFile( int nDevNo, char* pchFileName );
short SendFile( int nDevNo, char* pchFileName );
short CreateACK( int nDevNo );

#endif
