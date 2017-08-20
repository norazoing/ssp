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
*  PROGRAM ID :       bys_define.h                                             *
*                                                                              *
*  DESCRIPTION:       Definition of Bus defines and macros 			           *
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

#ifndef _BUS_DEFINE_H
#define _BUS_DEFINE_H

/*******************************************************************************
*  Inclusion of System Header Files                                            *
*******************************************************************************/
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/shm.h>

#ifndef __GCC
#include <wait.h>
#endif

/*******************************************************************************
*  Definition of Image Version                                                 *
*******************************************************************************/
#define MAIN_RELEASE_VER				"0409"

/*******************************************************************************
*  Macro Definition of File Names                                              *
*******************************************************************************/
#define CONTROL_TRANS_FILE				"control.trn"		// 운행정보기록파일
#define CONTROL_TRANS_BACKUP_FILE		"control.bak"		// 운행정보기록백업파일

#define MAIN_TERM_BACKUP_TRANS_FILE		"aboard_term_td.bak"// 승차백업거래내역파일
#define SUB_TERM_BACKUP_TRANS_FILE		"alight_term_td.bak"// 하차백업거래내역파일
#define SUB_TERM_TRANS_FILE				"alight_term_td.tmp"// 하차거래내역파일
#define TEMP_REMAKE_FILE				"temptd.dat"		// 임시대사파일

#define TERM_AGGR_SEQ_NO_FILE			"seqth.tmp"			// 단말기사용집계일련번호파일
#define BACKUP_TRANS_SEQ_NO_FILE		"backseq.tmp"		// 백업거래내역일련번호파일
#define CASH_GETON_SEQ_NO_FILE			"ticketseq.tmp"		// 일회권ID일련번호파일

#define PG_LOADER_VER_FILE				"pgver.dat"			// PG로더 버전 파일
#define VOICE0_FILE						"c_v0.dat"			// 음성파일
#define VOICEAPPLY_FLAGFILE				"voiceapply.dat"	// 음성적용파일
#define VOICEAPPLY_VERSION				"voicever.dat"		// 음성적용버전파일
#define VEHICLE_PARM_FILE				"c_op_par.dat"
#define ROUTE_PARM_FILE					"c_li_par.dat"
#define PREPAY_ISSUER_INFO_FILE			"c_ap_inf.dat"
#define POSTPAY_ISSUER_INFO_FILE		"c_dp_inf.dat"

#define BUS_STATION_INFO_FILE			"c_st_inf.dat"
#define XFER_APPLY_INFO_FILE			"c_tr_inf.dat"
#define DIS_EXTRA_INFO_FILE				"c_de_inf.dat"
#define HOLIDAY_INFO_FILE				"c_ho_inf.dat"
#define XFER_TERM_INFO_FILE				"c_tc_inf.dat"
#define NEW_FARE_INFO_FILE				"c_n_far.dat"

#define MASTER_BL_FILE					"c_fi_bl.dat"
#define MASTER_PREPAY_PL_FILE			"c_fa_pl.dat"
#define MASTER_POSTPAY_PL_FILE			"c_fd_pl.dat"
#define MASTER_AI_FILE					"c_fi_ai.dat"
#define CHANGE_BL_FILE					"c_ch_bl.dat"
#define UPDATE_PL_FILE					"c_ch_pl.dat"
#define UPDATE_AI_FILE					"c_ch_ai.dat"

#define DOWN_ING_MASTER_BL_FILE			"tmp_c_fi_bl.dat"
#define DOWN_ING_MASTER_PREPAY_PL_FILE	"tmp_c_fa_pl.dat"
#define DOWN_ING_MASTER_POSTPAY_PL_FILE	"tmp_c_fd_pl.dat"
#define DOWN_ING_MASTER_AI_FILE			"tmp_c_fi_ai.dat"

#define DOWN_ING_UPDATE_BL_FILE			"tmp_c_ch_bl.dat"
#define DOWN_ING_UPDATE_PL_FILE			"tmp_c_ch_pl.dat"
#define DOWN_ING_UPDATE_AI_FILE			"tmp_c_ch_ai.dat"

#define BUS_MAIN_IMAGE_FILE				"c_en_pro.dat"
#define BUS_SUB_IMAGE_FILE				"c_ex_pro.dat"
#define KPD_IMAGE_FILE					"c_dr_pro.dat"
#define KPDAPPLY_FLAGFILE				"driverdn.cfg"

#define SUB_TRANS_SEND_SUCC				"check.trn"
#define SUBTERM_ID_FILENAME				"subid.dat"
#define PSAMID_FILE						"simid.flg"
#define STATUS_FLAG_FILE				"statusFlag.dat"

#define COMMDCS_SUCCDATE_FILE			"connSucc.dat"

#define RECONCILE_FILE					"reconcileinfo.dat"
#define DOWN_FILE						"downloadinfo.dat"
#define RELAY_DOWN_INFO_FILE			"downfilelink.dat"
#define INSTALL_INFO_FILE				"install.dat"
#define BLPL_CRC_FILE        			"blpl_crc.dat"
#define SETUP_FILE						"setup.dat"
#define SETUP_BACKUP_FILE				"setup.backup"
#define TC_LEAP_FILE					"tc_leap.dat"
#define TC_LEAP_BACKUP_FILE				"tc_leap.backup"

#define GPS_INFO_FILE					"gps_info.dat"
#define GPS_LOG_FILE					"simxinfo.dat"
#define GPS_LOG_FILE2					"simxlog.dat"

#define ERR_LOG_MODE_FLAG_FILE			"c_ho_bl.dat"

#define KERNEL_VERSION_FILE				"/proc/version"
#define RAMDISK_VERSION_FILE			"/root/config.ini"

#define FLASH_DATA_OVER_ONE				"diskone.dat"
#define FLASH_DATA_OVER_TWO				"disktwo.dat"

#define BUS_BACKUP_DIR					"/mnt/mtd7"
#define BUS_EXECUTE_DIR					"/mnt/mtd8/bus"
#define BUS_EXECUTE_FILE				"bus100"
#define BUS_EXECUTE_BACKUP_FILE			"bus200"
#define BUS_EXECUTE_BEFORE_BACKUP_FILE	"bus300"

#define SEMA_KEY_TRANS					1000
#define SEMA_KEY_ERR_LOG				1001
#define SEMA_KEY_LOG					1002
#define SEMA_KEY_SHARED_CMD				1003

#define SUBTERM_TRANS_RECORD_SIZE		202
#define MSGQUEUE_MAX_DATA				1024		// Max Data Size of MsgQueue

#define SHARED_MEMORY_KEY				((key_t)5678)
#define MESSAGE_QUEUE_PRINTER_KEY		(900000 + 1)

/*******************************************************************************
*  Macro of Cmd between Processes											   *
*******************************************************************************/
#define CMD_SETUP						'A'	// MainProc → CommProc
#define CMD_RESETUP						'R'	// MainProc → CommProc
#define CMD_SUBID_SET					'I'	// KeypadProc → CommProc
#define CMD_NEW_CONF_IMG				'L'	// CommProc → MainProc/ KeypadProc
#define CMD_MULTI_GETON					'M'	// KeypadProc → MainProc
#define CMD_CANCEL_MULTI_GETON			'C'	// KeypadProc → MainProc
#define CMD_START_STOP					'D'	// KeypadProc → MainProc
#define CMD_STATION_CORRECT				'S'	// KeypadProc → MainProc
#define CMD_PRINT						'Q'	// KeypadProc → MainProc → PrinterProc
#define CMD_BL_CHECK					'B'	// MainProc → CommProc
#define CMD_PL_CHECK					'U'	// MainProc → CommProc
#define CMD_PARMS_RESET					'E'	// KeypadProc → CommProc
#define CMD_CONFIRM_CANCEL_STATION_CORRECT \
										'O'	// KeypadProc → MainProc
#define CMD_KEYSET_IDCENTER_WRITE		'K'	// CommProc → MainProc
#define CMD_INPUT_CITY_TOUR_BUS_TICKET	'T' // KeypadProc → MainProc
#define CMD_CANCEL_CITY_TOUR_BUS_TICKET	'V'	// KeypadProc → MainProc

/*******************************************************************************
*  Declaration of Date/ Version Size                                           *
*******************************************************************************/
#define FILE_DATE_SIZE					14
#define FILE_VERSION_SIZE				4

/*******************************************************************************
*  Declaration of Shared Memory Command Meaning                                *
*******************************************************************************/
#define CMD_REQUEST						'1'	//
#define CMD_SUCCESS_RES					'0'	//
#define CMD_FAIL_RES					'9'	//

/*******************************************************************************
*  MessageQueue Permission                                                     *
*******************************************************************************/
#define IPC_PERM ( 0660 )

/*******************************************************************************
*  Reconcile Send status                                                       *
*******************************************************************************/
#define RECONCILE_SEND_WAIT			'0'   	// TR파일 송신대기
#define RECONCILE_SEND_JUST			'1'   	//
#define RECONCILE_SEND_COMP			'2'   	// 송신완료 reconcile 대기
#define RECONCILE_RESEND_REQ		'3'   	// 재송신요청
#define RECONCILE_RESEND_JUST		'4'   	//
#define RECONCILE_RESEND_COMP		'5'   	// 재송신완료 reconcile 대기
#define RECONCILE_SEND_SETUP		'7'   	// 셋업파일 송신대기
#define RECONCILE_SEND_ERR_LOG		'8'   	// 에러로그 송신대기 (이벤트)
#define RECONCILE_SEND_VERSION		'9'   	// 버전파일 송신대기
#define RECONCILE_SEND_GPS		    'a'   	// GPS파일 송신대기
#define RECONCILE_SEND_GPSLOG	    'b'   	// GPSLOG 송신대기
#define RECONCILE_SEND_GPSLOG2	    'c'   	// GPSLOG2 송신대기
#define RECONCILE_SEND_TERM_LOG		'd'		// 단말기 로그
#define RECONCILE_SEND_STATUS_LOG	'e'		// 상태 로그


/*******************************************************************************
*  Reconcile result status                                                 *
*******************************************************************************/
#define RECONCILE_RESULT_DEL		'0'		// 삭제
#define RECONCILE_RESULT_RESEND		'1'		// 재송신
#define RECONCILE_RESULT_NONE		'2'		// 대기
#define RECONCILE_RESULT_ERR_DEL	'3'		// 에러로 인한 삭제

/*******************************************************************************
*  download status                                                 *
*******************************************************************************/
#define UNDER_DOWN					1		// 다운로드 중
#define DOWN_COMPL					2		// 다운로드 완료

/*******************************************************************************
*  Printer Message Queue Type                                                  *
*******************************************************************************/
#define PRINT_RECEIPT				1
#define PRINT_TERM_INFO				2

/*******************************************************************************
*  GPS SATELLITE DATA VALID                                                    *
*******************************************************************************/
#define GPS_DATA_VALID				'A'

#define PSAMID_SIZE					16

#define CURR						0
#define NEXT						1
#define MAX_FILE_READ_SIZE_OLD		1024
#define MAX_PKT_SIZE 				1024
#define MAX_PKT_SIZE_OLD			1034
#define MULTI_GET_ON_TIME			10		// 다인승입력처리시간

#define SYSTEM_RESET_REQ			(short)0x0001
#define OPENMODE 	S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH

/*******************************************************************************
*  Declaration of Comm Macros                                                  *
*******************************************************************************/
#define SOH								0x01
#define STX								0x02
#define	ETX								0x03
#define ACK								0x06
#define NAK								0x15

/*******************************************************************************
*  Declaration of Keypad Cmd Macros                                            *
*******************************************************************************/
#define KPDCMD_SET_TERMID_LEAPPWD		0x11
#define KPDCMD_SETUP					0x12
#define KPDCMD_SET_TIME					0x13
#define KPDCMD_START_STOP				0x14
#define KPDCMD_EXTRA					0x15
#define KPDCMD_CANCEL_MULTI_GETON		0x17
#define KPDCMD_BL_CHECK					0x18
#define KPDCMD_PL_CHECK					0x19
#define KPDCMD_MULTI_GETON				0x20
#define KPDCMD_STATION_CORRECT			0x21
#define KPDCMD_CASHENT_RECEIPT_PRINT	0x22
#define KPDCMD_SUBID_SET				0x23
#define KPDCMD_SERVERIP_SET				0x24
#define KPDCMD_TERMINFO_PRINT			0x25
#define KPDCMD_CONFIRM_STATION_CORRECT	0x26
#define KPDCMD_CANCEL_STATION_CORRECT	0x27

#define KPDCMD_SET_TERMID_LEAPPWD_0118	0x31
#define KPDCMD_MAINID_SET				0x32
#define KPDCMD_CITY_TOUR_BUS_TICKET		0x33

#define KPDCMD_RECV(bCmd) \
	((KPDCMD_SET_TERMID_LEAPPWD <= bCmd) && \
	(bCmd <=KPDCMD_CITY_TOUR_BUS_TICKET) )	
#define IS_KPDIMAGE_SEND(bCmd, bSendReq) \
	((bCmd == CMD_NEW_CONF_IMG) && \
	 (bSendReq == CMD_REQUEST) )

/*******************************************************************************
*  승차기에서 하차기ID와 수량을 기록하기위한 파일크기                          *
*******************************************************************************/
#define SERVER_IP_SIZE				12
#define MAINTERM_ID_SIZE			9
#define SUBTERM_ID_SIZE				9
#define SUBTERM_ID_COUNT_SIZE		(1 + SUBTERM_ID_SIZE*3)
#define END_MSG_COND( bTermCnt, nLoopCnt ) \
	(((bTermCnt <= 1)&&(nLoopCnt >= 320)) || ((bTermCnt > 1)&&(nLoopCnt >= 55)))

#define RECEIPT_ADULTCOUNT_SIZE		3
#define RECEIPT_YOUNGCOUNT_SIZE		3
#define RECEIPT_CHILDCOUNT_SIZE		3
#define RECEIPT_ALLCOUNT_SIZE \
	(RECEIPT_ADULTCOUNT_SIZE + RECEIPT_YOUNGCOUNT_SIZE + RECEIPT_CHILDCOUNT_SIZE)
#define STATION_ID_CORRECT_SIZE		1
#define CANCEL_MULTI_GETON_SIZE		1
#define DRIVER_ID_SIZE				7
				 
/*******************************************************************************
*  DEVICE CLASS                                          					   *
*******************************************************************************/
#define DEVICE_CLASS_MAIN					"14"		// 승차단말기
#define DEVICE_CLASS_SUB					"15"		// 하차단말기

#define SPACE   0x20	// ' '
#define ZERO    0x30	// '0'
#define CR		0x0D
#define LF		0x0A

#define FND_VOICEFILE_WRITE_TO_FLASH		"000999"	// FLASH에 음성파일 기록함

#define FND_ERR_MSG_WRITE_MAIN_TRANS_DATA	"999111"	// 승차단말기 거래내역기록 오류
#define FND_ERR_MSG_WRITE_SUB_TRANS_DATA	"119119"	// 하차단말기 거래내역기록 오류

#define FND_ERR_CRITERIA_INFOFILE_NO_EXIST	"999000"	// 운영정보파일이 한가지라도 미존재
#define FND_ERR_GET_PSAM_ID					"111111"	// PSAM ID획득 실패
#define FND_ERR_RF_INIT						"222222"	// RF초기화 에러발생
#define FND_ERR_ISAM						"333333"	// ISAM 에러발생
#define FND_ERR_PSAM						"444444"	// PSAM 에러발생
#define FND_ERR_RECEIVE_LATEST_INFO			"777777"	// 최신운행정보수신 요청		
#define FND_ERR_SYSTEM_MEMORY				"888888"	// 공유메모리 초기화에러
#define FND_ERR_SEMA_CREATE					"999222"	// SEMAPHORE Create Failed
#define FND_ERR_MAIN_SUB_COMM_PORT			"666666"	// 통신PORT 초기화 에러발생

#define FND_ERR_MAIN_SUB_COMM_POLLING		"999999"	// 승차 폴링없음 오류
#define FND_ERR_SUBTERM_TRN_EXIST			"114114"	// 하차단말기에 미전송 데이타존재

#define DOWNLOAD_START_FROM_DCS				"111111"	// 집계로부터 다운로드 시작
#define DOWNLOAD_SUBTERM_PARM				"122222"	// 하차단말기로 운영정보파일전송
#define FND_DOWNLOAD_START_SUBTERM_VOICE	"133333"	// 하차단말기로 음성파일전송
#define FND_DOWNLOAD_DRIVER_IMG				"911111"	// 운전자조작기 반영
#define FND_DOWNLOAD_SUBTERM_IMG			"922222"	// 하차단말기로 F/W파일전송
#define FND_DOWNLOAD_END_SUBTERM_VOICE		"933333"	// 하차단말기로 음성파일전송완료

#define FND_INIT_MSG 						"000000"
#define FND_READY_MSG						"0"

#define MAX_RETRY_COUNT         			3
#define MIN_RETRY_COUNT         			1

#define EEPROM_DEVICE						"/dev/i2c"
#define DIPSWTICH_DEVICE					"/dev/dipsw"
#define BUZZ_DEVICE							"/dev/buz"
#define FND_DEVICE							"/dev/seg"
#define PRINTER_DEVICE						"/dev/ttyE3"
#define RC531_DEVICE						"/dev/rc531"
#define VOICE_DEVICE						"/dev/voice"

#define LANCARD_SIGNAL_FILE					"/proc/driver/aironet/eth0/Status"

#define TEST_PRINT_CARD_PROC_INFO				// 이 매크로가 define되면 운영모드를 위한 이전환승정보/신규환승정보가 출력됨
//#define TEST_PRINT_CARD_PROC_TIME				// 이 매크로가 define되면 카드판별시간/카드처리시간이 출력됨
//#define TEST_NOT_LOG							// 이 매크로가 define되면 log.c를 이용한 로그가 남지 않음
//#define TEST_CARDTYPE_DEPOSIT				"10008001000114115409"
												// 이 매크로가 define되면 해당 카드번호를 가진 구선불카드를 마치 예치금카드처럼 처리함
//#define TEST_NOT_CHECK_BLPL					// 이 매크로가 define되면 BL/PL체크를 수행하지 않음
//#define TEST_NOT_CHECK_ISSUER					// 이 매크로가 define되면 발행사체크를 수행하지 않음
//#define TEST_NOT_CHECK_EXPIRY_DATE			// 이 매크로가 define되면 후불카드의 유효기간 체크를 수행하지 않음
//#define TEST_NOT_CHECK_TEST_CARD				// 이 매크로가 define되면 테스트카드도 사용할 수 있음
//#define TEST_NOT_SLEEP_DURING_DISPLAY			// 이 매크로가 define되면 카드처리결과 디스플레이시 SLEEP을 주지 않음
//#define TEST_NO_VOICE							// 이 매크로가 define되면 음성이 출력되지 않음
//#define TEST_NO_BUZZER						// 이 매크로가 define되면 부저가 출력되지 않음
//#define TEST_ALWAYS_ENT						// 이 매크로가 define되면 카드처리시 항상 승차로 처리됨
//#define TEST_TRANS_0_WON						// 이 매크로가 define되면 요금이 무조건 0원으로 처리됨
//#define TEST_TRANS_10_WON						// 이 매크로가 define되면 요금이 무조건 10원으로 처리됨
//#define TEST_NOT_CHECK_MIN_BAL				// 이 매크로가 define되면 선불카드 처리시 최소잔액 체크를 처리하지 않음
//#define TEST_WRITE_SLEEP
//#define TEST_MIF_POSTPAY_CANNOT_READ_SECTOR5
//#define TEST_MIF_POSTPAY_BLOCK20_ALL_ZERO
//#define TEST_MIF_POSTPAY_INVALID_ALIAS
//#define TEST_MIF_PREPAY_ALIAS_0
//#define TEST_NOT_SEND_SUBTERM_TRN_ON_POLLING	// 이 매크로가 define되면 폴링시 미전송 거래내역파일 생성함
//#define TEST_UPDATE_ONE_DAY_BL_PL_AI
//#define TEST_PRINT_TIME_CARD_PROC
//#define TEST_BLPL_CHECK
//#define TEST_DOWN_AND_ROLLBACK
//#define TEST_IDCENTER_KEYSET_REGIST
//#define TEST_DAESA
//#define TEST_SYNC_FW
//#define TEST_SENDING_NOT_SEND_SUBTERM_TRN_ON_POLLING
												// 이 매크로가 define되면 폴링시 미전송 거래내역파일 생성 후 타하차기부착 테스트

#endif

