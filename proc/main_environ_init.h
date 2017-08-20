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
*  PROGRAM ID :       main_environ_init.h                                      *
*                                                                              *
*  DESCRIPTION:       MAIN 프로그램을 구동하기위한 주변장치를 check한다.       *
*                                                                              *
*  ENTRY POINT:     void OpenDeviceFiles (void)                                *
*                   void CheckEnvironment( void )                              *
*                   bool SAMInitialization (void)                              *
*                   void RFInitialization(void)                                *
*                   void VoiceMent2EndUser(void)                               *
*                   void MemoryRemoval( void )                                 *
*                   void ClosePeripheralDevices ( void )                       *
*                   void CreateSharedMemNMsgQueue( void )                      * 
*                   void CheckFreeMemory( void )                               *
*                   short IsDriveNow( void ) 								   *
*                   void DisplayVehicleID( void )                              *
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

#ifndef _MAIN_ENVIRONMENT_H_
#define _MAIN_ENVIRONMENT_H_

/*******************************************************************************
*  Inclusion of User Header Files                                              *
*******************************************************************************/
#include "../system/bus_define.h"
#include "../system/bus_errno.h"
#include "../../common/type.h"

/*******************************************************************************
*  Declaration of Macro                                                        *
*******************************************************************************/
#define FIRST_STATION_NAME				"GPS 수신중"

#define GREATHERTHAN_POINT4SECS					1
#define LESSTHANOREUQAL_POINT4SECS				0

/*******************************************************************************
*  Declaration of Global Variables                                             *
*******************************************************************************/
extern bool gboolIsDisplaySwitchOn;		// FND에 정보가 Display여부

/*******************************************************************************
*  Declaration of Module Functions                                             *
*******************************************************************************/
void OpenDeviceFiles(void);
void CheckEnvironment( void );
bool SAMInitialization(void);
void RFInitialization(void);
void VoiceMent2EndUser(void);
void MemoryRemoval( void );
void ClosePeripheralDevices( void );
void CreateSharedMemNMsgQueue( void );
bool DisplaySubTermTime( void );
void CheckFreeMemory( void );
short IsDriveNow( void );
short CheckParmFilesExist( void );
short CheckMainTermBasicInfoFile( void );
bool IsExistMainTermBasicInfoFile( void );
void DisplayVehicleID( void );

#endif

