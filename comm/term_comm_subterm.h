
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
*  PROGRAM ID :       sc_read.c                                                *
*                                                                              *
*  DESCRIPTION:       This program reads Smart Card, checks validation of card *
*                     and saves Common Structure.                              *
*                                                                              *
*  ENTRY POINT:       SCRead()               ** optional **                    *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: 0 - Success                                              *
*                     (-) - Error                                              *
*                                                                              *
*  INPUT FILES:       Issure Info Files                                        *
*                                                                              *
*  OUTPUT FILES:      None                                                     *
*                                                                              *
*  SPECIAL LOGIC:     None                                                     *
*                                                                              *
********************************************************************************
*                         MODIFICATION LOG                                     *
*                                                                              *
*    DATE                SE NAME                      DESCRIPTION              *
* ---------- ---------------------------- ------------------------------------ *
* 2005/09/27 Solution Team  woolim        Initial Release                      *
* 2006/04/17 F/W Dev. Team  wangura       파일분리 및 추가 구조화              *
*                                                                              *
*******************************************************************************/
#ifndef _TERM_COMM_SUBTERM_H
#define _TERM_COMM_SUBTERM_H


typedef struct {
    byte    bEncryptAlgCd;  // 1 암호화 알고리즘
    byte    bIDCenter;      // 1 키셋버젼 발행사id
    byte    abSAMID[8];     // 8 SAM ID
    byte    bSortKey;       // sort key (키종류)
    byte    abVK[4];        // vk (키버젼)
    byte    abEKV[64];      // ekv 전자화폐사별 키셋
    byte    abSign[4];      // sign
} __attribute__((packed))OFFLINE_KEYSET_DATA;


typedef struct { // tot 35
    byte    bEncrytAlgCd;   // 알고리즘id
    byte    bIDCenter;      // 키셋버젼 발행사id
    byte    abSAMID[8];     // sam id
    byte    abEKV[16];      // EKV
    byte    abSign[4];      // Sign
} __attribute__((packed)) OFFLINE_IDCENTER_DATA;



/*******************************************************************************
*  Declaration of Header Files                                                 *
*******************************************************************************/
void SubTermProc( void );
short RegistOfflineID2PSAMbyEpurseIssuer(void);
short RegistOfflineKeyset2PSAMbyEpurseIssuer(void);

#endif
