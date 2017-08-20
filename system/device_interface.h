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
*  PROGRAM ID :       main.c                                                   *
*                                                                              *
*  DESCRIPTION:       Device Open, Use, Close								   *
*                                                                              *
*  ENTRY POINT:       None                  ** mandatory **                    *
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

#ifndef _DEVICE_INTERFACE_H
#define _DEVICE_INTERFACE_H

/*******************************************************************************
*  Inclusion of Header Files                                                 *
*******************************************************************************/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/statfs.h>

#include "../system/bus_type.h"

struct seg_info {
	int disp_no;
	unsigned long value;
	unsigned char *data_str;
} __attribute__((packed)) ;


/*******************************************************************************
*  Macro Defintion                                                             *
*******************************************************************************/
#define F_WRITE							0x01
#define F_READ							0x00

#define VOICE_VOLUME_ADDR				0x88
#define VOICE_IOCTL_TYPE 				'p'
#define VOICE_FLASH_DUMP				_IOR('p',0x82,int)
#define VOICE_FLASH_BLKNUM      		_IOW('p',0x83,long)
#define VOICE_FLASH_ERASE_ALL			_IOW('p',0x84,long)
#define VOICE_FLASH_ID_GET				_IOR('p',0x85,char)
#define VOICE_FLASH_WRITE_ON			_IOW('p',0x86,char)
#define VOICE_BEEP_SET					_IOW('p',0x87,int)
#define VOICE_FREQ_SET					_IOW('p',0x88,int)
#define VOICE_REPEAT_SET				_IOW('p',0x89,int)
#define VOICE_FLASH_BLANK_CK			_IOW('p',0x8a,char)
#define VOICE_FLASH_VERIFY				_IOW('p',0x8b,char)

/*** ioctl command ***/
#define SEG_IOCTL_TYPE 					's'

#define SEG_CONROL						_IOW('s', 0x80, char)
#define SEG_DISP_NUM					_IOW('s', 0x81, char)
#define ALL_LED_ON_OFF					_IOW('s', 0x82, char)
#define SEG_DISP_STR					_IOW('s', 0x83, char)
#define SEG_DISP_CLR					_IOW('s', 0x84, char)

#define I2C_SLAVE	0x0703	/* Change slave address			*/
#define I2C_TENBIT	0x0704	/* 0 for 7 bit addrs, != 0 for 10 bit	*/
#define I2C_RDWR	0x0707	/* Combined R/W transfer (one stop only)*/

#define I2C_M_TEN	0x10	/* we have a ten bit chip address	*/
#define I2C_M_RD						0x01
#define I2C_M_NOSTART					0x4000
#define I2C_M_REV_DIR_ADDR				0x2000

#define VOICE_END_DRIVE					3		// 운행을 종료합니다.
#define VOICE_RETAG_CARD				5		// 카드를 다시 대주세요.
#define VOICE_CHECK_TERM				15		// 단말기를 점검해주시기 바랍니다.
#define VOICE_PRESS_END_BUTTON			16		// 운행이 끝났습니다. 운행 종료 버튼을 눌러 주세요.
#define VOICE_PASSENGER_CHILD			20		// 어린이입니다.
#define VOICE_PASSENGER_YOUNG			21		// 청소년입니다.
#define VOICE_INSUFFICIENT_BAL			23		// 잔액이 부족합니다.
#define VOICE_INVALID_CARD				26		// 사용할 수 없는 카드입니다.
#define VOICE_XFER						27		// 환승입니다.
#define VOICE_ALREADY_PROCESSED			28		// 이미 처리되었습니다.
#define VOICE_START_DRIVE				29		// 운행을 시작합니다.
#define VOICE_EXPIRED_CARD				30		// 카드 유효기간이 지났습니다.
#define VOICE_TAG_ONE_CARD				33		// 카드를 한장만 대주십시오.
#define	VOICE_RECEIVE_LATEST_INFO		35		// 최신 운행정보를 수신하여 주십시오.
#define VOICE_MULTI_ENT					36		// 다인승입니다.
#define VOICE_NOT_APPROV				37		// 미승인 카드입니다.
#define VOICE_TEST_CARD					39		// 테스트 카드입니다.
#define VOICE_TAG_IN_EXT				40		// 내릴때 카드를 대주세요.
#define VOICE_CHECK_TERM_TIME			42		// 단말기 시간을 확인해 주세요.
#define VOICE_CANNOT_MULTI_ENT_CARD		46		// 다인승이 불가능한 카드입니다.
#define VOICE_INPUT_TICKET_INFO			49		// 승차권을 선택해 주세요.
#define VOICE_THANK_YOU					48		// Thank you.
#define VOICE_CITY_PASS_CARD			43		// 시티패스입니다.

/*******************************************************************************
*  Declaration of Function                                                     *
*******************************************************************************/
short OpenVoice( void );
short WriteVoiceFile2Flash( int* fdVoiceFile, int nVoiceFileSize );
void CloseVoice( void );
short VoiceOut( word wVoiceNo );
short OpenRC531( int *fdRC531Device );
void CloseRC531( int *fdRC531Device );
short OpenPrinter( int *fdPrinterDevice, word wBaudrate );
void ClosePrinter( int *fdPrinterDevice );
short DisplayASCInUpFND(byte *abASC);
short DisplayASCInDownFND(byte *abASC);
short DisplayDWORDInUpFND(dword dwNo);
short DisplayDWORDInDownFND(dword dwNo);
short OpenBuzz( void );
void CloseBuzz( void );
short Buzzer( word wCnt, dword dwDelayTime );
short WriteBuzz( char *pcVal, int nVal );
short OpenDipSwitch( void );
void CloseDipSwitch( void );
byte ReadDipSwitch( void );
short OpenEEPROM( void );
void CloseEEPROM( void );
short WriteEEPROM( byte* bWriteData );
short ReadEEPROM( byte* bReadData );
short CheckWriteEEPROM( byte* bWriteData, byte* bReadData );
long MemoryCheck( void );
void OLEDOn(void);
void OLEDOff(void);
void XLEDOn(void);
void XLEDOff(void);
short DisplayCommUpDownMsg( word wUpDown, word wWorkGubunClass );


#endif
