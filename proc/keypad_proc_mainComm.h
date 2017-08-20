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
*  PROGRAM ID :     keypad_proc_mainComm.c                                     *
*                                                                              *
*  DESCRIPTION:     This program comm. with Driver & Main Proc./ Comm Proc.    *
*                                                                              *
*  ENTRY POINT:     short GetImgFileApplyDatenVer( byte* pbFileVer,            *
						byte* pbFileApplyDate )                                *
*					short SetTermIDLeapPwd( byte* pbRecvBuf, word wRecvLen )   *
*					short ResetupReq( byte* pbRecvBuf, word wRecvLen )         *
*					short KpdSetTime( byte* pbRecvBuf, word wRecvLen )         *
*					short KpdStartStopReq( byte* pbRecvBuf, word wRecvLen )    *
*					short KpdExtra( byte* pbRecvBuf, word wRecvLen )           *
*					short CancelMultiEnt( byte* pbRecvBuf, word wRecvLen )     *
*					short KpdBLCheck( byte* pbRecvBuf, word wRecvLen )         *
*					short KpdPLCheck( byte* pbRecvBuf, word wRecvLen )         *
*					short MultiEnt( byte* pbRecvBuf, word wRecvLen )           *
*					short StationCorrectReq( byte* pbRecvBuf, word wRecvLen )  *
*					short CashEntReceiptPrintReq( byte* pbRecvBuf,             *
*                   	word wRecvLen )                                        *
*					short SubTermIDSetReq( byte* pbRecvBuf, word wRecvLen )    *
*					short ServerIPSetting( byte* pbRecvBuf, word wRecvLen )    *
*					short TermInfoPrintingReq2Main( void )                     *
*					short CtrlKpdSharedCmd( byte bCmd, char* pcCmdData,        *
*                   	word wCmdDataSize, word wTimeOut )                     *
*					void CheckLancardSignal( void )                            *
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
* ---------- ---------------------------- ------------------------------------ *
* 2005/08/13 Solution Team  Gwan Yul Kim  Initial Release                      *
*                                                                              *
*******************************************************************************/

#ifndef _KEYPAD_PROC_MAINCOMM_H
#define _KEYPAD_PROC_MAINCOMM_H

/*******************************************************************************
*  Inclusion of Header Files                                                   *
*******************************************************************************/
#include "../system/bus_type.h"
#include "../system/device_interface.h"
#include "keypad_proc.h"
#include "main_process_busTerm.h"
#include "blpl_proc.h"

/*******************************************************************************
*  Declaration of Header Files                                                 *
*******************************************************************************/
short GetImgFileApplyDatenVer( byte* pbFileVer, byte* pbFileApplyDate );
short SetTermIDLeapPwd( byte* pbRecvBuf, word wRecvLen );
short SetTermIDLeapPwd0118( byte *pbRecvBuf, word wRecvLen );
short SetMainTermID( byte *abRecvBuf, word wRecvLen );
short ResetupReq( byte* pbRecvBuf, word wRecvLen );
short KpdSetTime( byte* pbRecvBuf, word wRecvLen );
short KpdStartStopReq( byte* pbRecvBuf, word wRecvLen );
short KpdExtra( byte* pbRecvBuf, word wRecvLen );
short CancelMultiEnt( byte* pbRecvBuf, word wRecvLen );
short KpdBLCheck( byte* pbRecvBuf, word wRecvLen );
short KpdPLCheck( byte* pbRecvBuf, word wRecvLen );
short MultiEnt( byte* pbRecvBuf, word wRecvLen );
short CityTourTicketInput( byte* pbRecvBuf, word wRecvLen );
short StationCorrectReq( byte* pbRecvBuf, word wRecvLen );
short CashEntReceiptPrintReq( byte* pbRecvBuf, word wRecvLen );
short SubTermIDSetReq( byte* pbRecvBuf, word wRecvLen );
short ServerIPSetting( byte* pbRecvBuf, word wRecvLen );
short TermInfoPrintingReq2Main( void );
short CtrlKpdSharedCmd( byte bCmd, char* pcCmdData, word wCmdDataSize,
	int nTimeOutSec );
void CheckLancardSignal( void );

#endif

