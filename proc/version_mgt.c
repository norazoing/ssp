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
*  PROGRAM ID :       version_mgt.c                                            *
*                                                                              *
*  DESCRIPTION:       이 프로그램은 미래적용버전을 적용하고 각 버전을 롤백하거나  *
*						초기화하는 기능의 함수를 제공한다. 					   *
*                                                                              *
*  ENTRY POINT:     short ApplyNextVer( void );								   *
*					short RollbackBLPLVer( 	bool boolIsBLMergeFail,			   *
*                       					bool boolIsPLMergeFail,			   *
*                       					bool boolIsAIMergeFail );		   *
*					short SetMasterBLPLVer( void );		   					   *
*					void SetVerBLPLFileExistYN( void );						   *
*					short InitSAMRegistInfoVer( void );						   *
*					short WriteBLPLBefVer( char* pachFileName, 				   *
*										   char* pachBefVer, 				   *
*										   int nBefVerLength );				   *
*					short CreateVerInfoPkt( bool boolIsCurrVer,				   *
*											byte* pbVerInfoBuff );			   *
*																			   *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: None                                                     *
*                                                                              *
*  INPUT FILES:       c_ve_inf.dat                                             *
*					  v_bl_be.dat                                              *
*					  v_pl_be.dat                                              *
*					  v_ai_be.dat                                              *
*					  v_pl_be.dat                                              *
*					  donwlink.dat                                             *
*					  c_fi_bl.dat                                              *
*					  c_fa_pl.dat                                              *
(					  c_fd_pl.dat                                              *
*					  c_fi.ai.dat                                              *
*                                                                              *
*  OUTPUT FILES:      c_ve_inf.dat                                             *
*					  c_dr_pro.dat                                             *
*					  c_en_pro.dat                                             *
*					  c_ex_pro.dat                                             *
*					  c_v0.dat                                                 *
*					  c_op_par.dat                                             *
*					  c_li_par.dat                                             *
*					  c_n_far.dat                                              *
*					  c_st_inf.dat                                             *
*					  c_ap_inf.dat                                             *
*					  c_dp_inf.dat                                             *
*					  c_de_inf.dat                                             *
*					  c_ho_inf.dat                                             *
*					  c_tc_inf.dat                                             *
*					  c_idcenter.dat                                           *
*					  c_keyset.dat                                             *
*					  c_tr_inf.dat                                             *
*					  c_cd_inf.dat                                             *
*					  v_bl_be.dat                                              *
*					  v_pl_be.dat                                              *
*					  v_ai_be.dat                                              *
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
#include "version_file_mgt.h"
#include "../comm/download_dcs_comm.h"

/*******************************************************************************
*  Declaration of variables                                                    *
*******************************************************************************/
#define		PARAM_FILE_NAME_LEN		12
#define     BCD_VER_SIZE    7           // BCD
#define     ASCII_VER_SIZE  14          // ex)20060101090001
#define     APPL_VER_SIZE   4           // ex) "0401"
#define BUS_SYSTEM_CLASS_CD         	'0'
#define NOT_EXIST_MORE_PKT          	'1'
#define VER_INFO_CNT                	"0048"      // 다운받을 파일의 갯수
#define SIZE_LENGTH                  	4


#define NEXT_POSTPAY_ISSUER_VALID_PERIOD_INFO_FILE  "n_cd_inf.dat"
#define NEXT_XFER_CONDITION_INFO_FILE       		"n_tc_inf.dat"

/*******************************************************************************
*  Declaration of Module Functions                                             *
*******************************************************************************/
static int CheckApplyNextVer( char* pchFilename );
static short GetBeforeVer ( char *pchFileName, char *pchBeforeVer );

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      ApplyNextVer                                             *
*                                                                              *
*  DESCRIPTION :      미래적용파일의 현재 적용여부를 체크하여 현재 적용한다.	   *
*                                                                              *
*  INPUT PARAMETERS:  void                                                     *
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
short ApplyNextVer( void )
{
    int     nFileLoop               = 0;
    char    achNextFileName[50]     = { 0, };
    char    achCurrFileName[50]     = { 0, };
    char    achTmpCurrFileName[50]  = { 0, };

    for ( nFileLoop = 0 ; nFileLoop < DCS_COMM_FILE_CNT ; nFileLoop++ )
    {
        /*
         *  next버전의 파일명 생성
         */
        sprintf( achNextFileName, "n_%s", achDCSCommFile[nFileLoop][1] );

        /*
         *  apply대상인지 check
         */
        if ( CheckApplyNextVer( achNextFileName ) == NEXT )
        {
            /*
             * file replace
             */
            sprintf( achTmpCurrFileName,
                     "tmp_c_%s",
                     achDCSCommFile[nFileLoop][1] );
            sprintf( achCurrFileName,
                     "c_%s",
                     achDCSCommFile[nFileLoop][1] );

            rename(  achCurrFileName,  achTmpCurrFileName );
            rename(  achNextFileName,  achCurrFileName );

            unlink( achTmpCurrFileName );
            unlink( achNextFileName );

			printf( "[ApplyNextVer] [%s] -> [%s]\n", achNextFileName,
				achCurrFileName  );

            /*
             *  next버전 적용 여부 세팅
             */
            boolIsApplyNextVer = TRUE;

            if ( nFileLoop == 0 )
            {
                boolIsApplyNextVerDriverAppl = TRUE;
            }
            else if ( nFileLoop >= 1 && nFileLoop < 10 )
            {
                boolIsApplyNextVerAppl = TRUE;
            }
            else if ( nFileLoop >= 10 && nFileLoop < 20 )
            {
                boolIsApplyNextVerVoice = TRUE;
            }
            else if ( nFileLoop >= 21 && nFileLoop < 38 )
            {
                boolIsApplyNextVerParm = TRUE;
            }
        }

    }

    return SUCCESS;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      CheckApplyNextVer                                        *
*                                                                              *
*  DESCRIPTION :      미래적용파일을 현재적용해야하는지 체크한다.				   *
*                                                                              *
*  INPUT PARAMETERS:  char *pchFilename  - 체크할 next 파일                     *
*                                                                              *
*  RETURN/EXIT VALUE:   CURR   - 현재 버전                                     *
*                       NEXT   - 미래 버전                                     *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static int CheckApplyNextVer( char* pchFilename )
{
    int     fdFile                      = 0;
    int     nVerInfoOffset              = BCD_VER_SIZE;
    time_t  tCurrTime;
    byte    abCurrDtime[14]             = { 0, };

    char    achFileApplyDtime[15]       = { 0, };
    char    achFileApplyDtimeASC[15]    = { 0, };

    fdFile = open( pchFilename,  O_RDONLY, OPENMODE );

    /*
     *  해당 파일이 있으면
     */
    if ( fdFile > 0 )
    {

        /*
         *  다음의 파일의 경우 ASCII로 버전정보가 세팅되어 있음
         */
        if ( ( memcmp( pchFilename, 
        			   NEXT_VEHICLE_PARM_FILE,
        			   PARAM_FILE_NAME_LEN ) == 0 ) ||
             ( memcmp( pchFilename,
             		   NEXT_ROUTE_PARM_FILE,
             		   PARAM_FILE_NAME_LEN ) == 0 ) ||
             ( memcmp( pchFilename,
                       NEXT_XFER_CONDITION_INFO_FILE,
                       PARAM_FILE_NAME_LEN ) == 0 ) ||
             ( memcmp( pchFilename,
                       NEXT_POSTPAY_ISSUER_VALID_PERIOD_INFO_FILE,
                       PARAM_FILE_NAME_LEN ) == 0 ) ||
             ( memcmp( pchFilename,
             		   NEXT_KPD_IMAGE_FILE,
             		   PARAM_FILE_NAME_LEN ) == 0 ) ||
             ( memcmp( pchFilename,
             		   NEXT_BUS_MAIN_IMAGE_FILE,
             		   PARAM_FILE_NAME_LEN ) == 0 ) ||
             ( memcmp( pchFilename,
             		   NEXT_BUS_SUB_IMAGE_FILE,
             		   PARAM_FILE_NAME_LEN ) == 0 )
        )
        {
            nVerInfoOffset = ASCII_VER_SIZE;

            /*
             *  프로그램 파일의 경우 버전정보는 파일의 끝에 있음
             *
             *  운전자 조작기와 하차단말기 프로그램의 경우
             *  프로그램 버전도 함께 세팅됨 ex) "0401"
             */
            if ( ( memcmp( pchFilename,
            			   NEXT_KPD_IMAGE_FILE,
            			   PARAM_FILE_NAME_LEN ) == 0 ) ||
                 ( memcmp( pchFilename,
                 		   NEXT_BUS_SUB_IMAGE_FILE,
                 		   PARAM_FILE_NAME_LEN ) == 0 ) )
            {
                lseek ( fdFile, -(nVerInfoOffset + APPL_VER_SIZE), SEEK_END);
            }
            else if ( memcmp( pchFilename,
            				  NEXT_BUS_MAIN_IMAGE_FILE,
            				  PARAM_FILE_NAME_LEN ) == 0 )
            {
                lseek ( fdFile, -nVerInfoOffset, SEEK_END);
            }
        }

        if ( read( fdFile, achFileApplyDtime, nVerInfoOffset ) > 0  )
        {

            if ( nVerInfoOffset == BCD_VER_SIZE )
            {
                BCD2ASC( achFileApplyDtime,
                         achFileApplyDtimeASC,
                         nVerInfoOffset );
                memset( achFileApplyDtime,
                        0x00,
                        sizeof( achFileApplyDtime ) );
                memcpy( achFileApplyDtime,
                        achFileApplyDtimeASC,
                        ASCII_VER_SIZE );
            }

            GetRTCTime( &tCurrTime );
            TimeT2ASCDtime( tCurrTime, abCurrDtime );

            /*
             *  현재 시간과 파일의 적용일자를 비교하여
             *  파일 적용일자가 현재 시간보다 같거나 작으면
             */
            if ( memcmp( achFileApplyDtime, abCurrDtime, ASCII_VER_SIZE ) <= 0 )
            {
                close( fdFile );
                return NEXT;
            }
        }

        close( fdFile );
    }

    return CURR;

}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      RollbackBLPLVer                                          *
*                                                                              *
*  DESCRIPTION :      BLPL version을 다운로드 받기 이전의 version으로 rollback  *
*                                                                              *
*  INPUT PARAMETERS:  bool boolIsBLMergeFail,                                  *
*                     bool boolIsPLMergeFail,                                  *
*                     bool boolIsAIMergeFail                                   *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       ERR_FILE_OPEN | GetFileNo( UPDATE_BL_BEFORE_VER_FILE ) *
*                       ERR_FILE_READ | GetFileNo( UPDATE_BL_BEFORE_VER_FILE ) *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-13                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short RollbackBLPLVer( bool boolIsBLMergeFail,
                       bool boolIsPLMergeFail,
                       bool boolIsAIMergeFail )
{

    short       sReturnVal      = SUCCESS;
    char        achTmpVer[sizeof( stVerInfo.abUpdateBLVer )] = { 0, };
    VER_INFO    stTmpVerInfo;

    /*
     *  version info load
     */
    LoadVerInfo( &stTmpVerInfo );

    /*
     *  BL merge 실패시
     */
    if ( boolIsBLMergeFail == TRUE )
    {
        /*
         *  다운로드 받기 이전 버전을 얻어온다
         */
         sReturnVal = GetBeforeVer( UPDATE_BL_BEFORE_VER_FILE, achTmpVer );

        if ( sReturnVal  < 0 )
        {
            return sReturnVal;
        }

        /*
         *  읽어온 이전 update BL version을 버전정보에 세팅
         */
        memcpy( stVerInfo.abUpdateBLVer,
                achTmpVer,
                sizeof( stVerInfo.abUpdateBLVer ) );

        /*
         *  세팅된 버전정보를 저장
         */
        SaveVerFile();

        /*
         *  이전 update BL version을 저장하고 있는 v_bl_be.dat삭제
         */
        unlink( UPDATE_BL_BEFORE_VER_FILE );

        /*
         *  BL merge 실패 log
         */
        ctrl_event_info_write( BL_GENERAL_ERROR_EVENT );

    }

    /*
     *  PL merge 실패시
     */
    if ( boolIsPLMergeFail == TRUE )
    {
        /*
         *  다운로드 받기 이전 버전을 얻어온다
         */
        sReturnVal = GetBeforeVer( UPDATE_PL_BEFORE_VER_FILE, achTmpVer );

        if ( sReturnVal < 0 )
        {
            return sReturnVal;
        }

        /*
         *  읽어온 이전 update PL version을 버전정보에 세팅
         */
        memcpy( stVerInfo.abUpdatePLVer,
                achTmpVer,
                sizeof( stVerInfo.abUpdatePLVer ) );

        /*
         *  세팅된 버전정보를 저장
         */
        SaveVerFile();

        /*
         *  이전 update PL version을 저장하고 있는 v_pl_be.dat삭제
         */
        unlink( UPDATE_PL_BEFORE_VER_FILE );

        /*
         *  PL merge 실패 log
         */
        ctrl_event_info_write( PL_GENERAL_ERROR_EVENT );

    }

    /*
     *  AI merge 실패시
     */
    if ( boolIsAIMergeFail == TRUE )
    {
        sReturnVal = GetBeforeVer( UPDATE_AI_BEFORE_VER_FILE, achTmpVer );

        if ( sReturnVal < 0 )
        {
            return sReturnVal;
        }

        /*
         *  읽어온 이전 update AI version을 버전정보에 세팅
         */
        memcpy( stVerInfo.abUpdateAIVer,
                achTmpVer,
                sizeof( stVerInfo.abUpdateAIVer ) );

        /*
         *  세팅된 버전정보를 저장
         */
        SaveVerFile();

        /*
         *  이전 update AI version을 저장하고 있는 v_pl_be.dat삭제
         */
        unlink( UPDATE_AI_BEFORE_VER_FILE );

        /*
         *  AI merge 실패 log
         */
        ctrl_event_info_write(AI_GENERAL_ERROR_EVENT);

    }

    return sReturnVal;

}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      GetBeforeVer                                             *
*                                                                              *
*  DESCRIPTION :      백업해놓은 BLPL 버전을 읽어온다.		                   *
*                                                                              *
*  INPUT PARAMETERS:  char *pchFileName,    - 이전버전 저장 파일(BLPL)          *
*                     char *pchBeforeVer    - 이전 버전                         *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       ERR_FILE_OPEN | GetFileNo( UPDATE_BL_BEFORE_VER_FILE ) *
*                       ERR_FILE_READ | GetFileNo( UPDATE_BL_BEFORE_VER_FILE ) *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-13                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short GetBeforeVer ( char *pchFileName, char *pchBeforeVer )
{
    int         fdBeforeVer         = 0;
    int         nReadByte           = 0;
    char        achTmpVer[14]       = { 0, };

    fdBeforeVer = open( pchFileName, O_RDONLY, OPENMODE );

    if ( fdBeforeVer < 0 )
    {
        return ErrRet( ERR_FILE_OPEN | GetFileNo( UPDATE_BL_BEFORE_VER_FILE ) );
    }

    nReadByte = read( fdBeforeVer, achTmpVer, sizeof( achTmpVer ) );

    if ( nReadByte < sizeof( achTmpVer ) )
    {
        return ErrRet( ERR_FILE_READ | GetFileNo( UPDATE_BL_BEFORE_VER_FILE ) );
    }

    memcpy( pchBeforeVer, achTmpVer, sizeof( achTmpVer ) );

    if ( fdBeforeVer > 0 )
    {
        close( fdBeforeVer );
    }

    return SUCCESS;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SetMasterBLPLVer                                         *
*                                                                              *
*  DESCRIPTION :      다운로드 중인 고정 BLPL 실제 파일의 size와 팻킷 수를 세팅  *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*					ErrRet( ERR_FILE_OPEN | GetFileNo( RELAY_DOWN_INFO_FILE ) )*
*					ErrRet( ERR_FILE_READ | GetFileNo( RELAY_DOWN_INFO_FILE ) )*
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short SetMasterBLPLVer( void )
{
    short 	sRetVal 		= SUCCESS;
    int 	fdFileChain;            // chain  file
    int 	fdTmpFile;              // BLPL file temporary fd
    int 	nReadByte;
    int 	nFileSize 		= 0;
    int 	nPktCnt 		= 0;
    struct 	stat 	stFileStat;
    div_t  	stDivResult;
	FILE_RELAY_RECV_INFO	stFileRelayRecvInfo;	// 이어받기정보 구조체

    system( "chmod 755 /mnt/mtd8/bus/* " );	
	memset( &stFileRelayRecvInfo, 0x00, sizeof( stFileRelayRecvInfo ) );
	
	/*
	 *	이어받기 정보파일을 open
	 */
	fdFileChain = open( RELAY_DOWN_INFO_FILE, O_RDONLY, OPENMODE );
	
    if ( fdFileChain < 0 )
    {
        return ErrRet( ERR_FILE_OPEN | GetFileNo( RELAY_DOWN_INFO_FILE ) );
    }

	/*
	 *	이어받기 정보파일을 read
	 */
    nReadByte = read( fdFileChain,
    				  (void*)&stFileRelayRecvInfo,
                      sizeof( stFileRelayRecvInfo ) );

    if ( nReadByte == 0 )
    {
        close( fdFileChain );
        return SUCCESS;
    }

    if ( nReadByte < 0 )
    {
		close( fdFileChain );
		sRetVal = ErrRet( ERR_FILE_READ | GetFileNo( RELAY_DOWN_INFO_FILE ) );
    }

	/*
	 *	고정 BL/PL/AI의 경우 다운로드 중의 실제 파일의 size와 팻킷수를 계산하여
	 * 	버전정보에 세팅한다.
	 */
    switch ( stFileRelayRecvInfo.chFileNo )
    {
        case  38:			// master BL data
			printf( "[SetMasterBLPLVer] 고정BL 이어받기 SIZE/Pkt# 설정\n" );
            BCD2ASC( stFileRelayRecvInfo.abDownFileVer,
                     stVerInfo.abMasterBLVer,
                     sizeof( stFileRelayRecvInfo.abDownFileVer ) );
            stVerInfo.dwMasterBLPkt =
                GetDWORDFromLITTLE( stFileRelayRecvInfo.abDownFilePktNo );

            if ( ( fdTmpFile = open( DOWN_ING_MASTER_BL_FILE,
            						 O_RDONLY,
                                     OPENMODE
                                   ) ) > 0 )
            {
                if ( fstat( fdTmpFile, &stFileStat ) < 0 )
                {
                    nPktCnt = 1;
                }
                else
                {
                    nFileSize = (int)stFileStat.st_size;
                    stDivResult = div( nFileSize, MAX_PKT_SIZE );
                    nPktCnt = stDivResult.quot;

                    if ( stDivResult.rem != 0 )
                    {
                        DebugOut( "stDivResult.rem != 0  [%d]\r\n",
                                  stDivResult.rem );
                                  nPktCnt = 1;
                    }
                }

            }
            else
            {
                nPktCnt = 0;
            }

            if ( stVerInfo.dwMasterBLPkt != nPktCnt )
            {
                DebugOut( "\r\n master B/L  pack [%lu][%d]\r\n",
                          stVerInfo.dwMasterBLPkt,
                          nPktCnt );
                stVerInfo.dwMasterBLPkt = nPktCnt;
            }

            close( fdTmpFile );

            break;

        case  41:			// current master PL data - prepay
			printf( "[SetMasterBLPLVer] 고정구선불PL 이어받기 SIZE/Pkt# 설정\n" );
            BCD2ASC( stFileRelayRecvInfo.abDownFileVer,
                     stVerInfo.abMasterPrepayPLVer,
                     sizeof( stFileRelayRecvInfo.abDownFileVer ) );
            stVerInfo.dwMasterPrepayPLPkt =
                GetDWORDFromLITTLE( stFileRelayRecvInfo.abDownFilePktNo );

            if ( ( fdTmpFile = open( DOWN_ING_MASTER_PREPAY_PL_FILE,
                                     O_RDONLY, OPENMODE )
                                   ) > 0 )
            {
                if ( fstat( fdTmpFile, &stFileStat ) < 0 )
                {
                    nPktCnt = 1;
                }
                else
                {
                    nFileSize = (int)stFileStat.st_size;
                    stDivResult = div( nFileSize, MAX_PKT_SIZE );
                    nPktCnt = stDivResult.quot;

                    if ( stDivResult.rem != 0)
                    {
                        DebugOut( "stDivResult.rem != 0  [%d]\r\n",
                                  stDivResult.rem );
                                  nPktCnt = 1;
                    }

                }
            }
            else
            {
                nPktCnt = 0;
            }

            if ( stVerInfo.dwMasterPrepayPLPkt != nPktCnt )
            {
                DebugOut( "\r\n master P/L  prepay pack [%lu][%d]\r\n",
                        stVerInfo.dwMasterPrepayPLPkt,
                        nPktCnt );
                        stVerInfo.dwMasterPrepayPLPkt = nPktCnt;
            }

            close( fdTmpFile );
            break;

        case  42:			// current master PL data - postpay
			printf( "[SetMasterBLPLVer] 고정후불PL 이어받기 SIZE/Pkt# 설정\n" );
            BCD2ASC( stFileRelayRecvInfo.abDownFileVer,
                     stVerInfo.abMasterPostpayPLVer,
                     sizeof( stFileRelayRecvInfo.abDownFileVer ) );
            stVerInfo.dwMasterPostpayPLPkt =
                GetDWORDFromLITTLE( stFileRelayRecvInfo.abDownFilePktNo );

            if ( ( fdTmpFile = open( DOWN_ING_MASTER_POSTPAY_PL_FILE,
                                     O_RDONLY,
                                     OPENMODE )
                 ) > 0)
            {
                if ( fstat( fdTmpFile, &stFileStat ) < 0 )
                {
                    nPktCnt = 1;
                }
                else
                {
                    nFileSize = (int)stFileStat.st_size;
                    stDivResult = div( nFileSize, MAX_PKT_SIZE );
                    nPktCnt = stDivResult.quot;

                    if ( stDivResult.rem != 0 )
                    {
                        DebugOut( "stDivResult.rem != 0  [%d]\r\n",
                        		  stDivResult.rem);
                        nPktCnt = 1;
                    }

                }
            }
            else
            {
                nPktCnt = 0;
            }

            if ( stVerInfo.dwMasterPostpayPLPkt != nPktCnt )
            {
                DebugOut( "\r\n master P/L  postpay pack [%s][%d]\r\n",
                          stVerInfo.abMasterPostpayPLVer,
                          nPktCnt );
                stVerInfo.dwMasterPostpayPLPkt = nPktCnt;
            }
            
            close( fdTmpFile );
            break;

        case  45:   // master A/I data   ==> new prepay master
			printf( "[SetMasterBLPLVer] 고정신선불PL 이어받기 SIZE/Pkt# 설정\n" );
            BCD2ASC( stFileRelayRecvInfo.abDownFileVer,
                     stVerInfo.abMasterAIVer,
                     sizeof( stFileRelayRecvInfo.abDownFileVer ) );
            stVerInfo.dwMasterAIPkt =
                GetDWORDFromLITTLE( stFileRelayRecvInfo.abDownFilePktNo );

            if ( ( fdTmpFile = open( DOWN_ING_MASTER_AI_FILE, O_RDONLY,
                                     OPENMODE
                                    ) ) > 0 )
            {
                if ( fstat( fdTmpFile, &stFileStat ) < 0 )
                {
                    nPktCnt = 1;
                }
                else
                {
                    nFileSize = (int)stFileStat.st_size;
                    stDivResult = div( nFileSize, MAX_PKT_SIZE );
                    nPktCnt = stDivResult.quot;

                    if ( stDivResult.rem != 0 )
                    {
                        DebugOut( "stDivResult.rem != 0  [%d]\r\n",
                                  stDivResult.rem);
						nPktCnt = 1;
                    }

                }
            }
            else
            {
                nPktCnt = 0;
            }

            if ( stVerInfo.dwMasterAIPkt != nPktCnt )
            {
                DebugOut( "\r\n master  A/I  pack [%lu][%d]\r\n",
                          stVerInfo.dwMasterAIPkt,
                          nPktCnt );
                stVerInfo.dwMasterAIPkt = nPktCnt;
            }

            close( fdTmpFile );
            break;

        default     :
            break;
    }

    close( fdFileChain );

	if ( stFileRelayRecvInfo.chFileNo == 38 ||
		 stFileRelayRecvInfo.chFileNo == 41 ||
		 stFileRelayRecvInfo.chFileNo == 42 ||
		 stFileRelayRecvInfo.chFileNo == 45 )
	{
		printf( "[SetMasterBLPLVer] 실제파일사이즈      : %d\n", nFileSize );
		printf( "[SetMasterBLPLVer] 최종계산된 패킷번호 : %d\n", nPktCnt );
	}

    return sRetVal;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SetVerBLPLFileExistYN                                    *
*                                                                              *
*  DESCRIPTION :      고정 BLPL파일이 없으면 버전을 초기화한다.	               *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: None                                                     *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
void SetVerBLPLFileExistYN( void )
{

	/*
	 *	고정 BL 파일이 없으면 버전과 패킷을 0x00으로 세팅한다.
	 */
    if ( access( MASTER_BL_FILE,  F_OK ) != 0 )  // if file does not exist
    {
        DebugOut( "File not Exist[%s]\n", MASTER_BL_FILE );
        memset( stVerInfo.abMasterBLVer,
        		0x00,
                sizeof( stVerInfo.abMasterBLVer ) );
        stVerInfo.dwMasterBLPkt = 0;
    }

	/*
	 *	고정 PL(선불) 파일이 없으면 버전과 패킷을 0x00으로 세팅한다.
	 */
    if ( access( MASTER_PREPAY_PL_FILE,  F_OK ) != 0 )
    {
        DebugOut( "File not Exist[%s]\n", MASTER_PREPAY_PL_FILE );
        memset( stVerInfo.abMasterPrepayPLVer,
        		0x00,
                sizeof( stVerInfo.abMasterPrepayPLVer ) );
        stVerInfo.dwMasterPrepayPLPkt = 0;
    }

	/*
	 *	고정 PL(후불) 파일이 없으면 버전과 패킷을 0x00으로 세팅한다.
	 */
    if ( access( MASTER_POSTPAY_PL_FILE,  F_OK ) != 0 )
    {
        DebugOut( "File not Exist[%s]\n", MASTER_POSTPAY_PL_FILE );
        memset( stVerInfo.abMasterPostpayPLVer,
        		0x00,
                sizeof( stVerInfo.abMasterPostpayPLVer ) );
        stVerInfo.dwMasterPostpayPLPkt = 0;
    }

	/*
	 *	고정 AI 파일이 없으면 버전과 패킷을 0x00으로 세팅한다.
	 */
    if ( access( MASTER_AI_FILE,  F_OK ) != 0 )
    {
        DebugOut( "File not Exist[%s]\n", MASTER_AI_FILE );
        memset( stVerInfo.abMasterAIVer,
        		0x00,
                sizeof( stVerInfo.abMasterAIVer ) );
        stVerInfo.dwMasterAIPkt = 0;
    }
    
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      InitSAMRegistInfoVer                                     *
*                                                                              *
*  DESCRIPTION :      SAM등록정보의 버전을 초기화한다.	    	               *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: None                                                     *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short InitSAMRegistInfoVer( void )
{
    short 		sReturnVal 		= SUCCESS;
    VER_INFO 	stTmpVerInfo;

    DebugOut( "초기화 idcenter, keyset" );
    
    /*
     *	버전정보 로드
     */
    sReturnVal = LoadVerInfo( &stTmpVerInfo );

    if ( sReturnVal < SUCCESS )
    {
        return ErrRet( sReturnVal );
    }

	memset( stVerInfo.abEpurseIssuerRegistInfoVer,
            0,
            sizeof( stVerInfo.abEpurseIssuerRegistInfoVer ) );
    memset( stVerInfo.abPSAMKeysetVer,
            0,
            sizeof( stVerInfo.abPSAMKeysetVer ) );
    memset( stVerInfo.abAutoChargeKeysetVer,
            0,
            sizeof( stVerInfo.abAutoChargeKeysetVer ) );
    memset( stVerInfo.abAutoChargeParmVer,
            0,
            sizeof( stVerInfo.abAutoChargeParmVer ) );
	
	/*
	 *	버전정보 파일로 저장
	 */
    sReturnVal = SaveVerFile();

    return sReturnVal;

}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      WriteBLPLBefVer                                          *
*                                                                              *
*  DESCRIPTION :      변동 BLPL 파일의 이전 버전을 pachFileName에 write         *
*					  BL - v_bl_be.dat                                         *
*					  PL - v_pl_be.dat                                         *
*					  AI - v_ai_be.dat                                         *
*                                                                              *
*  INPUT PARAMETERS:  	char* pachFileName, - write할 파일명                   *
*						char* pachBefVer,   - write할 이전 버전                *
*						int nBefVerLength   - write할 length                   *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       ErrRet( ERR_FILE_OPEN | GetFileNo( pachFileName ) )	   *
*						ErrRet( ERR_FILE_WRITE | GetFileNo( pachFileName ) )   *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short WriteBLPLBefVer( char* pachFileName, char* pachBefVer, int nBefVerLength )
{
	short   sReturnVal          = SUCCESS;
    int     fdBefVer            = 0;    
    
    /*
     *	이전버전을 쓰기위해 파일 open
     */
    fdBefVer = open( pachFileName, O_RDWR | O_CREAT | O_TRUNC, OPENMODE );

    if ( fdBefVer < 0 )
    {
    	return ErrRet( ERR_FILE_OPEN | GetFileNo( pachFileName ) );
    }
 
    /*
     *	이전버전을 write
     */   
    if ( write( fdBefVer, pachBefVer, nBefVerLength ) != nBefVerLength )
    {
    	close( fdBefVer );
    	return ErrRet( ERR_FILE_WRITE | GetFileNo( pachFileName ) );
    }

    close( fdBefVer );
    
    return sReturnVal;
	
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      CreateVerInfoPkt                                         *
*                                                                              *
*  DESCRIPTION :      송신할 버전정보를 pkt에 세팅한다.                          *
*                                                                              *
*  INPUT PARAMETERS:  bool boolIsCurrVer, byte* pbVerInfoBuff                  *
*                                                                              *
*  RETURN/EXIT VALUE:     sizeof( VER_INFO_FOR_DOWN )                          *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short CreateVerInfoPkt( bool boolIsCurrVer, byte* pbVerInfoBuff )
{
    char 	chTmpFileNo;
    long 	lFileSize 	= 0;
    VER_INFO_FOR_DOWN  *pstVerInfoForDown = (VER_INFO_FOR_DOWN*)pbVerInfoBuff;

    pstVerInfoForDown->bSystemClassCode = BUS_SYSTEM_CLASS_CD; 	// System class
    memcpy( pstVerInfoForDown->achVehicleID,
            gstVehicleParm.abVehicleID,
            sizeof( gstVehicleParm.abVehicleID ) );         	// vehicle ID
    pstVerInfoForDown->bPktClassCode = NOT_EXIST_MORE_PKT;
    memcpy( pstVerInfoForDown->achVerInfoCnt,
            VER_INFO_CNT,
            strlen( VER_INFO_CNT ) );                       	// Version count
    pstVerInfoForDown->chOldNewClassCode = GetASCNoFromDEC((byte)boolIsCurrVer );

    if ( boolIsCurrVer == CURR )    // Current Version
    {
        memcpy( pstVerInfoForDown->achDriverOperatorApplCmd,
                achDCSCommFile[0][0],
                COMMAND_LENGTH ); 	//terminal Application 1 : driver operator
        ASC2BCD( stVerInfo.abDriverOperatorApplVer,
                 pstVerInfoForDown->achDriverOperatorApplVer,
                 sizeof( stVerInfo.abDriverOperatorApplVer ) );
        memcpy( &pstVerInfoForDown->dwDriverOperatorApplFileSize,
                (char*)&stVerInfo.dwDriverOperatorApplPkt,
                SIZE_LENGTH );
        memcpy( &pstVerInfoForDown->dwDriverOperatorApplPktSize,
                (char*)&stVerInfo.dwDriverOperatorApplPkt,
                SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achMainTermApplCmd,
        		achDCSCommFile[1][0],
                COMMAND_LENGTH ); 	// terminal Application 2 : main terminal
        ASC2BCD( stVerInfo.abMainTermApplVer,
                pstVerInfoForDown->achMainTermApplVer,
                 VER_INFO_LENGTH );
        memcpy( &pstVerInfoForDown->dwMainTermApplFileSize,
                (char*)&stVerInfo.dwMainTermApplPkt,
                SIZE_LENGTH );
        memcpy( &pstVerInfoForDown->dwMainTermApplPktSize,
                (char*)&stVerInfo.dwMainTermApplPkt,
                SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achSubTermApplCmd,
        		achDCSCommFile[2][0],
                COMMAND_LENGTH );  	// terminal Application 3 : sub terminal
        ASC2BCD( stVerInfo.abSubTermApplVer,
                 pstVerInfoForDown->achSubTermApplVer,
                 VER_INFO_LENGTH );
        memcpy( &pstVerInfoForDown->dwSubTermApplFileSize,
                (char*)&stVerInfo.dwSubTermApplPkt,
                SIZE_LENGTH );
        memcpy( &pstVerInfoForDown->dwSubTermApplPktSize,
                (char*)&stVerInfo.dwSubTermApplPkt,
                SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achAppl4Cmd,
        		achDCSCommFile[3][0],
                COMMAND_LENGTH ); 	// terminal Application 4 :
        memset( pstVerInfoForDown->achAppl4Ver,
        		0x99,
                sizeof( pstVerInfoForDown->achAppl4Ver ) );
        memset( &pstVerInfoForDown->dwAppl4FileSize, 0x00, SIZE_LENGTH );
        memset( &pstVerInfoForDown->dwAppl4PktSize, 0x00, SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achAppl5Cmd,
        		achDCSCommFile[4][0],
                COMMAND_LENGTH );	// terminal Application 5 :
        memset( pstVerInfoForDown->achAppl5Ver,
        		0x99,
                sizeof( pstVerInfoForDown->achAppl5Ver ) );
        memset( &pstVerInfoForDown->dwAppl5FileSize, 0x00, SIZE_LENGTH );
        memset( &pstVerInfoForDown->dwAppl5PktSize, 0x00, SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achAppl6Cmd,
        		achDCSCommFile[5][0],
                COMMAND_LENGTH );	// terminal Application 6 :
        memset( pstVerInfoForDown->achAppl6Ver,
        		0x99,
                sizeof( pstVerInfoForDown->achAppl6Ver ) );
        memset( &pstVerInfoForDown->dwAppl6FileSize, 0x00, SIZE_LENGTH );
        memset( &pstVerInfoForDown->dwAppl6PktSize, 0x00, SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achAppl7Cmd,
        		achDCSCommFile[6][0],
                COMMAND_LENGTH );	// terminal Application 7 :
        memset( pstVerInfoForDown->achAppl7Ver,
        		0x99,
                sizeof( pstVerInfoForDown->achAppl7Ver ) );
        memset( &pstVerInfoForDown->dwAppl7FileSize, 0x00, SIZE_LENGTH );
        memset( &pstVerInfoForDown->dwAppl7PktSize, 0x00, SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achAppl8Cmd,
        		achDCSCommFile[7][0],
                COMMAND_LENGTH );	// terminal Application 8 :
        memset( pstVerInfoForDown->achAppl8Ver,
        		0x99,
                sizeof( pstVerInfoForDown->achAppl8Ver ) );
        memset( &pstVerInfoForDown->dwAppl8FileSize, 0x00, SIZE_LENGTH );
        memset( &pstVerInfoForDown->dwAppl8PktSize, 0x00, SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achAppl9Cmd,
        		achDCSCommFile[8][0],
                COMMAND_LENGTH ); 	// terminal Application 9 :
        memset( pstVerInfoForDown->achAppl9Ver,
        		0x99,
                sizeof( pstVerInfoForDown->achAppl9Ver ) );
        memset( &pstVerInfoForDown->dwAppl9FileSize, 0x00, SIZE_LENGTH );
        memset( &pstVerInfoForDown->dwAppl9PktSize, 0x00, SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achAppl10Cmd,
        		achDCSCommFile[9][0],
                COMMAND_LENGTH );	// terminal Application 10 :
        memset( pstVerInfoForDown->achAppl10Ver, 0x99,
                sizeof( pstVerInfoForDown->achAppl10Ver ) );
        memset( &pstVerInfoForDown->dwAppl10FileSize, 0x00, SIZE_LENGTH );
        memset( &pstVerInfoForDown->dwAppl10PktSize, 0x00, SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achVoice0Cmd,
        		achDCSCommFile[10][0],
                COMMAND_LENGTH );		// terminal voice  information 0 :
        ASC2BCD( stVerInfo.abVoiceInfoVer,
        		 pstVerInfoForDown->achVoice0InfoVer,
                 VER_INFO_LENGTH );
        memcpy( &pstVerInfoForDown->dwVoice0FileSize,
                (char*)&stVerInfo.dwVoiceInfoPkt,
                SIZE_LENGTH );
        memcpy( &pstVerInfoForDown->dwVoice0PktSize,
                (char*)&stVerInfo.dwVoiceInfoPkt,
                SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achVoice1Cmd,
        		achDCSCommFile[11][0],
                COMMAND_LENGTH );       // terminal voice  information 1 :
        memset( pstVerInfoForDown->achVoice1InfoVer,
        		0x99,
                sizeof( pstVerInfoForDown->achVoice1InfoVer ) );
        memset( &pstVerInfoForDown->dwVoice1FileSize, 0x00, SIZE_LENGTH );
        memset( &pstVerInfoForDown->dwVoice1PktSize, 0x00, SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achVoice2Cmd,
        		achDCSCommFile[12][0],
                COMMAND_LENGTH ); 		// terminal voice  information 2 :
        memset( pstVerInfoForDown->achVoice2InfoVer,
        		0x99,
                sizeof( pstVerInfoForDown->achVoice2InfoVer ) );
        memset( &pstVerInfoForDown->dwVoice2FileSize, 0x00, SIZE_LENGTH );
        memset( &pstVerInfoForDown->dwVoice2PktSize, 0x00, SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achVoice3Cmd,
        		achDCSCommFile[13][0],
                COMMAND_LENGTH );       // terminal voice  information 3 :
        memset( pstVerInfoForDown->achVoice3InfoVer,
        		0x99,
                sizeof( pstVerInfoForDown->achVoice3InfoVer ) );
        memset( &pstVerInfoForDown->dwVoice3FileSize, 0x00, SIZE_LENGTH );
        memset( &pstVerInfoForDown->dwVoice3PktSize, 0x00, SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achVoice4Cmd,
        		achDCSCommFile[14][0],
                COMMAND_LENGTH );		// terminal voice  information 4 :
        memset( pstVerInfoForDown->achVoice4InfoVer,
        		0x99,
                sizeof( pstVerInfoForDown->achVoice4InfoVer ) );
        memset( &pstVerInfoForDown->dwVoice4FileSize, 0x00, SIZE_LENGTH );
        memset( &pstVerInfoForDown->dwVoice4PktSize, 0x00, SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achVoice5Cmd,
        		achDCSCommFile[15][0],
                COMMAND_LENGTH );       // terminal voice  information 5 :
        memset( pstVerInfoForDown->achVoice5InfoVer,
        		0x99,
                sizeof( pstVerInfoForDown->achVoice5InfoVer ) );
        memset( &pstVerInfoForDown->dwVoice5FileSize, 0x00, SIZE_LENGTH );
        memset( &pstVerInfoForDown->dwVoice5PktSize, 0x00, SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achVoice6Cmd,
        		achDCSCommFile[16][0],
                COMMAND_LENGTH );		// terminal voice  information 6 :
        memset( pstVerInfoForDown->achVoice6InfoVer,
        		0x99,
                sizeof( pstVerInfoForDown->achVoice6InfoVer ) );
        memset( &pstVerInfoForDown->dwVoice6FileSize, 0x00, SIZE_LENGTH );
        memset( &pstVerInfoForDown->dwVoice6PktSize, 0x00, SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achVoice7Cmd,
        		achDCSCommFile[17][0],
                COMMAND_LENGTH );       // terminal voice  information 7 :
        memset( pstVerInfoForDown->achVoice7InfoVer,
        		0x99,
                sizeof( pstVerInfoForDown->achVoice7InfoVer ) );
        memset( &pstVerInfoForDown->dwVoice7FileSize, 0x00, SIZE_LENGTH );
        memset( &pstVerInfoForDown->dwVoice7PktSize, 0x00, SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achVoice8Cmd,
        		achDCSCommFile[18][0],
                COMMAND_LENGTH );       // terminal voice  information 8 :
        memset( pstVerInfoForDown->achVoice8InfoVer,
        		0x99,
                sizeof( pstVerInfoForDown->achVoice8InfoVer ) );
        memset( &pstVerInfoForDown->dwVoice8FileSize, 0x00, SIZE_LENGTH );
        memset( &pstVerInfoForDown->dwVoice8PktSize, 0x00, SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achVoice9Cmd,
        		achDCSCommFile[19][0],
                COMMAND_LENGTH );       // terminal voice  information 9 :
        memset( pstVerInfoForDown->achVoice9InfoVer,
        		0x99,
                sizeof( pstVerInfoForDown->achVoice9InfoVer ) );
        memset( &pstVerInfoForDown->dwVoice9FileSize, 0x00, SIZE_LENGTH );
        memset( &pstVerInfoForDown->dwVoice9PktSize, 0x00, SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achVehicleParmCmd,
        		achDCSCommFile[20][0],
                COMMAND_LENGTH );		// running vehicle parameter
        ASC2BCD( stVerInfo.abVehicleParmVer,
                 pstVerInfoForDown->achVehicleParmVer,
                 VER_INFO_LENGTH );
        memcpy( &pstVerInfoForDown->dwVehicleParmFileSize,
                (char*)&stVerInfo.dwVehicleParmPkt,
                SIZE_LENGTH );
        memcpy( &pstVerInfoForDown->dwVehicleParmPktSize,
                (char*)&stVerInfo.dwVehicleParmPkt,
                SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achRouteParmCmd,
        		achDCSCommFile[21][0],
                COMMAND_LENGTH );		// bus terminal route  parameter
        ASC2BCD( stVerInfo.abRouteParmVer, pstVerInfoForDown->achRouteParmVer,
                 VER_INFO_LENGTH );
        memcpy( &pstVerInfoForDown->dwRouteParmFileSize,
                (char*)&stVerInfo.dwRouteParmPkt,
                SIZE_LENGTH );
        memcpy( &pstVerInfoForDown->dwRouteParmPktSize,
                (char*)&stVerInfo.dwRouteParmPkt,
                SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achBasicFareInfoCmd,
        		achDCSCommFile[22][0],
                COMMAND_LENGTH );		// new fare  information
        ASC2BCD( stVerInfo.abBasicFareInfoVer,
                 pstVerInfoForDown->achBasicFareInfoVer,
                 VER_INFO_LENGTH );
        memcpy( &pstVerInfoForDown->dwBasicFareInfoFileSize,
                (char*)&stVerInfo.dwBasicFareInfoPkt,
                SIZE_LENGTH );
        memcpy( &pstVerInfoForDown->dwBasicFareInfoPktSize,
                (char*)&stVerInfo.dwBasicFareInfoPkt,
                SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achOldFareCmd,
        		achDCSCommFile[23][0],
                COMMAND_LENGTH );		// old single fare  information
        ASC2BCD( stVerInfo.abOldFareVer,
        		 pstVerInfoForDown->achOldFareVer,
                 VER_INFO_LENGTH );
        memcpy( &pstVerInfoForDown->dwOldFareFileSize,
                (char*)&stVerInfo.dwOldFarePkt,
                SIZE_LENGTH );
        memcpy( &pstVerInfoForDown->dwOldFarePktSize,
                (char*)&stVerInfo.dwOldFarePkt,
                SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achOldAirFareCmd,
        		achDCSCommFile[24][0],
                COMMAND_LENGTH );		// old airport fare  information
        ASC2BCD( stVerInfo.abOldAirFareVer,
        		 pstVerInfoForDown->achOldAirFareVer,
                 VER_INFO_LENGTH );
        memcpy( &pstVerInfoForDown->dwOldAirFareFileSize,
                (char*)&stVerInfo.dwOldAirFarePkt,
                SIZE_LENGTH );
        memcpy( &pstVerInfoForDown->dwOldAirFarePktSize,
                (char*)&stVerInfo.dwOldAirFarePkt,
                SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achOldRangeFareCmd,
        		achDCSCommFile[25][0],
                COMMAND_LENGTH );		// old old interval fare  information
        ASC2BCD( stVerInfo.abOldRangeFareVer,
                 pstVerInfoForDown->achOldRangeFareVer,
                 VER_INFO_LENGTH );
        memcpy( &pstVerInfoForDown->dwOldRangeFareFileSize,
                (char*)&stVerInfo.dwOldRangeFarePkt,
                SIZE_LENGTH );
        memcpy( &pstVerInfoForDown->dwOldRangeFarePktSize,
                (char*)&stVerInfo.dwOldRangeFarePkt,
                SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achBusStationInfCmd,
        		achDCSCommFile[26][0],
                COMMAND_LENGTH );		// station  information
        ASC2BCD( stVerInfo.abBusStationInfoVer,
                 pstVerInfoForDown->achBusStationInfoVer,
                 VER_INFO_LENGTH );
        memcpy( &pstVerInfoForDown->dwBusStationInfFileSize,
                (char*)&stVerInfo.dwBusStationInfoPkt,
                SIZE_LENGTH );
        memcpy( &pstVerInfoForDown->dwBusStationInfoPktSize,
                (char*)&stVerInfo.dwBusStationInfoPkt,
                SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achPrepayIssuerInfoCmd,
        		achDCSCommFile[27][0],
                COMMAND_LENGTH );		// prepay issuer  information
        ASC2BCD( stVerInfo.abPrepayIssuerInfoVer,
                pstVerInfoForDown->achPrepayIssuerInfoVer,
                 VER_INFO_LENGTH );
        memcpy( &pstVerInfoForDown->dwPrepayIssuerInfoFileSize,
                (char*)&stVerInfo.dwPrepayIssuerInfoPkt,
                SIZE_LENGTH );
        memcpy( &pstVerInfoForDown->dwPrepayIssuerInfoPktSize,
                (char*)&stVerInfo.dwPrepayIssuerInfoPkt,
                SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achPostpayIssuerInfoCmd,
                achDCSCommFile[28][0],
                COMMAND_LENGTH );		// postpay issuer  information
        ASC2BCD( stVerInfo.abPostpayIssuerInfoVer,
                 pstVerInfoForDown->achPostpayIssuerInfoVer,
                 VER_INFO_LENGTH );
        memcpy( &pstVerInfoForDown->dwPostpayIssuerInfoFileSize,
                (char*)&stVerInfo.dwPostpayIssuerInfoPkt,
                SIZE_LENGTH );
        memcpy( &pstVerInfoForDown->dwPostpayIssuerInfoPktSize,
                (char*)&stVerInfo.dwPostpayIssuerInfoPkt,
                SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achDisExtraInfoCmd,
                achDCSCommFile[29][0],
                COMMAND_LENGTH );		// discount extra  information
        ASC2BCD( stVerInfo.abDisExtraInfoVer,
                 pstVerInfoForDown->achDisExtraInfoVer,
                 VER_INFO_LENGTH );
        memcpy( &pstVerInfoForDown->dwDisExtraInfoFileSize,
                (char*)&stVerInfo.dwDisExtraInfoPkt,
                SIZE_LENGTH );
        memcpy( &pstVerInfoForDown->dwDisExtraInfoPktSize,
                (char*)&stVerInfo.dwDisExtraInfoPkt,
                SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achHolidayInfoCmd,
                achDCSCommFile[30][0],
                COMMAND_LENGTH ); 		// holiday  information
        ASC2BCD( stVerInfo.abHolidayInfoVer,
                 pstVerInfoForDown->achHolidayInfoVer,
                 VER_INFO_LENGTH );
        memcpy( &pstVerInfoForDown->dwHolidayInfoFileSize,
                (char*)&stVerInfo.dwHolidayInfoPkt,
                SIZE_LENGTH );
        memcpy( &pstVerInfoForDown->dwHolidayInfoPktSize,
                (char*)&stVerInfo.dwHolidayInfoPkt,
                SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achXferApplyRegulationCmd,
                achDCSCommFile[31][0],
                COMMAND_LENGTH );		// transfer condition  information
        ASC2BCD( stVerInfo.abXferApplyRegulationVer,
                 pstVerInfoForDown->achXferApplyRegulationVer,
                 VER_INFO_LENGTH );
        memcpy( &pstVerInfoForDown->dwXferApplyRegulationFileSize,
                (char*)&stVerInfo.dwXferApplyRegulationPkt,
                SIZE_LENGTH );
        memcpy( &pstVerInfoForDown->dwXferApplyRegulationPktSize,
                (char*)&stVerInfo.dwXferApplyRegulationPkt,
                SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achEpurseIssuerRegistInfoCmd,
                achDCSCommFile[32][0],
                COMMAND_LENGTH );		// Issuer registration count
        ASC2BCD( stVerInfo.abEpurseIssuerRegistInfoVer,
                 pstVerInfoForDown->achEpurseIssuerRegistInfoVer,
                 VER_INFO_LENGTH );
        memcpy( &pstVerInfoForDown->dwEpurseIssuerRegistInfoFileSize,
                (char*)&stVerInfo.dwEpurseIssuerRegistInfoPkt,
                SIZE_LENGTH );
        memcpy( &pstVerInfoForDown->dwEpurseIssuerRegistInfoPktSize,
                (char*)&stVerInfo.dwEpurseIssuerRegistInfoPkt,
                SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achPSAMKeysetCmd,
        		achDCSCommFile[33][0],
                COMMAND_LENGTH );		// payment SAM Key Set  information
        ASC2BCD( stVerInfo.abPSAMKeysetVer,
        		 pstVerInfoForDown->achPSAMKeysetVer,
                 VER_INFO_LENGTH );
        memcpy( &pstVerInfoForDown->dwPSAMKeysetFileSize,
                (char*)&stVerInfo.dwPSAMKeysetPkt,
                SIZE_LENGTH );
        memcpy( &pstVerInfoForDown->dwPSAMKeysetPktSize,
                (char*)&stVerInfo.dwPSAMKeysetPkt,
                SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achAutoChargeKeyseCmd,
        		achDCSCommFile[34][0],
                COMMAND_LENGTH );		// auto-charge SAM Key Set information
        ASC2BCD( stVerInfo.abAutoChargeKeysetVer,
                 pstVerInfoForDown->achAutoChargeKeysetVer,
                 VER_INFO_LENGTH );
        memcpy( &pstVerInfoForDown->dwAutoChargeKeyseFileSize,
                (char*)&stVerInfo.dwAutoChargeKeysetPkt,
                SIZE_LENGTH );
        memcpy( &pstVerInfoForDown->dwAutoChargeKeysetPktSize,
                (char*)&stVerInfo.dwAutoChargeKeysetPkt,
                SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achAutoChargeParmCmd,
        		achDCSCommFile[35][0],
                COMMAND_LENGTH );		// auto-charge parameter information
        ASC2BCD( stVerInfo.abAutoChargeParmVer,
                 pstVerInfoForDown->achAutoChargeParmVer,
                 VER_INFO_LENGTH );
        memcpy( &pstVerInfoForDown->dwAutoChargeParmFileSize,
                (char*)&stVerInfo.dwAutoChargeParmPkt,
                SIZE_LENGTH );
        memcpy( &pstVerInfoForDown->dwAutoChargeParmPktSize,
                (char*)&stVerInfo.dwAutoChargeParmPkt,
                SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achXferApplyInfoCmd,
        		achDCSCommFile[36][0],
                COMMAND_LENGTH );		// transfer apply
        ASC2BCD( stVerInfo.abXferApplyInfoVer,
                 pstVerInfoForDown->achXferApplyInfoVer,
                 VER_INFO_LENGTH );
        memcpy( &pstVerInfoForDown->dwXferApplyInfoFileSize,
                (char*)&stVerInfo.dwXferApplyInfoPkt,
                SIZE_LENGTH );
        memcpy( &pstVerInfoForDown->dwXferApplyInfoPktSize,
                (char*)&stVerInfo.dwXferApplyInfoPkt,
                SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achIssuerValidPeriodInfoCmd,
                achDCSCommFile[37][0],
                COMMAND_LENGTH );		// issuer valid period check
        ASC2BCD( stVerInfo.abIssuerValidPeriodInfoVer,
                 pstVerInfoForDown->achIssuerValidPeriodInfoVer,
                 VER_INFO_LENGTH );
        memcpy( &pstVerInfoForDown->dwIssuerValidPeriodInfoFileSize,
                (char*)&stVerInfo.dwIssuerValidPeriodInfoPkt,
                SIZE_LENGTH );
        memcpy( &pstVerInfoForDown->dwIssuerValidPeriodInfoPktSize,
                (char*)&stVerInfo.dwIssuerValidPeriodInfoPkt,
                SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achMasterBLCmd,
        		achDCSCommFile[38][0],
                COMMAND_LENGTH );		// master BL information
        ASC2BCD( stVerInfo.abMasterBLVer,
        		 pstVerInfoForDown->achMasterBLVer,
                 VER_INFO_LENGTH );
        chTmpFileNo = 38;
        GetRelayRecvFileSize( chTmpFileNo, &lFileSize );
        memcpy( &pstVerInfoForDown->dwMasterBLFileSize,
                (char*)&lFileSize, SIZE_LENGTH );
        memcpy( &pstVerInfoForDown->dwMasterBLPktSize,
                (char*)&stVerInfo.dwMasterBLPkt,
                SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achUpdateBLCmd,
        		achDCSCommFile[39][0],
                COMMAND_LENGTH );		// update BL information
#ifdef TEST_UPDATE_ONE_DAY_BL_PL_AI
        tYesterday = GetTimeTFromASCDtime( stVerInfo.abUpdateBLVer );
        tYesterday -= 86400;
        TimeT2BCDDtime( tYesterday, pstVerInfoForDown->achUpdateBLVer );
#else
        ASC2BCD( stVerInfo.abUpdateBLVer,
        		 pstVerInfoForDown->achUpdateBLVer,
                 VER_INFO_LENGTH );
#endif
        memcpy( &pstVerInfoForDown->dwUpdateBLFileSize,
                (char*)&stVerInfo.dwUpdateBLPkt,
                SIZE_LENGTH );
        memcpy( &pstVerInfoForDown->dwUpdateBLPktSize,
                (char*)&stVerInfo.dwUpdateBLPkt,
                SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achHotBLCmd,
        		achDCSCommFile[40][0],
                COMMAND_LENGTH );		// HOT BL  information
        ASC2BCD( stVerInfo.abHotBLVer,
        		 pstVerInfoForDown->achHotBLVer,
                 VER_INFO_LENGTH );
        memcpy( &pstVerInfoForDown->dwHotBLFileSize,
                (char*)&stVerInfo.dwHotBLPkt, SIZE_LENGTH );
        memcpy( &pstVerInfoForDown->dwHotBLPktSize,
        		(char*)&stVerInfo.dwHotBLPkt,
                SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achMasterPrepayPLCmd,
        		achDCSCommFile[41][0],
                COMMAND_LENGTH );		// master PL information - prepay
        ASC2BCD( stVerInfo.abMasterPrepayPLVer,
                 pstVerInfoForDown->achMasterPrepayPLVer,
                 VER_INFO_LENGTH );
        chTmpFileNo = 41;
        GetRelayRecvFileSize( chTmpFileNo, &lFileSize );
        memcpy( &pstVerInfoForDown->dwMasterPrepayPLFileSize,
                (char*)&lFileSize, SIZE_LENGTH );
        memcpy( &pstVerInfoForDown->dwMasterPrepayPLPktSize,
                (char*)&stVerInfo.dwMasterPrepayPLPkt,
                SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achMasterPostpayPLCmd,
        		achDCSCommFile[42][0],
                COMMAND_LENGTH );		// master PL information - postpay
        ASC2BCD( stVerInfo.abMasterPostpayPLVer,
                 pstVerInfoForDown->achMasterPostpayPLVer,
                 VER_INFO_LENGTH );
        chTmpFileNo = 42;
        GetRelayRecvFileSize( chTmpFileNo, &lFileSize );
        memcpy( &pstVerInfoForDown->dwMasterPostpayPLFileSize,
                (char*)&lFileSize,
                SIZE_LENGTH );
        memcpy( &pstVerInfoForDown->dwMasterPostpayPLPktSize,
                (char*)&stVerInfo.dwMasterPostpayPLPkt,
                SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achUpdatePLCmd,
        		achDCSCommFile[43][0],
                COMMAND_LENGTH );		// update PL information
#ifdef TEST_UPDATE_ONE_DAY_BL_PL_AI
        tYesterday = GetTimeTFromASCDtime( stVerInfo.abUpdatePLVer );
        tYesterday -= 86400;
        TimeT2BCDDtime( tYesterday, pstVerInfoForDown->achUpdatePLVer );
#else
        ASC2BCD( stVerInfo.abUpdatePLVer,
        		 pstVerInfoForDown->achUpdatePLVer,
                 VER_INFO_LENGTH );
#endif
        memcpy( &pstVerInfoForDown->dwUpdatePLFileSize,
                (char*)&stVerInfo.dwUpdatePLPkt,
                SIZE_LENGTH );
        memcpy( &pstVerInfoForDown->dwUpdatePLPktSize,
                (char*)&stVerInfo.dwUpdatePLPkt,
                SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achHotPLCmd,
        		achDCSCommFile[44][0],
                COMMAND_LENGTH );		// HOT PL information
        ASC2BCD( stVerInfo.abHotPLVer,
        		 pstVerInfoForDown->achHotPLVer,
                 VER_INFO_LENGTH );
        memcpy( &pstVerInfoForDown->dwHotPLFileSize,
                (char*)&stVerInfo.dwHotPLVerPkt,
                SIZE_LENGTH );
        memcpy( &pstVerInfoForDown->dwHotPLPktSize,
                (char*)&stVerInfo.dwHotPLVerPkt,
                SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achMasterAICmd,
        		achDCSCommFile[45][0],
                COMMAND_LENGTH );		// master AI  information 
        ASC2BCD( stVerInfo.abMasterAIVer,
        		 pstVerInfoForDown->achMasterAIVer,
                 VER_INFO_LENGTH );
        chTmpFileNo = 45;
        GetRelayRecvFileSize( chTmpFileNo, &lFileSize );
        memcpy( &pstVerInfoForDown->dwMasterAIFileSize,
        		(char*)&lFileSize,
                SIZE_LENGTH );
        memcpy( &pstVerInfoForDown->dwMasterAIPktSize,
                (char*)&stVerInfo.dwMasterAIPkt,
                SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achUpdateAICmd,
        		achDCSCommFile[46][0],
                COMMAND_LENGTH );		// update AI information
#ifdef TEST_UPDATE_ONE_DAY_BL_PL_AI
        tYesterday = GetTimeTFromASCDtime( stVerInfo.abUpdateAIVer );
        tYesterday -= 86400;
        TimeT2BCDDtime(tYesterday, pstVerInfoForDown->achUpdateAIVer);
#else
        ASC2BCD( stVerInfo.abUpdateAIVer,
        		 pstVerInfoForDown->achUpdateAIVer,
                 VER_INFO_LENGTH );
#endif
        memcpy( &pstVerInfoForDown->dwUpdateAIFileSize,
                (char*)&stVerInfo.dwUpdateAIPkt,
                SIZE_LENGTH );
        memcpy( &pstVerInfoForDown->dwUpdateAIPktSize,
                (char*)&stVerInfo.dwUpdateAIPkt,
                SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achHotAICmd, 
        		achDCSCommFile[47][0],
                COMMAND_LENGTH );		// HOT AI information
        ASC2BCD( stVerInfo.abHotAIVer,
        		 pstVerInfoForDown->achHotAIVer,
                 VER_INFO_LENGTH );
        memcpy( &pstVerInfoForDown->dwHotAIFileSize,
                (char*)&stVerInfo.dwHotAIPkt,
                SIZE_LENGTH );
        memcpy( &pstVerInfoForDown->dwHotAIPktSize,
        		(char*)&stVerInfo.dwHotAIPkt,
                SIZE_LENGTH );

    }

    if ( boolIsCurrVer == NEXT )    // New Version
    {

        memcpy( pstVerInfoForDown->achDriverOperatorApplCmd,
                achDCSCommFile[0][0],
                COMMAND_LENGTH );   // terminal Application 1 : driver operator
        ASC2BCD( stVerInfo.abNextDriverOperatorApplVer,
                 pstVerInfoForDown->achDriverOperatorApplVer,
                 sizeof( stVerInfo.abDriverOperatorApplVer ) );
        memcpy( &pstVerInfoForDown->dwDriverOperatorApplFileSize,
                (char*)&stVerInfo.dwNextDriverOperatorApplPkt,
                SIZE_LENGTH );
        memcpy( &pstVerInfoForDown->dwDriverOperatorApplPktSize,
                (char*)&stVerInfo.dwNextDriverOperatorApplPkt,
                SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achMainTermApplCmd,
        		achDCSCommFile[1][0],
                COMMAND_LENGTH );   // terminal Application 2 : main terminal
        ASC2BCD( stVerInfo.abNextMainTermApplVer,
                pstVerInfoForDown->achMainTermApplVer,
                 VER_INFO_LENGTH );
        memcpy( &pstVerInfoForDown->dwMainTermApplFileSize,
                (char*)&stVerInfo.dwNextMainTermApplPkt,
                SIZE_LENGTH );
        memcpy( &pstVerInfoForDown->dwMainTermApplPktSize,
                (char*)&stVerInfo.dwNextMainTermApplPkt,
                SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achSubTermApplCmd,
        		achDCSCommFile[2][0],
                COMMAND_LENGTH );	// terminal Application 3 : sub terminal
        ASC2BCD( stVerInfo.abNextSubTermApplVer,
                 pstVerInfoForDown->achSubTermApplVer,
                 VER_INFO_LENGTH );
        memcpy( &pstVerInfoForDown->dwSubTermApplFileSize,
                (char*)&stVerInfo.dwNextSubTermApplPkt,
                SIZE_LENGTH );
        memcpy( &pstVerInfoForDown->dwSubTermApplPktSize,
                (char*)&stVerInfo.dwNextSubTermApplPkt,
                SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achAppl4Cmd,
        		achDCSCommFile[3][0],
                COMMAND_LENGTH ); 	// terminal Application 4 :
        memset( pstVerInfoForDown->achAppl4Ver,
        		0x99,
                sizeof( pstVerInfoForDown->achAppl4Ver ) );
        memset( &pstVerInfoForDown->dwAppl4FileSize, 0x00, SIZE_LENGTH );
        memset( &pstVerInfoForDown->dwAppl4PktSize, 0x00, SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achAppl5Cmd,
        		achDCSCommFile[4][0],
                COMMAND_LENGTH );	// terminal Application 5 :
        memset( pstVerInfoForDown->achAppl5Ver,
        		0x99,
                sizeof( pstVerInfoForDown->achAppl5Ver ) );
        memset( &pstVerInfoForDown->dwAppl5FileSize, 0x00, SIZE_LENGTH );
        memset( &pstVerInfoForDown->dwAppl5PktSize, 0x00, SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achAppl6Cmd,
        		achDCSCommFile[5][0],
                COMMAND_LENGTH );	// terminal Application 6 :
        memset( pstVerInfoForDown->achAppl6Ver,
        		0x99,
                sizeof( pstVerInfoForDown->achAppl6Ver ) );
        memset( &pstVerInfoForDown->dwAppl6FileSize, 0x00, SIZE_LENGTH );
        memset( &pstVerInfoForDown->dwAppl6PktSize, 0x00, SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achAppl7Cmd,
        		achDCSCommFile[6][0],
                COMMAND_LENGTH );	// terminal Application 7 :
        memset( pstVerInfoForDown->achAppl7Ver,
        		0x99,
                sizeof( pstVerInfoForDown->achAppl7Ver ) );
        memset( &pstVerInfoForDown->dwAppl7FileSize, 0x00, SIZE_LENGTH );
        memset( &pstVerInfoForDown->dwAppl7PktSize, 0x00, SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achAppl8Cmd,
        		achDCSCommFile[7][0],
                COMMAND_LENGTH );	// terminal Application 8 :
        memset( pstVerInfoForDown->achAppl8Ver,
        		0x99,
                sizeof( pstVerInfoForDown->achAppl8Ver ) );
        memset( &pstVerInfoForDown->dwAppl8FileSize, 0x00, SIZE_LENGTH );
        memset( &pstVerInfoForDown->dwAppl8PktSize, 0x00, SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achAppl9Cmd,
        		achDCSCommFile[8][0],
                COMMAND_LENGTH ); 	// terminal Application 9 :
        memset( pstVerInfoForDown->achAppl9Ver,
        		0x99,
                sizeof( pstVerInfoForDown->achAppl9Ver ) );
        memset( &pstVerInfoForDown->dwAppl9FileSize, 0x00, SIZE_LENGTH );
        memset( &pstVerInfoForDown->dwAppl9PktSize, 0x00, SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achAppl10Cmd,
        		achDCSCommFile[9][0],
                COMMAND_LENGTH );	// terminal Application 10 :
        memset( pstVerInfoForDown->achAppl10Ver,
        		0x99,
                sizeof( pstVerInfoForDown->achAppl10Ver ) );
        memset( &pstVerInfoForDown->dwAppl10FileSize, 0x00, SIZE_LENGTH );
        memset( &pstVerInfoForDown->dwAppl10PktSize, 0x00, SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achVoice0Cmd,
        		achDCSCommFile[10][0],
                COMMAND_LENGTH );		// terminal voice  information 0 :
        ASC2BCD( stVerInfo.abNextVoiceInfoVer,
                 pstVerInfoForDown->achVoice0InfoVer,
                 VER_INFO_LENGTH );
        memcpy( &pstVerInfoForDown->dwVoice0FileSize,
                (char*)&stVerInfo.dwNextVoiceInfoPkt,
                SIZE_LENGTH );
        memcpy( &pstVerInfoForDown->dwVoice0PktSize,
                (char*)&stVerInfo.dwNextVoiceInfoPkt,
                SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achVoice1Cmd,
        		achDCSCommFile[11][0],
                COMMAND_LENGTH );       // terminal voice  information 1 :
        memset( pstVerInfoForDown->achVoice1InfoVer,
        		0x99,
                sizeof( pstVerInfoForDown->achVoice1InfoVer ) );
        memset( &pstVerInfoForDown->dwVoice1FileSize, 0x00, SIZE_LENGTH );
        memset( &pstVerInfoForDown->dwVoice1PktSize, 0x00, SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achVoice2Cmd,
        		achDCSCommFile[12][0],
                COMMAND_LENGTH ); 		// terminal voice  information 2 :
        memset( pstVerInfoForDown->achVoice2InfoVer,
        		0x99,
                sizeof( pstVerInfoForDown->achVoice2InfoVer ) );
        memset( &pstVerInfoForDown->dwVoice2FileSize, 0x00, SIZE_LENGTH );
        memset( &pstVerInfoForDown->dwVoice2PktSize, 0x00, SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achVoice3Cmd,
        		achDCSCommFile[13][0],
                COMMAND_LENGTH );       // terminal voice  information 3 :
        memset( pstVerInfoForDown->achVoice3InfoVer,
        		0x99,
                sizeof( pstVerInfoForDown->achVoice3InfoVer ) );
        memset( &pstVerInfoForDown->dwVoice3FileSize, 0x00, SIZE_LENGTH );
        memset( &pstVerInfoForDown->dwVoice3PktSize, 0x00, SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achVoice4Cmd,
        		achDCSCommFile[14][0],
                COMMAND_LENGTH );		// terminal voice  information 4 :
        memset( pstVerInfoForDown->achVoice4InfoVer,
        		0x99,
                sizeof( pstVerInfoForDown->achVoice4InfoVer ) );
        memset( &pstVerInfoForDown->dwVoice4FileSize, 0x00, SIZE_LENGTH );
        memset( &pstVerInfoForDown->dwVoice4PktSize, 0x00, SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achVoice5Cmd,
        		achDCSCommFile[15][0],
                COMMAND_LENGTH );       // terminal voice  information 5 :
        memset( pstVerInfoForDown->achVoice5InfoVer,
        		0x99,
                sizeof( pstVerInfoForDown->achVoice5InfoVer ) );
        memset( &pstVerInfoForDown->dwVoice5FileSize, 0x00, SIZE_LENGTH );
        memset( &pstVerInfoForDown->dwVoice5PktSize, 0x00, SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achVoice6Cmd,
        		achDCSCommFile[16][0],
                COMMAND_LENGTH );		// terminal voice  information 6 :
        memset( pstVerInfoForDown->achVoice6InfoVer,
        		0x99,
                sizeof( pstVerInfoForDown->achVoice6InfoVer ) );
        memset( &pstVerInfoForDown->dwVoice6FileSize, 0x00, SIZE_LENGTH );
        memset( &pstVerInfoForDown->dwVoice6PktSize, 0x00, SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achVoice7Cmd,
        		achDCSCommFile[17][0],
                COMMAND_LENGTH );       // terminal voice  information 7 :
        memset( pstVerInfoForDown->achVoice7InfoVer,
        		0x99,
                sizeof( pstVerInfoForDown->achVoice7InfoVer ) );
        memset( &pstVerInfoForDown->dwVoice7FileSize, 0x00, SIZE_LENGTH );
        memset( &pstVerInfoForDown->dwVoice7PktSize, 0x00, SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achVoice8Cmd,
        		achDCSCommFile[18][0],
                COMMAND_LENGTH );       // terminal voice  information 8 :
        memset( pstVerInfoForDown->achVoice8InfoVer,
        		0x99,
                sizeof( pstVerInfoForDown->achVoice8InfoVer ) );
        memset( &pstVerInfoForDown->dwVoice8FileSize, 0x00, SIZE_LENGTH );
        memset( &pstVerInfoForDown->dwVoice8PktSize, 0x00, SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achVoice9Cmd,
        		achDCSCommFile[19][0],
                COMMAND_LENGTH );       // terminal voice  information 9 :
        memset( pstVerInfoForDown->achVoice9InfoVer,
        		0x99,
                sizeof( pstVerInfoForDown->achVoice9InfoVer ) );
        memset( &pstVerInfoForDown->dwVoice9FileSize, 0x00, SIZE_LENGTH );
        memset( &pstVerInfoForDown->dwVoice9PktSize, 0x00, SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achVehicleParmCmd,
        		achDCSCommFile[20][0],
                COMMAND_LENGTH );		// running vehicle parameter
        ASC2BCD( stVerInfo.abNextVehicleParmVer,
                 pstVerInfoForDown->achVehicleParmVer,
                 VER_INFO_LENGTH );
        memcpy( &pstVerInfoForDown->dwVehicleParmFileSize,
                (char*)&stVerInfo.dwNextVehicleParmPkt,
                SIZE_LENGTH );
        memcpy( &pstVerInfoForDown->dwVehicleParmPktSize,
                (char*)&stVerInfo.dwNextVehicleParmPkt,
                SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achRouteParmCmd,
        		achDCSCommFile[21][0],
                COMMAND_LENGTH );		// bus terminal route  parameter
        ASC2BCD( stVerInfo.abNextRouteParmVer,
                 pstVerInfoForDown->achRouteParmVer,
                 VER_INFO_LENGTH );
        memcpy( &pstVerInfoForDown->dwRouteParmFileSize,
                (char*)&stVerInfo.dwNextRouteParmPkt,
                SIZE_LENGTH );
        memcpy( &pstVerInfoForDown->dwRouteParmPktSize,
                (char*)&stVerInfo.dwNextRouteParmPkt,
                SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achBasicFareInfoCmd,
        		achDCSCommFile[22][0],
                COMMAND_LENGTH );		// new fare  information
        ASC2BCD( stVerInfo.abNextBasicFareInfoVer,
                 pstVerInfoForDown->achBasicFareInfoVer,
                 VER_INFO_LENGTH );
        memcpy( &pstVerInfoForDown->dwBasicFareInfoFileSize,
                (char*)&stVerInfo.dwNextBasicFareInfoPkt,
                SIZE_LENGTH );
        memcpy( &pstVerInfoForDown->dwBasicFareInfoPktSize,
                (char*)&stVerInfo.dwNextBasicFareInfoPkt,
                SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achOldFareCmd,
        		achDCSCommFile[23][0],
                COMMAND_LENGTH );		// old single fare  information
        ASC2BCD( stVerInfo.abNextOldFareVer,
                 pstVerInfoForDown->achOldFareVer,
                 VER_INFO_LENGTH );
        memcpy( &pstVerInfoForDown->dwOldFareFileSize,
                (char*)&stVerInfo.dwNextOldFarePkt,
                SIZE_LENGTH );
        memcpy( &pstVerInfoForDown->dwOldFarePktSize,
                (char*)&stVerInfo.dwNextOldFarePkt,
                SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achOldAirFareCmd,
        		achDCSCommFile[24][0],
                COMMAND_LENGTH );		// old airport fare  information
        ASC2BCD( stVerInfo.abNextOldAirFareVer,
                 pstVerInfoForDown->achOldAirFareVer,
                 VER_INFO_LENGTH );
        memcpy( &pstVerInfoForDown->dwOldAirFareFileSize,
                (char*)&stVerInfo.abNextOldAirFareVer,
                SIZE_LENGTH );
        memcpy( &pstVerInfoForDown->dwOldAirFarePktSize,
                (char*)&stVerInfo.abNextOldAirFareVer,
                SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achOldRangeFareCmd,
        		achDCSCommFile[25][0],
                COMMAND_LENGTH );		// old old interval fare  information
        ASC2BCD( stVerInfo.abNextOldRangeFareVer,
                 pstVerInfoForDown->achOldRangeFareVer,
                 VER_INFO_LENGTH );
        memcpy( &pstVerInfoForDown->dwOldRangeFareFileSize,
                (char*)&stVerInfo.dwNextOldRangeFarePkt,
                SIZE_LENGTH );
        memcpy( &pstVerInfoForDown->dwOldRangeFarePktSize,
                (char*)&stVerInfo.dwNextOldRangeFarePkt,
                SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achBusStationInfCmd,
        		achDCSCommFile[26][0],
                COMMAND_LENGTH );		// station  information
        ASC2BCD( stVerInfo.abNextBusStationInfoVer,
                 pstVerInfoForDown->achBusStationInfoVer,
                 VER_INFO_LENGTH );
        memcpy( &pstVerInfoForDown->dwBusStationInfFileSize,
                (char*)&stVerInfo.dwNextBusStationInfoPkt,
                SIZE_LENGTH );
        memcpy( &pstVerInfoForDown->dwBusStationInfoPktSize,
                (char*)&stVerInfo.dwNextBusStationInfoPkt,
                SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achPrepayIssuerInfoCmd,
        		achDCSCommFile[27][0],
                COMMAND_LENGTH );		// prepay issuer  information
        ASC2BCD( stVerInfo.abNextPrepayIssuerInfoVer,
                pstVerInfoForDown->achPrepayIssuerInfoVer,
                 VER_INFO_LENGTH );
        memcpy( &pstVerInfoForDown->dwPrepayIssuerInfoFileSize,
                (char*)&stVerInfo.dwNextPrepayIssuerInfoPkt,
                SIZE_LENGTH );
        memcpy( &pstVerInfoForDown->dwPrepayIssuerInfoPktSize,
                (char*)&stVerInfo.dwNextPrepayIssuerInfoPkt,
                SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achPostpayIssuerInfoCmd,
                achDCSCommFile[28][0],
                COMMAND_LENGTH );		// postpay issuer  information
        ASC2BCD( stVerInfo.abNextPostpayIssuerInfoVer,
                 pstVerInfoForDown->achPostpayIssuerInfoVer,
                 VER_INFO_LENGTH );
        memcpy( &pstVerInfoForDown->dwPostpayIssuerInfoFileSize,
                (char*)&stVerInfo.dwNextPostpayIssuerInfoPkt,
                SIZE_LENGTH );
        memcpy( &pstVerInfoForDown->dwPostpayIssuerInfoPktSize,
                (char*)&stVerInfo.dwNextPostpayIssuerInfoPkt,
                SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achDisExtraInfoCmd,
                achDCSCommFile[29][0],
                COMMAND_LENGTH );		// discount extra  information
        ASC2BCD( stVerInfo.abNextDisExtraInfoVer,
                 pstVerInfoForDown->achDisExtraInfoVer,
                 VER_INFO_LENGTH );
        memcpy( &pstVerInfoForDown->dwDisExtraInfoFileSize,
                (char*)&stVerInfo.dwNextDisExtraInfoPkt,
                SIZE_LENGTH );
        memcpy( &pstVerInfoForDown->dwDisExtraInfoPktSize,
                (char*)&stVerInfo.dwNextDisExtraInfoPkt,
                SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achHolidayInfoCmd,
                achDCSCommFile[30][0],
                COMMAND_LENGTH ); 		// holiday  information
        ASC2BCD( stVerInfo.abNextHolidayInfoVer,
                 pstVerInfoForDown->achHolidayInfoVer,
                 VER_INFO_LENGTH );
        memcpy( &pstVerInfoForDown->dwHolidayInfoFileSize,
                (char*)&stVerInfo.dwNextHolidayInfoPkt,
                SIZE_LENGTH );
        memcpy( &pstVerInfoForDown->dwHolidayInfoPktSize,
                (char*)&stVerInfo.dwNextHolidayInfoPkt,
                SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achXferApplyRegulationCmd,
                achDCSCommFile[31][0],
                COMMAND_LENGTH );       // transfer condition  information
        ASC2BCD( stVerInfo.abNextXferApplyRegulationVer,
                 pstVerInfoForDown->achXferApplyRegulationVer,
                 VER_INFO_LENGTH );
        memcpy( &pstVerInfoForDown->dwXferApplyRegulationFileSize,
                (char*)&stVerInfo.dwNextXferApplyRegulationPkt,
                SIZE_LENGTH );
        memcpy( &pstVerInfoForDown->dwXferApplyRegulationPktSize,
                (char*)&stVerInfo.dwNextXferApplyRegulationPkt,
                SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achEpurseIssuerRegistInfoCmd,
                achDCSCommFile[32][0],
                COMMAND_LENGTH );		// Issuer registration count
        ASC2BCD( stVerInfo.abNextEpurseIssuerRegistInfoVer,
                 pstVerInfoForDown->achEpurseIssuerRegistInfoVer,
                 VER_INFO_LENGTH );
        memcpy( &pstVerInfoForDown->dwEpurseIssuerRegistInfoFileSize,
                (char*)&stVerInfo.dwNextEpurseIssuerRegistInfoPkt,
                SIZE_LENGTH );
        memcpy( &pstVerInfoForDown->dwEpurseIssuerRegistInfoPktSize,
                (char*)&stVerInfo.dwNextEpurseIssuerRegistInfoPkt,
                SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achPSAMKeysetCmd,
        		achDCSCommFile[33][0],
                COMMAND_LENGTH );       // payment SAM Key Set  information
        ASC2BCD( stVerInfo.abNextPSAMKeysetVer,
                 pstVerInfoForDown->achPSAMKeysetVer,
                 VER_INFO_LENGTH );
        memcpy( &pstVerInfoForDown->dwPSAMKeysetFileSize,
                (char*)&stVerInfo.dwNextPSAMKeysetPkt,
                SIZE_LENGTH );
        memcpy( &pstVerInfoForDown->dwPSAMKeysetPktSize,
                (char*)&stVerInfo.dwNextPSAMKeysetPkt,
                SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achAutoChargeKeyseCmd,
        		achDCSCommFile[34][0],
                COMMAND_LENGTH );       // auto-charge SAM Key Set  information
        ASC2BCD( stVerInfo.abNextAutoChargeKeysetVer,
                 pstVerInfoForDown->achAutoChargeKeysetVer,
                 VER_INFO_LENGTH );
        memcpy( &pstVerInfoForDown->dwAutoChargeKeyseFileSize,
                (char*)&stVerInfo.dwNextAutoChargeKeysetPkt,
                SIZE_LENGTH );
        memcpy( &pstVerInfoForDown->dwAutoChargeKeysetPktSize,
                (char*)&stVerInfo.dwNextAutoChargeKeysetPkt,
                SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achAutoChargeParmCmd,
        		achDCSCommFile[35][0],
                COMMAND_LENGTH );       // auto-charge parameter   information
        ASC2BCD( stVerInfo.abNextAutoChargeParmVer,
                 pstVerInfoForDown->achAutoChargeParmVer,
                 VER_INFO_LENGTH );
        memcpy( &pstVerInfoForDown->dwAutoChargeParmFileSize,
                (char*)&stVerInfo.dwNextAutoChargeParmPkt,
                SIZE_LENGTH );
        memcpy( &pstVerInfoForDown->dwAutoChargeParmPktSize,
                (char*)&stVerInfo.dwNextAutoChargeParmPkt,
                SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achXferApplyInfoCmd,
        		achDCSCommFile[36][0],
                COMMAND_LENGTH );       // transfer apply
        ASC2BCD( stVerInfo.abNextXferApplyInfoVer,
                 pstVerInfoForDown->achXferApplyInfoVer,
                 VER_INFO_LENGTH );
        memcpy( &pstVerInfoForDown->dwXferApplyInfoFileSize,
                (char*)&stVerInfo.dwNextXferApplyInfoPkt,
                SIZE_LENGTH );
        memcpy( &pstVerInfoForDown->dwXferApplyInfoPktSize,
                (char*)&stVerInfo.dwNextXferApplyInfoPkt,
                SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achIssuerValidPeriodInfoCmd,
                achDCSCommFile[37][0],
                COMMAND_LENGTH );		// issuer valid period check
        ASC2BCD( stVerInfo.abNextIssuerValidPeriodInfoVer,
                 pstVerInfoForDown->achIssuerValidPeriodInfoVer,
                 VER_INFO_LENGTH );
        memcpy( &pstVerInfoForDown->dwIssuerValidPeriodInfoFileSize,
                (char*)&stVerInfo.dwNextIssuerValidPeriodInfoPkt,
                SIZE_LENGTH );
        memcpy( &pstVerInfoForDown->dwIssuerValidPeriodInfoPktSize,
                (char*)&stVerInfo.dwNextIssuerValidPeriodInfoPkt,
                SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achMasterBLCmd,
        		achDCSCommFile[38][0],
                COMMAND_LENGTH );		// master BL information
        ASC2BCD( stVerInfo.abMasterBLVer, pstVerInfoForDown->achMasterBLVer,
                 VER_INFO_LENGTH );
        chTmpFileNo = 38;
        GetRelayRecvFileSize( chTmpFileNo, &lFileSize );
        memcpy( &pstVerInfoForDown->dwMasterBLFileSize,
        		(char*)&lFileSize,
                SIZE_LENGTH );
        memcpy( &pstVerInfoForDown->dwMasterBLPktSize,
                (char*)&stVerInfo.dwMasterBLPkt,
                SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achUpdateBLCmd,
        		achDCSCommFile[39][0],
                COMMAND_LENGTH );		// update BL information
        ASC2BCD( stVerInfo.abUpdateBLVer,
        		 pstVerInfoForDown->achUpdateBLVer,
                 VER_INFO_LENGTH );
        memcpy( &pstVerInfoForDown->dwUpdateBLFileSize,
                (char*)&stVerInfo.dwUpdateBLPkt,
                SIZE_LENGTH );
        memcpy( &pstVerInfoForDown->dwUpdateBLPktSize,
                (char*)&stVerInfo.dwUpdateBLPkt,
                SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achHotBLCmd,
        		achDCSCommFile[40][0],
                COMMAND_LENGTH );		// HOT BL  information
        ASC2BCD( stVerInfo.abHotBLVer,
        		 pstVerInfoForDown->achHotBLVer,
                 VER_INFO_LENGTH );
        memcpy( &pstVerInfoForDown->dwHotBLFileSize,
                (char*)&stVerInfo.dwHotBLPkt,
                SIZE_LENGTH );
        memcpy( &pstVerInfoForDown->dwHotBLPktSize,
        		(char*)&stVerInfo.dwHotBLPkt,
                SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achMasterPrepayPLCmd,
        		achDCSCommFile[41][0],
                COMMAND_LENGTH );		// master PL information - prepay
        ASC2BCD( stVerInfo.abMasterPrepayPLVer,
                 pstVerInfoForDown->achMasterPrepayPLVer,
                 VER_INFO_LENGTH );
        chTmpFileNo = 41;
        GetRelayRecvFileSize( chTmpFileNo, &lFileSize );
        memcpy( &pstVerInfoForDown->dwMasterPrepayPLFileSize,
                (char*)&lFileSize, SIZE_LENGTH );
        memcpy( &pstVerInfoForDown->dwMasterPrepayPLPktSize,
                (char*)&stVerInfo.dwMasterPrepayPLPkt,
                SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achMasterPostpayPLCmd,
        		achDCSCommFile[42][0],
                COMMAND_LENGTH );		// master PL information - postpay
        ASC2BCD( stVerInfo.abMasterPostpayPLVer,
                 pstVerInfoForDown->achMasterPostpayPLVer,
                 VER_INFO_LENGTH );
        chTmpFileNo = 42;
        GetRelayRecvFileSize( chTmpFileNo, &lFileSize );
        memcpy( &pstVerInfoForDown->dwMasterPostpayPLFileSize,
                (char*)&lFileSize, SIZE_LENGTH );
        memcpy( &pstVerInfoForDown->dwMasterPostpayPLPktSize,
                (char*)&stVerInfo.dwMasterPostpayPLPkt,
                SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achUpdatePLCmd,
        		achDCSCommFile[43][0],
                COMMAND_LENGTH );		// update PL information
        ASC2BCD( stVerInfo.abUpdatePLVer,
        		 pstVerInfoForDown->achUpdatePLVer,
                 VER_INFO_LENGTH );
        memcpy( &pstVerInfoForDown->dwUpdatePLFileSize,
                (char*)&stVerInfo.dwUpdatePLPkt,
                SIZE_LENGTH );
        memcpy( &pstVerInfoForDown->dwUpdatePLPktSize,
                (char*)&stVerInfo.dwUpdatePLPkt,
                SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achHotPLCmd,
        		achDCSCommFile[44][0],
                COMMAND_LENGTH );		// HOT PL information
        ASC2BCD( stVerInfo.abHotPLVer,
        		 pstVerInfoForDown->achHotPLVer,
                 VER_INFO_LENGTH );
        memcpy( &pstVerInfoForDown->dwHotPLFileSize,
                (char*)&stVerInfo.dwHotPLVerPkt,
                SIZE_LENGTH );
        memcpy( &pstVerInfoForDown->dwHotPLPktSize,
                (char*)&stVerInfo.dwHotPLVerPkt,
                SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achMasterAICmd,
        		achDCSCommFile[45][0],
                COMMAND_LENGTH );		// master AI  information 
        ASC2BCD( stVerInfo.abMasterAIVer,
        		 pstVerInfoForDown->achMasterAIVer,
                 VER_INFO_LENGTH );
        chTmpFileNo = 45;
        GetRelayRecvFileSize( chTmpFileNo, &lFileSize );
        memcpy( &pstVerInfoForDown->dwMasterAIFileSize,
                (char*)&lFileSize, SIZE_LENGTH );
        memcpy( &pstVerInfoForDown->dwMasterAIPktSize,
                (char*)&stVerInfo.dwMasterAIPkt,
                SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achUpdateAICmd,
        		achDCSCommFile[46][0],
                COMMAND_LENGTH );		// update AI information
        ASC2BCD( stVerInfo.abUpdateAIVer,
        		 pstVerInfoForDown->achUpdateAIVer,
                 VER_INFO_LENGTH );
        memcpy( &pstVerInfoForDown->dwUpdateAIFileSize,
                (char*)&stVerInfo.dwUpdateAIPkt,
                SIZE_LENGTH );
        memcpy( &pstVerInfoForDown->dwUpdateAIPktSize,
                (char*)&stVerInfo.dwUpdateAIPkt,
                SIZE_LENGTH );

        memcpy( pstVerInfoForDown->achHotAICmd,
        		achDCSCommFile[47][0],
                COMMAND_LENGTH );		// HOT AI information
        ASC2BCD( stVerInfo.abHotAIVer,
        		 pstVerInfoForDown->achHotAIVer,
                 VER_INFO_LENGTH );
        memcpy( &pstVerInfoForDown->dwHotAIFileSize,
                (char*)&stVerInfo.dwHotAIPkt,
                SIZE_LENGTH );
        memcpy( &pstVerInfoForDown->dwHotAIPktSize,
        		(char*)&stVerInfo.dwHotAIPkt,
                SIZE_LENGTH );

    }

    return sizeof( VER_INFO_FOR_DOWN );
}

