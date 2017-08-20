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
*  PROGRAM ID :       load_parameter_file_mgt.h                                *
*                                                                              *
*  DESCRIPTION:       이 프로그램은 파라미터 파일과 집계와 통신하기위한  		   *
*                       정보를 로드한다.                                        *
*                                                                              *
*  ENTRY POINT:     void InitOperParmInfo( void );							   *
*					short LoadOperParmInfo( void );							   *
*					short LoadVehicleParm( void );							   *
*					short LoadRouteParm( ROUTE_PARM *pstRouteParmInfo );	   *
*					short LoadInstallInfo( void );							   *
*					short GetLEAPPasswd( void );                               *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: None                                                     *
*                                                                              *
*  INPUT FILES:         c_op_par.dat                                           *
*                       c_li_par.dat                                           *
*                       c_ap_inf.dat                                           *
*                       c_dp_inf.dat                                           *
*                       c_cd_inf.dat                                           *
*                       c_tr_inf.dat                                           *
*                       c_de_inf.dat                                           *
*                       c_ho_inf.dat                                           *
*                       c_n_far.dat                                            *
*                       c_st_inf.dat                                           *
*                       setup.dat                                              *
*                       setup.backup                                           *
*                       tc_leap.dat                                            *
*                       tc_leap.backup                                         *
*                                                                              *
*  OUTPUT FILES:                                                               *
*                                                                              *
*  SPECIAL LOGIC:     None                                                     *
*                                                                              *
********************************************************************************
*                         MODIFICATION LOG                                     *
*                                                                              *
*    DATE                SE NAME                      DESCRIPTION              *
* ---------- ---------------------------- ------------------------------------ *
* 2005/09/03 Solution Team Mi Hyun Noh  Initial Release                        *
*                                                                              *
*******************************************************************************/
#ifndef _CHECK_VALID_FILE_MGT_H_
#define _CHECK_VALID_FILE_MGT_H_

/*******************************************************************************
*  Inclusion of System Header Files                                            *
*******************************************************************************/
#include "file_mgt.h"

/*******************************************************************************
*  Declaration of Module Functions                                             *
*******************************************************************************/
void InitOperParmInfo( void );
short LoadOperParmInfo( void );
short LoadVehicleParm( void );
short LoadRouteParm( void );
short LoadInstallInfo( void );
short GetLEAPPasswd( void );
short LoadStationInfo( void );

#endif