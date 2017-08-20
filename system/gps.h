#ifndef _GPS_
#define _GPS_

#include "simx.h"
#include "gps_env.h"


// 김재의 추가 : 정류장 통과 여부 파일
#define GPS_INFO_FILE	"gps_info.dat"
#define GPS_INFO_FILE1	"simxinfo.dat"
#define GPS_INFO_FILE2  "simxinfo2.tmp"
#define GPS_ENVS		"gps_envs.dat"

// 버스 운행 회수 산정용 파일명들
#define TURN_CNT_FILE		"turn_cnt.dat"	// 하루 기준으로 버스가 운행한 횟수.
#define LAST_PASS_FILE		"lastpass.dat"	// 마지막 통과한 정류장 정보 파일.
#define PREV_PASS_FILE		"prevpass.dat"	// 직전 인식 정류장 정보 -- GPS 3.0
#define DRIVE_DIST_TMP		"drivedis.tmp"
#define DRIVE_DIST_FILE		"drivedis.dat"
#define GPS_INFO_BACKUP		"gps_info.bak"
#define LOG_INFO_FILE		"simxlog.dat"	// GPS 쓰레드에서 발생하는 에러에 대한 모든 기록을 함.
#define RAW_DATA_LOG		"raw_data.log"	// GPS 수신 데이타의 단계별 카운트 저장
#define GPS_INFO_TMP		"gps_info.tmp"	// 파일 합하는용 파일버퍼
#define GPS_RAW_FILE		"gps_raw.txt"	// 시뮬레이션용 파일명
#define DRIVE_INFO	        "drive_info.txt"


int GetGpsData(int fd, char* strGGA, char* strRMC);
int ParseRMC(RMC_DATA* rmc, unsigned char *Data, int DataNum);
void WriteTrace(RMC_DATA* rmc);
//extern driveFlag; //운행 시작 시간 설정 완료 플래그 '1' : 운행 시작 시간이 없음 , '0' : 운행 시작 시간이 들어 왔음 

extern int gpsRecv(int fd, unsigned char* buffer, int size, int timeout);
extern int gpsPktRecv(int fd, int size, int timeout);
extern int gpsOpen(int baudrate);
extern void *gpsMain(void *arg);
extern int gpstime_check(void);
extern void gpstime_convert (char *gps_time);
extern int GpsTimeCheck(void);
int ParseGGA(GGA_DATA* gga, unsigned char *Data, int DataNum);


extern GPS_DATA GPS;
extern GPS_INFO_STATION g_stGPS_INFO_STATION[MAXSTATIONCNT];

extern STA_INFO_LOAD	STA_IF_LOAD[256];

extern unsigned int g_nSelectFailCnt;	// Select Fail Count
extern unsigned int g_nTimeOutCnt;		// TimeOut	Count
extern unsigned int g_nCheckSumCnt;		// CheckSum Error Count
extern unsigned int g_nInvalidCnt;		// Invalid Count	'V'
extern unsigned int g_nAvailCnt;		// Avail Count	'A'

extern unsigned int g_nPrevInvalidCnt;	// Previous invalid count
extern unsigned int g_nPrevAvailCnt;     // previous avail count
extern unsigned int g_nPrevTimeOutCnt;   // previous time out count
extern unsigned int g_nPrevCheckSumCnt;	// previous checksum count

extern unsigned int g_n2PrevInvalidCnt;      // 
extern unsigned int g_n2PrevAvailCnt;        //
extern unsigned int g_n2PrevTimeOutCnt;

extern GPS_STATUS_CNT g_stGpsStatusCnt[MAXSTATIONCNT];

extern int g_nTotalStaCnt;		// 정류장 총 수
extern int g_nContinueFlag;		// 껐다 킨 경우 운행연속인가(1), 아닌가(0)?


extern int g_nPassMinDis;				// 정류장통과시 최소거리
extern char g_szPassMinLongitude[10];	
extern char g_szPassMinLatitude[9];	

extern char g_szVehcID[10];	// 차량 id
extern char g_szBusRouteID[9];	// 노선 id
	
extern char g_szPassMinTime[15];			
extern char g_szPassSatelliteCnt[3];	

extern int g_nNextStaOrder;
extern int g_nPrevStaOrder;
extern int g_nCurStaOrder;


extern char szGpsLogStartTime[15];	
extern char szGpsLogEndTime[15];	
extern char g_szRunDepartTime[15];	// 운행시작시간
extern int g_gps_fd;				// gps 포트 핸들
extern int g_bInc;
extern int g_nFirstRun;	// 운행시작후 정류장 검색이 첨인지?

//GPS 2.7 적용. 모듈에서 보내온 시간 (단말기 시간 아님.)
extern char g_szGpsCurrTime[15];

extern int g_nGpsExceptCnt;	// 비정상 운행된 정류장 총 수

extern byte g_gpsStatusFlag; // 현재 gps 수신상태 플래그 선언

extern double g_dbMaxSpeed;	// 운행시작 후 검지된 최대 속도값

extern int	nMoveDis;

// 통합, 2.7 모두 수용하기 위한 중간 버퍼
extern COMM_STA_INFO g_stStaInfo[MAXSTATIONCNT];

extern BUS_STATION	g_stBusStation[MAXSTATIONCNT];
extern double g_dbRoundX;	// 정류장 검색 반경 X'
extern double g_dbRoundY;	// 정류장 검색 반경 Y'

extern SORT_X	g_stSortX[MAXSTATIONCNT];
extern SORT_Y	g_stSortY[MAXSTATIONCNT];

extern RMC_DATA	s_stRMC;				// RMC 데이터 저장 구조체

extern BUSINFO			BusInfo;				// 정류장 인식 정보 저장 구조체(BusStationDetect함수의 리턴값)

extern RMC_DATA		s_stRMC;				// RMC 데이터 저장 구조체
extern GGA_DATA		g_stGGA;				// GGA 데이터 저장 구조체
extern COORDINATES		coordinates;			// 좌표변환 구조체
extern GPS_INFO_STATION stGPS_INFO_STATION[MAXSTATIONCNT];

extern char g_szStaFindTime[15];	// 현재 정류장 인식 시간

#endif
