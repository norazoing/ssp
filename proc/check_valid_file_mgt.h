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
*  PROGRAM ID :       check_valid_file_mgt.c                                   *
*                                                                              *
*  DESCRIPTION:       이 프로그램은 다운로드 받은 파일의 유효성을 체크하는     *
*                     함수를 제공한다.  	   								   *
*                                                                              *
*  ENTRY POINT:       short CheckValidFileForDownload( int achDCSCommFileNo,   *
*                         char* pchFileName, DOWN_FILE_INFO* stDownFileInfo )  *
*																			   *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: None                                                     *
*                                                                              *
*  INPUT FILES:       	c_dr_pro.dat                                           *
*						c_en_pro.dat                                           *
*						c_ex_pro.dat                                           *
*						c_vo.dat	                                           *
*						c_op_par.dat                                           *
*						c_li_par.dat                                           *
*						c_n_far.dat                                            *
*						c_st_inf.dat                                           *
*						c_ap_inf.dat                                           *
*						c_dp_inf.dat                                           *
*						c_de_inf.dat                                           *
*						c_ho_inf.dat                                           *
*						c_idcenter.dat										   *
*						c_keyset.dat										   *
*						c_tr_inf.dat                                           *
*						c_cd_inf.dat                                           *
*                                                                              *
*  OUTPUT FILES:                                                    		   *
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
short CheckValidFileForDownload( int nDCSCommFileNo, char* pchFileName,
	DOWN_FILE_INFO* pstDownFileInfo );
short CheckValidFile( int nDCSCommFileNo, byte *abFileName );

#endif