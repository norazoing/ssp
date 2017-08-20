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
*  PROGRAM ID :       check_valid_file_mgt.c                                   *
*                                                                              *
*  DESCRIPTION:       이 프로그램은 다운로드 받은 파일의 유효성을 체크하는     *
*                     함수를 제공한다.  	   								   *
*                                                                              *
*  ENTRY POINT:       short CheckValidFileForDownload( int achDCSCommFileNo,   *
*                         char* pchFileName, DOWN_FILE_INFO* stDownFileInfo )  *
*																			   *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: None                                                     *
*                                                                              *
*  INPUT FILES:       	c_dr_pro.dat                                           *
*						c_en_pro.dat                                           *
*						c_ex_pro.dat                                           *
*						c_vo.dat	                                           *
*						c_op_par.dat                                           *
*						c_li_par.dat                                           *
*						c_n_far.dat                                            *
*						c_st_inf.dat                                           *
*						c_ap_inf.dat                                           *
*						c_dp_inf.dat                                           *
*						c_de_inf.dat                                           *
*						c_ho_inf.dat                                           *
*						c_idcenter.dat										   *
*						c_keyset.dat										   *
*						c_tr_inf.dat                                           *
*						c_cd_inf.dat                                           *
*                                                                              *
*  OUTPUT FILES:                                                    		   *
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

/*******************************************************************************
*  Inclusion of System Header Files                                            *
*******************************************************************************/
#include "check_valid_file_mgt.h"
#include "download_file_mgt.h"

/*******************************************************************************
*  Declaration of variables                                                    *
*******************************************************************************/
#define STATION_MAX                 210
#define PREPAY_ISSUER_MAX           999
#define	APPLICATION_VER_LEN			4

static  int anTranspMethodCode[38] =
                            {100, 200, 300, 101, 102, 103, 104, 105, 106, 110,
                            115, 120, 130, 140, 201, 202, 203, 204, 301, 302,
                            303, 121, 122, 131, 151, 107, 108, 111, 112, 116,
                            117, 123, 124, 132, 133, 141, 142, 143};

TEMP_EPURSE_ISSUER_REGIST_INFO_HEADER stTmpEpurseIssuerRegistInfoHeader;
TEMP_EPURSE_ISSUER_REGIST_INFO      stTmpEpurseIssuerRegistInfo;
TEMP_PSAM_KEYSET_INFO_HEADER        stTmpPSAMKeysetInfoHeader;
TEMP_PSAM_KEYSET_INFO               stTmpPSAMKeysetInfo;


/*******************************************************************************
*  Declaration of Module Functions                                             *
*******************************************************************************/
static short CheckValidFileVer( int fdFile, char* pchFileName );
static short CheckValidFileDtimeVer( int fdFile, char* pchFileName );
static short CheckValidOperParmFile( int fdFile,
                                     char* pchFileName,
                                     long lFileSize );
static short CheckValidRouteParmFile( int fdFile,
									  char* pchFileName,
                                      long lFileSize );
static short CheckValidStationInfoFile( int fdFile,
										char* pchFileName,
                                        long lFileSize );
static short CheckValidDisExtraInfoFile( int fdFile,
										 byte *abFileName,
                                         dword dwFileSize );
static short CheckValidHolidayInfoFile( int fdFile,
										byte *abFileName,
                                        dword dwFileSize );
static short CheckValidPrepayIssuerInfoFile( int fdFile,
												byte *abFileName,
                                                dword dwFileSize );
static short CheckValidPostpayIssuerInfoFile( int fdFile,
											  byte *abFileName,
                                              dword dwFileSize );
static short CheckValidXferApplyInfoFile( int fdFile,
										  byte *abFileName,
                                          dword dwFileSize );
static short CheckValidEpurseIssuerRegistInfoFile( int fdFile,
                                                   char* pchFileName,
                                                   long lFileSize );
static short CheckValidNewFareInfoFile( int fdFile,
										byte *abFileName,
                                        dword dwFileSize );
static short CheckValidPSAMKeysetInfoFile( int fdFile,
										   char* pchFileName,
                                           long lFileSize );
static short CheckValidPLFile( long lFileSize ) ;
static short CheckValidSCPrepayPLFile( long lFileSize );

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      CheckValidFileForDownload                                *
*                                                                              *
*  DESCRIPTION :      파일별 유효성을 체크한다.                            	   *
*                                                                              *
*  INPUT PARAMETERS:  	int achDCSCommFileNo, - 파일 번호					   *
*						char* pchFileName,    - 파일명             			   *
*             			DOWN_FILE_INFO* stDownFileInfo - 다운로드 정보 구조체  *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*           			ERR_DCS_COMM_NUM 									   *
*						ERR_NOT_DOWN_COMPL                  				   *
*						ERR_FILE_SIZE | GetFileNo( pchFileName )			   *
*						ERR_FILE_OPEN | GetFileNo( pchFileName )			   *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short CheckValidFileForDownload( int nDCSCommFileNo, char* pchFileName,
	DOWN_FILE_INFO* pstDownFileInfo )
{
	short sReturnVal = SUCCESS;
    DOWN_FILE_INFO  stTmpDownFileInfo;

	/*
	 *	DCS_COMM_FILE_CNT보다 같거나 크면 다운로드와 관련 없는 파일로 간주한다.
	 */
    if ( nDCSCommFileNo >= DCS_COMM_FILE_CNT  )
    {
        return ErrRet( ERR_DCS_COMM_NUM );
    }

	/*
	 *	다운로드가 완료가 되지 않았으면 리턴
	 */
    if ( pstDownFileInfo->chDownStatus != DOWN_COMPL )
    {
        return ErrRet( ERR_NOT_DOWN_COMPL );
    }

	sReturnVal = CheckValidFile( nDCSCommFileNo, pchFileName );
	/*
	 *	유효하지 않은 파일이면 파일을 삭제하고 다운로드파일 목록에서
	 *	해당 레코드를 삭제한다.
	 */
    if ( sReturnVal < SUCCESS )
    {
        printf( "[CheckValidFileForDownload] 유효성체크 실패 및 삭제 : [%s]\n",
			pchFileName );
        DelDownFileList( pchFileName,  &stTmpDownFileInfo );
        unlink( pchFileName );
    }

    return sReturnVal;
}


short CheckValidFile( int nDCSCommFileNo, byte *abFileName )
{
	short sResult = SUCCESS;
	int fdFile = 0;
	long lFileSize = 0;

	lFileSize = GetFileSize( abFileName );
	if ( lFileSize == 0 )
	{
		return ErrRet( ERR_FILE_SIZE | GetFileNo( abFileName ) );
	}

	fdFile = open( abFileName, O_RDONLY, OPENMODE );
	if ( fdFile < 0 )
	{
		return ErrRet( ERR_FILE_OPEN | GetFileNo( abFileName )  );
	}

	switch ( nDCSCommFileNo )
	{
		case 0:    //  driver operator
            sResult = CheckValidFileVer( fdFile, abFileName );
		    break;
	    case 1:	//  main  terminal
		    sResult = CheckValidFileDtimeVer( fdFile, abFileName );
		    break;
	    case 2:	//  sub  terminal
		    sResult = CheckValidFileVer( fdFile, abFileName );
		    break;
	    case 10:   //  voice
		    sResult = CheckValidFileVer( fdFile, abFileName );
		    break;
	    case 20:   //  operation parameter
		    sResult = CheckValidOperParmFile( fdFile, abFileName, lFileSize );
		    break;
	    case 21:   //  routeparameter
		    sResult = CheckValidRouteParmFile( fdFile, abFileName,
				lFileSize );
		    break;
	    case 22:   //  faredata
		    sResult = CheckValidNewFareInfoFile( fdFile, abFileName,
				lFileSize );
		    break;
	    case 26:   //   stationdata
		    sResult = CheckValidStationInfoFile( fdFile, abFileName,
				lFileSize );
		    break;
	    case 27:   //   prepay  issuerdata
		    sResult = CheckValidPrepayIssuerInfoFile( fdFile, abFileName,
				lFileSize );
		    break;
	    case 28:   //   postpay issuerdata
		    sResult = CheckValidPostpayIssuerInfoFile( fdFile, abFileName,
				lFileSize );
		    break;
	    case 29:   //   discount/extra data
		    sResult = CheckValidDisExtraInfoFile( fdFile, abFileName,
				lFileSize );
		    break;
	    case 30:   //  holiday data
		    sResult = CheckValidHolidayInfoFile( fdFile, abFileName,
				lFileSize );
		    break;
	    case 32:   //   electronic purse Issuer registration data
		    sResult = CheckValidEpurseIssuerRegistInfoFile( fdFile, abFileName,
				lFileSize );
		    break;
	    case 33:   //   paymentSAM keyset
		    sResult = CheckValidPSAMKeysetInfoFile( fdFile, abFileName,
				lFileSize );
		    break;
	    case 36:   //  transfer apply
		    sResult = CheckValidXferApplyInfoFile( fdFile, abFileName,
				lFileSize );
		    break;
        case  41:						// 구선불PL
        case  42:						// 후불PL
            sResult = CheckValidPLFile( lFileSize );
            break;
        case  45:						// 신선불PL
            sResult = CheckValidSCPrepayPLFile( lFileSize );
            break;
	    default:
		    break;
	}

	close( fdFile );
	
	return sResult;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      CheckValidFileVer                                        *
*                                                                              *
*  DESCRIPTION :      파일의 끝에 날짜로 구성된 버전정보(14byte)와 어플리케이션   *
*						버전정보	 (4byte)가 유효한지 체크                        *
*																			   *
*  INPUT PARAMETERS:  int fdFile, char* pchFileName                            *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*           			ErrRet( ERR_FILE_READ  | GetFileNo( pchFileName ) )    *
*						ErrRet( ERR_DATA_TYPE_NUM )							   *
*						ErrRet( ERR_DATA_TYPE_DATE )						   *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS : 																   *
*                                                                              *
*******************************************************************************/
static short CheckValidFileVer( int fdFile, char* pchFileName )
{
    char    achCheckInfo[VER_INFO_LENGTH + APPLICATION_VER_LEN + 1]	= { 0, };
    int     nReadByte 			  = -1;

	/*
	 *	해당 파일의 끝에서 18자리 앞으로 seek
	 */
    lseek( fdFile, -(VER_INFO_LENGTH + APPLICATION_VER_LEN), SEEK_END );

    nReadByte = read( fdFile,
    				  achCheckInfo,
    				  VER_INFO_LENGTH + APPLICATION_VER_LEN );

    DebugOutlnASC( "\n\t\t -- achCheckInfo -->",
    				achCheckInfo,
                    sizeof(achCheckInfo));

    if ( nReadByte != ( VER_INFO_LENGTH + APPLICATION_VER_LEN ) )
    {
        DebugOut( "read error [%s]\r\n", pchFileName  );
        return ErrRet( ERR_FILE_READ  | GetFileNo( pchFileName ) );
    }

	/*
	 *	숫자로만 구성되었는지의 여부체크 - 버전 ex)0401
	 */
    if ( IsDigitASC( &achCheckInfo[VER_INFO_LENGTH], APPLICATION_VER_LEN
    			   ) == FALSE )
    {
        DebugOut( "app num error [%s]\r\n", pchFileName  );
        return ErrRet( ERR_DATA_TYPE_NUM );
    }

	/*
	 *	ASCII로 된 날짜인지 체크
	 */
    if ( IsValidASCDtime( achCheckInfo ) == FALSE )
    {
        DebugOut( "datetime error [%s]\r\n", pchFileName  );
        return ErrRet( ERR_DATA_TYPE_DATE );
    }

    return SUCCESS;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      CheckValidFileDtimeVer                                   *
*                                                                              *
*  DESCRIPTION :      파일의 끝에 날짜로 구성된 버전정보(14byte)가 유효한지 체크  *
*                                                                              *
*  INPUT PARAMETERS:  int fdFile, char* pchFileName                            *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*           			ErrRet( ERR_FILE_READ  | GetFileNo( pchFileName ) )    *
*						ErrRet( ERR_DATA_TYPE_DATE )						   *
*
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short CheckValidFileDtimeVer( int fdFile, char* pchFileName )
{

    char    achCheckInfo[VER_INFO_LENGTH + 1] 	= { 0, };
    int     nReadByte 			= -1;

	/*
	 *	해당 파일의 끝에서 14자리 앞으로 seek
	 */
    lseek( fdFile,  -VER_INFO_LENGTH,  SEEK_END );

    nReadByte = read( fdFile, achCheckInfo, VER_INFO_LENGTH );

    DebugOutlnASC( "\n\t\t -- achCheckInfo -->\n",
    			   achCheckInfo,
    			   sizeof(achCheckInfo));

    if ( nReadByte != VER_INFO_LENGTH )
    {
        DebugOut( "read error [%s]\r\n", pchFileName  );
        return ErrRet( ERR_FILE_READ | GetFileNo( pchFileName )  );
    }

	/*
	 *	ASCII로 된 날짜인지 체크
	 */
    if ( IsValidASCDtime( achCheckInfo ) == FALSE )
    {
        DebugOut( "datetime error [%s]\r\n", pchFileName  );
        return ErrRet( ERR_DATA_TYPE_DATE );
    }

    return SUCCESS;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      CheckValidOperParmFile                                   *
*                                                                              *
*  DESCRIPTION :      차량 운영파라미터 파일의 유효성을 체크 				       *
*                                                                              *
*  INPUT PARAMETERS:  int fdFile, char* pchFileName, long lFileSize            *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*						ErrRet( ERR_FILE_SIZE | GetFileNo(VEHICLE_PARM_FILE) ) *
*              			ErrRet( ERR_FILE_READ | GetFileNo(VEHICLE_PARM_FILE) ) *
*						ErrRet( ERR_DATA_TYPE_DATE )						   *
*						ErrRet( ERR_DATA )									   *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short CheckValidOperParmFile( int fdFile,
                                     char* pchFileName,
                                     long lFileSize )
{

    int     nReadByte 				= -1;
    int     i 						= 0;
    int     nTranspMethodCode   	= 0;
    int     nTranspMethodCodeCnt 	= 0;

	TEMP_VEHICLE_PARM	stTmpVehicleParm;

    /*
     *	size 체크
     */
    if ( lFileSize != sizeof( stTmpVehicleParm ) + 2 )
    {
        DebugOut( "FileName [%s] size err\r\n", pchFileName );
        return ErrRet( ERR_FILE_SIZE | GetFileNo( VEHICLE_PARM_FILE ) );
    }

    memset( &stTmpVehicleParm, 0x00, sizeof( stTmpVehicleParm ) );

    nReadByte = read( fdFile, &stTmpVehicleParm, sizeof( stTmpVehicleParm ) );

    if ( nReadByte != sizeof( stTmpVehicleParm ) )
    {
        DebugOut( "FileName [%s] size err [%ld]\r\n", pchFileName, lFileSize );
        return ErrRet( ERR_FILE_READ | GetFileNo( VEHICLE_PARM_FILE ) );
    }

	/*
	 *	적용일자가 유효한지 체크
	 */
    if ( IsValidASCDtime( (char*)&stTmpVehicleParm.abApplyDtime ) == FALSE )
    {
        DebugOut( "datetime error [%s]\r\n", pchFileName  );
        return ErrRet( ERR_DATA_TYPE_DATE );
    }

	/*
	 *	운행수단코드 유효성 체크
	 */
    nTranspMethodCodeCnt = sizeof( anTranspMethodCode );

    nTranspMethodCode =
                GetINTFromASC( (byte*)&stTmpVehicleParm.abTranspMethodCode,
                                sizeof( stTmpVehicleParm.abTranspMethodCode) );

    DebugOut( "nTranspMethodCode[%d]\n", nTranspMethodCode );

    for( i = 0 ; i < nTranspMethodCodeCnt ; i++ )
    {

        if ( anTranspMethodCode[i] == nTranspMethodCode )
        {
            break;
        }
    }

    if (  i >= nTranspMethodCodeCnt  )
    {
        DebugOut( "CheckValidOperParmFile TranspMethodCode error [%d]\r \n",
        		  nTranspMethodCode );
        return ErrRet( ERR_DATA );
    }

    return SUCCESS;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      CheckValidRouteParmFile                                  *
*                                                                              *
*  DESCRIPTION :      노선별 파라미터  파일의 유효성을 체크 				       *
*                                                                              *
*  INPUT PARAMETERS:  int fdFile, char* pchFileName, long lFileSize            *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*						ErrRet( ERR_FILE_SIZE | GetFileNo( ROUTE_PARM_FILE )   *
*           			ErrRet( ERR_FILE_READ | GetFileNo( ROUTE_PARM_FILE ) ) *
*						ErrRet( ERR_DATA_TYPE_DATE )						   *
*						ErrRet( ERR_DATA )									   *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short CheckValidRouteParmFile( int fdFile,
									  char* pchFileName,
                                      long lFileSize )
{

    int     nReadByte 				= -1;
    int     i 						= 0;
    int     nTranspMethfodCode  	= 0;    // ASC -> INT conversion
    int     nTranspMethodCodeCnt 	= 0;
	TEMP_ROUTE_PARM	stTmpRouteParm;			// 노선별파라미터 구조체( read용 )

    /*
     *	size 체크
     */
    if ( lFileSize != sizeof( stTmpRouteParm ) + 2  )
    {
        DebugOut( "FileName [%s] size err [%ld]\r\n",
                pchFileName,
                lFileSize );
        return ErrRet( ERR_FILE_SIZE | GetFileNo( ROUTE_PARM_FILE ) );
    }

    memset( &stTmpRouteParm, 0x00, sizeof( stTmpRouteParm ) );

    nReadByte = read( fdFile, &stTmpRouteParm, sizeof( stTmpRouteParm ) );

    if ( nReadByte != sizeof( stTmpRouteParm ) )
    {
        DebugOut( "read error [%s]\r\n", pchFileName  );
        return ErrRet( ERR_FILE_READ | GetFileNo( ROUTE_PARM_FILE ) );
    }

	/*
	 *	적용일자가 유효한지 체크
	 */
    if ( IsValidASCDtime( (char*)&stTmpRouteParm.abApplyDtime ) == FALSE )
    {
        DebugOut( "datetime error [%s]\r\n", pchFileName  );
        return ErrRet( ERR_DATA_TYPE_DATE );
    }

	/*
	 *	운행수단코드 유효성 체크
	 */
    nTranspMethodCodeCnt = sizeof( anTranspMethodCode );
    nTranspMethfodCode =
    			 GetINTFromASC( stTmpRouteParm.abTranspMethodCode,
                                sizeof( stTmpRouteParm.abTranspMethodCode) );

    DebugOut( "nTranspMethfodCode[%d]", nTranspMethfodCode );

    for( i = 0 ; i < nTranspMethodCodeCnt ; i++ )
    {

        if ( anTranspMethodCode[i] == nTranspMethfodCode )
        {
            break;
        }
    }

    if (  i >= nTranspMethodCodeCnt  )
    {
        DebugOut( "CheckValidRouteParmFile TranspMethodCode error [%d]\r\n",
                 anTranspMethodCode[i] );
        return ErrRet( ERR_DATA );
    }

    return SUCCESS;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      CheckValidStationInfoFile                                *
*                                                                              *
*  DESCRIPTION :      정류장정보 파일의 유효성을 체크 					       *
*                                                                              *
*  INPUT PARAMETERS:  int fdFile, char* pchFileName, long lFileSize            *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*            		ErrRet( ERR_FILE_READ | GetFileNo( BUS_STATION_INFO_FILE ) *
*					ErrRet( ERR_DATA )										   *
*					ErrRet( ERR_FILE_SIZE | GetFileNo(BUS_STATION_INFO_FILE) ) *
*					ErrRet( ERR_DATA_TYPE_DATE )							   *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short CheckValidStationInfoFile( int fdFile,
										char* pchFileName,
                                        long lFileSize )
{
    int     nReadByte 			= -1;
    char    achApplyTime[15]	= { 0, };
    int     nRecCnt 			= 0;
	TEMP_STATION_INFO_HEADER	stTmpStationInfoHeader;
	TEMP_STATION_INFO			stTmpStationInfo;

    memset( &stTmpStationInfoHeader, 0x00, sizeof( stTmpStationInfoHeader ) );

    nReadByte = read( fdFile,
    				  &stTmpStationInfoHeader,
                      sizeof( stTmpStationInfoHeader ) );

    if ( nReadByte != sizeof( stTmpStationInfoHeader ) )
    {
        DebugOut( "read error [%s]\r\n", pchFileName  );
        return ErrRet( ERR_FILE_READ | GetFileNo( BUS_STATION_INFO_FILE ) );
    }

    /*
     *	max records 체크
     */
    nRecCnt = GetINTFromASC( (byte*)&stTmpStationInfoHeader.abRecordCnt,
                             sizeof( stTmpStationInfoHeader.abRecordCnt ) );

    if ( nRecCnt > STATION_MAX )
    {
        DebugOut( "CheckValidStationInfoFile too many records\r\n" );
        return ErrRet( ERR_DATA );
    }

    /*
     *	size 체크
     */
    if ( lFileSize != ( sizeof( stTmpStationInfoHeader ) +
                        ( sizeof( stTmpStationInfo )* nRecCnt ) + 2 ) )
    {
        DebugOut( "CheckValidStationInfoFile Tatal filesize error[%ld][%d]\r\n",
                lFileSize,
                ( sizeof( stTmpStationInfoHeader ) +
                  ( sizeof( stTmpStationInfo )* nRecCnt ) + 2 ) );
        return ErrRet( ERR_FILE_SIZE | GetFileNo( BUS_STATION_INFO_FILE ) );
    }

	/*
	 *	적용일자가 유효한지 체크
	 */
    BCD2ASC( (byte*)&stTmpStationInfoHeader.abApplyDtime,
              achApplyTime,
              sizeof( stTmpStationInfoHeader.abApplyDtime ) );

    if ( IsValidASCDtime( achApplyTime ) == FALSE )
    {
        DebugOut( "datetime error [%s]\r\n", pchFileName  );
        return ErrRet( ERR_DATA_TYPE_DATE );
    }

    return SUCCESS;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      CheckValidDisExtraInfoFile                               *
*                                                                              *
*  DESCRIPTION :      할인할증정보 파일 유효성 체크 							   *
*                                                                              *
*  INPUT PARAMETERS:  int fdFile, byte *abFileName, dword dwFileSize           *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS                                                  *
*				ErrRet( ERR_FILE_READ | GetFileNo( DIS_EXTRA_INFO_FILE ) )	   *
*            	ErrRet( ERR_FILE_SIZE | GetFileNo( DIS_EXTRA_INFO_FILE ) )     *
*				ErrRet( ERR_DATA_TYPE_DATE )								   *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short CheckValidDisExtraInfoFile( int fdFile,
										 byte *abFileName,
                                         dword dwFileSize )
{

    TEMP_DIS_EXTRA_INFO_HEADER 	stInfoHeader;
    TEMP_DIS_EXTRA_INFO 		stInfo;

    int 	nReadByte 							= -1;
    dword 	dwRecordCnt 						= 0;
    byte 	abASCApplyDtime[VER_INFO_LENGTH] 	= { 0, };

    /*
     *	header  SIZE check
     */
    memset( &stInfoHeader, 0, sizeof( stInfoHeader ) );
    nReadByte = read( fdFile, &stInfoHeader, sizeof( stInfoHeader ) );

    if ( nReadByte != sizeof( stInfoHeader ) )
    {
        DebugOut( "read error [%s]\r\n", abFileName  );
        return ErrRet( ERR_FILE_READ | GetFileNo( DIS_EXTRA_INFO_FILE ) );
    }

    /*
     *	size 체크
     */
    dwRecordCnt = GetDWORDFromBCD( stInfoHeader.abRecordCnt,
    							   sizeof( stInfoHeader.abRecordCnt ) );

    if ( dwFileSize != ( sizeof( stInfoHeader ) +
                         ( sizeof( stInfo ) * dwRecordCnt ) ) )
    {
        ErrRet( ERR_FILE_SIZE | GetFileNo( DIS_EXTRA_INFO_FILE ) );
    }

	/*
	 *	적용일자가 유효한지 체크
	 */
    BCD2ASC( stInfoHeader.abApplyDtime,
    		 abASCApplyDtime,
    		 sizeof( stInfoHeader.abApplyDtime ) );

    if ( IsValidASCDtime( abASCApplyDtime ) == FALSE )
    {
        DebugOut( "datetime error [%s]\r\n", abFileName );
        ErrRet( ERR_DATA_TYPE_DATE );
    }

    return SUCCESS;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      CheckValidHolidayInfoFile                                *
*                                                                              *
*  DESCRIPTION :      휴일정보 파일 유효성 체크 							       *
*                                                                              *
*  INPUT PARAMETERS:  int fdFile, byte *abFileName, dword dwFileSize           *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*						ErrRet( ERR_FILE_READ | GetFileNo( HOLIDAY_INFO_FILE ))*
*						ErrRet( ERR_FILE_SIZE | GetFileNo( HOLIDAY_INFO_FILE ))*
*           			ErrRet( ERR_DATA_TYPE_DATE )                           *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short CheckValidHolidayInfoFile( int fdFile,
										byte *abFileName,
                                        dword dwFileSize )
{
    TEMP_HOLIDAY_INFO_HEADER 	stInfoHeader;

    int 	nReadByte 							= -1;
    dword 	dwRecordCnt 						= 0;
    byte 	abASCApplyDtime[VER_INFO_LENGTH] 	= { 0, };

    /*
     *	header  SIZE check
     */
    memset( &stInfoHeader, 0, sizeof( stInfoHeader ) );
    nReadByte = read( fdFile, &stInfoHeader, sizeof( stInfoHeader ) );

    if ( nReadByte != sizeof( stInfoHeader ) )
    {
        DebugOut( "read error [%s]\r\n", abFileName );
        return ErrRet( ERR_FILE_READ | GetFileNo( HOLIDAY_INFO_FILE ) );
    }

    /*
     *	size 체크
     */
    dwRecordCnt = GetDWORDFromASC( stInfoHeader.abRecordCnt,
    							   sizeof( stInfoHeader.abRecordCnt ) );

    if ( dwFileSize != ( sizeof( stInfoHeader )
                         + ( sizeof( HOLIDAY_INFO ) * dwRecordCnt ) + 2 ) )
    {
        return ErrRet( ERR_FILE_SIZE | GetFileNo( HOLIDAY_INFO_FILE ) );
    }

	/*
	 *	적용일자가 유효한지 체크
	 */
    BCD2ASC( stInfoHeader.abApplyDtime,
    		 abASCApplyDtime,
    		 sizeof( stInfoHeader.abApplyDtime ) );

    if ( IsValidASCDtime( abASCApplyDtime ) == FALSE )
    {
        ErrRet( ERR_DATA_TYPE_DATE );
    }

    return SUCCESS;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      CheckValidPrepayIssuerInfoFile                           *
*                                                                              *
*  DESCRIPTION :      선불 발행사정보 파일 유효성 체크 						   *
*                                                                              *
*  INPUT PARAMETERS:  int fdFile, byte *abFileName, dword dwFileSize           *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS                                                  *
*        		ErrRet( ERR_FILE_READ | GetFileNo( PREPAY_ISSUER_INFO_FILE ) ) *
*				ErrRet( ERR_DATA )											   *
*				ErrRet( ERR_FILE_SIZE | GetFileNo( PREPAY_ISSUER_INFO_FILE ) ) *
*				ErrRet( ERR_DATA_TYPE_DATE )								   *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short CheckValidPrepayIssuerInfoFile( int fdFile,
												byte *abFileName,
                                                dword dwFileSize )
{
    TEMP_PREPAY_ISSUER_INFO_HEADER		stInfoHeader;
    TEMP_PREPAY_ISSUER_INFO 			stInfo;

    int 	nReadByte 							= -1;
    dword 	dwRecordCnt 						= 0;
    byte 	abASCApplyDtime[VER_INFO_LENGTH] 	= { 0, };

    /*
     *	header  SIZE check
     */
    memset( &stInfoHeader, 0, sizeof( stInfoHeader ) );
    nReadByte = read( fdFile, &stInfoHeader, sizeof( stInfoHeader ) );

    if ( nReadByte != sizeof( stInfoHeader ) )
    {
        DebugOut( "read error [%s]\r\n", abFileName );
        return ErrRet( ERR_FILE_READ | GetFileNo( PREPAY_ISSUER_INFO_FILE ) );
    }

    /*
     *	maximum record  count check
     */
    dwRecordCnt = GetDWORDFromASC( stInfoHeader.abRecordCnt,
    							   sizeof( stInfoHeader.abRecordCnt ) );

    if ( dwRecordCnt > PREPAY_ISSUER_MAX )
    {
        DebugOut( "CheckValidPrepayIssuerInfo too many records\r\n" );
        return ErrRet( ERR_DATA );
    }

    /*
     *	size 체크
     */
    if ( dwFileSize != ( sizeof( stInfoHeader ) +
                            ( sizeof( stInfo ) * dwRecordCnt ) + 2 ) )
    {
        DebugOut( "CheckValidPrepayIssuerInfo Tatal filesize error[%lu]
                    [%lu]\r\n",
                    dwFileSize, ( sizeof( stInfoHeader )
                    + ( sizeof( stInfo ) * dwRecordCnt ) + 2 ) );
        return ErrRet( ERR_FILE_SIZE | GetFileNo( PREPAY_ISSUER_INFO_FILE ) );
    }

	/*
	 *	적용일자가 유효한지 체크
	 */
    BCD2ASC( stInfoHeader.abApplyDtime, abASCApplyDtime, 7 );

    if ( IsValidASCDtime( abASCApplyDtime ) == FALSE )
    {
        DebugOut( "datetime error [%s]\r\n", abFileName );
        return ErrRet( ERR_DATA_TYPE_DATE );
    }

    return SUCCESS;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      CheckValidPostpayIssuerInfoFile                          *
*                                                                              *
*  DESCRIPTION :      후불발행사 정보파일 유효성 체크					           *
*                                                                              *
*  INPUT PARAMETERS:  int fdFile, byte *abFileName, dword dwFileSize           *
*                                                                              *
*  RETURN/EXIT VALUE:  SUCCESS                                                 *
*				ErrRet( ERR_FILE_READ | GetFileNo( POSTPAY_ISSUER_INFO_FILE ) )*
*       		ErrRet( ERR_FILE_SIZE | GetFileNo( POSTPAY_ISSUER_INFO_FILE ) )*
*       		ErrRet(ERR_DATA_TYPE_DATE )                                    *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short CheckValidPostpayIssuerInfoFile( int fdFile,
											  byte *abFileName,
                                              dword dwFileSize )
{
    TEMP_POSTPAY_ISSUER_INFO_HEADER 	stInfoHeader;
    TEMP_POSTPAY_ISSUER_INFO 			stInfo;

    int 	nReadByte 							= -1;
    dword 	dwRecordCnt 						= 0;
    byte 	abASCApplyDtime[VER_INFO_LENGTH] 	= { 0, };

    /*
     *	header  SIZE check
     */
    memset( &stInfoHeader, 0, sizeof( stInfoHeader ) );
    nReadByte = read( fdFile, &stInfoHeader, sizeof( stInfoHeader ) );

    if ( nReadByte != sizeof( stInfoHeader ) )
    {
        DebugOut( "read error [%s]\r\n", abFileName );
        return ErrRet( ERR_FILE_READ | GetFileNo( POSTPAY_ISSUER_INFO_FILE ) );
    }

    /*
     *	size 체크
     */
    dwRecordCnt = stInfoHeader.wRecordCnt;
    if ( dwFileSize != ( sizeof( stInfoHeader )
                         + ( sizeof( stInfo ) * dwRecordCnt ) + 2 ) )
    {
        DebugOut( "CheckValidPostpayIssuerInfo Tatal filesize error
                    [%lu] [%lu]\r\n",
                    dwFileSize, ( sizeof( stInfoHeader )
                    + ( sizeof( stInfo ) * dwRecordCnt ) + 2 ) );
        return ErrRet( ERR_FILE_SIZE | GetFileNo( POSTPAY_ISSUER_INFO_FILE ) );
    }

	/*
	 *	적용일자가 유효한지 체크
	 */
    BCD2ASC( stInfoHeader.abApplyDtime,
    		 abASCApplyDtime,
    		 sizeof( stInfoHeader.abApplyDtime ) );

    if ( IsValidASCDtime( abASCApplyDtime ) == FALSE )
    {
        DebugOut( "datetime error [%s]\r\n", abFileName );
        return ErrRet( ERR_DATA_TYPE_DATE );
    }

    return SUCCESS;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      CheckValidXferApplyInfoFile                              *
*                                                                              *
*  DESCRIPTION :      환승적용정보 파일 유효성 체크 					           *
*                                                                              *
*  INPUT PARAMETERS:  int fdFile, byte *abFileName, dword dwFileSize           *
*                                                                              *
*  RETURN/EXIT VALUE:  SUCCESS                                                 *
*				ErrRet( ERR_FILE_SIZE | GetFileNo( XFER_APPLY_INFO_FILE ) )    *
*       		ErrRet( ERR_FILE_READ | GetFileNo( XFER_APPLY_INFO_FILE ) )    *
*       		ErrRet( ERR_DATA_TYPE_DATE )                                   *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short CheckValidXferApplyInfoFile( int fdFile,
										  byte *abFileName,
                                          dword dwFileSize )
{
    TEMP_XFER_APPLY_INFO stInfo;

    int 	nReadByte 			= -1;
    byte 	abASCApplyDtime[VER_INFO_LENGTH] = { 0, };

    /*
     *	size 체크
     */
    if ( ( dwFileSize % sizeof( stInfo ) ) != 0 )
    {
        DebugOut( "CheckValidXferApplyInfo Tatal filesize error\r\n" );
        return ErrRet( ERR_FILE_SIZE | GetFileNo( XFER_APPLY_INFO_FILE ) );
    }

    /*
     *	read file
     */
    nReadByte = read( fdFile, &stInfo, sizeof( stInfo ) );
    if ( nReadByte != sizeof( stInfo ) )
    {
        DebugOut( "read error %d %d [%s]\r\n", nReadByte, sizeof( stInfo ),
                abFileName );
        ErrRet( ERR_FILE_READ | GetFileNo( XFER_APPLY_INFO_FILE ) );
    }

	/*
	 *	ASCII로 된 날짜인지 체크
	 */
    BCD2ASC( stInfo.abApplyDtime, abASCApplyDtime, 7 );
    if ( IsValidASCDtime( abASCApplyDtime ) == FALSE )
    {
        DebugOut( "datetime error [%s]\r\n", abFileName );
        ErrRet( ERR_DATA_TYPE_DATE);
    }

    return SUCCESS;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      CheckValidEpurseIssuerRegistInfoFile                     *
*                                                                              *
*  DESCRIPTION :      발행사 등록정보 파일 유효성 체크                           *
*                                                                              *
*  INPUT PARAMETERS:  int fdFile, char* pchFileName, long lFileSize            *
*                                                                              *
*  RETURN/EXIT VALUE:  SUCCESS                                                 *
*   	ErrRet( ERR_FILE_READ | GetFileNo( EPURSE_ISSUER_REGIST_INFO_FILE ) )  *
*   	ErrRet( ERR_FILE_SIZE | GetFileNo( EPURSE_ISSUER_REGIST_INFO_FILE )	   *
*   	ErrRet( ERR_DATA_TYPE_DATE)                                            *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short CheckValidEpurseIssuerRegistInfoFile( int fdFile,
                                                   char* pchFileName,
                                                   long lFileSize )
{
    int     nReadByte 		= -1;
    int     nRecCnt 		= 0;        // ASC -> INT conversion
    char    achApplyTime[VER_INFO_LENGTH + 1] = { 0, };
	TEMP_EPURSE_ISSUER_REGIST_INFO_HEADER 	stTmpEpurseIssuerRegistInfoHeader;
	TEMP_EPURSE_ISSUER_REGIST_INFO      	stTmpEpurseIssuerRegistInfo;

    /*
     *	read file
     */
    nReadByte = read( fdFile,
    				  &stTmpEpurseIssuerRegistInfoHeader,
                      sizeof( stTmpEpurseIssuerRegistInfoHeader ) );

    nReadByte = nReadByte + read( fdFile,
    							  &stTmpEpurseIssuerRegistInfo,
                                  sizeof( stTmpEpurseIssuerRegistInfo ) );

    if ( nReadByte != ( sizeof( stTmpEpurseIssuerRegistInfoHeader ) +
                        sizeof( stTmpEpurseIssuerRegistInfo ) ) )
    {
        DebugOut( "read error [%s]\r\n", pchFileName  );
        return ErrRet( ERR_FILE_READ |
                       GetFileNo( EPURSE_ISSUER_REGIST_INFO_FILE ) );
    }

    /*
     *	max records 체크
     */
    nRecCnt =
       GetINTFromASC( (byte*)&stTmpEpurseIssuerRegistInfoHeader.abRecordCnt,
                      sizeof( stTmpEpurseIssuerRegistInfoHeader.abRecordCnt ) );

    /*
     *	size 체크
     */
    if ( lFileSize != ( sizeof( stTmpEpurseIssuerRegistInfoHeader ) +
                        ( sizeof( stTmpEpurseIssuerRegistInfo ) * nRecCnt ) ) )
    {
        return ErrRet( ERR_FILE_SIZE |
                       GetFileNo( EPURSE_ISSUER_REGIST_INFO_FILE ) );
    }

	/*
	 *	적용일자가 유효한지 체크
	 */
    BCD2ASC( (byte*)&stTmpEpurseIssuerRegistInfoHeader.abApplyDtime,
             achApplyTime,sizeof(
             stTmpEpurseIssuerRegistInfoHeader.abApplyDtime ) );

    if ( IsValidASCDtime( achApplyTime ) == FALSE )
    {
        DebugOut( "datetime error [%s]\r\n", pchFileName  );
        return ErrRet( ERR_DATA_TYPE_DATE );
    }

    return SUCCESS;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      CheckValidNewFareInfoFile                                *
*                                                                              *
*  DESCRIPTION :      요금정보 파일 유효성 체크 							       *
*                                                                              *
*  INPUT PARAMETERS:  int fdFile, byte *abFileName, dword dwFileSize           *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS                                                  *
*					ErrRet( ERR_FILE_SIZE | GetFileNo( NEW_FARE_INFO_FILE ) )  *
*             		ErrRet( ERR_FILE_READ | GetFileNo( NEW_FARE_INFO_FILE ) )  *
*             		ErrRet( ERR_DATA_TYPE_DATE )                               *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short CheckValidNewFareInfoFile( int fdFile,
										byte *abFileName,
                                        dword dwFileSize )
{

    TEMP_NEW_FARE_INFO stInfo;
    int 	nReadByte 							= -1;
    byte 	abASCApplyDtime[VER_INFO_LENGTH] 	= { 0, };

    /*
     *	size 체크
     */
    if ( (dwFileSize % sizeof( stInfo )) != 0 )
    {
        DebugOut( "CheckValidNewFareInfo Tatal filesize error\r\n" );
        return ErrRet( ERR_FILE_SIZE | GetFileNo( NEW_FARE_INFO_FILE ) );
    }

    /*
     *	read file
     */
    nReadByte = read(fdFile, &stInfo, sizeof(stInfo));
    if (nReadByte != sizeof(stInfo))
    {
        DebugOut( "read error %d %d [%s]\r\n", nReadByte,
                sizeof(stInfo), abFileName );
        ErrRet( ERR_FILE_READ | GetFileNo( NEW_FARE_INFO_FILE ) );
    }

	/*
	 *	적용일자가 유효한지 체크
	 */
    BCD2ASC(stInfo.abApplyDtime, abASCApplyDtime, 7);
    if (IsValidASCDtime(abASCApplyDtime) == FALSE)
    {
        DebugOut( "datetime error [%s]\r\n", abFileName );
        ErrRet(ERR_DATA_TYPE_DATE);
    }

    return SUCCESS;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      CheckValidPSAMKeysetInfoFile                             *
*                                                                              *
*  DESCRIPTION :      지불SAM keyset정보 파일 유효성 체크 					   *
*                                                                              *
*  INPUT PARAMETERS:  int fdFile, char* pchFileName, long lFileSize            *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS                                                  *
*				ErrRet( ERR_FILE_READ | GetFileNo( PSAM_KEYSET_INFO_FILE ) )   *
*          		ErrRet( ERR_FILE_SIZE | GetFileNo( PSAM_KEYSET_INFO_FILE ) )   *
*          		ErrRet(ERR_DATA_TYPE_DATE)                                     *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short CheckValidPSAMKeysetInfoFile( int fdFile,
										   char* pchFileName,
                                           long lFileSize )
{
    int     nReadByte 			= -1;
    int     nRecCnt 			= 0;        // ASC -> INT conversion
    char    achApplyTime[VER_INFO_LENGTH+1]	= { 0, };
	TEMP_PSAM_KEYSET_INFO_HEADER	stTmpPSAMKeysetInfoHeader;
	TEMP_PSAM_KEYSET_INFO			stTmpPSAMKeysetInfo;

    /*
     *	read file
     */
    nReadByte = read( fdFile,
                &stTmpPSAMKeysetInfoHeader,
                sizeof( stTmpPSAMKeysetInfoHeader ) );
    nReadByte = nReadByte + read( fdFile,
                &stTmpPSAMKeysetInfo,
                sizeof( stTmpPSAMKeysetInfo ) );

    if ( nReadByte != ( sizeof( stTmpPSAMKeysetInfoHeader ) +
                        sizeof( stTmpPSAMKeysetInfo ) ) )
    {
        DebugOut( "read error [%s]\r\n", pchFileName  );
        return ErrRet( ERR_FILE_READ | GetFileNo( PSAM_KEYSET_INFO_FILE ) );
    }

    /*
     *	max records 체크
     */
    nRecCnt =
	        GetINTFromASC( (byte*)&stTmpPSAMKeysetInfoHeader.abRecordCnt,
	                        sizeof( stTmpPSAMKeysetInfoHeader.abRecordCnt ) );

    /*
     *	size 체크
     */
    if ( lFileSize != ( sizeof( stTmpPSAMKeysetInfoHeader ) +
                      ( sizeof( stTmpPSAMKeysetInfo )
                      * nRecCnt ) ) )
    {
        return ErrRet( ERR_FILE_SIZE | GetFileNo( PSAM_KEYSET_INFO_FILE ) );
    }

	/*
	 *	적용일자가 유효한지 체크
	 */
    BCD2ASC( (byte*)&stTmpPSAMKeysetInfoHeader.abApplyDtime,
             achApplyTime,
             sizeof( stTmpPSAMKeysetInfoHeader.abApplyDtime ) );

    if ( IsValidASCDtime( achApplyTime ) == FALSE )
    {
        DebugOut( "datetime error [%s]\r\n", pchFileName  );
        return ErrRet( ERR_DATA_TYPE_DATE );
    }

    return SUCCESS;

}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      CheckValidPLFile                             			   *
*                                                                              *
*  DESCRIPTION :      PL file size check					   				   *
*                                                                              *
*  INPUT PARAMETERS:  long lFileSize            							   *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS                                                  *
*          		ErrRet( ERR_FILE_SIZE )   									   *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2006-10-24                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short CheckValidPLFile( long lFileSize ) 
{

	if ( lFileSize != 7529472 )
	{
		return ErrRet( ERR_FILE_SIZE );
	}
	
   	return SUCCESS;
}

static short CheckValidSCPrepayPLFile( long lFileSize ) 
{

	if ( lFileSize != 5913088 )
	{
		return ErrRet( ERR_FILE_SIZE );
	}
	
   	return SUCCESS;
}
