
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
*  PROGRAM ID :       sc_read.c                                                *
*                                                                              *
*  DESCRIPTION:       This program reads Smart Card, checks validation of card *
*                     and saves Common Structure.                              *
*                                                                              *
*  ENTRY POINT:       SCRead()               ** optional **                    *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: 0 - Success                                              *
*                     (-) - Error                                              *
*                                                                              *
*  INPUT FILES:       Issure Info Files                                        *
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
* 2005/07/13 Solution Team  ByungYong Jo  Initial Release                      *
*                                                                              *
*******************************************************************************/
#ifndef _BLPL_PROC_H_
#define _BLPL_PROC_H_

/*******************************************************************************
*  Inclusion of Header Files                                                   *
*******************************************************************************/
#include "../system/bus_type.h"

#define MIF_PREPAY_START_ALIAS			1
#define MIF_PREPAY_END_ALIAS			30000000
#define POSTPAY_START_ALIAS				30000001
#define POSTPAY_END_ALIAS				90000000
#define SC_PREPAY_START_ALIAS			90000001
#define SC_PREPAY_END_ALIAS				113558400



/*******************************************************************************
*  Declaration of Function Prototype                                           *
*******************************************************************************/
short BLPLMerge(void);
short SearchBLinBus(byte *abCardNo, byte *ASCPrefix, dword dwCardNum,
	byte *bBLResult);
short SearchPLinBus(byte *abCardNo, dword dwAliasNo, byte *bResult);
short ReadBLPLFileCRC( byte* abBLCRC, 
	                   byte* abMifPrepayPLCRC, 
	                   byte* abPostpayPLCRC,
	                   byte* abAICRC );

#endif
