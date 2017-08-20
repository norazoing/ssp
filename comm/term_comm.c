/*******************************************************************************
*                                                                             *
*                      KSCC - Bus Embeded System                               *
*                                                                              *
*            All rights reserved-> No part of this publication may be          *
*            reproduced, stored in a retrieval system or transmitted           *
*            in any form or by any means  -  electronic, mechanical,           *
*            photocopying, recording, or otherwise, without the prior          *
*            written permission of LG CNS.                                     *
*                                                                              *
********************************************************************************
*                                                                              *
*  PROGRAM ID :       term_comm.c                                              *
*                                                                              *
*  DESCRIPTION:       ½ÂÇÏÂ÷°£ÀÇ Åë½ÅÀ» Ã³¸®ÇÏ´Â ¸ÞÀÎÇÁ·Î¼¼½º                    *
*                                                                              *
*  ENTRY POINT:       CommProc()                                               *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: 0 - Success                                              *
*                     (-) - Error                                              *
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
* ---------- ----------------------------------------------------------------- *
* 2005/09/27 Solution Team  woolim        Initial Release                      *
* 2006/04/17 F/W Dev. Team  wangura       ÆÄÀÏºÐ¸® ¹× Ãß°¡ ±¸Á¶È­              *
*                                                                              *
*******************************************************************************/
/*  ½ÂÇÏÂ÷°£ÀÇ Æß¿þ¾î ´Ù¿î·Îµå ±¸¼º //////////////////////////////////////////// 
*																			   *	
* 	½ÅÆß¿þ¾î¿¡¼­´Â ¹ö½º ½ÂÇÏÂ÷°£ÀÇ Åë½ÅÇÁ·ÎÅäÄÝÀÇ º¯°æÀ¸·Î ÀÎÇØ ½ÂÇÏÂ÷°£ÀÇ     *
* 	ÇöÀç »ç¿ëÇÏ´Â ÇÁ·ÎÅäÄÝÀÌ ±¸ÀÎÁö ½ÅÀÎÁö ÆÇº°ÇÏ¿© ±¸ÇÁ·ÎÅäÄÝÀÎ °æ¿ì ½ÅÆß¿þ¾î *
*   ¸¦ ´Ù¿î·ÎµåÇÏ¿© ¾÷±×·¹ÀÌµå ÇÑ´Ù. ½ÅÇÁ·ÎÅäÄÝÀ» »ç¿ëÇÏ´Â °æ¿ì¿¡´Â ½ÂÇÏÂ÷°£ÀÇ *
*   Ã³¸®ÇÔ¼ö MainTermProc()/SubTermProc()ÀÌ ½ÇÇàµÇ¸é ¹öÀüÀÌ ³·À» °æ¿ì ´Ù¿î·Îµå *
* 	°¡ ´ç¿¬È÷ µÈ´Ù.                                                            *
*                                                                              *
*	Áö¿ø¹æ¹ý )                                                                 *
*	½ÂÂ÷±â Ã³¸®ÇÔ¼ö(MainTermProc)½ÃÀÛÀü¿¡ ÇÏÂ÷±â ±¸ÇÁ·ÎÅäÄÝ »ç¿ëÈ®ÀÎ ¹×        *
*	½ÅÇÁ·ÎÅäÄÝ Æß¿þ¾î·Î ´Ù¿î·ÎµåÇÏ´Â ÇÔ¼öÀÎ UpdateSubTermImgÇÔ¼ö¸¦             *
*	°¡Àå ¸ÕÀú ½ÇÇàÇÏ°Ô ÇÏ¿© Ã³¸®ÇÑ´Ù.                                          *
*                                                                              *
*	                                                                           *
*	UpdateSubTermImgÇÔ¼ö´Â ±¸ÇÁ·ÎÅäÄÝÀÇ ÇüÅÂ·Î ÇÏÂ÷±â ¹öÀüÃ¼Å© ¸í·É¾î¸¦ º¸³»°Ô *
*	µÈ´Ù.                                                                      *
*	( Âü°í.                                                                    *
*	   1) ±¸ÇÁ·ÎÅäÄÝÀ» »ç¿ëÇÏ´Â Æß¿þ¾î´Â ¹öÀüÃ¼Å©(V) ¸í·É¾î Àü¼Û½Ã 40 byte¸¦   *
*	      Àü¼ÛÇÏ±â µÇ´Âµ¥                                                      *
*          STX LEN LEN CMD DEVNO µ¥ÀÌÅÍ(30b) ETX CRC CRC CRC CRC               *
*	      µ¥ÀÌÅÍºÎºÐ¿¡´Â DEVNO(1b),CSAMID(4b),PSAMID(8b),½ÂÂ÷´Ü¸»±âID(9),      *
*         0x00(8b)·Î ±¸¼º                                                      *
*	                                                                           *
*       2) ½ÅÆß¿þ¾î¿¡¼­´Â UpdateSubTermImg³» ChkSubTermImgVerOldProtcol() ÇÔ¼ö *
*          ¿¡¼­ ¾Æ·¡¿Í °°ÀÌ ±¸ÇÁ·ÎÅäÄÝÀ» »ç¿ëÇÏ´Â °ÍÃ³·³ µ¿ÀÏÇÏ°Ô ÆÐÅ¶À» ±¸¼º  *
*          ÇÏµÇ µ¥ÀÌÅÍ¿¡ 0x00(8b)ºÎºÐ¿¡ ½ÂÂ÷±â¹öÀü(4b)À» ³Ö¾î¼­ Àü¼ÛÇÏ°Ô µÈ´Ù. *
*	      STX LEN LEN CMD DEVNO µ¥ÀÌÅÍ(30b) ETX CRC CRC CRC CRC                *
*	      µ¥ÀÌÅÍºÎºÐ¿¡´Â DEVNO(1b),CSAMID(4b),PSAMID(8b),½ÂÂ÷´Ü¸»±âID(9),      *
*         ½ÂÂ÷±â¹öÀü(4b),0x00(4b)·Î ±¸¼º.                                      *
*	)                                                                          *
*	                                                                           *
*	ÇÏÂ÷±â¿¡¼­´Â                                                               *
*	1)±¸ÇÁ·ÎÅäÄÝÀ» »ç¿ëÇÏ´Â Æß¿þ¾î´Â ÀÚ½ÅÀÇ ¹öÀü 4byte¸¦ ÀÀ´äÀ¸·Î Àü¼Û         *
*	2)½ÅÇÁ·ÎÅäÄÝÀ» »ç¿ëÇÏ´Â Æß¿þ¾î´Â ¹öÀü 4byteÀÌ¿Ü¿¡ "new"¶ó´Â 3byte¸¦        *
*	  Ãß°¡·Î Àü¼ÛÇÔÀ¸·Î¼­ ÀÚ½ÅÀÌ ½ÅÇÁ·ÎÅäÄÝÀ» »ç¿ëÇÏ´Â Æß¿þ¾îÀÓÀ» ½ÂÂ÷±â¿¡     *
*     ¾Ë¸®°Ô µÈ´Ù.                                                             *
*                                                                              *
*	                                                                           *
*	½ÂÂ÷±â°¡ ±¸ÇÁ·ÎÅäÄÝÀ» »ç¿ëÇÏ´Â Æß¿þ¾î·Î ÆÇ¸íÀÌ µÇ¸é ±¸ÇÁ·ÎÅäÄÝÀ» ÀÌ¿ëÇÏ¿©  *
*	½ÅÇÁ·ÎÅäÄÝÀ» »ç¿ëÇÏ´Â Æß¿þ¾î¸¦ Àü¼ÛÇÔÀ¸·Î¼­ ¾÷±×·¹ÀÌµå°¡ ¿Ï·á µÈ´Ù.        *
*                                                                              *
*	½ÂÂ÷±â°¡ ½ÅÇÁ·ÎÅäÄÝÀ» »ç¿ëÇÏ´Â Æß¿þ¾î·Î ÆÇ¸íµÇ¸é UpdateSubTermImgÇÔ¼ö¸¦    *
*   Á¾·áÇÏ°í ½ÂÂ÷±âÃ³¸®ÇÔ¼ö(MainTermProc)³»¿¡¼­ ¹öÀüÀ» ºñ±³ÇÏ¿© ¾÷±×·¹ÀÌµå°¡   *
*   µÇµµ·Ï Á¤»óÀûÀÎ ÇÁ·Î¼¼½º¸¦ ÅëÇØ Ã³¸®ÇÏµµ·Ï ÇÑ´Ù.                           *
*                                                                              *
*	¹Ý¸é ½ÂÂ÷±â°¡ ±¸ÇÁ·ÎÅäÄÝÀ» »ç¿ëÇÏ´Â Æß¿þ¾îÀÌ°í ÇÏÂ÷±â°¡ ½ÅÇÁ·ÎÅäÄÝÀ» »ç¿ë  *
*   ÇÏ´Â Æß¿þ¾îÀÎ°æ¿ì                                                          *
*	( ÃÊ±â¹èÆ÷½Ã ¹®Á¦°¡ ¹ß»ýÇÏ¿© ±¸ÇÁ·ÎÅäÄÝÀ» »ç¿ëÇÏ´Â 03xx´ëÀÇ Æß¿þ¾î¸¦       *
*	¹öÀü¸¸ 04xx·Î ÇØ¼­ ½ÂÂ÷±â¿¡ ¹èÆ÷ÇÑ °æ¿ì¿¡ ¹ß»ýÇÑ´Ù)                        *
*    ¹öÀüÃ¼Å© ¸í·É¾î¸¦ ½ÅÇÁ·ÎÅäÄÝÀ» »ç¿ëÇÏ´Â Æß¿þ¾î°¡ ¼ö½ÅÇÏ¸é ±¸ÇÁ·ÎÅäÄÝÀ»    *
*    »ç¿ëÇÏ´Â Æß¿þ¾î¿Í µ¿ÀÏÇÏ°Ô ÀÀ´äÇÒ ¼ö ÀÖµµ·Ï ½ÅÆß¿þ¾îÀÇ ÇÏÂ÷±â¿¡ ±¸ÇöÀÌ    *
*    µÇ¾îÀÖÀ¸¸ç ¹öÀüÀÌ ³·Àº °æ¿ì ±¸ÇÁ·ÎÅäÄÝÀ» »ç¿ëÇÏ´Â 04xxÆß¿þ¾î¸¦ ´Ù¿î·Îµå½Ã *
*    ¼ö½ÅÇÒ¼ö ÀÖ´Â ºÎºÐÀÌ ½ÅÆß¿þ¾î ±¸ÇöµÇ¾î ÀÖ¾î¾ß ÇÑ´Ù.                       *
*	                                                                           *
*                                                                              *
*	SendPkt/RecvPktÇÔ¼ö´Â ½ÂÇÏÂ÷±â°¡ °øÅëÀ¸·Î »ç¿ëÇÏ´Â ÇÔ¼ö·Î                  *
*	±¸ÇÁ·ÎÅäÄÝ¿¡ ¸Â°Ô SendPktÇÏ±âÀ§ÇØ¼­ ½ÂÂ÷±â¿¡¼­´Â boolIsOldProtocol Àü¿ªº¯¼ö*
*	¸¦ ¹Ì¸® ¼³Á¤ÇÏ¿© ±¸ÇÁ·ÎÅäÄÝ·Î ¼Û½ÅÇØ¾ßÇÔÀ» ¾Ë·ÁÁØ´Ù.                       *
*	                                                                           *
*	±¸ÇÁ·ÎÅäÄÝ¿¡ ¸Â°Ô RecvPktÇÏ±âÀ§ÇØ¼­	ÇÏÂ÷±â¿¡¼­´Â 1)2)ÀÇ Á¶°Ç¿¡ ¸¸Á·ÇÏ´Â    *
*   °æ¿ì ±¸ÇÁ·ÎÅäÄÝ·Î ¼ö½ÅÇÏ°Ô µÈ´Ù.                                           *
*	1) Cmd°¡ 'V'ÀÌ°í ¼ö½Åµ¥ÀÌÅÍ»çÀÌÁî°¡ 40byteÀÎ °æ¿ì - UpdateSubTermImg()¿¡¼­ *
*      ÇÏÂ÷ ¹öÀüÈ®ÀÎ½Ã                                                         *
*	2) Cmd°¡ 'D'ÀÎ °æ¿ì - UpdateSubTermImg()¿¡¼­ ±¸ÇÁ·ÎÅäÄÝ·Î Æß¿þ¾î ´Ù¿î·Îµå½Ã* 
*	                                                                           *
*	ÀÌ´Â ±¸/½ÅÇÁ·ÎÅäÄÝ »ç¿ë Æß¿þ¾î°£ÀÌ ¼Û¼ö½Å ÃÖ´ëÆÐÅ¶»çÀÌÁî°¡ ´Ù¸£±â ¶§¹®ÀÌ´Ù.*
*                                                                              *
*******************************************************************************/

/*******************************************************************************
*  Declaration of Header Files                                                 *
*******************************************************************************/

#include "dcs_comm.h"
#include "term_comm.h"
#include "term_comm_mainterm.h"
#include "term_comm_subterm.h"
#include "../proc/main.h"
#include "../proc/file_mgt.h"
#include "../proc/write_trans_data.h"
#include "../proc/blpl_proc.h"
#include "../system/bus_type.h"
#include "../system/device_interface.h"
#include "../proc/card_proc_util.h"

MAINSUB_COMM_USR_PACKET stSendPkt; /* Send Data */
MAINSUB_COMM_USR_PACKET stRecvPkt; /* Receviced Data */

SUB_VERSION_INFO_LOAD stSubVerInfo;/* SubTerminal Version Infomaiton */

/* Result of adding KeySet/IDCenter to mainterm PSAM */
//static MAINTERM_PSAM_ADD_RESULT stAddMainTermPSAM;

/* Result of adding KeySet/IDCenter to subterm PSAM */
SUBTERM_PSAM_ADD_RESULT  stSubTermPSAMAddResult;

/* ÇÏÂ÷±â°¡ ½ÂÂ÷±âÀÇ ¹öÀüÃ¼Å©¸í·É¿¡ ´ëÇØ ±¸ÇÁ·ÎÅäÄÝ·Î ÀÀ´äÀ» ÇØ¾ßÇÏ´ÂÁö ¿©ºÎ   */
bool boolIsRespVerNeedByOldProtocol = FALSE;
/* ÆÐÅ¶¼Û½Å/¼ö½Å½Ã ±¸ÇÁ·ÎÅäÄÝÀ» »ç¿ëÇÏ¿© º¸³»¾ßÇÏ´ÂÁö ¿©ºÎ */
bool boolIsOldProtocol;  

/* old protocol¿¡¼­ stSendPkt.abData´ë½Å »ç¿ëÇÏ±â À§ÇÑ buffer */
 byte abSendData[1024] = { 0, };  
/* old protocol¿¡¼­ stRecvPkt.abData´ë½Å »ç¿ëÇÏ±â À§ÇÑ buffer */
 byte abRecvData[1048] = { 0, };  

bool boolIsMainSubCommFileDownIng;
/* 
 * ½ÂÇÏÂ÷ Åë½ÅÇÁ·Î¼¼½º¿¡¼­ ÇöÀç ½ÇÇàµÇ°í ÀÖ´Â ¸í·É¾î ÀúÀå º¯¼ö
 */
byte bCurrCmd; 
/* 
 * ÇÏÂ÷´Ü¸»±â °¹¼ö 
 */
int nIndex = 0;  

/*******************************************************************************
*  Declaration of function prototype                                           *
*******************************************************************************/
short UpdateSubTermImg( void );
short IsCmdValidCheck( byte bCmd );
/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       CommProc                                                 *
*                                                                              *
*  DESCRIPTION:       CommProc is main part of processing communication between*
*                     mainterminal and subterminal                             *
*                                                                              *
*  INPUT PARAMETERS:  void                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: void                                                     *
*                                                                              *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:           Following function open rs422 communication channel      *
*                     , try channel test and then call main part according     *
*                     to the terminal type                                     *
*******************************************************************************/
void CommProc( void )
{
    short sRetVal = SUCCESS;
    int nErrCnt = 0;

	/* 
	 * ½ÂÇÏÂ÷Åë½Å Æ÷Æ® Open - RS422 
	 */
    while(1)
    {
    	LogTerm( "CommOpen \n" );
        sRetVal = CommOpen( DEV_MAINSUB ); /* Open Serial Channel */
        if ( sRetVal < 0 )
        {
            nErrCnt++;
            /* Try 3 times to open it */
            if ( nErrCnt == MAX_TRIAL_COUNT_OPEN_UART )
            {
                ErrProc( sRetVal);
            }
        }
        else
        {
            break;
        }
    }

    /* 
     * ´Ü¸»±â ½ÂÇÏ±¸ºÐ¿¡ µû¶ó Ã³¸® 
     */	
    if ( gboolIsMainTerm == TRUE ) 
    {
	    /* 
	     * ½ÂÂ÷±âÀÎ °æ¿ì ÇÏÂ÷±â°¡ 03xx´ëÀÇ ±¸¹öÀüÆß¿þ¾îÀÎÁö ¸ÕÀú È®ÀÎÇÏ°í 
	     * ±¸¹öÀüÀÏ °æ¿ì ´Ù¿î·Îµå·Î ÇÏÂ÷±â¸¦ ¾÷µ¥ÀÌÆ® ÇØÁÖ°í ÇÏÂ÷ Ã³¸®ÇÔ¼ö ½ÃÀÛ 
	     */	
		/* 
		 * ÇÏÂ÷±â ±¸¹öÀü È®ÀÎ ¹× ±¸¹öÀüÀÏ°æ¿ì ½Å¹öÀüÀ¸·Î ´Ù¿î·Îµå Ã³¸® 
		 */ 	     
		UpdateSubTermImg();

		/* 
		 * ½ÂÂ÷±â Ã³¸® ÇÔ¼ö ½ÃÀÛ
		 */
        MainTermProc(); 
    }
    else  
    {
	    /* 
	     * ÇÏÂ÷±âÀÎ °æ¿ì 
	     */          
        SubTermProc();  /* ÇÏÂ÷±â Ã³¸® ÇÔ¼ö ½ÃÀÛ*///////////////////////////////
    }
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       UpdateSubTermImg                                         *
*                                                                              *
*  DESCRIPTION:       ÇÏÂ÷±â ±¸¹öÀü È®ÀÎ ¹× ±¸¹öÀüÀÏ°æ¿ì ½Å¹öÀüÀ¸·Î ´Ù¿î·Îµå   *
*                     ¸¦ ½ÇÇàÇÑ´Ù.                                             *
*                                                                              *
*  INPUT PARAMETERS:  void                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ½ÇÇà¼º°ø                                       *
*                     SendPkt,RecvPkt ÇÔ¼öÀÇ ¿¡·¯ÄÚµå¸¦ ±×´ë·Î ¸®ÅÏ            *                      *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:           														   *
*                                                                              *
*******************************************************************************/
short UpdateSubTermImg( void )
{
    int nDevNo = 0;     /* sub Term No. */
    int nRetryCnt = 0;
    short sRetVal = SUCCESS;
    char achNewVerYn[3] = { 0, }; /* ÇÏÂ÷±â ½ÅÇÁ·ÎÅäÄÝ »ç¿ë¿©ºÎ */
	int  nSubIDInfoExistYN = 0;

	LogTerm( "UpdateSubTermImg \n" );
        
    /* 
     * ¿îÀüÀÚ Á¶ÀÛ±â¿¡ "ÀÛ¾÷Áß.." Ç¥½Ã 
     */    
    gpstSharedInfo->boolIsKpdLock = TRUE;

	if ( gpstSharedInfo->boolIsDriveNow == TRUE)
	{
		ctrl_event_info_write( "9001" );
	}
	
    /* 
     * SendPkt/RecvPkt ÇÔ¼ö³»¿¡¼­ ±¸ÇÁ·ÎÅäÄÝÀ» »ç¿ëÇÏ¿© ¼Û¼ö½ÅÇØ¾ß ÇÔÀ» ¾Ë·ÁÁÖ´Â
     * Flag ¼³Á¤ 
     */
    boolIsOldProtocol = TRUE;
	
	/* 
	 * ÇÏÂ÷´Ü¸»±â ¹øÈ£ ÃÊ±â°ª ¼³Á¤ 
	 */
    nDevNo = 1; 
    DebugOut( "gbSubTermCnt=>[%d]\n", gbSubTermCnt );

	nSubIDInfoExistYN = access( SUBTERM_ID_FILENAME, F_OK );  // subid.dat

    /* 
     * ±¸ÇÁ·ÎÅäÄÝÇü½Ä¿¡ ¸ÂÃç ÇÏÂ÷±â ¹öÁ¯ Ã¼Å© ¸í·É¾î ½ÇÇà  
     * - ÇÏÂ÷´Ü¸»±â °¹¼ö(gbSubTermCnt)¸¸Å­ ¹Ýº¹
     */
    while ( nDevNo <= gbSubTermCnt )
    {
    	if ( nSubIDInfoExistYN == 0 )
    	{
	        nRetryCnt = 3; /* Àç½Ãµµ È½¼ö */
    	}
		else
		{
			nRetryCnt = 2; /* Àç½Ãµµ È½¼ö */
		}

        while( nRetryCnt-- )
        {
#ifdef TEST_SYNC_FW        
            printf( "[%d] ¹ø ÇÏÂ÷±â ±¸ÇÁ·ÎÅäÄÝ»ç¿ë¿©ºÎ Ã¼Å© ½ÃÀÛ\n", nDevNo );
#endif
			usleep( 100000 );
            sRetVal = ChkSubTermImgVerOldProtcol( nDevNo, achNewVerYn );

            if ( sRetVal != SUCCESS )
            {
                printf( "±¸¹öÁ¯ Ã¼Å© Áß ¿À·ù- [%x]\n", sRetVal );
            }
            else
            {
                printf( "[%d] ¹ø ÇÏÂ÷±â ±¸¹öÁ¯ Ã¼Å© ¼º°ø\n",nDevNo );
                break;
            }
        }
        /* 
         * ÇÏÂ÷±â´Ü¸»±â ¹øÈ£ Áõ°¡
         */
        nDevNo++;
    }


    nDevNo = 1;

    while ( nDevNo <= gbSubTermCnt )
    {
        nRetryCnt = 5;

        /* 
         * ÇÏÂ÷±â°¡ ±¸ÇÁ·ÎÅäÄÝÀÌ¸é ¹«Á¶°Ç ½ÂÂ÷±â ½ÇÇàÀÌ¹ÌÁöÀÎ bus100
         * À» ÇÏÂ÷±â·Î ´Ù¿î·Îµå                      
         */             
        if ( achNewVerYn[nDevNo-1] == SUBTERM_USE_OLD_PROTOCOL )
		{
           while( nRetryCnt-- )
            {
                printf( "[%d] ¹ø ÇÏÂ÷±â ´Ù¿î·Îµå ½Ãµµ : %dÈ¸/ÃÖ´ë10È¸\n",
                          nDevNo, 5-nRetryCnt );
				/* 
				 * bus100¿¡ ³¯Â¥¿Í ¹öÀü(18byte)¸¦ ´õÇØ ÇÏÂ÷±â·Î ÇÁ·Î±×·¥ ´Ù¿î·Îµå 
				 */
                sRetVal = SendSubTermImgOldProtcol( nDevNo, BUS_EXECUTE_FILE );
                if ( sRetVal != SUCCESS )
                {
                    printf( "[%d]¹ø ÇÏÂ÷±â·Î ½ÅÇÁ·ÎÅäÄÝ »ç¿ë f/w ´Ù¿î·Îµå Áß ¿À·ù\n",nDevNo );
                    tcflush( nfdMSC, TCIOFLUSH );          
                    sleep(2);
                }
                else
                {
                    printf( "[%d]¹ø ÇÏÂ÷±â·Î ½ÅÇÁ·ÎÅäÄÝ »ç¿ë f/w ´Ù¿î·Îµå ¼º°ø\n",nDevNo );
                    tcflush( nfdMSC, TCIOFLUSH );             
                    if (  gbSubTermCnt > 1 )
                    {
			            /* 
			             * ÇÏÂ÷±â ÇÁ·Î±×·¥ ´Ù¿î·Îµå½Ã ´Ù¿î·Îµå ¼º°øÈÄ
			             * ºÎÆÃ½Ã°£ È®º¸¸¦ À§ÇØ 50ÃÊ°£ sleepÀ» ÁØ´Ù
			             * ÀÌÀ¯ - ½Ã°£À» ÁÖÁö ¾ÊÀ» °æ¿ì ´ÙÀ½ ´Ü¸»±âÀÇ 
			             *        ´Ù¿î·Îµå°¡ Á¤»óÀûÀ¸·Î µÇÁö ¾Ê´Â Çö»óÀÌ ¹ß»ý
			             */                         
                        sleep( 50 );
                    }
					else
					{
					   /* 
                        * ÇÏÂ÷±â°¡ 1´ëÀÎ°æ¿ì¿¡´Â 1´ëÀÇ ÇÏÂ÷±â°¡ ºÎÆÃÇÏ´Â ½Ã°£¸¸Å­
					    */
						sleep( 25 );
					}
                    break;
                }
            }
        }
        else
        {
            printf( "[%d] ¹ø ÇÏÂ÷±â ¹öÁ¯", nDevNo );
            PrintlnASC( ":", gpstSharedInfo->abSubVer[nDevNo-1], 4 );
        }
        /* 
         * ÇÏÂ÷±â´Ü¸»±â ¹øÈ£ Áõ°¡
         */
        nDevNo++;
    }


    /* 
     * SendPkt/RecvPkt ÇÔ¼ö³»¿¡¼­ ±¸ÇÁ·ÎÅäÄÝÀ» »ç¿ëÇÏ¿© ¼Û¼ö½ÅÇØ¾ß ÇÔÀ» ¾Ë·ÁÁÖ´Â
     * Flag ÇØÁ¦ 
     */

	 boolIsOldProtocol = FALSE;

	/* 
     * 0401 ÀÌÈÄ c_ex_pro.dat´Â Áý°è¿¡¼­ ½ÂÂ÷±â·Î Àü¼ÛÇÏÁö ¾Ê±â·Î åÇßÀ¸¹Ç·Î
     * ±âÁ¸ÀÇ c_ex_pro.dat°¡ ³²¾ÆÀÖÀ» °æ¿ì ÇÑ¹øÀº »èÁ¦ÇØÁÖ¾î¾ß ÇÔ.
	 */
	system("rm c_ex_pro.dat");

	return SUCCESS;
}




/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       SendPkt												   *
*                                                                              *
*  DESCRIPTION:       Àü¼Û±¸Á¶Ã¼ÀÇ µ¥ÀÌÅÍ¸¦ ÇÁ·ÎÅäÄÝ¿¡ ¸Â°Ô ÆÐÅ¶À» »ý¼ºÇÏ¿©    *
*                     Àü¼ÛÇÑ´Ù.												   *
*																			   *
*  INPUT PARAMETERS:  void 									                   *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ½ÇÇà ¼º°ø 	                                   *
* 																			   *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS: ½Å ÇÁ·ÎÅäÄÝ ÆÐÅ¶±¸Á¶´Â ¾Æ·¡¿Í °°´Ù.                                *
*			STX LEN LEN CMD DEVNO SEQ SEQ DATA(N byte) ETX CRC CRC CRC CRC     *
*           ±¸ ÇÁ·ÎÅäÄÝ ÆÐÅ¶±¸Á¶´Â ¾Æ·¡¿Í °°´Ù.                                *
*			STX LEN LEN CMD DEVNO SEQ SEQ DATA(N byte) ETX CRC CRC CRC CRC     *
*																			   *		
*          																	   *
*                                                                              *
*******************************************************************************/
short SendPkt( void )
{
    byte abSendBuffer[1100] = { 0, }; /* ¼Û½Åµ¥ÀÌÅÍ ¹öÆÛ */
    int nSendBufferLen = 0;           /* ¼Û½Åµ¥ÀÌÅÍ ¹öÆÛ ±æÀÌ */  
    dword dwCommCrc;                  /* CRC °ª */
    short sRetVal;
    /*
     * ±¸ ÇÁ·ÎÅäÄÝ·Î Àü¼Û(03xx ¹öÀü)
     */
   /* if ( ( boolIsOldProtocol == TRUE ) ||
         ( stSendPkt.nDataSize == 1 &&
           stSendPkt.bCmd == MAIN_SUB_COMM_SUB_TERM_IMG_VER ) ||
         ( stSendPkt.nDataSize == 17 &&
           stSendPkt.bCmd == MAIN_SUB_COMM_SUB_TERM_IMG_VER ) )
           */     
	         
    if ( boolIsOldProtocol == TRUE )	
    {

        DebugOut( "±¸ ÇÁ·ÎÅäÄÝ·Î send==\n" );

        nSendBufferLen = stSendPkt.nDataSize + 10;
		/* 1. STX - 1byte */
        abSendBuffer[0] = STX;   
		/* 2. µ¥ÀÌÅÍ±æÀÌ - 2byte */		
        abSendBuffer[1] = ((stSendPkt.nDataSize + 2) & 0xFF00)>>8; 
        abSendBuffer[2] = (stSendPkt.nDataSize + 2) & 0x00FF;
		/* 3. bCmd(¸í·É¾î)- 1byte */			
        abSendBuffer[3] = stSendPkt.bCmd;  
		/* 4. bDevNo(´Ü¸»±â¹øÈ£) - 1byte */		
        abSendBuffer[4] = stSendPkt.bDevNo;   

		/* ¼Û½ÅBuffer¿¡ µ¥ÀÌÅÍ Copy */
        if ( boolIsMainSubCommFileDownIng == TRUE ) 
        {
			/* 
			 * ÇÏÂ÷±âÇÁ·Î±×·¥ÆÄÀÏ ´Ù¿î·Îµå¸¦ ³ªÅ¸³»´Â FlagÀÌ ¼³Á¤ µÇ¾îÀÖ´Ù¸é 
			 * stSendPkt±¸Á¶Ã¼³»ÀÇ abData´ë½Å¿¡ abSendData±¸Á¶Ã¼¸¦ »ç¿ëÇÑ´Ù.
			 * ÀÌÀ¯ - ±¸ÇÁ·ÎÅäÄÝ°ú ÃÖ´ëÆÐÅ¶»çÀÌÁî°¡ Æ²¸®±â ¶§¹®¿¡ 
			 */        
            memcpy( &abSendBuffer[5], abSendData, stSendPkt.nDataSize );
        }
        else
        {
			/* 
			 * ÇÏÂ÷±âÇÁ·Î±×·¥ÆÄÀÏ ´Ù¿î·Îµå ÁßÀÌ ¾Æ´Ñ°æ¿ì´Â 
			 * ±×´ë·Î stSendPkt±¸Á¶Ã¼ ÀÌ¿ë
			 */
            memcpy( &abSendBuffer[5], stSendPkt.abData, stSendPkt.nDataSize );
        }

    }
   /*
    * ½Å ÇÁ·ÎÅäÄÝ·Î Àü¼Û(04xx ¹öÀü) 
    */
    else
    {
        nSendBufferLen = stSendPkt.nDataSize + 12;

		/* 1. STX - 1byte*/
        abSendBuffer[0] = STX; 
		/* 2. µ¥ÀÌÅÍ±æÀÌ - 2byte*/
        abSendBuffer[1] = ((stSendPkt.nDataSize) & 0xFF00)>>8; 
        abSendBuffer[2] = (stSendPkt.nDataSize ) & 0x00FF;
		/* 3. bCmd(¸í·É¾î)- 1byte*/		
        abSendBuffer[3] = stSendPkt.bCmd; 
		/* 4. bDevNo(´Ü¸»±â¹øÈ£) - 1byte */
        abSendBuffer[4] = stSendPkt.bDevNo; 

		/* 5. Sequence - 2byte	*/
        memcpy( &abSendBuffer[5], ( byte*)&stSendPkt.wSeqNo, 2);
        /*
		 * 6. µ¥ÀÌÅÍ¸¦ µ¥ÀÌÅÍ »çÀÌÁî¸¸Å­ ¼Û½Å¹öÆÛ¿¡ Copy         
 		 * µ¥ÀÌÅÍ°¡ ¾ø´Â °æ¿ì Áï, µ¥ÀÌÅÍ»çÀÌÁî°¡ 0ÀÎ°æ¿ì¿¡´Â ÇÁ·ÎÅäÄÝÀÇ µ¥ÀÌÅÍ
 		 * ¿µ¿ª¿¡ ¾Æ¹«°Íµµ ¾²Áö ¾Ê´Â´Ù.
		 */
        if ( stSendPkt.nDataSize != 0 )
        {
            memcpy( &abSendBuffer[7], stSendPkt.abData, stSendPkt.nDataSize );
        }
    }

	/* 6. ETX - 1byte	*/  
    abSendBuffer[nSendBufferLen-5] = ETX; 
	/* 7. ÆÐÅ¶ CRC - 4byte	*/
    dwCommCrc = MakeCRC32( abSendBuffer, nSendBufferLen-4 );
    memcpy( &abSendBuffer[nSendBufferLen-4], ( byte *)&dwCommCrc, 4 );

   /*
   	* ¼Û½Åµ¥ÀÌÅÍ ¹öÆÛÀÇ µ¥ÀÌÅÍ¸¦ Àü¼Û
   	*/
    sRetVal = CommSend( nfdMSC, abSendBuffer, nSendBufferLen );
	//PrintlnBCD("Send Data", abSendBuffer, nSendBufferLen);
    if ( sRetVal < 0 )
    {
       /*
       	* Àü¼Û½ÇÆÐ½Ã Àü¼Ûµ¥ÀÌÅÍ ±¸Á¶Ã¼ ÃÊ±âÈ­ 
       	*/
        memset( &stSendPkt, 0x00, sizeof(stSendPkt) );
        return ErrRet( sRetVal );
    }

    return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       RecvPkt												   *
*                                                                              *
*  DESCRIPTION:       Àü¼Û±¸Á¶Ã¼ÀÇ µ¥ÀÌÅÍ¸¦ ÇÁ·ÎÅäÄÝ¿¡ ¸Â°Ô ÆÐÅ¶À» »ý¼ºÇÏ¿©    *
*                     Àü¼ÛÇÑ´Ù.												   *
*																			   *
*  INPUT PARAMETERS:  int nTimeOut - ¼ö½Å TimeOut½Ã°£                          *
*                     int nDevNo   - ´Ü¸»±â ¹øÈ£                               *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ½ÇÇà ¼º°ø 	                                   *
* 					  ERR_MAINSUB_COMM_STX_IN_PKT - STX¿¡·¯      			   *
*                     ERR_MAINSUB_COMM_NOT_CURR_PROT_PKT 					   *
*                      - ½ÂÂ÷±â°¡ ¼Û½ÅÈÄ ¼ö½ÅÀ» ±â´Ù¸®´Â ´Ü¸»±â°¡ ¾Æ´Ñ Å¸      *
*                        ´Ü¸»±â ÆÐÅ¶¼ö½Å ¿¡·¯ 								   *
* 				      ERR_MAINSUB_COMM_NOT_MINE_PKT							   *
*                      - ÇÏÂ÷±â°¡ ÀÚ½ÅÀÇ ´Ü¸»±â¹øÈ£¿Í ´Ù¸¥ ÆÐÅ¶¼ö½Å ¿¡·¯       *
*                     ERR_MAINSUB_COMM_INVALID_LENGTH                          *
*                      - µ¥ÀÌÅÍ ±æÀÌ ¿¡·¯                                      *
*                     ERR_MAINSUB_COMM_SEQ_IN_PKT                              *
*                      - µ¥ÀÌÅÍ ½ÃÄö½º ¿¡·¯                                    *
*                 	  ERR_MAINSUB_COMM_ETX_IN_PKT                              *
*                      - ETX ¿¡·¯                                              *
* 					  ERR_MAINSUB_COMM_CRC_IN_PKT                              *
*                      - CRC ¿¡·¯                                              *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*                                                                              *
*******************************************************************************/
short RecvPkt( int nTimeOut, int nDevNo )
{

    byte abTmpBuff[25] = { 0, };    /* ÀÓ½Ã ¹öÆÛ  */
    byte abRecvBuff[1048]= { 0, };  /* ¼ö½Åµ¥ÀÌÅÍ ¹öÆÛ */
    int nRecvDataLen;				/* ¼ö½Åµ¥ÀÌÅÍ ±æÀÌ */
    int nRecvBuffLen;				/* ¼ö½Å¹öÆÛ ±æÀÌ */
    word wSeqNo;					/* ½ÃÄö½º ¹øÈ£ */
    dword dwCommCRC;				/* CRC°ª */
    short sRetVal = SUCCESS;
    short sRetVal1 = SUCCESS;

    int nEtxPoint = 5;              /* ÇÁ·ÎÅäÄÝ»ó¿¡¼­ ETXÀÇ À§Ä¡ - ³¡¿¡¼­ 5¹øÂ° */
	/* 
	 * ±¸¹öÀü Æß¿þ¾î(03xx)¿¡¼­´Â Åë½Å¿¡¼­ CRC, BCC°¡ È¥ÀçµÇ¾î »ç¿ëµÇ¾ú°í
	 * µû¶ó¼­ ¸ÞÀÎÇÁ·Î¼¼½º¿¡¼­ ºÎÆÃ½Ã ÃÊ±â¿¡ CRC¸¦ »ç¿ëÇÏ´Â ´Ü¸»±âÀÎ °æ¿ì¿¡´Â
	 * ¸í·É¾î 'C'¸¦ Á¦ÀÏ¸ÕÀú Àü¼ÛÇÏ¿© ÀÚ½ÅÀÌ CRC¸¦ ¾²´Â ´Ü¸»±âÀÎ°ÍÀ» »ó´ë¹æ¿¡°Ô
	 * ¾Ë·ÁÁÖ°Ô µÇ¾îÀÖ¾úÀ½.
	 * µû¶ó¼­ ½Å¹öÀü¿¡¼­´Â ÆÐÅ¶À» ¼ö½ÅÇÏ¿© 'C'ÀÏ °æ¿ì¿¡´Â boolIsCcmdRecv
	 * º¯¼ö¸¦ flagÀ¸·Î »ç¿ëÇÏ¿© ´Ù½ÃÇÑ¹ø µ¥ÀÌÅÍ¸¦ ¼ö½Å ÇÏµµ·Ï Ã³¸®ÇÏ¿´À½
	 */
    bool boolIsCcmdRecv = FALSE;    /* CRC Command('C') ¼ö½Å¿©ºÎ Flag */

   /* 
	* ¼ö½Åµ¥ÀÌÅÍ ±¸Á¶Ã¼ ÃÊ±âÈ­
	*/
    memset( &stRecvPkt, 0, sizeof( stRecvPkt ) );

   /* 
	* do ~ while( C ¸í·É¾î ¼ö½ÅÀÎ°æ¿ì ) ±¸Á¶ÀÇ Loop½ÃÀÛ 
	*/
    do
    {
        /*
         * 1. µ¥ÀÌÅÍ ÃÖÃÊ ¼ö½Å - 1byte//////////////////////////////////////////
         */
        DebugOut( "\n***************************************************" );
        sRetVal = CommRecv( nfdMSC, abTmpBuff, 1, nTimeOut );
        /*
         * µ¥ÀÌÅÍ ÃÖÃÊ ¼ö½Å ½ÇÆÐ½Ã 
         */
        if ( sRetVal < 0 )  
        {
            DebugOut( "\n    [STX  ¼ö½Å]½ÇÆÐ => [%d]\n", sRetVal );
            memset( &stRecvPkt, 0x00, sizeof( stRecvPkt ) );
            tcflush( nfdMSC, TCIOFLUSH );
            DebugOut( "***************************************************\n" );
            return ErrRet( sRetVal );
        }
        else
        {
            DebugOut( "\n    [STX  ¼ö½Å]¼º°ø[%02x]", abTmpBuff[0] );
        }

        /*
         * µ¥ÀÌÅÍ ÃÖ¼Ò ¼ö½Å½Ã STX ¼ö½Å¿©ºÎ °ËÁõ 
         */
        if ( abTmpBuff[0] != 0x02 )
        {
            memset( &stRecvPkt, 0x00, sizeof(stRecvPkt) );
            tcflush ( nfdMSC, TCIOFLUSH );
            DebugOut( "STX ¿¡·¯¹ß»ý [%02x]\n", abTmpBuff[0] );
            return ErrRet( ERR_MAINSUB_COMM_STX_IN_PKT );    
        }

        /*
         * 2. Length ¼ö½Å - 2 byte//////////////////////////////////////////////
         */
        sRetVal = CommRecv( nfdMSC, &abTmpBuff[1], 2, nTimeOut );

        if ( sRetVal < 0 )
        {
            DebugOut( "\n    sRetVal 2 [%d]", sRetVal );
            memset( &stRecvPkt, 0x00, sizeof(stRecvPkt) );
            tcflush ( nfdMSC, TCIOFLUSH );
            DebugOut( "\n    [LEN  ¼ö½Å]½ÇÆÐ\n" );
            DebugOut( "***************************************************\n" );
            return ErrRet( sRetVal );

        }
        else
        {
          DebugOut( "\n    [LEN  ¼ö½Å]¼º°ø[%02x][%02x]",
                    abTmpBuff[1], abTmpBuff[2] );
        }

        /*
         * 3. Command ¼ö½Å - 1 byte/////////////////////////////////////////////
         */
        sRetVal = CommRecv( nfdMSC, &abTmpBuff[3], 1, nTimeOut );	
        if ( sRetVal != SUCCESS )
        {
            DebugOut( "\n    sRetVal 3 [%d]", sRetVal );
            memset( &stRecvPkt, 0x00, sizeof(stRecvPkt) );
            tcflush( nfdMSC, TCIOFLUSH );
            DebugOut( "\n    [CMD  ¼ö½Å]½ÇÆÐ(NegativeValue)\n" );
            DebugOut( "***************************************************\n" );
            return ErrRet( sRetVal );
        }


        /*
         * Command ¼ö½Å ÈÄ °ËÁõ  
         */            
        sRetVal = IsCmdValidCheck( abTmpBuff[3] );
		if ( sRetVal != SUCCESS )
		{
            DebugOut( "\n    sRetVal 3 [%d]", sRetVal );
            DebugOut( "\n    [CMD  ¼ö½Å]½ÇÆÐ(ValidationFail)[ch:%c][hex:%x]\n",
                       abTmpBuff[3],abTmpBuff[3] );
            memset( &stRecvPkt, 0x00, sizeof(stRecvPkt) );
            tcflush( nfdMSC, TCIOFLUSH );
            DebugOut( "***********************************************\n" );
            return ErrRet( sRetVal );
        }
		
        /*
         * C command¿©ºÎ Ã¼Å© 
         */ 
		if ( abTmpBuff[3] == MAIN_SUB_COMM_CHK_CRC )
		{
			/*
			* C Command¸¦ ¹ÞÀº °æ¿ì ´Ù½Ã ÇÑ¹ø µ¥ÀÌÅÍ ¼ö½ÅÇÏµµ·Ï flagÃ³¸®
			*/
		    DebugOut( "\n    [CMD  ¼ö½Å]¼º°ø[%c]", abTmpBuff[3] );
            boolIsCcmdRecv = TRUE;
		}
		else
		{
			DebugOut( "\n    [CMD  ¼ö½Å]¼º°ø[%c]", abTmpBuff[3] );
		    boolIsCcmdRecv = FALSE;
		}
		
        /*
         * 4. ´Ü¸»±â ¹øÈ£(devNo) ¼ö½Å - 1byte///////////////////////////////////
         */         
        sRetVal = CommRecv( nfdMSC, &abTmpBuff[4], 1, nTimeOut );
		
        /*
         * ¼ö½ÅµÈ ´Ü¸»±â¹øÈ£°¡ 1,2,3ÀÌ ¾Æ´Ñ °æ¿ì¿¡µµ ¿¡·¯Ã³¸®
         */ 
        if ( sRetVal < 0  ||  abTmpBuff[4] < '1' || abTmpBuff[4] > '3' )
        {
            DebugOut( "\r\n sRetVal 4 [%d]", sRetVal );
            memset( &stRecvPkt, 0x00, sizeof(stRecvPkt) );
            tcflush( nfdMSC, TCIOFLUSH );
            DebugOut( "\n    [devNo¼ö½Å]½ÇÆÐ\n" );
            DebugOut( "***************************************************\n" );
            return ErrRet( sRetVal );
        }
		
		/* 
		 * ´Ü¸»±â ¹øÈ£ °ËÁõ
		 */
        if ( gboolIsMainTerm == TRUE  )
		{
		 	/* ½ÂÂ÷´Ü¸»±âÀÎ °æ¿ì - ³»°¡ ¼Û½ÅÇÑ ´Ü¸»±â¹øÈ£¿Í ºñ±³ */
            if( nDevNo != abTmpBuff[4]-'0' ) 
            {
	            LogTerm("[DevNo¼ö½Å]ÇÁ·ÎÅäÄÝ ¿¡·¯ [%02x]\n", abTmpBuff[4] );
	            DebugOut("[DevNo¼ö½Å]ÇÁ·ÎÅäÄÝ ¿¡·¯ [%02x]\n", abTmpBuff[4] );
	            sRetVal1 = ERR_MAINSUB_COMM_NOT_CURR_PROT_PKT;
            }
        }
        else
        {
       		/* ÇÏÂ÷´Ü¸»±âÀÎ °æ¿ì - ³» ´Ü¸»±â ¹øÈ£¿Í ºñ±³ */
 			if ( abTmpBuff[4]-'0' != gbSubTermNo ) 
 			{
 				DebugOut( "    [DevNo¼ö½Å]Å¸ ½ÅÈ£[%02x]\n", abTmpBuff[4] );
            	//tcflush( nfdMSC, TCIOFLUSH );
            	//DebugOut( "    [¼ö½Å °á°ú]Å¸ ÇÏÂ÷±â½ÅÈ£\n" );
            	//DebugOut( "***************************************************\n" );
            	sRetVal1 = ERR_MAINSUB_COMM_NOT_MINE_PKT;	
 			}
        }
        DebugOut( "\n    [DevNo¼ö½Å]¼º°ø[%c¹ø ´Ü¸»±â]", abTmpBuff[4] );
        

        /*
         * ±¸ ÇÁ·ÎÅäÄÝ·Î Æß¿þ¾î¸¦ ´Ù¿î·Îµå ÇÏ·Á´Â ¸í·É¾îÀÎÁö È®ÀÎÇÏ±â À§ÇØ
         * ±¸ ÇÁ·ÎÅäÄÝÀÇ DataºÎºÐ ±æÀÌ °è»ê¹æ¹ý´ë·Î ¼ö½Åµ¥ÀÌÅÍ ±æÀÌ¸¦ °è»ê.
         * ±¸ ÇÁ·ÎÅäÄÝÀº STX LEN LEN CMD DEVNO DATA ETX CRC CRC CRC CRC.
         * ±¸ SendPkt¿¡¼­ µ¥ÀÌÅÍ»çÀÌÁî¿¡ +2ÇÑ °ªÀ» ³Ö±â ¶§¹®¿¡ ¼ö½ÅÇÏ´Â ÂÊ¿¡¼­´Â
         * + 8À» ÇØ¼­ ÀüÃ¼ ¼ö½Åµ¥ÀÌÅÍ ±æÀÌ·Î °è»êÇÔ.
         * 
         */
        nRecvDataLen = ( ( abTmpBuff[1] << 8 ) | abTmpBuff[2] ) + 8;
        /*
         * ½ÂÂ÷±â¿¡¼­´Â ±¸ÇÁ·ÎÅäÄÝÀ» ÀÌ¿ëÇÏ¿© ¼ö½ÅÇØ¾ßÇÏ´ÂÁö ¿©ºÎ¸¦ 
         * boolIsOldProtocol·Î ±¸ºÐÇÏ°í
         * ÇÏÂ÷±â¿¡¼­´Â µ¥ÀÌÅÍ ±æÀÌ°¡ 40ÀÌ°í ¸í·ÉÀÌ MAIN_SUB_COMM_SUB_TERM_IMG_VER
         * ¶Ç´Â ¸í·ÉÀÌ MAIN_SUB_COMM_SUB_TERM_IMG_DOWN_OLDÀÎ °æ¿ì·Î ±¸ºÐÇÑ´Ù.
         */			
		if ( ( boolIsOldProtocol== TRUE && gboolIsMainTerm == TRUE ) ||
			 ( 
		       (( nRecvDataLen == 40 && abTmpBuff[3] == MAIN_SUB_COMM_SUB_TERM_IMG_VER ) ||
                ( abTmpBuff[3] == MAIN_SUB_COMM_SUB_TERM_IMG_DOWN_OLD ))
                &&
               ( gboolIsMainTerm == FALSE )
             )
           )
        {
            /* ½ÂÂ÷±â´Â ½Å¹öÀü ±¸¹öÀü ¸ðµÎ ÃÊ±â¿¡ ±¸ÇÁ·ÎÅäÄÝ·Î Æß¿þ¾î¹öÀü Ã¼Å©
             * ¸í·É¾î¸¦ ÇÏÂ÷±â·Î º¸³»´Âµ¥ ÇÏÂ÷±â¿¡¼­´Â  
			 * RecvPkt()»ç¿ë ¼ö½Å½Ã ¾Æ·¡ºÎºÐ¿¡¼­  TRUE·Î ¼ÂÆÃÇÏ°Ô µÈ´Ù.
			 * ½ÂÂ÷±â ÇÁ·Î±×·¥ ¹öÀüÀ» ¼ö½ÅÇÏ´Â ÇÔ¼öÀÎ SubRecvImgVer ½ÇÇà½Ã
			 * boolIsMainTermPreVer°¡ TRUEÀÎ °æ¿ì¿¡´Â ÇÏÂ÷±â¿Í ½ÂÂ÷±â ¹öÀüÀ» 
			 * ¼­·Î ºñ±³ÇÏ¿© ÇÏÂ÷±âÀÇ Rollback ½ÇÇàÀ» ÆÇ´ÜÇÏ°Ô µÈ´Ù.
			 */
            boolIsRespVerNeedByOldProtocol = TRUE;
			/* old protocol»ó¿¡¼­ÀÇ ÃÑ size */
            nRecvBuffLen = nRecvDataLen;    

	        /*
	         * ¼ö½Å¹öÆÛ ±æÀÌ °ËÁõ  
	         */
	        if ( nRecvBuffLen > MAX_PKT_SIZE_OLD )
	        {
	            DebugOut( "\r\n data size error [%d]", nRecvBuffLen );
	            memset( &stRecvPkt, 0x00, sizeof(stRecvPkt) );
	            tcflush ( nfdMSC, TCIOFLUSH );
	            DebugOut( "    [LENGTH ¼ö½Å]½ÇÆÐ\n" );
	            DebugOut( "***************************************************\n" );
	            return ErrRet( ERR_MAINSUB_COMM_INVALID_LENGTH );
	        }			
        }
        else
        {
            boolIsRespVerNeedByOldProtocol = FALSE;
		   /*
			* ½ÅÇÁ·ÎÅäÄÝÀº STX LEN LEN CMD DEVNO SEQ SEQ DATA ETX CRC CRC CRC CRC. 
			* DATAºÎºÐÀ» Á¦¿ÜÇÏ¸é 12byte°¡ ´õ ÀÖÀ¸¹Ç·Î ÀüÃ¼ ¹öÆÛ±æÀÌ´Â 
			* µ¥ÀÌÅÍ±æÀÌ + 12¸¦ ÇÏ°Ô µÈ´Ù.
			*/
            nRecvBuffLen = ( (abTmpBuff[1] << 8) | abTmpBuff[2] ) + 12;
		   /*
	         * ¼ö½Å¹öÆÛ ±æÀÌ °ËÁõ  
	         */
	        if ( nRecvBuffLen > MAX_PKT_SIZE )
	        {
	            DebugOut( "\r\n data size error [%d]", nRecvBuffLen );
	            memset( &stRecvPkt, 0x00, sizeof(stRecvPkt) );
	            tcflush ( nfdMSC, TCIOFLUSH );
	            DebugOut( "    [LENGTH ¼ö½Å]½ÇÆÐ\n" );
	            DebugOut( "***************************************************\n" );
	            return ErrRet( ERR_MAINSUB_COMM_INVALID_LENGTH );
	        }		   
        }

        memcpy( abRecvBuff, abTmpBuff, 5 );

        /*
         * 5. ³ª¸ÓÁö ÆÐÅ¶(SEQ/µ¥ÀÌÅÍ/ETX/CRC)¼ö½Å///////////////////////////////   
         */
        sRetVal = CommRecv( nfdMSC, &abRecvBuff[5], nRecvBuffLen-5, nTimeOut );
		/*
         * µ¥ÀÌÅÍ ºÎºÐ ¼ö½Å ÈÄ °ËÁõ    
         */
        /*
         * 1) ÀÚ½ÅÀÇ ÆÐÅ¶ÀÌ ¾Æ´Ñ °æ¿ì return
         *    ÀÚ½ÅÀÇ ÆÐÅ¶ÀÎÁö´Â DevNo¸¸ ¼ö½ÅÇÏ¿© °ËÁõÇÏ¸é ¾Ë¼ö ÀÖÁö¸¸
         *    ¹öÆÛ¿¡ µ¥ÀÌÅÍ¸¦ ³²±âÁö ¾Ê±â À§ÇØ ¸ðµç µ¥ÀÌÅÍ¸¦ ¼ö½ÅÇÑÈÄ¿¡
         *    ºñ±³¸¦ ÇØÁÖ°Ô µÈ´Ù.
         *    ±×·¡¼­ DevNo¼ö½Å½Ã ÀÚ±â ÆÐÅ¶ÀÌ ¾Æ´Ï¸é ½ÂÂ÷±â´Â 
         *    ERR_MAINSUB_COMM_NOT_CURR_PROT_PKT, ÇÏÂ÷±â´Â ERR_MAINSUB_COMM_NOT_MINE_PKT
         *    ¿¡·¯ÄÚµå¸¦ º¯¼ö¿¡ ÀúÀåÈÄ ¿©±â¼­ ºñ±³ÇÏ°Ô µÇ´Â °ÍÀÓ.    
         */
        if ( sRetVal1 == ERR_MAINSUB_COMM_NOT_CURR_PROT_PKT )
        {
        	/* ½ÂÂ÷´Ü¸»±âÀÎ °æ¿ì - ¼Û½ÅÇÑ ´Ü¸»±â¿¡¼­ ¼ö½ÅÇÒ ´Ü¸»±â°¡ ¾Æ´Ñ 
        	 * ´Ù¸¥ ´Ü¸»±â¿¡¼­ µ¥ÀÌÅÍ°¡ ¼ö½ÅµÈ °æ¿ì -ÇÁ·ÎÅäÄÝ ¿¡·¯ÄÚµå ¸®ÅÏ
        	 */ 
            printf( "    [DATA ¼ö½Å]ERR_MAINSUB_COMM_NOT_CURR_PROT_PKT\n" );
            memset( &stRecvPkt, 0x00, sizeof(stRecvPkt) );
            tcflush( nfdMSC, TCIOFLUSH );
            return ErrRet( sRetVal1 );
        } 
        else if ( sRetVal1 == ERR_MAINSUB_COMM_NOT_MINE_PKT )
        {
        	/* ÇÏÂ÷´Ü¸»±âÀÇ °æ¿ì - ¼ö½ÅÇÑÆÐÅ¶ÀÌ ÀÚ½ÅÀÇ ´Ü¸»±â¹øÈ£¿Í Æ²¸° °æ¿ì·Î 
        	 * ÀÚ½ÅÀÌ ¼ö½ÅÇÒ ÆÐÅ¶ÀÌ ¾Æ´Ñ°æ¿ì·Î - Not Mine Pkt ¿¡·¯ÄÚµå ¸®ÅÏ
        	 */             
            return ErrRet( sRetVal1 );
        }
		
        /*
         * 2) µ¥ÀÌÅÍ ºÎºÐ ¼ö½Å½Ã ¿¡·¯Ã³¸®
         */
        if ( sRetVal < 0 )
        {
            printf( "\r\n sRetVal 5 [%d]", sRetVal );
            memset( &stRecvPkt, 0x00, sizeof( stRecvPkt ) );
            tcflush ( nfdMSC, TCIOFLUSH );
            printf( "    [DATA ¼ö½Å]½ÇÆÐ\n" );
            printf( "***************************************************\n" );
            return ErrRet( sRetVal );
        }
        DebugOut( "\n    [DATA ¼ö½Å]¼º°ø-±æÀÌ:%d\n", nRecvBuffLen-5 );


        /*
         *  Sequence °ËÁõ - ½ÅÇÁ·ÎÅäÄÝÀÎ °æ¿ì¿¡¸¸ Ã¼Å© 
         */    
		if ( ( boolIsOldProtocol== TRUE && gboolIsMainTerm == TRUE ) ||
			 ( 
		       (( nRecvDataLen == 40 && abTmpBuff[3] == MAIN_SUB_COMM_SUB_TERM_IMG_VER ) ||
                ( abTmpBuff[3] == MAIN_SUB_COMM_SUB_TERM_IMG_DOWN_OLD ))
                &&
               ( gboolIsMainTerm == FALSE )
             )
           )
        {
            DebugOut( "    [SeqNo Ã¼Å©] Skip!\n" );
        }
        else
        {
            memcpy( ( byte*)&wSeqNo, &abRecvBuff[5], 2 );
            DebugOut( "[¼ö½Å SEQ] %d ¹ø ÆÐÅ¶", wSeqNo );
	        /*
	         *  Sequence Range Ã¼Å© 
	         */			
            if ( wSeqNo > 9999 )
            {
                DebugOut( "[SEQ °ËÁõ]SEQ ¿¡·¯¹ß»ý\n" );
                return ErrRet( ERR_MAINSUB_COMM_SEQ_IN_PKT );
            }
            else
            {
	        /*
	         *  Sequence¸¦ ¼ö½ÅÆÐÅ¶±¸Á¶Ã¼¿¡ ÀúÀå
	         */	            
                stRecvPkt.wSeqNo = wSeqNo;
            }

        }
        /*
         *  ETX Ã¼Å©
         */ 
        if ( abRecvBuff[nRecvBuffLen-nEtxPoint] != ETX )
        {
        	///printf( "etx 1- %x\n",  abRecvBuff[nRecvBuffLen-nEtxPoint]);
			///printf( "etx 2- %x\n",abRecvBuff[nRecvBuffLen-nEtxPoint+1]);
			///printf( "etx -1- %x\n",abRecvBuff[nRecvBuffLen-nEtxPoint-1]);
			
            memset( &stRecvPkt, 0x00, sizeof( stRecvPkt ) );
            DebugOut( "\n    [DATA °ËÁõ]ETX ¿¡·¯¹ß»ý \n" );
            tcflush ( nfdMSC, TCIOFLUSH );
            DebugOut( "***************************************************\n" );

            return ErrRet( ERR_MAINSUB_COMM_ETX_IN_PKT );    
        }

        /*
         * CRC Ã¼Å©
         */       
        dwCommCRC = MakeCRC32( abRecvBuff, nRecvBuffLen-4 );

        if ( memcmp( &abRecvBuff[nRecvBuffLen-4], ( byte*)&dwCommCRC, 4 ) != 0 )
        {
            int i = 0;

            for ( i = 0 ; i < nRecvBuffLen ; i++ )
            {
                DebugOut( "%02x ", abRecvBuff[i] );
            }

            DebugOutlnBCD( "    [DATA °ËÁõ]CRC ¿¡·¯¹ß»ý ¹ÞÀº CRC     ==> ",
                           &abRecvBuff[nRecvBuffLen-4], 4 );
            DebugOutlnBCD( "    [DATA °ËÁõ]CRC ¿¡·¯¹ß»ý °è»êÇÑ  CRC ==> ",
                           ( byte*)&dwCommCRC, 4 );
            memset( &stRecvPkt, 0x00, sizeof(stRecvPkt) );
            DebugOut( "\n    [DATA °ËÁõ]CRC ¿¡·¯¹ß»ý \n" );
            tcflush( nfdMSC, TCIOFLUSH );

            return ErrRet( ERR_MAINSUB_COMM_CRC_IN_PKT );
        }

    }
    while ( boolIsCcmdRecv == TRUE );

    /*
     * ¼ö½Å data ±¸Á¶Ã¼¿¡ °ª ÀúÀå
     */      
    stRecvPkt.bCmd = abRecvBuff[3];                /* ¸í·É¾î ÀúÀå */
    stRecvPkt.bDevNo = abRecvBuff[4] - '0';		   /* ´Ü¸»±â¹øÈ£ ÀúÀå */
    /*
     * µ¥ÀÌÅÍ»çÀÌÁî ¸¸Å­ µ¥ÀÌÅÍ¸¦ ¼ö½Å µ¥ÀÌÅÍ ±¸Á¶Ã¼¿¡ ÀúÀå
     */ 
    if ( ( boolIsOldProtocol == TRUE && gboolIsMainTerm == TRUE ) ||
         ( ( nRecvDataLen == 40 && abTmpBuff[3] == MAIN_SUB_COMM_SUB_TERM_IMG_VER ) &&
           ( gboolIsMainTerm == FALSE ) )
       )
    {
        /* 
         *±¸ÇÁ·ÎÅäÄÝÀº µ¥ÀÌÅÍ ¹öÆÛ±æÀÌ¿¡¼­ -10ÇØÁØ°ÍÀÌ ½ÇÁ¦ µ¥ÀÌÅÍ »çÀÌÁî
		 * STX(1) LEN(2) LEN(3) CMD(4) DEVNO(5) DATA ETX(6) CRC(7)CRC(8)CRC(9)CRC(10)
		 */
        stRecvPkt.nDataSize = nRecvDataLen - 10;

        if ( stRecvPkt.nDataSize != 0 )
        {
        	/* 
	         * 6 byteºÎÅÍ ¼ö½ÅµÈ µ¥ÀÌÅÍÀÌ¹Ç·Î 
			 */        
            memcpy( stRecvPkt.abData, &abRecvBuff[5], stRecvPkt.nDataSize );
        }
    }
	else if (( abTmpBuff[3] == MAIN_SUB_COMM_SUB_TERM_IMG_DOWN_OLD )&&
		     ( gboolIsMainTerm == FALSE )
		    )
	{
	   /* 
	    * ±¸ ÇÁ·ÎÅäÄÝÀ» ÀÌ¿ëÇÏ¿© ½ÂÂ÷±â¿¡¼­ Æß¿þ¾î´Ù¿î·Îµå¸¦ ÇÏ´Â °æ¿ì¿¡´Â
	    * ÃÖ´ëÆÐÅ¶»çÀÌÁîÀÇ Â÷ÀÌ°¡ ÀÖ¾î stRecvPkt.abData´ë½Å abRecvData¸¦ 
	    * »ç¿ëÇÑ´Ù.
	    */
		stRecvPkt.nDataSize = nRecvDataLen - 10;		
		if ( stRecvPkt.nDataSize != 0 )
        {
            memcpy( abRecvData, &abRecvBuff[5], stRecvPkt.nDataSize );
	    }   
    }
    else
    {
		/* 
         * ½Å ÇÁ·ÎÅäÄÝÀº µ¥ÀÌÅÍ ¹öÆÛ±æÀÌ¿¡¼­ -12 ÇØÁØ°ÍÀÌ ½ÇÁ¦ µ¥ÀÌÅÍ »çÀÌÁî
		 * STX(1) LEN(2) LEN(3) CMD(4) DEVNO(5) SEQ(6) SEQ(7) DATA ETX(8) CRC(9)CRC(10)CRC(11)CRC(12)
		 */    
        stRecvPkt.nDataSize = nRecvBuffLen - 12 ;

        if ( stRecvPkt.nDataSize != 0 )
        {
        	/* 
	         * 8 byteºÎÅÍ ¼ö½ÅµÈ µ¥ÀÌÅÍÀÌ¹Ç·Î 
			 */
            memcpy( stRecvPkt.abData, &abRecvBuff[7], stRecvPkt.nDataSize );
        }
    }

    return sRetVal;

}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       SendFile                             				       *
*                                                                              *
*  DESCRIPTION:       ÆÄÀÏÀ» Àü¼ÛÇÑ´Ù.   								       *
*                                                                              *
*  INPUT PARAMETERS:  int nDevNo- ´Ü¸»±â¹øÈ£ 				                   *
*                     char* pchFileName - ¼Û½Å´ë»ó ÆÄÀÏ¸í 				       *
* 																			   *
*  RETURN/EXIT VALUE: SUCCESS - ½ÇÇà ¼º°ø 	                                   *
*                     ERR_MAINSUB_COMM_FILE_NOT_FOUND_EOT_SEND                 *
*                      - EOTÀü¼ÛÁß ¿¡·¯ 		                        	   *
*                     ERR_MAINSUB_COMM_FILE_NOT_FOUND                          *
*                      - ¿äÃ»ÆÄÀÏ ¹ÌÁ¸Àç                                       *
*                     ERR_MAINSUB_COMM_PARM_DOWN_NAK                           *
*                      - ¼ö½ÅÆÐÅ¶Àü¼Û¿¡ ´ëÇÑ NAKÀÀ´ä                           *
*                     ERR_MAINSUB_COMM_FILE_NOT_FOUND_RECV                     *
*                      - EOTÀü¼Û ÈÄ ¼ö½Å¿¡·¯                                   *
*                                                                              *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:       	 ¼Û½ÅÇÒ ÆÄÀÏÀÌ Á¸ÀçÇÏÁö ¾Ê´Â °æ¿ì EOT Àü¼Û                 *
*                    ¼Û½ÅÇÒ ÆÄÀÏÀÌ Á¸ÀçÇÏ´Â °æ¿ì¿¡´Â ÆÄÀÏÀÇ ¸¶Áö¸· ÆÐÅ¶¿¡      *
*                    ¸¶Áö¸· ÆÐÅ¶ÀÇ ÀÇ¹Ì·Î EOT¸¦ Àü¼Û                           *
*                    °¢ ÆÐÅ¶ Àü¼Û¸¶´Ù ACK·Î¼­ ÆÐÅ¶Àü¼Û¿Ï·á¸¦ È®ÀÎÇÑ´Ù.         *
*																			   *
*******************************************************************************/
short SendFile( int nDevNo, char* pchFileName )
{
    short sRetVal = SUCCESS;
    int fdFile;
    bool boolIsFileEnd = FALSE;   /* ÆÄÀÏÀÇ ³¡À» ³ªÅ¸³»´Â Flag */         
	int nByte;					  /* ¼Û½ÅÆÄÀÏ¿¡¼­ ÀÐÀº Byte¼ö */
    word wSeqNo = 0;			  /* Sequence ¹øÈ£ */
	bool boolIsEOTUse = FALSE;
    usleep( 100000 );

	/*
	 * Àü¼ÛÇÒ ÆÄÀÏ ¿­±â
	 */
	fdFile = open( pchFileName, O_RDONLY, OPENMODE );

    if ( fdFile < 0 )
    {
        printf( "\r\n ÆÄÀÏ ¿ÀÇÂ ¾ÈµÊ\r\n" );
	   /*
	   	* TDÆÄÀÏ Àü¼ÛÀÌ ¾Æ´Ñ °æ¿ì¿¡´Â ÆÄÀÏ ¿ÀÇÂÀÌ ¾ÈµÇ¸é 
	   	*/
		if ( bCurrCmd != MAIN_SUB_COMM_GET_TRANS_FILE )
		{
			return -1;
		}
	   /*
	   	* Àü¼ÛÇÒ ÆÄÀÏÀÌ ¿ÀÇÂÀÌ ¾ÈµÇ´Â °æ¿ì¿¡´Â ÆÄÀÏÀÌ Á¸ÀçÇÏÁö ¾Ê´Â °æ¿ì·Î °£¼ö
	   	* ÆÄÀÏÀü¼ÛÀ» ¿äÃ»ÇÑ ÂÊ¿¡ EOT¸¦ º¸³»°Ô µÈ´Ù. 
	   	*/
   		/* Cmd(Ä¿¸Çµå) : ÇÏÂ÷±â¿¡¼­ ¼ö½ÅÇÑ Ä¿¸Çµå */
        stSendPkt.bCmd = bCurrCmd; 
		/* bDevNo(´Ü¸»±â¹øÈ£) : ´Ü¸»±â¹øÈ£ + ASCII 0  */ 
        stSendPkt.bDevNo = nDevNo + '0';
		/* wSeqNo(½ÃÄö½º ¹øÈ£) : 0  */ 
        stSendPkt.wSeqNo = 0;   
		/* nDataSize(µ¥ÀÌÅÍ»çÀÌÁî) : 1  */ 		
        stSendPkt.nDataSize = 1;
		/* abData(µ¥ÀÌÅÍ) : µ¥ÀÌÅÍºÎºÐ¿¡ EOT  */ 		
        stSendPkt.abData[0] = EOT;

		boolIsEOTUse = TRUE;
	   /*
	   	* EOTÀü¼Û 
	   	*/
        sRetVal = SendPkt();
        if ( sRetVal < 0 )
        {
            printf( "\r\n  ÆÄÀÏ Á¸ÀçÇÏÁö ¾Ê¾Æ EOT Send Áß Error \r\n" );
            sRetVal =  ERR_MAINSUB_COMM_FILE_NOT_FOUND_EOT_SEND;
            return ErrRet( sRetVal );
        }
	   /*
	   	* EOTÀü¼Û¿¡ ´ëÇÑ ÀÀ´ä¼ö½Å
	   	*/
        sRetVal = RecvPkt( 3000, nDevNo );
        if ( sRetVal < 0 )
        {
            printf( "\r\n  ÆÄÀÏ Á¸ÀçÇÏÁö ¾Ê¾Æ  EOT Send ÈÄ Recv Error \r\n" );
            sRetVal =  ERR_MAINSUB_COMM_FILE_NOT_FOUND_RECV;
            return ErrRet( sRetVal );
        }
	   /*
	   	* EOTÀü¼Û¿¡ ´ëÇÑ ÀÀ´äÀ¸·Î ACK ¶Ç´Â NAK ¼ö½Å 
	   	*/
        if ( ( stRecvPkt.bCmd == ACK ) && ( boolIsEOTUse == TRUE ) )
        {
            printf( "\r\n ÆÄÀÏ Á¸ÀçÇÏÁö ¾Ê¾Æ EOT SendÈÄ ACK ¼ö½Å¿Ï·á!\r\n" );
            sRetVal =  ERR_MAINSUB_COMM_FILE_NOT_FOUND;
            return ErrRet( sRetVal );
        }
        else if ( stRecvPkt.bCmd == NAK )
        {
            printf( "NAK ¼ö½Å\n" );
            sRetVal =  ERR_MAINSUB_COMM_PARM_DOWN_NAK;
            return ErrRet( sRetVal );
        }

    }
   /*
   	* ÆÄÀÏÀü¼Û Loop ½ÃÀÛ 
   	*/
    while( TRUE )
    {
    	printf( "." );						// Àü¼ÛÁßÀÓÀ» ÄÜ¼Ö¿¡ Ç¥½ÃÇÑ´Ù.
    	fflush( stdout );

		/*
		 * Àü¼ÛÇÒ ÆÄÀÏ ÃÖ´ë 1024byte±îÁö µ¥ÀÌÅÍ¿µ¿ª¿¡ ÀÐ±â  
		 */    
        nByte = read( fdFile, stSendPkt.abData, DOWN_DATA_SIZE );
	   /*
	   	* ÀÐ¾î µéÀÎ µ¥ÀÌÅÍ°¡ Á¸ÀçÇÏ´Â °æ¿ì ÆÐÅ¶ »ý¼º
	   	*/  
	   	if ( nByte > 0 )
        {
            /* Cmd(Ä¿¸Çµå) : ÇöÀç ´Ü¸»±â°¡ ½ÇÇàÁßÀÎ Ä¿¸Çµå */
            stSendPkt.bCmd = bCurrCmd; 
 			/* bDevNo(´Ü¸»±â¹øÈ£) : ´Ü¸»±â¹øÈ£ + ASCII 0   */			
            stSendPkt.bDevNo = nDevNo + '0'; 
			/* nDataSize(µ¥ÀÌÅÍ»çÀÌÁî) : ÆÄÀÏ¿¡¼­ ÀÐÀº byte  */ 					
            stSendPkt.nDataSize = nByte;     
			/* wSeqNo(½ÃÄö½º ¹øÈ£) : ÆÐÅ¶ÀÇ ½ÃÄö½º ¹øÈ£  */ 
            stSendPkt.wSeqNo = wSeqNo;        
        }
	   /*
	   	* ÀÐ¾î µéÀÎ µ¥ÀÌÅÍ°¡ 0ÀÎ °æ¿ì Áï, ÆÄÀÏÀÇ ³¡ÀÎ°æ¿ì EOT ÆÐÅ¶ »ý¼º
	   	*/  		
        else if ( nByte == 0 ) 
        {
            DebugOut( "È­ÀÏ³¡À¸·Î µé¾î¿À´Ù %d \n", nByte );
			/* Cmd(Ä¿¸Çµå) : ÇöÀç ´Ü¸»±â°¡ ½ÇÇàÁßÀÎ Ä¿¸Çµå */			
            stSendPkt.bCmd = bCurrCmd;
			/* bDevNo(´Ü¸»±â¹øÈ£) : ´Ü¸»±â¹øÈ£ + ASCII 0   */					
            stSendPkt.bDevNo = nDevNo + '0';
			/* nDataSize(µ¥ÀÌÅÍ»çÀÌÁî) : 1  */ 			
            stSendPkt.nDataSize = 1;
			/* wSeqNo(½ÃÄö½º ¹øÈ£) : ÆÐÅ¶ÀÇ ½ÃÄö½º ¹øÈ£  */ 		
            stSendPkt.wSeqNo = wSeqNo;
			/* abData[0] : µ¥ÀÌÅÍ¿¡ EOT¸¦ ³Ö¾î ¼ö½ÅÃø¿¡¼­ ÆÄÀÏ³¡À» Ç¥½Ã */ 
            stSendPkt.abData[0] = EOT;
		   /*
		   	* ÆÄÀÏ ³¡ Ç¥½Ã 
		   	*/ 			
            boolIsFileEnd = TRUE;
        }
        else
        {
            sRetVal = -1;
            break;
        }

        if ( boolIsFileEnd == TRUE )
        {
            DebugOut( "\n[SendPkt-EOTto %d¹ø ´Ü¸»±â]", nIndex + 1 );
        }
        else
        {
            DebugOut( "\n[SendPkt-to %d¹ø ´Ü¸»±â]", nIndex + 1 );
        }
        DebugOut( "[¼Û½Å SEQ] %d ¹ø ÆÐÅ¶", wSeqNo );

	   /*
	   	* ÆÐÅ¶ Àü¼Û 
	   	*/ 			
        sRetVal = SendPkt();
        if ( sRetVal < 0 )
        {
            break;
        }

	   /*
	   	* ÀÀ´äÆÐÅ¶ ¼ö½Å
	   	*/ 	
        sRetVal = RecvPkt( 3000, nDevNo );
        if ( boolIsFileEnd == TRUE )
        {
            DebugOut( "\n[RecvPkt-EOTÀÀ´ä-from %d¹ø ´Ü¸»±â]", nIndex+  1 );
        }
        else
        {
            DebugOut( "\n[RecvPkt-from %d¹ø ´Ü¸»±â]", nIndex + 1 );
        }
		
        if ( sRetVal < 0 )
        {
            DebugOut( "\r\n ÆÄÀÏ Àü¼ÛÁß ÀÀ´ä¾øÀ½[%d]\r\n", sRetVal );
            break;
        }
	   /*
	   	* ÆÄÀÏ Àü¼Û¿Ï·á 
	   	* - ÆÄÀÏÀÌ Àü¼Û¿Ï·á°¡ µÇ¾úÀ½À» ÆÇ´ÜÇÏ´Â Á¶°ÇÀ¸·Î´Â ÆÄÀÏ³¡¿¡ µµ´ÞÇÏ°í
	   	*   ACK°¡ ¼ö½ÅµÈ °æ¿ì¶ó¾ß ÇÑ´Ù. 
	   	*/ 
        if ( ( stRecvPkt.bCmd == ACK ) && ( boolIsFileEnd == TRUE) )
        {
            printf( "/\n" );					// Àü¼Û¿Ï·á¸¦ ÄÜ¼Ö¿¡ Ç¥½ÃÇÑ´Ù.
            sRetVal = SUCCESS;
            break;
        }
	   /*
	   	* ÆÄÀÏ Àü¼ÛÁß NAK ÀÀ´äÀ¸·Î Á¾·á
	   	*/ 		
        else if ( stRecvPkt.bCmd == NAK )
        {
            sRetVal = ERR_MAINSUB_COMM_PARM_DOWN_NAK;
            break;
        }
	   /*
	   	* ÆÐÅ¶ Sequence ¹øÈ£ Áõ°¡
	   	*/ 	
        wSeqNo++;
    }

    close( fdFile );
    return ErrRet( sRetVal );

}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       RecvFile                             				       *
*                                                                              *
*  DESCRIPTION:       ÆÄÀÏÀ» ¼ö½ÅÇÑ´Ù.   								       *
*                                                                              *
*  INPUT PARAMETERS:  int nDevNo- ´Ü¸»±â¹øÈ£ 				                   *
*                     char* pchFileName - ¼ö½ÅÆÐÅ¶À» ÀúÀåÇÒ ÆÄÀÏ¸í             *
* 																			   *
*  RETURN/EXIT VALUE: SUCCESS - ½ÇÇà ¼º°ø 	                                   *
*                     ERR_MAINSUB_COMM_SEQ_DURING_RECV_FIL                     *
*                      - ÆÄÀÏ¼ö½Å½Ã ½ÃÄö½º ¿¡·¯                         	   *
*                     ERR_MAINSUB_COMM_REQ_FILE_NOT_EXIST                      *
*                      - ¿äÃ»ÆÄÀÏ ¹ÌÁ¸Àç                                       *
* 																			   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:       	 ÆÄÀÏÀü¼Û¿äÃ»À» ¹ÞÀº ´Ü¸»±â¿¡¼­´Â ÆÄÀÏÀÌ ¾ø´Â °æ¿ì ÀÀ´äÀ»  *
*                    EOT·Î º¸³»°Ô µÇ°í ÆÄÀÏÀÇ ¸¶Áö¸· ÆÐÅ¶¿¡µµ EOT¸¦ º¸³»°Ô µÈ´Ù*
*					 Â÷ÀÌ´Â ÆÄÀÏÀü¼Û¿äÃ»ÈÄ Ã¹ ÀÀ´äÀÌ EOT°¡ ¿À¸é ¿äÃ»ÆÄÀÏÀÌ     *
*                    Á¸ÀçÇÏÁö ¾Ê´Â °ÍÀÌ¸ç Ã¹ ÀÀ´ä ÀÌÈÄ EOT°¡ ¿À¸é ÆÄÀÏÀü¼ÛÀÌ   *
*                    ¿Ï·áµÇ¾úÀ½À» ÀÇ¹ÌÇÑ´Ù.  								   *
*                    Á¤¸®ÇÏ¸é, 												   *
*                    ÆÄÀÏÀü¼ÛÀ» ¿äÃ»ÇÑ ´Ü¸»±â¿¡¼­´Â EOT°¡ ¼ö½ÅµÈ °æ¿ì¿¡´Â      *
*                    Ã¹ ÆÐÅ¶¼ö½Å¿¡ EOT°¡ ¿Â °æ¿ì- ¿äÃ»ÇÑ ÆÄÀÏÀÌ ¾ø´Ù´Â ÀÇ¹Ì    *
*                    Ã¹ ÆÐÅ¶¼ö½ÅÀÌÈÄ EOT°¡ ¿Â °æ¿ì- Àü¼ÛÀÌ ¿Ï·á µÇ¾ú´Ù´Â ÀÇ¹Ì  *
*																			   *
*******************************************************************************/
short RecvFile( int nDevNo, char* pchFileName )
{
    int fdFile;
    short sRetVal = SUCCESS;
    int nRecvCnt = 0;         /* ÆÐÅ¶¼ö½Å Count */
    word nPreSeqNo = 0;		  /* ÀÌÀü Sequence ¹øÈ£ */

   /*
    * ¼ö½ÅµÈ ÆÄÀÏÀ» ÀúÀåÇÒ ÆÄÀÏ »ý¼º/¿­±â
    */
    fdFile = open( pchFileName, O_WRONLY | O_CREAT | O_TRUNC, OPENMODE );

    if ( fdFile < 0 )
    {
    	printf( " %s file open fail\n", pchFileName);
        close( fdFile );
        return -1;
    }
   /*
    * ÆÐÅ¶¼ö½Å ¹× ÀúÀå Loop 
    */
    while( 1 )
    {
    	printf( "." );						// ¼ö½ÅÁßÀÓÀ» ÄÜ¼Ö¿¡ Ç¥½ÃÇÑ´Ù.
    	fflush( stdout );

	   /*
	    * ÆÐÅ¶¼ö½Å
	    */
        sRetVal = RecvPkt( 8000, nDevNo );

        if ( sRetVal < 0 )
        {
            printf( "/r/n file recv Error 1: [%x] /r/n", sRetVal );
            break;
        }
	   /*
	    * ÆÄÀÏ ÆÐÅ¶ÀÇ Sequence Check
	    * ÇöÀç ¼ö½Å½ÃÄö½º°¡ ÀÌÀü ÆÐÅ¶½ÃÄö½º + 1ÀÎÁö ¿©ºÎ, 0ÀÌ ¾Æ´ÑÁö Ã¼Å©
	    */
        if ( ( stRecvPkt.wSeqNo - nPreSeqNo != 1 ) && ( stRecvPkt.wSeqNo != 0 ) )
        {
             DebugOut( "ÆÐÅ¶ Sequence Error!! ¼ö½Å¿¹Á¤ÆÐÅ¶¹øÈ£ : %d ¹ø, ",
                       nPreSeqNo+1 );
             DebugOut( "½ÇÁ¦¼ö½ÅÆÐÅ¶¹øÈ£ : %d ¹ø ", stRecvPkt.wSeqNo );
             sRetVal = ERR_MAINSUB_COMM_SEQ_DURING_RECV_FILE;
             break;
        }

	   /* 
	    * ÆÐÅ¶¼ö½Å Count Áõ°¡ - Ã¹ ÆÐÅ¶ ¼ö½ÅÀ» ±¸ºÐÇÏ±â À§ÇØ
	    */
        nRecvCnt++;
	   /* 
	    * ÆÐÅ¶¼ö½Å½Ã ACKÀÀ´äÀ» À§ÇÑ µ¥ÀÌÅÍ »ý¼º 
	    */
        stSendPkt.bCmd = ACK;
        stSendPkt.bDevNo = nDevNo + '0';
        stSendPkt.nDataSize = 0;
        stSendPkt.wSeqNo = 0;
	   /* 
	    * ACKÀü¼Û 
	    */
        sRetVal = SendPkt();
        if ( sRetVal < 0 )
        {
            DebugOut( "/r/n SendPkt Error  : [2] /r/n" );
            break;
        }

	   /* 
	    *  EOT¼ö½Å¿©ºÎ Ã¼Å© 
	    */
        if ( ( stRecvPkt.nDataSize == 1 ) && ( stRecvPkt.abData[0] == EOT ) )
        {
            if ( nRecvCnt == 1 )  // YYYYMMDDHHMMSS.tmp
            {
                /* Ã¹ ÆÐÅ¶¼ö½ÅÀÎ °æ¿ì´Â EOTÀÇ ÀÇ¹Ì´Â ¿äÃ»ÆÄÀÏ ¾ø´Ù´Â ÀÀ´äÀÓ */
                DebugOut( "\r\n[ÆÄÀÏ]ÇÏÂ÷´Ü¸»±â¿¡ ¿äÃ»ÇÑ ÆÄÀÏÀÌ ¾ø½À´Ï´Ù.\r\n" );
                sRetVal = ERR_MAINSUB_COMM_REQ_FILE_NOT_EXIST;
            }
            else
            {
                /* Ã¹ ÆÐÅ¶¼ö½ÅÀÌ ¾Æ´Ñ°æ¿ì´Â  EOTÀÇ ÀÇ¹Ì´Â ÆÄÀÏÀü¼Û¿Ï·á */
                DebugOut( "\r\n[ÆÄÀÏ]Àü¼Û¿Ï·á\r\n" );
            }
            break;
        }
	    /* 
	     *  ÆÐÅ¶¼ö½Åµ¥ÀÌÅÍ ÆÄÀÏ¿¡ ÀúÀå 
	     */
        write( fdFile, stRecvPkt.abData, stRecvPkt.nDataSize );
		/* 
	     *  ´ÙÀ½ ÆÐÅ¶¼ö½Å½Ã ½ÃÄö½º Ã¼Å©¸¦ À§ÇØ ÇöÀç ½ÃÄö½º¸¦ ÀúÀå
	     */
        nPreSeqNo = stRecvPkt.wSeqNo;
    }

    close( fdFile );

	if ( sRetVal == SUCCESS )
		printf( "/" );						// Àü¼Û¿Ï·á¸¦ ÄÜ¼Ö¿¡ Ç¥½ÃÇÑ´Ù.
	else
		printf( "e\n" );

    return sRetVal;
}



/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       CreateACK                             				   *
*                                                                              *
*  DESCRIPTION:       ACKµ¥ÀÌÅÍ¸¦ »ý¼ºÇÑ´Ù.								       *
*                                                                              *
*  INPUT PARAMETERS:  int nDevNo- ´Ü¸»±â¹øÈ£ 				                   *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ½ÇÇà ¼º°ø 	                                   *
*																			   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:       	  ±¸ ÇÁ·ÎÅäÄÝ°ú ½Å ÇÁ·ÎÅäÄÝ¿¡ ´ëÇØ °¢°¢ ACK ÀÀ´äÀ» ÇÒ¼ö    *
*                     ÀÖµµ·Ï ±¸¼º. 											   *
*																			   *
* 					  04xx¹öÀüÀÇ ½ÅÇÁ·Î±×·¥Àº ½ÂÂ÷±â¿¡¼­ °¡Àå¸ÕÀú ÇÏÂ÷±âÀÇ     *
*                     ¹öÀüÀ» È®ÀÎÇÏ°í ½ÅÇÁ·Î±×·¥À¸·Î ¾÷µ¥ÀÌÆ®ÇÏ±âÀ§ÇØ          *
*                     UpdateSubTermImg()ÇÔ¼ö¸¦ ½ÇÇàÇÏ°Ô µÇ´Âµ¥ ÀÌ ÇÔ¼ö³»¿¡¼­   *
*                     ±¸ÇÁ·ÎÅäÄÝ·Î ¹öÀüÈ®ÀÎ¸í·É¾î('V')¸¦ ÇÏÂ÷±â·Î º¸³»°Ô µÈ´Ù. *
*                     ÀÌ°æ¿ì ÇÏÂ÷±â¿¡¼­´Â nDataSize°¡ 30 ÀÌ°í Cmd °¡           *
*                     MAIN_SUB_COMM_SUB_TERM_IMG_VERÀÎ°ÍÀ» º¸°í ÀÌ ÆÐÅ¶Àº      *
*                     UpdateSubTermImg()ÇÔ¼ö°¡ º¸³½°ÍÀ¸·Î ÆÇº° ±¸ÇÁ·ÎÅäÄÝ·Î    *
*                     ÀÀ´äÀ» ÇØÁÖ°Ô µÈ´Ù. 									   *
*                     														   *
*                     UpdateSubTermImg()ÇÔ¼ö°¡ ¹öÀüÈ®ÀÎ¸í·É¾î('V')¸¦ º¸³½°Ô    *
*                     ¾Æ´Ñ°æ¿ì¿¡´Â Áï, nDataSize°¡ 30ÀÌ ¾Æ´Ï°Å³ª ¸í·É¾î°¡      *
*                     MAIN_SUB_COMM_SUB_TERM_IMG_VER°¡ ¾Æ´Ñ°æ¿ì·Î              *
*                     ½ÅÇÁ·ÎÅäÄÝ·Î ACK¸¦ º¸³»°Ô µÈ´Ù.      					   *                                              * 
*                                                                              *
*******************************************************************************/
short CreateACK( int nDevNo )
{
    short sRetVal = SUCCESS;

	/* 
	 * ±¸ ÇÁ·ÎÅäÄÝ·Î ¼ö½ÅÇÑ °æ¿ì ±¸ÇÁ·ÎÅäÄÝ¿¡ ¸Â°Ô ÀÀ´äÆÐÅ¶±¸¼º 
	 */
    if ( boolIsRespVerNeedByOldProtocol == TRUE )
    {
   		/* Cmd(Ä¿¸Çµå) : ÇÏÂ÷±â¿¡¼­ ¼ö½ÅÇÑ Ä¿¸Çµå */
        stSendPkt.bCmd = stRecvPkt.bCmd;     
		/* bDevNo(´Ü¸»±â¹øÈ£) : ´Ü¸»±â¹øÈ£ + ASCII 0  */ 
        stSendPkt.bDevNo = stRecvPkt.bDevNo + '0';
		/* wSeqNo(½ÃÄö½º ¹øÈ£) : 0  */ 
        stSendPkt.wSeqNo = 0;
		/* nDataSize(µ¥ÀÌÅÍ»çÀÌÁî) : 1  */ 		
        stSendPkt.nDataSize = 1;
		/* abData(µ¥ÀÌÅÍ) : µ¥ÀÌÅÍºÎºÐ¿¡ ACK  */ 		
        stSendPkt.abData[0] = ACK;
		/* ÇÏÂ÷±â SendPkt()½Ã ±¸ÇÁ·ÎÅäÄÝ·Î »ç¿ëÇÏµµ·Ï Flag Set */ 		
		boolIsOldProtocol = TRUE;		
    }
    else
    {
		/* 
		 * ½Å ÇÁ·ÎÅäÄÝ·Î ¼ö½ÅÇÑ °æ¿ì ½ÅÇÁ·ÎÅäÄÝ¿¡ ¸Â°Ô ÀÀ´äÆÐÅ¶±¸¼º 
		 * ±¸ ÇÁ·ÎÅäÄÝ°ú ´Ù¸¥ Á¡Àº Ä¿¸ÇµåºÎºÐ¿¡ ACK¸¦ »ç¿ëÇÏ°í 
		 * ½ÇÁ¦ µ¥ÀÌÅÍ»çÀÌÁîµµ µ¥ÀÌÅÍÀÇ Å©±â¸¦ ³ªÅ¸³½´Ù´Â Á¡ÀÌ´Ù.
		 * ACKÀÀ´äÀÇ °æ¿ì µ¥ÀÌÅÍ°¡ ¾øÀ¸¹Ç·Î µ¥ÀÌÅÍ»çÀÌÁî°¡ 0 ÀÓ.
		 */   
	 	/* bCmd(Ä¿¸Çµå) : Ä¿¸ÇµåºÎºÐ¿¡ ACK  */ 
        stSendPkt.bCmd = ACK;
		/* bDevNo(´Ü¸»±â¹øÈ£) : ´Ü¸»±â¹øÈ£ + ASCII 0  */ 		
        stSendPkt.bDevNo = nDevNo + '0';
		/* wSeqNo(½ÃÄö½º ¹øÈ£) : 0  */ 		
        stSendPkt.wSeqNo = 0;
		/* nDataSize(µ¥ÀÌÅÍ»çÀÌÁî) : 0  */ 			
        stSendPkt.nDataSize = 0;
		/* ÇÏÂ÷±â SendPkt()½Ã ½ÅÇÁ·ÎÅäÄÝ·Î »ç¿ëÇÏµµ·Ï Flag Set */ 		
		boolIsOldProtocol = FALSE;	
    }

    return ErrRet( sRetVal );
}



/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       CreateNAK                                    		       *
*                                                                              *
*  DESCRIPTION:       NAKÀÀ´äµ¥ÀÌÅÍ¸¦ »ý¼ºÇÑ´Ù.				                   *
*                                                                              *
*  INPUT PARAMETERS:  int nDevNo - ´Ü¸»±â ¹øÈ£    		                       *
*                                                                              *
*  RETURN/EXIT VALUE: void                                                     *
*                                                                              *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
short CreateNAK( int nDevNo )
{
    short sRetVal = SUCCESS;
   /*
	* ¼Û½Å ÆÐÅ¶±¸Á¶Ã¼¿¡ °ª ¼³Á¤ 
	*/
	/* bCmd(Ä¿¸Çµå) : Ä¿¸ÇµåºÎºÐ¿¡ NAK  */ 	
    stSendPkt.bCmd = NAK; 	
	/* bDevNo(´Ü¸»±â¹øÈ£) : ´Ü¸»±â¹øÈ£ + ASCII 0 */    
    stSendPkt.bDevNo = nDevNo + '0';	
	/* Sequence No */
    stSendPkt.wSeqNo = 0;	
	/* Data Size */	
    stSendPkt.nDataSize = 0;			

    DebugOut( "create NAK \n" );

    return ErrRet( sRetVal ); 
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       CloseMainSubComm                                         *
*                                                                              *
*  DESCRIPTION:       ½ÂÇÏÂ÷°£ Åë½ÅÃ¤³ÎÀ» ´Ý´Â´Ù. 	    				       *
*                                                                              *
*  INPUT PARAMETERS:  void                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: void                                                     *
*                                                                              *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
short CloseMainSubComm( void )
{
    close( nfdMSC );
    return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       IsCmdValidCheck                            		       *
*                                                                              *
*  DESCRIPTION:       ÇÏÂ÷±â·ÎºÎÅÍ ¼ö½ÅÇÑ µ¥ÀÌÅÍÁß CommandºÎºÐÀ» °ËÁõÇÑ´Ù.     *
*                   														   *
*																			   *
*  INPUT PARAMETERS:  byte  bCmd - ¼ö½ÅÇÑ Command                              *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ½ÇÇà ¼º°ø 	                                   *
* 																			   *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*                                                                              *
*******************************************************************************/
short IsCmdValidCheck( byte bCmd )
{
    short sRetVal = SUCCESS;

    switch ( bCmd )
    {
        case MAIN_SUB_COMM_SUB_TERM_IMG_VER :
        case MAIN_SUB_COMM_SUB_TERM_IMG_DOWN :
		case MAIN_SUB_COMM_SUB_TERM_IMG_DOWN_OLD:
        case MAIN_SUB_COMM_GET_SUB_TERM_ID :
        case MAIN_SUB_COMM_SUB_TERM_PARM_DOWN :
        case MAIN_SUB_COMM_SUB_TERM_VOICE_VER :
        case MAIN_SUB_COMM_SUB_TERM_VOIC_DOWN :
        case MAIN_SUB_COMM_REGIST_KEYSET :
        case MAIN_SUB_COMM_POLLING :
        case MAIN_SUB_COMM_CHK_CRC :
        case MAIN_SUB_COMM_REQ_PL_SEARCH :
        case MAIN_SUB_COMM_REQ_BL_SEARCH :
        case MAIN_SUB_COMM_GET_TRANS_CONT :
        case MAIN_SUB_COMM_ASSGN_SUB_TERM_ID :
        case MAIN_SUB_COMM_GET_TRANS_FILE :
        case ACK :
        case NAK :
        case ETB : return ErrRet( sRetVal );
        default  : return -1;
    }
}




