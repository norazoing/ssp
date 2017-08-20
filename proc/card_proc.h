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
*  PROGRAM ID :       card_proc.h                                              *
*                                                                              *
*  DESCRIPTION:       카드의 태그여부 체크에서부터 처리결과 출력까지 카드처리와*
*                     관련된 모든 사항을 처리한다.                             *
*                                                                              *
*  ENTRY POINT:       CardProc()                                               *
*                                                                              *
*  INPUT PARAMETERS:  void                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: void                                                     *
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

#ifndef _CARD_PROC_H_
#define _CARD_PROC_H_

#define ENT									0			// 승차
#define EXT									1			// 하차

void CardProc(void);

#endif
