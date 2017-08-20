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
*  PROGRAM ID :       main_Load_CriteriaInfo.H                                 *
*                                                                              *
*  DESCRIPTION:       MAIN 프로그램을 구동하기위한 기본정보 Load.              *
*                                                                              *
*  ENTRY POINT:     short OperParmNBasicInfo (void)                            *
*                   short VoiceVerLoad( void )								   *
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

#ifndef _MAIN_LOADCRITERIAINFO_H_
#define _MAIN_LOADCRITERIAINFO_H_

/*******************************************************************************
*  Inclusion of User Header Files                                              *
*******************************************************************************/
#include "../system/bus_type.h"

/*******************************************************************************
*  Declaration of Module Functions                                             *
*******************************************************************************/
short OperParmNBasicInfo ( NEW_IMGNVOICE_RECV* pstNewImgNVoiceRecv );
short VoiceVerLoad( void );


#endif
