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
*  PROGRAM ID :       printer_proc.h                                           *
*                                                                              *
*  DESCRIPTION:       Includes prototypes of printer interface functions       *
*                                                                              *
*  REMARKS    :                                                                *
*																			   *
********************************************************************************
*                         MODIFICATION LOG                                     *
*                                                                              *
*    DATE                SE NAME                      DESCRIPTION              *
* ---------- ---------------------------- ------------------------------------ *
* 2005/08/11 Solution Team  Gwan Yul Kim  Initial Release                    *
*                                                                              *
*******************************************************************************/
#ifndef _PRINTER_PROC_H
#define _PRINTER_PROC_H

/*******************************************************************************
*  Declaration of Header Files                                                 *
*******************************************************************************/
#include <sys/msg.h>

#include "../system/bus_type.h"
#include "../system/device_interface.h"
#include "load_parameter_file_mgt.h"


/*******************************************************************************
*  Declaration of Functions                                                    *
*******************************************************************************/
void CommPrinter( void );

#endif