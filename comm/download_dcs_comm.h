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
*  PROGRAM ID :       download_dcs_comm.h	                                   *
*                                                                              *
*  DESCRIPTION:       집계시스템으로부터 파일 다운로드를 위한 함수들을 제공한다.  *
*                                                                              *
*  ENTRY POINT:     short DownVehicleParmFile( void );						   *
*					short DownFromDCS( void );								   *
*                                                                              *
*  INPUT FILES:     c_ve_inf.dat                                               *
*                                                                              *
*  OUTPUT FILES:    achDCSCommFile에 등록된 파일들                      		   *
*					c_ve_inf.dat			- 버전정보						   *
*					downloadinfo.dat		- 다운로드된 파일 목록 정보		   *
*					downfilelink.dat		- 이어받던 파일의 정보			   *
*					connSucc.dat			- 집계통신 성공시간				   *
*                                                                              *
*  SPECIAL LOGIC:     None                                                     *
*                                                                              *
********************************************************************************
*                         MODIFICATION LOG                                     *
*                                                                              *
*    DATE                SE NAME                      DESCRIPTION              *
* ---------- ---------------------------- ------------------------------------ *
* 2006/03/27 F/W Dev Team Mi Hyun Noh  Initial Release                         *
*                                                                              *
*******************************************************************************/
#ifndef _DOWNLOAD_DCS_COMM_H_
#define _DOWNLOAD_DCS_COMM_H_

/*******************************************************************************
*  Declaration of Module Functions                                             *
*******************************************************************************/
short DownVehicleParmFile( void );
short DownFromDCS( void );
short GetRelayRecvFileSize( char chFileNo, long *plFileSize );

#endif