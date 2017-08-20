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
*  PROGRAM ID :       reconcile_file_mgt.h                                    *
*                                                                              *
*  DESCRIPTION:       이 프로그램은 레컨사일 파일의 CRUD의 기능을 제공 한다.      *
*                                                                              *
*  ENTRY POINT:       WriteReconcileFileList                                   *
*                     ReadReconcileFileList                                    *
*                     UpdateReconcileFileList                                  *
*                     DelReconcileFileList                                     *
*                     CopyReconcileFile                                        *
*                     LoadReconcileFileList                                    *
*                     GetReconcileCnt                                          *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: None                                                     *
*                                                                              *
*  INPUT FILES:       reconcileinfo.dat                                        *
*                                                                              *
*  OUTPUT FILES:      reconcileinfo.dat                                        *
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
#ifndef _RECONCILE_FILE_MGT_H_
#define _RECONCILE_FILE_MGT_H_

/*******************************************************************************
*  Inclusion of System Header Files                                            *
*******************************************************************************/
#include "file_mgt.h"

/*******************************************************************************
*  Declaration of Module Functions                                             *
*******************************************************************************/
short WriteReconcileFileList( RECONCILE_DATA* pstReconcileData );
short ReadReconcileFileList( char* pchFileName,
                             RECONCILE_DATA* pstReconcileData );
short UpdateReconcileFileList( char* pchFileName,
                               RECONCILE_DATA* pstReconcileData );
short DelReconcileFileList( char* pchFileName,
                            RECONCILE_DATA* pstReconcileData );
short CopyReconcileFile( char* pchTmpFileName );
short LoadReconcileFileList( RECONCILE_DATA* pstReconcileData,
							 int* nReconcileCnt,
							 int* pnFileCnt,
							 int* pnMsgCnt );
short GetReconcileCnt( int *pnRecCnt );

#endif