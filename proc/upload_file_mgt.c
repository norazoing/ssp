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
*  PROGRAM ID :       upload_file_mgt.c                                       *
*                                                                              *
*  DESCRIPTION:       이 프로그램은 단말기 설치정보와 버전정보의 생성 기능을 	   *
*						제공한다.       										   *
*                                                                              *
*  ENTRY POINT:       CreateInstallInfoFile                                    *
*                     SetUploadVerInfo                                         *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: None                                                     *
*                                                                              *
*  INPUT FILES:       None			                                           *
*                                                                              *
*  OUTPUT FILES:      install.dat                                              *
*                     version.trn                                              *
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
#include "upload_file_mgt.h"
#include "../comm/socket_comm.h"
#include "../system/device_interface.h"
#include "reconcile_file_mgt.h"
#include "version_file_mgt.h"
#include "Blpl_proc.h"

/*******************************************************************************
*  Declaration of variables                                                    *
*******************************************************************************/
#define     VERSION                 'V'         // main appl version prefix
#define     IP_CLASS_CODE           "DH"        // DHCP 방식
#define     YES                     'Y'         // PSAM 존재 여부(있음)
#define     NO                      'N'         // PSAM 존재 여부(없음0)
#define		PSAM_INIT_VALUE			"0000000000000000"
#define		TERM_ID_INIT_VALUE		"000000000"

static int  nTermIDLength = sizeof( gpstSharedInfo->abMainTermID );
static int  nPSAMIDLength = sizeof( gpstSharedInfo->abMainPSAMID );

/*******************************************************************************
*  Declaration of Module Functions                                             *
*******************************************************************************/
static short CopyVerInfo2SendUse( UPLOAD_VER_INFO* pstUploadVerInfo );

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      CreateInstallInfoFile                                    *
*                                                                              *
*  DESCRIPTION :      단말기 정보를 모아 install.dat 파일로 만든다.			   *
*                                                                              *
*  INPUT PARAMETERS:    void                                                   *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       ErrRet( ERR_FILE_OPEN | GetFileNo( INSTALL_INFO_FILE)) *
*						ErrRet( ERR_FILE_WRITE | GetFileNo( INSTALL_INFO_FILE))*
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short CreateInstallInfoFile( void )
{
    short   sReturnVal          = SUCCESS;          // return value
    int     fdInstallInfoFile   = 0;                // install.dat
    int     fdPGVERFile         = 0;                // "pgver.dat"

    byte    abLocalIP[13]       = { 0, };           // Terminal Local IP
    int     i     				= 0;                // Sub Term Loop
    DWORD   dwFlashMemFreeSize  = 0;                // FLASH MEMORY Free Size
    byte    bFlashMemFreeSize[4]= { SPACE, };       // FLASH MEMORY Free Size
    char    achPGLoaderVer[5]   = { SPACE, };       // PG Loader Version
    byte    bPSAMCnt            = 0;                // PSAM Count
    time_t  tCurrTime;                              // current time
    byte    abTmpInstallInfo[200]   = { 0, };
    int     nTmpInstallInfoSize     = 0;            // copy할 size
    int		nWriteLen				= 0;
    RECONCILE_DATA stRecData;                       // reconcile 등록할 구조체
    INSTALL_INFO   stInstallInfo;                   // 생성할 파일의 구조체

    memset( stInstallInfo.abSamIDList,
    		SPACE,
            sizeof( stInstallInfo.abSamIDList ) );
            
    /*
     *  0x41('A')로 초기화
     *  web에 표시할 데이터( PSAM 미존재시 )
     */
    memset( stInstallInfo.achPSAMPort,
            0x41,
            sizeof( stInstallInfo.achPSAMPort ) );

    memset( stInstallInfo.achSubTermPSAMExistYN,
            0x41,
            sizeof( stInstallInfo.achSubTermPSAMExistYN ) );

    /*
     *  main term application version
     */
    stInstallInfo.achTermApplVer[0] = VERSION;
    memcpy( &stInstallInfo.achTermApplVer[1],
    		MAIN_RELEASE_VER, 
    		sizeof( stInstallInfo.achTermApplVer ) - 1 );

    /*
     * terminal ID
     */
    memcpy( stInstallInfo.achMainTermID,
            gpstSharedInfo->abMainTermID,
            sizeof( stInstallInfo.achMainTermID ) );

    /*
     * TranspBizr ID
     */
    memcpy( stInstallInfo.achTranspBizrID,
            gstVehicleParm.abTranspBizrID,
            sizeof( stInstallInfo.achTranspBizrID ) );

    /*
     * Vehicle ID
     */
    memcpy( stInstallInfo.achVehicleID,
            gstVehicleParm.abVehicleID,
            sizeof( stInstallInfo.achVehicleID ) );

    /*
     * terminal setup time
     */
    GetRTCTime( &tCurrTime );
    TimeT2BCDDtime( tCurrTime, stInstallInfo.abTermSetupDtime );

    /*
     * transp method code
     */
    memcpy( stInstallInfo.abTranspMethodCode,
            gstVehicleParm.abTranspMethodCode,
            sizeof( stInstallInfo.abTranspMethodCode ) );

    /*
     *  only main  terminal creates install.dat file
     */
    memcpy( stInstallInfo.achDeviceClassCode,
            DEVICE_CLASS_MAIN,
            sizeof( stInstallInfo.achDeviceClassCode ) );

    /*
     *  DCS IP ADDR
     */
    memcpy( stInstallInfo.achDCSIPAddr,
            gstCommInfo.abDCSIPAddr,
            sizeof( stInstallInfo.achDCSIPAddr ) );

    /*
     * IP address  class code
     */
    memcpy( stInstallInfo.achIPAddrClassCode,
            IP_CLASS_CODE,
            sizeof( stInstallInfo.achIPAddrClassCode ) );

    /*
     * DHCP allocated IP  address
     */
    GetLocalIP( abLocalIP );
    memcpy( stInstallInfo.achLocalIPAddr,
            abLocalIP,
            sizeof( stInstallInfo.achLocalIPAddr ) );

	/*
	 * gpstSharedInfo->bMainPSAMPort
	 * gpstSharedInfo->abSubPSAMPort
	 * 위의 두 변수에는 PSAM 포트가 '0' ~ '3'의 값이 설정되어 있으며,
	 * 웹에서는 '1' ~ '4'의 값으로 표시되어야 하므로
	 * install.dat 파일에는 1을 더하여 설정한다.
	 */

    /*
     *  MainTerm SAM Port
     *  web에서의 포트번호는 bus terminal psamport + 1을 해줘야함
     */
    stInstallInfo.achPSAMPort[0] = gpstSharedInfo->bMainPSAMPort + 1;

    if ( gpstSharedInfo->bMainPSAMPort < ZERO ) // '0'보다 작을 경우
    {
        stInstallInfo.achPSAMPort[0] = SPACE;   // SPACE로 초기화
    }

    /*
     *  SubTerm PSAM Port 번호와 PSAM 존재 여부
     */
    for ( i = 0 ; i < gbSubTermCnt ; i++ )
    {

        if ( gpstSharedInfo->abSubPSAMPort[i] == ZERO ||
             gpstSharedInfo->abSubPSAMPort[i] == 0x00 )
        {
            /*
             *  stInstallInfo.achPSAMPort[0] - Main PSAM Port
             *  stInstallInfo.achPSAMPort[1] - Sub1 PSAM Port
             *  stInstallInfo.achPSAMPort[2] - Sub2 PSAM Port
             *  stInstallInfo.achPSAMPort[3] - Sub3 PSAM Port
             *  data가 없을 경우 'A'로 세팅( web에서 보여지는 데이터 )
             */
            stInstallInfo.achPSAMPort[i + 1] = 'A';
        }
        else
        {
			stInstallInfo.achPSAMPort[i + 1] =
				gpstSharedInfo->abSubPSAMPort[i] + 1;
        }

        /*
         * 하차단말기와 통신이 이루어진 경우 
         * gpstSharedInfo->abSubPSAMPort 변수에는
         * PSAM 포트가 '0' ~ '3'의 값이 설정되어 있음
         */
		if ( gpstSharedInfo->abSubPSAMPort[i] >= '0' &&
			 gpstSharedInfo->abSubPSAMPort[i] <= '3' )
        {
            stInstallInfo.achSubTermPSAMExistYN[i] = YES;
        }
        else
        {
            stInstallInfo.achSubTermPSAMExistYN[i] = NO;
        }

    }

    /*
     * Flash memory free size
     */
    dwFlashMemFreeSize = MemoryCheck();
    DWORD2ASCWithFillLeft0( dwFlashMemFreeSize,
                            bFlashMemFreeSize,
                            sizeof( stInstallInfo.achFlashMemFreeSize ) );
    memcpy( stInstallInfo.achFlashMemFreeSize,
            bFlashMemFreeSize,
            sizeof( stInstallInfo.achFlashMemFreeSize ) );

    /*
     *  PGLoader version
     */
    fdPGVERFile = open( PG_LOADER_VER_FILE, O_RDONLY, OPENMODE );

    if ( fdPGVERFile < 0 )
    {
        DebugOut( "\rError!! ERR_FILE_OPEN [%s]\r\n", PG_LOADER_VER_FILE );
    }
    else
    {
        read( fdPGVERFile,
              achPGLoaderVer,
              sizeof( achPGLoaderVer ) - 1 );

        close( fdPGVERFile );
    }

    memcpy( stInstallInfo.achPGLoaderVer,
            achPGLoaderVer,
            sizeof( stInstallInfo.achPGLoaderVer ) );

    /*
     *  ISAM ID
     */
    ASC2BCD( gpstSharedInfo->abMainISAMID,
             stInstallInfo.abMainISAMID,
             sizeof( gpstSharedInfo->abMainISAMID ) );

    ASC2BCD( gpstSharedInfo->abSubISAMID[0],
             stInstallInfo.abSubISAMID,
             sizeof( gpstSharedInfo->abSubISAMID[0] ) );

    /*
     *  PSAM ID
     *  PSAM ID를 얻는데 실패했을 경우
     */
	// 승차단말기 PSAM
	if ( IsAllZero( gpstSharedInfo->abMainPSAMID, sizeof( gpstSharedInfo->abMainPSAMID ) ) == FALSE &&
		 IsAllASCZero( gpstSharedInfo->abMainPSAMID, sizeof( gpstSharedInfo->abMainPSAMID ) ) == FALSE )
    {
        memcpy( &stInstallInfo.abSamIDList[bPSAMCnt * nPSAMIDLength],
                gpstSharedInfo->abMainPSAMID,
                nPSAMIDLength );
        bPSAMCnt++;
    }

	// 1번 하차단말기 PSAM
	if ( IsAllZero( gpstSharedInfo->abSubPSAMID[0], sizeof( gpstSharedInfo->abSubPSAMID[0] ) ) == FALSE &&
		 IsAllASCZero( gpstSharedInfo->abSubPSAMID[0], sizeof( gpstSharedInfo->abSubPSAMID[0] ) ) == FALSE )
    {
        memcpy( &stInstallInfo.abSamIDList[bPSAMCnt * nPSAMIDLength],
                gpstSharedInfo->abSubPSAMID[0],
                nPSAMIDLength );
        bPSAMCnt++;
    }

	// 2번 하차단말기 PSAM
	if ( IsAllZero( gpstSharedInfo->abSubPSAMID[1], sizeof( gpstSharedInfo->abSubPSAMID[1] ) ) == FALSE &&
		 IsAllASCZero( gpstSharedInfo->abSubPSAMID[1], sizeof( gpstSharedInfo->abSubPSAMID[1] ) ) == FALSE )
    {
        memcpy( &stInstallInfo.abSamIDList[bPSAMCnt * nPSAMIDLength],
                gpstSharedInfo->abSubPSAMID[1],
                nPSAMIDLength );
        bPSAMCnt++;
    }

	// 3번 하차단말기 PSAM
	if ( IsAllZero( gpstSharedInfo->abSubPSAMID[2], sizeof( gpstSharedInfo->abSubPSAMID[2] ) ) == FALSE &&
		 IsAllASCZero( gpstSharedInfo->abSubPSAMID[2], sizeof( gpstSharedInfo->abSubPSAMID[2] ) ) == FALSE )
    {
        memcpy( &stInstallInfo.abSamIDList[bPSAMCnt * nPSAMIDLength],
                gpstSharedInfo->abSubPSAMID[2],
                nPSAMIDLength );
        bPSAMCnt++;
    }

    /* 
     *  BLPL CRC read
     */
    ReadBLPLFileCRC( stInstallInfo.abBLCRC, 
    				   stInstallInfo.abMifPrepayPLCRC, 
    				   stInstallInfo.abPostpayPLCRC, 
    				   stInstallInfo.abAICRC );

    /*
     *  PSAM ID count
     */
    stInstallInfo.bSAMCnt = bPSAMCnt;

    /*
     * TPSAM 여부 설정
     */
    // 승차단말기
    stInstallInfo.abPSAMVer[0] = gpstSharedInfo->bMainPSAMVer << 4;
    // 하차단말기 #1
    stInstallInfo.abPSAMVer[0] |= gpstSharedInfo->abSubPSAMVer[0];
    // 하차단말기 #2
    stInstallInfo.abPSAMVer[1] = gpstSharedInfo->abSubPSAMVer[1] << 4;
    // 하차단말기 #3
    stInstallInfo.abPSAMVer[1] |= gpstSharedInfo->abSubPSAMVer[2];

    /*
     *  install.dat 파일에 write
     */
    fdInstallInfoFile = open( INSTALL_INFO_FILE, O_WRONLY | O_CREAT, OPENMODE );

    if ( fdInstallInfoFile < 0 )
    {
        return ErrRet( ERR_FILE_OPEN | GetFileNo( INSTALL_INFO_FILE ) );
    }

    /*
     *  real input data size
     */
    nTmpInstallInfoSize = sizeof( stInstallInfo ) -
                                ( ( 4 - bPSAMCnt ) * nPSAMIDLength );

    memcpy( &abTmpInstallInfo, &stInstallInfo, nTmpInstallInfoSize );

    abTmpInstallInfo[nTmpInstallInfoSize]  = CR;
    abTmpInstallInfo[nTmpInstallInfoSize + 1] = LF;

    nWriteLen = write( fdInstallInfoFile,
    				   abTmpInstallInfo,
    				   nTmpInstallInfoSize + 2 );
    				   
    if ( nWriteLen < ( nTmpInstallInfoSize + 2 ) )
    {
    	close( fdInstallInfoFile );
    	return ErrRet( ERR_FILE_WRITE | GetFileNo( INSTALL_INFO_FILE ) );
    }
    close( fdInstallInfoFile );

    /*
     *  reconcile 등록
     */
    memset( &stRecData, 0, sizeof( stRecData ) );
    sprintf( stRecData.achFileName, "%s", INSTALL_INFO_FILE );
    stRecData.bSendStatus = RECONCILE_SEND_SETUP;
	stRecData.tWriteDtime = tCurrTime;

    WriteReconcileFileList( &stRecData );

    return sReturnVal;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SetUploadVerInfo                                         *
*                                                                              *
*  DESCRIPTION :      단말기에 설치된 각 파일의 버전을 모아 version.trn으로 생성 *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       ERR_FILE_OPEN | GetFileNo( UPLOAD_VER_INFO_FILE )      *
*                       ERR_FILE_WRITE | GetFileNo( UPLOAD_VER_INFO_FILE )     *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short SetUploadVerInfo( void )
{
    int             fdVerFile           = 0;
    short           sReturnVal          = SUCCESS;  // return value
    RECONCILE_DATA  stRecData;                      // reconcile 등록할 구조체
    UPLOAD_VER_INFO stUploadVerInfo;                // upload할 버전 정보

    /*
     *  copy version info
     */
    sReturnVal = CopyVerInfo2SendUse( &stUploadVerInfo );

    /*
     *  version.trn에 write
     */
    fdVerFile = open( UPLOAD_VER_INFO_FILE,
                      O_WRONLY | O_CREAT | O_TRUNC,
                      OPENMODE );

    if ( fdVerFile < 0 )
    {
        return ErrRet( ERR_FILE_OPEN | GetFileNo( UPLOAD_VER_INFO_FILE ) );
    }

    lseek( fdVerFile, 0, SEEK_SET );

    if ( write( fdVerFile, (void*)&stUploadVerInfo, sizeof( stUploadVerInfo ) )
         != sizeof( stUploadVerInfo ) )
    {
        close( fdVerFile );
        return ErrRet( ERR_FILE_WRITE | GetFileNo( UPLOAD_VER_INFO_FILE ) );
    }

    close( fdVerFile );

    /*
     *  reconcile 등록
     */
    memset( &stRecData, 0x00, sizeof( RECONCILE_DATA ) );
    memcpy( &stRecData.achFileName,
            UPLOAD_VER_INFO_FILE,
            strlen( UPLOAD_VER_INFO_FILE ) );
    stRecData.bSendStatus = RECONCILE_SEND_VERSION;

    sReturnVal = WriteReconcileFileList( &stRecData );

    return sReturnVal;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      CopyVerInfo2SendUse                                      *
*                                                                              *
*  DESCRIPTION :      This program copies version information to send use.     *
*                                                                              *
*  INPUT PARAMETERS:  UPLOAD_VER_INFO* pstUploadVerInfo                        *
*                                                                              *
*  RETURN/EXIT VALUE:  SUECCESS                                                *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short CopyVerInfo2SendUse( UPLOAD_VER_INFO* pstUploadVerInfo )
{
    byte abTmpCmp[4]        = { 0, };       // sub term appl version Y/N
    byte abTmpCmpSubID[9]   = { 0, };       // sub term SAM Y/N
    byte abFileSize[11]     = { 0, };       // file size 세팅용
    int nFileSizeLength     = 0;            // file size length

    /*
     *  application version length
     */
    int nVerNameLength      = sizeof( pstUploadVerInfo->abSubTerm1ApplVerName );
    int nTmpCmpLength       = sizeof( abTmpCmp );

    memset( pstUploadVerInfo, 0x00, sizeof( *pstUploadVerInfo ) );

    /*
     *  Main Terminal Application Version
     */
    pstUploadVerInfo->abMainTermApplVerName[0] = VERSION;
    memcpy( &(pstUploadVerInfo->abMainTermApplVerName[1]),
            MAIN_RELEASE_VER,
            sizeof( pstUploadVerInfo->abMainTermApplVerName ) - 1 );

    /*
     *  version info creation date
     */
    ASC2BCD( stVerInfo.abVerInfoCreateDtime,
             pstUploadVerInfo->abVerInfoCreateDtime,
             sizeof( stVerInfo.abVerInfoCreateDtime ) );

    /*
     *  Sub Terminal#1 Application Version
     *  SPACE로 초기화
     */
    memset( pstUploadVerInfo->abSubTerm1ApplVerName, SPACE, nVerNameLength );

    /*
     *  하차기가 없으면 SPACE로 세팅함
     */
    if( memcmp( gpstSharedInfo->abSubVer[0], abTmpCmp, nTmpCmpLength
              ) == 0  )
    {
        memset( pstUploadVerInfo->abSubTerm1ApplVerName,
        		SPACE,
        		nVerNameLength );
    }
    else
    {
        memcpy( pstUploadVerInfo->abSubTerm1ApplVerName,
                gpstSharedInfo->abSubVer[0],
                nTmpCmpLength );
    }

    /*
     *  Sub Terminal#2 Application Version
     *  SPACE로 초기화
     */
    memset( pstUploadVerInfo->abSubTerm2ApplVerName, SPACE, nVerNameLength);

    if( memcmp( gpstSharedInfo->abSubVer[1], abTmpCmp, nTmpCmpLength
              ) == 0  )
    {
        memset( pstUploadVerInfo->abSubTerm2ApplVerName,
        		SPACE,
        		nVerNameLength );
    }
    else
    {
        memcpy( pstUploadVerInfo->abSubTerm2ApplVerName,
                gpstSharedInfo->abSubVer[1],
                nTmpCmpLength );
    }

    /*
     *  Sub Terminal#3 Application Version
     *  SPACE로 초기화
     */
    memset( pstUploadVerInfo->abSubTerm3ApplVerName, SPACE, nVerNameLength );

    if( memcmp( gpstSharedInfo->abSubVer[2], abTmpCmp, nTmpCmpLength
              ) == 0  )
    {
        memset( pstUploadVerInfo->abSubTerm3ApplVerName,
        		SPACE,
        		nVerNameLength );
    }
    else
    {
        memcpy( pstUploadVerInfo->abSubTerm3ApplVerName,
                gpstSharedInfo->abSubVer[2],
                nTmpCmpLength );
    }

    /*
     *  Driver Operator Application Version
     *  마지막 byte는 SPACE로 세팅
     */
    memcpy( pstUploadVerInfo->abDriverOperatorApplVerName,
            gpstSharedInfo->abKpdVer,
            sizeof( pstUploadVerInfo->abDriverOperatorApplVerName ) - 1 );
    pstUploadVerInfo->abDriverOperatorApplVerName[4] = SPACE;

    /*
     *  main terminal kernel version
     */
    memcpy( pstUploadVerInfo->abMainTermKernelVerName,
            gabKernelVer,
            sizeof( pstUploadVerInfo->abMainTermKernelVerName ) );

    /*
     *  교통 사업자 ID
     */
    memcpy( pstUploadVerInfo->abTranspBizrID,
            gstVehicleParm.abTranspBizrID,
            sizeof( gstVehicleParm.abTranspBizrID ) );

    /*
     *  버스 영업소 ID
     */
    memcpy( pstUploadVerInfo->abBusBizOfficeID,
            gstVehicleParm.abBusBizOfficeID,
            sizeof( gstVehicleParm.abBusBizOfficeID ) );

    /*
     *  버스 노선ID
     */
    ASC2BCD( gstVehicleParm.abRouteID,
             pstUploadVerInfo->abRouteID,
             sizeof( gstVehicleParm.abRouteID ) );

    /*
     *  차량 ID
     */
    memcpy( pstUploadVerInfo->abVehicleID,
            gstVehicleParm.abVehicleID,
            sizeof( gstVehicleParm.abVehicleID ) );

    /*
     *  main terminal ID
     */
    memcpy( pstUploadVerInfo->abMainTermID,
            gpstSharedInfo->abMainTermID,
            nTermIDLength );

    /*
     *  하차단말기 갯수
     *  하차단말기가 없으면 '0'로 세팅하나 default로 gbSubTermCnt는 1임
     */
    if ( gbSubTermCnt == 0 )
    {
        pstUploadVerInfo->bSubTermCnt = ZERO;
    }
    else
    {
        /*
         *  ASCII로 변경
         */
        pstUploadVerInfo->bSubTermCnt = gbSubTermCnt + ZERO;
    }

    /*
     * sub terminal#1 ID
     * sub term이 없으면 SPACE로 세팅
     */
    if( memcmp( gpstSharedInfo->abSubTermID[0], 
    			TERM_ID_INIT_VALUE, 
    			nTermIDLength
              ) == 0 ||
        memcmp( gpstSharedInfo->abSubTermID[0], abTmpCmpSubID, nTermIDLength
              ) == 0 )
    {
        memset( pstUploadVerInfo->abSubTerm1ID,
                SPACE,
                nTermIDLength );
    }
    else
    {
        memcpy( pstUploadVerInfo->abSubTerm1ID,
                gpstSharedInfo->abSubTermID[0],
                nTermIDLength );
    }

    /*
     * sub terminal#2 ID
     * sub term이 없으면 SPACE로 세팅
     */
    if( memcmp( gpstSharedInfo->abSubTermID[1], 
    			TERM_ID_INIT_VALUE, 
    			nTermIDLength
              ) == 0 ||
        memcmp( gpstSharedInfo->abSubTermID[1], abTmpCmpSubID, nTermIDLength
              ) == 0  )
    {
        memset( pstUploadVerInfo->abSubTerm2ID,
                SPACE,
                nTermIDLength );
    }
    else
    {
        memcpy( pstUploadVerInfo->abSubTerm2ID,
                gpstSharedInfo->abSubTermID[1],
                nTermIDLength );
    }

    /*
     * sub terminal#3 ID
     * sub term이 없으면 SPACE로 세팅
     */
    if( memcmp( gpstSharedInfo->abSubTermID[2], 
    			TERM_ID_INIT_VALUE, 
    			nTermIDLength
              ) == 0 ||
        memcmp( gpstSharedInfo->abSubTermID[2], abTmpCmpSubID, nTermIDLength
              ) == 0  )
    {
        memset( pstUploadVerInfo->abSubTerm3ID,
                SPACE,
                nTermIDLength );
    }
    else
    {
        memcpy( pstUploadVerInfo->abSubTerm3ID,
                gpstSharedInfo->abSubTermID[2],
                nTermIDLength );
    }

    /*
     * driver operator ID
     */
    if ( IsAllZero( gpstSharedInfo->abDriverOperatorID,
			sizeof( gpstSharedInfo->abDriverOperatorID ) ) ||
		 IsAllASCZero( gpstSharedInfo->abDriverOperatorID,
			sizeof( gpstSharedInfo->abDriverOperatorID ) ) )
    {
		memset( pstUploadVerInfo->abDriverOperatorID, SPACE,
			sizeof( pstUploadVerInfo->abDriverOperatorID ) );
    }
	else
	{
		memcpy( pstUploadVerInfo->abDriverOperatorID,
			gpstSharedInfo->abDriverOperatorID,
			sizeof( gpstSharedInfo->abDriverOperatorID ) );
	}

    /*
     * SAM ID SPACE로 초기화
     */
    memset( pstUploadVerInfo->abMainTermSAMID,
    		SPACE,
            nPSAMIDLength );
    memset( pstUploadVerInfo->abSubTerm1SAMID,
    		SPACE,
            nPSAMIDLength );
    memset( pstUploadVerInfo->abSubTerm2SAMID,
    		SPACE,
            nPSAMIDLength );
    memset( pstUploadVerInfo->abSubTerm3SAMID,
    		SPACE,
            nPSAMIDLength );

    /*
     *  Main Term SAM ID
     */
    memcpy( pstUploadVerInfo->abMainTermSAMID,
            gpstSharedInfo->abMainPSAMID,
            nPSAMIDLength );

    /*
     *  Sub Term#1 SAM ID
     *  Sub Term #1이 없으면 SPACE로 세팅
     */
    if ( IsAllASCZero( gpstSharedInfo->abSubPSAMID[0],
			sizeof( gpstSharedInfo->abSubPSAMID[0] ) ) ||
		 IsAllZero( gpstSharedInfo->abSubPSAMID[0],
			sizeof( gpstSharedInfo->abSubPSAMID[0] ) ) )
    {
        memset( pstUploadVerInfo->abSubTerm1SAMID,
                SPACE,
                nPSAMIDLength );
    }
    else
    {
        memcpy( pstUploadVerInfo->abSubTerm1SAMID,
                gpstSharedInfo->abSubPSAMID[0],
                nPSAMIDLength );
    }

    /*
     *  Sub Term#2 SAM ID
     *  Sub Term #2이 없으면 SPACE로 세팅
     */
    if ( IsAllASCZero( gpstSharedInfo->abSubPSAMID[1],
			sizeof( gpstSharedInfo->abSubPSAMID[1] ) ) ||
		 IsAllZero( gpstSharedInfo->abSubPSAMID[1],
			sizeof( gpstSharedInfo->abSubPSAMID[1] ) ) )
    {
        memset( pstUploadVerInfo->abSubTerm2SAMID,
                SPACE,
                nPSAMIDLength );
    }
    else
    {
        memcpy( pstUploadVerInfo->abSubTerm2SAMID,
                gpstSharedInfo->abSubPSAMID[1],
                nPSAMIDLength );
    }

    /*
     *  Sub Term#3 SAM ID
     *  Sub Term #3이 없으면 SPACE로 세팅
     */
    if ( IsAllASCZero( gpstSharedInfo->abSubPSAMID[2],
			sizeof( gpstSharedInfo->abSubPSAMID[2] ) ) ||
		 IsAllZero( gpstSharedInfo->abSubPSAMID[2],
			sizeof( gpstSharedInfo->abSubPSAMID[2] ) ) )
    {
        memset( pstUploadVerInfo->abSubTerm3SAMID,
                SPACE,
                nPSAMIDLength );
    }
    else
    {
        memcpy( pstUploadVerInfo->abSubTerm3SAMID,
                gpstSharedInfo->abSubPSAMID[2],
                nPSAMIDLength );
    }

    /*
     *  DCS IP
     */
    memcpy( pstUploadVerInfo->abDCSIPAddr,
            gstCommInfo.abDCSIPAddr,
            sizeof( pstUploadVerInfo->abDCSIPAddr ) );

    /*
     *  교통 수단 코드
     */
    memcpy( pstUploadVerInfo->abTranspMethodCode,
            gstVehicleParm.abTranspMethodCode,
            sizeof( pstUploadVerInfo->abTranspMethodCode ) );

    nFileSizeLength = sizeof( abFileSize );

    /*
     *  master bl file size
     */
    sprintf( abFileSize, "%10ld", GetFileSize( MASTER_BL_FILE ) );
    ASC2BCD( abFileSize,
             pstUploadVerInfo->abMasterBLSize,
			 nFileSizeLength - 1 );

    /*
     *  master pl(prepay) file size
     */
    sprintf( abFileSize, "%10ld",
             GetFileSize( MASTER_PREPAY_PL_FILE ) );
    ASC2BCD( abFileSize,
             pstUploadVerInfo->abMasterPrepayPLSize,
             nFileSizeLength - 1 );

    /*
     *  master pl(postpay) file size
     */
    sprintf( abFileSize, "%10ld",
             GetFileSize( MASTER_POSTPAY_PL_FILE ) );
    ASC2BCD( abFileSize,
             pstUploadVerInfo->abMasterPostpayPLSize,
             nFileSizeLength - 1 );

    /*
     *  master ai file size
     */
    sprintf( abFileSize, "%10ld", GetFileSize( MASTER_AI_FILE ) );
    ASC2BCD( abFileSize,
             pstUploadVerInfo->abMasterAISize,
             nFileSizeLength - 1 );

    /*
     *  BL version
     */
    ASC2BCD( stVerInfo.abMasterBLVer,
             pstUploadVerInfo->abMasterBLVer,
             sizeof( stVerInfo.abMasterBLVer ) );
    ASC2BCD( stVerInfo.abUpdateBLVer,
             pstUploadVerInfo->abUpdateBLVer,
             sizeof( stVerInfo.abUpdateBLVer ) );
    ASC2BCD( stVerInfo.abHotBLVer,
             pstUploadVerInfo->abHotBLVer,
             sizeof( stVerInfo.abHotBLVer ) );

    /*
     *  PL version
     */
    ASC2BCD( stVerInfo.abMasterPrepayPLVer,
             pstUploadVerInfo->abMasterPrepayPLVer,
             sizeof( stVerInfo.abMasterPrepayPLVer ) );
    ASC2BCD( stVerInfo.abMasterPostpayPLVer,
             pstUploadVerInfo->abMasterPostpayPLVer,
             sizeof( stVerInfo.abMasterPostpayPLVer ) );
    ASC2BCD( stVerInfo.abUpdatePLVer,
             pstUploadVerInfo->abUpdatePLVer,
             sizeof( stVerInfo.abUpdatePLVer ) );
    ASC2BCD( stVerInfo.abHotPLVer,
             pstUploadVerInfo->abHotPLVer,
             sizeof( stVerInfo.abHotPLVer ) );

    /*
     *  AI version
     */
    ASC2BCD( stVerInfo.abMasterAIVer,
             pstUploadVerInfo->abMasterAIVer,
             sizeof( stVerInfo.abMasterAIVer ) );
    ASC2BCD( stVerInfo.abUpdateAIVer,
             pstUploadVerInfo->abUpdateAIVer,
             sizeof( stVerInfo.abUpdateAIVer ) );
    ASC2BCD( stVerInfo.abHotAIVer,
             pstUploadVerInfo->abHotAIVer,
             sizeof( stVerInfo.abHotAIVer ) );

    /*
     *  parameter file version
     */
    ASC2BCD( stVerInfo.abVehicleParmVer,
             pstUploadVerInfo->abVehicleParmVer,
             sizeof( stVerInfo.abVehicleParmVer ) );
    ASC2BCD( stVerInfo.abRouteParmVer,
             pstUploadVerInfo->abRouteParmVer,
             sizeof( stVerInfo.abRouteParmVer ) );
    ASC2BCD( stVerInfo.abBasicFareInfoVer,
             pstUploadVerInfo->abBasicFareInfoVer,
             sizeof( stVerInfo.abBasicFareInfoVer ) );
    ASC2BCD( stVerInfo.abBusStationInfoVer,
             pstUploadVerInfo->abBusStationInfoVer,
             sizeof( stVerInfo.abBusStationInfoVer ) );
    ASC2BCD( stVerInfo.abPrepayIssuerInfoVer,
             pstUploadVerInfo->abPrepayIssuerInfoVer,
             sizeof( stVerInfo.abPrepayIssuerInfoVer ) );
    ASC2BCD( stVerInfo.abPostpayIssuerInfoVer,
             pstUploadVerInfo->abPostpayIssuerInfoVer,
             sizeof( stVerInfo.abPostpayIssuerInfoVer ) );
    ASC2BCD( stVerInfo.abDisExtraInfoVer,
             pstUploadVerInfo->abDisExtraInfoVer,
             sizeof( stVerInfo.abDisExtraInfoVer ) );
    ASC2BCD( stVerInfo.abHolidayInfoVer,
             pstUploadVerInfo->abHolidayInfoVer,
             sizeof( stVerInfo.abHolidayInfoVer ) );
    ASC2BCD( stVerInfo.abXferApplyRegulationVer,
             pstUploadVerInfo->abXferApplyRegulationVer,
             sizeof( stVerInfo.abXferApplyRegulationVer ) );
    ASC2BCD( stVerInfo.abXferApplyInfoVer,
             pstUploadVerInfo->abXferApplyVer,
             sizeof( stVerInfo.abXferApplyInfoVer ) );

    /*
     *  application version
     */
    ASC2BCD( stVerInfo.abMainTermApplVer,
             pstUploadVerInfo->abMainTermApplVer,
             sizeof( stVerInfo.abMainTermApplVer ) );
    ASC2BCD( stVerInfo.abSubTermApplVer,
             pstUploadVerInfo->abSubTerm1ApplVer,
             sizeof( stVerInfo.abSubTermApplVer ) );
    ASC2BCD( stVerInfo.abSubTermApplVer,
             pstUploadVerInfo->abSubTerm2ApplVer,
             sizeof( stVerInfo.abSubTermApplVer ) );
    ASC2BCD( stVerInfo.abSubTermApplVer,
             pstUploadVerInfo->abSubTerm3ApplVer,
             sizeof( stVerInfo.abSubTermApplVer ) );
    ASC2BCD( stVerInfo.abDriverOperatorApplVer,
             pstUploadVerInfo->abDriverOperatorApplVer,
             sizeof( stVerInfo.abDriverOperatorApplVer ) );

    /*
     *  SAM info version
     */
    ASC2BCD( stVerInfo.abEpurseIssuerRegistInfoVer,
             pstUploadVerInfo->abEpurseIssuerRegistInfoVer,
             sizeof( stVerInfo.abEpurseIssuerRegistInfoVer ) );
    ASC2BCD( stVerInfo.abPSAMKeysetVer,
             pstUploadVerInfo->abPSAMKeysetVer,
             sizeof( stVerInfo.abPSAMKeysetVer ) );
    ASC2BCD( stVerInfo.abAutoChargeKeysetVer,
             pstUploadVerInfo->abAutoChargeKeysetVer,
             sizeof( stVerInfo.abAutoChargeKeysetVer ) );
    ASC2BCD( stVerInfo.abAutoChargeParmVer,
             pstUploadVerInfo->abAutoChargeParmVer,
             sizeof( stVerInfo.abAutoChargeParmVer ) );

    /*
     *  후불 발행사 유효기간 version
     */
    ASC2BCD( stVerInfo.abIssuerValidPeriodInfoVer,
             pstUploadVerInfo->abPostpayIssuerChkVer,
             sizeof( stVerInfo.abIssuerValidPeriodInfoVer ) );

    pstUploadVerInfo->abCrLf[0] = CR;
    pstUploadVerInfo->abCrLf[1] = LF ;

    DebugOut( "CopyVerInfo2SendUse Complete!!! \n" );

    return SUCCESS;
}

