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
*  PROGRAM ID :       socket_comm.h	                                   		   *
*                                                                              *
*  DESCRIPTION:       소켓통신을 위한 함수들을 제공한다.						   *
*                                                                              *
*  ENTRY POINT:     short InitIP( char *pchDevice );						   *
*					short GetLocalIP( char *IPAddr );						   *
*					short SetLocalIP( void );						   		   *
*					int OpenSock( char *pchIP, char *pchPort );				   *
*					short CloseSock( int nSock );							   *
*					short SockSendPkt( int fdSock, int nMsgSize, byte* pbMsg );*
*					short SockRecvPkt( 	int fdSock,						       *
*                          				int nTimeOut,						   *
*                          				long* plMsgSize,					   *
*                          				byte* pbMsg );						   *
*                                                                              *
*  INPUT FILES:     None	                                                   *
*                                                                              *
*  OUTPUT FILES:    None				   									   *
*                                                                              *
*  SPECIAL LOGIC:   None                                                       *
*                                                                              *
********************************************************************************
*                         MODIFICATION LOG                                     *
*                                                                              *
*    DATE                SE NAME                      DESCRIPTION              *
* ---------- ---------------------------- ------------------------------------ *
* 2006/03/27 F/W Dev Team Mi Hyun Noh  Initial Release                         *
*                                                                              *
*******************************************************************************/
#ifndef _SOCKET_COMM_H_
#define _SOCKET_COMM_H_

/*******************************************************************************
*  Declaration of variables                                                    *
*******************************************************************************/
#define MAX_RECV_PKT_SIZE           ( 23 + MAX_PKT_SIZE )
#define MAX_SEND_PKT_SIZE           ( 23 + MAX_PKT_SIZE )
#define IP_LENGTH                   15

#define STX     				   	0x02		//Start of TeXt
#define ETX     				   	0x03		//End of TeXt

#define NETWORK_DEVICE              "eth0"

/*******************************************************************************
*  Declaration of Module Functions                                             *
*******************************************************************************/
short InitIP( char *pchDevice );
short GetLocalIP( char *IPAddr );
short SetLocalIP( void );
int OpenSock( char *pchIP, char *pchPort );
short CloseSock( int nSock );
short SockSendPkt( int fdSock, int nTimeOutSec, int nMsgSize, byte *pbMsg );
short SockRecvPkt( int fdSock, int nTimeOutSec, int *pnMsgSize, byte *pbMsg );
#endif