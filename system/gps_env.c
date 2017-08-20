#include "gps_define.h"

#ifdef	_GPS_27_
	#include "../include/bus100.h"
#else
	#include <stdio.h>
	#include <stdlib.h>
	#include "../../common/type.h"
	#include "../system/bus_type.h"
	#include "gps.h"		
//	#include "gps_env.h"
	#include "gps_search.h"
	#include "gps_util.h"
#endif

/*****************************************************
* GPS ¼ö½Å RAW µ¥ÀÌÅ¸ ´Ü°èº° Ä«¿îÆ® º¯¼ö             *
******************************************************/
unsigned int g_nSelectFailCnt = 0;	// Select Fail Count
unsigned int g_nTimeOutCnt = 0;		// TimeOut	Count
unsigned int g_nCheckSumCnt = 0;		// CheckSum Error Count
unsigned int g_nInvalidCnt = 0;		// Invalid Count	'V'
unsigned int g_nAvailCnt = 0;		// Avail Count	'A'

unsigned int g_nPrevInvalidCnt =0;	// Previous invalid count
unsigned int g_nPrevAvailCnt =0;     // previous avail count
unsigned int g_nPrevTimeOutCnt =0;   // previous time out count
unsigned int g_nPrevCheckSumCnt = 0;	// previous checksum count

unsigned int g_n2PrevInvalidCnt =1;      // 
unsigned int g_n2PrevAvailCnt =1;        //
unsigned int g_n2PrevTimeOutCnt =0;      //

GPS_STATUS_CNT g_stGpsStatusCnt[MAXSTATIONCNT];
//////////////////////////////////////////////////////

/******************************************************************************
*                                                                             *
*    ÇÔ¼ö¸í : GpsPortOpen()                                                   *
*                                                                             *
*      ¼³¸í : [gps µ¥ÀÌÅ¸ ¼ö½ÅÀ» À§ÇÑ ½Ã¸®¾ó Æ÷Æ®¸¦ ÃÊ±âÈ­ ÇÑ´Ù.]             *
*                                                                             *
*    ÀÛ¼ºÀÚ : ±è Àç ÀÇ                                                        *
*                                                                             *
*    ÀÛ¼ºÀÏ : 2005.05.24                                                      *
*                                                                             *
*  ÆÄ¶ó¸ÞÅÍ : IN/OUT  PARAM NAME  TYPE      DESCRIPTION                       *
*             ------  ----------  --------  --------------------------------- *
*             IN      dev         char*     ½Ã¸®¾ó Æ÷Æ® ÀåÄ¡ ¸í               *
*             IN	  baudrate	  int		rs-232c Åë½Å¼Óµµ                  *
*			  IN	  lineread	  int		¶óÀÎ´ÜÀ§·Î ÀÐÀ½(1),		    	  *
								  char ´ÜÀ§·Î ÀÐÀ½(±× ¿Ü°ª)					  *
*    ¸®ÅÏ°ª : 0 ÀÌ»ó °ª - ¿ÀÇÂÇÑ ÀåÄ¡ ÇÚµé                                    *
*             -1        - Æ÷Æ® ¿­±â ½ÇÆÐ 								      *
*			  -2		- Æ÷Æ® ±âº» ¼Ó¼º È®ÀÎ ½ÇÆÐ							  *
*			  -3		- Æ÷Æ® ¼Ó¼º°ª ¼³Á¤ ½ÇÆÐ						          *
*			  -4		- ÆÄ¶ó¹ÌÅÍ dev °ª ¿À·ù								  *
*			  -5		- ÆÄ¶ó¹ÌÅÍ baudrate °ª ¿À·ù							  *
*  ÁÖÀÇ»çÇ× :                                                                 *
*                                                                             *
******************************************************************************/
int GpsPortOpen(char* dev, int baudrate, int lineread)
{
	int fd;
	struct termios newtio;
	struct termios oldtio;
	
//	int result = -1;
	speed_t	speed;

	if (dev == NULL || strlen(dev) < 1)	{
		printf("\n gpsPortOpen() Ã¹¹øÂ° ÆÄ¶ó¹ÌÅÍ dev °ª ¿À·ù!!\n");
		return -4;
	}
	
	if (baudrate < 9600 || baudrate > 921600)	{
		printf("\n gpsPortOpen() µÎ¹øÂ° ÆÄ¶ó¹ÌÅÍ baudrate °ª ¿À·ù!!\n");
		return -5;
	}
	
	if((fd = open(dev, O_RDONLY | O_NOCTTY)) < 0)
	{
		printf("\n");
		return -1;
	}
	
// CS8 : 8N1 (8bit, no parity, 1 stopbit) 
// CLOCAL : Local connection. ¸ðµ© Á¦¾î¸¦ ÇÏÁö ¾Ê´Â´Ù. 
// CREAD : ¹®ÀÚ ¼ö½ÅÀ» °¡´ÉÇÏ°Ô ÇÑ´Ù

	// Æ÷Æ® ±âº»¿¬°á ¿É¼ÇÀ» ¾ò´Â´Ù.
	// -1 : ¿¡·¯
	//  0 : ¼º°ø
	if (tcgetattr(fd, &oldtio) == -1)	{
		return -2;
	}

	bzero(&newtio, sizeof(newtio));	
/*
	switch(baudrate)
	{
		case 9600 :
			cfsetispeed(&newtio,B9600);
			cfsetospeed(&newtio,B9600);						
			break;
		case 115200 :
			cfsetispeed(&newtio,B115200);
			cfsetospeed(&newtio,B115200);			
			break;
		case 230400 :
			cfsetispeed(&newtio,B230400);
			cfsetospeed(&newtio,B230400);			
			break;
		case 460800 :
			cfsetispeed(&newtio,B460800);
			cfsetospeed(&newtio,B460800);			
			break;
		case 500000 :
			cfsetispeed(&newtio,B500000);
			cfsetospeed(&newtio,B500000);			
			break;
		case 576000 :
			cfsetispeed(&newtio,B576000);
			cfsetospeed(&newtio,B576000);			
			break;
		case 921600 :
			cfsetispeed(&newtio,B921600);
			cfsetospeed(&newtio,B921600);			
			break;
		default :
			cfsetispeed(&newtio,B9600);
			cfsetospeed(&newtio,B9600);						
			break;			
	}
*/
	switch(baudrate)	{
		case 9600:
			speed = B9600;	break;
		case 115200:
			speed = B115200;	break;
		default:
			speed = B9600;			
	}	
	
//  newtio.c_cflag &= ~PARENB;
//  newtio.c_cflag &= ~CSTOPB;
//  newtio.c_cflag &= ~CSIZE;
  newtio.c_cflag |= speed | CRTSCTS | CS8 | CLOCAL | CREAD;


	if (lineread == 1)	
		 newtio.c_lflag = ICANON;
	else
		 newtio.c_lflag = 0;
	

//  newtio.c_iflag |= (IGNPAR|IGNBRK);
//  newtio.c_iflag = IGNPAR | ICRNL;
  newtio.c_iflag = IGNPAR;

  newtio.c_oflag = 0;

	newtio.c_cc[VINTR]    = 0;     /* Ctrl-c */
	newtio.c_cc[VQUIT]    = 0;     /* Ctrl-\ */
	newtio.c_cc[VERASE]   = 0;     /* del */
	newtio.c_cc[VKILL]    = 0;     /* @ */
	newtio.c_cc[VEOF]     = 0;     /* Ctrl-d */
	newtio.c_cc[VTIME]    = 2;     /* inter-character timer unused */
	newtio.c_cc[VMIN]     = 1;     /* blocking read until 1 character arrives */
	newtio.c_cc[VSWTC]    = 0;     /* '\0' */
	newtio.c_cc[VSTART]   = 0;     /* Ctrl-q */
	newtio.c_cc[VSTOP]    = 0;     /* Ctrl-s */
	newtio.c_cc[VSUSP]    = 0;     /* Ctrl-z */
	newtio.c_cc[VEOL]     = 0;     /* '\0' */
	newtio.c_cc[VREPRINT] = 0;     /* Ctrl-r */
	newtio.c_cc[VDISCARD] = 0;     /* Ctrl-u */
	newtio.c_cc[VWERASE]  = 0;     /* Ctrl-w */
	newtio.c_cc[VLNEXT]   = 0;     /* Ctrl-v */
	newtio.c_cc[VEOL2]    = 0;     /* '\0' */


	tcflush(fd, TCIFLUSH);		
	if (tcsetattr(fd, TCSANOW , &newtio) == -1)	{
		return -3;		
	}
	
	return fd;
	
}


/******************************************************************************
*                                                                             *
*    ÇÔ¼ö¸í : DataStatusSet()                                           	  *
*                                                                             *
*      ¼³¸í : [GPS µ¥ÀÌÅ¸ ¼ö½Å »óÅÂ¿¡ µû¸¥ Ä«¿îÆ® °ª ¼¼ÆÃ]					  *
*                                                                             *
*    ÀÛ¼ºÀÚ : ±è Àç ÀÇ                                                        *
*                                                                             *
*    ÀÛ¼ºÀÏ : 2005.05.30                                                      *
*                                                                             *
*  ÆÄ¶ó¸ÞÅÍ : IN/OUT  PARAM NAME  TYPE            DESCRIPTION                 *
*             ------  ----------  --------        --------------------------- *
*			  IN	  param		  int  	          ¼ö½Å µ¥ÀÌÅ¸º° ±¸ºÐ°ª	      *
*    ¸®ÅÏ°ª : ¾øÀ½.				   						                      *
*  ÁÖÀÇ»çÇ× :                                                                 *
*                                                                             *
******************************************************************************/
void DataStatusSet(int param)
{
	switch (param)
	{
		case 1:
			g_nAvailCnt = g_nAvailCnt + 1;
			break;
		case 0:
			g_nInvalidCnt = g_nInvalidCnt + 1;
			break;
		case -1:
			g_nCheckSumCnt = g_nCheckSumCnt + 1;
			break;
		case -2:
			g_nTimeOutCnt = g_nTimeOutCnt + 1;
			close(g_gps_fd);
			g_gps_fd = 0x00;
			g_gps_fd = GpsPortOpen("/dev/ttyE1", 9600, 1);
			memset(&s_stRMC, 0x00, sizeof(RMC_DATA));
			break;
	}
}


/******************************************************************************
*                                                                             *
*    ÇÔ¼ö¸í : GpsDataLogRead()                                                *
*                                                                             *
*      ¼³¸í : [GPS ¼ö½Å µ¥ÀÌÅ¸ÀÇ ´Ü°èº° Ä«¿îÆ® ÀÐ±â]                          *
*                                                                             *
*    ÀÛ¼ºÀÚ : ±è Àç ÀÇ                                                        *
*                                                                             *
*    ÀÛ¼ºÀÏ : 2005.05.24                                                      *
*                                                                             *
*  ÆÄ¶ó¸ÞÅÍ : IN/OUT  PARAM NAME  TYPE      DESCRIPTION                       *
*             ------  ----------  --------  --------------------------------- *
*    ¸®ÅÏ°ª : ¾øÀ½                                   						  *
*  ÁÖÀÇ»çÇ× :                                                                 *
*                                                                             *
******************************************************************************/
void GpsDataLogRead()
{
	FILE* file = NULL;
	
	file = fopen(RAW_DATA_LOG, "rb");
	if (file != NULL)	{
		fread(&g_nSelectFailCnt, sizeof(unsigned int), 1, file);
		fread(&g_nTimeOutCnt, sizeof(unsigned int), 1, file);
		fread(&g_nCheckSumCnt, sizeof(unsigned int), 1, file);
		fread(&g_nInvalidCnt, sizeof(unsigned int), 1, file);
		fread(&g_nAvailCnt, sizeof(unsigned int), 1, file);
		fread(&g_nPrevInvalidCnt, sizeof(unsigned int), 1, file);  //2005-02-18 4:30¿ÀÈÄ
		fread(&g_nPrevAvailCnt, sizeof(unsigned int), 1, file); //2005-02-18 4:30¿ÀÈÄ
		fread(&g_nPrevTimeOutCnt,sizeof(unsigned int), 1, file);  //2005-02-18 4:44¿ÀÈÄ
				
		fclose(file);
 	}
 	else	{
 		g_nSelectFailCnt = 0;
 		g_nTimeOutCnt = 0;
 		g_nCheckSumCnt = 0;
 		g_nInvalidCnt = 0;
 		g_nAvailCnt = 0;
 		g_nPrevInvalidCnt = 0;   //2005-02-18 4:30¿ÀÈÄ
 		g_nPrevAvailCnt =0;  //2005-02-18 4:30¿ÀÈÄ
 		g_nPrevTimeOutCnt = 0; // 2005-02-18 4:45¿ÀÈÄ
 		
 	}
	
}

// GPS ¼ö½Å µ¥ÀÌÅ¸ÀÇ ´Ü°èº° Ä«¿îÆ® ÀúÀå
void GpsDataLogWrite()
{
	FILE* file = NULL;
	
	file = fopen(RAW_DATA_LOG, "wb+");
	if (file != NULL)	{
		fwrite(&g_nSelectFailCnt, sizeof(unsigned int), 1, file);
		fwrite(&g_nTimeOutCnt, sizeof(unsigned int), 1, file);
		fwrite(&g_nCheckSumCnt, sizeof(unsigned int), 1, file);
		fwrite(&g_nInvalidCnt, sizeof(unsigned int), 1, file);
		fwrite(&g_nAvailCnt, sizeof(unsigned int), 1, file);
		fwrite(&g_nPrevInvalidCnt,sizeof(unsigned int),1,file);  //2005-02-18 4:30¿ÀÈÄ
		fwrite(&g_nPrevAvailCnt,sizeof(unsigned int),1,file);  //2005-02-18 4:30¿ÀÈÄ
		fwrite(&g_nPrevTimeOutCnt,sizeof(unsigned int),1,file); //2005-02-18 4:43¿ÀÈÄ
		fflush(file);
		fclose(file);
 	}
}

void GlobalParameterInit(void)
{
	// Àü¿ªº¯¼ö ÃÊ±âÈ­ ÀÌ°Å ¾ÈÇÏ¸é.. Àý´ë ¾ÈµÊ.
	g_nFirstRun =0;
	
	g_nSelectFailCnt = 0;	// Select Fail Count
	g_nTimeOutCnt = 0;	// TimeOut	Count
	g_nCheckSumCnt = 0;	// CheckSum Error Count
	g_nInvalidCnt = 0;	// Invalid Count	'V'
	g_nAvailCnt = 0;		// Avail Count	'A'

	g_nPrevInvalidCnt =0;	// Previous invalid count
	g_nPrevAvailCnt =0;         // previous avail count
	g_nPrevTimeOutCnt =0;       // previous time out counter


	nMoveDis = 0;	// ¿îÇà½ÃÀÛ ~ Á¾·á±îÁö ÀÌµ¿ÇÑ °Å¸®
	// Á¤·ùÀåÅë°ú½Ã ÃÖ¼Ò°Å¸® ´Ü¼øÈ÷ ·Î±×¿¡ ±â·ÏÇÏ±â À§ÇØ »ç¿ëÇÑ´Ù.
	g_nPassMinDis = 0;

	g_nGpsExceptCnt = 0;	// ºñ Á¤»ó¿îÇà Á¤·ùÀå ÃÑ ¼ö

	// Á¤·ùÀåº° GPS »óÅÂ ¼ö½Å Ä«¿îÆ® º¯¼ö ÃÊ±âÈ­
	memset(&g_stGpsStatusCnt, 0X00, sizeof(GPS_STATUS_CNT)*MAXSTATIONCNT);

	// ¸ÞåÀÎÂÊ¿¡¼­ Á¤ÀÇµÇ¸é »ç¿ë°¡´É
	//gpstSharedInfo->boolIsValidGPSSearch = TRUE;
}

