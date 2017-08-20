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
*  PROGRAM ID :       trans_proc.h                                             *
*                                                                              *
*  DESCRIPTION:       승하차유형판단 및 요금을 계산하고 신규환승정보를         *
*                     생성한다.                                                *
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
* 2005/07/13 Solution Team  Boohyeon Jeon Initial Release                      *
*                                                                              *
*******************************************************************************/

#ifndef _TRANS_PROC_H_
#define _TRANS_PROC_H_

#define USER_TYPE					0
#define USER_CNT					1

short TransProc( TRANS_INFO *pstTransInfo );
dword GetBaseFare( byte bCardType, byte bUserType, word wTranspMethodCode );
dword GetHardCodedBaseFare( word wTranspMethodCode );
void CreateDisExtraTypeID( byte bCardType, byte bUserType,
	time_t tEntExtDtime, bool boolIsXfer, byte *abDisExtraTypeID );

#endif
