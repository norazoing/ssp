
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
*  PROGRAM ID :       device_interface.c                                       *
*                                                                              *
*  DESCRIPTION:       This Program Open/ Write/ Read/ Close Devices			   *
*                                                                              *
*  ENTRY POINT:       None													   *			
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: None		                                               *
*                                                                              *
*  INPUT FILES:       None			                                           *
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
* 2005/08/13 Solution Team  Gwan Yul Kim  Initial Release                      *
*                                                                              *
*******************************************************************************/

/*******************************************************************************
*  Inclusion of Header Files                                                   *
*******************************************************************************/
#include "device_interface.h"

/*******************************************************************************
*  Declaration of Module Global Variables                                      *
*******************************************************************************/
static int gfdVoiceDevice = 0;
static int gfdFNDDevice = 0;
static int gfdBuzzDevice = 0;
static int gfdDipSwitch = 0;
static int gfdEEPROM = 0;

static short OpenFND( void );
static void CloseFND( void );
static short OnOffFND( word wCtrlVal );
static short FNDDisplayNo(int nFNDNo, dword wDispVal );
static short FNDDisplayString(int nFNDNo, byte* bDispData );

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      OpenVoice                                                *
*                                                                              *
*  DESCRIPTION:       This Program Opens Voice Device						   *
*                                                                              *
*  INPUT PARAMETERS:  None                      							   *
*                                                                              *
*  RETURN/EXIT VALUE: 0 - Success                                              *
*                     (-) - Error                                              *
*                                                                              *
*  Author : GwanYul Kim													       *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short OpenVoice( void )
{
	/*디바이스 open */
	gfdVoiceDevice = open( VOICE_DEVICE, O_RDWR );
	if ( gfdVoiceDevice < 0 )
	{
		DebugOut( "====voice open error1====\n" );
		return ErrRet( ERR_DEVICE_VOICE_OPEN );
	}

	return SUCCESS;	
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      WriteVoiceFile2Flash                                     *
*                                                                              *
*  DESCRIPTION:       This program reads c_v0.dat & Write to Flash			   *
*                     and saves Common Structure.                              *
*                                                                              *
*  INPUT PARAMETERS:  int* fdVoiceFile										   *	
*					  int nVoiceFileSize                                       *
*                                                                              *
*  RETURN/EXIT VALUE: 1 - Success                                              *
*                     (-) - Error                                              *
*                                                                              *
*  Author : GwanYul Kim													       *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short WriteVoiceFile2Flash( int* fdVoiceFile, int nVoiceFileSize )
{
	int nRetVal = 0;
	int nIndex = 0;
	int nReadByte = 0;
	int nSelectReadWrite = 0;
	int nOffset = 0;
	int nSectorNum = 0;
	volatile unsigned char acBuf[257];
	
	/***** Voice Flash AT29LV040 write ************/
	nSelectReadWrite = F_WRITE;
	
	/* very important */
	nRetVal = ioctl( gfdVoiceDevice, VOICE_FLASH_WRITE_ON, &nSelectReadWrite );
	if( nRetVal != SUCCESS )
	{
		DebugOut( "====VOICE_FLASH_WRITE_ON=====\n" );
		ErrLogWrite( ERR_DEVICE_VOICE_FLASH_WRITE_ON );
	}

	nRetVal = ioctl( gfdVoiceDevice, VOICE_FLASH_ID_GET , 0 );
	if( nRetVal != SUCCESS )
	{
		DebugOut( "====VOICE_FLASH_ID_GET=====\n" );
		ErrLogWrite( ERR_DEVICE_VOICE_FLASH_ID_GET );
	}
		
	nOffset = 0x800;
	nRetVal = ioctl( gfdVoiceDevice, VOICE_FLASH_DUMP ,&nOffset );
	if( nRetVal != SUCCESS )
	{
		DebugOut( "=====debugging flash memory dump=====\n" );
		ErrLogWrite( ERR_DEVICE_VOICE_FLASH_DUMP );		
	}
	
	DebugOut( "Erase blank checking..." );
	nRetVal = ioctl( gfdVoiceDevice, VOICE_FLASH_BLANK_CK, 0 );	
	if ( nRetVal != SUCCESS ) 
	{
		DebugOut( "Flash Not Erased !!\n" );	
		ErrLogWrite( ERR_DEVICE_VOICE_FLASH_BLANK_CK );
	}
	DebugOut( "WriteVoiceFile2Flash ... Done.\n" );
	
	DebugOut( "Programming..." );
	nSectorNum = ( nVoiceFileSize >> 8 ) + 1;
	DebugOut( "Sector Num = %d\n", nSectorNum );
	for( nIndex = 0; nIndex < nSectorNum; nIndex++ ) 
	{
		memset( (void*)acBuf, 0x00, sizeof(acBuf) );
		nReadByte = read( *fdVoiceFile, (void *)acBuf, 256 );
		write( gfdVoiceDevice, (void *)acBuf, 256 );
	}
		
	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      CloseVoice                                               *
*                                                                              *
*  DESCRIPTION:       This program Close Voice Device                          *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: None                                                     *
*                                                                              *
*  Author : GwanYul Kim													       *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
void CloseVoice( void )
{
	/*디바이스 Close */
	close( gfdVoiceDevice );
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      VoiceOut                                                 *
*                                                                              *
*  DESCRIPTION:       Out VoiceNo to Speaker                                   *
*                                                                              *
*  INPUT PARAMETERS:  word wVoiceNo                                            *
*                                                                              *
*  RETURN/EXIT VALUE: 0 - Success                                              *
*                     (-) - Error                                              *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short VoiceOut( word wVoiceNo )
{
	int nResult = 0;

#ifndef TEST_NO_VOICE
	nResult = read( gfdVoiceDevice, &wVoiceNo, 0 );
#endif
	if ( nResult < 0 )
	{
		DebugOut( "\n<VoiceOut>VoiceOut Error ! \n" );
		return ErrRet( ERR_DEVICE_VOICE_OUT );
	}
	
	return SUCCESS;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      OpenRC531                                                *
*                                                                              *
*  DESCRIPTION:       This program opens RC531 Device Port.                    *
*                                                                              *
*  INPUT PARAMETERS:  int *fdRC531Device                                       *
*                                                                              *
*  RETURN/EXIT VALUE: 0 - Success                                              *
*                     (-) - Error                                              *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short OpenRC531( int *fdRC531Device )
{	
	*fdRC531Device = open( RC531_DEVICE, O_RDWR );
	if ( *fdRC531Device <= 0 ) 
	{
		DebugOut( "\n<OpenRC531>RC531 Open Error(카드 처리가 불가능) !!!!!\n" );
		return ErrRet( ERR_DEVICE_RC531_OPEN );
	}
	
	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      CloseRC531                                               *
*                                                                              *
*  DESCRIPTION:       This program closes RC531 Device Port.                   *
*                                                                              *
*  INPUT PARAMETERS:  int *fdRC531Device                                       *
*                                                                              *
*  RETURN/EXIT VALUE: None                                                     *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
void CloseRC531( int *fdRC531Device )
{
	/*디바이스 Close */
	close( *fdRC531Device );
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      OpenPrinter                                              *
*                                                                              *
*  DESCRIPTION:       This program opens Receipt Printer Port.                 *
*                                                                              *
*  INPUT PARAMETERS:  int *fdPrinterDevice                                     *
*                     word wBaudrate                                           *
*                                                                              *
*  RETURN/EXIT VALUE: 0 - Success                                              *
*                     (-) - Error                                              *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/	
short OpenPrinter( int *fdPrinterDevice, word wBaudrate )
{
	struct termios stOptions;
	struct termios stNewCommIO;

	*fdPrinterDevice = open( PRINTER_DEVICE, O_RDWR | O_NOCTTY | O_NONBLOCK);
	if( *fdPrinterDevice < 0 )
	{
		DebugOut( "\n<OpenPrinter>open error\n" );
		return ErrRet( ERR_DEVICE_PRINTER_OPEN );
	}

	memset( &stOptions, 0x00, sizeof(struct termios) );
	memset( &stNewCommIO, 0x00, sizeof(struct termios) );
	
	tcgetattr( *fdPrinterDevice, &stOptions );

	switch( wBaudrate )
	{
		case 9600 :
			cfsetispeed( &stNewCommIO, B9600 );
			cfsetospeed (&stNewCommIO, B9600 );						
			break;
		case 38400 :
			cfsetispeed (&stNewCommIO, B38400 );
			cfsetospeed (&stNewCommIO, B38400 );			
			break;
		case 115200 :
			cfsetispeed (&stNewCommIO, B115200 );
			cfsetospeed (&stNewCommIO, B115200 );			
			break;
		case 230400 :
			cfsetispeed (&stNewCommIO, B230400 );
			cfsetospeed (&stNewCommIO, B230400 );			
			break;
		case 460800 :
			cfsetispeed (&stNewCommIO, B460800 );
			cfsetospeed (&stNewCommIO, B460800 );			
			break;
		case 500000 :
			cfsetispeed (&stNewCommIO, B500000 );
			cfsetospeed (&stNewCommIO, B500000 );			
			break;
		case 576000 :
			cfsetispeed (&stNewCommIO, B576000 );
			cfsetospeed (&stNewCommIO, B576000 );			
			break;
		case 921600 :
			cfsetispeed (&stNewCommIO, B921600 );
			cfsetospeed (&stNewCommIO, B921600 );			
			break;
	}

	stNewCommIO.c_cflag &= ~PARENB;
	stNewCommIO.c_cflag &= ~CSTOPB;
	stNewCommIO.c_cflag &= ~CSIZE;  
	
	stNewCommIO.c_cflag |= (CS8| CLOCAL | CREAD);
	
//	stNewCommIO.c_lflag |= ICANON;
	stNewCommIO.c_iflag = IGNPAR | ICRNL;			
	stNewCommIO.c_oflag = 0;//OPOST;
	
	stNewCommIO.c_cc[VINTR] = 0;
	stNewCommIO.c_cc[VQUIT] = 0;
	stNewCommIO.c_cc[VERASE] = 0;
	stNewCommIO.c_cc[VKILL] = 0;
	stNewCommIO.c_cc[VEOF] = 4;
	stNewCommIO.c_cc[VTIME] = 0;	//20; //20*0.1 = 2sec rx time out
	stNewCommIO.c_cc[VMIN] = 0;
	stNewCommIO.c_cc[VSWTC] = 0;
	stNewCommIO.c_cc[VSTART] = 0;
	stNewCommIO.c_cc[VSTOP] = 0;
	stNewCommIO.c_cc[VEOL] = 0;
	stNewCommIO.c_cc[VREPRINT] = 0;
	stNewCommIO.c_cc[VDISCARD] = 0;
	stNewCommIO.c_cc[VWERASE] = 0;
	stNewCommIO.c_cc[VLNEXT] = 0;
	stNewCommIO.c_cc[VEOL2] = 0;
	
	tcflush( *fdPrinterDevice, TCIFLUSH );		
	tcsetattr(* fdPrinterDevice, TCSANOW , &stNewCommIO );
	
	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      ClosePrinter                                             *
*                                                                              *
*  DESCRIPTION:       This program closes Receipt Printer Port.                *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: None                                                     *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
void ClosePrinter( int *fdPrinterDevice )
{
	/*디바이스 Close */
	close( *fdPrinterDevice );
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      OpenFND                                                  *
*                                                                              *
*  DESCRIPTION:       This program opens FND Display Port.                     *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: 0 - Success                                              *
*                     (-) - Error                                              *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/	
static short OpenFND( void )
{
	/*디바이스 open */
	gfdFNDDevice = open( FND_DEVICE, O_RDONLY);
	if( gfdFNDDevice < 0 )
	{
		DebugOut("segment open error1\n");
		return ErrRet( ERR_DEVICE_FND_OPEN );
	}
	
	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      CloseFND                                                 *
*                                                                              *
*  DESCRIPTION:       This program closes FND Device Port.                     *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: None                                                     *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static void CloseFND( void )
{
	close( gfdFNDDevice );
}

short DisplayASCInUpFND(byte *abASC)
{
	return FNDDisplayString(0, abASC);
}

short DisplayASCInDownFND(byte *abASC)
{
	return FNDDisplayString(1, abASC);
}

short DisplayDWORDInUpFND(dword dwNo)
{
	return FNDDisplayNo(0, dwNo);
}

short DisplayDWORDInDownFND(dword dwNo)
{
	return FNDDisplayNo(1, dwNo);
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      FNDDisplayNo                                            *
*                                                                              *
*  DESCRIPTION:       This program displays FND Number  					   *
*                                                                              *
*  INPUT PARAMETERS:  int nFNDNo                                               *
*                     dword wDispVal                                           *
*                                                                              *
*  RETURN/EXIT VALUE: 0 - Success                                              *
*                     (-) - Error                                              *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short FNDDisplayNo(int nFNDNo, dword wDispVal )
{
	short sRetVal = 0;		
 	struct seg_info stFND;
	
	stFND.disp_no = nFNDNo;
	stFND.value   = wDispVal;
	
	OpenFND();		
	sRetVal = ioctl( gfdFNDDevice, SEG_DISP_NUM ,&stFND );	
	CloseFND();
	
	if ( sRetVal < 0 )
	{
		return ErrRet( ERR_DEVICE_FND_DISPLAY_NO1 );
	}
	
	return SUCCESS;	
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      FNDDisplayString1                                        *
*                                                                              *
*  DESCRIPTION:       This program displays FND String  					   *
*                                                                              *
*  INPUT PARAMETERS:  int nFNDNo                                               *
*                     byte* bDispData                                          *
*                                                                              *
*  RETURN/EXIT VALUE: 0 - Success                                              *
*                     (-) - Error                                              *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short FNDDisplayString(int nFNDNo, byte* bDispData )
{
	short sRetVal = 0;	
	struct seg_info stFND;
	
	memset( &stFND, 0x00, sizeof(struct seg_info) );
	stFND.disp_no  = nFNDNo;
	stFND.value    = 0;
	stFND.data_str = bDispData;	
	
	OpenFND();	
	sRetVal = ioctl( gfdFNDDevice, SEG_DISP_STR ,&stFND );		
	CloseFND();
	
	if ( sRetVal < 0 )
	{
		return ErrRet( ERR_DEVICE_FND_DISPLAY_STIRNG1 );
	}
	
	return SUCCESS;	
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      OnOffFND                                                 *
*                                                                              *
*  DESCRIPTION:       This program displays FND O, X						   *
*                                                                              *
*  INPUT PARAMETERS:  word wCtrlVal                                            *
*                                                                              *
*  RETURN/EXIT VALUE: 0 - Success                                              *
*                     (-) - Error                                              *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short OnOffFND( word wCtrlVal )
{
	short sRetVal = 0;
		
	sRetVal = ioctl( gfdFNDDevice, SEG_CONROL, &wCtrlVal );	
	
	if ( sRetVal < 0 )
	{
		return ErrRet( ERR_DEVICE_FND_ONOFF );
	}
	
	return SUCCESS;	
}	


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      OpenBuzz                                                 *
*                                                                              *
*  DESCRIPTION:       This program opens Buzz Device                           *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: 0 - Success                                              *
*                     (-) - Error                                              *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short OpenBuzz( void )
{
	gfdBuzzDevice = open( BUZZ_DEVICE, O_WRONLY );
	if ( gfdBuzzDevice < 0 )
	{
		DebugOut("buzz open error\n");
		return ErrRet( ERR_DEVICE_BUZZ_OPEN );
	}	
		
	return SUCCESS;	
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      CloseEEPROM                                              *
*                                                                              *
*  DESCRIPTION:       This program closes EEPROM Device Port				   *
*                     and saves Common Structure.                              *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: None                                                     *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
void CloseBuzz( void )
{
	close( gfdBuzzDevice );
}
	

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      Buzzer                                                   *
*                                                                              *
*  DESCRIPTION:       This program beeps 									   *
*                                                                              *
*  INPUT PARAMETERS:  word wCnt                                                *
*                     dword dwDelayTime                                        *
*                                                                              *
*  RETURN/EXIT VALUE: 0 - Success                                              *
*                     (-) - Error                                              *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short Buzzer( word wCnt, dword dwDelayTime )
{
	short sRetVal = 0;
	word wIndex = 0;

	for ( wIndex = 0; wIndex < wCnt; wIndex++ ) 
	{
#ifndef TEST_NO_BUZZER
		sRetVal = write( gfdBuzzDevice,  "1000", 50 );
#endif
		usleep( dwDelayTime );
	}
	
	if ( sRetVal < 0 )
	{
		return ErrRet( ERR_DEVICE_BUZZER );
	}
	
	return SUCCESS;	
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      WriteBuzz                                                *
*                                                                              *
*  DESCRIPTION:       This program writes Argument Data to Buzz                *
*                                                                              *
*  INPUT PARAMETERS:  char *pcVal                                              *
*                     int nVal                                                 *
*                                                                              *
*  RETURN/EXIT VALUE: 0 - Success                                              *
*                     (-) - Error                                              *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short WriteBuzz( char *pcVal, int nVal )
{
	short sRetVal = 0;

#ifndef TEST_NO_BUZZER	
	sRetVal = write( gfdBuzzDevice, pcVal, nVal );
#endif
	
	if ( sRetVal < 0 )
	{
		return ErrRet( ERR_DEVICE_BUZZ_WRITE );
	}
	
	return SUCCESS;	
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      OpenDipSwitch                                            *
*                                                                              *
*  DESCRIPTION:       This program opens DipSwitch Device Port			       *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: 0 - Success                                              *
*                     (-) - Error                                              *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short OpenDipSwitch( void )
{
	gfdDipSwitch = open( DIPSWTICH_DEVICE, O_RDONLY );
	if ( gfdDipSwitch < 0)
	{
		DebugOut("\n<OpenDipSwitch>Dipsw open error\n");
		return ErrRet( ERR_DEVICE_DIPSWITCH_OPEN );
	}			
	
    return SUCCESS;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      CloseDipSwitch                                           *
*                                                                              *
*  DESCRIPTION:       This program closes DipSwtich Device Port				   *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: None                                                     *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
void CloseDipSwitch( void )
{
	/*디바이스 Close */
	close( gfdDipSwitch );
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      ReadDipSwitch                                            *
*                                                                              *
*  DESCRIPTION:       This program reads Terminal Type  	   			       *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: byte - DipSwitch Read Data                               *
*                     (-) - Error                                              *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
byte ReadDipSwitch( void )
{
	short sRetVal = 0;
	byte bDipSwitchVal = 0;
	
	sRetVal = read( gfdDipSwitch, &bDipSwitchVal, 1 );	
	if ( sRetVal < 0 )
	{
		return ErrRet( ERR_DEVICE_DIPSWITCH_READ );
	}
	
	return bDipSwitchVal;	
}
	
/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      OpenEEPROM                                               *
*                                                                              *
*  DESCRIPTION:       This program opens EEPROM Device Port	   			       *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: 0 - Success                                              *
*                     (-) - Error                                              *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short OpenEEPROM( void )
{
	gfdEEPROM = open( EEPROM_DEVICE, O_RDWR );
	if( gfdEEPROM < 0 )
	{
		DebugOut( "app - i2c eeprom open error1\n" );
		return ErrRet( ERR_DEVICE_EEPROM_OPEN );
	}
	
	return SUCCESS;	
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      CloseEEPROM                                              *
*                                                                              *
*  DESCRIPTION:       This program closes EEPROM Device Port				   *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: None                                                     *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
void CloseEEPROM( void )
{
	close( gfdEEPROM );
}
	
/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      WriteEEPROM                                              *
*                                                                              *
*  DESCRIPTION:       This program writes Data to EEPROM					   *
*                                                                              *
*  INPUT PARAMETERS:  byte* bWriteData                                         *
*                                                                              *
*  RETURN/EXIT VALUE: 0 - Success                                              *
*                     (-) - Error                                              *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short WriteEEPROM( byte* bWriteData )
{
	short sRetVal = 0;

	int nIndex = 0;	
	byte acWriteData[3];

	memset( acWriteData, 0x00, sizeof(acWriteData) );
	
	ioctl( gfdEEPROM, I2C_SLAVE , 0x50);
	
	for ( nIndex = 0; nIndex < 9 ; nIndex++ )
	{
		acWriteData[0] = (unsigned char)(nIndex);
		acWriteData[1] =  bWriteData[nIndex];	
		sRetVal = write( gfdEEPROM, acWriteData , 2);		
		if( sRetVal < 0 )
		{
			DebugOut ("write eeprom failed (I/O error) [2]\n");
			return ErrRet( ERR_DEVICE_EEPROM_WRITE );
		}
	}
	   
	return SUCCESS;	
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      ReadEEPROM                                               *
*                                                                              *
*  DESCRIPTION:       This program reads Data From EEPROM					   *
*                                                                              *
*  INPUT PARAMETERS:  byte* bReadData                                          *
*                                                                              *
*  RETURN/EXIT VALUE: 0 - Success                                              *
*                     (-) - Error                                              *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short ReadEEPROM( byte* bReadData )
{
	short sRetVal = 0;
	
	byte acWriteData;
	byte acReadData[11];

	int nIndex = 0;

	memset(acReadData,0x00,sizeof(acReadData));
	ioctl( gfdEEPROM, I2C_SLAVE, 0x50 );

	for ( nIndex = 0; nIndex < 9 ; nIndex++ )
	{
		acWriteData = (unsigned char)(nIndex);
		sRetVal = write( gfdEEPROM, &acWriteData , 1 );
		
		if( sRetVal < 0 )
		{
			DebugOut ( "write eeprom failed (I/O error) [2]\n" );
			return ErrRet( ERR_DEVICE_EEPROM_WRITE );
		}
		
		sRetVal = read( gfdEEPROM, acReadData, 2 );
		if( sRetVal <= 0 )
		{
			DebugOut ( "read eeprom failed (I/O error) [2]\n" );		
			return ErrRet( ERR_DEVICE_EEPROM_READ );
		}
		
		bReadData[nIndex] = acReadData[0];	
	}
	   
	return SUCCESS;	
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      CheckWriteEEPROM                                         *
*                                                                              *
*  DESCRIPTION:       This program writes and read EEPROM Data				   *
*                                                                              *
*  INPUT PARAMETERS:  byte* bWriteData                                         *
*                     byte* bReadData                                          *
*                                                                              *
*  RETURN/EXIT VALUE: 0 - Success                                              *
*                     (-) - Error                                              *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short CheckWriteEEPROM( byte* bWriteData, byte* bReadData )
{
	byte abBuf[1025] = { 0, };
	
	if ( strlen(bWriteData) > 1025 )
	{
		return ErrRet( ERR_DEVICE_EEPROM_WRITE );
	}
		
	if ( OpenEEPROM() != SUCCESS )
	{
		DebugOut("app - i2c eeprom open error1\n");
		return ErrRet( ERR_DEVICE_EEPROM_OPEN );
	}

	memset( abBuf, 0x00, sizeof(abBuf) );	
	memcpy( abBuf, bWriteData, strlen(bWriteData) );
	if ( WriteEEPROM( abBuf ) != SUCCESS )
	{
		DebugOut( "=====<CheckWriteEEPROM>eepRom Write Fail=======\n" );
		CloseEEPROM();
		return ErrRet( ERR_DEVICE_EEPROM_WRITE );
	}
	
	memset( abBuf, 0x00, sizeof(abBuf) );	
	if ( ReadEEPROM(abBuf) != SUCCESS )
	{
		DebugOut( "==========<CheckWriteEEPROM>eepRom Read Fail==========\n" );
		CloseEEPROM();
		return ErrRet( ERR_DEVICE_EEPROM_READ );	
	}
	else
	{
		memcpy( bReadData, abBuf, strlen(bWriteData) );
		DebugOut( "=========eepRom Read Success==========\n" );
	}

	CloseEEPROM();
	
	return SUCCESS;		
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      MemoryCheck                                              *
*                                                                              *
*  DESCRIPTION:       This program checks Free Memory                          *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: long - Free Memory                                       *
*                     (-) - Error                                              *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
long MemoryCheck( void )
{
    struct statfs stStatFS;
    long lUsedBlocks = 0;
    long lUsedPercent = 0;
    long lFreePrecent = 0;
    long lFreeMemory = 0;
    long lMem = 0;
    char acDir[40];

    memset( &stStatFS, 0x00, sizeof(struct statfs) );
    
    strcpy( acDir, "/mnt/mtd8/" );    
    if ( statfs((const char*)acDir, &stStatFS ) !=0 )
    {
        DebugOut( " ===  statfs  === \n" );
        return ErrRet( ERR_DEVICE_STAT_FILESYSTEM );
    }

	if ( stStatFS.f_blocks > 0 )
	{
		lUsedBlocks = stStatFS.f_blocks - stStatFS.f_bfree;
		if ( lUsedBlocks == 0 )
		{
			lUsedPercent = 0;
		}
		else
		{
			lMem = (lUsedBlocks * 100.0) / (lUsedBlocks + stStatFS.f_bavail);
			lUsedPercent = (long)( lMem + 0.5 );
		}
		
		if ( stStatFS.f_bfree == 0 )
		{
			lFreePrecent = 0;
		}
		else
		{
			lMem = ((stStatFS.f_bavail) * 100.0) / (stStatFS.f_blocks);
			lFreePrecent = (long)( lMem + 0.5 );
		}
		
		lMem = (long)(stStatFS.f_bavail * (stStatFS.f_bsize/1024));
		lFreeMemory = (lMem / 1000);
	}
	
	return lFreeMemory;
}

void OLEDOn(void)
{
	OpenFND();
	OnOffFND(0x01);
 	CloseFND();
}

void OLEDOff(void)
{
	OpenFND();
	OnOffFND(0x00);
 	CloseFND();
}

void XLEDOn(void)
{
	OpenFND();
	OnOffFND(0x02);
 	CloseFND();
}

void XLEDOff(void)
{
	OpenFND();
	OnOffFND(0x00);
 	CloseFND();
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      DisplayCommUpDownMsg                                     *
*                                                                              *
*  DESCRIPTION:       This program displays Segment according Arguments		   *
*                                                                              *
*  INPUT PARAMETERS:  word wUpDown                                             *
*					  word wWorkGubunClass									   *		
*                                                                              *
*  RETURN/EXIT VALUE: 0 - Success                                              *
*                     (-) - Error                                              *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short DisplayCommUpDownMsg( word wUpDown, word wWorkGubunClass )
{	
	char achDisplay[7] = { 0, };
	
	memset( achDisplay, 0, sizeof(achDisplay) );	
	if ( wUpDown == 0 )
	{
		switch(wWorkGubunClass)
		{
			/* 
			 * 집계PC에서 다운로드 시작
			*/
			case 1 :    
				memcpy( achDisplay, DOWNLOAD_START_FROM_DCS,
					strlen(DOWNLOAD_START_FROM_DCS) );		
				break;
			/* 
			 * 하차단말기로 운영정보 다운로드
			*/
			case 2 :			
				memcpy( achDisplay, DOWNLOAD_SUBTERM_PARM,
					strlen(DOWNLOAD_START_FROM_DCS) );				
				break;	
			/* 
			 * 하차단말기로 음성정보 다운로드 시작
			*/					
			case 3 :									
				memcpy( achDisplay, FND_DOWNLOAD_START_SUBTERM_VOICE,
					strlen(FND_DOWNLOAD_START_SUBTERM_VOICE) );				
				break;				
			default :
				memcpy( achDisplay, FND_READY_MSG, strlen(FND_READY_MSG) );
				break;		
		}
	}
	else if ( wUpDown == 1 )
	{
		switch(wWorkGubunClass)
		{	
			/* 
			 * 운전자 프로그램
			*/
			case 1 :    
				memcpy( achDisplay, FND_DOWNLOAD_DRIVER_IMG, 
						strlen(FND_DOWNLOAD_DRIVER_IMG) );
				break;			
			/* 
			 * 하차 프로그램
			*/
			case 2 :
				memcpy( achDisplay, FND_DOWNLOAD_SUBTERM_IMG, 
						strlen(FND_DOWNLOAD_SUBTERM_IMG) );			
				break;				
			/* 
			 * 하차 음성 다운로드
			*/
			case 3 :
				memcpy( achDisplay, FND_DOWNLOAD_END_SUBTERM_VOICE, 
						strlen(FND_DOWNLOAD_END_SUBTERM_VOICE) );			
				break;
			default :
				memcpy( achDisplay, FND_READY_MSG, strlen(FND_READY_MSG) );
				break;
		}
	}
	else
	{
		memcpy( achDisplay, FND_READY_MSG, strlen(FND_READY_MSG) );
	}

	DisplayASCInDownFND( achDisplay );	
	
	return SUCCESS;
}

