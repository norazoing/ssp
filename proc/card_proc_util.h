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
*  PROGRAM ID :       card_proc_util.h                                         *
*                                                                              *
*  DESCRIPTION:       카드처리시 필요한 유틸성 기능들을 제공한다.              *
*                                                                              *
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
*    DATE                SE NAME                      DESCRIPTION              *
* ---------- ---------------------------- ------------------------------------ *
* 2006/01/11 F/W Dev Team Boohyeon Jeon   Initial Release                      *
*                                                                              *
*******************************************************************************/

#ifndef _CARD_PROC_UTIL_H_
#define _CARD_PROC_UTIL_H_

bool IsValidPrepayIssuer( byte *abCardNo );
bool IsValidPostpayCardNo( byte *abCardNo, byte bIssuerCode );
bool IsSamsungAmexCard( byte *abCardNo );
bool IsBCCardInvalidBIN( byte *abCardNo );
bool IsValidPostpayIssuer( byte *abCardNo );
bool IsValidIssuerValidPeriod( byte *abCardNo, byte *abExpiryDate );
bool IsValidMifPrepayIssueDate( byte *abCardNo, byte *abIssueDate );
bool IsValidSCPrepayIssueDate( byte *abCardNo, time_t tIssueDate );
short SearchCardErrLog( TRANS_INFO *pstTransInfo );
void InitCardErrLog( void );
void AddCardErrLog( short sResult, TRANS_INFO *pstTransInfo );
void DeleteCardErrLog( byte *abCardNo );
void PrintTransInfo( TRANS_INFO *pstTransInfo );
void InitCardNoLog( void );
void AddCardNoLog( byte *abCardNo );
bool IsExistCardNoLog( byte *abCardNo );
bool IsValidSCPurseInfo( SC_EF_PURSE_INFO *pstPurseInfo );
void InitWatch( void );
double StopWatch( void );
void PrintXferInfo( COMMON_XFER_DATA *pstCommonXferData );

#endif
