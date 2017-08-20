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
*  PROGRAM ID :       main.h                                                   *
*                                                                              *
*  DESCRIPTION:       This program includes function declarations              *
*                                                                              *
*  ENTRY POINT:       None               ** Mandotary **                       *
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
* 2005/09/04 Solution Team  Gwan Yul Kim  Initial Release                      *
*                                                                              *
*******************************************************************************/

#ifndef _MAIN_H
#define _MAIN_H

/*******************************************************************************
*  Inclusion of Header Files                                                   *
*******************************************************************************/
/*******************************************************************************
*  Inclusion of Header Files                                                   *
*******************************************************************************/
#include "../system/bus_type.h"

/*******************************************************************************
*  Declaration of Global Variables                                             *
*******************************************************************************/
extern PROC_SHARED_INFO *gpstSharedInfo;	// 공유메모리 포인터
extern byte gbSubTermCnt;				// 하차단말기 갯수
extern MYTERM_INFO gstMyTermInfo;			// 프로그램이 구동되는 Terminal 정보
extern pthread_t nCommGPSThreadID;

#endif
