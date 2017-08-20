#include "gps_define.h"

#ifdef	_GPS_27_
	#include "../include/bus100.h"
#else
	#include <stdio.h>
	#include <stdlib.h>
	#include "../../common/type.h"
	#include "../system/bus_type.h"
	#include "gps.h"		
//	#include "gps_env.h"
	#include "gps_search.h"
	#include "gps_util.h"
	#include "device_interface.h"

#endif




//가상 검색을 위한 변수들
static int s_nVirDetectCnt = 0;
static int s_nCurVirStaOrder = 1;
static int s_nPrevVirStaOrder = 1;

static int s_nRecogFlag = 0;	// 정류장 인지 여부 플래그

void ErrorLogCheck(unsigned int Order, unsigned int Index);

/******************************************************************************
*                                                                             *
*    함수명 : BusStationDetecting()                                        	  *
*                                                                             *
*      설명 : [정류장 검색]										  			  *
*                                                                             *
*    작성자 : 김 재 의                                                        *
*                                                                             *
*    작성일 : 2005.05.30                                                      *
*                                                                             *
*  파라메터 : IN/OUT  PARAM NAME      TYPE            DESCRIPTION             *
*             ------  ----------      --------        ----------------------- *
*			  IN	  stFoundStaInfo  FOUND_STA_INFO* 검색된 정류장 정보      *
*    리턴값 : 발견한 정류장 순서.				   						      *
*  주의사항 :                                                                 *
*                                                                             *
******************************************************************************/
int BusStationDetecting(FOUND_STA_INFO *stFoundStaInfo, double x, double y, 
                                                   double utctime, double speed)
{
	double GPSLatDeg;
	double GPSLonDeg;
	double GPSHgt;
	double EnuPos[3]={0};
	double TempLLH[3];
	double TempXYZ[3];
	double CurrentVelms =0;         //현재 GPS 속도 정보
	
	unsigned int hour;
	unsigned int min;
	double sec;	
	
	int nCurStaOrder = 1;	// 현재 정류장 순서
	int nNextStaOrder = 0;
	
	int	nMinOrder = 1;	//현재 위치에서 가장 가까운 정류장순서.
	double dbDis = 0;	//가장 가까운 정류장 까지의 거리
	double dbCurStaOrderDis = 0; // 현 위치, 현재 정류장 간 거리
	double dbNextStaOrderDis = 0; // 현 위치, 다음 정류장 간 거리
	
	int nRecogFlag = 0;	// 인지 여부 플래그
	int nDecidedFlag = 0;	// 인식 여부 플래그

	char szCurrTime[15];	// 현재 시간


	//--------------------------------------------------
	// 현재 위치를 계산 시작
	hour = (int)(utctime/10000);
	min = (int)((utctime - hour*10000)/100);
	sec = utctime - hour*10000 - min*100;
//	GPSsec = hour*3600 + min*60 + sec;

	// 도 분 -> 도 
	GPSLatDeg = (int)(x/100) + ((x - (int)(x/100)*100.)/60) ;
	GPSLonDeg = (int)(y/100) + ((y - (int)(y/100)*100.)/60) ;
	GPSHgt = 30;

	TempLLH[0] = GPSLatDeg*D2R;
	TempLLH[1] = GPSLonDeg*D2R;
	TempLLH[2] = GPSHgt;
	

	//LLH값을 XYZ값으로 
	LatLonHgtToXYZ_D(TempLLH,TempXYZ);  //현재 좌표를 XYZ으로 


	//XYZ값을 기준 점을 위치로 ENU값으로 바꾸어줌 
	//DB 1에 대해 ENU 값을 구하게 된다. 
	//EnuPos 값이 현재 위치 X, Y
	XyztoENU_D(TempXYZ,coordinates.OrgXYZ,coordinates.OrgLLH,EnuPos); 

	// 현재 속도
	CurrentVelms = speed *1.852/3.6;  // unit m/s

	if (CurrentVelms > g_dbMaxSpeed)
		g_dbMaxSpeed = CurrentVelms;

	/*
	 * 기준속도 이하이면 정류장 검색하지 않음
	 */
	if (CurrentVelms < SPEED_BOUND_SEARCH)
	{
		//printf("\n검색기준속도 이하입니다. 현재속도[%f]\n", CurrentVelms);
		return CANNOT_FIND_NEW_STATION;
	}
	
	// 현재 위치 계산 끝	
	//--------------------------------------------------	

	// 현 위치 대비 가장 가까운 정류장 순서를 얻는다.
	GetShortestStaOrder(EnuPos[0], EnuPos[1], &nMinOrder, &dbDis);
	
//	printf("\n가장 가까운 정류장 순서:%d, 거리:%f \n", nMinOrder, dbDis);

	
	nCurStaOrder = g_nCurStaOrder;
	nNextStaOrder = nCurStaOrder + 1;
	if (nNextStaOrder > g_nTotalStaCnt)
		nNextStaOrder = 1;

	// 현재 정류장과 현 위치 사이의 거리 계산
	dbCurStaOrderDis = GetDis(g_stBusStation[nCurStaOrder-1].ENU[0], 
							  g_stBusStation[nCurStaOrder-1].ENU[1], 
							  EnuPos[0], EnuPos[1]);
			
	// 다음 정류장과 현 위치 사이의 거리 계산
	dbNextStaOrderDis = GetDis(g_stBusStation[nNextStaOrder-1].ENU[0], 
								g_stBusStation[nNextStaOrder-1].ENU[1], 
								EnuPos[0], EnuPos[1]);
	
#ifdef	_DEBUGGPS_	
	printf("\n 현재 위치 좌표 : x[%f], y[%f] \n", EnuPos[0], EnuPos[1]);
	printf("\n 현 정류장 순서 : %d \n", nCurStaOrder);
	printf("\n 현 정류장 순서 좌표 : x[%f], y[%f] \n", g_stBusStation[nCurStaOrder-1].ENU[0],
			g_stBusStation[nCurStaOrder-1].ENU[1]);
	printf("\n 현 정류장까지 거리 : %f \n", dbCurStaOrderDis);
	printf("\n 다음 정류장까지 거리 : %f \n", dbNextStaOrderDis);
#endif	

	/*
	 * 정상거리인지, 아닌지
	 */
	if (IsOverDistance(nCurStaOrder, dbNextStaOrderDis) == 1)
	{
		g_gpsStatusFlag = DIST_OVER ; // 거리 이상으로 판명
	}
	
	// 정류장 인지 여부를 판독한다.
	nRecogFlag = RecognizeStation(dbCurStaOrderDis, dbNextStaOrderDis, dbDis,
									nMinOrder, nCurStaOrder, nNextStaOrder);

#ifdef	_DEBUGGPS_		
	printf("\n 판독한 정류장 순서 : %d \n", nRecogFlag);
#endif

	if (nRecogFlag >= 1)	{
		// 인지된 정류장을 최종 인식했는지 판독한다.
		nDecidedFlag = DecideStation(nRecogFlag, dbDis);
		
		if (nDecidedFlag > 0)	{
			g_nPrevStaOrder = g_nCurStaOrder;	// 전 정류장 정보로 현재정류장값을
			g_nCurStaOrder = nDecidedFlag;
			g_nPassMinDis = (int)dbDis;
			
			//printf("\n g_nPrevStaOrder:%d, g_nCurStaOrder:%d\n",g_nPrevStaOrder, g_nCurStaOrder);
			// 인식한 정류장 정보등을 로그에 기록한다.
			WriteLog();

			// 현재 인식한 정류장 정보를 저장한다.
			SaveCurrentStation(nDecidedFlag, 1);

			memcpy(stFoundStaInfo->station_id, g_stStaInfo[nDecidedFlag-1].abStationID, 7);
			
			// 인식했을 때의 시간을 보관한다
			memcpy(g_szStaFindTime, szCurrTime, 14);
		}
	}
	return nDecidedFlag;
}

/******************************************************************************
*                                                                             *
*    함수명 : GetShortestStaOrder()                                        	  *
*                                                                             *
*      설명 : [현위치 대비 가장 가까운 정류장 순서 검색]  			  		  *
*                                                                             *
*    작성자 : 김 재 의                                                        *
*                                                                             *
*    작성일 : 2005.05.30                                                      *
*                                                                             *
*  파라메터 : IN/OUT  PARAM NAME  TYPE          DESCRIPTION             	  *
*             ------  ----------  --------      ----------------------- 	  *
*			  IN	  curX  	  double 		현 위치 X좌표     			  *
*			  IN	  curY		  double		현 위치 Y좌표				  *
*			  IN/OUT  nMinOrder	  int *			최소 거리 정류장 순서		  *
*			  IN/OUT  dbDis		  double *		최소 거리					  *
*    리턴값 : 없음.						                      				  *
*  주의사항 :                                                                 *
*                                                                             *
******************************************************************************/
void GetShortestStaOrder(double curX, double curY, int *nMinOrder, double *dbDis)
{
	int i=0;
	int nFlag=0;	// X, Y 반경 범주에 있는 지 확인 1:X 반경만, 2:Y 반경만, 3:X, Y 둘다
	double dbMaxX, dbMinX;
	double dbMaxY, dbMinY;
	double dbTmpX, dbTmpY;
	double dbMinDis = 9999999;


	*dbDis = 0;
	*nMinOrder = 0;
	
	dbMaxX = curX + g_dbRoundX;
	dbMinX = curX - g_dbRoundX;
	dbMaxY = curY + g_dbRoundY;
	dbMinY = curY - g_dbRoundY;
#ifdef	_DEBUGGPS_
	printf("curX:[%f], dbMaxX:[%f], dbMinX:[%f]\n", curX, dbMaxX, dbMinX);
	printf("curY:[%f], dbMaxY:[%f], dbMinY:[%f]\n", curY, dbMaxY, dbMinY);
#endif	

	
	for (i=0; i<g_nTotalStaCnt; i++)	{
		// 반경안에 있는 정류장인지 검사
#ifdef	_DEBUGGPS_
		printf("g_stSortX[%d].dbX = %f\n", i, g_stSortX[i].dbX);
		printf("g_stSortY[%d].dbY = %f\n", i, g_stSortY[i].dbY);		
#endif
		if (g_stSortX[i].dbX < dbMaxX && g_stSortX[i].dbX > dbMinX)	{

			dbTmpX=g_stBusStation[g_stSortX[i].nOrder-1].ENU[0];
			dbTmpY=g_stBusStation[g_stSortX[i].nOrder-1].ENU[1];

#ifdef	_DEBUGGPS_
		printf("g_stSortX[%d].nOrder = %d\n", i, g_stSortX[i].nOrder);
		printf("dbTmpX:[%f], dbTmpY:[%f]\n", dbTmpX, dbTmpY);		
#endif
			
			// 범주안에 있는 정류장과 현재 위치와의 거리를 구해 최소값과 비교
			*dbDis=sqrt((dbTmpX-curX)*(dbTmpX-curX)+(dbTmpY-curY)*(dbTmpY-curY));
			if (*dbDis < dbMinDis)	{
				dbMinDis = *dbDis;
				*nMinOrder = g_stSortX[i].nOrder;
			}
			nFlag = 1;
		}
		if (g_stSortY[i].dbY < dbMaxY && g_stSortY[i].dbY > dbMinY)	{
			dbTmpX=g_stBusStation[g_stSortY[i].nOrder-1].ENU[0];
			dbTmpY=g_stBusStation[g_stSortY[i].nOrder-1].ENU[1];
			
			// 범주안에 있는 정류장과 현재 위치와의 거리를 구해 최소값과 비교
			*dbDis=sqrt((dbTmpX-curX)*(dbTmpX-curX)+(dbTmpY-curY)*(dbTmpY-curY));
			if (*dbDis < dbMinDis)	{
				dbMinDis = *dbDis;
				*nMinOrder = g_stSortY[i].nOrder;
			}
			if (nFlag > 0)	nFlag = 3;
			else			nFlag = 2;
		}
		   
	}
	
	*dbDis = dbMinDis;
	
}

int SetInitStaOrder(int nMinOrder, double dbDis)
{
	int nInitStaOrder = 1;
	
	if (g_nContinueFlag == 1)
		nInitStaOrder = g_nPrevStaOrder+1;
	else
		nInitStaOrder = GetStartOrder(nMinOrder, dbDis);
	
	if (nInitStaOrder > g_nTotalStaCnt)
		nInitStaOrder = 1;
		
	return nInitStaOrder;
}


int GetStartOrder(int nMinOrder, double dbDis)
{
	int nStartOrder = 1;	// 기본값 기점.
	
	if (dbDis <= 50) {
		if (nMinOrder == g_nPrevStaOrder)
			nStartOrder = g_nPrevStaOrder;
		else	{
			if (abs(nMinOrder-g_nPrevStaOrder) <= 3)
				nStartOrder = g_nPrevStaOrder;
			else
				nStartOrder = nMinOrder;
		}
	}
	
	return nStartOrder;
}

/******************************************************************************
*                                                                             *
*    함수명 : RecognizeStation()                                         	  *
*                                                                             *
*      설명 : [정류장 인지여부 판단] 										  *
*                                                                             *
*    작성자 : 김 재 의                                                        *
*                                                                             *
*    작성일 : 2005.05.30                                                      *
*                                                                             *
*  파라메터 : IN/OUT  PARAM NAME  		TYPE    DESCRIPTION             	  *
*             ------  ----------  		------  ----------------------- 	  *
*			  IN	  dbCurStaOrderDis  double 	현재 정류장과 현위치 거리 	  *
*			  IN	  dbNextStaOrderDis	double	다음 정류장과 현위치 거리	  *
*			  IN	  dbShortDis		double  가장 가까운 정류장과의 거리	  *
*			  IN	  nMinOrder			int		현위치에서 가장 가까운 정류장 *
*												순서						  *
*			  IN	  nCurStaOrder		int		현재 정류장 순서			  *
*			  IN	  nNextStaOrder		int		다음 정류장 순서			  *
*    리턴값 : 정류장 인지 여부 .						                      *
*			  0 : 미인지													  *
*			  1이상 : 인지한 정류장 순서									  *
*  주의사항 :                                                                 *
*                                                                             *
******************************************************************************/
int RecognizeStation(double dbCurStaOrderDis, double dbNextStaOrderDis, 
					 double dbShortDis, int nMinOrder, int nCurStaOrder, 
					 int nNextStaOrder)
{
	int nRecogStaOrder = 0;
	int nEntryAngle	= 0;	// 현재 차량 진입각
	int nEntryNextSta = 0;	// 다음 정류장 진입각
	int nEntryMinOrder = 0;	// 가장 가까운 정류장의 진입각
	int nVirCurStaOrder = 0;	// 가상 프로세스에서 리턴하는 정류장 순서

	// 인지 기준 변수 값을 먼저 세팅 (0:미인지, 1이상:인지)
	nRecogStaOrder = s_nRecogFlag;
	
	nEntryAngle = (int)s_stRMC.TrackTrue;
	nEntryNextSta = g_stBusStation[nNextStaOrder-1].Angle;
	nEntryMinOrder = g_stBusStation[nMinOrder-1].Angle;
#ifdef	_DEBUGGPS_
	printf("\n nRecogStaOrder:%d, nEntryAngle:%d, nEntryNextSta:%d, nEntryMinOrder:%d \n ",
			nRecogStaOrder, nEntryAngle, nEntryNextSta, nEntryMinOrder);
#endif	

	// 현재 정류장까지 거리보다 다음정류장 까지 거리가 더 짧은 경우
	if (dbCurStaOrderDis > dbNextStaOrderDis)	{
		s_nRecogFlag = nNextStaOrder;

	}
	// 현 위치대비 가장 가까운 정류장이 현재 및 다음정류장이 아닌 경우
	if (nMinOrder != nCurStaOrder && nMinOrder != nNextStaOrder)	{
		if (dbShortDis <= SEARCH_ROUND_DIS / 2
			   && HeadingErrorABS_D(nEntryAngle, nEntryMinOrder) <= SEARCH_ROUND_ANGLE)
		{
				// 가상 검색을 위한 프로세스
				printf("\n---- VirturalMode ----\n ");
				
				nVirCurStaOrder = VirturalDetectProcess(nMinOrder);
				
				printf("s_nVirDetectCnt : %d \n", s_nVirDetectCnt);
				
				// 정류장 순서에 맞게 연속 3개의 정류장이 인지 되었다면, 
				// 마지막번째 정류장을 최종 인지한 것으로 간주.
				if (nVirCurStaOrder == 0)	{
					nRecogStaOrder = g_nNextStaOrder;
					s_nRecogFlag = nRecogStaOrder;
				}
				else
					return 0;
		}
	}

	nRecogStaOrder = s_nRecogFlag;
	
	return nRecogStaOrder;
}

/******************************************************************************
*                                                                             *
*    함수명 : VirturalDetectProcess()                                         *
*                                                                             *
*      설명 : [순서상 다음정류장이 아닌 임의 정류장이 인식 조건에 들어온 경우 *
*                                                                             *
*    작성자 : 김 재 의                                                        *
*                                                                             *
*    작성일 : 2005.05.30                                                      *
*                                                                             *
*  파라메터 : IN/OUT  PARAM NAME  		TYPE    DESCRIPTION             	  *
*             ------  ----------  		------  ----------------------- 	  *
*			  IN	  nDectectStaOrder  int 	검색된 정류장 순서 		  	  *
*    리턴값 : 정류장 확정 여부 .						                      *
*			  0 : 확정														  *
*			  1이상 : 가상 인지로 확정										  *
*			  -1 : 미확정													  *
*  주의사항 :                                                                 *
*                                                                             *
******************************************************************************/
int VirturalDetectProcess(int nDetectStaOrder)
{
	int ret = -1;
	
	if (s_nVirDetectCnt == 0)	{
		s_nVirDetectCnt = 1;
		s_nCurVirStaOrder = nDetectStaOrder;
		s_nPrevVirStaOrder = s_nCurVirStaOrder;
		
		return s_nCurVirStaOrder;
	}
	
	if (s_nVirDetectCnt <= 2)	{
		s_nCurVirStaOrder = nDetectStaOrder;
		if (abs(s_nPrevVirStaOrder - s_nCurVirStaOrder) == 1)	{
			s_nPrevVirStaOrder = s_nCurVirStaOrder;
			s_nVirDetectCnt = s_nVirDetectCnt + 1;
		}
		else if (s_nPrevVirStaOrder == s_nCurVirStaOrder)
			;
		else
			s_nVirDetectCnt = 0;
	}
	else if (s_nVirDetectCnt == 3)	{
		s_nVirDetectCnt = 0;
		s_nPrevVirStaOrder = 0;
		s_nCurVirStaOrder = nDetectStaOrder;
		g_nNextStaOrder = s_nCurVirStaOrder;
		ret = 0;
	}
	
	return ret;
}

/******************************************************************************
*                                                                             *
*    함수명 : DecideStation()                                         	  	  *
*                                                                             *
*      설명 : [정류장 인식여부 판단] 										  *
*                                                                             *
*    작성자 : 김 재 의                                                        *
*                                                                             *
*    작성일 : 2005.05.30                                                      *
*                                                                             *
*  파라메터 : IN/OUT  PARAM NAME  			TYPE    DESCRIPTION               *
*             ------  ----------  			------  -----------------------   *
*			  IN	  nRecogOrder  			int 	인지한 정류장 순서 	  	  *
*			  IN	  dbRecogStaOrderDis	double	다음 정류장과 현위치 거리 *
*    리턴값 : 정류장 인지 여부 .						                      *
*			  0 : 미인식													  *
*			  1이상 : 인식한 정류장 순서									  *
*  주의사항 :                                                                 *
*                                                                             *
******************************************************************************/
int DecideStation(int nRecogOrder, double dbRecogStaOrderDis)
{
	int nDecidedFlag = 0;
	int nEntryAngle = 0;
	int nAngleRecog = 0;
	
	if (nRecogOrder == g_nCurStaOrder)	return nDecidedFlag;
	
	nAngleRecog = g_stBusStation[nRecogOrder-1].Angle;
	nEntryAngle = (int)s_stRMC.TrackTrue;	// 차량 진입각
	
	if (dbRecogStaOrderDis <= SEARCH_ROUND_DIS 
		&& HeadingErrorABS_D(nEntryAngle,nAngleRecog)  <= SEARCH_ROUND_ANGLE)	{
		nDecidedFlag = nRecogOrder;
	}
	
	return nDecidedFlag;
}

/******************************************************************************
*                                                                             *
*    함수명 : SaveCurrentStation()                                         	  *
*                                                                             *
*      설명 : [직전 인식 정류장 파일 저장] 									  *
*                                                                             *
*    작성자 : 김 재 의                                                        *
*                                                                             *
*    작성일 : 2005.05.30                                                      *
*                                                                             *
*  파라메터 : IN/OUT  PARAM NAME  		TYPE    DESCRIPTION   				  *
*			  IN	  nDecidedOrder  	int 	인식한 정류장 순서 	  		  *
*			  IN	  nContinueFlag		int		운행 1, 운행종료 후 0		  *
*    리턴값 : 없음.					                      					  *
*  주의사항 :                                                                 *
*                                                                             *
******************************************************************************/
void SaveCurrentStation(int nDecidedOrder, int nContinueFlag)
{
	FILE*	file = NULL;
	int nSaveStaOrder;
//	int nContinueFlag = 1;

	nSaveStaOrder = nDecidedOrder;

	file = fopen(PREV_PASS_FILE, "wb+");
	
	printf("SaveCurStation \n");
	
	if (file != NULL)	
	{
		fwrite(&nSaveStaOrder, sizeof(int), 1, file); // 직전 인식 정류장 순서
		fwrite(&nContinueFlag, sizeof(int), 1, file); // 운행연속 여부 저장
		fwrite(g_szRunDepartTime, 14, 1, file);	// 운행출발시간 저장
		fclose(file);
	}
	
	// 수신 데이타별 카운트 값 백업
	g_nPrevAvailCnt = g_nAvailCnt;
	g_nPrevInvalidCnt = g_nInvalidCnt;
	g_nPrevTimeOutCnt = g_nTimeOutCnt;
	g_nPrevCheckSumCnt = g_nCheckSumCnt;
	
	GpsDataLogWrite();
}

void WriteLog()
{
	int i=0;
	int i_order = 0;
	char buff[11] = {0, };
	char strbuff[11] = {0, };		
	unsigned int write22_flag =0;		// 기점외 시작시 22 Set flag
	FILE *gpsFile2 = NULL;	
	
	unsigned int GPSDataCnt = 0;  //마지막 인식한 정류장과 현재 인식한 정류장사이의 총 Data count를 저장함
	unsigned int InvalidCnt = 0; // 마지막 인식한 정류장과 현재 인식한 정류장 사이의 invalid count 
	unsigned int AvailCnt = 0; //마지막 인식한 정류장과 현재 인식한 정류장 사이의 Avail count 
	unsigned int TimeOutCnt = 0; //마지막 인식한 정류장과 현재 인식한 정류장 사이의 Avail count
	double AvailErrorPerc = 0;   // 총 데이터 개수 중에 valid count의 퍼센트
	
//	unsigned int InvalidCntbtST =0; 
//	unsigned int TimeOutCntbtST =0; 


	InvalidCnt = g_nInvalidCnt - g_nPrevInvalidCnt; 
	AvailCnt = g_nAvailCnt - g_nPrevAvailCnt;
	TimeOutCnt =(unsigned int)((g_nTimeOutCnt - g_nPrevTimeOutCnt)*2);
	GPSDataCnt = InvalidCnt + AvailCnt + TimeOutCnt; //역간 전체 데이터 개수(check sum error은 고려 하지 않음)
	
	if(GPSDataCnt == 0)
		GPSDataCnt++;
		
	AvailErrorPerc = ((double)(AvailCnt) / (double)(GPSDataCnt)) *100.0;

	// 현재 인식한 정류장 순서를 임시 변수에 세팅
	i_order = g_nCurStaOrder;

	memcpy(g_stGPS_INFO_STATION[i_order].szLineNum, g_szBusRouteID, 8);	// 버스노선 ID
	memcpy(g_stGPS_INFO_STATION[i_order].szBusID, g_szVehcID, 9);				// 버스차량 ID
	memcpy(g_stGPS_INFO_STATION[i_order].szStartTime, g_szRunDepartTime,14);
	memcpy(g_stGPS_INFO_STATION[i_order].szOrder, 	g_stStaInfo[i_order-1].bStationOrder, 3);	// 정류장 순서
	memcpy(g_stGPS_INFO_STATION[i_order].szID, 		g_stStaInfo[i_order-1].abStationID, 7);	// 정류장 id
	memcpy(g_stGPS_INFO_STATION[i_order].szLongitude, g_szPassMinLongitude, 10);	// 위도
	memcpy(g_stGPS_INFO_STATION[i_order].szLatitude,  g_szPassMinLatitude, 9);	// 경도
	sprintf(strbuff, "%3d", g_nPassMinDis);
	memcpy(g_stGPS_INFO_STATION[i_order].szPassDis,   strbuff, 3);		// 인식거리
	memcpy(g_stGPS_INFO_STATION[i_order].szPass, 	  "01", 2);					// 통과여부 
	memcpy(g_stGPS_INFO_STATION[i_order].szPassTime,  g_szGpsCurrTime, 14);		// 통과시간
	memcpy(g_stGPS_INFO_STATION[i_order].szSateCnt,   g_szPassSatelliteCnt, 2);	// 위성개수
	sprintf(strbuff, "%5d", g_nInvalidCnt);
	memcpy(g_stGPS_INFO_STATION[i_order].szIVcnt, strbuff, 5); // Invalid Cnt
	memset(strbuff, 0x00, 5); 
	sprintf(strbuff, "%5d", g_nAvailCnt);
	memcpy(&g_stGPS_INFO_STATION[i_order].szIVcnt[5], strbuff, 5); // Valid Cnt

	memset(g_stGPS_INFO_STATION[i_order].szErrorLog,  0x30, 19);		  
	memset(g_stGPS_INFO_STATION[i_order].szTemp,  0x20, 4);					
	memset(g_stGPS_INFO_STATION[i_order].szSerialNum,  0x20, 4);
	
/*	
	//로그를 쓸때마다 자기보다 오더가 높은 정류장 정보 초기화 
	for (i=i_order+1; i<=g_nTotalStaCnt; i++)	
	{
		
		memcpy(g_stGPS_INFO_STATION[i].szLineNum, g_szBusRouteID, 8);	// 버스노선 ID
		memcpy(g_stGPS_INFO_STATION[i].szBusID, g_szVehcID, 9);				// 버스차량 ID
		memset(g_stGPS_INFO_STATION[i].szStartTime,0x20,14);
		memcpy(g_stGPS_INFO_STATION[i].szOrder, 	g_stStaInfo[i].bStationOrder, 3);	// 정류장 순서
		memcpy(g_stGPS_INFO_STATION[i].szID, 		g_stStaInfo[i].abStationID, 7);	// 정류장 id
		memset(g_stGPS_INFO_STATION[i].szLongitude, 0x20, 10);						// 위도
		memset(g_stGPS_INFO_STATION[i].szLatitude,  0x20, 9);							// 경도
		memset(g_stGPS_INFO_STATION[i].szPassDis,   0x20, 3);							// 통과최소거리
		memset(g_stGPS_INFO_STATION[i].szPass, 	  0x20, 2);							// 통과여부 
		memset(g_stGPS_INFO_STATION[i].szPassTime,  0x20, 14);						// 통과시간
		memset(g_stGPS_INFO_STATION[i].szSateCnt,   0x20, 2);							// 위성개수
		memset(g_stGPS_INFO_STATION[i].szIVcnt,	  0x20, 10);		
		memset(g_stGPS_INFO_STATION[i].szErrorLog,  0x30, 19);		  
		memset(g_stGPS_INFO_STATION[i].szTemp,  0x20, 4);					
		memset(g_stGPS_INFO_STATION[i].szSerialNum,  0x20, 4);
	}
*/	
/*
	//운행 시작 시간을 불러 온다. 
	if (i_order != 0)
	{
		gpsFile2 = fopen(GPS_INFO_FILE2, "r");
		
		if(gpsFile2 != NULL)
		{
			fread(buffer,SIMXINFO_SIZE,1,gpsFile2);
			memcpy(szStartTimebuff,&buffer[17],14);
			fclose(gpsFile2);
		}
	}
*/	

	printf("i_order = [%d], g_nPrevStaOrder = [%d]", i_order, g_nPrevStaOrder);
	///////////////////////중간에 예외 상황 발생시 에러 코드 채움/////////////////////////////
	// 중간에 Jump 했을 경우들
	if( i_order > g_nPrevStaOrder+1)
	{
		printf("\n중간에 점프한 경우\n");
		//2005-02-18 7:21오후
		for(i = g_nPrevStaOrder+1; i <= i_order-1; i++)
		{
			if(AvailErrorPerc < AVAIL_CNT_TH)//Avail count가 30%이하이면 88이나 99중 카운트가 많은것을 직는다. 
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
					printf("\n 99 기록 1 \n");
				}
				
			}
			else//  Avail count가 30%이상이면 66이나 다른 에러로 찍는다. 
			{
				//memcpy(g_stGPS_INFO_STATION[i].szPass,g_stGPS_INFO_STATION[i_order].szTemp, 2);
				//memcpy(g_stGPS_INFO_STATION[i].szTemp,g_stGPS_INFO_STATION[i_order].szTemp, 2);
				memcpy(g_stGPS_INFO_STATION[i].szPass, "66", 2);
				memcpy(g_stGPS_INFO_STATION[i].szTemp, "66", 2);
			}

			memcpy(g_stGPS_INFO_STATION[i].szStartTime,g_szRunDepartTime,14);
			//나머지 필드는 모두 초기화 (스페이스 바로)
			memset(g_stGPS_INFO_STATION[i].szIVcnt,0x20, 10);
			memset(g_stGPS_INFO_STATION[i].szPassTime, 0x20, 14);
			memset(g_stGPS_INFO_STATION[i].szLongitude,0x20, 10);
			memset(g_stGPS_INFO_STATION[i].szLatitude, 0x20, 9);
			memset(g_stGPS_INFO_STATION[i].szErrorLog,  0x30, 19);
			memset(g_stGPS_INFO_STATION[i].szPassDis,   0x20, 3);
			memset(g_stGPS_INFO_STATION[i].szSateCnt,   0x20, 2);		
		}

	}
	
	// 종점 부근에서 Jump 했을 경우
	if( i_order < g_nPrevStaOrder)
	{
		printf("\n종점 부근에서 점프한경우\n");
		//2005-02-18 7:21오후
		for(i = g_nPrevStaOrder + 1; i<= g_nTotalStaCnt;i++)
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
					printf("\n 99 기록 2 \n");
				}
				
			}
			else // 66 이나 다른 에러 
			{
	
				// 중간에 시스템을 끄고 회차 한후 기점에서 다시 시스템을 켰을 경우에 대비
				//if( memcmp(g_stGPS_INFO_STATION[i_order].szTemp, "  ", 2) == 0 )
				//{
					memcpy(g_stGPS_INFO_STATION[i].szPass, "66", 2);
					memcpy(g_stGPS_INFO_STATION[i].szTemp, "66", 2);
				//}
				//else
				//{
				//	memcpy(g_stGPS_INFO_STATION[i].szPass,g_stGPS_INFO_STATION[i_order].szTemp, 2);
				//	memcpy(g_stGPS_INFO_STATION[i].szTemp,g_stGPS_INFO_STATION[i_order].szTemp, 2);
				//}
			}	
			memcpy(g_stGPS_INFO_STATION[i].szStartTime,g_szRunDepartTime,14);
			//나머지 필드는 모두 초기화 (스페이스 바로)
			memset(g_stGPS_INFO_STATION[i].szIVcnt,0x20, 10);
			memset(g_stGPS_INFO_STATION[i].szPassTime, 0x20, 14);
			memset(g_stGPS_INFO_STATION[i].szLongitude,0x20, 10);
			memset(g_stGPS_INFO_STATION[i].szLatitude, 0x20, 9);
			memset(g_stGPS_INFO_STATION[i].szErrorLog,  0x30, 19);
			memset(g_stGPS_INFO_STATION[i].szPassDis,   0x20, 3);
			memset(g_stGPS_INFO_STATION[i].szSateCnt,   0x20, 2);							
				
		}
		for(i =1; i<= i_order-1;i++)
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
					printf("\n 99 기록 3 \n");			
				}
				
			}
			else // 66 이나 다른 에러 
			{
				
				//memcpy(g_stGPS_INFO_STATION[i].szPass,g_stGPS_INFO_STATION[i_order].szTemp, 2);
				//memcpy(g_stGPS_INFO_STATION[i].szTemp,g_stGPS_INFO_STATION[i_order].szTemp, 2);
				memcpy(g_stGPS_INFO_STATION[i].szPass, "66", 2);
				memcpy(g_stGPS_INFO_STATION[i].szTemp, "66", 2);

			}			
			memcpy(g_stGPS_INFO_STATION[i].szStartTime,g_szRunDepartTime,14);

			//나머지 필드는 모두 초기화 (스페이스 바로)
			memset(g_stGPS_INFO_STATION[i].szIVcnt,0x20, 10);
			memset(g_stGPS_INFO_STATION[i].szPassTime, 0x20, 14);
			memset(g_stGPS_INFO_STATION[i].szLongitude,0x20, 10);
			memset(g_stGPS_INFO_STATION[i].szLatitude, 0x20, 9);
			memset(g_stGPS_INFO_STATION[i].szErrorLog,  0x30, 19);
			memset(g_stGPS_INFO_STATION[i].szPassDis,   0x20, 3);
			memset(g_stGPS_INFO_STATION[i].szSateCnt,   0x20, 2);								
				
		}		

	}


	// 정상적으로 정류장을 인식했을경우 szPass '01'로 Set
	if(i_order != 0)
	{
		printf("\n 정류장 %d 에  '01' 기록\n", i_order);
		memcpy(g_stGPS_INFO_STATION[i_order].szPass, "01", 2);
		memcpy(g_stGPS_INFO_STATION[i_order].szStartTime,g_szRunDepartTime,14);
	}
	////////////////////////////////////////////////////////////////////////////////////////
	
	memcpy(g_stGPS_INFO_STATION[i_order].szSateCnt, 	g_szPassSatelliteCnt, 2);
//	memcpy(g_stGPS_INFO_STATION[i_order].szPassTime, 	szPassMinTime, 14);	ver03.33 까지 적용
	// 단말기 시간이 아닌 gps 모듈 시간으로 기록
	memcpy(g_stGPS_INFO_STATION[i_order].szPassTime, 	g_szGpsCurrTime, 14);	
																		// 03.34 적용 2005-06-08 5:59오후

	sprintf(buff, "%03d", g_nPassMinDis);
	memcpy(g_stGPS_INFO_STATION[i_order].szPassDis, 	buff, 3);

	memset(g_stGPS_INFO_STATION[0].szLongitude,0x20, 10);	
	memset(g_stGPS_INFO_STATION[0].szLatitude,0x20, 9);	
	memset(g_stGPS_INFO_STATION[0].szPassDis,0x20, 3);
	memcpy(g_stGPS_INFO_STATION[0].szPassTime,g_stGPS_INFO_STATION[0].szStartTime, 14);

	
	
	//파일에 기록 한다. 

	//gpsFile = fopen(GPS_INFO_FILE1, "ab+");
	gpsFile2 = fopen(GPS_INFO_FILE2,"ab+");
	
	if(gpsFile2 != NULL)
	{
	//처음에 인식했을때 까지 정류장에 대해서 파일에 기록

		if(i_order != 0)
		{
			// 기점외 출발시
			if(g_nFirstRun == 0)
			{
				printf("\n운행시작 후 첫 인식인 경우\n");
				if(i_order != 1)
				{
					// 처음에 새로 인식한 정류장이 2 인경우 첫번째 정류장 인식로그를 기록한다.
					if(i_order == 2)
					{
						memcpy(g_stGPS_INFO_STATION[1].szPass, "01", 2);
						memset(g_stGPS_INFO_STATION[1].szTemp, 0x20, 2);
						memcpy(g_stGPS_INFO_STATION[1].szIVcnt, g_stGPS_INFO_STATION[2].szIVcnt, 10);
						memcpy(g_stGPS_INFO_STATION[1].szPassTime,g_stGPS_INFO_STATION[2].szPassTime, 14);
						memcpy(g_stGPS_INFO_STATION[1].szLongitude,g_stGPS_INFO_STATION[2].szLongitude, 10);
						memcpy(g_stGPS_INFO_STATION[1].szStartTime, g_stGPS_INFO_STATION[2].szStartTime,14);
							memcpy(g_stGPS_INFO_STATION[1].szLatitude, g_stGPS_INFO_STATION[2].szLatitude, 9);
						
						fwrite(&g_stGPS_INFO_STATION[1], SIMXINFO_SIZE, 1, gpsFile2);
						write22_flag =1;

					}
					else	// 기점외 출발시에 기점부터 현재 order까지 '22'로 Set
					{
						for(i = 1; i < i_order ; i++) //2005-02-18 7:21오후
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
									printf("\n 99 기록 5 \n");			
								}
								
							}
							else
							{
								memcpy(g_stGPS_INFO_STATION[i].szPass, "22", 2);
								memcpy(g_stGPS_INFO_STATION[i].szTemp, "22", 2);
							}
							memset(g_stGPS_INFO_STATION[i].szIVcnt,0x20, 10);
							memset(g_stGPS_INFO_STATION[i].szPassDis,0x20, 3);
							memset(g_stGPS_INFO_STATION[i].szPassTime, 0x20, 14);
							memset(g_stGPS_INFO_STATION[i].szLongitude,0x20, 10);
							memcpy(g_stGPS_INFO_STATION[i].szStartTime, g_szRunDepartTime,14);	
							memset(g_stGPS_INFO_STATION[i].szLatitude,  0x20, 9);	
							memset(g_stGPS_INFO_STATION[i].szSateCnt,   0x20, 2);	
							fwrite(&g_stGPS_INFO_STATION[i], SIMXINFO_SIZE, 1, gpsFile2);
							memset(g_stGPS_INFO_STATION[i].szErrorLog,  0x30, 19);


						}
						write22_flag = 1;
					}
				}
				g_nFirstRun = 1;
			}
		}
		
		// 2005-01-18 12:49오후 기종점 위치및 방위각이 같은 경우에 대한 로그 처리 
		// 인포매틱스 처리.
		///////////////////////중간에 빠진 정류장을 기록한다. ////////////////////////////////
		// 시스템 처음 시작한 경우가 아니면
		if(write22_flag != 1)
		{
			// 중간에 Jump 했을 경우의 에러로그 파일 기록
			if( i_order > g_nPrevStaOrder)
			{
				for(i = g_nPrevStaOrder+1; i <= i_order-1; i++)
				{

					fwrite(&g_stGPS_INFO_STATION[i], SIMXINFO_SIZE, 1, gpsFile2);

					memset(g_stGPS_INFO_STATION[i].szErrorLog,  0x30, 19);
				}
				
			}
			// 기점 부근에서 Jump 했을 경우의 에러로그 파일 기록
			if( i_order < g_nPrevStaOrder)
			{
				
				for(i = g_nPrevStaOrder + 1; i<= g_nTotalStaCnt;i++)
				{
					fwrite(&g_stGPS_INFO_STATION[i], SIMXINFO_SIZE, 1, gpsFile2);	

					memset(g_stGPS_INFO_STATION[i].szErrorLog,  0x30, 19);
				}
				for(i =1; i<= i_order-1;i++)
				{

					fwrite(&g_stGPS_INFO_STATION[i], SIMXINFO_SIZE, 1, gpsFile2);

					memset(g_stGPS_INFO_STATION[i].szErrorLog,  0x30, 19);
				}
			}
		}
		///////////////////////////////////////////////////////////////////////////////////


		//현재 인식한 정류장 을 기록 한다. 
		//printf("\n %d 번째 정류장 인식 로그 기록 \n [%s] \n", i_order, &g_stGPS_INFO_STATION[i_order]);

		// 정상적으로 인식했을 경우 에러로그 파일에 기록한다.
		fwrite(&g_stGPS_INFO_STATION[i_order], SIMXINFO_SIZE, 1, gpsFile2);
		
		memset(g_stGPS_INFO_STATION[i_order].szErrorLog,  0x30, 19);

		fflush(gpsFile2);
		fclose(gpsFile2);
	}


}

int	GetStation(void)
{
	int i_order = 0; //검색된 정류장 순서
	double dis_z1 = 0.; // 검색된 정류장 까지 거리
	int bPassStation = FALSE;	// 정류장을 인식했음. 표시	
	
	////////////////////////////////////
	//현재 버스 정류장 정보 얻어옴
	BusStationDetect(&g_stBusStation[0], &BusInfo, &s_stRMC, 
	                 &g_stGGA, &coordinates, g_nPrevStaOrder);

	// 검색된 정류장 순서로 1부터 시작한다
 	i_order = BusInfo.CurrentStationOrder;
	dis_z1 = BusInfo.CurrentStationDist;	

 	switch (BusInfo.Status)	{
 		case INIT_STATION_SEARCH : 
 			ErrorLogCheck(BusInfo.CurrentStationOrder, 1);		
			bPassStation = TRUE;				
 			break;
		case INIT_STATION_SEARCH_COMPLETE1 : 
			ErrorLogCheck(BusInfo.CurrentStationOrder, 2);	
			bPassStation = TRUE;				
			break;						
		case INIT_STATION_SEARCH_COMPLETE2 : 
			ErrorLogCheck(BusInfo.CurrentStationOrder, 3);
			bPassStation = TRUE;
			break;
		case NEXT_STATION_SEARCH :
			ErrorLogCheck(BusInfo.CurrentStationOrder, 4);		
			bPassStation = TRUE;		
			break;						
		case STATION_VERIFICATION :
			ErrorLogCheck(BusInfo.CurrentStationOrder, 5);
			bPassStation = TRUE;
			break;
		case NEXT_STATION_UPDATE_COMPLETE1 :
			ErrorLogCheck(BusInfo.CurrentStationOrder, 6);
			memcpy(stGPS_INFO_STATION[i_order].szPass, "01", 2);
			memcpy(stGPS_INFO_STATION[i_order].szTemp, "55", 2);				
			bPassStation = TRUE;
			break;
		case NEXT_STATION_UPDATE_COMPLETE2 :
			ErrorLogCheck(BusInfo.CurrentStationOrder, 7);	
			bPassStation = TRUE;
			break;					
		case NEXT_STATION_UPDATE_COMPLETE3 :
			ErrorLogCheck(BusInfo.CurrentStationOrder, 8);	
			bPassStation = TRUE;
			break;
		case STATION_DETECTION_FAIL : 
			ErrorLogCheck(BusInfo.CurrentStationOrder, 9);		
			memcpy(stGPS_INFO_STATION[i_order].szPass, "01", 2);
			
			if((memcmp(stGPS_INFO_STATION[g_nPrevStaOrder].szTemp, "88", 2)!= 0) && (memcmp(stGPS_INFO_STATION[g_nPrevStaOrder].szTemp, "99", 2)!= 0))
			{
				memcpy(stGPS_INFO_STATION[i_order].szTemp, "66", 2);
			}
				
			bPassStation = TRUE;
			break;
		case GPS_INVALID :					
			ErrorLogCheck(BusInfo.CurrentStationOrder, 10);
			break;					
		case GPS_DOP_ERROR :		
			ErrorLogCheck(BusInfo.CurrentStationOrder, 11);
			break;						
		case GPS_INVALID_TIMEOUT :
			ErrorLogCheck(BusInfo.CurrentStationOrder, 14);	
			break;						
		case INIT_DB_ERROR:
			break;
		default:
			bPassStation = TRUE;
 	}

 	if (i_order < 1)	
	{
		return 0;
	}

	
	// 정류장 순서가 전에 인식한 것과 같고, 정류장 아이디도 같은면 무시
	if (g_nPrevStaOrder == i_order)
	{
		return 0;
	}	

	if (bPassStation)  
	{

//		nOldLastSuccessOrder = nLastSuccessOrder;
	 	g_nPrevStaOrder = i_order;

		SaveCurrentStation(i_order, 1);

		printf("\n마지막 인식한 정류장 %d \n",i_order);

		g_nPassMinDis = dis_z1;

		if(InitStationIDFixFlag == 1)
		{
			g_nCurStaOrder = i_order;
			WriteLog();	// gps 로그 파일 저장..

			g_n2PrevInvalidCnt = g_nPrevInvalidCnt;
			g_n2PrevAvailCnt = g_nPrevAvailCnt;
			g_n2PrevTimeOutCnt = g_nPrevTimeOutCnt;
			//로그 파일 정장후에 현재 invalid count값과 valid count값을 이전 값에 저장 한다.
			//로그 파일이 써진다는 의미는 현재 정류장을 갱신 했다는 의미  
			g_nPrevInvalidCnt = g_nInvalidCnt;	//2005-02-18 4:34오후	
			g_nPrevAvailCnt = g_nAvailCnt;  //2005-02-18 4:34오후
			g_nPrevTimeOutCnt = g_nTimeOutCnt; // 2005-02-18 6:19오후

		}
	
    }
    if (i_order<1) i_order = 0;

//	memcpy( gpstSharedInfo->abNowStationID, gpstStationInfo[i_order-1].abStationID, 7 );
//	memcpy( gpstSharedInfo->abNowStationName, gpstStationInfo[i_order-1].abStationName, 16 );
//	memcpy( gabGPSStationID, gpstStationInfo[i_order-1].abStationID, 7 );
	
	return i_order;
}

void CheckingCurrStation(void)
{
	int i=0;
	int nFoundSta[10] = {0,};
	int nFoundOrder = 0;
	int nGab = 0;
	FOUND_STA_INFO stFoundStaInfo;
	

	/*
	 * 운행시작하자마자 현재 버스 위치를 검색한다.
	 * 5번 반복하여 가장 많이 나오는 정류장 순서로 정한다
	 */
	for (i=0; i<5; i++)
	{
		GpsDataProcessing();
		if (g_nTotalStaCnt > 0 && g_gpsStatusFlag == GPS_STATUS_OK)
		{
			nFoundSta[i] = BusStationDetecting(&stFoundStaInfo, s_stRMC.Lat, 
		                                 s_stRMC.Lon, s_stRMC.UTCTime, 
		                                 s_stRMC.SpeedKn);
		}
	}

	nFoundOrder = GetConstantlyNumber(nFoundSta, 5);

	nGab = abs(g_nPrevStaOrder - nFoundOrder);
	
	if (g_nPrevStaOrder == nFoundOrder || nFoundOrder == 0)
		return;

	if (nFoundOrder == 1)	{
		g_nPrevStaOrder = 1;
		g_nCurStaOrder = 1;
	}
	else if (g_nPrevStaOrder < 4 || g_nTotalStaCnt-3 < g_nPrevStaOrder) {
		g_nPrevStaOrder = 1;
		g_nCurStaOrder = 1;
	}
	else if ( nGab < g_nTotalStaCnt-3 && nGab > 3 ) {
		// 정류장을 확인해 주시기 바랍니다 출력
		VoiceOut( 45 );	
	}
	else
		;
	
}

// 정류장과 정류장 사이에 발생한 인식 및 인지 또는 GPS 상태, 초기 DB 오류에 대한 Flag 설정함수
void ErrorLogCheck(unsigned int Order, unsigned int Index)
{
//	char strbuff[5];
	memcpy(&stGPS_INFO_STATION[Order].szErrorLog[Index-1], "1", 1);

}