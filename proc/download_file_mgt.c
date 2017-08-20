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
*  PROGRAM ID :       download_file_mgt.c                                     *
*                                                                              *
*  DESCRIPTION:       이 프로그램은 집계에서 다운로드 받은 파일의 목록을 관리하는 *
*						downloadinfo.dat의 CRD와 이어받기 정보를 관리하는 	   *
*						downfilelink.dat의 CU 기능, BLPL의 다운로드 여부와	   *
*						SAM 정보의 다운로드 여부를 체크하는 기능을 수행한다.	   *
*                                                                              *
*  ENTRY POINT:     short WriteDownFileList( DOWN_FILE_INFO* pstDownFileInfo );*
*					short WriteDownFileInfo( char* pachFileName,  			   *
*											 byte* pabFileVer, 				   *
*											 char chDownStatus, 			   *
*											 int nRecvdFileSize );			   *
*					short ReadDownFileList( char* pchFileName,				   *
*											DOWN_FILE_INFO* stDownFileInfo );  *
*					short DelDownFileList( char* pchFileName,				   *
*										   DOWN_FILE_INFO* stDownFileInfo );   *
*					short WriteRelayRecvFile( FILE_RELAY_RECV_INFO* 		   *
*													pstFileRelayRecvInfo );    *
*					short UpdateRelayRecvFile(  FILE* fdRelayRecv,			   *
*                            	FILE_RELAY_RECV_INFO* 	pstFileRelayRecvInfo );*
*					void WriteBLPLDownInfo( void );							   *
*				    static short CheckSAMInfoDown( void );                     *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: None                                                     *
*                                                                              *
*  INPUT FILES:       downloadinfo.dat  - 집계에서 다운로드 받은 파일의 목록     *
*                     tmp_c_ch_bl.dat   - 다운받은 변동 BL파일명( 업데이트 이전 )*
*                     tmp_c_ch_pl.dat   - 다운받은 변동 PL파일명( 업데이트 이전 )*
*                     tmp_c_ch_ai.dat   - 다운받은 변동 AI파일명( 업데이트 이전 )*
*                                                                              *
*  OUTPUT FILES:      downloadinfo.dat  - 집계에서 다운로드받은 파일의 목록      *
*                     downfilelink.dat  - 이어받기 정보를 관리                  *
*                     v_bl_be.dat       - 변동 BL의 다운받기 이전의 버전정보     *
*                     v_pl_be.dat       - 변동 PL의 다운받기 이전의 버전정보     *
*                     v_ai_be.dat       - 변동 AI의 다운받기 이전의 버전정보     *
*                     bl_pl_ctrl.flg    - BLPL을 다운여부를 세팅해주는 파일      *
*										  blpl_proc.c에서 참고함				   *
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
#include "download_file_mgt.h"
#include "main_process_busTerm.h"
#include "version_mgt.h"
#include "version_file_mgt.h"
#include "main.h"

/*******************************************************************************
*  Declaration of variables                                                    *
*******************************************************************************/
#define	FILE_NAME_LENGTH		50

#define     BLPL_DOWN       0x01        // BLPL download
#define     CMD_REQ         '1'         // 공유메모리에 요청시
#define     CMD_FAIL        '9'         // 공유메모리에 요청시 처리 실패
#define     CMD_COMP        '0'         // 공유메모리에 요청시 처리 완료







/*
 *	미래적용 파일을 현재적용해야하는지 여부
 *	FALSE- cannot apply, TRUE - can apply

bool boolIsApplyNextVer 			= 0;
bool boolIsApplyNextVerParm 		= 0;	// 파라미터 파일을 재로딩 요청
bool boolIsApplyNextVerVoice 		= 0;	// 음성파일을 ROM에 write
bool boolIsApplyNextVerAppl 		= 0;	// 프로그램 재부팅
bool boolIsApplyNextVerDriverAppl 	= 0;	// 운전자 조작기에 프로그램 upload */

/*******************************************************************************
*  Declaration of Module Functions                                             *
*******************************************************************************/
static short WriteBLPLDownFlag( char* pachMergeChk );

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      WriteDownFileList                                        *
*                                                                              *
*  DESCRIPTION :      downloadinfo.dat에 다운로드 받은 파일 정보를 write         *
*                                                                              *
*  INPUT PARAMETERS:  DOWN_FILE_INFO* pstDownFileInfo                          *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       ERR_FILE_OPEN | GetFileNo( DOWN_FILE )                 *
*                       ERR_FILE_WRITE | GetFileNo( DOWN_FILE )                *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short WriteDownFileList( DOWN_FILE_INFO* pstDownFileInfo )
{
    short           sReturnVal          = SUCCESS;
    int             fdDownFile;

    fdDownFile = open( DOWN_FILE, O_WRONLY | O_CREAT | O_APPEND, OPENMODE );

    if ( fdDownFile < 0 )
    {
        return ErrRet( ERR_FILE_OPEN | GetFileNo( DOWN_FILE ) );
    }

    if ( write( fdDownFile, (void*)pstDownFileInfo, sizeof( DOWN_FILE_INFO ) )
         != sizeof( DOWN_FILE_INFO ) )
    {
        sReturnVal = ERR_FILE_WRITE | GetFileNo( DOWN_FILE );
    }

    close( fdDownFile );

    return sReturnVal;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      WriteDownFileInfo                                        *
*                                                                              *
*  DESCRIPTION :      파라미터 정보를 다운로드목록 구조체에 세팅하여 			   *
*						downloadinfo.dat에 다운로드 받은 파일 정보를 write      *
*                                                                              *
*  INPUT PARAMETERS:  char* pachFileName,  									   *
*					  byte* pabFileVer, 									   *
*					  char chDownStatus, 									   *
*					  int nRecvdFileSize                          			   *
*                                                                              *
*  RETURN/EXIT VALUE:   sReturnVal                                             *
*           			ErrRet( ERR_FILE_OPEN | GetFileNo( DOWN_FILE )         *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short WriteDownFileInfo( char* pachFileName,  
						 byte* pabFileVer, 
						 char chDownStatus, 
						 int nRecvdFileSize )
{
    int 		fdFile;
    short 		sReturnVal 	= SUCCESS;
    DOWN_FILE_INFO  	stDownFileInfo;     	// 다운로드파일 정보 구조체

    memset( &stDownFileInfo, 0, sizeof( DOWN_FILE_INFO ) );
	
    strcpy( stDownFileInfo.achFileName, pachFileName );
    memcpy( stDownFileInfo.abFileVer,
          	pabFileVer,
          	sizeof( stDownFileInfo.abFileVer ) );
    stDownFileInfo.chDownStatus = chDownStatus;
    DWORD2LITTLE( nRecvdFileSize, stDownFileInfo.abDownSize );
    
    fdFile = open( DOWN_FILE, O_WRONLY | O_CREAT | O_APPEND, OPENMODE );
	
    if ( fdFile < 0 )
    {
        return ErrRet( ERR_FILE_OPEN | GetFileNo( DOWN_FILE ) );
    }

    if ( ( write( fdFile, &stDownFileInfo, sizeof( DOWN_FILE_INFO ) )
                ) != sizeof( DOWN_FILE_INFO ) )
    {
        sReturnVal = ERR_FILE_WRITE | GetFileNo( DOWN_FILE );
    }

    close( fdFile );

    return sReturnVal;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      ReadDownFileList                                         *
*                                                                              *
*  DESCRIPTION :      downloadinfo.dat에서  pchFileName에 대한 정보를 read      *
*                                                                              *
*  INPUT PARAMETERS:  char* pchFileName, - 읽을 파일명                          *
*                     DOWN_FILE_INFO* stDownFileInfo - 읽어들인 정보            *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS  - 해당파일을 찾은 경우                         *
*                       ERR_FILE_OPEN | GetFileNo( DOWN_FILE )                 *
*                       ERR_FILE_DATA_NOT_FOUND | GetFileNo( DOWN_FILE )       *
*                       ERR_FILE_READ | GetFileNo( DOWN_FILE )                 *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short ReadDownFileList( char* pchFileName, DOWN_FILE_INFO* pstDownFileInfo )
{
    short           sReturnVal          = FALSE;
    int             fdDownFile;
    DOWN_FILE_INFO  stReadDownFileInfo;
    int             nReadByte           = 0;

    fdDownFile = open( DOWN_FILE, O_RDONLY, OPENMODE );

    if ( fdDownFile < 0 )
    {
        return ErrRet( ERR_FILE_OPEN | GetFileNo( DOWN_FILE ) );
    }

    while( TRUE )
    {
        nReadByte = read( fdDownFile,
                          &stReadDownFileInfo,
                          sizeof( DOWN_FILE_INFO ) );

        if ( nReadByte == 0 )
        {
            sReturnVal = ERR_FILE_DATA_NOT_FOUND | GetFileNo( DOWN_FILE );
            break;
        }

        if ( nReadByte < 0 )
        {
            sReturnVal = ERR_FILE_READ | GetFileNo( DOWN_FILE );
            break;
        }

        if ( nReadByte > 0 && nReadByte < sizeof( DOWN_FILE_INFO ) )
        {
            sReturnVal = ERR_FILE_READ | GetFileNo( DOWN_FILE );
            break;
        }
        
        /*
         *  해당 파일을 찾으면 내용을 copy하고 break함
         */
        if ( strcmp( pchFileName, stReadDownFileInfo.achFileName ) == 0 )
        {
            memcpy( pstDownFileInfo,
                    &stReadDownFileInfo,
                    sizeof( DOWN_FILE_INFO) );

            sReturnVal = SUCCESS;
            break;
        }

    }


    close( fdDownFile );

    return sReturnVal;

}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      DelDownFileList                                          *
*                                                                              *
*  DESCRIPTION :      downloadinfo.dat에서 pchFileName을 찾아 해당 레코드 삭제  *
*                                                                              *
*  INPUT PARAMETERS:  char* pchFileName, - 지울 파일명                          *
*                     DOWN_FILE_INFO* stDownFileInfo - 지운 데이터 구조체       *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       ERR_FILE_OPEN | GetFileNo( DOWN_FILE )                 *
*                       ERR_FILE_OPEN | GetFileNo( DOWN_FILE_BACKUP_FILE )     *
*                       ERR_FILE_DATA_NOT_FOUND | GetFileNo( DOWN_FILE )       *
*                       ERR_FILE_READ | GetFileNo( DOWN_FILE )                 *
*                       ERR_FILE_WRITE | GetFileNo( DOWN_FILE_BACKUP_FILE )    *
*                       ERR_FILE_REMOVE | GetFileNo( DOWN_FILE )               *
*                       ERR_FILE_RENAME | GetFileNo( DOWN_FILE_BACKUP_FILE )   *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short DelDownFileList( char* pchFileName, DOWN_FILE_INFO* pstDownFileInfo )
{
    short           sReturnVal          = SUCCESS;
    int             fdSrcFile;
    int             fdDestFile;

    int             nReadByte           = 0;
    DOWN_FILE_INFO  stReadDownFileInfo;
    
    /*
     *  downloadinfo.dat open
     */
    fdSrcFile = open( DOWN_FILE, O_RDONLY, OPENMODE );

    if ( fdSrcFile < 0 )
    {
        return ErrRet( ERR_FILE_OPEN | GetFileNo( DOWN_FILE ) );
    }

    /*
     *  downloadino.dat.back open
     */
    fdDestFile = open( DOWN_FILE_BACKUP_FILE, O_WRONLY | O_CREAT, OPENMODE );

    if ( fdDestFile < 0 )
    {
        close( fdSrcFile );
        return ErrRet( ERR_FILE_OPEN | GetFileNo( DOWN_FILE_BACKUP_FILE ) );
    }

    while( TRUE )
    {
        nReadByte = read( fdSrcFile,
                          &stReadDownFileInfo,
                          sizeof( DOWN_FILE_INFO ) );

        if ( nReadByte == 0 )
        {
            sReturnVal = ERR_FILE_DATA_NOT_FOUND | GetFileNo( DOWN_FILE );
            break;
        }

        if ( nReadByte < 0 )
        {
            sReturnVal = ERR_FILE_READ | GetFileNo( DOWN_FILE );
            break;
        }
        
        /*
         *  해당 파일을 찾으면 데이터를 copy한 후 continue
         */
        if ( strcmp( pchFileName, stReadDownFileInfo.achFileName ) == 0 )
        {
            memcpy( pstDownFileInfo,
                    &stReadDownFileInfo,
                    sizeof( DOWN_FILE_INFO ) );
            continue;
        }

        nReadByte = write( fdDestFile,
                           &stReadDownFileInfo,
                           sizeof( DOWN_FILE_INFO ) );

        if ( nReadByte < sizeof( DOWN_FILE_INFO ) )
        {
            sReturnVal = ERR_FILE_WRITE | GetFileNo( DOWN_FILE_BACKUP_FILE );
            break;
        }
    }

    close( fdSrcFile );
    close( fdDestFile );

    if ( ( remove( DOWN_FILE ) ) < 0 )
    {
        sReturnVal = ERR_FILE_REMOVE | GetFileNo( DOWN_FILE );
        return sReturnVal;
    }

    if ( ( rename( DOWN_FILE_BACKUP_FILE, DOWN_FILE ) ) < 0 )
    {
        sReturnVal = ERR_FILE_RENAME | GetFileNo( DOWN_FILE_BACKUP_FILE );
    }

    return sReturnVal;

}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      WriteRelayRecvFile                                       *
*                                                                              *
*  DESCRIPTION :      이어받기 정보(pstFileRelayRecvInfo)를 downfilelink.dat에  *
*					    write                  								   *
*                                                                              *
*  INPUT PARAMETERS:  FILE_RELAY_RECV_INFO* pstFileRelayRecvInfo               *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       ERR_FILE_OPEN | GetFileNo( RELAY_DOWN_INFO_FILE )      *
*                       ERR_FILE_WRITE | GetFileNo( RELAY_DOWN_INFO_FILE )     *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short WriteRelayRecvFile( FILE_RELAY_RECV_INFO* pstFileRelayRecvInfo )
{
    short           sReturnVal          = SUCCESS;
    int             fdRelayRecvFile;

    fdRelayRecvFile = open( RELAY_DOWN_INFO_FILE,
                            O_WRONLY | O_CREAT,
                            OPENMODE );

    if ( fdRelayRecvFile < 0 )
    {
        return ErrRet( ERR_FILE_OPEN | GetFileNo( RELAY_DOWN_INFO_FILE ) );
    }

    lseek ( fdRelayRecvFile, 0, SEEK_SET );

    if ( write( fdRelayRecvFile,
                (void*)pstFileRelayRecvInfo,
                sizeof( RELAY_DOWN_INFO_FILE ) )
         != sizeof( RELAY_DOWN_INFO_FILE ) )
    {
        sReturnVal = ERR_FILE_WRITE | GetFileNo( RELAY_DOWN_INFO_FILE );
    }

    close( fdRelayRecvFile );

    return sReturnVal;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      UpdateRelayRecvFile                                      *
*                                                                              *
*  DESCRIPTION :      이어받기 중에 이어받기 정보(pstFileRelayRecvInfo)를 update *
*                                                                              *
*  INPUT PARAMETERS:  FILE *fdRelayRecv,  - downfilelink.dat                   *
*                     FILE_RELAY_RECV_INFO* pstFileRelayRecvInfo               *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       ERR_FILE_WRITE | GetFileNo( RELAY_DOWN_INFO_FILE )     *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short UpdateRelayRecvFile(  FILE*					fdRelayRecv,
                            FILE_RELAY_RECV_INFO* 	pstFileRelayRecvInfo )
{
    short           sReturnVal          = SUCCESS;

    fseek( fdRelayRecv, 0L, SEEK_SET );

    if ( fwrite( pstFileRelayRecvInfo,
                 sizeof( FILE_RELAY_RECV_INFO ),
                 1,
                 fdRelayRecv
               ) != 1 )
    {
        sReturnVal = ERR_FILE_WRITE | GetFileNo( RELAY_DOWN_INFO_FILE );
    }

    return sReturnVal;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      CheckBLPLDown                                            *
*                                                                              *
*  DESCRIPTION :      BLPL파일을 다운로드 했으면 이전 버전을 파일로 남기고  	   *
*						bl_pl_ctrl.flg에 다운 여부 flag를 남긴다.               *
*						BL 이전 버전 - v_bl_be.dat							   *
*						PL 이전 버전 - v_pl_be.dat							   *
*						AI 이전 버전 - v_ai_be.dat							   *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE:   void                                                   *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
void WriteBLPLDownInfo( void )
{
    short	sReturnVal	= SUCCESS;
    /*
     *  achMergeChk [0] - BL
     *  achMergeChk [1] - PL
     *  achMergeChk [2] - AI
     */
    char    		achMergeChk[3]      = { 0, };
	DOWN_FILE_INFO	stReadDownFileInfo;
	
	/*
	 *	변동 BL 다운로드 여부 체크
	 */
    sReturnVal = ReadDownFileList( DOWN_ING_UPDATE_BL_FILE, 
    							   &stReadDownFileInfo );
    
    /*
     *	변동 BL을 다운로드 받았고 다운로드가 완료되었다면 이전버전을 write하고
     *	download flag를 write한다.
     */
    if ( sReturnVal == SUCCESS && 
    	 stReadDownFileInfo.chDownStatus == DOWN_COMPL )
    {

        achMergeChk[0] = BLPL_DOWN;

        /*
         *	다운로드 받은 버전의 이전버전 write
         */
        sReturnVal= WriteBLPLBefVer( UPDATE_BL_BEFORE_VER_FILE,
									 stVerInfo.abUpdateBLVer,
			                         sizeof( stVerInfo.abUpdateBLVer ) );

		if ( sReturnVal < SUCCESS )
		{
			DelDownFileList( DOWN_ING_UPDATE_BL_FILE, &stReadDownFileInfo );
			unlink ( DOWN_ING_UPDATE_BL_FILE );
		}
		
        /*
         *	BLPL 다운로드 flag를 write
         */
		sReturnVal = WriteBLPLDownFlag( achMergeChk );
		
		if ( sReturnVal < SUCCESS )
		{
			DelDownFileList( DOWN_ING_UPDATE_BL_FILE, &stReadDownFileInfo );
			unlink ( DOWN_ING_UPDATE_BL_FILE );
			unlink ( UPDATE_BL_BEFORE_VER_FILE );
		}

	}
	
	/*
	 *	변동 PL 다운로드 여부 체크
	 */
    sReturnVal = ReadDownFileList( DOWN_ING_UPDATE_PL_FILE, 
    							   &stReadDownFileInfo );
    
    /*
     *	변동 PL을 다운로드 받았고 다운로드가 완료되었다면 이전버전을 write하고
     *	download flag를 write한다.
     */
    if ( sReturnVal == SUCCESS && 
    	 stReadDownFileInfo.chDownStatus == DOWN_COMPL )
    {

        achMergeChk[1] = BLPL_DOWN;

        /*
         *	다운로드 받은 버전의 이전버전 write
         */
		sReturnVal= WriteBLPLBefVer( UPDATE_PL_BEFORE_VER_FILE,
									 stVerInfo.abUpdatePLVer,
			                         sizeof( stVerInfo.abUpdatePLVer ) );

		if ( sReturnVal < SUCCESS )
		{
			DelDownFileList( DOWN_ING_UPDATE_PL_FILE, &stReadDownFileInfo );
			unlink ( DOWN_ING_UPDATE_PL_FILE );
		}
		
        /*
         *	BLPL 다운로드 flag를 write
         */
		sReturnVal = WriteBLPLDownFlag( achMergeChk );
		
		if ( sReturnVal < SUCCESS )
		{
			DelDownFileList( DOWN_ING_UPDATE_PL_FILE, &stReadDownFileInfo );
			unlink ( DOWN_ING_UPDATE_PL_FILE );
			unlink ( UPDATE_PL_BEFORE_VER_FILE );
		}
	}
	
	/*
	 *	변동 AI 다운로드 여부 체크
	 */
    sReturnVal = ReadDownFileList( DOWN_ING_UPDATE_AI_FILE, 
    							   &stReadDownFileInfo );
    
    /*
     *	변동 AI을 다운로드 받았고 다운로드가 완료되었다면 이전버전을 write하고
     *	download flag를 write한다.
     */
    if ( sReturnVal == SUCCESS && 
    	 stReadDownFileInfo.chDownStatus == DOWN_COMPL )
    {

        achMergeChk[2] = BLPL_DOWN;
        
        /*
         *	다운로드 받은 버전의 이전버전 write
         */
		sReturnVal= WriteBLPLBefVer( UPDATE_AI_BEFORE_VER_FILE,
									 stVerInfo.abUpdateAIVer,
			                         sizeof( stVerInfo.abUpdateAIVer ) );

		if ( sReturnVal < SUCCESS )
		{
			DelDownFileList( DOWN_ING_UPDATE_AI_FILE, &stReadDownFileInfo );
			unlink ( DOWN_ING_UPDATE_AI_FILE );
		}
		
        /*
         *	BLPL 다운로드 flag를 write
         */
		sReturnVal = WriteBLPLDownFlag( achMergeChk );
		
		if ( sReturnVal < SUCCESS )
		{
			DelDownFileList( DOWN_ING_UPDATE_AI_FILE, &stReadDownFileInfo );
			unlink ( DOWN_ING_UPDATE_AI_FILE );
			unlink ( UPDATE_AI_BEFORE_VER_FILE );
		}
	}
	
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      WriteBLPLDownFlag                                        *
*                                                                              *
*  DESCRIPTION :      merge_flag.dat파일에 BL/PL/AI download 여부를 write       *
*					  BLPL의 경우 한 파일을 다운로드 받으면 update하게 되어있으  *
*					  므로 flag를 한 byte씩 write해야함( 기존에 다운받은 flag를  *
*					  덮어 쓸 우려 있으므로 )									   *
*                                                                              *
*  INPUT PARAMETERS:  	char* pachMergeChk  - BL/PL/AI download 여부           *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       ERR_FILE_OPEN | GetFileNo( MERGE_FLAG_FILE )           *
*                       ERR_FILE_WRITE | GetFileNo( MERGE_FLAG_FILE )          *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short WriteBLPLDownFlag( char* pachMergeChk )
{
	short   sReturnVal          = SUCCESS;
    int     fdMergeFlagFile     = 0;  
    
    /*
     *  Merge Flag File 생성
     */
    if ( pachMergeChk[0] == BLPL_DOWN ||
         pachMergeChk[1] == BLPL_DOWN ||
         pachMergeChk[2] == BLPL_DOWN )
    {
    	
        fdMergeFlagFile = open( MERGE_FLAG_FILE, O_RDWR | O_CREAT, OPENMODE );

        if ( fdMergeFlagFile < 0 )
        {
            return ErrRet( ERR_FILE_OPEN | GetFileNo( MERGE_FLAG_FILE ) );
        }

		/*
		 *	변동 BL 다운로드 받았을 경우 
		 */
        if ( pachMergeChk[0] == BLPL_DOWN )
        {
            lseek( fdMergeFlagFile, 0, SEEK_SET );
            
            sReturnVal = write( fdMergeFlagFile,
                                &pachMergeChk[0],
                                sizeof( pachMergeChk[0] ) );

            if ( sReturnVal < sizeof( pachMergeChk[0] ) )
            {
            	LogDCS( "[WriteBLPLDownFlag] 변동BL 머지 FLAG WRITE 실패\n" );
				printf( "[WriteBLPLDownFlag] 변동BL 머지 FLAG WRITE 실패\n" );
            	close( fdMergeFlagFile );
            	return ErrRet( ERR_FILE_WRITE | GetFileNo( MERGE_FLAG_FILE ) );
                
            }
        }

		/*
		 *	변동 PL 다운로드 받았을 경우 
		 */
        if ( pachMergeChk[1] == BLPL_DOWN )
        {
            lseek( fdMergeFlagFile, 1, SEEK_SET );
            sReturnVal = write( fdMergeFlagFile,
                                &pachMergeChk[1],
                                sizeof( pachMergeChk[1] ) );

            if ( sReturnVal < sizeof( pachMergeChk[1] ) )
            {
            	LogDCS( "[WriteBLPLDownFlag] 변동PL 머지 FLAG WRITE 실패\n" );
				printf( "[WriteBLPLDownFlag] 변동PL 머지 FLAG WRITE 실패\n" );
            	close( fdMergeFlagFile );
            	return ErrRet( ERR_FILE_WRITE | GetFileNo( MERGE_FLAG_FILE ) );
                
            }
        }

		/*
		 *	변동 AI 다운로드 받았을 경우 
		 */
        if ( pachMergeChk[2] == BLPL_DOWN )
        {
            lseek( fdMergeFlagFile, 2, SEEK_SET );
            sReturnVal = write( fdMergeFlagFile,
                                &pachMergeChk[2],
                                sizeof( pachMergeChk[2] ) );

            if ( sReturnVal < sizeof( pachMergeChk[2] ) )
            {
            	LogDCS( "[WriteBLPLDownFlag] 변동신선불PL 머지 FLAG WRITE 실패\n" );
				printf( "[WriteBLPLDownFlag] 변동신선불PL 머지 FLAG WRITE 실패\n" );
            	close( fdMergeFlagFile );
            	return ErrRet( ERR_FILE_WRITE | GetFileNo( MERGE_FLAG_FILE ) );
                
            }
        }

        close( fdMergeFlagFile );
    }
    
    return sReturnVal;
	
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      CheckSAMInfoDown                                         *
*                                                                              *
*  DESCRIPTION :      SAM정보를 다운로드 받았는지 여부 체크 		               *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       ERR_FILE_OPEN | GetFileNo( DOWN_FILE )                 *
*                       ERR_FILE_DATA_NOT_FOUND | GetFileNo( DOWN_FILE )       *
*                       ERR_FILE_READ | GetFileNo( DOWN_FILE )                 *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short CheckSAMInfoDown( void )
{
    short           sReturnVal          = SUCCESS;

    DOWN_FILE_INFO  stDownFileInfo;                 // download info
    // tmp_c_idcenter.dat 파일명
    char            achDownFileName[FILE_NAME_LENGTH] = { 0, };   

    byte            SharedMemoryCmd;
    char            achCmdData[6]       = { 0, };
    word            wCmdDataSize;
    int             i                   = 0;

    /*
     *  tmp_c_idcenter.dat 파일명 생성
     */
    sprintf( achDownFileName, "%s", TMP_EPURSE_ISSUER_REGIST_INFO_FILE );

    /*
     *  id_center와 keyset 두번 수행
     */
    for ( i = 0 ; i < 2 ; i++ )
    {
        /*
         *  download정보 read
         */
        sReturnVal = ReadDownFileList( achDownFileName, &stDownFileInfo );

        /*
         *  다운을 받았다면
         */
        if ( sReturnVal == SUCCESS )
        {
            achCmdData[0] = CMD_REQ;

            while( TRUE )
            {
                /*
                 *  keyset 등록 요청
                 */
                sReturnVal = SetSharedCmdnData( CMD_KEYSET_IDCENTER_WRITE,
                                                &achCmdData[0],
                                                sizeof( achCmdData[0] ) );

                if( sReturnVal == SUCCESS )
                {
                    break;
                }
            }

            while( TRUE )
            {
                /*
                 *  처리 결과 체크
                 */
                GetSharedCmd( &SharedMemoryCmd, achCmdData, &wCmdDataSize );

                /*
                 *  처리가 끝났을 경우
                 */
                if( ( SharedMemoryCmd == CMD_KEYSET_IDCENTER_WRITE )
                    && ( achCmdData[ 0 ] == CMD_FAIL
                         || achCmdData[ 0 ] == CMD_COMP ) )
                {
                    /*
                     *  요청 데이터 clear
                     */
                    ClearSharedCmdnData( CMD_KEYSET_IDCENTER_WRITE );

                    if( achCmdData[0] == CMD_FAIL )
                    {
                        /*
                         *  main term PSAM 등록 여부 세팅
                         */
                        boolIsRegistMainTermPSAM = FALSE;

                        /*
                         *  SAM version정보를 초기화시켜서 다음 집계통신시
                         *  새로 다운로드 받을 수 있도록 한다.
                         */
                        InitSAMRegistInfoVer( );
                    }
                    else if( achCmdData[0] == CMD_COMP )
                    {
                        boolIsRegistMainTermPSAM = TRUE;
                    }

                    break;
                }
                else if ( SharedMemoryCmd != CMD_KEYSET_IDCENTER_WRITE )
                {
                    break;
                }
                    usleep( 500000 );
            }

            break;

        }

        /*
         *  tmp_c_keyset.dat으로 파일명 세팅
         */
        sprintf( achDownFileName, "%s", TMP_PSAM_KEYSET_INFO_FILE );
    }

    return sReturnVal;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      WriteOperParmFile                                        *
*                                                                              *
*  DESCRIPTION :      집계시스템에서 다운받은 파라미터 파일을 write              *
*                                                                              *
*  INPUT PARAMETERS:    byte* pbBuff    - 다운로드 받은 메세지                  *
*                       long lBuffSize  - 메세지의 size                        *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       ERR_FILE_OPEN | GetFileNo( VEHICLE_PARM_FILE )         *
*                       ERR_FILE_WRITE | GetFileNo( VEHICLE_PARM_FILE )        *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short WriteOperParmFile( byte* pbBuff, long lBuffSize )
{
    int     fdOperParmFile;

    /*
     *  운영 파라미터 파일 오픈
     */

    fdOperParmFile = open( VEHICLE_PARM_FILE, O_WRONLY | O_CREAT, OPENMODE );

    if ( fdOperParmFile < 0 )
    {
        return ErrRet( ERR_FILE_OPEN | GetFileNo( VEHICLE_PARM_FILE ) );
    }

    /*
     *  운영파라미터 파일 write
     */
    if ( write( fdOperParmFile, pbBuff, lBuffSize ) != lBuffSize )
    {
        close ( fdOperParmFile );
        return ErrRet( ERR_FILE_WRITE | GetFileNo( VEHICLE_PARM_FILE ) );
    }

    close ( fdOperParmFile );

    return SUCCESS;
}
