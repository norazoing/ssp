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
*  PROGRAM ID :       socket_comm.c	                                   		   *
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

/*******************************************************************************
*  Inclusion of System Header Files                                            *
*******************************************************************************/
#include "../system/bus_type.h"
#include "../system/device_interface.h"

#include "socket_comm.h"

extern byte gbDCSCommType;

/*******************************************************************************
*  Declaration of Module Functions                                             *
*******************************************************************************/
static int InitSock( word wPortNo );
static void DisplayDCSCommFND( void );

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      InitIP                                                   *
*                                                                              *
*  DESCRIPTION :      IP를 0.0.0.0으로 초기화 한다.                             *
*                                                                              *
*  INPUT PARAMETERS:  char *pchDevice                                          *
*                                                                              *
*  RETURN/EXIT VALUE:     SUCCESS                                              *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short InitIP( char *pchDevice )
{
    char achCmd[128] = { 0, };

	printf( "[InitIP] IP 초기화\n" );

    sprintf( achCmd, "/sbin/ifconfig %s 0.0.0.0", pchDevice );
    system( achCmd );

    return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      GetLocalIP                                               *
*                                                                              *
*  DESCRIPTION :      socket으로 부터 IP를 얻는다.                              *
*                                                                              *
*  INPUT PARAMETERS:  char *IPAddr                                             *
*                                                                              *
*  RETURN/EXIT VALUE:  0 - SUCCESS                                             *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short GetLocalIP( char *IPAddr )
{
    int     nIFCLenLoop;

    int     nNumReqs        = 30;      // ethernet data struct count
    char    achIPBuff[30]   = { 0,};
    const   char  achDelimiters[] = ".";
    char*   pchAddr;
    char    achAddr1[4] = { 0, };
    char    achAddr2[4] = { 0, };
    char    achAddr3[4] = { 0, };
    char    achAddr4[4] = { 0, };
    int     nAddr1      = 0;
    int     nAddr2      = 0;
    int     nAddr3      = 0;
    int     nAddr4      = 0;

    int     fdSock;

    struct ifreq*       ifr;      // ethernet data struct
    struct ifconf       ifcfg;    // ethernet config struct
    struct sockaddr_in* sin;

    // initialize config struct for loading ethernet config information
    memset( &ifcfg, 0, sizeof( ifcfg ) );

    fdSock = socket( AF_INET, SOCK_DGRAM, 0 );

    /*
     * load ifreq data into ifc_buf,
     * enough space allocating for several network device
     * in general, loop-back address and one ethernet card, 2EA device
     */
    ifcfg.ifc_buf = NULL;
    ifcfg.ifc_len = sizeof( struct ifreq ) * nNumReqs;
    ifcfg.ifc_buf = malloc( ifcfg.ifc_len );
    ifcfg.ifc_len = sizeof( struct ifreq ) * nNumReqs;
    ifcfg.ifc_buf = realloc( ifcfg.ifc_buf, ifcfg.ifc_len );

    if ( ioctl( fdSock, SIOCGIFCONF, (char *)&ifcfg ) < 0 )
    {
        return ErrRet( ERR_SIOCGIFCONF );
    }

    /*
     * reads network device  information
     * in general, loop-back address and one ethernet card, 2EA device data
     * will be printed
     */
    ifr = ifcfg.ifc_req;

    for ( nIFCLenLoop = 0 ;
          nIFCLenLoop < ifcfg.ifc_len ;
          nIFCLenLoop += sizeof( struct ifreq ) )
    {
        // address value print, check loop-back address
        if ( memcmp( ifr->ifr_name, NETWORK_DEVICE, 4 ) == 0 )
        {
            sin = (struct sockaddr_in *)&ifr->ifr_addr;

            // convert from binary type IP to internet address
            memcpy( achIPBuff, inet_ntoa( sin->sin_addr ), 15 );

            pchAddr = strtok( achIPBuff, achDelimiters );
            nAddr1 = GetINTFromASC( pchAddr, 3 );
            sprintf( achAddr1, "%03d", nAddr1 );
            pchAddr = strtok( NULL, achDelimiters );
            nAddr2 = GetINTFromASC( pchAddr, 3 );
            sprintf( achAddr2, "%03d", nAddr2 );
            pchAddr = strtok( NULL, achDelimiters );
            nAddr3 = GetINTFromASC( pchAddr, 3 );
            sprintf( achAddr3, "%03d", nAddr3 );
            pchAddr = strtok( NULL, achDelimiters );
            nAddr4 = GetINTFromASC( pchAddr, 3 );
            sprintf( achAddr4, "%03d", nAddr4 );

            memcpy( IPAddr, achAddr1, 3 );
            memcpy( &IPAddr[3], achAddr2, 3 );
            memcpy( &IPAddr[6], achAddr3, 3 );
            memcpy( &IPAddr[9], achAddr4, 3 );

            DebugOut( "\r\n IPAddress[%s] \r\n", IPAddr );

        }
        ifr++;
    }

    return SUCCESS;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SetLocalIP                                               *
*                                                                              *
*  DESCRIPTION :      udhcp를 이용하여 IP를 세팅한다.                           *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       ERR_SET_LOCAL_IP                                       *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short SetLocalIP( void )
{
    short   sReturnVal      = ERR_SET_LOCAL_IP;
    FILE*   fdFile          = NULL;
    char    achBufff[255]   = { 0, };
    int     nResult;

    // Kill udhcpc Process
    KillProc( "udhcpc" );

    // udhcpc 실행
    nResult = system( "udhcpc" );
    DebugOut( "\r\nUDHCP system return Valude ==> %d\r\n", nResult );

    fdFile = popen( "udhcpc", "r" );

    while ( TRUE )
    {
        if ( fgets( achBufff, 255, fdFile ) == NULL )
        {
            break;
        }

        if ( strstr( achBufff, "deleting routers" ) != NULL )
        {
            sReturnVal = SUCCESS;
        }

    }

    pclose( fdFile );

    // Kill udhcpc Process
    KillProc( "udhcpc" );

    return sReturnVal;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      InitSock                                                 *
*                                                                              *
*  DESCRIPTION :      소켓을 초기화한다.                                        *
*                                                                              *
*  INPUT PARAMETERS:  word wPortNo  - 오픈될 socket의 port No                  *
*                                                                              *
*  RETURN/EXIT VALUE:     fdSock                                               *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static int InitSock( word wPortNo )
{
    // struct for local or remote address connecting socket
    struct  sockaddr_in stServerAddr;
    int     fdSock;
    long    lOptLen;
    int     nOptOn              = 1;        // socket option on : 1,  off: 0
    long    lSockFlag;                      // socket flag
    int     nConnResult;

    fd_set  readFd;
    fd_set  writeFd;
    struct  timeval stTimeVal;
    int     nTimeOutSec            = 2;

//    int     nRecvPktSize        = MAX_RECV_PKT_SIZE;
//    int     nSendPktSize        = MAX_SEND_PKT_SIZE;

    int     nSelectLoop;
    int     nError;
    int     nSockOpt;

    int     nErrLen;

    memset( (char *)&stServerAddr, 0, sizeof( stServerAddr ) );

    stServerAddr.sin_family      = AF_INET;

    /*
     * host byte order -> network byte order
     */
    stServerAddr.sin_port        = htons( wPortNo );

    /*
     *  retun value which is converted from internet address
     *  to 32bit binary address
     */
    if ( inet_aton( gabDCSIPAddr, &(stServerAddr.sin_addr) ) == 0 )
    {
        printf( "[InitSock] inet_aton() 오류\n" );
        return ErrRet( ERR_SOCKET_CREATE );
    }

    /*
     *  socket creation
     */
    if( ( fdSock = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 )
    {
        printf( "[InitSock] socket() 오류\n" );
        return ErrRet( ERR_SOCKET_CREATE );
    }

    /*
     * when socket closed abnormally and re-creation event occur,
     * use the address
     */
    lOptLen = (size_t)sizeof( nOptOn );

    /*
     * socket option setting
     * 옵션을 넣을 경우 서버에서 socket을 끊을 경우 멈춰버리는 현상 발생 
    if ( setsockopt( fdSock, SOL_SOCKET, SO_REUSEADDR, (char*)&nOptOn, lOptLen
                   ) < 0 )
    {
        printf( "[InitSock] setsockopt() 1 오류\n" );
        return ErrRet( ERR_SOCKET_SET_OPTION );
    }

    if ( setsockopt( fdSock, SOL_SOCKET, SO_SNDBUF, (char*)&nSendPktSize,
                     lOptLen
                   ) < 0 )
    {
        printf( "[InitSock] setsockopt() 2 오류\n" );
        return ErrRet( ERR_SOCKET_SET_OPTION );
    }

    if ( setsockopt( fdSock, SOL_SOCKET, SO_RCVBUF, (char*)&nRecvPktSize,
                     lOptLen
                   ) < 0 )
    {
        printf( "[InitSock] setsockopt() 3 오류\n" );
        return ErrRet( ERR_SOCKET_SET_OPTION );
    }
*/

	lSockFlag = fcntl( fdSock, F_GETFL, 0 );        // fdSock flag value
	fcntl( fdSock, F_SETFL, lSockFlag | O_NONBLOCK );

	nConnResult = connect( fdSock, (struct sockaddr *)&stServerAddr,
		sizeof( stServerAddr ) );
    if ( nConnResult < 0 )
    {
        if ( errno != EINPROGRESS )
        {
	        printf( "[InitSock] connect() 오류 (errno : %d)\n", errno );
			close( fdSock );
            return ErrRet( ERR_SOCKET_CONNECT );
        }
    }

    if ( nConnResult == 0 )
    {
        printf( "[InitSock] connect() 성공\n" );
        return fdSock;
    }

    FD_ZERO( &readFd );
    FD_ZERO( &writeFd );
    FD_SET( fdSock, &writeFd );
	FD_SET( fdSock, &readFd );

    stTimeVal.tv_sec = nTimeOutSec;
    stTimeVal.tv_usec = 0;

    for ( nSelectLoop = 0 ; nSelectLoop < 5 ; nSelectLoop++ )
    {
		nError = select( fdSock + 1, &readFd, &writeFd, NULL, &stTimeVal );
        if ( nError == 0 )
        {
	        printf( "[InitSock] select() 타임아웃\n" );
            errno = ETIMEDOUT;
			sleep( 2 );
            continue;
        }

        if ( nError == -1 )
        {
	        printf( "[InitSock] select() 오류 (errno : %d)\n", errno );
            continue;
        }

        break;
    }
    if ( nSelectLoop >= 5 )
    {
        printf( "[InitSock] select() 5회 오류/타임아웃\n" );
		close( fdSock );
        return ErrRet( ERR_SOCKET_CONNECT );
    }

    if ( FD_ISSET( fdSock, &readFd ) || FD_ISSET( fdSock, &writeFd ) )
    {
        nErrLen = sizeof( nError );

        /*
         * reads socket option value
         */
        nSockOpt = getsockopt( fdSock, SOL_SOCKET, SO_ERROR, &nError, &nErrLen );
        if ( nSockOpt != 0 )
        {
	        printf( "[InitSock] getsockopt() 오류 (errno : %d)\n", errno );
			close( fdSock );
            return ErrRet( ERR_SOCKET_CONNECT );
        }

        if ( nError )
        {
	        printf( "[InitSock] nError != 0 오류 (nError : %d)\n", nError );
            errno = nError;
			close( fdSock );
			sleep( 2 );
            return ErrRet( ERR_SOCKET_CONNECT );
        }
    }
    else
    {
        printf( "[InitSock] FD_ISSET() 오류\n" );
		close( fdSock );
        return ErrRet( ERR_SOCKET_CONNECT );
    }

    return fdSock;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      OpenSock                                                 *
*                                                                              *
*  DESCRIPTION :      해당 IP의 해당 port에 socket을 open한다.                  *
*                                                                              *
*  INPUT PARAMETERS:  char *pchIP, char *pchPort                               *
*                                                                              *
*  RETURN/EXIT VALUE:     InitSock( nPortNo )                                  *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
int OpenSock( char *pchIP, char *pchPort )
{
	byte i = 0;
	int nResult = 0;
    int nIPLenLoop  = 0;
    int nIPLen      = 0;
    int nPortNo     = 0;

    for ( nIPLenLoop = 0 ; nIPLenLoop < strlen( pchIP ) ; nIPLenLoop++ )
    {
        if ( pchIP[nIPLenLoop] == SPACE )
        {
            pchIP[nIPLenLoop] = 0x00;
        }
    }

    nIPLen = strlen( pchIP );

    if ( nIPLen > IP_LENGTH )
    {
        nIPLen = IP_LENGTH;
    }
    nPortNo = GetINTFromASC( pchPort, 5 );

	for ( i = 0; i < 3; i++ )
	{
		nResult = InitSock( nPortNo );
		if ( nResult >= 0 )
		{
			break;
		}
	}

    return nResult;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      CloseSock                                                *
*                                                                              *
*  DESCRIPTION :      socket을 close한다.                                      *
*                                                                              *
*  INPUT PARAMETERS:  int nSock                                                *
*                                                                              *
*  RETURN/EXIT VALUE:     close( nSock )                                       *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short CloseSock( int nSock )
{
    return close( nSock );
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SockSendPkt                                              *
*                                                                              *
*  DESCRIPTION :      해당 socket으로 패킷을 송신한다.                          *
*                                                                              *
*  INPUT PARAMETERS:    int fdSock,                                            *
*                       int nMsgSize,                                          *
*                       byte* pbMsg                                            *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       ERR_SOCKET_SEND                                        *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short SockSendPkt( int fdSock, int nTimeOutSec, int nMsgSize, byte *pbMsg )
{
    short   sReturnVal;
    fd_set  fdWrite;
    struct  timeval stTimeVal;
    int     nResult;

    FD_ZERO( &fdWrite );
    FD_SET( fdSock, &fdWrite );

    stTimeVal.tv_sec    = 4;
    stTimeVal.tv_usec   = 0;

    switch( select( fdSock + 1, NULL, &fdWrite, NULL, &stTimeVal ) )
    {
        case -1 :
			LogDCS( "[SockSendPkt] select() 오류 (errno : %d)\n", errno );
            printf( "[S_SELECT(%d)]", errno );
			fflush( stdout );
            return ErrRet( ERR_SOCKET_SELECT );
        case 0 :
			LogDCS( "[SockSendPkt] select() 타임아웃\n" );
            printf( "[S_TIMEOUT]" );
			fflush( stdout );
            return ErrRet( ERR_SOCKET_TIMEOUT );
        default :
            break;
    }

    if ( !FD_ISSET( fdSock, &fdWrite ) )
    {
		LogDCS( "[SockSendPkt] FD_ISSET 오류\n" );
		printf( "[S_FD_ISSET]" );
		fflush( stdout );
		return ErrRet( ERR_SOCKET_SELECT );
    }

    nResult = send( fdSock, pbMsg, nMsgSize, 0 );
    if ( nResult == nMsgSize)
    {
        sReturnVal = SUCCESS;
    }
    else if ( nResult == -1 )
    {
		LogDCS( "[SockSendPkt] send() 오류 (errno : %d)\n", errno );
        printf( "[S_SEND(%d)]", errno );
		fflush( stdout );
        sReturnVal = ERR_SOCKET_SEND;
    }
	else
	{
		LogDCS( "[SockSendPkt] 모두 send() 실패 (성공길이 : %d)\n", nResult );
        printf( "[S_SEND_LEN(%d)]", nResult );
		fflush( stdout );
        sReturnVal = ERR_SOCKET_SEND;
	}

	DisplayDCSCommFND();

    return sReturnVal;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SockRecvPkt                                              *
*                                                                              *
*  DESCRIPTION :      해당 socket으로부터 패킷을 수신받는다.                     *
*                                                                              *
*  INPUT PARAMETERS:  int fdSock, int nTimeOut, long* plMsgSize, byte* pbMsg   *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       ErrRet( ERR_SOCKET_TIMEOUT )                           *
*                       ErrRet( ERR_SOCKET_RECV )                              *
*                       ErrRet( ERR_SOCKET_BUFF_OVER_FLOW )                    *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short SockRecvPkt( int fdSock, int nTimeOutSec, int *pnMsgSize, byte *pbMsg )
{
    fd_set  readFd;
    struct  timeval stTimeVal;
    int     nRecvByte;
    int     nTotalRecvSize = 0;
    byte*   pbMsgPtr;

    pbMsgPtr = pbMsg;

    while ( TRUE )
    {
        FD_ZERO( &readFd );
        FD_SET( fdSock, &readFd );

        stTimeVal.tv_sec    = nTimeOutSec;
        stTimeVal.tv_usec   = 0;

        switch( select( fdSock + 1, &readFd, NULL, NULL, &stTimeVal ) )
        {
            case -1 :
				LogDCS( "[SockRecvPkt] select() 오류 (errno : %d)\n", errno );
                printf( "[R_SELECT(%d)]", errno );
				fflush( stdout );
                return ErrRet( ERR_SOCKET_SELECT );
            case 0 :
				LogDCS( "[SockRecvPkt] select() 타임아웃 (nTotalRecvSize : %d)\n", nTotalRecvSize );
                printf( "[R_TIMEOUT(%d)]", nTotalRecvSize );
				fflush( stdout );
                return ErrRet( ERR_SOCKET_TIMEOUT );
            default :
                break;
        }

	    if ( !FD_ISSET( fdSock, &readFd ) )
	    {
			LogDCS( "[SockRecvPkt] FD_ISSET 오류\n" );
			printf( "[R_FD_ISSET]" );
			fflush( stdout );
			continue;
	    }

        nRecvByte = recv( fdSock, pbMsgPtr, MAX_RECV_PKT_SIZE, 0 );
        if ( nRecvByte < 0 )
        {
			LogDCS( "[SockRecvPkt] recv() 오류 (errno : %d)\n", errno );
            printf( "[R_RECV(%d)]", errno );
			fflush( stdout );
            return ErrRet( ERR_SOCKET_RECV );
        }

        nTotalRecvSize += nRecvByte;
        pbMsgPtr += nRecvByte;

        if ( nTotalRecvSize > MAX_RECV_PKT_SIZE )
        {
			LogDCS( "[SockRecvPkt] Over Flow 오류\n" );
            printf( "[R_OVER_FLOW]" );
			fflush( stdout );
            return ErrRet( ERR_SOCKET_BUFF_OVER_FLOW );
        }

        if ( pbMsgPtr[-2] == ETX )
        {
            break;
        }

        if ( nRecvByte == 0 )
        {
			LogDCS( "[SockRecvPkt] recv() 결과 0 오류\n" );
            printf( "[R_RECV_0]" );
			fflush( stdout );
            return ErrRet( ERR_SOCKET_RECV );
        }
    }

    *pnMsgSize = nTotalRecvSize;

	DisplayDCSCommFND();

    return SUCCESS;
}

static void DisplayDCSCommFND( void )
{
	static byte i = 0;
	static byte bDCSCommSeq = 0;


	if ( i++ % 20 == 0 )
	{
		switch ( gbDCSCommType )
		{
			case 1:
				DisplayDWORDInDownFND( 111110 + ( bDCSCommSeq++ % 10 ) );
				break;
			case 2:
				DisplayDWORDInDownFND( 122220 + ( bDCSCommSeq++ % 10 ) );
				break;
			case 3:
				DisplayDWORDInDownFND( 133330 + ( bDCSCommSeq++ % 10 ) );
				break;
			default:
		}
	}	
}

