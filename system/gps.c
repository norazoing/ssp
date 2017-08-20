/*****************************************************************************
* 신 교통카드 프로젝트
*
* Copyright (c) 2003-2005 by LG CNS, Inc.
*
*     All rights reserved.
*
*****************************************************************************
*  내용설명   : 정류장 인식 프로그램 입니다.
*  INPUT      : c_st_inf.dat
*  OUTPUT     : simxlog.dat, simxinfo.dat
*  $Id: gps.c,v 1.48 2006/07/11 00:57:53 jaeekim Exp $
*  $Log: gps.c,v $
*  Revision 1.48  2006/07/11 00:57:53  jaeekim
*  운행궤적 사이즈 1M로,
*  단말기 껐다 켰을 경우에 이어지도록
*
*  Revision 1.47  2006/06/16 10:41:39  jeonbh
*  memcpy() 관련 size 변경
*
*  Revision 1.46  2006/06/15 01:05:49  jeonbh
*  sprintf() 관련 오류 수정
*
*  Revision 1.45  2006/06/12 00:31:41  jeonbh
*  현재시간 출력 포맷 변경
*
*  Revision 1.44  2006/05/19 02:47:41  jaeekim
*  *** empty log message ***
*
*  Revision 1.43  2006/05/16 09:58:14  jaeekim
*  *** empty log message ***
*
*  Revision 1.42  2006/05/16 07:45:35  jaeekim
*  *** empty log message ***
*
*  Revision 1.41  2006/02/28 05:25:19  jaeekim
*  *** empty log message ***
*
*  Revision 1.40  2006/02/24 01:55:20  jeonbh
*  컴파일시 warning 을 발생시키는 코드 제거
*
*  Revision 1.39  2006/01/24 09:49:39  jaeekim
*  *** empty log message ***
*
*  Revision 1.38  2006/01/24 09:40:15  jaeekim
*  *** empty log message ***
*
*  Revision 1.37  2005/11/24 07:31:59  jaeekim
*  *** empty log message ***
*
*  Revision 1.36  2005/11/24 07:29:52  jaeekim
*  *** empty log message ***
*
*  Revision 1.35  2005/11/23 07:43:19  jaeekim
*  *** empty log message ***
*
*  Revision 1.34  2005/11/19 11:26:48  jaeekim
*  *** empty log message ***
*
*  Revision 1.16  2005/09/22 01:13:08  jaeekim
*  *** empty log message ***
*
*  Revision 1.15  2005/09/21 01:09:13  mhson
*  컴파일오류
*
*  Revision 1.14  2005/09/15 08:37:05  jaeekim
*  *** empty log message ***
*
*  Revision 1.9  2005/09/09 06:32:31  gykim
*  *** empty log message ***
*
*  Revision 1.8  2005/09/09 06:02:10  gykim
*  *** empty log message ***
*
*  Revision 1.7  2005/09/08 04:58:34  jeonbh
*  신규버스단말기 프로그램에서 컴파일되도록 수정
*
*  Revision 1.6  2005/09/08 04:22:42  jaeekim
*  *** empty log message ***
*
*****************************************************************************/
#define VERSION		"GPS 3.0"

#include "gps_define.h"

#ifdef	_GPS_27_
	#include "../include/bus100.h"
	#include "../include/simx.h"
#else
	#include <stdio.h>
	#include <stdlib.h>
	#include "../../common/type.h"
	#include "../system/bus_type.h"
	#include "../proc/reconcile_file_mgt.h"
	#include "gps.h"	
	#include "gps_env.h"
	#include "gps_search.h"
	#include "gps_util.h"

#endif

GPS_DATA GPS;						// RMC 데이타 저장 구조체
GPS_INFO_STATION st_GPS_FINAL_INFO; // 에러 로그파일의 마지막 줄에 쓰일 구조체
									// (인식률, 운행거리, 운행횟수)
									
GPS_INFO_STATION stGPS_INFO_STATION[MAXSTATIONCNT];

int g_nTotalStaCnt = 0;		// 정류장 총 수
int g_nContinueFlag = 0;		// 껐다 킨 경우 운행연속인가(1), 아닌가(0)?

int g_nNextStaOrder = 0;
int g_nPrevStaOrder = 0;
int g_nCurStaOrder = 0;

int g_nPassMinDis = 0;				// 정류장통과시 최소거리
char g_szPassMinLongitude[10];	
char g_szPassMinLatitude[9];	

char g_szVehcID[10];	// 차량 id
char g_szBusRouteID[9];	// 노선 id
	
char g_szPassMinTime[15];			
char g_szPassSatelliteCnt[3];	

char szGpsLogStartTime[15];	
char szGpsLogEndTime[15];	
char g_szRunDepartTime[15];	// 운행시작시간

char g_szStaFindTime[15];	// 현재 정류장 인식시간

int g_gps_fd = -1;				// gps 포트 핸들
int g_bInc = FALSE;
int g_nFirstRun = 0;	// 운행시작후 정류장 검색이 첨인지?

int gpsloopcheck = 4;

//GPS 2.7 적용. 모듈에서 보내온 시간 (단말기 시간 아님.)
char g_szGpsCurrTime[15];

int g_nGpsExceptCnt = 0;	// 비정상 운행된 정류장 총 수

byte g_gpsStatusFlag = 0;	// 현재 gps 수신상태 플래그 선언
//////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////
// 버스 운행거리 산정을 위한 변수들..
//static DRIVEDIST stDriveDist;

int	nMoveDis = 0;	// 운행시작부터 운행종료시까지 이동한 거리

byte old_station_id[8];

////////////////////////////
// SIMX Format...
BUSINFO			BusInfo;				// 정류장 인식 정보 저장 구조체(BusStationDetect함수의 리턴값)
COORDINATES		coordinates;			// 좌표변환 구조체
RMC_DATA		s_stRMC;				// RMC 데이터 저장 구조체
GGA_DATA		g_stGGA;				// GGA 데이터 저장 구조체
////////////////////////////

#ifdef	_SIMMODE_
static FILE*	s_SimFile = NULL;	// 시뮬레이션용 RAW 파일
#endif

//static unsigned int FirstRun =0;
//static unsigned int FirstWriteFlag =0;

GPS_INFO_STATION g_stGPS_INFO_STATION[MAXSTATIONCNT];


STA_INFO_LOAD	STA_IF_LOAD[256];       // 정류장 정보 로딩

// 기점을 기준으로 하여 가장 가까운 순서대로 정렬한 데이타 값을 가지고 
// 있는 배열.
// 현재 위치 대비 기준 반경내 들어오는 정류장을 검색하기 위함.
SORT_X	g_stSortX[MAXSTATIONCNT];
SORT_Y	g_stSortY[MAXSTATIONCNT];

double g_dbRoundX;	// 정류장 검색 반경 X'
double g_dbRoundY;	// 정류장 검색 반경 Y'


static	struct timeval stTime1, stTime2;
static 	int bTimeCheck = 0;

// 통합, 2.7 모두 수용하기 위한 중간 버퍼
COMM_STA_INFO g_stStaInfo[MAXSTATIONCNT];

// 버스 정류장 DB 정보 저장 구조체
BUS_STATION	g_stBusStation[MAXSTATIONCNT];

double g_dbMaxSpeed = 0;	// 운행시작 후 검지된 최대 속도값
//===========================================================================//
// 함수 정의 부분 시작
void ThreadStartCheck(void);

int BusStationInfoLoading(int current_new);
unsigned int ReadyStaInfo(int nStation_cnt);
int ContinueDriveCheck(void);
int ReadyWriteLogFile(void);
void SyncDeparttimeWithMainLogic(char* szStartLogTime);
int GpsDataProcessing(void);

int GpsRecv(int fd, unsigned char* buffer, int timeout);

int ParseGGA(GGA_DATA* gga, unsigned char *Data, int DataNum);
int CheckSum(unsigned char *Data);
void clean_up(void *arg);
void GpsErrorProcess(int param);
void WriteTrace(RMC_DATA* rmc);
void* gpsMain(void *arg);


void GpsExitProc(void);
//void GpsDataProc(void);
short GpsPortProc(void);
void BusStationInfoSet(void);
static void GpsDataProc(void);

// 함수 정의 부분 끝
//=============================================================================

void* gpsMain(void *arg)
{
	
	GlobalParameterInit();	// static 변수 초기화

	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_cleanup_push(clean_up, (void *)NULL);

	ThreadStartCheck();		// 스레드 정상 기동했는지 검사
	
	BusStationInfoSet();	// 버스 정류장 기본정보 세팅

	if (GpsPortProc() < 0)	// GPS 포트 열기
	{
		GpsExitProc();// Gps 스레드 종료 프로세싱
		return (void*)NULL;	// gps 포트 관련 에러 발생하여 스레드 종료
	}
	
	ReadyWriteLogFile();	// 로그 기록 준비	

	strcpy(g_szStaFindTime, g_szRunDepartTime);	// 처음 정류장 인식한 시간은 운행시작시간


	/*
	 * 운행시작 전에 현재 검색된 정류장과 파일에서 읽어온 이전 마지막 인식 정류장
	 * 과 비교하여 다음과 같은 기준으로 차이가 발생하면
	 * [정류장 위치를 확인해 주시기 바랍니다] 라는 음성출력
	 */
	CheckingCurrStation();
	 
	/*
	 *	GPS 데이타를 수신받아 정류장 검색 및 로그 기록
	*/
	while (1)	
	{
		GpsDataProc();	// GPS 데이타 프로세싱(정류장 검색 포함)
	}
	
	GpsExitProc();	// Gps 스레드 종료 프로세싱

	pthread_cleanup_pop(0);	
	
}

/*******************************************************************************
 *		
 *	function: ReadyStaInfo
 *	Param: g_stBusStation :버스 정류장 정보를 저장하는 구조체배열 포인터 
 * 	       nSTA_IF_LOAD : 버스 정류장 정보를 저장하는 아스키 구조체 
 *             coordinates : 좌표값을 저장하는 구조체 
 * 	Return: 
 *		0 : no error
 *		1 : g_stBusStation is null pointer
 *		2 : nStation_cnt is overflow data number, check the nStation_cnt 
 *													or MAXSTATIONCNT
 *		3 : nStation_cnt is overflow data number, check the nStation_cnt 
 *		4 : g_stBusStation[i].StationOrder is wrong value
 *		5 : g_stBusStation[i].DBLen is zero
 *		6 : nStation_Cnt is zero
 *		7 : g_stBusStation[i].LonDeg is wrong value
 *		8 : g_stBusStation[i].LatDeg  is wrong value
 *		9 : g_stBusStation[i].Angle is wrong value
 *	discription : 버스 정류장 처리를 위한 전처리를 한다. 
 *	note: 이 함수를 호출하기 전에 g_stBusStation구조체에 반드시 버스 DB 
 * 									데이터가 디코딩된값이 저장되어 있어야 한다.
 *	        g_stBusStation구조체 멤버중 	
 * 		unsigned int StationID; 	//ID
 *		unsigned int StationOrder; 	//Station order
 *		unsigned int DBLen;  		//Station number
 *		double LonDeg;  		//Station longitude Degree, DD.ddddd
 * 		double LatDeg;  		//Station latitude  Degree, DD.ddddd
 *		double Angle;  			//Station Angle
 *	      값들은 값이 채워져 있어야 한다. 	
 *            이 함수는 반드시 BusStationDetect()을 호출하기 전에 반드시 선행 
 * 																되어져야 한다. 
 *	      일반적으로 GPS main 함수에서 초기화 루틴에서 선언해 주면 된다. 
 ******************************************************************************/
unsigned int ReadyStaInfo(int nStation_cnt)
{
	int i=0;
	double tempXYZ[3];
	double tempLLH[3];
	double pos_x, pos_y;
	
	memset(&coordinates,0x00,sizeof(COORDINATES));
	///////////////////////////////////////////////////////////////////////////////
	
	for (i = 0; i < nStation_cnt; i++)	{
		g_stBusStation[i].StationID = atointeger(g_stStaInfo[i].abStationID, 7);
		g_stBusStation[i].StationOrder = atointeger(g_stStaInfo[i].bStationOrder, 3);
	  	g_stBusStation[i].DBLen = nStation_cnt;  //Station number

		pos_x = g_stStaInfo[i].dLongitude;
		pos_y = g_stStaInfo[i].dLatitude;
		g_stBusStation[i].LatDeg = (int)(pos_y/100) + ((pos_y - (int)(pos_y/100)*100.0)/60);
		g_stBusStation[i].LonDeg = (int)(pos_x/100) + ((pos_x - (int)(pos_x/100)*100.0)/60);
		
	 	g_stBusStation[i].Angle  = g_stStaInfo[i].wAngle;  //Station Angle
	 	g_stBusStation[i].AccumDist = g_stStaInfo[i].dwDist; //Station Accumulated Distance
	 	g_stBusStation[i].DiffDist = 0;  //70% Distance between current station and previous station 
	 	g_stBusStation[i].DiffDist2 = 0; // Distance between current station and previous station 
	 	g_stBusStation[i].ENU[0] = 0;  //ENU value
	 	g_stBusStation[i].ENU[1] = 0;  //ENU value
	 	g_stBusStation[i].ENU[2] = 0;  //ENU value
	}
	
	
	/*********************************Excetion Process ****************************************/
	
	//null pointer test
	if(g_stBusStation == 0x00)
	{
		return 1;
	}
	
	//nStation_cnt value test
	if(nStation_cnt > MAXSTATIONCNT)
	{
		return 2;
	}
	for(i =0; i < nStation_cnt; i++)
	{
		if(nStation_cnt == 0)
		{
			return 5;
		}
		if(g_stBusStation[i].StationOrder > nStation_cnt)
		{
			return 6;
		}
		if(g_stBusStation[i].LonDeg > 180.0 || g_stBusStation[i].LonDeg < 0)
		{
			return 7;
		}
		if(g_stBusStation[i].LatDeg > 90.0 || g_stBusStation[i].LatDeg < 0)
		{
			return 8;
		}
		if(g_stBusStation[i].Angle > 360.0 || g_stBusStation[i].Angle < 0)
		{
			return 9;
		}
	}
	
	/************************Exception Precess End*********************************************/
	

	//1번 DB에 대한 기준 좌표 LLH값 저장 
	coordinates.OrgLLH[0] = g_stBusStation[0].LatDeg*D2R;
	coordinates.OrgLLH[1] = g_stBusStation[0].LonDeg*D2R;
	coordinates.OrgLLH[2] = 30;  //heigth 30m fix

	LatLonHgtToXYZ_D(coordinates.OrgLLH,coordinates.OrgXYZ);


    //초기 DB의 ENU값은 모두 0이므로 계산하지 않고 바로 0으로 
	g_stBusStation[0].ENU[0] =0;
	g_stBusStation[0].ENU[1] =0;
	g_stBusStation[0].ENU[2] =0;


	//1번 정류장에 대해서 모든 DB에 대해 ENU 값을 구한다. 
	for(i = 1 ; i < nStation_cnt ; i++)
	{
		tempLLH[0] = g_stBusStation[i].LatDeg*D2R;
		tempLLH[1] = g_stBusStation[i].LonDeg*D2R;
		tempLLH[2] = 30;  //heigth 30m fix 
	  	LatLonHgtToXYZ_D(tempLLH,tempXYZ);
		XyztoENU_D(tempXYZ,coordinates.OrgXYZ,coordinates.OrgLLH,g_stBusStation[i].ENU);
#ifdef	_DEBUGGPS_		
		printf("g_stBusStation[%d].ENU[0] = %f\n", i, g_stBusStation[i].ENU[0]);
		printf("g_stBusStation[%d].ENU[1] = %f\n", i, g_stBusStation[i].ENU[1]);
#endif
	}

	//역간 거리를 계산하여 저장 한다. 
	g_stBusStation[0].DiffDist = DIFF_DIST_P_FIRST;
	g_stBusStation[0].DiffDist2 = 0;	
	for(i =1; i < nStation_cnt; i++)
	{
		g_stBusStation[i].DiffDist = (g_stBusStation[i].AccumDist - g_stBusStation[i-1].AccumDist) * DIFF_DIST_P;
		g_stBusStation[i].DiffDist2 = (g_stBusStation[i].AccumDist - g_stBusStation[i-1].AccumDist);	
#ifdef	_DEBUGGPS_		
		printf("g_stBusStation[%d].DiffDist = %f\n", i, g_stBusStation[i].DiffDist);
		printf("g_stBusStation[%d].DiffDist2 = %f\n", i, g_stBusStation[i].DiffDist2);
#endif		
	}	


	// 정류장 좌표 정렬을 위해 배열에 값을 저장함. 
    for (i=0; i<g_nTotalStaCnt; i++)	{
    	g_stSortX[i].dbX = g_stBusStation[i].ENU[0];
    	g_stSortX[i].nOrder = i+1;
    	g_stSortY[i].dbY = g_stBusStation[i].ENU[1];
    	g_stSortY[i].nOrder = i+1;
    }
	
    // 정류장 좌표를 X, Y 기준으로 각각 정렬하여 배열을 만듦.
	BubbleSortStaInfo();
	
#ifdef	_DEBUGGPS_		
		printf("g_stSortX[g_nTotalStaCnt-1].dbX = %f\n", g_stSortX[g_nTotalStaCnt-1].dbX);
		printf("g_stSortY[g_nTotalStaCnt-1].dbY = %f\n", g_stSortY[g_nTotalStaCnt-1].dbY);
#endif	 
	// 정류장 검색 반경 구하기.
	g_dbRoundX = g_stSortX[g_nTotalStaCnt-1].dbX / g_nTotalStaCnt * SEARCH_ROUND_VALUE;
	g_dbRoundY = g_stSortY[g_nTotalStaCnt-1].dbY / g_nTotalStaCnt * SEARCH_ROUND_VALUE;

	g_dbRoundX = abs(g_dbRoundX);
	g_dbRoundY = abs(g_dbRoundY);
	
#ifdef	_DEBUGGPS_		
		printf("g_dbRoundX = %f\n", g_dbRoundX);
		printf("g_dbRoundY = %f\n", g_dbRoundY);
#endif
	return 0;

}

void ThreadStartCheck(void)
{
		
	// pid 얻어서 전역변수에 값 세팅
#ifdef _GPS_27_
	gpspid = getpid();

	// 스레드 시작을 Main Process에서 임의로 했다면, 
	// 전역변수 gpsstatus에 44로 세팅되어 있다.
	// 따라서, 이 gpsstatus 값이 44 이면, 정류장 통과로그중 0번째 
	// pass_yn 에 값을 '44'로 남긴다.
	if (gpsstatus == 44)	{
		gpsstatus = 0;	// 다시 0 으로 만들어 준다.
		// 0번째 정류장 통과 로그에 '44'를 기록한다.
		memcpy(g_stGPS_INFO_STATION[0].szPass, 	  "44", 2);
	}
#else
	gpstSharedInfo->nCommGPSProcessID= getpid();

	// 스레드 시작을 Main Process에서 임의로 했다면, 
	// 전역변수 gpsstatus에 44로 세팅되어 있다.
	// 따라서, 이 gpsstatus 값이 44 이면, 정류장 통과로그중 0번째 
	// pass_yn 에 값을 '44'로 남긴다.
	if (gnGPSStatus == 44)	{
		gnGPSStatus = 0;	// 다시 0 으로 만들어 준다.
		// 0번째 정류장 통과 로그에 '44'를 기록한다.
		memcpy(g_stGPS_INFO_STATION[0].szPass, 	  "44", 2);
	}
#endif
	
}

/******************************************************************************
*                                                                             *
*    함수명 : ContinueDriveCheck()                                         	  *
*                                                                             *
*      설명 : [이전 인식 정류장 정보 로딩]									  *
*                                                                             *
*    작성자 : 김 재 의                                                        *
*                                                                             *
*    작성일 : 2005.08.01                                                      *
*                                                                             *
*  파라메터 : IN/OUT  PARAM NAME  TYPE            DESCRIPTION                 *
*             ------  ----------  --------        --------------------------- *
*			  없음.	  														  *
*    리턴값 : 정상	- 직전 인식 정류장 순서	                                  *
*			  에러	- -1		   							              	  *
*  주의사항 :                                                                 *
*                                                                             *
******************************************************************************/
int ContinueDriveCheck()
{
	int nTmpOrder = -1;
	int nContinueFlag = 0;
	FILE*	file = NULL;

//	g_bInc = FALSE;	// 기본은 1 증가 하지 않은 상태
	
	file = fopen(PREV_PASS_FILE, "rb");
	
	if (file != NULL)	{
		fread(&nTmpOrder, sizeof(int), 1, file);
		fread(&nContinueFlag, sizeof(int), 1, file);
		fread(g_szRunDepartTime, 14, 1, file);
		g_szRunDepartTime[14] = 0x00;
		fclose(file);
	}
	
	g_nContinueFlag = nContinueFlag;
	
	if ( nContinueFlag == 1)	{
		printf("\n중간에 이어서 계속 하는 경우\n");
		GpsDataLogRead();
	}
		
	if ( nTmpOrder < 1 || nTmpOrder > g_nTotalStaCnt)	{

		nTmpOrder = 1;
	}
	
	if(nTmpOrder == g_nTotalStaCnt)//2005-02-18 8:02오후
	{
		nTmpOrder = 1;
	}
	
	return nTmpOrder;
}

/*******************************************************************************
*                                                                              *
*    함수명 : ReadyWriteLogFile()                                         	   *
*                                                                              *
*      설명 : [기 존재하는 로그 파일 상태 확인 및 GPS 구조체 초기화]           *
*    작성자 : 김 재 의                                                         *
*                                                                              *
*    작성일 : 2005.08.01                                                       *
*                                                                              *
*  파라메터 : IN/OUT  PARAM NAME  TYPE            DESCRIPTION                  *
*             ------  ----------  --------        ---------------------------  *
*			  없음.	  														   *
*    리턴값 : 정상	- 0           INT             로그 파일 정상               *
*			  에러	- -1		  INT			  SIMXINFO2.TMP 파일 비정상    *
*			          -2          INT			  SIMXINFO.DAT 파일 비정상     *
*  주의사항 : 반드시 운행시작 시간이 설정된 후에 실행되어야 한다.              *
*                                                                              *
*******************************************************************************/
int ReadyWriteLogFile()
{
	FILE *gpsFile2 = NULL;	
	int n44 = 0;
	int filesize = 0;
	int i=0;

	// 2005.01.17 김재의 변경
	/////////////////////////////////////////////////////////////////////////////////// 
	// 05.01.13
	// TMP File 사이즈를 체크해서 118 Byte 의 정수배가 되지 않으면 파일을 지운다.
	filesize = getFileSize(GPS_INFO_FILE2);
	// tmp파일에 문제가 발생하였다면, tmp 파일을 삭제하고, dat 파일에 '44'로그를 기록한다.			
	if(filesize % SIMXINFO_SIZE != 0 || filesize >LOG_MAX_SIZE)
	{
		system("rm simxinfo2.tmp");
		usleep(100);
		// tmp는 지우고 dat 파일에 44를 기록한다.
		n44 = -1;
	}

	///////////////////////////////////////////////////////////
	// 05.01.17				// 김재의 추가
	// dat 파일검사
	filesize = 0;
	filesize = getFileSize(GPS_INFO_FILE1);
	if (filesize % SIMXINFO_SIZE != 0 || filesize > LOG_MAX_SIZE)	{
		printf("\n이전 파일저장 문제 발생.. 파일 삭제함...simxinfo.dat\n");
		system ("rm simxinfo.dat");
		usleep(100);
//		system ("rm simxinfo2.tmp");

		n44 = -2;
	}
	///////////////////////////////////////////////////////////
	
	//GPS 구조체 초기화, 단말기 운행시작시 한번만 초기화
	memset(&GPS,0x00,sizeof(GPS_DATA));


	// 운행로그 기록 최초 시작부분을 먼저 기록한다.
	if (g_nContinueFlag == 0)	{
		// 버스노선 ID
		memcpy(g_stGPS_INFO_STATION[0].szLineNum, g_szBusRouteID,
			sizeof(g_stGPS_INFO_STATION[0].szLineNum));
		// 버스차량 ID
		memcpy(g_stGPS_INFO_STATION[0].szBusID, g_szVehcID,
			sizeof(g_stGPS_INFO_STATION[0].szBusID));
		memcpy(g_stGPS_INFO_STATION[0].szStartTime, g_szRunDepartTime,
			sizeof(g_stGPS_INFO_STATION[0].szStartTime));
		memset(g_stGPS_INFO_STATION[0].szOrder, 	0x30, 3);	// 정류장 순서
		memset(g_stGPS_INFO_STATION[0].szID, 		0x30, 7);	// 정류장 id
		memset(g_stGPS_INFO_STATION[0].szLongitude, 0x20, 10);				// 위도
		memset(g_stGPS_INFO_STATION[0].szLatitude,  0x20, 9);					// 경도
		memset(g_stGPS_INFO_STATION[0].szPassDis,   0x20, 3);					// 통과최소거리
		memset(g_stGPS_INFO_STATION[0].szPass, 	  0x20, 2);					// 통과여부 
		// 통과시간
		memcpy(g_stGPS_INFO_STATION[0].szPassTime, g_szRunDepartTime,
			sizeof(g_stGPS_INFO_STATION[0].szPassTime));
		memset(g_stGPS_INFO_STATION[0].szSateCnt,   0x20, 2);					// 위성개수
		memset(g_stGPS_INFO_STATION[0].szIVcnt,	  0x20, 10);		
		memset(g_stGPS_INFO_STATION[0].szErrorLog,  0x30, 19);		  
		memset(g_stGPS_INFO_STATION[0].szTemp,  0x20, 4);					
		memset(g_stGPS_INFO_STATION[0].szSerialNum,  0x20, 4);
		
				
		// 단말기 메인에서 GPS 스레드를 임의로 시작했는지 여부 기록	
		if (memcmp(g_stGPS_INFO_STATION[0].szPass, 	  "44", 2) != 0)	
			memcpy(g_stGPS_INFO_STATION[0].szPass, 	  "00", 2);
	
		memset(g_stGPS_INFO_STATION[0].szIVcnt,0x20, 5);	// Invalid Cnt
		memset(&g_stGPS_INFO_STATION[0].szIVcnt[5], 0x20, 5);	// Valid Cnt	
		
		gpsFile2 = fopen(GPS_INFO_FILE2,"ab+");
		if(gpsFile2 != NULL)
		{
			fwrite(&g_stGPS_INFO_STATION[0],SIMXINFO_SIZE, 1, gpsFile2);
			fflush(gpsFile2);
			fclose(gpsFile2);
		}
	}
	
	// 0번째는 다른 정보를 저장하기 위해..비워놓고, 1번째부터 정보를 넣는다.
	for (i=1; i<=g_nTotalStaCnt; i++)	
	{
		// 버스노선 ID
		memcpy(g_stGPS_INFO_STATION[i].szLineNum, g_szBusRouteID,
			sizeof(g_stGPS_INFO_STATION[i].szLineNum));	
		// 버스차량 ID
		memcpy(g_stGPS_INFO_STATION[i].szBusID, g_szVehcID,
			sizeof(g_stGPS_INFO_STATION[i].szBusID));
		memcpy(g_stGPS_INFO_STATION[i].szStartTime, g_szRunDepartTime,
			sizeof(g_stGPS_INFO_STATION[i].szStartTime));
		// 정류장 순서
		memcpy(g_stGPS_INFO_STATION[i].szOrder, g_stStaInfo[i-1].bStationOrder,
			sizeof(g_stGPS_INFO_STATION[i].szOrder));
		// 정류장 id
		memcpy(g_stGPS_INFO_STATION[i].szID, g_stStaInfo[i-1].abStationID,
			sizeof(g_stGPS_INFO_STATION[i].szID));
		memset(g_stGPS_INFO_STATION[i].szLongitude, 0x20, 10);			// 위도	
		memset(g_stGPS_INFO_STATION[i].szLatitude,  0x20, 9);				// 경도	
		memset(g_stGPS_INFO_STATION[i].szPassDis,   0x20, 3);				// 통과최소거리
		memset(g_stGPS_INFO_STATION[i].szPass, 	  0x20, 2);				// 통과여부
		memset(g_stGPS_INFO_STATION[i].szPassTime,  0x20, 14);	// 통과시간
		memset(g_stGPS_INFO_STATION[i].szSateCnt,   0x20, 2);				// 위성개수
		memset(g_stGPS_INFO_STATION[i].szIVcnt,	  0x20, 10);		
		memset(g_stGPS_INFO_STATION[i].szErrorLog,  0x30, 19);		   
		memset(g_stGPS_INFO_STATION[i].szTemp,  0x20, 4);					
		memset(g_stGPS_INFO_STATION[i].szSerialNum,  0x20, 4);
	}
		
	return n44;
}


/******************************************************************************
*                                                                             *
*    함수명 : SyncDeparttimeWithMainLogic()                                   *
*                                                                             *
*      설명 : [단말기 메인의 운행시작시간과 운행로그의 운행시작시간을         *
               동기화 시킴]                                                   *
*                                                                             *
*    작성자 : 김 재 의                                                        *
*                                                                             *
*    작성일 : 2005.08.01                                                      *
*                                                                             *
*  파라메터 : IN/OUT  PARAM NAME  		TYPE      DESCRIPTION                 *
*             ------  ----------     	--------  --------------------------- *
*             IN      szStartLogTime    char*     운행로그 운행시작 시간      *
*    리턴값 : 없음.						                                      *
*  주의사항 :                                                                 *
*                                                                             *
******************************************************************************/
void SyncDeparttimeWithMainLogic(char* szStartLogTime)
{
	int nLoopCnt = 0;
	time_t tNowDtime = 0;

	
#ifdef _GPS_27_
	rtc_gettime(szStartLogTime);
#else
	GetRTCTime(&tNowDtime);
	TimeT2ASCDtime(tNowDtime, szStartLogTime);
#endif	
	szStartLogTime[14] = 0x00;
	
	nLoopCnt = 0;
	while(1)// 운행 시작 시간 데이터가 들어올때까지 
	{
#ifdef _GPS_27_
		//driver flag는 단말기쪽 전역 변수 , 값이 1이면 시작 시간이 없는상태 
		if(driveFlag == '0')  
		{
			strncpy(szStartLogTime,TRAN_TH.run_start_date_time, 14);
#else
		// TODO : if문의 조건이 '운행중이면'이 맞는지 확인 필요
		if (gpstSharedInfo->boolIsDriveNow)  
		{
			TimeT2ASCDtime(gtDriveStartDtime, szStartLogTime);
#endif		
			szStartLogTime[14] = 0x00;
			printf("[2] TRAN_TH.run_start_date_time = %s \n", szStartLogTime);
			nLoopCnt = 0;
			break;
		}
		else
		{
			usleep(100);
		}
		nLoopCnt++;
		if (nLoopCnt == 3)	{
#ifdef _GPS_27_			
			rtc_gettime(szStartLogTime);
#else
			GetRTCTime(&tNowDtime);
			TimeT2ASCDtime(tNowDtime, szStartLogTime);
#endif
			szStartLogTime[14] = 0x00;
			
			nLoopCnt = 0;
			break;
		}
	}

}


/******************************************************************************
*                                                                             *
*    함수명 : GetGpsData()                                                    *
*                                                                             *
*      설명 : [gps 시리얼 포트 닫기]										  *
*                                                                             *
*    작성자 : 김 재 의                                                        *
*                                                                             *
*    작성일 : 2005.05.30                                                      *
*                                                                             *
*  파라메터 : IN/OUT  PARAM NAME  TYPE      DESCRIPTION                       *
*             ------  ----------  --------  --------------------------------- *
*             IN      fd          int       시리얼 포트 핸들번호              *
*			  IN/OUT  strGGA	  char*		GPGGA 데이타용 버퍼				  *
*			  IN/OUT  strRMC	  char*		GPRMC 데이타용 버퍼				  *
*    리턴값 : 0		- 정상												      *
*			  0이하	- gps 데이타 수신중 에러 발생							  *
*  주의사항 :                                                                 *
*                                                                             *
******************************************************************************/
int GetGpsData(int fd, char* strGGA, char* strRMC)
{
	int ret = 0;
	int	bFindGGA = 0;
	int	bFindRMC = 0;
	char strGpsdata[LINE_MAX_BUFFER];
		
	while (1)	{
#ifdef	_SIMMODE_
  	    char* recv = NULL;
		if (s_SimFile != NULL)	{
			recv = fgets(strGpsdata, LINE_MAX_BUFFER, s_SimFile);
		}
		if (recv == NULL)	ret = -1;
		else				ret = 1;
#else		
		ret = GpsRecv(fd, strGpsdata, 2);
		if (ret < 0)	{
			//printf("\nGpsRecv func error!!\n");
			break;
		}
#endif

		//printf("%s",strGpsdata);
		if (strncmp(strGpsdata, "$GPGGA", 6) == 0)	{
			bFindGGA = 1;
			strcpy(strGGA, strGpsdata);
		}
		if (strncmp(strGpsdata, "$GPRMC", 6) == 0)	{
			bFindRMC = 1;
			strcpy(strRMC, strGpsdata);
		}
		
		if (bFindGGA && bFindRMC)
			break;
	}
	
	return ret;
}

/******************************************************************************
*                                                                             *
*    함수명 : WriteTrace()                                                    *
*                                                                             *
*      설명 : [GPS 궤적 파일 기록]										      *
*                                                                             *
*    작성자 : 김 재 의                                                        *
*                                                                             *
*    작성일 : 2005.05.30                                                      *
*                                                                             *
*  파라메터 : IN/OUT  PARAM NAME  TYPE      DESCRIPTION                       *
*             ------  ----------  --------  --------------------------------- *
*             IN      rmc         RMC_DATA* RMC 구조체 포인터                 *
*    리턴값 : 없음.							 								  *
*  주의사항 :                                                                 *
*                                                                             *
******************************************************************************/
void WriteTrace(RMC_DATA* rmc)
{
	unsigned int temp1 =0;
	double temp2=0;
	float temp3=0;
	unsigned char temp4 =0;
	
	FILE* file = NULL;
	char temp[LINE_MAX_BUFFER];
	
	long filesize = 0L;

	
//	static unsigned int init =0;
	
	memset(temp, 0x00, LINE_MAX_BUFFER);
/** // 껐다 켰을 경우에도 사이즈 검사해서 로그기록 함
	
	//단말기를 껐다 켰을 경우 이 값은 0, 즉 단말기 껐다 켰을경우 다시 처음부터 파일을 쓰도록 함 
	if(init == 0)
	{
		file = fopen(LOG_INFO_FILE, "w+");
		memcpy(temp, g_szVehcID, 9);		// GPS 2.7 적용 운행궤적 파일에 차량 id  추가 2005-06-09 5:24오후
		UsrFwrite(temp, 9, 1, file);
		init = 1;
	}
	else 
	{
**/		
		filesize = getFileSize(LOG_INFO_FILE);
		if (filesize == 0L || filesize > MaxFileSize)
		{
			file = fopen(LOG_INFO_FILE, "w+");
			memcpy(temp, g_szVehcID, 9);	// GPS 2.7 적용 운행궤적 파일에 차량 id  추가 2005-06-09 5:24오후
			UsrFwrite(temp, 9, 1, file);
		}
		else
		{
			file = fopen(LOG_INFO_FILE, "a+");
		}
/**		
	}// if.. end.. 
**/		
	if( file != NULL )		
	{
		//utc time store
		temp1 = (unsigned int)(rmc->UTCTime);	
		UsrFwrite(&temp1,sizeof(temp1),1,file);
		
		//lat lon
		temp2 = rmc->Lat;
		UsrFwrite(&temp2,sizeof(temp2),1,file);
		temp2 = rmc->Lon;
		UsrFwrite(&temp2,sizeof(temp2),1,file);
		
		//angle
		temp3 = (float)(rmc->TrackTrue);
		UsrFwrite(&temp3,sizeof(temp3),1,file);
		//velocity
		temp3 = (float)(rmc->SpeedKn);
		UsrFwrite(&temp3,sizeof(temp3),1,file);
		
		//valid
		temp4 = (unsigned char)(rmc->PosStatus);
		UsrFwrite(&temp4,sizeof(temp4),1,file);

		
		fflush(file);
		fclose(file);	
	}
		
}

/******************************************************************************
*                                                                             *
*    함수명 : GpsRecv()                                                       *
*                                                                             *
*      설명 : [gps 데이타 수신]									              *
*                                                                             *
*    작성자 : 김 재 의                                                        *
*                                                                             *
*    작성일 : 2005.05.24                                                      *
*                                                                             *
*  파라메터 : IN/OUT  PARAM NAME  TYPE      DESCRIPTION                       *
*             ------  ----------  --------  --------------------------------- *
*             IN      fd          int       시리얼 포트 핸들번호              *
*             IN	  buffer   	  char*		gps 수신 데이타                   *
*		      IN	  timeout	  int		데이타 수신 timeout 시간(초)  	  *
*    리턴값 : 0,1	- 정상									                  *
*            -1  	- select 함수 timeout 			  				          *
* 			 -2		- select 함수 에러              	  					  *
*			 -3		- read 함수 에러										  *
*			 -4	    - 첫번째 파라미터 값 오류								  *
*			 -5		- 두번째 파라미터 값 오류								  *
*  주의사항 :                                                                 *
*                                                                             *
******************************************************************************/
int GpsRecv(int fd, unsigned char* buffer, int timeout)
{
	fd_set sockSet;
	struct timeval timeoutVal;
	int nbyte = 0;
	
	if (fd < 0)	{
		printf("\n GpsRecv() 첫번째 파라미터 포트 핸들 값 오류 !!\n");
		return -4;
	}
	
	if (buffer == NULL)	{
		printf("\n GpsRecv() 두번째 파라미터 값이 null 임 !!\n");
		return -5;
	}
	FD_ZERO(&sockSet);
	FD_SET(fd, &sockSet);
	
	timeoutVal.tv_sec = timeout;
	timeoutVal.tv_usec = 0;	

	switch (select(fd + 1, &sockSet, NULL, NULL,  &timeoutVal  ))
	{
		case -1 :	// Select 함수 에러
			printf("\nSelect 함수 에러 ....\n");
			return -1;
		case 0 :	// Timeout 발생
			return -2;
	}	
	
	nbyte = read(fd, buffer, LINE_MAX_BUFFER);
	
	if (nbyte == 0)
	{
		printf("\n포트에서 읽은 데이타 없음.....\n");
		return 0;
	}
	
	if (nbyte < 0)
	{
		printf("\n포트 읽기 에러.....\n");
		return -5;
	}
	
	buffer[nbyte] = 0x00;	
	
	return 1;
}

/******************************************************************************
*                                                                             *
*    함수명 : ParseRMC()              	                                      *
*                                                                             *
*      설명 : [GPRMC 데이타 파싱]                                             *
*                                                                             *
*    작성자 : 김 재 의                                                        *
*                                                                             *
*    작성일 : 2005.05.30                                                      *
*                                                                             *
*  파라메터 : IN/OUT  PARAM NAME  TYPE            DESCRIPTION                 *
*             ------  ----------  --------        --------------------------- *
*             IN/OUT  rmc         RMC_DATA*       GPRMC 데이타 구조체용 변수  *
*			  IN	  Data  	  unsigned char*  GPRMC 데이타 전체			  *
*			  IN	  DataNum	  int			  GPRMC 데이타 길이			  *
*    리턴값 : 0	: 비활성 데이타						                          *
*			  1 : 활성 데이타												  *
*			 -1 : CheckSum 에러												  *
*  주의사항 :                                                                 *
*                                                                             *
******************************************************************************/
int ParseRMC(RMC_DATA* rmc, unsigned char *Data, int DataNum)
{
	char buffer[LINE_MAX_BUFFER] ={0};
	int i=0,j=0;
	int temp[100];
	int ret = 0;
	
	memcpy(buffer,Data,DataNum);  //데이터를 복사 한다. 

	if (CheckSum(Data) == 1)	// CheckSum Err
		return -1;
	
	//콤마 위치를 저장 한다. 
	while(buffer[j]!='*')
	{
		if(buffer[j]==0x2c){
			temp[i]=j;
			i++;
		}
		j++;		
	}

	for (j=0; j<i; j++)	{	
		switch (j)	{
			case 0:
				sscanf((buffer+temp[0]+1),"%lf",&rmc->UTCTime);
				memcpy(GPS.utc_world_time,(buffer+temp[0]+1),10);
				break;
			case 1:
				sscanf((buffer+temp[1]+1),"%c",&rmc->PosStatus);
				memcpy(GPS.use_kind,(buffer+temp[1]+1),1);
				GPS.use_kind[1] = 0x00;
				break;
			case 2:
				sscanf((buffer+temp[2]+1),"%lf",&rmc->Lat);
				memcpy(g_szPassMinLatitude, (buffer + temp[2] + 1),
					sizeof(g_szPassMinLatitude));
				memcpy(GPS.lattitude,(buffer+temp[2]+1),9);
				break;
			case 3:
				sscanf((buffer+temp[3]+1),"%c",&rmc->LatDir);
				memcpy(GPS.north_south,(buffer+temp[3]+1),1);
				break;
			case 4:
				sscanf((buffer+temp[4]+1),"%lf",&rmc->Lon);
				memcpy(g_szPassMinLongitude, (buffer + temp[4] + 1),
					sizeof(g_szPassMinLongitude));
				memcpy(GPS.longitude,(buffer+temp[4]+1),10);
				break;
			case 5:
				sscanf((buffer+temp[5]+1),"%c",&rmc->LonDir);
				memcpy(GPS.east_west,(buffer+temp[5]+1),1);	
				break;
			case 6:
				sscanf((buffer+temp[6]+1),"%lf",&rmc->SpeedKn);
				break;
			case 7:
				sscanf((buffer+temp[7]+1),"%lf",&rmc->TrackTrue);
				break;
			case 8:
				sscanf((buffer+temp[8]+1),"%lf",&rmc->date);
				memcpy(GPS.date,(buffer+temp[8]+1),6);
				break;
		}
	}
	
	// GPS 2.7 적용. 현재 GPS 모듈에서 전달 받은 시간을 보관한다. 2005-06-08 5:07오후
	memcpy(g_szGpsCurrTime,"20",2);
	memcpy(&g_szGpsCurrTime[2],&GPS.date[4],2);
	memcpy(&g_szGpsCurrTime[4],&GPS.date[2],2);
	memcpy(&g_szGpsCurrTime[6],&GPS.date[0],2);
	memcpy(&g_szGpsCurrTime[8], GPS.utc_world_time, 6);
	gpstime_convert(g_szGpsCurrTime);
	g_szGpsCurrTime[14] = 0x00;
	
	if (strcmp(GPS.use_kind, "A") == 0)	ret = 1;
    
    return ret;
}

/******************************************************************************
*                                                                             *
*    함수명 : ParseGGA()              	                                      *
*                                                                             *
*      설명 : [GPGGA 데이타 파싱]											  *
*                                                                             *
*    작성자 : 김 재 의                                                        *
*                                                                             *
*    작성일 : 2005.05.30                                                      *
*                                                                             *
*  파라메터 : IN/OUT  PARAM NAME  TYPE            DESCRIPTION                 *
*             ------  ----------  --------        --------------------------- *
*             IN/OUT  gga         GGA_DATA*       GPGGA 데이타 구조체용 변수  *
*			  IN	  Data  	  unsigned char*  GPGGA 데이타 전체			  *
*			  IN	  DataNum	  int			  GPGGA 데이타 길이			  *
*    리턴값 : 0	: 현재는 의미 없음.				                  			  *
*  주의사항 :                                                                 *
*                                                                             *
******************************************************************************/
int ParseGGA(GGA_DATA* gga, unsigned char *Data, int DataNum)
{
	char buffer[LINE_MAX_BUFFER] ={0};
	int i=0;
	int j=0;
	int temp[100];
	
	
	memcpy(buffer,Data,DataNum);  //데이터를 복사 한다. 

	//콤마 위치를 저장 한다. 
	while(buffer[j]!='*')
	{
		if(buffer[j]==0x2c){
			temp[i]=j;
			i++;
		}
		j++;		
	}

	//이전 프로그램에서는 ,가 연속으로 나오는 경우를 대비하여 처리하였다.
	//그러나 sscanf자체에서 숫자가 아닌 경우 자동으로 처리를 안하기 때문에 문제가 안생김 
	sscanf((buffer+temp[0]+1),"%lf",&gga->UTCTime);

	sscanf((buffer+temp[1]+1),"%lf",&gga->Lat);

	sscanf((buffer+temp[2]+1),"%c",&gga->LatDir);

	sscanf((buffer+temp[3]+1),"%lf",&gga->Lon);

	sscanf((buffer+temp[4]+1),"%c",&gga->LonDir);

	sscanf((buffer+temp[5]+1),"%d",&gga->GPSQual);

	sscanf((buffer+temp[6]+1),"%d",&gga->SatNum);
	
	memcpy(g_szPassSatelliteCnt,(buffer+temp[6]+1),2);

	sscanf((buffer+temp[7]+1),"%lf",&gga->HDOP);

	sscanf((buffer+temp[8]+1),"%lf",&gga->Altitude);

    return 0;	
}

/******************************************************************************
*                                                                             *
*    함수명 : CheckSum()                                                  	  *
*                                                                             *
*      설명 : [gps 시리얼 포트 닫기]										  *
*                                                                             *
*    작성자 : 김 재 의                                                        *
*                                                                             *
*    작성일 : 2005.05.30                                                      *
*                                                                             *
*  파라메터 : IN/OUT  PARAM NAME  TYPE            DESCRIPTION                 *
*             ------  ----------  --------        --------------------------- *
*			  IN	  Data		  unsigned char*	GPRMC 데이타 전체		  *
*    리턴값 : 정상	- 0				   						                  *
*			  에러	- 1											              *
*  주의사항 :                                                                 *
*                                                                             *
******************************************************************************/
int CheckSum(unsigned char *Data)
{
	char* lpEnd;
	char tmp[3];
	int i;
	DWORD dwCheckSum = 0, dwSentenceCheckSum;

	for (i=1; i<strlen(Data) && Data[i] != '*'; i++)
		dwCheckSum ^= Data[i];
	
	memcpy(tmp, &Data[i+1], 2);	tmp[2] = 0x00;
	dwSentenceCheckSum = strtoul(tmp, &lpEnd, 16);
	if (dwCheckSum != dwSentenceCheckSum)	{
		return 1;
	}
	
	return 0;
}

// 쓰레드 종료시 호출될 함수 
void clean_up(void *arg)
{
	FILE *gpsFile = NULL;
	FILE *gpsFile2 = NULL;
	unsigned int i=0;
	unsigned char strbuff[32]={0};
	double successRate = 0;				// 인식률
	unsigned int total_cnt=0;			// '01'로 인식한정류장 + '88' + '99' + '55' + '11' 포함
	unsigned int tmpTotal=0;			// 에러로그 파일 라인수 카운트
	unsigned int cnt_01=0;				// '01'로 인식한정류장 수
	unsigned int cnt_22=0;				// 기점외 출발한 정류장
	unsigned int cnt_33=0;				// 종점전에 종료한 정류장
	unsigned int cnt_00=0;				// '00' 갯수 .. 에러로그 구조체의 첫번째 배열
	unsigned int cnt_66=0;				// 노선이탈한 정류장
	unsigned int cnt_space=0;			// 알수없는 경우의 공백
	unsigned int cur_order=0;			// 현재 정류장 order
	unsigned char Order_buff[3]={0};
	unsigned int Total_Dist=0;			// 운행거리
	unsigned char buffer[SIMXINFO_SIZE]={0};
	unsigned int filesize =0;
	unsigned int loopCnt =0;			// 에러로그 파일의 라인수 카운트
	unsigned int turn_cnt =0;			// 운행횟수
	unsigned int Order_check =0;		// 시리얼 번호 생성시 현재 정류장과 마지막 정류장 order값 비교
	unsigned int SN_num =0;				// 시리얼 번호
//	unsigned int clean_ck_flag =0;		// 시리얼 번호 생성시 마지막 정류장 order값 검지 flag
//	unsigned int reset_flag =0;			// 시리얼 번호 reset flag
//	unsigned char szStartTimebuff[14] ={0};
	unsigned int remainder=0;			// 파일 사이즈의 118 byte(한개의 레코드 길이) 정수배 검사
	
	//2005.02.17 김재의 추가
	unsigned char pass = 0;	// 거리계산용 변수

	//
	unsigned int GPSDataCnt = 0;  //마지막 인식한 정류장과 현재 인식한 정류장사이의 총 Data count를 저장함 2005-02-18 4:55오후
	unsigned int InvalidCnt =0; // 마지막 인식한 정류장과 현재 인식한 정류장 사이의 invalid count 2005-02-18 4:55오후
	unsigned int AvailCnt =0; //마지막 인식한 정류장과 현재 인식한 정류장 사이의 Avail count 2005-02-18 4:55오후
	unsigned int TimeOutCnt =0; //마지막 인식한 정류장과 현재 인식한 정류장 사이의 Timeout count 2005-02-18 4:55오후
	double AvailErrorPerc =0;   // 총 데이터 개수 중에 valid count의 퍼센트 2005-02-18 4:55오후
	
	int i_order = 0;
	time_t tNowDtime = 0;
	
	InvalidCnt = g_nInvalidCnt - g_nPrevInvalidCnt; 
	AvailCnt = g_nAvailCnt - g_nPrevAvailCnt;
	TimeOutCnt =(unsigned int)((g_nTimeOutCnt - g_nPrevTimeOutCnt)*2);
	GPSDataCnt = InvalidCnt + AvailCnt +TimeOutCnt; //역간 전체 데이터 개수(check sum error은 고려 하지 않음)
	
	if(GPSDataCnt == 0)
		GPSDataCnt++;
		
	AvailErrorPerc = ((double)(AvailCnt) / (double)(GPSDataCnt)) *100.0;

	if (AvailErrorPerc < 1)	{
		// 이도저도 아닌경우 30%의 비율 값으로 세팅한다.
		// 아래 if(AvailErrorPerc < AvailCntTh) 이 부분의 소스로 인해
		// 30% 보다 작으면 기본으로 로그를 '33'으로 기록하게 되어 있음.
		// 따라서 InvalidCnt , TimeOutCnt 값이 필요가 없음.
		AvailErrorPerc = AVAIL_CNT_TH;
	}
	
	printf("\n InvalidCnt:%d, AvailCnt:%d, TimeOutCnt:%d, GPSDataCnt:%d \n",
			InvalidCnt, AvailCnt, TimeOutCnt, GPSDataCnt);
	
	// 위에서 계산된 AvailErrorPerc 값이 기준값인 전체 카운트 대비 30%가 안되면,
	// Invaild 비율과 TimeOut 비율을 비교하여 로그를 기록한다.

	
	memset(strbuff, 0x00, 32);

#ifdef _GPS_27_
	if (gpslogwritecheck == 1) {
#else
	if (gboolIsRunningGPSThread) {
#endif
		printf("\r\n GPS log write 없이 종료\r\n");
		close(g_gps_fd);
		return;
	}
	
	// 로그 맨 마지막에 기록하는 한 레코드를 위한 변수값 초기화 하는 것 같음.	
	memset(&st_GPS_FINAL_INFO,0x20,SIMXINFO_SIZE);
	/////////////////////////////////종료시 마지막 필드 값 ///////////////////////////////////////
	memset(strbuff, 0x00, 32);	
	sprintf(strbuff, "%5d", g_nInvalidCnt);
	memcpy(st_GPS_FINAL_INFO.szIVcnt, strbuff, strlen(strbuff));
	
	memset(strbuff, 0x00, 32); 
	sprintf(strbuff, "%5d", g_nAvailCnt);
	memcpy(&(st_GPS_FINAL_INFO.szIVcnt[5]), strbuff, 5);
	// 버스노선 ID
	memcpy(st_GPS_FINAL_INFO.szLineNum, g_szBusRouteID, sizeof(st_GPS_FINAL_INFO.szLineNum));
	memcpy(st_GPS_FINAL_INFO.szBusID, g_szVehcID, 9);				// 버스차량 ID
	memset(strbuff, 0x00, 32);
	sprintf(strbuff, "%03d",g_nTotalStaCnt+1);								// 정류장 총수 보다 하나 큰값을 씀
	memcpy(st_GPS_FINAL_INFO.szOrder, 	strbuff, 3);						// 정류장 순서
	memcpy(st_GPS_FINAL_INFO.szID, 		"0000000", 7);						// 정류장 id
	memset(strbuff,0x00,32);
	sprintf(strbuff,"%05d%05d",g_nTimeOutCnt,g_nCheckSumCnt);
	memcpy(st_GPS_FINAL_INFO.szLongitude,strbuff,10);
	memset(st_GPS_FINAL_INFO.szLatitude,  0x20, 9);							// 경도	
	memset(st_GPS_FINAL_INFO.szPassDis,	  0x20, 3);							// 통과최소거리 
	memset(st_GPS_FINAL_INFO.szErrorLog,  0x20, 19);	
	memset(st_GPS_FINAL_INFO.szSateCnt,   0x20, 2);					
	memset(st_GPS_FINAL_INFO.szTemp,  0x20, 4);						
	memset(st_GPS_FINAL_INFO.szSerialNum,  0x20, 4);	
	//종료한 시간을 넣는다
	memset(strbuff, 0x00, 32); 	
#ifdef _GPS_27_			
	rtc_gettime(strbuff);
#else
	GetRTCTime(&tNowDtime);
	TimeT2ASCDtime(tNowDtime, strbuff);
#endif
	memcpy(st_GPS_FINAL_INFO.szPassTime,strbuff, 14);  		//마지막 종료 시간을 넣는다. 
	
	// 2005.01.14 김재의 변경
	/////////////////////////////////////////////////////////////////////////////////// 
	// 05.01.13
	// File 사이즈를 체크해서 118 Byte 의 정수배가 되지 않으면 파일을 지운다.
	filesize = 0;
	filesize = getFileSize(GPS_INFO_FILE2);
	remainder = filesize % SIMXINFO_SIZE;

	// tmp파일에 문제가 발생하였다면, tmp 파일을 삭제하고, dat 파일에 '44'로그를 기록한다.			
	if(remainder != 0 || filesize >LOG_MAX_SIZE)
	{
		remainder =0;
		system("rm simxinfo2.tmp");
		usleep(100);
		
		// tmp는 지우고 dat 파일에 44를 기록한다.
		gpsFile = fopen(GPS_INFO_FILE1, "ab+");
		if(gpsFile != NULL)
		{
			memcpy(&g_stGPS_INFO_STATION[0].szTemp[2], "44", 2);
			memcpy(&st_GPS_FINAL_INFO.szTemp[2], "44", 2);
			fwrite(&g_stGPS_INFO_STATION[0], SIMXINFO_SIZE, 1, gpsFile);
			fwrite(&st_GPS_FINAL_INFO, SIMXINFO_SIZE, 1, gpsFile);
			fflush(gpsFile);
			fclose(gpsFile);
		}
		close(g_gps_fd);
		return;
	}

	filesize = 0;
	filesize = getFileSize(GPS_INFO_FILE1);
	remainder = filesize % SIMXINFO_SIZE;
	// 2005.01.14 김재의 추가
	// 위 tmp 파일은 이상이 없는데, dat 파일에 문제가 있으면, 기존 dat 파일은 삭제하고, 새로운 dat 파일에 '44'로그를 기록한다.
	if(remainder != 0 || filesize > LOG_MAX_SIZE)
	{
		remainder =0;
		system("rm simxinfo.dat");
		usleep(100);
		gpsFile = fopen(GPS_INFO_FILE1, "ab+");
		if(gpsFile != NULL)
		{
			memcpy(&g_stGPS_INFO_STATION[0].szTemp[2], "44", 2);
			memcpy(&st_GPS_FINAL_INFO.szTemp[2], "44", 2);
			fwrite(&g_stGPS_INFO_STATION[0], SIMXINFO_SIZE, 1, gpsFile);
			fwrite(&st_GPS_FINAL_INFO, SIMXINFO_SIZE, 1, gpsFile);
			fflush(gpsFile);
			fclose(gpsFile);
		}
	}
	
	
	//운행 거리 계산을 위한 구조체 초기화 
//	memset(g_accum_dist,0x00,sizeof(ACCUM_DIST_DATA)*1000);

	// GPS 수신 데이타 단계별 카운터 로그파일에 기록	
	sprintf(strbuff, "%d", g_nInvalidCnt);
	memcpy(g_stGPS_INFO_STATION[0].szIVcnt, strbuff, 5);	
	
	memset(strbuff, 0x00, 32); 
	sprintf(strbuff, "%d", g_nAvailCnt);
	memcpy(&(g_stGPS_INFO_STATION[0].szIVcnt[5]), strbuff, 5);	

	memset(strbuff, 0x00, 32); 
	sprintf(strbuff, "%d", g_nTimeOutCnt);
	memcpy((g_stGPS_INFO_STATION[0].szErrorLog), strbuff, 19);

	SaveCurrentStation(g_nCurStaOrder, 0);
	printf("마지막 통과한 정류장 기록\n");
	
	// 인식률 전역변수	
#ifdef _GPS_27_
	gpsrate = 0;	// 우선 0%으로 초기화
#else
	gwGPSRecvRate = 0;
#endif


	//////////////////////////운행 종료시 관련 정보 파일에 씀///////////////////////////////////////
	//tmp 파일의 첫번째 레코드에 기록한 운행시작시간을 읽어온다. 
	filesize = 0;
//	gpsFile2 = NULL;
	
//	gpsFile2 = fopen(GPS_INFO_FILE2, "rb");
//	if(gpsFile2 != NULL)
//	{
//		fread(buffer,SIMXINFO_SIZE,1,gpsFile2);
//		memcpy(szStartTimebuff,&buffer[17],14);
//		memcpy(st_GPS_FINAL_INFO.szStartTime, 	szStartTimebuff, 14);  //마지막 필드 시작 시간값을 셋팅 한다.
//		fclose(gpsFile2);
//	}

	memcpy(st_GPS_FINAL_INFO.szStartTime, 	g_szRunDepartTime, 14);  //마지막 필드 시작 시간값을 셋팅 한다.
	
	
	i_order = g_nCurStaOrder;
		
	gpsFile = gpsFile2 = NULL;
	gpsFile2 = fopen(GPS_INFO_FILE2, "ab+");
	
	if (gpsFile2 != NULL) 
	{
		/////////////////////////종료한후 종점까지 정류장을 3으로 채워서 파일에 쓴다. 
		
//		if(InitStationIDFixFlag == 1)
//		{
			if(i_order != 1 && i_order != 2)
			{
				// 현재 order 이후의 에러 로그를 33 으로 Set한다.
				for(i = i_order + 1 ; i <= g_nTotalStaCnt; i++)
				{
					if(AvailErrorPerc < AVAIL_CNT_TH)
					{
						if(InvalidCnt > TimeOutCnt)
						{
							memcpy(g_stGPS_INFO_STATION[i].szPass, "88", 2);
							memcpy(g_stGPS_INFO_STATION[i].szTemp, "88", 2);								
						}
						else						
						{
							memcpy(g_stGPS_INFO_STATION[i].szPass, "99", 2);
							memcpy(g_stGPS_INFO_STATION[i].szTemp, "99", 2);								
						}
						
					}
					else
					{
						memcpy(g_stGPS_INFO_STATION[i].szPass, "33", 2);
						memcpy(g_stGPS_INFO_STATION[i].szTemp, "33", 2);						
					}
					
					memset(g_stGPS_INFO_STATION[i].szLongitude,0x20,10);
					memset(g_stGPS_INFO_STATION[i].szLatitude,  0x20, 9);	
					memset(g_stGPS_INFO_STATION[i].szPassTime, 0x20, 14);
					memset(g_stGPS_INFO_STATION[i].szPassDis, 0x20, 3);						
					memset(g_stGPS_INFO_STATION[i].szIVcnt, 0x20, 10);
					memset(g_stGPS_INFO_STATION[i].szSateCnt,   0x20, 2);
					memcpy(g_stGPS_INFO_STATION[i].szStartTime, g_szRunDepartTime, 14);
					
					if(i_order !=0)						
					{
						fwrite(&g_stGPS_INFO_STATION[i], SIMXINFO_SIZE, 1, gpsFile2);
						
					}// if.. end...
				}// for.. end...
			}
//		}

		fflush(gpsFile2);
		fclose(gpsFile2);

	}
	else
	{
		
	}


	/*인식률 , 누적 거리를 계산 한다.                 */
	//로그 tmp 파일 사이즈를 계산 한다. 
	filesize = 0;
	filesize = getFileSize(GPS_INFO_FILE2);
	loopCnt = filesize / SIMXINFO_SIZE; 				//전체 로그 루프 카운트 계산 ..로그파일 라인수 계산

	gpsFile = gpsFile2 = NULL;
	gpsFile2 = fopen(GPS_INFO_FILE2, "rb");
	//gpsFile = fopen(GPS_INFO_FILE1, "rb");

	
	if(gpsFile2 != NULL)
	{
		fread(buffer, SIMXINFO_SIZE, 1, gpsFile2);  	//첫번째 필드값은 시작값이므로 제외 시킨다.
		
			
		//두번째 필드부터 인식률과 누적거리를 계산 한다. 
		for(i =1; i < loopCnt ; i++)
		{
												// 로그파일 라인수
			fread(buffer, SIMXINFO_SIZE, 1, gpsFile2);
			memcpy(Order_buff, &buffer[31], 3);			// order 값 저장
			cur_order = atointeger(Order_buff, 3);		// order 값 정수화

			if(cur_order < 1 || cur_order > g_nTotalStaCnt)	// 현재 order가 1보다 작거나 전체 정류장수 보다 클때
			{
				continue;
			}
			
			tmpTotal++;
//			g_accum_dist[tmpTotal-1].order = cur_order;		// 통과한 정류장 order를 저장
			
			// szPass 검사
			if( memcmp(&buffer[67], "01", 2) == 0 )		
			{
				cnt_01++;
//				g_accum_dist[tmpTotal-1].pass = 1;
				  pass = 1;
			}
			else if( memcmp(&buffer[67], "22", 2) == 0 )
			{
				cnt_22++;
//				g_accum_dist[tmpTotal-1].pass = 0;
				  pass = 0;
			}
			else if( memcmp(&buffer[67], "00", 2) == 0 )
			{
				cnt_00++;
//				g_accum_dist[tmpTotal-1].pass = 0;	
				  pass = 0;		
			}
			else if( memcmp(&buffer[67], "33", 2) == 0 )
			{
				cnt_33++;
//				g_accum_dist[tmpTotal-1].pass = 0;
				  pass = 0;
			}
			else if( memcmp(&buffer[67], "66", 2) == 0 )
			{
				cnt_66++;
//				g_accum_dist[tmpTotal-1].pass = 0;
				  pass = 0;
			}
			else if( memcmp(&buffer[67], "55", 2) == 0 )
			{
//				g_accum_dist[tmpTotal-1].pass = 1;
				  pass = 1;
			}
			else if( memcmp(&buffer[67], "88", 2) == 0 )
			{
//				g_accum_dist[tmpTotal-1].pass = 1;
				  pass = 1;
			}
			else if( memcmp(&buffer[67], "99", 2) == 0 )
			{
//				g_accum_dist[tmpTotal-1].pass = 1;
				  pass = 1;
			}
			else if( memcmp(&buffer[67], "11", 2) == 0 )
			{
//				g_accum_dist[tmpTotal-1].pass = 1;
				  pass = 1;
			}
			else if( memcmp(&buffer[67], "  ", 2) == 0 )
			{
				cnt_space++;
//				g_accum_dist[tmpTotal-1].pass = 0;
				  pass = 0;
			}
			else
			{
				pass =0;  //2005-02-18 4:20오후
			}
						
//			Total_Dist += (int)((sBus_station[cur_order-1].DiffDist2)*(g_accum_dist[tmpTotal-1].pass));
			Total_Dist += (int)((g_stBusStation[cur_order-1].DiffDist2)*(pass));
			pass =0;  //2005-02-18 4:20오후
				
		}// for..end...
		
		printf("\ntmpTotal = %d, cnt_00 = %d, cnt_33 = %d, cnt_22 = %d, cnt_66 = %d, cnt_space = %d\n"
		        ,tmpTotal,cnt_00,cnt_33,cnt_22,cnt_66,cnt_space);
		printf("cnt_01 = %d \n", cnt_01);
		total_cnt = tmpTotal-cnt_00-cnt_33-cnt_22-cnt_66-cnt_space; 
				
		// 비 정상 운행 정류장 총 수 전역변수
		g_nGpsExceptCnt = cnt_33+cnt_22+cnt_66;		
		if(total_cnt !=0)
		{
			// 운행횟수 계산
			successRate = (double)(cnt_01*100) / (double)(total_cnt); // 
//			turn_cnt =(int)( ((double)(loopCnt-cnt_33-cnt_00-cnt_space)/(double)nStation_cnt ) + 0.4);
			turn_cnt =(int)( ((double)(loopCnt-cnt_33-cnt_22-cnt_66-cnt_00-cnt_space)/(double)g_nTotalStaCnt ) + 0.4);
		}
		else
		{
			successRate =0;
			turn_cnt =0;
		}
		gwDriveCnt = turn_cnt;

		gwGPSRecvRate = (word)(successRate);

		sprintf(&st_GPS_FINAL_INFO.szErrorLog[0], "%03d", gwGPSRecvRate);

		// 운행종료시점의 이동거리를 파일로 저장한다.
		// 운행종료하여 파일로 기록
		gdwDistInDrive = Total_Dist;

		sprintf(&st_GPS_FINAL_INFO.szErrorLog[3], "%06d", Total_Dist);
		sprintf(&st_GPS_FINAL_INFO.szErrorLog[9], "%02d", turn_cnt);
		sprintf(&st_GPS_FINAL_INFO.szErrorLog[11], "%04d", g_nGpsExceptCnt);
		memset(strbuff,0x00,32);
		
		
		fclose(gpsFile2);
		
	}// if.. end...

	
	
	// 01.21 추가 마지막 시리얼 번호 부가하는 루틴수정함.
	
	//33까지 포함한 전체 로그 파일 사이즈를 계산 한다. 
	filesize = 0;
	filesize = getFileSize(GPS_INFO_FILE2);

	loopCnt = filesize / SIMXINFO_SIZE; //전체 로그 루프 카운트 계산 
	
	
	gpsFile = gpsFile2 = NULL;

	
	gpsFile2 = fopen(GPS_INFO_FILE1,"ab");   //로그용 파일을 연다
	gpsFile = fopen(GPS_INFO_FILE2,"rb");  //인식률 계산시 여는 파일을 연다. 
	SN_num =0;
	if(gpsFile != NULL && gpsFile2 != NULL)
	{
		//로그 파일에 시리얼 번호를 적는다. 
		for(i = 0; i <loopCnt ; i++)
		{
	
			fread(buffer,31,1,gpsFile);		//버스노선ID, 차량ID, 운행출발시간
			fwrite(buffer,31,1,gpsFile2);
				
			fread(buffer,3,1,gpsFile);		//정류장순서(i_order)
			fwrite(buffer,3,1,gpsFile2);
			
			Order_check = atointeger(buffer,3);
			
			fread(buffer,7,1,gpsFile);		//버스 정류장 ID
			fwrite(buffer,7,1,gpsFile2);
			
			///////////////////////////////////
			sprintf(strbuff,"%04d",SN_num);		//시리얼 번호
			fwrite(strbuff,4,1,gpsFile2);
			///////////////////////////////////
			fread(buffer,4,1,gpsFile); 			// serial skip...
			
			
			
			//위도, 경도 , 인식 거리 
			fread(buffer,22,1,gpsFile);
			fwrite(buffer,22,1,gpsFile2);

			
			memset(strbuff,0x00,sizeof(strbuff));
			
			
			fread(buffer,2,1,gpsFile);
			fwrite(buffer,2,1,gpsFile2);
			
			fread(buffer,49,1,gpsFile);
			fwrite(buffer,49,1,gpsFile2);

			//현재 시리얼 번호에서 계속 증가				
			SN_num++;


		}
		
		if(g_nTimeOutCnt >=1 && successRate==0)
		{
			memcpy(st_GPS_FINAL_INFO.szPass, "99", 2);
		}
			
		sprintf(strbuff,"%04d",SN_num);	
		memcpy(st_GPS_FINAL_INFO.szSerialNum,  strbuff, 4);	
		
		fwrite(&st_GPS_FINAL_INFO, SIMXINFO_SIZE, 1, gpsFile2);
			
		fflush(gpsFile);
		fclose(gpsFile);
		fflush(gpsFile2);
		fclose(gpsFile2);
	}
	else
	{
		//파일이 제대로 열리지 않았으면 	
	}	

	
	system("rm simxinfo2.tmp");
	usleep(100);
	
	printf("인식률: %d\n",(int)successRate);
	printf("운행 거리: %lu\n",gdwDistInDrive);


	close(g_gps_fd);
}

void BusStationInfoSet()
{
	int i=0;
	char strbuff[6] = {0, };
		
    /*
     * 정류장 총 수를 세팅한다.
     */
	g_nTotalStaCnt = gstStationInfoHeader.dwRecordCnt;
	memcpy(g_szVehcID, gstVehicleParm.abVehicleID, 9);
	memcpy(g_szBusRouteID, gstStationInfoHeader.abRouteID, 8);

	GetStationInfoForNewLogic();

	PreProcess(&g_stBusStation[0], &STA_IF_LOAD[0], &coordinates);

	/*
	 * 정류장 기초정보를 구조체 배열에 세팅한다.
	 */
	for (i=0; i<g_nTotalStaCnt; i++)	{
		memcpy(g_stStaInfo[i].abStationID, gpstStationInfo[i].abStationID, 7);
		sprintf(strbuff, "%03d", gpstStationInfo[i].wStationOrder);
		memcpy(g_stStaInfo[i].bStationOrder, strbuff, sizeof(g_stStaInfo[i].bStationOrder));
	 	g_stStaInfo[i].dLongitude = gpstStationInfo[i].dStationLongitude;  //Station longitude
	 	g_stStaInfo[i].dLatitude = gpstStationInfo[i].dStationLatitude;  //Station latitude
	 	g_stStaInfo[i].wAngle  = gpstStationInfo[i].wStationApproachAngle;  //Station Angle
	 	g_stStaInfo[i].dwDist = gpstStationInfo[i].dwDistFromFirstStation; //Station Accumulated Distance

#ifdef	_DEBUGGPS_
		printf("STA_IF_LOAD[%d].bus_sta_pos_x = %f \n", i+1, g_stStaInfo[i].dLongitude);
#endif
	}

	/*
	 * GPS 로그 기록을 위한 최소시간 값과 최대시간 값	
	 */
	strcpy(szGpsLogStartTime, "20040101010101");
	strcpy(szGpsLogEndTime, "99990101010101");
		
	printf("\n 정류장 총 수 : %d \n", g_nTotalStaCnt);
	
	ReadyStaInfo(g_nTotalStaCnt);	// 정류장 좌표 변환등 검색을 위한 작업
	
	g_nPrevStaOrder = ContinueDriveCheck();// 이전 인식 정류장 정보 로딩

	g_nCurStaOrder = g_nPrevStaOrder; // 직전인식 정류장을 현재 정류장으로
	
	printf("\n 초기치 정류장 순서 : %d \n", g_nCurStaOrder);

	/*
	 * 정류장 수가 1개 이상 즉, 정상으로 로딩이 되었다면?
	 * 처음 정류장을 위에서 결정된 현재정류장으로 공유메모리에 설정
	 * station_id : 정류장 id
	 * station_nm : 정류장 명칭
	 */
	if (g_nTotalStaCnt > 0)
	{
		memcpy( gpstSharedInfo->abNowStationID, gpstStationInfo[g_nCurStaOrder-1].abStationID, 7 );
		memcpy( gpstSharedInfo->abNowStationName, gpstStationInfo[g_nCurStaOrder-1].abStationName, 16 );
	}
	else	
	{
		memcpy(gpstSharedInfo->abNowStationID, "9999999", 7);
		memcpy(gpstSharedInfo->abNowStationName, "                ", 16);
	}
	
}

short GpsPortProc()
{
	char szStartLogTime[15];
		
#ifdef _SIMMODE_
	printf("\r\n s_SimFile = fopen(GPS_RAW_FILE OPEN... \r\n");
	s_SimFile = fopen(GPS_RAW_FILE, "r");
	if (s_SimFile != NULL)	printf("\n open Success........\n");
	printf("\r\n s_SimFile = fopen(GPS_RAW_FILE OPEN SUCCESS... \r\n");
#endif
	// GPS 포트 오픈
	g_gps_fd = GpsPortOpen(GPS_DEV_NAME, BAUD_RATE, 1);
	if (g_gps_fd > 0)
		printf("\n GPS 포트 오픈 : %d", g_gps_fd);
	else
		return g_gps_fd;

	// 운행중 껐다 킨것이 아니고, 새로 운행시작하는 경우
	if (g_nContinueFlag == 0) {
		SyncDeparttimeWithMainLogic(szStartLogTime);
		strcpy(g_szRunDepartTime, szStartLogTime);
		g_szRunDepartTime[14] = 0x00;
	}
	
	return g_gps_fd;
}

static void GpsDataProc()
{
//	struct timeval stTime1, stTime2;
	char szCurrTime[15];
//	int bTimeCheck = 0;
	int	 nParam = 0;	
	int	 nFoundSta = 0;
//	FOUND_STA_INFO stFoundStaInfo;
	time_t tTime1, tTime2;
	int nMaxSearchTime = 0;
	double dbGpsActStatus = 0;
	time_t tReadDtime;

	gpsloopcheck = 4;


	nParam = GpsDataProcessing();
//		GpsErrorProcess(nParam);

	gpstSharedInfo->gbGPSStatusFlag = g_gpsStatusFlag;


	GetRTCTime( &tReadDtime );
	TimeT2ASCDtime( tReadDtime, szCurrTime );

	/*
	 * 현재 정류장을 인식한 시간과 현재 시간을 비교하여 기준시간 이상 
	 * 정류장을 인식하지 못하고, GPS 안테나 상태가 현재 단선 또는 감도 불량인경우
	 * 추가거리를 '0'로 하는 플래그를 세팅한다
	 */
	tTime1 = GetTimeTFromASCDtime(szCurrTime);	// 현재시간
	tTime2 = GetTimeTFromASCDtime(g_szStaFindTime);	// 현재 정류장 인식했을 때 시간

	nMaxSearchTime = GetMaxTimeforNextStaSearch(); // 기준시간 구하기
	dbGpsActStatus = GetGpsActStatus();
	
	if ( dbGpsActStatus < GPS_STATUS_VALID_BOUND && abs(tTime1-tTime2) > nMaxSearchTime) 
	{
//		printf("\n정류장 검색을 하지 못하고 있는 상태입니다.\n");
		/*
		 * boolIsValidGPSSearch 정의되어 있어야 함
		 * EVENT_GPS_SEARCH_ERROR 정의되어 있어야 함
		 */
		//gpstSharedInfo->boolIsValidGPSSearch = FALSE;
		//ctrl_event_info_write(EVENT_GPS_SEARCH_ERROR);
	}
	else
	{
		;//gpstSharedInfo->boolIsValidGPSSearch = TRUE;
	}
	
	
	/*
	 * 정류장 정보가 있고, gps 수신 데이타가 정상인 경우만 정류장
	 * 검색하도록 함
	 */
	if (g_nTotalStaCnt > 0 && g_gpsStatusFlag == GPS_STATUS_OK)	{
		GPSCorr(&GPS, &s_stRMC, &g_stGGA);
		DBManage(&g_stBusStation[0]);
		nFoundSta = GetStation();
//		nFoundSta = BusStationDetecting(&stFoundStaInfo, s_stRMC.Lat, 
//		                                 s_stRMC.Lon, s_stRMC.UTCTime, 
//		                                 s_stRMC.SpeedKn);
	}
	else
		nFoundSta = 0;
		
	if (nFoundSta > 0)	{
		// 통합에서는 ?
		memcpy( gpstSharedInfo->abNowStationID, gpstStationInfo[nFoundSta-1].abStationID, 7 );
		memcpy( gpstSharedInfo->abNowStationName, gpstStationInfo[nFoundSta-1].abStationName, 16 );
		memcpy( gabGPSStationID, gpstStationInfo[nFoundSta-1].abStationID, 7 );
	}

	// 시간 보정루틴
	if(bTimeCheck == 0)	
	{
		if(gettimeofday ( &stTime1, NULL ) != -1) 
		{
/**		
#ifdef	_GPS_27_				
			rtc_gettime(szCurrTime);
#else
			GetRTCTime( &tReadDtime );
			TimeT2ASCDtime( tReadDtime, szCurrTime );
#endif
**/
//			printf("[GpsDataProc] 현재시간 : %s\n", szCurrTime); //0330 2005.2.28
			bTimeCheck = 1;
		}
		else
		{
			close(g_gps_fd);
			printf("Thread terminate 1 \n");
			return;
		}
	}
	
	if(gettimeofday ( &stTime2, NULL ) == -1 )	{
		
		close(g_gps_fd);			
		printf("Thread terminate 3 \n");
		return;
	}	
	if (stTime2.tv_sec - stTime1.tv_sec >= TIME_CHECK_GAP)	{
		bTimeCheck = 0;
		gpstime_check();
	}
			
	
	EndStationAdviceMsg();		
	
}

void GpsExitProc(void)
{
#ifdef _SIMMODE_
	fclose(s_SimFile);
#endif	
	close(g_gps_fd);  
	
}
