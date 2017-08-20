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
*  PROGRAM ID :       version_mgt.h                                            *
*                                                                              *
*  DESCRIPTION:       이 프로그램은 미래적용버전을 적용하고 각 버전을 롤백하거나  *
*						초기화하는 기능의 함수를 제공한다. 					   *
*                                                                              *
*  ENTRY POINT:     short ApplyNextVer( void );								   *
*					short RollbackBLPLVer( 	bool boolIsBLMergeFail,			   *
*                       					bool boolIsPLMergeFail,			   *
*                       					bool boolIsAIMergeFail );		   *
*					short SetMasterBLPLVer( void );		   					   *
*					void SetVerBLPLFileExistYN( void );						   *
*					short InitSAMRegistInfoVer( void );						   *
*					short WriteBLPLBefVer( char* pachFileName, 				   *
*										   char* pachBefVer, 				   *
*										   int nBefVerLength );				   *
*					short CreateVerInfoPkt( bool boolIsCurrVer,				   *
*											byte* pbVerInfoBuff );			   *
*																			   *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: None                                                     *
*                                                                              *
*  INPUT FILES:       c_ve_inf.dat                                             *
*					  v_bl_be.dat                                              *
*					  v_pl_be.dat                                              *
*					  v_ai_be.dat                                              *
*					  v_pl_be.dat                                              *
*					  donwlink.dat                                             *
*					  c_fi_bl.dat                                              *
*					  c_fa_pl.dat                                              *
(					  c_fd_pl.dat                                              *
*					  c_fi.ai.dat                                              *
*                                                                              *
*  OUTPUT FILES:      c_ve_inf.dat                                             *
*					  c_dr_pro.dat                                             *
*					  c_en_pro.dat                                             *
*					  c_ex_pro.dat                                             *
*					  c_v0.dat                                                 *
*					  c_op_par.dat                                             *
*					  c_li_par.dat                                             *
*					  c_n_far.dat                                              *
*					  c_st_inf.dat                                             *
*					  c_ap_inf.dat                                             *
*					  c_dp_inf.dat                                             *
*					  c_de_inf.dat                                             *
*					  c_ho_inf.dat                                             *
*					  c_tc_inf.dat                                             *
*					  c_idcenter.dat                                           *
*					  c_keyset.dat                                             *
*					  c_tr_inf.dat                                             *
*					  c_cd_inf.dat                                             *
*					  v_bl_be.dat                                              *
*					  v_pl_be.dat                                              *
*					  v_ai_be.dat                                              *
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

#ifndef _VERSION_MGT_H_
#define _VERSION_MGT_H_

/*******************************************************************************
*  Inclusion of System Header Files                                            *
*******************************************************************************/
#include "file_mgt.h"

/*******************************************************************************
*  Declaration of Module Functions                                             *
*******************************************************************************/
short ApplyNextVer( void );
short RollbackBLPLVer( bool boolIsBLMergeFail,
                       bool boolIsPLMergeFail,
                       bool boolIsAIMergeFail );
short SetMasterBLPLVer( void );
void SetVerBLPLFileExistYN( void );
short InitSAMRegistInfoVer( void );
short WriteBLPLBefVer( char* pachFileName,
				       char* pachBefVer,
				       int nBefVerLength );
short CreateVerInfoPkt( bool boolIsCurrVer, byte* pbVerInfoBuff );

#endif