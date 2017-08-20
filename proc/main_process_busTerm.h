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
*  PROGRAM ID :       mainProcessBusTerm.c                                     *
*                                                                              *
*  DESCRIPTION:       MAIN 프로그램을 구동하기위한 기본정보 Load.              *
*                                                                              *
*  ENTRY POINT:     void MainProc( NEW_IMGNVOICE_RECV* pstNewImgNVoiceRecv )   *
*                   short CheckNewCriteriaInfoRecv( byte* bpRecvData,          *
*                                                   word wDataSize,            *
*							NEW_IMGNVOICE_RECV* pstNewImgNVoiceRecv )          *
*                   short ReceiptPrintingReq( byte* bpRecvData,                *
*                                             word wDataSize )                 *
*                   short ClearSharedCmdnData( byte bCmd )                     *
*                   short ClearAllSharedCmdnData( void )                       *
*                   short SetSharedCmdnDataLooping( byte bCmd, char* pcCmdData,*
*                                                   word wCmdDataSize )        *
*                   short SetSharedCmdnData( byte bCmd, char* pcCmdData,       *
*                                            word wCmdDataSize )               *
*                   short SetSharedDataforCmd( byte bCmd, char* pcCmdData,     *
*                                              word wCmdDataSize )             *
*                   short GetSharedCmd( byte* bCmd, char* pcCmdData,           *
*                                       word* wCmdDataSize )                   *
*                   short WriteGPSLog2Reconcile( void )                        *
*                   int CalculateTimeDif( struct timeval* stStartTime,         *
*                                         struct timeval* stEndTime )          *
*                                                                              *
*  INPUT FILES:     None                                                       *
*                                                                              *
*  OUTPUT FILES:    None                                                       *
*                                                                              *
*  SPECIAL LOGIC:   None                                                       *
*                                                                              *
********************************************************************************
*                         MODIFICATION LOG                                     *
*                                                                              *
*    DATE                SE NAME                      DESCRIPTION              *
* ---------- ---------------------------- ------------------------------------ *
* 2006/03/27 F/W Dev Team GwanYul Kim  Initial Release                         *
*                                                                              *
*******************************************************************************/

#ifndef _MAIN_PROCESS_BUSTERM_H_
#define _MAIN_PROCESS_BUSTERM_H_

#include "../system/bus_type.h"

#define VALID_CMD(bCmd) ((bCmd > '0') && (bCmd <= 'Z'))
#define VALID_CMD_SIZE(wCmdSize) ((wCmdSize > 0) && (wCmdSize <= 40))
	
/*******************************************************************************
*  Declaration of Module Functions                                             *
*******************************************************************************/
void MainProc( NEW_IMGNVOICE_RECV* pstNewImgNVoiceRecv );
short CheckNewCriteriaInfoRecv( byte* bpRecvData, word wDataSize, 
								NEW_IMGNVOICE_RECV* pstNewImgNVoiceRecv );
short ReceiptPrintingReq( byte* bpRecvData, word wDataSize );
short ClearSharedCmdnData( byte bCmd );
short ClearAllSharedCmdnData( void );
short SetSharedCmdnDataLooping( byte bCmd, char* pcCmdData, word wCmdDataSize );
short SetSharedCmdnData( byte bCmd, char* pcCmdData, word wCmdDataSize );
short SetSharedDataforCmd( byte bCmd, char* pcCmdData, word wCmdDataSize );
short GetSharedCmd( byte* bCmd, char* pcCmdData, word* wCmdDataSize );
short WriteGPSLog2Reconcile( void );
int CalculateTimeDif( struct timeval* stStartTime, struct timeval* stEndTime );
void CheckNDisplayDynamicInfo( void );

void SendKillAllProcSignal( void );

#endif

