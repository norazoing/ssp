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
*  PROGRAM ID :       bys_type.h                                               *
*                                                                              *
*  DESCRIPTION:       Definition of Bus Common Function & Variables.           *
*  ENTRY POINT:       None                                                     *
*                                                                              *
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
*    DATE                SE NAME              DESCRIPTION                      *
* ---------- ---------------------------- -------------------------------------*
* 2005/08/09 Solution Team Gwan Yul Kim  Initial Release                       *
*                                                                              *
*******************************************************************************/

#ifndef _BUS_TYPE_H
#define _BUS_TYPE_H

/*******************************************************************************
*  Inclusion of System Header Files                                            *
*******************************************************************************/
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <arpa/inet.h>
#include <assert.h>
#include <ctype.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/io.h>
#include <malloc.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <string.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/param.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/vfs.h>
#include <sys/wait.h>
#include <termio.h>
#include <termios.h>
#include <unistd.h>

#ifndef __GCC
#include <asm/io.h>
#include <err.h>
#include <linux/if_arp.h>       /* For ARPHRD_ETHER */
#include <linux/ioctl.h>
#include <linux/rtc.h>
#include <linux/socket.h>       /* For AF_INET & struct sockaddr */
#include <linux/soundcard.h>
#include <linux/wireless.h>
#endif

/*******************************************************************************
*  Inclusion of Common Header Files                                            *
*******************************************************************************/
#include "../../common/type.h"
#include "../../common/define.h"
#include "../../common/errno.h"
#include "../../common/util.h"
#include "../../common/transp_util.h"
#include "../../common/rtc.h"
#include "../../common/comm.h"
#include "../../common/rf_comm.h"
#include "../../common/sam.h"
#include "../../common/card_if.h"
#include "../../common/bl.h" 
#include "../../common/pl.h"
#include "../../common/card_if_mif.h"
#include "../../common/card_if_sc.h"
#include "../../common/mif_key_mgt.h"
#include "../../common/sam_if_csam.h"
#include "../../common/sam_if_isam.h"
#include "../../common/sam_if_psam.h"
#include "../../common/sam_if_lsam.h"
#include "../../common/sam_if_olsam.h"
#include "../../common/sam_if_passsam.h"
#include "../../common/pay_lib_mif.h"
#include "../../common/pay_lib_sc.h"
#include "bus_define.h"
#include "bus_errno.h"
#include "log.h"							// 향후 삭제한다.

#define MAX_ERROR_LOG				10		// 오류LOG 최대 갯수
#define RECONCILE_MAX				1024

typedef struct {
	dword dwChipSerialNo;					// 칩 시리얼 번호
	byte bPLUserType;						// PL에 기록된 사용자유형
	byte bCardUserType;						// 카드에 기록된 사용자 유형
	byte bCardType;							// 카드유형
	byte abCardNo[20];						// 카드번호
	dword dwAliasNo;						// alias 번호
	bool boolIsTCard;						// T구선불카드 여부

	byte bSCAlgoriType;						// 신카드 알고리즘유형
	byte bSCTransType;						// 신카드 거래유형
	byte bSCTransKeyVer;					// 신카드 개별거래수집키버전
	byte bSCEpurseIssuerID;					// 신카드 전자화폐사ID
	byte abSCEpurseID[16];					// 신카드 전자지갑ID
	dword dwSCTransCnt;						// 신카드 거래건수

	byte abPSAMID[16];						// PSAM ID
	dword dwPSAMTransCnt;					// PSAM 거래카운터
	dword dwPSAMTotalTransCnt;				// PSAM 총액거래수집카운터
	word wPSAMIndvdTransCnt;				// PSAM 개별거래수집건수
	dword dwPSAMAccTransAmt;				// PSAM 누적거래총액
	byte abPSAMSign[4];						// PSAM 서명

	dword dwBalAfterCharge;					// 직전충전후카드잔액
	dword dwChargeTransCnt;					// 직전충전시카드거래건수
	dword dwChargeAmt;						// 직전충전금액
	byte abLSAMID[16];						// 직전충전기SAMID
	dword dwLSAMTransCnt;					// 직전충전기SAM거래일련번호
	byte bChargeTransType;					// 직전충전거래유형
	byte abMifPrepayChargeAppvNop[14];		// 구선불카드 직전충전승인번호
	byte abMifPrepayTCC[8];					// 구선불카드 TCC

	byte bEntExtType;						// 승하차유형 (ENT / EXT)
	byte bMifTermGroupCode;					// 구카드 단말기그룹코드 (구환승영역기록)
	time_t tMifEntExtDtime;					// 구카드 이용시간 (구환승영역기록)
	bool boolIsMultiEnt;					// 다인승여부
	bool boolIsAddEnt;						// 추가승차여부
	byte abUserType[3];						// 다인승사용자유형
	byte abUserCnt[3];						// 다인승사용자수
	byte abDisExtraTypeID[3][6];			// 다인승할인할증유형ID
	dword dwDist;							// 사용거리
	byte bNonXferCause;						// 비환승 사유
	bool boolIsChangeMonth;					// 후불카드이면서 월변경여부
	time_t tMifTourFirstUseDtime;			// 관광권카드 최초 사용 일시
	byte abMifTourExpiryDate[8];			// 관광권카드 만기일
	word wMifTourCardType;					// 관광권카드 유형
	byte bMifTourUseCnt;					// 관광권카드 이번 승/하차시 증가 횟수
	bool boolIsAdjustExtDtime;				// 하차시간보정여부

	COMMON_XFER_DATA stPrevXferInfo;		// 이전환승정보
	COMMON_XFER_DATA stNewXferInfo;			// 신규환승정보
	MIF_PREPAY_BLOCK18 stMifPrepayBlock18;	// 구선불카드 BLOCK18
	MIF_POSTPAY_OLD_XFER_DATA stOldXferInfo;// 구후불카드 구환승정보
	short sErrCode;							// 오류코드
	byte bWriteErrCnt;						// WRITE오류 발생 횟수
}__attribute__((packed)) TRANS_INFO;

typedef struct
{
	bool boolIsDeleted;
	TRANS_INFO stTransInfo;
}__attribute__((packed)) TRANS_ERR_LOG;

/*******************************************************************************
*  Declaration of Process Shared Memory                                        *
*******************************************************************************/
typedef struct
{
	pid_t  	nCommProcProcessID;		// Polling & DCS Comm Process ID
	pid_t	nCommPrinterProcessID;	// Printing Process ID
	pid_t	nCommDriverProcessID;	// Keypad Comm Process ID
	pid_t	nCommGPSProcessID;		// GPS Thread ID
	byte    abMainTermID[9];        // TerminalID( MainTerm ID)
	byte    abSubTermID[3][9];      // SubTerm ID
	byte    abDriverOperatorID[9];	// 운전자조작기ID
	byte    abMainVer[4];           // MainTerm Execution File Version
	byte    abSubVer[3][4];         // SubTerm Execution File Version
	byte    abMainVoiceVer[4];      // MainTerm Voice File Version
	byte    abSubVoiceVer[3][4];    // SubTerm Voice File Version
	byte    abMainPSAMID[16];		// MainTerm PSAMID
	byte    abSubPSAMID[3][16];		// SubTerm PSAMID
	byte    bMainPSAMPort;     		// MainTerm PSAM PORT #
	byte    abSubPSAMPort[3];   	// SubTerm PSAMID Comm PORT
	byte    abMainISAMID[7];        // MainTerm ISAMID
	byte    abSubISAMID[3][7];      // SubTerm ISAMID
	byte    abMainCSAMID[8];        // MainTerm CSAMID
	byte    abSubCSAMID[3][8];      // SubTerm CSAMID
	byte	bMainPSAMVer;			// 승차단말기 PSAM 버전
	byte	abSubPSAMVer[3];		// 하차단말기 PSAM 버전
	byte	abKpdVer[4];			// Keypad Execution File Version

	bool	boolIsReadyToDrive;		// Ready to StartAlarm
	bool    boolIsKpdLock;          // Execution /Voice File Transfer Status
	byte    bVoiceApplyStatus;      // Voice File Apply Status

	bool    boolIsBLPLMergeNow;			// BL/PL 머지중 여부
	bool    boolIsBLMergeNow;			// BL 머지중 여부
	bool    boolIsMifPrepayPLMergeNow;	// 구선불PL 머지중 여부
	bool    boolIsPostpayPLMergeNow;	// 후불PL 머지중 여부
	bool    boolIsSCPrepayPLMergeNow;	// 신선불PL 머지중 여부

	byte    bCommCmd;               // Command Between Process
	char    abCommData[40];         // Command Data Between Process
	byte	bCmdDataSize;			// Command Data Size
	byte    bPollingResCnt;         // Polling Response Status
	bool    boolIsDriveNow;         // Driving Status
	byte    abTransFileName[19];    // TR Data FileName
	byte    abEndedTransFileName[15];
	time_t  tTermTime;              // Term Time
	byte    abNowStationID[7];      // Station ID
	byte	abNowStationName[16];	// Station Name
	bool    boolIsCardProc;         // Card Process Status
	bool	boolIsTransFileUpdateEnable;	// TR File Write Enable
	
	bool	boolIsKpdImgRecv;		// 신규 운전자조작기 파일 수신
	byte	bCardUserType;			// 카드구분('1':일반 '2':청소년 '3':어린이)
	byte	abTotalFare[6];			// 총요금
	bool	boolIsXfer;				// 환승여부
	bool	boolIsGpsValid;			// GPS DATA의 유효성여부
	bool	boolIsSetStationfromKpd;// 폴링용 0:실제모드, 1:운전자조작기변경모드
	byte	abKeyStationID[7];		//
	byte	abKeyStationName[16];	//
	byte	abLanCardStrength[3];
	byte	abLanCardQuality[3];
	dword	dwDriveChangeCnt;

	byte gbGPSStatusFlag;			// 거래내역에 기록되는 GPS 상태 플래그

	byte bTransErrLogPtr;			// 거래미완료로그 포인터
	TRANS_ERR_LOG astTransErrLog[MAX_ERROR_LOG];
									// 거래미완료로그
} __attribute__((packed)) PROC_SHARED_INFO;

/*******************************************************************************
*  Declaration of Terminal Info                                                *
*******************************************************************************/
typedef struct
{
    byte    abVoiceVer[4];      // MainTerm Voice File Version
    byte    abPSAMID[16];		// MainTerm PSAMID
	byte    bPSAMPort;     		// MainTerm PSAM PORT #
	byte    abISAMID[7];        // MainTerm ISAMID
	byte    abCSAMID[8];        // MainTerm CSAMID
} __attribute__((packed)) MYTERM_INFO;


/*******************************************************************************
*  Declaration of Header Files                                                 *
*******************************************************************************/
typedef struct
{
	long lMsgType;
	char achMsgData[MSGQUEUE_MAX_DATA + 1];
} __attribute__((packed)) MSGQUEUE_DATA;

/*******************************************************************************
*  Declaration of Rebooting qualification                                                *
*******************************************************************************/
typedef struct
{
	bool boolIsCriteriaRecv;	// 신규운영정보 수신
	bool boolIsImgRecv;			// 신규 버스 IMAGE수신
	bool boolIsVoiceFileRecv;	// 신규 음성파일수신
	bool boolIsKpdImgRecv;		// 신규 운전자조작기 파일 수신
	bool boolIsReset;			// 단말기 RESET 필요
}__attribute__((packed)) NEW_IMGNVOICE_RECV;

/*******************************************************************************
*  Declaration of MULTI_ENT_INFO                        					   *
*******************************************************************************/
typedef struct
{
	bool boolIsMultiEnt;				// 다인승입력여부
	time_t tInputDatetime;				// 다인승입력일시
	bool boolIsAddEnt;					// 추가승차여부
	byte abUserType[3];					// 사용자 유형
	byte abUserCnt[3];					// 사용자 명수
} __attribute__((packed)) MULTI_ENT_INFO;

/*******************************************************************************
* 시티투어버스 승차권입력정보                                                  *
*******************************************************************************/
typedef struct
{
	bool boolIsTicketInput;				// 승차권정보입력여부
	time_t tInputDtime;					// 승차권정보입력일시
	bool boolIsOneTimeTicket;			// 1회권여부 (TRUE:1회권, FALSE:종일권)
	byte abUserType[2];					// 사용자 유형 (어린이 미존재)
	byte abUserCnt[2];					// 사용자 명수 (어린이 미존재)
	dword dwTicketAmt;					// 계산된 승차권 금액
} __attribute__((packed)) CITY_TOUR_BUS_TICKET_INFO;


/*******************************************************************************
*  structure of reconcileinfo.dat                                              *
*******************************************************************************/
typedef struct
{
	char 	achFileName[30];	// 집계PC로 전송할 파일명
	byte 	bSendStatus;		// 전송할 파일상태
	int		nSendSeqNo;			// 전송횟수
	time_t 	tWriteDtime;		// 파일목록Write시간
} __attribute__((packed)) RECONCILE_DATA;


/*******************************************************************************
*  Declaration of MSGQUEUE_RECEIPT_DATA                                        *
*******************************************************************************/
typedef struct
{
	byte abBizNo[10];
	byte abDateTime[14];
	byte abVehicleNo[20];
	byte abTranspMethodCodeName[8];
	byte abUserTypeName[12];
	byte achBusStationName[16] ;
	byte abFare[6] ;
} __attribute__((packed)) MSGQUEUE_RECEIPT_DATA;

/*******************************************************************************
* 구선불발행사정보 - ap_inf.dat (B0670)                                        *
*******************************************************************************/
typedef struct
{
	time_t tApplyDtime;					// 적용일시
	byte bApplySeqNo;					// 적용일련번호
	dword dwRecordCnt;					// 레코드 건수
} __attribute__((packed)) PREPAY_ISSUER_INFO_HEADER;

typedef struct
{
	byte abPrepayIssuerID[7];			// 선불 발행사 ID
	byte bAssocCode;					// 단말기 소속 조합 구분코드
	byte bXferDisYoungCardApply;		// 환승할인/학생카드 적용유무
} __attribute__((packed)) PREPAY_ISSUER_INFO;

/*******************************************************************************
* 후불발행사정보 - dp_inf.dat (B0690)                                          *
*******************************************************************************/
typedef struct
{
	time_t tApplyDtime;					// 적용일시
	word wRecordCnt;					// 레코드 건수
} __attribute__((packed)) POSTPAY_ISSUER_INFO_HEADER;

typedef struct
{
	byte abPrefixNo[6];					// Prefix 번호
	word wCompCode;						// 압축코드
} __attribute__((packed)) POSTPAY_ISSUER_INFO;

/*******************************************************************************
* 후불발행사유효기간정보 - cp_inf.dat                                          *
*******************************************************************************/
typedef struct
{
	time_t tApplyDtime;					// 적용일시
	word wRecordCnt;					// 레코드 건수
} __attribute__((packed)) ISSUER_VALID_PERIOD_INFO_HEADER;

typedef struct
{
	byte abPrefixNo[6];					// Prefix 번호
	byte abIssuerID[7];					// 발행사ID
	byte abExpiryDate[6];				// 카드유효기간 (YYYYMM)
} __attribute__((packed)) ISSUER_VALID_PERIOD_INFO;

/*******************************************************************************
* 환승적용정보 - tr_inf.dat (B1700)                                            *
*******************************************************************************/
typedef struct
{
	dword dwRecordCnt;					// 레코드 건수
} __attribute__((packed)) XFER_APPLY_INFO_HEADER;

typedef struct
{
	time_t tApplyDtime;					// 적용일시
	word wXferApplyStartTime;			// 환승적용시작시간
	word wXferApplyEndTime;				// 환승적용종료시간
	byte bHolidayClassCode;				// 휴일구분코드
	dword dwXferEnableTime;				// 환승가능시간
	word wXferEnableCnt;				// 환승가능횟수
} __attribute__((packed)) XFER_APPLY_INFO;

/*******************************************************************************
* 할인할증정보 - de_inf.dat (B0640)                                            *
*******************************************************************************/
typedef struct
{
	time_t tApplyDtime;					// 적용일시
	byte bApplySeqNo;					// 적용일련번호
	dword dwRecordCnt;					// 레코드 건수
} __attribute__((packed)) DIS_EXTRA_INFO_HEADER;

typedef struct
{
	byte bTranspCardClassCode;			// 교통카드구분코드
	byte abDisExtraTypeID[6];			// 할인할증유형ID
	byte bDisExtraApplyCode;			// 할인할증적용기준코드
	float fDisExtraRate;				// 할인/할증률
	int nDisExtraAmt;					// 할인/할증 금액
} __attribute__((packed)) DIS_EXTRA_INFO;

/*******************************************************************************
* 휴일정보 - ho_inf.dat (B0630)                                                *
*******************************************************************************/
typedef struct
{
	time_t tApplyDtime;					// 적용일시
	dword dwRecordCnt;					// 레코드 건수
} __attribute__((packed)) HOLIDAY_INFO_HEADER;

typedef struct
{
	byte abHolidayDate[8];				// 휴일일자ID
	byte bHolidayClassCode;				// 휴일구분코드
} __attribute__((packed)) HOLIDAY_INFO;

/*******************************************************************************
* 신요금정보 - n_far.dat (B0070)                                               *
*******************************************************************************/
typedef struct
{
	time_t tApplyDatetime;				// 적용일시
	byte bApplySeqNo;					// 적용일련번호
	word wTranspMethodCode;				// 교통수단코드
	dword dwSingleFare;					// 단일요금 - 미사용
	dword dwBaseFare;					// 기본운임
	dword dwBaseDist;					// 기본거리
	dword dwAddedFare;					// 부가운임
	dword dwAddedDist;					// 부가거리
	dword dwOuterCityAddedDist;			// 시외부가거리 - 미사용
	dword dwOuterCityAddedDistFare;		// 시외부가거리운임 - 미사용
	dword dwAdultCashEntFare;			// 성인 현금승차요금
	dword dwChildCashEntFare;			// 어린이 현금승차요금
	dword dwYoungCashEntFare;			// 청소년 현금승차요금
	dword dwPermitErrDist;				// 허용오차거리 - 미사용
} __attribute__((packed)) NEW_FARE_INFO;

/*******************************************************************************
*  structure of STATION_INFO - c_st_inf.dat                              					   *
*******************************************************************************/
typedef struct
{
	time_t tApplyDtime;					// 적용일시
	byte bApplySeqNo;					// 적용일련번호
	byte abRouteID[8];					// 버스노선 ID
	byte abTranspMethodName[16];		// 버스노선명
	dword dwRecordCnt;					// 레코드 건수
} __attribute__((packed)) STATION_INFO_HEADER;

typedef struct
{
	byte abStationID[7];				// 버스정류장 ID
	byte bCityInOutClassCode;			// 시계내외구분 코드
	word wStationOrder;					// 정류장순서
	byte abStationName[16];				// 버스정류장명
	double dStationLongitude;			// 버스정류장 경도
	double dStationLatitude;			// 버스정류장 위도
	word wOffset;						// offset
	dword dwDistFromFirstStation;		// 첫정류장에서의 거리
	word wStationApproachAngle;			// 정류장 진입각
} __attribute__((packed)) STATION_INFO;


/*******************************************************************************
*  structure of VEHICLE_PARM - op_par.dat                 					   *
*******************************************************************************/
typedef struct
{
	byte	abApplyDtime[14];      			// 적용 일시
	byte    abTranspBizrID[9];            	// 교통사업자 ID
	byte    abBusBizOfficeID[2];          	// 버스영업소 ID
	byte    abVehicleID[9];              	// 차량 ID
	byte    abVehicleNo[20];             	// 차량번호
	byte    abRouteID[8];                	// 노선 ID
	byte    abDriverID[6];               	// 운전자 ID
	byte    abTranspMethodCode[3];       	// 교통수단코드 - byte 배열
	word	wTranspMethodCode;				// 교통수단코드 - word
	byte    abBizNo[10];                  	// 사업자번호
	byte    abTranspBizrNm[20];           	// 교통사업자명
	byte    abAddr[103];                  	// 주소
	byte    abRepreNm[10];                	// 대표자명
} __attribute__((packed)) VEHICLE_PARM;


typedef struct
{
	byte	abApplyDtime[14];				// 적용일시
	byte	abRouteID[8];					// 노선ID
	byte	abTranspMethodCode[3];			// 교통수단코드 - byte 배열
	word	wTranspMethodCode;				// 교통수단코드 - word
	byte	abXferDisApplyFreq[2];			// 환승할인적용횟수 (미사용)
	byte	abXferApplyTime[3];				// 환승적용시간 (미사용)
	byte	abTermGroupCode[2];				// 단말기그룹코드 (미사용)
	byte	abSubTermCommIntv[3];			// 하차단말기와통신간격 (미사용)
	byte	abCardProcTimeIntv[3];			// 카드처리시간간격 (미사용)
	byte	abUpdateBLValidPeriod[2];		// 변동BL유효기간 (미사용)
	byte	abUpdatePLValidPeriod[2];		// 변동PL유효기간 (미사용)
	byte	abUpdateAIValidPeriod[2];		// 변동AI유효기간 (미사용)
	byte	bDriverCardUseYN;				// 운전자카드사용유무 (미사용)
	byte	abChargeInfoSendFreq[2];		// 충전정보전송횟수 (미사용)
	byte	bRunKindCode;					// 운행종류코드 - 운전자조작기전송
	byte	bGyeonggiIncheonRangeFareInputWay;
											// 경기인천구간요금입력방식 - 운전자조작기전송
	byte	abLEAPPasswd[20];				// LEAP 암호 (미사용)
	byte	bECardUseYN;					// 후불ecard 사용유무 (미사용)

} __attribute__((packed)) ROUTE_PARM;

/*******************************************************************************
*  Declaration of Extern Global Variables                                      *
*******************************************************************************/
extern bool gboolIsMainTerm;			// Div of MainTerm & SubTerm
extern byte gabStationName[17];			// StationName
extern byte gbSubTermNo;				// # of SubTerm
//bool gboolIsDriveNow;            		// 공유메모리에 존재함.
extern bool gboolIsWhileNow;            // WhileLoop Status
extern bool boolIsAllLogMode;			// Whether LogMode is AllLOG Mode or Not
extern byte gbSubTermCnt;				// 하차단말기갯수

// GPS Thread와 동시에 사용하는변수
extern bool boolIsEndStation;			// Bus가 종점에 있는지 여부
extern char gbchStationID[8];

// DCS Thread와 동시에 사용하는변수
extern bool boolIsDCSThreadComplete;    /* DCS thread complete flag */
extern bool boolIsDCSThreadStartEnable; /* DCS thread start enable/disable */
extern int nDCSCommSuccCnt;				// DCS comm success count
extern time_t gtDCSCommStart;		// DCS comm 종료 시간 

extern PROC_SHARED_INFO *gpstSharedInfo;	// Shared Memory Pointer
extern MYTERM_INFO		gstMyTermInfo;

extern byte gabKernelVer[3];

// Decla. of File Desc.
extern int gfdRC531Device;				// RC531 Device File Descriptor

extern MULTI_ENT_INFO gstMultiEntInfo;	// 다인승정보
extern CITY_TOUR_BUS_TICKET_INFO gstCityTourBusTicketInfo;
										// 시티투어버스 승차권입력정보

// Message Queue for Printing
extern int gnMsgQueue;

extern  byte 	gabLEAPPasswd[21];
extern	byte	gabDCSIPAddr[16];			// DCS server IP

// 요금처리에서 사용되는 기본정보
extern VEHICLE_PARM 					gstVehicleParm;
extern ROUTE_PARM						gstRouteParm;
extern PREPAY_ISSUER_INFO_HEADER 		gstPrepayIssuerInfoHeader;
extern PREPAY_ISSUER_INFO 				*gpstPrepayIssuerInfo;
extern POSTPAY_ISSUER_INFO_HEADER 		gstPostpayIssuerInfoHeader;
extern POSTPAY_ISSUER_INFO 				*gpstPostpayIssuerInfo;
extern ISSUER_VALID_PERIOD_INFO_HEADER
										gstIssuerValidPeriodInfoHeader;
extern ISSUER_VALID_PERIOD_INFO 		*gpstIssuerValidPeriodInfo;
extern XFER_APPLY_INFO_HEADER 			gstXferApplyInfoHeader;
extern XFER_APPLY_INFO 					*gpstXferApplyInfo;
extern DIS_EXTRA_INFO_HEADER 			gstDisExtraInfoHeader;
extern DIS_EXTRA_INFO 					*gpstDisExtraInfo;
extern HOLIDAY_INFO_HEADER 				gstHolidayInfoHeader;
extern HOLIDAY_INFO 					*gpstHolidayInfo;
extern NEW_FARE_INFO 					gstNewFareInfo;
extern STATION_INFO_HEADER				gstStationInfoHeader;
extern STATION_INFO 					*gpstStationInfo;

extern word gwGPSRecvRate;				// GPS 수신율
extern word gwDriveCnt;					// 운행횟수
extern time_t gtDriveStartDtime;		// 운행시작시간
extern dword gdwDistInDrive;			// 운행중 이동한 거리
extern bool gboolIsRunningGPSThread;	// GPS쓰레드 실행 여부 (GPS로그 작성 관련)
extern int gnGPSStatus;					// GPS Status
extern bool gboolIsEndStation;
extern byte gabEndStationID[7];			// 종점정류장ID
extern dword gdwDelayTimeatEndStation;	// 종점에 도착한뒤 delay time
extern byte gabGPSStationID[7];			// GPS에서만 업데이트하는 정류장ID
extern bool boolIsRegistMainTermPSAM;   // thread exit enabl for DCS Comm
extern word gwRetagCardCnt;				// '카드를 다시 대주세요' 음성 횟수

#endif

