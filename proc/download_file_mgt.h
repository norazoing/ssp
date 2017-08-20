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
*  PROGRAM ID :       download_file_mgt.h                                     *
*                                                                              *
*  DESCRIPTION:       이 프로그램은 집계에서 다운로드 받은 파일의 목록을 관리하는 *
*						downloadinfo.dat의 CRD와 이어받기 정보를 관리하는 	   *
*						downfilelink.dat의 CU 기능, BLPL의 다운로드 여부와	   *
*						SAM 정보의 다운로드 여부를 체크하는 기능을 수행한다.	   *
*                                                                              *
*  ENTRY POINT:     short WriteDownFileList( DOWN_FILE_INFO* pstDownFileInfo );*
*					short WriteDownFileInfo( char* pachFileName,  			   *
*											 byte* pabFileVer, 				   *
*											 char chDownStatus, 			   *
*											 int nRecvdFileSize );			   *
*					short ReadDownFileList( char* pchFileName,				   *
*											DOWN_FILE_INFO* stDownFileInfo );  *
*					short DelDownFileList( char* pchFileName,				   *
*										   DOWN_FILE_INFO* stDownFileInfo );   *
*					short WriteRelayRecvFile( FILE_RELAY_RECV_INFO* 		   *
*													pstFileRelayRecvInfo );    *
*					short UpdateRelayRecvFile(  FILE* fdRelayRecv,			   *
*                            	FILE_RELAY_RECV_INFO* 	pstFileRelayRecvInfo );*
*					void WriteBLPLDownInfo( void );							   *
*				    static short CheckSAMInfoDown( void );                     *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: None                                                     *
*                                                                              *
*  INPUT FILES:       downloadinfo.dat  - 집계에서 다운로드 받은 파일의 목록     *
*                     tmp_c_ch_bl.dat   - 다운받은 변동 BL파일명( 업데이트 이전 )*
*                     tmp_c_ch_pl.dat   - 다운받은 변동 PL파일명( 업데이트 이전 )*
*                     tmp_c_ch_ai.dat   - 다운받은 변동 AI파일명( 업데이트 이전 )*
*                                                                              *
*  OUTPUT FILES:      downloadinfo.dat  - 집계에서 다운로드받은 파일의 목록      *
*                     downfilelink.dat  - 이어받기 정보를 관리                  *
*                     v_bl_be.dat       - 변동 BL의 다운받기 이전의 버전정보     *
*                     v_pl_be.dat       - 변동 PL의 다운받기 이전의 버전정보     *
*                     v_ai_be.dat       - 변동 AI의 다운받기 이전의 버전정보     *
*                     bl_pl_ctrl.flg    - BLPL을 다운여부를 세팅해주는 파일      *
*										  blpl_proc.c에서 참고함				   *
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
#ifndef _DOWNLOAD_FILE_MGT_H_
#define _DOWNLOAD_FILE_MGT_H_

/*******************************************************************************
*  Inclusion of System Header Files                                            *
*******************************************************************************/
#include "file_mgt.h"


/*******************************************************************************
*  Declaration of Module Functions                                             *
*******************************************************************************/
short WriteDownFileList( DOWN_FILE_INFO* pstDownFileInfo );
short WriteDownFileInfo( char* pachFileName,  
						 byte* pabFileVer, 
						 char chDownStatus, 
						 int nRecvdFileSize );
short ReadDownFileList( char* pchFileName, DOWN_FILE_INFO* stDownFileInfo );
short DelDownFileList( char* pchFileName, DOWN_FILE_INFO* stDownFileInfo );
short WriteRelayRecvFile( FILE_RELAY_RECV_INFO* pstFileRelayRecvInfo );
short UpdateRelayRecvFile(  FILE*					fdRelayRecv,
                            FILE_RELAY_RECV_INFO* 	pstFileRelayRecvInfo );
void WriteBLPLDownInfo( void );
short CheckSAMInfoDown( void );
short WriteOperParmFile( byte* pbBuff, long lBuffSize );

#endif