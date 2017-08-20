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
*  PROGRAM ID :       write_trans_data.c                                       *
*                                                                              *
*  DESCRIPTION:       거래내역 기록을 위한 함수들을 제공한다.                  *
*                                                                              *
*  ENTRY POINT:       short WriteTransHeader( void );                          *
*                     void WriteTransData( TRANS_INFO *pstTransInfo );         *
*                     short WriteTransDataForCommProcess( byte *abTransTD );   *
*                     void WriteCashTransData( byte bUserType );               *
*                     short WriteTransTail( void );                            *
*                     short UpdateTransDCSRecvDtime( byte *abTransFileName );  *
*                     short RemakeTrans( void );                               *
*                                                                              *
*  INPUT FILES:       control.trn - 운행정보기록파일                           *
*                     YYYYMMDDhhmmss.trn - 승차거래내역파일                    *
*                     YYYYMMDDhhmmss.10 - 하차임시대사파일                     *
*                     YYYYMMDDhhmmss.20 - 하차임시대사파일                     *
*                     YYYYMMDDhhmmss.30 - 하차임시대사파일                     *
*                                                                              *
*  OUTPUT FILES:      control.trn - 운행정보기록파일                           *
*                     YYYYMMDDhhmmss.trn - 승차거래내역파일                    *
*                     YYYYMMDDhhmmss.trn - 하차대사거래내역파일                *
*                     alight_term_td.tmp - 하차거래내역파일                    *
*                     aboard_term_td.bak - 승차백업거래내역파일                *
*                     alight_term_td.bak - 하차백업거래내역파일                *
*                     temptd.dat - 임시대사파일                                *
*                                                                              *
*  SPECIAL LOGIC:     None                                                     *
*                                                                              *
********************************************************************************
*                         MODIFICATION LOG                                     *
*                                                                              *
*    DATE                SE NAME                      DESCRIPTION              *
* ---------- ---------------------------- ------------------------------------ *
* 2006/01/11 F/W Dev Team Boohyeon Jeon   Initial Release                      *
*                                                                              *
*******************************************************************************/

/*

승차거래내역파일			YYYYMMDDhhmmss.trn					MAIN_TRANS
하차거래내역파일			alight_term_td.tmp		203B		SUB_TRANS
하차대사거래내역파일		YYYYMMDDhhmmss.trn					SUB_REMAKE
임시대사파일				temptd.dat							TEMP_REMAKE
승차백업거래내역파일		aboard_term_td.bak					BACKUP_TRANS
하차백업거래내역파일		alight_term_td.bak					BACKUP_TRANS
하차임시대사파일			YYYYMMDDhhmmss.10					SUB_TEMP_REMAKE

*/

#ifndef _WRITE_TRANS_DATA_H_
#define _WRITE_TRANS_DATA_H_

typedef struct {
	byte bDriveNow;							// 운행구분
	byte abTotalCnt[9] ;					// 총건수
	byte abTotalAmt[10];					// 총금액
	byte abTRFileName[18];					// TR파일명
}__attribute__((packed)) CONTROL_TRANS;

short WriteTransHeader( void );
void WriteTransData( TRANS_INFO *pstTransInfo );
short WriteTransDataForCommProcess( byte *abTransTD );
void WriteCashTransData( byte bUserType );
short WriteTransTail( void );
short UpdateTransDCSRecvDtime( byte *abTransFileName );
short RemakeTrans( void );

#endif
