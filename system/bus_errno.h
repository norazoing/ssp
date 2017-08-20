#ifndef _BUS_ERRNO_H_
#define _BUS_ERRNO_H_

#include "../../common/errno.h"

/*******************************************************************************
*  Definition of SetUp Process Error (Main Function)                           *
*******************************************************************************/
#define ERR_SETUP_DRIVE									(short)0x8100

#define ERR_SETUP_DRIVE_MAIN_EXIT						(ERR_SETUP_DRIVE | 0x01)

#define ERR_SETUP_DRIVE_VOICEDATA_2_FLASH				(ERR_SETUP_DRIVE | 0x41)


#define ERR_SETUP_DRIVE_TRAN_SUM_FILE_CHECK				(ERR_SETUP_DRIVE | 0x51)

#define ERR_SETUP_DRIVE_MEMORY_SIZE_ONE					(ERR_SETUP_DRIVE | 0x71)
#define ERR_SETUP_DRIVE_MEMORY_SIZE_TWO					(ERR_SETUP_DRIVE | 0x72)

#define ERR_SETUP_DRIVE_GPS_INFO_FILE					(ERR_SETUP_DRIVE | 0x77)
#define ERR_SETUP_DRIVE_GPS_LOG_FILE					(ERR_SETUP_DRIVE | 0x78)
#define ERR_SETUP_DRIVE_GPS_LOG_FILE2					(ERR_SETUP_DRIVE | 0x79)

#define ERR_SETUP_DRIVE_CMD_SETUP						(ERR_SETUP_DRIVE | 0x81)
#define ERR_SETUP_DRIVE_CANCEL_MULTI_GETON				(ERR_SETUP_DRIVE | 0x82)
#define ERR_SETUP_DRIVE_DRIVE_START_STOP				(ERR_SETUP_DRIVE | 0x83)
#define ERR_SETUP_DRIVE_STATION_ID_CORRECTION			(ERR_SETUP_DRIVE | 0x84)
#define ERR_SETUP_DRIVE_PRINTING_REQ					(ERR_SETUP_DRIVE | 0x85)
#define ERR_SETUP_DRIVE_MSGSEND							(ERR_SETUP_DRIVE | 0x86)

#define ERR_SETUP_DRIVE_GET_CMD_DATA					(ERR_SETUP_DRIVE | 0x97)
#define ERR_SETUP_DRIVE_SET_CMD_DATA					(ERR_SETUP_DRIVE | 0x98)
#define ERR_SETUP_DRIVE_SET_DATA_FOR_CMD				(ERR_SETUP_DRIVE | 0x99)

/*******************************************************************************
*  Definition of KeyPad Process Error                                          *
*******************************************************************************/
#define ERR_KEYPAD										(short)0x8200

#define ERR_KEYPAD_OPEN									(ERR_KEYPAD | 0x01)

#define ERR_KEYPAD_WRITE								(ERR_KEYPAD | 0x11)
#define ERR_KEYPAD_SELECT								(ERR_KEYPAD | 0x12)
#define ERR_KEYPAD_TIMEOUT								(ERR_KEYPAD | 0x13)
#define ERR_KEYPAD_READ									(ERR_KEYPAD | 0x14)
#define ERR_KEYPAD_STX									(ERR_KEYPAD | 0x15)
#define ERR_KEYPAD_ETX									(ERR_KEYPAD | 0x16)
#define ERR_KEYPAD_BCC									(ERR_KEYPAD | 0x17)
#define ERR_KEYPAD_ACK									(ERR_KEYPAD | 0x18)
#define ERR_KEYPAD_CMD									(ERR_KEYPAD | 0x19)

#define ERR_KEYPAD_FLAGFILE_OPEN						(ERR_KEYPAD | 0x21)
#define ERR_KEYPAD_IMAGEFILE_OPEN						(ERR_KEYPAD | 0x22)
#define ERR_KEYPAD_IMAGEFILE_READ						(ERR_KEYPAD | 0x22)
#define ERR_KEYPAD_IMAGEFILE_WRITE						(ERR_KEYPAD | 0x23)
#define ERR_KEYPAD_TC_LEAP_FILE_OPEN					(ERR_KEYPAD | 0x24)

#define ERR_KEYPAD_SETUP_FILE_OPEN						(ERR_KEYPAD | 0x27)
#define ERR_KEYPAD_SETUP_BACKUP_FILE_OPEN				(ERR_KEYPAD | 0x30)

#define ERR_KEYPAD_VERSION_RECEIVE						(ERR_KEYPAD | 0x81)

#define ERR_KEYPAD_IS_DRIVE_NOW							(ERR_KEYPAD | 0x91)
#define ERR_KEYPAD_IS_DCS_COMM							(ERR_KEYPAD | 0x92)
#define ERR_KEYPAD_DRIVER_CMD_LENGTH					(ERR_KEYPAD | 0x93)
#define ERR_KEYPAD_DRIVER_CMD_DATA						(ERR_KEYPAD | 0x94)
#define ERR_KEYPAD_DRIVER_CMD_TIMEOVER					(ERR_KEYPAD | 0x95)
#define ERR_KEYPAD_DRIVER_CMD_LOSS						(ERR_KEYPAD | 0x96)
#define ERR_KEYPAD_DRIVER_CMD_FAIL_RES					(ERR_KEYPAD | 0x97)
/*******************************************************************************
*  Definition of File Management Process Error                                 *
*******************************************************************************/
#define 	ERR_FILE_MGT				(short)0x8300
                                        
#define 	ERR_OUT_OF_MEMORY			(short)0x8351
#define 	ERR_DATA_TYPE_NUM			(short)0x8352
#define 	ERR_DATA_TYPE_DATE			(short)0x8353
#define 	ERR_DATA_CODE				(short)0x8354
#define 	ERR_DATA					(short)0x8355
#define 	ERR_DATA_VERSION			(short)0x8356
#define 	ERR_SET_UPLOAD_VER			(short)0x8357
#define	ERR_LOAD_PARM				(short)0x8358


/*******************************************************************************
*  Definition of Error about Communication MainTerminal with Data Collection   *
*  System                                                                      *
*******************************************************************************/
#define 	ERR_DCS_COMM				(short)0x8400
#define 	ERR_DCS_COMM_NUM			(short)0x8401
#define 	ERR_NOT_DOWN_COMPL			(short)0x8402
#define 	ERR_SIOCGIFCONF				(short)0x8403
#define 	ERR_SOCKET_CREATE			(short)0x8404
#define 	ERR_SOCKET_SET_OPTION		(short)0x8405
#define 	ERR_SOCKET_CONNECT			(short)0x8406
#define 	ERR_SOCKET_TIMEOUT			(short)0x8407
#define 	ERR_SOCKET_AUTH				(short)0x8408
#define 	ERR_SOCKET_SEND				(short)0x8409
#define 	ERR_SOCKET_RECV				(short)0x8410
#define 	ERR_SOCKET_BUFF_OVER_FLOW	(short)0x8411
#define 	ERR_PACKET_STX				(short)0x8412
#define 	ERR_PACKET_ETX				(short)0x8413
#define 	ERR_PACKET_BCC				(short)0x8414
#define 	ERR_DCS_AUTH				(short)0x8415
#define		ERR_RECV_OPER_PARM_FILE		(short)0x8416
#define		ERR_RECV_FILE				(short)0x8417
#define		ERR_SEND_VER_IFNO			(short)0x8418
#define		ERR_SESSION_OPEN			(short)0x8419
#define		ERR_SESSION_CLOSE			(short)0x8420
#define		ERR_SEND_RECONCILE			(short)0x8421
#define		ERR_TRFILE_NOT_EXIST		(short)0x8421
#define		ERR_SEND_FILE				(short)0x8423
#define		ERR_DCS_UPLOAD				(short)0x8424
#define		ERR_DCS_SETUP				(short)0x8425
#define		ERR_SET_LOCAL_IP			(short)0x8426
#define		ERR_RECV_VEHICLE_FILE		(short)0x8427
#define 	ERR_SOCKET_ACK_RECV			(short)0x8428
#define 	ERR_SOCKET_RS_RECV			(short)0x8429
#define 	ERR_SOCKET_RS_SEND			(short)0x8430
#define 	ERR_UPDATE_SEND_RESULT		(short)0x8431
#define		ERR_SOCKET_SELECT			(short)0x8432

/*******************************************************************************
*  Definition of  Error about Communication MainTerminal with SubTerminal      *
*******************************************************************************/
#define ERR_MAINSUB_COMM	(short)0x8500

//#define ERR_MAINSUB_COMM_SELECT                      (ERR_MAINSUB_COMM | 0x0001)
//#define ERR_MAINSUB_COMM_TIMEOUT                     (ERR_MAINSUB_COMM | 0x0002)
#define ERR_MAINSUB_COMM_UART_CLOSE                  (ERR_MAINSUB_COMM | 0x0003)
#define ERR_MAINSUB_COMM_ETX_IN_PKT                  (ERR_MAINSUB_COMM | 0x0004)
#define ERR_MAINSUB_COMM_STX_IN_PKT                  (ERR_MAINSUB_COMM | 0x0005)
#define ERR_MAINSUB_COMM_CRC_IN_PKT                  (ERR_MAINSUB_COMM | 0x0006)
#define ERR_MAINSUB_COMM_PACKET                      (ERR_MAINSUB_COMM | 0x0007)
#define ERR_MAINSUB_COMM_SUB_RECV_DATA_PARSE         (ERR_MAINSUB_COMM | 0x0008)
#define ERR_MAINSUB_COMM_MAIN_RECV_DATA_PARSE        (ERR_MAINSUB_COMM | 0x0009)
#define ERR_DATASIZE_SUBTERM_SEND_BY_CMD_V           (ERR_MAINSUB_COMM | 0x000A)
#define ERR_DATA_SUBTERM_SEND_BY_CMD_G               (ERR_MAINSUB_COMM | 0x000B)
#define ERR_DATASIZE_SUBTERM_SEND_BY_CMD_G           (ERR_MAINSUB_COMM | 0x000C)
//#define   ERR_DATASIZE_SUBTERM_SEND_BY_CMD_F                  0x000D
#define ERR_DATASIZE_SUBTERM_SEND_BY_CMD_H           (ERR_MAINSUB_COMM | 0x000E)
#define ERR_MAINSUB_COMM_SUBTERM_KEYSET_IDCENTER_ADD_PSAM  (ERR_MAINSUB_COMM | 0x000F)
#define ERR_DATASIZE_SUBTERM_SEND_BY_CMD_K           (ERR_MAINSUB_COMM | 0x0011)
#define ERR_DATASIZE_SUBTERM_SEND_BY_CMD_X           (ERR_MAINSUB_COMM | 0x0012)
#define ERR_MAINSUB_COMM_IDCENTER_FILE_OPEN          (ERR_MAINSUB_COMM | 0x0013)
#define ERR_MAINSUB_COMM_IDCENTER_FILE_APPLY_DATA    (ERR_MAINSUB_COMM | 0x0014)
#define ERR_MAINSUB_COMM_IDCENTER_FILE_RECORD_CNT    (ERR_MAINSUB_COMM | 0x0015)
#define ERR_MAINSUB_COMM_IDCENTER_FILE_READ_OR_EOF   (ERR_MAINSUB_COMM | 0x0016)
#define ERR_MAINSUB_COMM_MAINTERM_PSAM_DIFFERENT     (ERR_MAINSUB_COMM | 0x0017)
#define ERR_MAINSUB_COMM_SUBTERM_PSAM_DIFFERENT      (ERR_MAINSUB_COMM | 0x0018)
#define ERR_MAINSUB_COMM_ADD_IDCENTER_OFFLINE        (ERR_MAINSUB_COMM | 0x0019)
#define ERR_MAINSUB_COMM_KEYSET_FILE_OPEN            (ERR_MAINSUB_COMM | 0x001A)
#define ERR_MAINSUB_COMM_KEYSET_FILE_APPLY_DATA      (ERR_MAINSUB_COMM | 0x001B)
#define ERR_MAINSUB_COMM_KEYSET_FILE_RECORD_CNT      (ERR_MAINSUB_COMM | 0x001C)
#define ERR_MAINSUB_COMM_KEYSET_FILE_READ_OR_EOF     (ERR_MAINSUB_COMM | 0x001D)
#define ERR_MAINSUB_COMM_ADD_KEYSET_OFFLINE          (ERR_MAINSUB_COMM | 0x001E)
#define ERR_MAINSUB_COMM_UART_RECV_READ              (ERR_MAINSUB_COMM | 0x001F)
#define ERR_MAINSUB_COMM_ALLTERM_KEYSET_IDCENTER_ADD_PSAM  (ERR_MAINSUB_COMM | 0x0021)
#define ERR_MAINSUB_COMM_MAINTERM_KEYSET_IDCENTER_ADD_PSAM (ERR_MAINSUB_COMM | 0x0022)
#define ERR_MAINSUB_SUBTERM_OPEN_VERSION_FILE        (ERR_MAINSUB_COMM | 0x0023)
#define ERR_MAINSUB_COMM_SUBSTATION_SET              (ERR_MAINSUB_COMM | 0x0024)
#define ERR_MAINSUB_COMM_REQ_FILE_NOT_EXIST          (ERR_MAINSUB_COMM | 0x0025)
#define ERR_MAINSUB_COMM_CMD_P_RECV_FINAL            (ERR_MAINSUB_COMM | 0x0026)
#define ERR_MAINSUB_COMM_CMD_P_SEND_FINAL            (ERR_MAINSUB_COMM | 0x0027)
#define ERR_MAINSUB_COMM_CMD_V_RECV_FINAL            (ERR_MAINSUB_COMM | 0x0028)
#define ERR_MAINSUB_COMM_CMD_V_SEND_FINAL            (ERR_MAINSUB_COMM | 0x0029)
#define ERR_MAINSUB_COMM_CMD_D_RECV_FINAL            (ERR_MAINSUB_COMM | 0x0030)
#define ERR_MAINSUB_COMM_CMD_A_SEND_FINAL			 (ERR_MAINSUB_COMM | 0x0031) 
#define ERR_MAINSUB_COMM_CMD_A_RECV_FINAL			 (ERR_MAINSUB_COMM | 0x0032)
#define ERR_MAINSUB_COMM_CMD_D_SEND_FINAL            (ERR_MAINSUB_COMM | 0x0033)
#define ERR_MAINSUB_COMM_CMD_T_RECV_FINAL            (ERR_MAINSUB_COMM | 0x0034)
#define ERR_MAINSUB_COMM_CMD_M_RECV_FINAL            (ERR_MAINSUB_COMM | 0x0035)
#define ERR_MAINSUB_COMM_CMD_M_SEND_FINAL            (ERR_MAINSUB_COMM | 0x0036)
#define ERR_MAINSUB_COMM_SUBTERM_CMD_M_ACK_FINAL     (ERR_MAINSUB_COMM | 0x0037)
#define ERR_MAINSUB_COMM_SUBTERM_CMD_D               (ERR_MAINSUB_COMM | 0x0038)
#define ERR_MAINSUB_COMM_UART_TESTDATA_SEND_TO_SUBTERM       (ERR_MAINSUB_COMM |0x0039)
#define ERR_MAINSUB_COMM_UART_TESTDATA_RECEIVE_FROM_SUBTERM  (ERR_MAINSUB_COMM |0x0040)
#define ERR_MAINSUB_COMM_UART_TESTDATA_RECEIVE_FROM_MAINTERM (ERR_MAINSUB_COMM |0x0041)
#define ERR_MAINSUB_COMM_UART_TESTDATA_SEND_TO_MAINTERM      (ERR_MAINSUB_COMM |0x0042)
#define ERR_MAINSUB_COMM_SUBTERM_CMD_A						 (ERR_MAINSUB_COMM |0x0043)
#define ERR_MAINSUB_COMM_SUBTERM_SELECT						 (ERR_MAINSUB_COMM |0x0044)

#define ERR_MAINSUB_COMM_NOT_MINE_PKT                   (ERR_MAINSUB_COMM | 0x0045)
#define ERR_MAINSUB_COMM_NOT_CURR_PROT_PKT 	(ERR_MAINSUB_COMM | 0x0054)
#define ERR_MAINSUB_COMM_INVALID_LENGTH					(ERR_MAINSUB_COMM | 0x0047)

#define ERR_MAINSUB_COMM_CMD_A_PARSE_FINAL 				(ERR_MAINSUB_COMM | 0x0049)
#define ERR_MAINSUB_COMM_CMD_D_PARSE_FINAL 				(ERR_MAINSUB_COMM | 0x0050)
#define ERR_MAINSUB_COMM_CMD_M_PARSE_FINAL 				(ERR_MAINSUB_COMM | 0x0051)
#define ERR_MAINSUB_COMM_SEQ_DURING_RECV_FILE			(ERR_MAINSUB_COMM | 0x0052)
#define ERR_MAINSUB_COMM_SEQ_IN_PKT						(ERR_MAINSUB_COMM | 0x0053)
#define ERR_MAINSUB_COMM_ASSGN_SUM_TERM_ID				(ERR_MAINSUB_COMM | 0x0054)

#define ERR_MAINSUB_COMM_IGNORE_IN_PKT					(ERR_MAINSUB_COMM | 0x0055)
/* SendFile 함수 관련 에러 */
#define ERR_MAINSUB_COMM_FILE_NOT_FOUND				    (ERR_MAINSUB_COMM | 0x0046)
#define ERR_MAINSUB_COMM_PARM_DOWN_NAK					(ERR_MAINSUB_COMM | 0x0048)
#define ERR_MAINSUB_COMM_FILE_NOT_FOUND_RECV		    (ERR_MAINSUB_COMM | 0x0056)
#define ERR_MAINSUB_COMM_FILE_NOT_FOUND_EOT_SEND	    (ERR_MAINSUB_COMM | 0x0057)
/* SendSubTermImgFile 함수 관련 에러 */
#define ERR_MAINSUB_COMM_SUBTERM_IMG_FILE_OPEN			(ERR_MAINSUB_COMM | 0x0058)
#define ERR_MAINSUB_COMM_SUBTERM_IMG_DOWN_NAK			(ERR_MAINSUB_COMM | 0x0059)
/*******************************************************************************     
* Following definition exist in errno.h        								   *
********************************************************************************
* #define ERR_RF_COMMM									(short)0x8600          *
* #define ERR_SAM_COMM									(short)0x8700          *
* #define ERR_MIF_KEY_MGT	           					(short)0x8800          *
*******************************************************************************/

/*******************************************************************************
*  Definition of  Error about BLPL Process                                     *
*******************************************************************************/
#define ERR_BLPL_PROC		(short)0x8900

#define ERR_BL_GENERAL_	 	(short)0x8901
#define ERR_PL_GENERAL_	 	(short)0x8902
#define ERR_AI_GENERAL_	 	(short)0x8903

#define ERR_BLPL_PROC_SUBTERM_BL_REQ_CHECK_TIME_OUT		(ERR_BLPL_PROC | 0x01)
#define ERR_BLPL_PROC_SUBTERM_PL_REQ_CHECK_TIME_OUT		(ERR_BLPL_PROC | 0x02)
#define ERR_BLPL_PROC_BL_MERGE_FAIL						(ERR_BLPL_PROC | 0x03)
/* Error code relating to write BLPL CRC to install.dat file*/
#define ERR_BLPL_PROC_OPEN_BLPL_CRC_FILE				(ERR_BLPL_PROC | 0x04)
#define ERR_BLPL_PROC_READ_BLPL_CRC_FILE				(ERR_BLPL_PROC | 0x05)
#define ERR_BLPL_PROC_WRITE_BL_CRC_FILE		     	    (ERR_BLPL_PROC | 0x06)
#define ERR_BLPL_PROC_WRITE_PL_CRC_FILE		     	    (ERR_BLPL_PROC | 0x07)
#define ERR_BLPL_PROC_WRITE_AI_CRC_FILE		     	    (ERR_BLPL_PROC | 0x08)

#define ERR_BLPL_PROC_CRCTYPE                           (ERR_BLPL_PROC | 0x09)

#define ERR_BLPL_PROC_PL_MERGE_FAIL					    (ERR_BLPL_PROC | 0x0A)
#define ERR_BLPL_PROC_AI_MERGE_FAIL						(ERR_BLPL_PROC | 0x0B)
#define ERR_BLPL_PROC_BL_CHECK_TIMEOUT					(ERR_BLPL_PROC | 0x0C)
#define ERR_BLPL_PROC_PL_CHECK_TIMEOUT					(ERR_BLPL_PROC | 0x0D)
/*******************************************************************************
*  Definition of  Error about Card Process                                     *
*******************************************************************************/
#define ERR_CARD_PROC									(short)0x9100

#define ERR_CARD_PROC_LOG								(ERR_CARD_PROC | 0x01)
#define ERR_CARD_PROC_MIF_PREPAY_INVALID_ISSUER         (ERR_CARD_PROC | 0x02)

#define ERR_CARD_PROC_MIF_POSTPAY_INVALID_CARD_NO       (ERR_CARD_PROC | 0x04)
#define ERR_CARD_PROC_MIF_POSTPAY_EXPIRE                (ERR_CARD_PROC | 0x05)
#define ERR_CARD_PROC_MIF_POSTPAY_INVALID_ISSUER        (ERR_CARD_PROC | 0x06)
#define ERR_CARD_PROC_MIF_POSTPAY_INVALID_ISSUER_VALID_PERIOD \
												        (ERR_CARD_PROC | 0x07)
#define ERR_CARD_PROC_MIF_POSTPAY_BL_PL_CHECK           (ERR_CARD_PROC | 0x08)
#define ERR_CARD_PROC_MIF_POSTPAY_NL_CARD               (ERR_CARD_PROC | 0x09)
#define ERR_CARD_PROC_MIF_POSTPAY_INVALID_ALIAS         (ERR_CARD_PROC | 0x0A)
#define ERR_CARD_PROC_MIF_POSTPAY_CHK_CICC              (ERR_CARD_PROC | 0x0B)
#define ERR_CARD_PROC_STATION_ID						(ERR_CARD_PROC | 0x0C)
#define ERR_CARD_PROC_MIF_PREPAY_PL_CHECK				(ERR_CARD_PROC | 0x0D)
#define ERR_CARD_PROC_MIF_PREPAY_NL_CARD				(ERR_CARD_PROC | 0x0E)
#define ERR_CARD_PROC_MIF_PREPAY_PL_ALIAS				(ERR_CARD_PROC | 0x0F)

#define ERR_CARD_PROC_FILE_OPEN_CONTROL_TRANS           (ERR_CARD_PROC | 0x50)
#define ERR_CARD_PROC_FILE_READ_CONTROL_TRANS           (ERR_CARD_PROC | 0x51)
#define ERR_CARD_PROC_FILE_WRITE_CONTROL_TRANS          (ERR_CARD_PROC | 0x52)
#define ERR_CARD_PROC_FILE_OPEN_TRANS					(ERR_CARD_PROC | 0x53)
#define ERR_CARD_PROC_FILE_READ_TRANS					(ERR_CARD_PROC | 0x54)
#define ERR_CARD_PROC_FILE_WRITE_TRANS					(ERR_CARD_PROC | 0x55)
#define ERR_CARD_PROC_FILE_RENAME_TRANS					(ERR_CARD_PROC | 0x56)
#define ERR_CARD_PROC_FILE_OPEN_MAIN_TRANS				(ERR_CARD_PROC | 0x57)
#define ERR_CARD_PROC_FILE_READ_MAIN_TRANS				(ERR_CARD_PROC | 0x58)
#define ERR_CARD_PROC_FILE_WRITE_MAIN_TRANS				(ERR_CARD_PROC | 0x59)
#define ERR_CARD_PROC_FILE_RENAME_MAIN_TRANS			(ERR_CARD_PROC | 0x5A)
#define ERR_CARD_PROC_FILE_OPEN_SUB_TRANS				(ERR_CARD_PROC | 0x5B)
#define ERR_CARD_PROC_FILE_READ_SUB_TRANS				(ERR_CARD_PROC | 0x5C)
#define ERR_CARD_PROC_FILE_WRITE_SUB_TRANS				(ERR_CARD_PROC | 0x5D)
#define ERR_CARD_PROC_FILE_RENAME_SUB_TRANS				(ERR_CARD_PROC | 0x5E)
#define ERR_CARD_PROC_FILE_OPEN_BACKUP_TRANS			(ERR_CARD_PROC | 0x5F)
#define ERR_CARD_PROC_FILE_READ_BACKUP_TRANS			(ERR_CARD_PROC | 0x60)
#define ERR_CARD_PROC_FILE_WRITE_BACKUP_TRANS			(ERR_CARD_PROC | 0x61)
#define ERR_CARD_PROC_FILE_RENAME_BACKUP_TRANS			(ERR_CARD_PROC | 0x62)
#define ERR_CARD_PROC_FILE_OPEN_SUB_REMAKE				(ERR_CARD_PROC | 0x63)
#define ERR_CARD_PROC_FILE_READ_SUB_REMAKE				(ERR_CARD_PROC | 0x64)
#define ERR_CARD_PROC_FILE_WRITE_SUB_REMAKE				(ERR_CARD_PROC | 0x65)
#define ERR_CARD_PROC_FILE_RENAME_SUB_REMAKE			(ERR_CARD_PROC | 0x66)
#define ERR_CARD_PROC_FILE_OPEN_TEMP_REMAKE				(ERR_CARD_PROC | 0x67)
#define ERR_CARD_PROC_FILE_READ_TEMP_REMAKE				(ERR_CARD_PROC | 0x68)
#define ERR_CARD_PROC_FILE_WRITE_TEMP_REMAKE			(ERR_CARD_PROC | 0x69)
#define ERR_CARD_PROC_FILE_RENAME_TEMP_REMAKE			(ERR_CARD_PROC | 0x6A)
#define ERR_CARD_PROC_FILE_OPEN_SUB_TEMP_REMAKE			(ERR_CARD_PROC | 0x6B)
#define ERR_CARD_PROC_FILE_READ_SUB_TEMP_REMAKE			(ERR_CARD_PROC | 0x6C)
#define ERR_CARD_PROC_FILE_WRITE_SUB_TEMP_REMAKE		(ERR_CARD_PROC | 0x6D)
#define ERR_CARD_PROC_FILE_RENAME_SUB_TEMP_REMAKE		(ERR_CARD_PROC | 0x6E)
#define ERR_CARD_PROC_FILE_WRITE_RECONCILE_LIST         (ERR_CARD_PROC | 0x6F)

#define ERR_CARD_PROC_SCREAD							(ERR_CARD_PROC | 0xA0)
#define ERR_CARD_PROC_SCREAD_PURSEINFO					(ERR_CARD_PROC | 0xA1)
#define ERR_CARD_PROC_SCREAD_TRANS						(ERR_CARD_PROC | 0xA2)
#define ERR_CARD_PROC_SCREAD_PURSE						(ERR_CARD_PROC | 0xA3)
#define ERR_CARD_PROC_SCREAD_INSUFF_FARE				(ERR_CARD_PROC | 0xA4)
#define ERR_CARD_PROC_SCREAD_PREPAY_CHECK				(ERR_CARD_PROC | 0xA5)
#define ERR_CARD_PROC_SCREAD_POSTPAY_CHECK				(ERR_CARD_PROC | 0xA6)
#define ERR_CARD_PROC_SCREAD_VERIFY_PURSE				(ERR_CARD_PROC | 0xA7)
#define ERR_CARD_PROC_SCREAD_PURSE_LOAD					(ERR_CARD_PROC | 0xA8)
#define ERR_CARD_PROC_SCREAD_PAY_PREV					(ERR_CARD_PROC | 0xA9)
#define ERR_CARD_PROC_SCREAD_NL_CARD					(ERR_CARD_PROC | 0xAA)

#define ERR_CARD_PROC_SCWRITE							(ERR_CARD_PROC | 0xB1)

#define ERR_CARD_PROC_NOT_ONE_CARD						(ERR_CARD_PROC | 0xF0)
#define ERR_CARD_PROC_CANNOT_USE                        (ERR_CARD_PROC | 0xF1)
#define ERR_CARD_PROC_RETAG_CARD                        (ERR_CARD_PROC | 0xF2)
#define ERR_CARD_PROC_INSUFFICIENT_BAL                  (ERR_CARD_PROC | 0xF3)
#define ERR_CARD_PROC_ALREADY_PROCESSED                 (ERR_CARD_PROC | 0xF4)
#define ERR_CARD_PROC_EXPIRED_CARD                      (ERR_CARD_PROC | 0xF5)
#define ERR_CARD_PROC_TAG_IN_EXT						(ERR_CARD_PROC | 0xF6)
#define ERR_CARD_PROC_NOT_APPROV						(ERR_CARD_PROC | 0xF7)
#define ERR_CARD_PROC_NO_VOICE							(ERR_CARD_PROC | 0xF8)
#define ERR_CARD_PROC_CANNOT_MULTI_ENT					(ERR_CARD_PROC | 0xF9)
#define ERR_CARD_PROC_INPUT_TICKET						(ERR_CARD_PROC | 0xFA)
#define ERR_CARD_PROC_CITY_PASS_CARD					(ERR_CARD_PROC | 0xFB)

/*******************************************************************************
* Following definition exist in errno.h        								   *
********************************************************************************
*  #define ERR_CARD_IF_SC									(short)0x9200      *
*  #define ERR_CARD_IF_MIF									(short)0x9300	   *
*  #define ERR_PAY_LIB										(short)0x9400      *
*  #define ERR_SAM_IF										(short)0x9500      *
*  #define ERR_UART_LIB                                   	(short)0x9600      *
*******************************************************************************/


/*******************************************************************************
*  Peripherial Device                                                          *
*******************************************************************************/
#define ERR_DEVICE										(short)0x9700
                                                    	
#define ERR_DEVICE_VOICE_OPEN							(ERR_DEVICE | 0x01)
#define ERR_DEVICE_VOICE_FLASH_WRITE_ON					(ERR_DEVICE | 0x02)
#define ERR_DEVICE_VOICE_FLASH_ID_GET					(ERR_DEVICE | 0x03)
#define ERR_DEVICE_VOICE_FLASH_DUMP						(ERR_DEVICE | 0x04)
#define ERR_DEVICE_VOICE_FLASH_BLANK_CK					(ERR_DEVICE | 0x05)
#define ERR_DEVICE_VOICE_OUT							(ERR_DEVICE | 0x06)
#define ERR_DEVICE_RC531_OPEN							(ERR_DEVICE | 0x11)
#define ERR_DEVICE_PRINTER_OPEN							(ERR_DEVICE | 0x21)
#define ERR_DEVICE_FND_OPEN								(ERR_DEVICE | 0x31)
#define ERR_DEVICE_FND_DISPLAY_NO						(ERR_DEVICE | 0x32)
#define ERR_DEVICE_FND_DISPLAY_STIRNG					(ERR_DEVICE | 0x33)
#define ERR_DEVICE_FND_DISPLAY_STIRNG1					(ERR_DEVICE | 0x34)
#define ERR_DEVICE_FND_DISPLAY_NO1						(ERR_DEVICE | 0x35)
#define ERR_DEVICE_FND_ONOFF							(ERR_DEVICE | 0x36)
#define ERR_DEVICE_BUZZ_OPEN							(ERR_DEVICE | 0x41)
#define ERR_DEVICE_BUZZER								(ERR_DEVICE | 0x42)
#define ERR_DEVICE_BUZZ_WRITE							(ERR_DEVICE | 0x43)
#define ERR_DEVICE_DIPSWITCH_OPEN						(ERR_DEVICE | 0x51)
#define ERR_DEVICE_DIPSWITCH_READ						(ERR_DEVICE | 0x52)
#define ERR_DEVICE_EEPROM_OPEN							(ERR_DEVICE | 0x61)
#define ERR_DEVICE_EEPROM_WRITE							(ERR_DEVICE | 0x62)
#define ERR_DEVICE_EEPROM_READ							(ERR_DEVICE | 0x63)
#define ERR_DEVICE_STAT_FILESYSTEM						(ERR_DEVICE | 0x71)

/*******************************************************************************
*  Definition of  Generail File Error using file error code                    *
********************************************************************************
* ex) return ErrRet( ERR_FILE_OPEN | FCODE1 );                                 *
*******************************************************************************/
#define ERR_FILE_OPEN								(short)0xA100
#define ERR_FILE_WRITE								(short)0xA200
#define ERR_FILE_READ								(short)0xA300
#define ERR_FILE_REMOVE								(short)0xA400
#define ERR_FILE_DATA_NOT_FOUND						(short)0xA500
#define ERR_FILE_RENAME								(short)0xA600
#define ERR_FILE_SIZE								(short)0xA700
#define ERR_FILE_INFO_READ							(short)0xA800

////////////////////////////////////////////////////////////////////////////////
/*******************************************************************************
*  ALARM CODE Definition for ctrl_event_info_write function is used limitedly. *
*  After ver.0401 program running status is stable, following macro will be    *
*  obsolete.                                                                   *
*******************************************************************************/
/* 0401에서 UPDATEPL함수의 구조변경에 따라 필요없어진 에러코드들 */
/* #define BL_PREFIX_ERROR   -2
   #define BL_SERIAL_ERROR   -3
   #define BL_SIZE_ERROR     -4
   #define SORT_ERROR        -3
   #define PRE_RANGE_ERROR   -4
   #define POST_RANGE_ERROR  -5
   #define TOTAL_RANGE_ERROR -9
	
   #define AI_SORT_ERROR     -6
   #define AI_RANGE_ERROR    -7    */

/* 파일 다운로드 관련 알람코드 */
// 운행에 필수적인 운영정보 ////////////////////////////////////////////////////
#define EVENT_LOAD_ERR_VEHICLE_INFO				"2001"	// 차량정보
#define EVENT_LOAD_ERR_ROUTE_INFO				"2002"	// 노선정보
#define EVENT_LOAD_ERR_NEW_FARE_INFO			"2003"	// 신요금정보
#define EVENT_LOAD_ERR_STATION_INFO				"2004"	// 정류장정보

// 미존재하더라도 운행에는 큰 지장이 없는 운영정보 /////////////////////////////
#define EVENT_LOAD_ERR_XFER_APPLY_INFO			"2005"	// 환승적용정보
#define EVENT_LOAD_ERR_DIS_EXTRA_INFO			"2006"	// 할인할증정보
#define EVENT_LOAD_ERR_HOLIDAY_INFO				"2007" 	// 휴일정보
#define EVENT_LOAD_ERR_PREPAY_ISSUER_INFO		"2008"	// 선불발행사정보
#define EVENT_LOAD_ERR_POSTPAY_ISSUER_INFO		"2009"	// 후불발행사정보
#define EVENT_LOAD_ERR_ISSUER_VALID_PERIOD_INFO	"2010"	// 발행사유효기간정보

#define EVENT_DRIVE_START_WITHOUT_MANDATORY_INFO \
												"2031"	// 필수운영정보 없이 운행시작
#define EVENT_DRIVE_START_INVALID_VEHICLE_INFO	"2032"	// 차량정보 오류시 운행시작

/* KeySet Write관련 알람코드  */
#define KEYSET_SUCC								"3000"
#define KEYSET_FAIL1							"3100"
#define KEYSET_FAIL2							"3001"
#define KEYSET_FAIL3							"3101"

/* UpdatePL함수의 구조변경에 따라 필요없어진 알람코드들 */
//#define SORT_ERROR_EVENT  				"4001" 
//#define PRE_RANGE_ERROR_EVENT 			"4002"
//#define POST_RANGE_ERROR_EVENT 			"4003"
//#define AI_SORT_ERROR_EVENT           	"4005"

/* UpdatePL관련 알람코드  -> pl.c의 common module 사용으로 errno.h로 이전*/
//#define TOTAL_RANGE_ERROR_EVENT 			"4004"
//#define AI_RANGE_ERROR_EVENT         		"4006"

/* 머지중 BL/PL 버림 이벤트 코드 */
#define EVENT_UNLINK_BL_DURING_MERGE			"4031"
#define EVENT_UNLINK_MIF_PREPAY_PL_DURING_MERGE	"4032"
#define EVENT_UNLINK_POSTPAY_PL_DURING_MERGE	"4033"
#define EVENT_UNLINK_SC_PREPAY_PL_DURING_MERGE	"4034"

/* BL Merge관련 알람코드  */
#define BL_PREFIX_ERROR_EVENT					"5001"
#define BL_SERIAL_ERROR_EVENT 					"5002"
#define BL_SIZE_ERROR_EVENT     				"5003"

/* BLPL VER ROLLBACK실패시 발생 알람코드 */
#define PL_GENERAL_ERROR_EVENT					"4007"
#define AI_GENERAL_ERROR_EVENT					"4008"
#define BL_GENERAL_ERROR_EVENT					"5004"

/* 거래내역 파일 관련 알람코드 */
#define TR_FILE_OPEN_ERROR_EVENT 				"6001"
#define TR_FILE_WRITE_ERROR_EVENT 				"6002"
#define TR_FILE_BACKUP_ERROR_EVENT 				"6003"
#define TR_REMAKE_RENAME_ERROR_EVENT			"6004"
#define TR_FILE_HEADER_ERROR_EVENT				"6005"

/* 메모리 사이즈 관련 알람코드 */
#define MEMORY_SIZE_ONE_ERROR_EVENT 			"6010"
#define MEMORY_SIZE_TWO_ERROR_EVENT 			"6011"
#define EVENT_INSUFFICIENT_DISK_DURING_DOWNLOAD	"6012"

/* Control.trn관련 알람코드 */
#define CONTROLTRN_NOTFOUND_ERROR_EVENT			"6020"
#define CONTROLTRN_OPEN_ERROR_EVENT 			"6021"

#define TRFILE_EXIST_ERROR_EVENT 				"7001"

#define EVENT_BOOTING_DURING_DRIVING			"8001"
#define EVENT_RESETUP_TERM						"8002"
#define EVENT_RTC_ERROR							"8003"
#define EVENT_GPS_THREAD_REFRESH				"8004"

////////////////////////////////////////////////////////////////////////////////


extern char* achFileCode[];

void ErrProc( short sErrCode );
void ErrLogWrite( short sErrCode );
void LogWrite( short sLogCode );
int SemaCreate( int nKey );
int SemaAlloc( int nKey );
int SemaFree( int nKey );

/* 0401버젼에서만 한시적으로 사용 */
int ctrl_event_info_write (char * ALARM_CD);

#endif

