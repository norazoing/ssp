
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

#ifndef _TERM_COMM_MAINTERM_H
#define _TERM_COMM_MAINTERM_H
/*******************************************************************************
*  Declaration of Header Files                                                 *
*******************************************************************************/

void MainTermProc( void );
short ChkSubTermImgVerOldProtcol( int nDevNo, char *pachhNewVerYn );
short SendSubTermImgOldProtcol( int nDevNo, char* pchFileName );
short PSAMIDCompareMainWithSubTerm(void);




#endif
