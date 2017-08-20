
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
*  ENTRY POINT:                                                                *
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
* 2005/08/16 Solution Team  Woo il Lim    Initial Release                      *
*                                                                              *
*******************************************************************************/

#include "bus_type.h"
#include "../proc/reconcile_file_mgt.h"

/*******************************************************************************
*  Device interface incldue로 추후 삭제                                        *
*******************************************************************************/

#define COLON   0x3A
/*******************************************************************************
*  File Macro Definition                                                       *
*******************************************************************************/
#define ERR_LOG_FILE "event.trn"
#define TERM_LOG_FILE "term_log.log"
#define STATUS_LOG_FILE "term_status.info"
/*******************************************************************************
*   Macro Definition for ctrl_event_info_write function is used limitedly      *
*  After ver.0401 program running status is stable, following macro will be    *
*  obsolete.                                                                   *
*******************************************************************************/
#define ERR_EVENT_TRN_FILE_OPEN  	0xc0	// event.trn 파일 오픈 에러
#define ERR_EVENT_TRN_FILE_WRITE	0xc2	// event.trn 파일 쓰기 오류
/*******************************************************************************
*  CTRL_EVENT_INFO structure for logging error to event.trn file is used       *
*  limitedly at ver.0401 program.                                              *
*  After ver.0401 program running status is stable, this structure wil be      *
*  substitued to following ERR_INFO_HEDAER, ERR_INFO_BODY                      *                                              *
*******************************************************************************/
typedef struct				       
{
	byte err_dtime[7];             //오류 정보 생성일시(오류일시)(BCD)	
	byte transp_method_cd[3];      //운행 구분 코드(교통수단코드)	
	byte transp_bizr_id[9];        //교통 사업자 id	                
	byte bus_biz_office_id[2];     //버스 영업소 id	                
	byte vehc_id[9];               //차량 id	                        
	byte tc_id[9];                 //단말기 id	                
	byte dev_class_cd[2];          //장비구분코드	                
	byte alarm_cd[4];              //단말기 알람코드	                
	byte crlf[2];                  //개행문자	                
}__attribute__((packed))  CTRL_EVENT_INFO;          // 관리이벤트정보파일 로딩
/*******************************************************************************
*  structure of event.trn to be applied to program of next 0402 version        *
*  after 0401 vesion running status is stable                                  *                                              *
*******************************************************************************/
typedef struct				       
{
	byte abTranspMethodCode[3];      	//운행 구분 코드(교통수단코드)	
	byte abTranspBizrID[9];         	//교통 사업자 id	                
	byte abBusBizOfficeID[2];      		//버스 영업소 id	                
	byte abVehicleID[9];                //차량 id	                        
	byte abMainTermID[9];               //단말기 id	                
	byte abDeviceClassCode[2];          //장비구분코드	
	byte abCrLf[2];                  	//개행문자	                                
}__attribute__((packed))  ERR_INFO_HEADER;      

typedef struct				       
{
	byte 	abErrDtime[8];             	//오류 정보 생성일시(오류일시)(BCD)	
	short 	sErrCode;              		//단말기 알람코드	                
	byte 	abCrLf[2];                   //개행문자	                
}__attribute__((packed))  ERR_INFO_BODY;       

/*******************************************************************************
*  structure of term_log.log to be applied to program of next 0402 version     *
*  after 0401 vesion running status is stable                                  * 
*                                                                              *
*******************************************************************************/
typedef struct				       
{
	byte abTranspMethodCode[3];      	//운행 구분 코드(교통수단코드)	
	byte abTranspBizrID[9];         	//교통 사업자 id	                
	byte abBusBizOfficeID[2];      		//버스 영업소 id	                
	byte abVehicleID[9];                //차량 id	                        
	byte abMainTermID[9];               //단말기 id	                
	byte abDeviceClassCode[2];          //장비구분코드	
	byte abCrLf[2];                  	//개행문자	                                
}__attribute__((packed))  SIMPLE_LOG_INFO_HEADER;      

typedef struct				       
{
	byte 	abLogDtime[8];             	//오류 정보 생성일시(오류일시)(BCD)	
	short 	sLogCode;              		//단말기 알람코드	                
	byte 	abCrLf[2];                   //개행문자	                
}__attribute__((packed))  SIMPLE_LOG_INFO_BODY;  


/*******************************************************************************
*  Declaration of Structure Variables                                          *
*******************************************************************************/
static ERR_INFO_HEADER         stErrInfoHeader; 
static ERR_INFO_BODY           stErrInfoBody; 
//static SIMPLE_LOG_INFO_HEADER  stSimpleLogHeader;
//static SIMPLE_LOG_INFO_BODY    stSimpleLogBody; 
ERR_LOG_MODE gstErrLogMode;

byte abDeviceClassCode[2] = "14";                          //장비구분코드 14승,15하,19운,18택시

void ErrProc( short sErrCode )
{	
	//ErrLogWrite( sErrCode );	
	switch( sErrCode & 0xFF00 )
	{  
		case 0x8100 :	
			switch( sErrCode )
			{
				case ERR_PAY_LIB_MIF_PREPAY_READ_SECTOR1 :   
	        	case ERR_PAY_LIB_MIF_PREPAY_READ_SECTOR2 :   
			}        
		case 0x8200 :	
			switch( sErrCode )
			{
				case ERR_PAY_LIB_MIF_PREPAY_READ_SECTOR1 :   
	        	case ERR_PAY_LIB_MIF_PREPAY_READ_SECTOR2 :   
			}     
		case 0x8300 :	
			switch( sErrCode )
			{
				case ERR_PAY_LIB_MIF_PREPAY_READ_SECTOR1 :   
	        	case ERR_PAY_LIB_MIF_PREPAY_READ_SECTOR2 :   
			}     
		case 0x8400 :	
			switch( sErrCode )
			{
				case ERR_PAY_LIB_MIF_PREPAY_READ_SECTOR1 :   
	        	case ERR_PAY_LIB_MIF_PREPAY_READ_SECTOR2 :   
			}     
		case 0x8500 :	
			switch( sErrCode )
			{
				case ERR_PAY_LIB_MIF_PREPAY_READ_SECTOR1 :   
	        	case ERR_PAY_LIB_MIF_PREPAY_READ_SECTOR2 :   
			}     
		case 0x8600 :	
			switch( sErrCode )
			{
				case ERR_PAY_LIB_MIF_PREPAY_READ_SECTOR1 :   
	        	case ERR_PAY_LIB_MIF_PREPAY_READ_SECTOR2 :   
			}     
		case 0x8700 :	
			switch( sErrCode )
			{
				case ERR_PAY_LIB_MIF_PREPAY_READ_SECTOR1 :   
	        	case ERR_PAY_LIB_MIF_PREPAY_READ_SECTOR2 :   
			}     
		case 0x8800 :	
			switch( sErrCode )
			{
				case ERR_PAY_LIB_MIF_PREPAY_READ_SECTOR1 :   
	        	case ERR_PAY_LIB_MIF_PREPAY_READ_SECTOR2 :   
			}     
		case 0x8900 :	
			switch( sErrCode )
			{
				case ERR_PAY_LIB_MIF_PREPAY_READ_SECTOR1 :   
	        	case ERR_PAY_LIB_MIF_PREPAY_READ_SECTOR2 :   
			}     
		case 0x9100 :	
			switch( sErrCode )
			{
				case ERR_PAY_LIB_MIF_PREPAY_READ_SECTOR1 :   
	        	case ERR_PAY_LIB_MIF_PREPAY_READ_SECTOR2 :   
			}     
		case 0x9200 :	
			switch( sErrCode )
			{
				case ERR_PAY_LIB_MIF_PREPAY_READ_SECTOR1 :   
	        	case ERR_PAY_LIB_MIF_PREPAY_READ_SECTOR2 :   
			}     
		case 0x9300 :	
			switch( sErrCode )
			{
				case ERR_PAY_LIB_MIF_PREPAY_READ_SECTOR1 :   
	        	case ERR_PAY_LIB_MIF_PREPAY_READ_SECTOR2 :   
			}     																													
		case 0x9400 :	
			switch( sErrCode )
			{
				case ERR_PAY_LIB_MIF_PREPAY_READ_SECTOR1 :   
	        	case ERR_PAY_LIB_MIF_PREPAY_READ_SECTOR2 :   
			}     
		case 0x9500 :	
			switch( sErrCode )
			{
				case ERR_PAY_LIB_MIF_PREPAY_READ_SECTOR1 :   
	        	case ERR_PAY_LIB_MIF_PREPAY_READ_SECTOR2 :   
			}     				        
	}
}

void ErrLogWrite( short sErrCode )
{
	byte abErrDtime[7];
	time_t CurTime;	
	int nRet = 0;
	int fdLogFile = 0;
	bool boolFileCreateNow = FALSE;
	struct stat stFileStatus;
	div_t stResult;
    bool boolModuleLog = FALSE;
    int nIndex = 0;
    int i = 0;
    byte bModuleLogCheck = '0';
    time_t tCurTime;
    RECONCILE_DATA ReconcileData;
//    short sReturnVal = 0;
    short sRetVal = 0;
    int nRetrySemaAlloc = 5;
	//init variables
	memset(abErrDtime,0,sizeof(abErrDtime));
	memset(&ReconcileData,0x00, sizeof( RECONCILE_DATA));

	// get the sema  
	for( nIndex =0 ; nIndex < nRetrySemaAlloc ; nIndex++)
	{                                                     
		sRetVal = SemaAlloc( SEMA_KEY_ERR_LOG );
		if ( sRetVal < 0 )
		{
			if ( nIndex == nRetrySemaAlloc -1 )
			 	return;
			else
			 	usleep(1000);			
		}
		else
		{
			break;
		}
	}
	
	// Log File Exist Check
	nRet = access(ERR_LOG_FILE, F_OK);	
	
	if ( nRet < 0)
	{	
		DebugOut("로그파일 없음 ....초기생성\n");					
		fdLogFile = open(ERR_LOG_FILE, O_RDWR | O_CREAT );
		if ( fdLogFile < 0 ) 
		{ 
			SemaFree( SEMA_KEY_ERR_LOG );
			return;
		}    	
		boolFileCreateNow = TRUE;					
	}
	else //debug
	{ DebugOut("로그파일 있음 ....오픈\n");	
		fdLogFile = open(ERR_LOG_FILE, O_RDWR | O_APPEND ); 
	    if ( fdLogFile < 0 ) 
		{ 
			SemaFree( SEMA_KEY_ERR_LOG );
			return;
		}    	
	} 		

	// Check File Size
	fstat( fdLogFile, &stFileStatus );
	stResult = div(stFileStatus.st_size,1024000);
	
	DebugOut("stResult.quot : %d\n\n", stResult.quot);
	// If FileSize is above 1MegaByte, Delete LogFile and recreate 
	if ( stResult.quot >= 1 )	
	{	
		DebugOut("파일사이즈 1M가 넘음 \n");
		close( fdLogFile );
		unlink( ERR_LOG_FILE );
		    	nRet = access(ERR_LOG_FILE, F_OK);	
				if ( nRet < 0) DebugOut("화일삭제 확인\n");	
		fdLogFile = open(ERR_LOG_FILE, O_RDWR | O_CREAT );
				          DebugOut( " fdLogFile : %d /n/n", fdLogFile);  
		if ( fdLogFile < 0 ) 
		{ 
			SemaFree( SEMA_KEY_ERR_LOG );
			return;
		}    	
		boolFileCreateNow = TRUE;	
	}
	else //debug
	{
		DebugOut("파일 사이즈 1M미만\n");	
    }
	
	lseek(fdLogFile, 0, SEEK_END);
	
	// Read LogMode, ModuleLogFlag

	gstErrLogMode.boolIsAllLogMode = TRUE;
	
	DebugOut("\r\n SelectLogMode : [%02d] AllLogMode : [%02d]\r\n", gstErrLogMode.boolIsSelectLogMode,  gstErrLogMode.boolIsAllLogMode);
	 
	// Write Log properly according to specific function on/off flag
	if ( gstErrLogMode.boolIsSelectLogMode == TRUE )
	{
		bModuleLogCheck = ( sErrCode >> 2 ) & 0xFF;
		 
		for( i = 0 ; i < NUM_OF_MODULE ; i++ )
		{	
			if ( bModuleLogCheck == gstErrLogMode.abModuleLog[i] )
			{
				boolModuleLog = TRUE;		     
				break;
			}
		}
    }
        
	if (( gstErrLogMode.boolIsAllLogMode == TRUE) || ( boolModuleLog == TRUE ))// logging specific function logging Judge if input code log
	{
		if ( boolFileCreateNow == TRUE )
		{
			memset( &stErrInfoHeader,0x00,sizeof(stErrInfoHeader));
    	
			memcpy( stErrInfoHeader.abTranspMethodCode , gstVehicleParm.abTranspMethodCode   , sizeof(stErrInfoHeader.abTranspMethodCode)   );
			DebugOut( " %d\n", sizeof(gstVehicleParm.abTranspMethodCode) );
			memcpy( stErrInfoHeader.abTranspBizrID     , gstVehicleParm.abTranspBizrID     , sizeof(stErrInfoHeader.abTranspBizrID)       );
			DebugOut( " %d\n", sizeof(gstVehicleParm.abTranspBizrID)  );
			memcpy( stErrInfoHeader.abBusBizOfficeID   , gstVehicleParm.abBusBizOfficeID   , sizeof(stErrInfoHeader.abBusBizOfficeID)     ); 
			DebugOut( " %d\n", sizeof(gstVehicleParm.abBusBizOfficeID)   );
			memcpy( stErrInfoHeader.abVehicleID        , gstVehicleParm.abVehicleID        , sizeof(stErrInfoHeader.abVehicleID)          );
			DebugOut( " %d\n",  sizeof(gstVehicleParm.abVehicleID)  );			
			memcpy( stErrInfoHeader.abMainTermID       , gpstSharedInfo->abMainTermID      , sizeof(stErrInfoHeader.abMainTermID)         );   
			DebugOut( " %d\n",  sizeof( gpstSharedInfo->abMainTermID)   );		
			memcpy( stErrInfoHeader.abDeviceClassCode  , abDeviceClassCode                 , sizeof(stErrInfoHeader.abDeviceClassCode)    );
			DebugOut( " %d %s\n", sizeof(abDeviceClassCode), abDeviceClassCode );
			
			stErrInfoHeader.abCrLf[0] = CR;
			stErrInfoHeader.abCrLf[1] = LF;
			
		   	// RECONCILE 파일에 등록 ///////////////////////////////////////////
			memset((byte *)&ReconcileData, 0, sizeof(RECONCILE_DATA));

			memcpy( ReconcileData.achFileName, ERR_LOG_FILE, strlen( ERR_LOG_FILE));
			ReconcileData.bSendStatus = RECONCILE_SEND_ERR_LOG;
			GetRTCTime( &tCurTime );
			ReconcileData.tWriteDtime = tCurTime;

		    DebugOut( "Reconcile write [%s]\n", ERR_LOG_FILE );
		    if ( WriteReconcileFileList( &ReconcileData ) < 0 )   
		       	return;	 
		
		
		    if ( (write(fdLogFile, (void*)&stErrInfoHeader, sizeof(ERR_INFO_HEADER))) != sizeof(ERR_INFO_HEADER))
			{ 
				SemaFree( SEMA_KEY_ERR_LOG );
				return;
			}    	
		}
		
	    // Get Log Time  -	int	rtc_gettime(char *time)
		GetRTCTime( &CurTime );
		TimeT2BCDDtime( CurTime, abErrDtime );
		
		memcpy( stErrInfoBody.abErrDtime , abErrDtime, sizeof(abErrDtime) );
		stErrInfoBody.abErrDtime[7] = COLON ;
		stErrInfoBody.sErrCode = sErrCode;
		DebugOut("%x", stErrInfoBody.sErrCode);
		stErrInfoBody.abCrLf[0]  = CR;
		stErrInfoBody.abCrLf[1]  = LF;	 	

		//WriteLogInfo();		
		if ( (write(fdLogFile, (void*)&stErrInfoBody, sizeof(ERR_INFO_BODY))) != sizeof(ERR_INFO_BODY))
		{ 
			SemaFree( SEMA_KEY_ERR_LOG );
			return;
		}    	

		DebugOut("\r\n ctrl_event_info_write O.K");
	}

	close( fdLogFile );
	  
    // free the sema
	SemaFree( SEMA_KEY_ERR_LOG );

}


void LogWrite( short sLogCode )
{
	/*0401에서는 LogWrite에 의한 term_log.log를 생성하지 않음.*/
	/*
	byte abLogTime[15];
	time_t CurTime;
	
	int nRet;
	int fdTermLogFile;
	bool boolFileCreateNow;
	struct stat stFileStatus;
	div_t stResult;
	short sRetVal = 0;
	time_t tCurTime;
    RECONCILE_DATA ReconcileData;
//    short sReturnVal = 0;
    int nRetrySemaAlloc = 5;
    int nIndex = 0;
    
	//init variables
	memset(abLogTime,0,sizeof(abLogTime));
	memset(&ReconcileData,0x00, sizeof( RECONCILE_DATA));
	boolFileCreateNow = FALSE;                                                                                   
	                                                                             
	// get the sema  
	for( nIndex =0 ; nIndex < nRetrySemaAlloc ; nIndex++)
	{                                                     
		sRetVal = SemaAlloc( SEMA_KEY_LOG );
		if ( sRetVal < 0 )
		{
			if ( nIndex == nRetrySemaAlloc -1 )
			 	return;
			else
			 	usleep(1000);			
		}
		else
		{
			break;
		}
	}
	
	                                                                             
	// Log File Exist Check                                                      
	nRet = access(TERM_LOG_FILE, F_OK);	                                         
	                                                                             
	if ( nRet < 0)                                                               
	{						                                                     
		DebugOut("로그파일 없음 ....초기생성\n");
		fdTermLogFile = open(TERM_LOG_FILE, O_RDWR | O_CREAT );
		DebugOut("로그파일 없음 ....생성완료\n");                               
		if ( fdTermLogFile < 0 )  
		{ 
			SemaFree( SEMA_KEY_LOG );
			return;
		}  	                                     
		boolFileCreateNow = TRUE;					                             
	}
 	else //debug                                                    
 	{ 
 		DebugOut("로그파일 있음 ....오픈\n");	                        
 		fdTermLogFile = open(TERM_LOG_FILE, O_RDWR | O_CREAT | O_APPEND );                     
 	    if ( fdTermLogFile < 0 ) 		
 	    { 
			SemaFree( SEMA_KEY_LOG );
			return;
		}                           
 	}	                                                                         
	                                                                             
	// Check File Size                                                           
	fstat( fdTermLogFile, &stFileStatus );                                       
	stResult = div(stFileStatus.st_size,1024000);                                
	                                                                             
	// If FileSize is above 1MegaByte, Delete LogFile and recreate               
	if ( stResult.quot >= 1 )	                                                 
	{	 
		DebugOut("파일사이즈 1M가 넘음 \n");                                                                        
		close( fdTermLogFile );                                                  
		unlink( TERM_LOG_FILE );                                                 
 		    	nRet = access(TERM_LOG_FILE, F_OK);	                
 				if ( nRet < 0) DebugOut("화일삭제 확인\n");	 		                                                                         
		fdTermLogFile = open(TERM_LOG_FILE, O_RDWR | O_CREAT );
              
		if ( fdTermLogFile < 0 )  
		{ 
			SemaFree( SEMA_KEY_LOG );
			return;
		}  
	}
	else //debug                                                    
 	{                                                               
 		DebugOut("파일 사이즈 1M미만\n");	                        
    }                                                                             
                                                                                 
	if ( boolFileCreateNow == TRUE )                                             
	{                                                                            
		memset( &stSimpleLogHeader,0x00,sizeof(stSimpleLogHeader));                           
	                                                                              
		memcpy( &stSimpleLogHeader.abTranspMethodCode , gstVehicleParm.abTranspMethodCode , sizeof(gstVehicleParm.abTranspMethodCode)   );
		memcpy( &stSimpleLogHeader.abTranspBizrID     , gstVehicleParm.abTranspBizrID     , sizeof(gstVehicleParm.abTranspBizrID)       );
		memcpy( &stSimpleLogHeader.abBusBizOfficeID   , gstVehicleParm.abBusBizOfficeID   , sizeof(gstVehicleParm.abBusBizOfficeID)     ); 
		memcpy( &stSimpleLogHeader.abVehicleID        , gstVehicleParm.abVehicleID        , sizeof(gstVehicleParm.abVehicleID)          );
		memcpy( &stSimpleLogHeader.abMainTermID       , gpstSharedInfo->abMainTermID     , sizeof( gpstSharedInfo->abMainTermID)          );   
		memcpy( &stSimpleLogHeader.abDeviceClassCode  , abDeviceClassCode                , sizeof(abDeviceClassCode)     );
   
		stSimpleLogHeader.abCrLf[0] = CR;
		stSimpleLogHeader.abCrLf[1] = LF;
		
		//reconcielList 등록;
		memcpy( ReconcileData.achFileName, TERM_LOG_FILE, strlen( TERM_LOG_FILE));
		ReconcileData.bSendStatus = RECONCILE_SEND_TERM_LOG;
		GetRTCTime( &tCurTime );
		ReconcileData.tWriteDtime = tCurTime;
		
	   	// RECONCILE 파일에 등록 ///////////////////////////////////////////
		memset((byte *)&ReconcileData, 0, sizeof(RECONCILE_DATA));		
		//reconcielList 기등록여부 확인
        if ( WriteReconcileFileList( &ReconcileData ) < 0 )   
	       	return;	 
	   
			
		if ( (write(fdTermLogFile, (void*)&stSimpleLogHeader, sizeof(SIMPLE_LOG_INFO_HEADER))) != sizeof(SIMPLE_LOG_INFO_HEADER))
		{ 
			SemaFree( SEMA_KEY_LOG );
			return;
		}  
	}


    // Get Log Time  -	int	rtc_gettime(char *time)
	GetRTCTime( &CurTime );
	TimeT2BCDDtime( CurTime, abLogTime );
		
	memcpy( stSimpleLogBody.abLogDtime , abLogTime, sizeof(&stSimpleLogBody.abLogDtime) );
   	stSimpleLogBody.abLogDtime[7] = COLON ;
	stSimpleLogBody.sLogCode = sLogCode;
		DebugOut("%x", stErrInfoBody.sErrCode);
	stSimpleLogBody.abCrLf[0]  = CR;
	stSimpleLogBody.abCrLf[1]  = LF;	 	 
	
	if ( (write(fdTermLogFile, (void*)&stSimpleLogBody, sizeof(SIMPLE_LOG_INFO_BODY))) != sizeof(SIMPLE_LOG_INFO_BODY))
	{ 
		SemaFree( SEMA_KEY_LOG );
		return;
	}  

	DebugOut("\r\n ctrl_event_info_write O.K");

	close( fdTermLogFile );
	  
 	SemaFree( SEMA_KEY_LOG );
 	*/
}



int SemaCreate( int nkey )
{
	int semid_Comm = -1;  //에러로그 세마포어 아이디

	union semun 
	{ 
		int val; 
		struct semid_ds *buf; 
		unsigned short int *array; 
	}; 

	int status = 0; 
	union semun sem_union; 

	semid_Comm = semget((key_t)nkey, 1, 0666|IPC_CREAT);
	if (semid_Comm == -1) {
		printf("\r\n 에러로그 세마포어 생성 실패\r\n");    
		return -1;
	}

	sem_union.val = 1; 
	status = semctl(semid_Comm, 0, SETVAL, sem_union); 

	if (status == -1){		
		printf("\r\n 에러로그 세마포어 생성 실패 [%d]\r\n",status);    
		return -1;		
	}
		
	return 1;
	
}


int SemaAlloc( int nkey )
{
	int semid_Comm = -1;  //에러로그 세마포어 아이디

	struct sembuf sem_TranOpen  = {0, -1, SEM_UNDO}; // 세마포어 얻기

	semid_Comm = semget((key_t)nkey, 0, 0666|IPC_CREAT);
	if (semid_Comm == -1) {
		printf("\r\n 에러로그 세마포어 접근 실패 \r\n");
		return -1;
	}

	if(semop(semid_Comm, &sem_TranOpen, 1) == -1)
	{
       	printf("\r\n 에러로그 세마포어 사용 실패 \r\n");
		return -1;
	}
	
    	return 1;	
}

//사용내역 만들어진 세마포어에 접근 해제
int SemaFree( int nkey )
{

	int semid_Comm = -1;  //에러로그 세마포어 아이디
	
	struct sembuf sem_TranClose = {0, 1, SEM_UNDO};  // 세마포어 돌려주기

	semid_Comm = semget((key_t)nkey, 0, 0666|IPC_CREAT);
	if (semid_Comm == -1) {
		printf("\r\n 에러로그 세마포어 접근 실패 \r\n");
		return -1;
	}

	if (semop(semid_Comm, &sem_TranClose, 1) == -1){
       	printf("\r\n 에러로그 세마포어 해제 실패 \r\n");
	}

    	return 1;	
}


//0401 버젼이 안정화되기까지 에러로그를 남기는 function을 한시적으로 적용
//사유 : 0401버젼과 03xx버젼이 운행을 같이 하게되는 상황이 올 수도 있으므로
//       집계로 올리는 file의 변경이 불가능하다. 추후 0401버젼이 모두 설치된
//       것을 확인하고 변경하도록 한다.
int ctrl_event_info_write (char * ALARM_CD)
{
	int fileFd = 0;
	int retVal = 0;
	RECONCILE_DATA 	ReconcileData;
	time_t tCurTime;
	CTRL_EVENT_INFO data;

	if ( gboolIsMainTerm == FALSE )
	{
		return ERR_FILE_WRITE;
	}

	tCurTime = 0;
	memset(&data,0x00,sizeof(data));
   
	GetRTCTime( &tCurTime );
	TimeT2BCDDtime( tCurTime, data.err_dtime );

	   	   
    memcpy(data.transp_method_cd,  gstVehicleParm.abTranspMethodCode   , sizeof(data.transp_method_cd));
    memcpy(data.transp_bizr_id,    gstVehicleParm.abTranspBizrID     , sizeof(data.transp_bizr_id));
    memcpy(data.bus_biz_office_id, gstVehicleParm.abBusBizOfficeID   , sizeof(data.bus_biz_office_id)); 
    memcpy(data.vehc_id,           gstVehicleParm.abVehicleID        , sizeof(data.vehc_id));
    memcpy(data.tc_id,             gpstSharedInfo->abMainTermID      , sizeof(data.tc_id));   
    memcpy(data.dev_class_cd,      abDeviceClassCode                 , sizeof(data.dev_class_cd));
    memcpy(data.alarm_cd,          ALARM_CD                          , sizeof(data.alarm_cd));   
   
	data.crlf[0] = CR;
	data.crlf[1] = LF;	 
   
	if ((fileFd = open(ERR_LOG_FILE, O_WRONLY | O_CREAT | O_APPEND, OPENMODE)) < 0)
	{
    	printf("[ctrl_event_info_write] event.trn 파일 OPEN 실패\n");
		return ERR_FILE_OPEN;
	}

	if ((write(fileFd, (void*)&data, sizeof(CTRL_EVENT_INFO))) != sizeof(CTRL_EVENT_INFO))
	{
		close(fileFd);
    	printf("[ctrl_event_info_write] event.trn 파일 WRITE 실패\n");
		retVal = ERR_FILE_WRITE;
	}

	close(fileFd);

   	// RECONCILE 파일에 등록 ///////////////////////////////////////////////////
	memset((byte *)&ReconcileData, 0, sizeof(RECONCILE_DATA));
	
	memcpy( ReconcileData.achFileName, ERR_LOG_FILE, strlen( ERR_LOG_FILE));
	ReconcileData.bSendStatus = RECONCILE_SEND_ERR_LOG;
	GetRTCTime( &tCurTime );
	ReconcileData.tWriteDtime = tCurTime;

	// ReconcielList Write- 기등록시 Return
    if ( WriteReconcileFileList( &ReconcileData ) < 0 )   
    {
//    	printf("[ctrl_event_info_write] event.trn 파일에 기등록됨\n");
       	return ERR_FILE_WRITE;
    }

	return retVal;
}
