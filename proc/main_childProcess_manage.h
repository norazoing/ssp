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
*  PROGRAM ID :       mainChildProcessManage.h                                 *
*                                                                              *
*  DESCRIPTION:       MAIN외에 CHILD PROCESS를 CHECK한다.                      *
*                                                                              *
*  ENTRY POINT:     void CreateChildProcess(void);                             *
*                   void CheckChildProcess(void);                              *
*                   void KillChildProcess( void );                             *
*                   void KillGPSThread( void );								   *
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
#ifndef _MAIN_CHILDPROCESS_MANAGE_H_
#define _MAIN_CHILDPROCESS_MANAGE_H_

/*******************************************************************************
*  Declaration of Module Functions                                             *
*******************************************************************************/
void CreateChildProcess(void);
void CheckChildProcess(void);
void KillChildProcess( void );
void KillGPSThread( void );
void CheckGPSThread( void );
void CreateGPSThread( void );

#endif

