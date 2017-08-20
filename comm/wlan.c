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
*  PROGRAM ID :       wlan.c                                                   *
*                                                                              *
*  DESCRIPTION:       This program is utility for Cisco LEAP on iPAQ.	       *
*			(Version 0.1)		                               *
*                                                                              *
*  ENTRY POINT:       LEAPMain( )                   ** optional **             *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: None                                                     *
*                                                                              *
*  INPUT FILES:       None                                                     *
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
* 2005/09/03 Solution Team Mi Hyun Noh  Initial Release                        *
*                                                                              *
*******************************************************************************/

#include "../system/bus_type.h"
#include "md4.h"
#include  "if_aironet_ieee.h"


/*******************************************************************************
*  Declaration of variables                                                    *
*******************************************************************************/

//#define SIOCIWFIRSTPRIV SIOCDEVPRIVATE
#define AIROIOCTL       SIOCDEVPRIVATE //SIOCIWFIRSTPRIV
#define AIROIDIFC       AIROIOCTL + 1

/*******************************************************************************
*  Declaration of function                                                     *
*******************************************************************************/

static int CtrlIOLanCard( const char *iface, airo_ioctl *areq );
static int initLEAPMode( const char *iface );


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      CtrlIOLanCard                                            *
*                                                                              *
*  DESCRIPTION :      This program controls lan card IO                        *
*                                                                              *
*  INPUT PARAMETERS:  const char *iface, airo_ioctl *areq                      *
*                                                                              *
*  RETURN/EXIT VALUE:     ioctl( s, AIROIOCTL, &ifr ) - SUCCESS                *
*                         err( 1, "error message" )   - ERROR                  *
*                                                                              *
*  Author  : Mi Hyun Noh   						       *
*                                                                              *
*  DATE    : 2005-09-03 						       *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/

static int CtrlIOLanCard( const char *iface, airo_ioctl *areq )
{
	struct iwreq		ifr;
	int			s;
	int	rv;

	bzero( (char *)&ifr, sizeof( ifr ) );

	strncpy( ifr.ifr_name, iface, sizeof( ifr.ifr_name ) );
	ifr.u.data.pointer = (caddr_t )areq;

	s = socket( AF_INET, SOCK_DGRAM, 0 );

	if ( s == -1 )
	{
		err( 1, "socket" );
	}

	rv = ioctl( s, AIROIOCTL, &ifr );
	
	if ( rv == -1 )
	{
		err( 1, "AIROIOCTL" );
	}

	close( s );

	return rv;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      initLEAPMode                                             *
*                                                                              *
*  DESCRIPTION :      This program initals LEAP mode                           *
*                                                                              *
*  INPUT PARAMETERS:  const char *iface			                       *
*                                                                              *
*  RETURN/EXIT VALUE:     (0) - SUCCESS             		               *
*                         (-1)- ERROR		                               *
*                                                                              *
*  Author  : Mi Hyun Noh   						       *
*                                                                              *
*  DATE    : 2005-09-03 						       *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/

static int initLEAPMode( const char *iface )
{
	char username[10];
	char password[21];
	   
	airo_ioctl	areq;
	struct an_req	anreq;
	
	struct an_ltv_status	*sts;
	struct an_ltv_genconfig	*cfg;
	struct an_ltv_caps	*caps;
	struct an_ltv_leap_username an_username;
	struct an_ltv_leap_password an_password;
//	char *password;
	MD4_CTX context;
	int len;
	int i;
	int rv;
	char unicode_password[ LEAP_PASSWORD_MAX * 2 ];

	memset( username, 0x00, sizeof( username ) );
	memset( password, 0x00, sizeof( password ) );
	
	memcpy( username, "sscbs2004", 9 );
	memcpy( password, gabLEAPPasswd, sizeof( password ) );	   
	bzero( &anreq, sizeof( anreq ) );

	areq.command = AIROGCAP;
	areq.len = sizeof( anreq );
	areq.data = (char*)&anreq;
	rv = CtrlIOLanCard( iface, &areq );
	
	caps = ( struct an_ltv_caps * )&anreq;
	
	if ( ( caps->an_softcaps & AN_AUTHTYPE_LEAP ) == 0 ) 
	{
		fprintf( stderr, "Firmware does not support LEAP\n" );
		return -1;
	}

	printf( "manuf, prod: %s %s %s\n", (char*)caps->an_manufname, 
			(char*)caps->an_prodname, (char*)caps->an_prodvers );
	printf( "uis, ppw: [%s] [%s] \n", username, password );
	
	bzero( &an_username, sizeof( an_username ) );
	bzero( &an_password, sizeof( an_password ) );

	len = strlen( username );
	
	if ( len > LEAP_USERNAME_MAX ) 
	{
		printf( "Username too long ( max %d )\n", LEAP_USERNAME_MAX );
		return -1;
	}
	
	an_username.an_len  = sizeof( an_username );	
	an_username.an_username_len = len;
	strncpy( an_username.an_username, username, len );
	
	//password = getpass( "Enter LEAP password:" );


	len = strlen( password );
	
	if ( len > LEAP_PASSWORD_MAX ) 
	{
		printf( "Password too long ( max %d )\n", LEAP_PASSWORD_MAX );
		return -1;
	}
	
	bzero( &unicode_password, sizeof( unicode_password ) );
	
	for( i = 0 ; i < len ; i++ ) 
	{
		unicode_password[i * 2] = password[i];
	}
	
	/* First half */
	MD4Init( &context );
	MD4Update( &context, unicode_password, len * 2 );
	MD4Final( &an_password.an_password[0], &context );
	
	/* Second half */
	MD4Init ( &context );
	MD4Update ( &context, &an_password.an_password[0], 16 );
	MD4Final ( &an_password.an_password[16], &context );

	an_password.an_len  = sizeof( an_password );	
	an_password.an_password_len = 32;

	areq.command = AIROPLEAPUSR;
	areq.len = sizeof( an_username );
	areq.data = (char*)&an_username;
	rv = CtrlIOLanCard( iface, &areq );

	areq.command = AIROPLEAPPWD;
	areq.len = sizeof( an_password );
	areq.data = (char*)&an_password;
	rv = CtrlIOLanCard( iface, &areq );

	areq.command = AIROGCFG;
	areq.len = sizeof( anreq );
	areq.data = (char*)&anreq;
	rv = CtrlIOLanCard( iface, &areq );
	
	cfg = (struct an_ltv_genconfig *)&anreq;
	cfg->an_authtype = ( AN_AUTHTYPE_PRIVACY_IN_USE | AN_AUTHTYPE_LEAP );
	
	areq.command = AIROPCFG;
	areq.len = sizeof( anreq );
	areq.data = (char*)&anreq;
	rv = CtrlIOLanCard( iface, &areq );

	sts = ( struct an_ltv_status * )&anreq;
	
	for ( i = 5 ; i > 0 ; i-- ) 
	{
		bzero( &anreq, sizeof( anreq ) );
		areq.command = AIROGSTAT;
		areq.len = sizeof( anreq );
		areq.data = (char*)&anreq;
		rv = CtrlIOLanCard( iface, &areq );
		printf( "." );
		
		if ( sts->an_opmode & AN_STATUS_OPMODE_LEAP ) 
		{
			printf( "Authenticated\n" );
			break;
		}
		sleep( 1 );
		
	}

	if ( i == 0 ) 
	{
		fprintf( stderr, "Failed LEAP authentication\n" );
		return -1;
	}
	return SUCCESS;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      LEAPMain                                                 *
*                                                                              *
*  DESCRIPTION :      This program is LEAP main function                       *
*                                                                              *
*  INPUT PARAMETERS:  None				                       *
*                                                                              *
*  RETURN/EXIT VALUE:     initLEAPMode( buff ) - SUCCESS             	       *
*                         (-1)- ERROR		                               *
*                                                                              *
*  Author  : Mi Hyun Noh   						       *
*                                                                              *
*  DATE    : 2005-09-03 						       *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/

int LEAPMain( void )
{
	unsigned char buf[1024];
  	int skfd = -1;
  	struct iwreq wrq;
  	airo_ioctl ai;
 
	char buff[5];
/*		
	if ( argc < 4 ) {
		printf( "leap <iface> <user> <pw>\n" );
		return 1;
	}
*/

	printf( "---LEAP Main start!! ---\n" );
	
	skfd = socket( AF_INET, SOCK_DGRAM, 0 );
	
	if( skfd < 0 ) 
	{
		perror( "socket" );
      		return -1;
	}
	
	bzero( &buf, sizeof( buf ) );

//  	strcpy( wrq.ifr_name, argv[1] );
    memset( buff, 0x00, sizeof( buff ) );
  	memcpy( buff, "eth0", 4 );	
  	strcpy( wrq.ifr_name, buff );	
	
	wrq.u.data.pointer = (caddr_t)&ai;
  	ai.len = sizeof( buf );
	ai.data = buf;

  	if( ioctl( skfd, AIROIDIFC, &wrq ) < 0 ) 
  	{
		perror( "AIROIDIFC" );
		close( skfd );
		return( -1 );
  	}
	close( skfd );
	
  	if ( *( int * )buf != AIROMAGIC ) 
  	{
		fprintf ( stderr, "Aironet extensions not found\n" );
		return( -1 );
  	}

	return initLEAPMode( buff );
}