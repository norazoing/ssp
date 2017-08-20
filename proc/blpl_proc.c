/*******************************************************************************
*																			   *
*					   KSCC	- Bus Embeded System							   *
*																			   *
*			 All rights	reserved. No part of this publication may be		   *
*			 reproduced, stored	in a retrieval system or transmitted		   *
*			 in	any	form or	by any means  -	 electronic, mechanical,		   *
*			 photocopying, recording, or otherwise,	without	the	prior		   *
*			 written permission	of LG CNS.									   *
*																			   *
********************************************************************************
*																			   *
*  PROGRAM ID :		  blpl_proc.c											   *
*																			   *
*  DESCRIPTION:		  카드별로 bl(blacklist),pl(positive list)을 판별하기 위해 *
*                     BL/PL 파일을 검색하여 그 결과를 알려주는 기능을 담당한다   *
*                     BL/PL 정보는 매일 변하게 마련이고 이에 따라              *
*					  새로운 bl/pl리스트가 추가될경우 BL의 경우는 Merge를      *
*                     PL의 경우는 Update를 하여 갱신된 정보를 유지한다.        *														   *
*																			   *
*  ENTRY POINT:		  머지하는 경우 Main에서 BLPLMerge 함수를 호출			   *
*					  검색하는 경우에는 카드나 통신프로세스에서 개별적으로     *
*                     SearchBLinBus 함수나 SearchPLinBus 함수를 호출           *
*													                           *
*  INPUT PARAMETERS:  void                                                     *
*                                                                              *
*																			   *
*  RETURN/EXIT VALUE: void													   *
*																			   *
*  INPUT FILES:	기존의 고정 BL/PL파일들과 변동 BL/PL 파일들 	               *
*																			   *
*            TEMP_MASTER_BL_FILE "temp.bl" : 머지를 위한 임시 BL 파일로 머지가 *    
*                                         성공적으로 끝나면 새로운 고정 BL이 됨*    
*            MASTER_BL_FILE			  "c_fi_bl.dat"  : Master BL File          *    
*            MASTER_PREPAY_PL_FILE	  "c_fa_pl.dat"  : 선불 PL File            *    
*            MASTER_POSTPAY_PL_FILE	  "c_fd_pl.dat"  : 후불 PL File            *    
* 			 MASTER_AI_FILE			  "c_fi_ai.dat"  : 신선불 PL File          *    
* 			 CHANGE_BL_FILE			  "c_ch_bl.dat"  : 변동 BL File            *    
* 			 UPDATE_PL_FILE			  "c_ch_pl.dat"  : 변동 선/후불 PL File    *    
* 			 UPDATE_AI_FILE			  "c_ch_ai.dat"  : 변동 신선불 PL File     *    
* 			 POSTPAY_ISSUER_INFO_FILE "c_dp_inf.dat" : 후불발행사 File         *	
*																			   *
*  OUTPUT FILES: 머지시 - 새로운 고정 BL/PL 파일들(파일이름은 동일)            *
*                                                                              *
*            MASTER_BL_FILE			  "c_fi_bl.dat"  : New Master BL File      *    
*            MASTER_PREPAY_PL_FILE	  "c_fa_pl.dat"  : New 선불 PL File        *    
*            MASTER_POSTPAY_PL_FILE	  "c_fd_pl.dat"  : New 후불 PL File        *    
* 			 MASTER_AI_FILE			  "c_fi_ai.dat"  : New 신선불 PL File      *  
*																			   *
*  SPECIAL LOGIC: 공통모듈의 SearchBL, SeachPL, MergeBL, UpdatePL의 내용 참조  *
*                                                                              *																		   *
********************************************************************************
*						  MODIFICATION LOG									   *
*																			   *
*	 DATE				 SE	NAME					  DESCRIPTION			   *
* ---------- ---------------------------- ------------------------------------ *
* 2005/09/29 Solution Team	Boohyeon Jeon Initial Release(PL Part)			   *
* 2005/09/29 Solution Team	Woolim		  Initial Release(BL Part)			   *
*																			   *
*******************************************************************************/
#include "blpl_proc.h"
#include "main.h"
#include "file_mgt.h"
#include "../system/bus_type.h"
#include "../system/bus_errno.h"
#include "version_mgt.h"
#include "main_process_busTerm.h"

/*******************************************************************************
*  Declaration of Function Prototype    									   *
*******************************************************************************/
short BLMergeMain( void );
short PLMergeMain( void );
short AIMergeMain( void );
short WriteBLPLFileCRC(	int TypeOfCRCFile );
static word GetBLTypePrefixInFile(byte *prefix);
static word GetBLTypePrefix(byte *SixAscPrefix);
/*******************************************************************************
*  Declaration of Defines in BL												   *
*******************************************************************************/
/*
 * 머지의 상태를 알려주는 파일로서 3byte로 구성
 * 1byte : [0] : B/L,	2byte [1] : 구선후불P/L, 신후불	P/L, 3byte[2]:신선불 P/L
 * 각 byte의 값이 1:머지 요청,	2:머지중, 0:머지종료 or 머지필요없음.	
 */
#define	MERGE_FLAG_FILE							"bl_pl_ctrl.flg"
/*
 * 머지 FLAG파일의 각 BYTE의 값의 의미를 정의
 */	 
#define	MERGE_COMPLETE_OR_NOT_NEED            ( 0 ) // 머지완료 OR 필요없음
#define MERGE_REQUEST                         ( 1 ) // 머지요청
#define MERGE_WORKING_NOW                     ( 2 ) // 머지실행중
/*
 * FILE ACCESS를 통해 존재여부판단시 사용할 매크로
 */
#define FILE_EXIST                            ( 0 )
/*
 * CRC값 기록시 기록할 파일 구분을 위한 매크로
 */
#define BL_FILE 							  ( 0 )
#define PL_FILE  						      ( 1 )
#define AI_FILE 							  ( 2 ) 
/*
 * CRC값 기록시 기록할 파일 구분을 위한 매크로
 */
#define PL_CARD_NO_SIZE                       ( 20 )

#ifndef	__GCC
/*******************************************************************************
*																			   *
*  FUNCTION	ID :	  BLPLMerge										           *
*																			   *
*  DESCRIPTION :	  BL/PL/AI의 머지실행여부를 체크하여 머지를 실행한다  	   *
*					  메인함수에서 호출하도록 되어 있는 이함수는 머지시에      *
*                     프로세스를 Fork하여 실행후 프로세스를 종료하게 되어있다  *
*																			   *
*  INPUT PARAMETERS:  None													   *
*																			   *
*  RETURN/EXIT VALUE: SUCCESS - 실행성공									   *
*			  BL_PREFIX_ERROR_EVENT	: BL Prefix 오류   			               *
*             BL_SERIAL_ERROR_EVENT : BL Prefix를 제외한 나머지 번호오류       *
*             BL_SIZE_ERROR_EVENT   : BL SIZE 오류                             *
*																			   *
*  Author  : Woolim															   *
*																			   *
*  DATE	   : 2005-09-03														   *
*																			   *
*  REMARKS : BL/PL/AI머지실행여부를 알기위해 bl_pl_ctrl.flg파일을 체크하여     *
*            각각의 머지함수를 실행하고 실행 후 해당 파일의 CRC값을            *
*            install.dat파일에 기록한다.                     				   *
*            머지 실패시 RollBackBLPLVer 함수를 호출하여 BLPL 파일의 버전을    *
*            이전으로 롤백하여 집계로부터 다시 변동파일을 수신할수 있도록      *
*            처리하게 된다.                                                    *
*                                                        					   *
*******************************************************************************/
short BLPLMerge(void)
{
	int fd, ret = 0;
	int	bl_ret = 0,	pl_ret = 0,	ai_ret = 0;
	pid_t pid;
	short sRetVal =	SUCCESS;

	/*
	 * 각 byte가 나타내는 의미는 아래와 같다.
     * 1byte : [0] : B/L,	2byte [1] : 구선후불P/L, 신후불	P/L, 3byte[2]:신선불 P/L
     * 각 byte의 값의 의미는
     *  MERGE_REQUEST:머지 요청
     *  MERGE_WORKING_NOW:머지중
     *  MERGE_COMPLETE_OR_NOT_NEED:머지종료 or 머지필요없음.	
	 */
	byte merge_flag[3];


	/*
     * 고정 BL/PL파일을 갱신할 변동파일 존재여부 확인
	 */
	bl_ret = access( CHANGE_BL_FILE,F_OK);  // 변동 BL
	pl_ret = access( UPDATE_PL_FILE,F_OK);  // 변동 PL(구선불,구후불)
	ai_ret = access( UPDATE_AI_FILE,F_OK);  // 변동 PL(AI라고도 함 : 신선/후불)

	if ( bl_ret	== 0 ||	pl_ret == 0	|| ai_ret == 0 )
	{
		printf( "[BLPLMerge] 머지할 파일이 존재함\n" );
	}
	else
	{
//		printf( "[BLPLMerge] 머지할 파일이 존재하지 않음\n" );
		return -1;
	}

	memset(merge_flag, 0, 3);

    /*
     * 머지상태를 나타내는 flag파일 Open
     */     
	fd = open( MERGE_FLAG_FILE, O_RDWR );
	if (fd < 0)	
	{
		printf( "[BLPLMerge] bl_pl_ctrl.flg 파일 OPEN 오류\n" );
		return -1;
	}
	
	/*
	 * 머지 flag파일 Read
	 */
	ret	= read(fd, merge_flag, 3);

	if (ret	== 2)
	{
		printf( "[BLPLMerge] bl_pl_ctrl.flg이 구버전이므로 변환함\n" );
		merge_flag[2] =	0;
		lseek(fd, 0, SEEK_SET);
		write(fd, merge_flag, 3);
	}
	close(fd);

    /*
     * BL/PL에 대한 머지요청여부 확인
     */
	if ( merge_flag[0] == MERGE_REQUEST || merge_flag[1] == MERGE_REQUEST ||
		 merge_flag[2] == MERGE_REQUEST )
	{
	   /*
		* 머지실행시 프로세스 fork실행
		*/
		pid	= fork();
		if (pid	< 0)
		{
			printf( "[BLPLMerge] fork() 실패\n" );
		}
		else
		{
			if (pid	== 0)
			{
				/*
				 * 공유메모리내 머지상태 변수를 머지실행중으로 설정
				 */
				gpstSharedInfo->boolIsBLPLMergeNow = TRUE;
				printf( "[BLPLMerge] gpstSharedInfo->boolIsBLPLMergeNow = TRUE 설정\n" );

				/*
				 * 각각 BL/PL/AI 요청에 따라 각각의 Merge Main함수 실행
				 */
				if ( merge_flag[0] == MERGE_REQUEST ) // BL 머지 요청
				{
					gpstSharedInfo->boolIsBLMergeNow = TRUE;
					sRetVal = BLMergeMain();		  			
					WriteBLPLFileCRC( BL_FILE );      // CRC값 기록
					
					if ( sRetVal != SUCCESS )
					{
						goto BLPLEXIT;
					}
				}

				if ( merge_flag[1] == MERGE_REQUEST ) // PL 머지 요청
				{
					gpstSharedInfo->boolIsMifPrepayPLMergeNow = TRUE;
					gpstSharedInfo->boolIsPostpayPLMergeNow = TRUE;
					sRetVal = PLMergeMain();
					WriteBLPLFileCRC( PL_FILE );      // CRC값 기록
					if ( sRetVal != SUCCESS )
					{
						goto BLPLEXIT;
					}					
				}

				if ( merge_flag[2] == MERGE_REQUEST ) // AI 머지 요청
				{
					gpstSharedInfo->boolIsSCPrepayPLMergeNow = TRUE;
					sRetVal = AIMergeMain();
					WriteBLPLFileCRC( AI_FILE );      // CRC값 기록
					
					if ( sRetVal != SUCCESS )
					{
						goto BLPLEXIT;
					}				
				}

BLPLEXIT:
				/*
				 * 공유메모리내 머지상태 변수를 머지완료로 설정
				 */				
				gpstSharedInfo->boolIsBLPLMergeNow = FALSE;

				gpstSharedInfo->boolIsBLMergeNow = FALSE;
				gpstSharedInfo->boolIsMifPrepayPLMergeNow = FALSE;
				gpstSharedInfo->boolIsPostpayPLMergeNow = FALSE;
				gpstSharedInfo->boolIsSCPrepayPLMergeNow = FALSE;
				
				exit(0);
			}
		}
	}
	return ErrRet( sRetVal );
}

#endif

/*******************************************************************************
*																			   *
*  FUNCTION	ID :	  BLMergeMain										       *
*																			   *
*  DESCRIPTION :	  BL의 머지를 실행하는 MergeBL함수를                 	   *
*                     호출하고 성공실패에 따른 파일처리 및 오류기록을 담당     *
*																			   *
*  INPUT PARAMETERS:  None													   *
*																			   *
*  RETURN/EXIT VALUE:                                                          *
*             SUCCESS - 실행성공시 기존의 고정 BL파일과 변동 파일을            *
*                               삭제하고 머지된 임시 고정파일을 새로운 고정BL  *
*                               파일로 rename한다.           				   *
*             실패시 - 머지중인 임시고정파일과 변동파일을 삭제하고 이전버전으로*
*                      파일버전을 롤백한다.                                    *
*                      또한 MergeBL함수에서 리턴한 오류코드를 event.trn파일에  *
*                      기록한다.                                               *
*																			   *
*  Author  : Woolim															   *
*																			   *
*  DATE	   : 2005-09-03														   *
*																			   *
*  REMARKS : MergeBL은 공통모듈 함수										   *
*                                                        					   *
*******************************************************************************/
short BLMergeMain( void )
{
	short retVal = SUCCESS;
	int	fd	= 0;
	byte merge_flag[3];
	
	memset( merge_flag, 0x00, sizeof( merge_flag ) );
	
	printf("BL Merge Start.\n");
	/*
	 * MergeBL 공통모듈 함수 호출
	 */
	if ((retVal = MergeBL( MASTER_BL_FILE, CHANGE_BL_FILE)) != SUCCESS )
	{
		printf("BL Merge Fail.\n");
		/*
		 * 실패시 파일삭제 및 버전롤백- RollbackBLPLVer(BL롤백, PL롤백, AI롤백)
		 */
		unlink(CHANGE_BL_FILE);               // 변동 BL파일 삭제
		unlink(TEMP_MASTER_BL_FILE);          // 임시 BL파일 삭제
		RollbackBLPLVer( TRUE, FALSE, FALSE); // BL파일 버전롤백
		/*
		 * 머지 FLAG파일에 머지 완료 설정
		 */		
		fd = open(MERGE_FLAG_FILE, O_RDWR);
		if ( fd	 > 0)
		{
			merge_flag[0] =	0;
			lseek(fd, 0, SEEK_SET);
			write(fd, merge_flag, 1);
			close(fd);
		}
		/*
		 * 에러코드 기록
		 */			
		if ( retVal	== ERR_BL_PREFIX_ERROR	)  // BL Prefix 에러
		{
			ctrl_event_info_write(BL_PREFIX_ERROR_EVENT);
			return ErrRet( retVal );
		}
		if ( retVal	== ERR_BL_SERIAL_ERROR	)  // Prefix를 제외한 나머지번호에러
		{
			ctrl_event_info_write(BL_SERIAL_ERROR_EVENT);
			return ErrRet( retVal );
		}
		if ( retVal	== ERR_BL_SIZE_ERROR )     // BL Size 에러
		{
			ctrl_event_info_write(BL_SIZE_ERROR_EVENT);  
			return ErrRet( retVal );
		}
		return ERR_BLPL_PROC_BL_MERGE_FAIL;
	}
	else
	{
		/*
		 * 머지 FLAG파일에 머지 완료 설정
		 */			
		fd = open(MERGE_FLAG_FILE, O_RDWR);
		merge_flag[0] =	0;
		lseek(fd, 0, SEEK_SET);
		write(fd, merge_flag, 1);
		close(fd);
		/*
		 * 성공시 파일삭제 및 임시고정파일 Rename
		 */			
		unlink(CHANGE_BL_FILE);                      // 변동 BL 파일삭제
		unlink(MASTER_BL_FILE);                      // 변동 BL 파일삭제
		rename(TEMP_MASTER_BL_FILE, MASTER_BL_FILE); // 새로운 고정파일로 개명
		unlink(UPDATE_BL_BEFORE_VER_FILE);           // 이전버전 BL버전파일 삭제
		printf("BL Merge Complete.\n");
		return SUCCESS;
	}
}
/*******************************************************************************
*																			   *
*  FUNCTION	ID :	  PLMergeMain										       *
*																			   *
*  DESCRIPTION :	  PL의 머지를 실행하는 UpdatePL함수를 PL을 파라미터로 넣어 *
*                     호출하고 성공실패에 따른 파일처리 및 오류기록을 담당     *
*																			   *
*  INPUT PARAMETERS:  None													   *
*																			   *
*  RETURN/EXIT VALUE:                                                          *
*             SUCCESS - 실행성공 - 변동 PL파일을 삭제한다.                     *
*             실패시 - 변동 PL 파일을 삭제하고 이전버전으로 파일버전을 롤백한다*
*																			   *
*  Author  : Woolim															   *
*																			   *
*  DATE	   : 2005-09-03														   *
*																			   *
*  REMARKS : UpdatePL은 공통모듈 함수										   *
*                                                        					   *
*******************************************************************************/
short PLMergeMain( void )
{
	short retVal = SUCCESS;
	int	fd	= 0;
	byte merge_flag[3];
	
	memset( merge_flag, 0x00, sizeof( merge_flag ) );	
	printf("PL Merge Start.\n");

	if ((retVal	= UpdatePL(PL))	< 0)
	{
		printf("PL Merge Fail.\n");
		unlink(UPDATE_PL_FILE);
		RollbackBLPLVer( FALSE,	TRUE, FALSE);
		fd = open(MERGE_FLAG_FILE, O_RDWR);
		if ( fd	 > 0)
		{
			merge_flag[1] =	0;
			lseek(fd, 1, SEEK_SET);
			write(fd, merge_flag+1,	1);
			close(fd);
		}
		return ERR_BLPL_PROC_PL_MERGE_FAIL;
	}
	else
	{
		fd = open(MERGE_FLAG_FILE, O_RDWR);
		merge_flag[1] =	0;
		lseek(fd, 1, SEEK_SET);
		write(fd, merge_flag+1,	1);
		close(fd);
		unlink(UPDATE_PL_FILE);
		unlink(UPDATE_PL_BEFORE_VER_FILE);
		printf("PL Merge Complete.\n");
		return SUCCESS;
	}
}
/*******************************************************************************
*																			   *
*  FUNCTION	ID :	  AIMergeMain										       *
*																			   *
*  DESCRIPTION :	  AI의 머지를 실행하는 UpdatePL함수를 AI를 파라미터로 넣어 *
*                     호출하고 성공실패에 따른 파일처리 및 오류기록을 담당     *
*																			   *
*  INPUT PARAMETERS:  None													   *
*																			   *
*  RETURN/EXIT VALUE:                                                          *
*             SUCCESS - 실행성공 - 변동 PL파일을 삭제한다.                     *
*             실패시 - 변동 AI 파일을 삭제하고 이전버전으로 파일버전을 롤백한다*
*																			   *
*  Author  : Woolim															   *
*																			   *
*  DATE	   : 2005-09-03														   *
*																			   *
*  REMARKS : UpdatePL은 공통모듈 함수										   *
*                                                        					   *
*******************************************************************************/
short AIMergeMain( void )
{
	short retVal = SUCCESS;
	int	fd	= 0;
	byte merge_flag[3];
	
	memset( merge_flag, 0x00, sizeof( merge_flag ) );	
	printf("AI Merge Start.\n");

	if ((retVal	= UpdatePL(AI))	< 0	)
	{
		printf("AI Merge Fail.\n");
		unlink(UPDATE_AI_FILE);
		RollbackBLPLVer( FALSE,	FALSE, TRUE);
		fd = open(MERGE_FLAG_FILE, O_RDWR);
		if ( fd	 > 0)
		{
			merge_flag[2] =	0;
			lseek(fd, 2, SEEK_SET);
			write(fd, merge_flag+2,	1);
			close(fd);
		}
		return ERR_BLPL_PROC_AI_MERGE_FAIL;
	}
	else
	{
		fd = open(MERGE_FLAG_FILE, O_RDWR);
		merge_flag[2] =	0;
		lseek(fd, 2, SEEK_SET);
		write(fd, merge_flag+2,	1);
		close(fd);
		unlink(UPDATE_AI_FILE);
		unlink(UPDATE_AI_BEFORE_VER_FILE);
		printf("AI Merge Complete.\n");
		return SUCCESS;
	}
}
/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       SearchPLinBus                                            *
*                                                                              *
*  DESCRIPTION:       입력된 정보에 따라 PL을 검색하고 그 결과를 리턴한다.     *
*                                                                              *
*  INPUT PARAMETERS:  *abCardNo - 카드번호(20byte)스트링 포인터                *
*                     dwAliasNo - alias 번호                                   *
*                     *bResult - PL체크 결과를 가져오기 위한 byte 변수 포인터  *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공                                      *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2006-01-09                                               *
*                                                                              *
*  REMARKS:           공통모듈의 SearchPL함수를 호출한다.                      *
*                                                                              *
*******************************************************************************/
short SearchPLinBus(byte *abCardNo, dword dwAliasNo, byte *bResult)
{
	short sResult = SUCCESS;
	/*
	 * 하차기에서 PL요청시 필요한 변수 선언
	 */
	byte pcCmdData[40];        // 공유메모리에 커맨드와 데이터를 넣기위한 변수
	byte SharedMemoryCmd;      // 공유메모리의 커맨드를 가져와 확인하기 위한 변수
	word wCmdDataSize;         // 공유메모리의 커맨드와 데이터의 사이즈 변수
	/*
	 * 하차기에서 PL요청 후 대기하기위한 Loop Count변수
	 */	
	int check_wait;   
	/*
	 * 하차기에서 PL요청이 승차기에서 실패한 에러코드
	 */		
	short PLCheckErrCode =0;	
	/*
	 * 하차기에서 PL요청 결과
	 */	
	*bResult = 0;

#ifndef __GCC

	/*
	 * 하차단말기에서 PL요청을 한 경우 
	 */
	if ( gboolIsMainTerm != TRUE )
	{
		/*
		 * 공유메모리에 PL요청을 위한 데이터 설정
		 */
		memset( pcCmdData, 0x00, sizeof(pcCmdData) );
		pcCmdData[0] = CMD_REQUEST;							// PL요청(1byte) 
		memcpy( pcCmdData + 1, abCardNo, PL_CARD_NO_SIZE);	// 카드번호 설정(20byte)
		memcpy( pcCmdData + 21, &dwAliasNo, sizeof(dwAliasNo) );
															// alias 설정(4byte)

		while(1)
		{
		   /*
		    * 공유메모리에 PL요청정보(25byte) 설정 
		    */
			sResult = SetSharedCmdnData( CMD_PL_CHECK, pcCmdData, 25 );
			if ( sResult < 0 )
			{
				continue;
			}
			else
			{
				break;
			}
		}
	   /*
	    * 공유메모리 PL요청후 대기시간 Loop Count 설정 
	    */
		check_wait = 1000000;

	   /*
	    * 하차 SearchPL 요청후 결과가 오는지 대기 LoopCount동안 체크
	    * 결과 리턴시 반환
	    */
		while( check_wait-- )
		{
		   /*
		    * 공유메모리 요청에 PL결과가 왔는지 여부를 체크
		    */			
		    memset( pcCmdData, 0x00, sizeof(pcCmdData) );
			GetSharedCmd(&SharedMemoryCmd, pcCmdData, &wCmdDataSize);
			
	   		/*
		    * 공유메모리 요청에 PL결과 도착시 -'4' : 체크완료 '8' : 체크불가
		    */	
			if ( SharedMemoryCmd == CMD_PL_CHECK )
			{
				if (pcCmdData[0] == '4')          // Check complete
				{	
					*bResult = pcCmdData[3];
					break;
				}
				else if (pcCmdData[0] == '8')     // Check fail
				{				
					*bResult = -1;
					/*
					* 승차기에서 검색불가의 사유 에러코드 리턴 
					*/
					memcpy( &PLCheckErrCode, &pcCmdData[1], 2);
					ClearAllSharedCmdnData();
					return PLCheckErrCode;
				}
			}
		}

		//체크 타임아웃시
		if	(check_wait == -1)
		{
			ClearAllSharedCmdnData();
			*bResult = -1;
			return ERR_BLPL_PROC_PL_CHECK_TIMEOUT;
		}
		ClearAllSharedCmdnData();
		return ErrRet( sResult );
	}

#endif

		sResult  = SearchPL(dwAliasNo, bResult);

		return sResult;

}

/*******************************************************************************
*																			   *
*  FUNCTION	ID :	  SearchBL												   *
*																			   *
*  DESCRIPTION :	  This program is main of BL Search						   *
*																			   *
*  INPUT PARAMETERS:  byte *ASCPrefix -	6 bytes	Prefix(ASCII): 1byte = 1number *
*					  dword	dwCardNum -	9 bytes	Card Number	 : 1byte = 1number *
*					  byte *bBLResult -	Result of Search BL	File			   *
*																			   *
*																			   *
*  RETURN/EXIT VALUE:  RETURN												   *
*					   1)  0 : 성공	- 결과 Pass(0),	Not	Pass(1)				   *
*					   2) -1 : "카드를 다시	대주세요"						   *
*																			   *
*  Author  : Woolim															   *
*																			   *
*  DATE	   : 2005-09-03														   *
*																			   *
*  REMARKS : 상위 카드처리에서는 RETURN값이	성공일 경우에만	bBLResult값을	   *
*			 판별하여 PASS(0),NOT PASS(1)처리하게 된다.						   *
*			 RETURN값이	실패(-1)경우에는 "카드를 다시 대주세요"처리를 한다.	   *
*			   - 변동BL	검색시 Open/Read Error : 고정 BL검색				   *
*			   - 고정BL	검색시 Open/Read Error : 3회 재시도	후 - Pass(0)	   *
*																			   *
*******************************************************************************/
short SearchBLinBus(byte *abCardNo, byte *ASCPrefix, dword dwCardNum,
	byte *bBLResult)
{
	int	check_wait = 0;     	    /* 하차 BL검색 후 대기시간(LoopCnt)*/
	byte pcCmdData[40];		         /* 공유메모리 Data 변수    */
	byte SharedMemoryCmd;           /* 공유메모리 Command 변수 */
	word wCmdDataSize;                  /* 공유메모리 Data Size    */
	short sRetVal = SUCCESS;           /* Return Variable         */
	word wBLtypePrefix;
   /* 
	* 하차기전용변수 - 승차기에서 BL검색실패시	리턴하는 에러코드를 저장하는 변수 
	*/
	short BLCheckErrCode = 0; 	
	
#ifdef TEST_BLPL_CHECK
	byte debug_pcCmdData[40];
	byte debug_SharedMemoryCmd;
	word debug_wCmdDataSize;
	dword debug_alias;
#endif

#ifdef TEST_BLPL_CHECK
	PrintlnASC(	"[SearchBL]	ASCPrefix :",  ASCPrefix, 6	);
	printf("[SearchBL] CardNum	: %lu\n", dwCardNum);
#endif

	/* 하차단말기의	경우 *//////////////////////////////////////////////////////
	if	(gboolIsMainTerm !=	TRUE )
	{
		memset(	pcCmdData, 0x00, sizeof(pcCmdData));
		pcCmdData[0] = '1';
		memcpy(	pcCmdData + 1, abCardNo, PL_CARD_NO_SIZE );
		memcpy(	pcCmdData + 21, ASCPrefix,	6);
		memcpy(	pcCmdData + 27, &dwCardNum,	sizeof( dwCardNum ) );

		/* 공유메모리에서 BL체크를 요청	: Prefix(6)	6byte와	CardNum(9):	4byte */
#ifdef TEST_BLPL_CHECK
		printf("[BLPLProc]하차 SearchBL->하차공유메모리	요청시작////////////\n");
#endif
		while(1)
		{
			sRetVal	= SetSharedCmdnData( 'B', pcCmdData, 31);
			if ( sRetVal < 0 )
			{
#ifdef TEST_BLPL_CHECK
				printf("[2]Error!!SearchBL->하차공유메모리 요청실패/////////\n");
				memset(debug_pcCmdData,	0x00, sizeof(debug_pcCmdData));
				GetSharedCmd(&debug_SharedMemoryCmd, debug_pcCmdData, &debug_wCmdDataSize);
				printf(	"[3]머가 들어있냐?====SharedMemoryCmd =	%c====\n", debug_SharedMemoryCmd );
#endif
				continue;
			}
			else
			{
#ifdef TEST_BLPL_CHECK
				printf("[BLPLProc]하차 SearchBL->하차공유메모리	요청성공////\n");
				memset(debug_pcCmdData,	0x00, sizeof(debug_pcCmdData));
				GetSharedCmd(&debug_SharedMemoryCmd, debug_pcCmdData, &debug_wCmdDataSize);
				memcpy(	&debug_alias, debug_pcCmdData + 7, sizeof( debug_alias ) );
				printf(	"\t요청	Cmd=%c,	요청 blnum=%lu \n",	debug_SharedMemoryCmd,debug_alias );
#endif
				break;
			}
		}


		check_wait = 1000000;
		while (check_wait--)
		{
			/* 공유메모리에서 BL결과가 왔는지 여부를	체크 */
			memset(	pcCmdData, 0x00, sizeof(pcCmdData) );
			GetSharedCmd( &SharedMemoryCmd,	pcCmdData, &wCmdDataSize );
#ifdef TEST_BLPL_CHECK
			if ( check_wait%10000 == 0)
			{
				printf("[BLPLProc대기중] Check_Cmd : %c", SharedMemoryCmd);
				PrintlnASC("[BLPLProc대기중]Check_pcCmdData	: ",pcCmdData+1, 10);
			}
#endif
			if	(SharedMemoryCmd ==	'B')
			{			
				if ( pcCmdData[0] =='4') /*	check completed	*/
				{							
			
					*bBLResult = pcCmdData[3];
					break;
#ifdef TEST_BLPL_CHECK
					printf("[최종결과]BL Check Result ==> %d[hex:%x]\n\n\n", pcCmdData[1],pcCmdData[1]);
#endif					
				}
				else if	(pcCmdData[0] == '8')/*	check불가 */
				{
#ifdef TEST_BLPL_CHECK
					printf("[최종결과]check불가\n");
#endif				
					*bBLResult = -1;
					memcpy( &BLCheckErrCode, &pcCmdData[1], 2);
					ClearAllSharedCmdnData();
					return BLCheckErrCode;
				}
			}
		}

		/* 하차요청	BL체크 타임아웃시-"카드를 다시 대주세요" *//////////////////
		if	(check_wait	== -1)
		{
#ifdef TEST_BLPL_CHECK
			printf("[최종결과]체크 타임아웃	: 에러코드 리턴\n");
			printf("ClearAllSharedCmdnData\n");
#endif
			ClearAllSharedCmdnData();
			*bBLResult = -1;
			return ERR_BLPL_PROC_BL_CHECK_TIMEOUT;
		}
#ifdef TEST_BLPL_CHECK
		printf("ClearAllSharedCmdnData\n");
#endif
		ClearAllSharedCmdnData();
		return ErrRet( sRetVal );
	}

	/*
	 *  승차기의 경우 공통모듈의 SearchBL 함수 호출
	 */
	/* 
	 * 고정 BL파일 존재시 검색이전에 카드번호의 Prefix에 매핑되는 
	 * 압축코드(CompCode) 찾기
	 */
	 wBLtypePrefix = GetBLTypePrefix( ASCPrefix );
	
   	 sRetVal = SearchBL( wBLtypePrefix , dwCardNum, bBLResult );

	 return ErrRet( sRetVal );
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       ReadBLPLFileCRC                            			   *
*                                                                              *
*  DESCRIPTION:       blpl_crc.dat파일의 CRC값을 읽어 입력값의 주소에 복사한다.*
*                                                                              *
*  INPUT PARAMETERS:   byte* abBLCRC           - BL CRC                        *
*	                   byte* abMifPrepayPLCRC  - 구선불 PL CRC	               *
*	                   byte* abPostpayPLCRC    - 후불 PL CRC                   *
*	                   byte* abAICRC		   - 신선불 PL CRC  	           *							           *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공 	                                   *
*                     ERR_BLPL_PROC_OPEN_BLPL_CRC_FILE - blpl_crc.dat open에러 *
*                     ERR_BLPL_PROC_READ_BLPL_CRC_FILE - blpl_crc.dat read에러 *
*																			   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:          BLPL_CRC_FILE - blpl_crc.dat파일                          *
*                    총 16byte인 파일로 내부구조는 각각의 CRC 4byte씩 구성     *
*                    0 ~ 4 byte   - BL CRC                                     *
*                    5 ~ 8 byte   - 구선불 PL CRC							   *
*                    9 ~ 12 byte  - 후불 PL CRC                                *
*                    13 ~ 16 byte - 신선불 PL CRC                              *
*******************************************************************************/
short ReadBLPLFileCRC( byte* abBLCRC, 
	                   byte* abMifPrepayPLCRC, 
	                   byte* abPostpayPLCRC,
	                   byte* abAICRC )
{
	int	fdFile = 0;
	byte abTmpBLPLFileCRC[16];	
	short sRetVal = SUCCESS;
	char chSpace[5] = { 0, };
	/*
	 * blpl_crc.dat파일 오픈
	 */		 
	fdFile = open( BLPL_CRC_FILE, O_CREAT | O_TRUNC | O_RDWR ,OPENMODE);
	if ( fdFile	< 0	)
	{
		/*
		 * blpl_crc.dat파일 오픈에러 발생시 space(0x20)으로 값을 채운다.
		 * 0x20이 아닌 'A'(0x41)로 채워야 할지는 집계담당자와 추후 상의
		 */		
		memcpy( abBLCRC, chSpace, 4 );        
		memcpy( abMifPrepayPLCRC, chSpace, 4 ); 
		memcpy( abPostpayPLCRC, chSpace, 4 );   
		memcpy( abAICRC, chSpace, 4 );			 	
		DebugOut("[BLPLPROC]blpl_crc.dat open 에러 \n");
		return ErrRet( ERR_BLPL_PROC_OPEN_BLPL_CRC_FILE );
	}
	/*
	 * blpl_crc.dat파일 read
	 */	
	sRetVal	= read(	fdFile,	abTmpBLPLFileCRC , sizeof( abTmpBLPLFileCRC ));
	close( fdFile );
	if ( sRetVal < 0 )
	{
		/*
		 * blpl_crc.dat파일 Read에러 발생시 space(0x20)으로 값을 채운다.
		 * 0x20이 아닌 'A'(0x41)로 채워야 할지는 집계담당자와 추후 상의
		 */		
		memcpy( abBLCRC, chSpace, 4 );        
		memcpy( abMifPrepayPLCRC, chSpace, 4 ); 
		memcpy( abPostpayPLCRC, chSpace, 4 );   
		memcpy( abAICRC, chSpace, 4 );		
		DebugOut("[BLPLPROC]blpl_crc.dat read 에러 \n");
		return ErrRet( ERR_BLPL_PROC_READ_BLPL_CRC_FILE );
	}
       
	/*
	 * 입력 파라미터에 CRC값 Copy
	 */		
	memcpy( abBLCRC, abTmpBLPLFileCRC, 4 );            /* BL CRC 기록 */
	memcpy( abMifPrepayPLCRC, abTmpBLPLFileCRC+4, 4 ); /* 구선불 PL CRC 기록 */
	memcpy( abPostpayPLCRC, abTmpBLPLFileCRC+8, 4 );   /* 후불 PL CRC 기록 */
	memcpy( abAICRC, abTmpBLPLFileCRC+12, 4 );		   /* 신선불 PL CRC 기록 */

	return sRetVal;

}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       WriteBLPLFileCRC                            			   *
*                                                                              *
*  DESCRIPTION:       blpl_crc.dat파일의 CRC값을 읽어 입력값의 주소에 복사한다.*
*                                                                              *
*  INPUT PARAMETERS:   byte* abBLCRC           - BL CRC                        *
*	                   byte* abMifPrepayPLCRC  - 구선불 PL CRC	               *
*	                   byte* abPostpayPLCRC    - 후불 PL CRC                   *
*	                   byte* abAICRC		   - 신선불 PL CRC  	           *							           *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공 	                                   *
*                     ERR_BLPL_PROC_OPEN_BLPL_CRC_FILE                         *
*                      - blpl_crc.dat open에러                                 *
*                     ERR_BLPL_PROC_WRITE_BLPL_CRC_FILE						   *
*                      - input parameter로 들어온 CRC type 오류                *
*                                                                              *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:   blpl_crc.dat파일에 CRC값을 기록하면 집게통신 SetUpTerm()함수에서 *
*             ReadBLPLFileCRC함수를 호출하여 install.dat파일에 기록하여 집게로 *
*             전송하는 구조로 되어있다.									       *		
*             abTmpBLPLCRC 16byte를 space에 해당하는 hex값으로 초기화 하는     *
*             이유는 MakeCRC32FromFile함수 실행이 실패한 경우 해당 4byte를     *
*             space로 남겨두어 집계에 전송시 오류발생을 나타내기 위함이다.     *
*                                                                              *
*******************************************************************************/
short WriteBLPLFileCRC(	int TypeOfCRCFile )
{
	short sRetVal =	0;
	int	fdFile = 0;	
	/*
	 * 고정 BL/PL/AI파일의 CRC값을 저장하기 위한 변수
	 */			
	dword dwBLCRC =	0;         /* BL CRC             */
	dword dwPrepayPLCRC	= 0;   /* 구선불 PL	CRC		 */
	dword dwPostpayPLCRC = 0;  /* 구후불 PL	CRC		 */
	dword dwAICRC =	0;         /* AI(신선불/후불 PL) CRC */

	/*
	 * CRC값 16byte를 write하기위한 Buffer
	 */		
	byte abTmpBLPLCRC[16] = {0, };

	/*
	 * space(0x20)으로 초기화
	 */	
	memset( abTmpBLPLCRC, 0x20, sizeof(abTmpBLPLCRC));
	
	/*
	 * blpl_crc.dat파일 오픈
	 */		 
	fdFile = open( BLPL_CRC_FILE, O_CREAT | O_TRUNC | O_RDWR ,OPENMODE);
	
	if ( fdFile	< 0	)
	{
		DebugOut("[BLPLPROC]blpl_crc.dat open 에러 \n");
		return ErrRet( ERR_BLPL_PROC_OPEN_BLPL_CRC_FILE );
	}

	/*
	 * 함수의 파라미터(TypeOfCRCFile)를 판별하여 CRC기록을 요청한 파일에 
	 * Util.c의 MakeCRC32FromFile함수를 호출하여 CRC값을 저장
	 */		
    switch( TypeOfCRCFile )
    {
    	case BL_FILE :  
			/*
			 * 마스터BL 파일의 CRC값 얻기
	    	 */		
			dwBLCRC		   = MakeCRC32FromFile(	MASTER_BL_FILE );
			if ( dwBLCRC > 0 )
			{
				memcpy(	abTmpBLPLCRC, &dwBLCRC,	4 );
				DebugOut("[BLPLPROC]MASTER_BL_FILE %lu\n",dwBLCRC );
			}
			else
			{			
				DebugOut("[BLPLPROC]MASTER_BL_FILE 파일없음\n");
			}
			/*
			 * BL CRC값 4byte 파일에 기록 
			 */
			lseek( fdFile, 0, SEEK_SET);
			sRetVal	= write( fdFile, abTmpBLPLCRC ,	4);
			if ( sRetVal < 0 )
			{
				DebugOut("[BLPLPROC]BL CRC값 blpl_crc.dat write 에러\n");
				close(fdFile);
				return ErrRet( ERR_BLPL_PROC_WRITE_BL_CRC_FILE	);
			}
			
			break;
			
		case PL_FILE : 
			/*
			 * 마스터 구선불PL 파일의 CRC값을 얻기
	    	 */	
			dwPrepayPLCRC  = MakeCRC32FromFile(	MASTER_PREPAY_PL_FILE );
			/*
			 * 마스터 후불PL 파일의 CRC값을 얻기
	    	 */				
			dwPostpayPLCRC = MakeCRC32FromFile(	MASTER_POSTPAY_PL_FILE );
			if ( dwPrepayPLCRC > 0 )
			{
				 memcpy( abTmpBLPLCRC, &dwPrepayPLCRC, 4 );
				 DebugOut("[BLPLPROC]MASTER_PREPAY_PL_FILE %lu\n",dwPrepayPLCRC	);
			}
			else
			{
				DebugOut("[BLPLPROC]MASTER_PREPAY_PL_FILE 파일없음\n");
			}
		
			if ( dwPostpayPLCRC	> 0	)
			{
				 memcpy( abTmpBLPLCRC + 4, &dwPostpayPLCRC,	4 );
				 DebugOut("[BLPLPROC]MASTER_POSTPAY_PL_FILE	%lu\n",dwPostpayPLCRC );
			}
			else
			{
				DebugOut("[BLPLPROC]MASTER_POSTPAY_PL_FILE 파일없음\n");
			}
			/*
			 * 구선불/후불 PL CRC값(8byte) 파일에 기록 
			 */
			lseek( fdFile, 4, SEEK_SET);			 
			sRetVal	= write( fdFile, abTmpBLPLCRC ,	8);
			if ( sRetVal < 0 )
			{
				DebugOut("[BLPLPROC]BL CRC값 blpl_crc.dat write 에러\n");
				close(fdFile);
				return ErrRet( ERR_BLPL_PROC_WRITE_PL_CRC_FILE	);
			}
		
			break;
			
		case AI_FILE :
			/*
			 * 마스터 신선불PL 파일의 CRC값을 얻기
	    	 */				
			dwAICRC		   = MakeCRC32FromFile(	MASTER_AI_FILE );
			if ( dwAICRC > 0 )
			{
				memcpy(	abTmpBLPLCRC, &dwAICRC, 4 );
				DebugOut("[BLPLPROC]MASTER_AI_FILE %lu\n",dwAICRC );
			}
			else
			{
				DebugOut("[BLPLPROC]MASTER_AI_FILE 파일없음\n");
			}
			/*
			 * 신선불PL CRC값(4byte) 파일에 기록 
			 */
			lseek( fdFile, 12, SEEK_SET);
			sRetVal	= write( fdFile, abTmpBLPLCRC ,	4);
			if ( sRetVal < 0 )
			{
				DebugOut("[BLPLPROC]BL CRC값 blpl_crc.dat write 에러\n");
				close(fdFile);
				return ErrRet( ERR_BLPL_PROC_WRITE_AI_CRC_FILE	);
			}			
			break;
			
		default : 
			close(fdFile);
			return ErrRet( ERR_BLPL_PROC_CRCTYPE );
	}


	close(fdFile);
	return ErrRet( sRetVal );
}




static word GetBLTypePrefix(byte *SixAscPrefix)
{
	pid_t nProcessId =0;
	int i,nTotPrefixCnt;
	word wBLTypePrefix;

	// 하차에서 요청한 경우는 통신프로세스,운전자조작기에서 요청한 경우는
	// 운전자조작기 프로세스에서  SearchBL을 호출하므로 프로세스 id로 판별하여
	// File에서 직업 compressed prefix code를 읽는다
	// printf( "getpid %d\n", getpid() );
	// printf("gpstSharedInfo->nCommPrinterProcessID %d",gpstSharedInfo->nCommPrinterProcessID);
	nProcessId = getpid();
	if (( nProcessId == gpstSharedInfo->nCommProcProcessID) ||
	    ( nProcessId == gpstSharedInfo->nCommDriverProcessID))
	{
		wBLTypePrefix = GetBLTypePrefixInFile( SixAscPrefix);
		DebugOut( " 하차요청 wBLTypePrefix           : %u\n",
	    wBLTypePrefix );
	}
	else // 승차에서 요청한 경우는 기 로드된 구조체에서 Compressed prefix code를 읽는다
	{
		nTotPrefixCnt = gstPostpayIssuerInfoHeader.wRecordCnt;
		wBLTypePrefix = -1;
//		printf("승차요청 gstPostpayIssuerInfoHeader.wRecordCnt; %d ",gstPostpayIssuerInfoHeader.wRecordCnt);
		for (i = 0; i < nTotPrefixCnt; i++)
		{
			//PrintlnASC  ( "비교Prefix  No.                           : ",
			//   gpstPostpayIssuerInfo[i].abPrefixNo, 6 );
			//PrintlnASC  ( "비교SixAscPrefix                         : ",
			//              SixAscPrefix, 6 );
			if (memcmp(gpstPostpayIssuerInfo[i].abPrefixNo, SixAscPrefix, 6) == 0)
			{
			  // PrintlnASC  ( "Prefix  No.                           : ",
			  //            gpstPostpayIssuerInfo[i].abPrefixNo, 6 );
			  // 	printf( "compressed code                   : %u\n",
	         //     gpstPostpayIssuerInfo[i].wCompCode );
				wBLTypePrefix = gpstPostpayIssuerInfo[i].wCompCode;
			//		printf( "wBLTypePrefix           : %u\n",
	         //     wBLTypePrefix );
				break;
			}
		}
	}
	DebugOut("prefix matched[%d]\n", wBLTypePrefix);
	return wBLTypePrefix;
}


static word GetBLTypePrefixInFile(byte *prefix)
{
	byte record[10], bcd_prefix[3+1];
	int	fd, min, max, comp;
	short cnt, int_prefix;
	dword offset;

	if	((fd = open( POSTPAY_ISSUER_INFO_FILE, O_RDWR, OPENMODE)) < 0) {
/**/		printf("File not found.... \n");
 		return (-1);		// File not found
	}

 	if	((read(fd, record, 9)) != 9) {
		close(fd);
/**/		printf ("Record Read Error 1... \n");
 		return (-1);		// record read_error
	}
	ASC2BCD( prefix, bcd_prefix, 6);
	//ascii_to_bcd(bcd_prefix, prefix, 6);

//	printf ("bcd_prefix : [%02x][%02x][%02x] \n", bcd_prefix[0], bcd_prefix[1], bcd_prefix[2]);
	memcpy(&cnt, &record[7], sizeof(cnt));

	min = 1;
	max = cnt;

//    printf ("min = %d   max = %d \n", min, max);

	while (min <= max)
	{
		comp = (min + max) / 2;
   		if	((max < 0) || (min < 0))
   			break;
		offset = ((comp-1) * 5) + 9;
		lseek (fd, offset, 0);
 		if	((read (fd, record, 5)) != 5)
 		{
			close(fd);
/**/		printf("Record Read Error 2... \n");
			return (-1);
		}

//	printf ("comp_prefix : [%02x][%02x][%02x] \n", record[0], record[1], record[2]);

     	if	(memcmp(record, bcd_prefix, 3) == 0)
     	{
			memcpy(&int_prefix, &record[3], sizeof(int_prefix));
			close(fd);
//	printf ("Search Prefix code = %d \n", int_prefix);
		return (int_prefix);
		}
     	if	(memcmp(record, bcd_prefix, 3) < 0)
     	{
		min = comp + 1;
		continue;
		}
		if	(memcpy(record, bcd_prefix, 3) > 0)
		{
			max = comp - 1;
			continue;
		}
	}
/**/	printf("Prefix not found.... \n");

	close (fd);
	return (-1);
}
