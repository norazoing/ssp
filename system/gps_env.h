#ifndef _GPS_ENV_
#define _GPS_ENV_




#define LOG_MAX_SIZE	2097152L	// 로그파일 최대 크기

#define recvData_size 	1024
#define GPS_BPS_SPEED	9600

#define LINE_MAX_BUFFER		255		// 버퍼라인 최대값
#define GPSTIMEOUT			2		// GPS 타임아웃 시간 초
#define MAXSTATIONCNT		256		// 최대 한노선의 정류장 수
#define SIMXINFO_SIZE		118		// SIMXINFO 로그 파일 레코드 사이즈
#define MaxFileSize (1024*1024)  	// simx log file size
#define AVAIL_CNT_TH		30.0   	// avail count threshold
#define SEARCH_ROUND_VALUE	5		// 정류장 검색 반경 값을 구하기 위한 상수값
#define TIME_CHECK_GAP		10		// 시간 보정 간격

#define SEARCH_ROUND_DIS	50
#define SEARCH_ROUND_ANGLE	40

#define GPS_STATUS_OK		0
#define GPS_INVALID_DATA	1
#define GPS_CONN_OUT		2
#define DIST_OVER			3

#define MAX_BOUND_HDOP		4.5		// 수신 데이타 신뢰성 확보를 위해

#define TIME_BOUND_SEARCH	100	// 다음 정류자을 검색하기까지 기다리는 최대시간
								// 다음 정류장까지 거리를 기준으로 이동시간을 현 속도대비 측정
								
#define SPEED_BOUND_SEARCH	0	// 초속 5 m/s 이상되어야 정류장 검색을 함
#define DEFAULT_MAX_SPEED		5	// 초속 5 m/s 기준속도로
#define GPS_STATUS_VALID_BOUND	40	// GPS 상태 비정산 판단 기준값
#define GPS_STA_CNT_REF_BOUND	3	// 최근 3개의 정류장을 인식했을 때의 누적 수신별 상태 값

#define DISTANCE_MAX_TIMES	3	// 기준거리 보다 몇배 이상 떨어졌나?

#define GPS_DEV_NAME		"/dev/ttyE1"
#define BAUD_RATE			9600

#define CANNOT_FIND_NEW_STATION	0

/////////////////////////////////////////////////////////////////////////
// 2004.09.25
// 김재의 추가
// 버스 운행거리 산정용
// 파일명 : drivedis.tmp
typedef struct _PASSDIST
{
	int sta_order;	// 정류장 통과 순서번호
	int dist;		// 현재 통과한 정류장까지의 운행거리
}__attribute__((packed)) PASSDIST, *LPPASSDIST;

/////////////////////////////////////////////////////////////////////////
// 2004.09.25
// 김재의 추가
// 버스 운행거리 산정 파일저장용 
// 파일명 : drivedis.dat
typedef struct _DRIVEDIST
{
	char date[14];	// 운행시작일시
	int  dist;		// 운행시작부터 종료시까지 계산한 운행거리
}__attribute__((packed)) DRIVEDIST, *LPDRIVEDIST;

/////////////////////////////////////////////////////////////////////////
// 2004.08.17
// 김재의 추가
// 버스 운행 횟수 카운트용
typedef struct _TURN_CNT_STATION
{
	char szDate[8];
	int	nTotalCnt;	// 하루 전체 카운트
	int	nCnt;		// 운행종료시 가져가는 카운트
}__attribute__((packed)) TURN_CNT_STATION, *LPTURN_CNT_STATION;


/////////////////////////////////////////////////////////////////////////
// 2004.07.10 
// 김재의 추가
// 버스정류장 통과 여부 파일생성
/////////////////////////////////////////////////////////////////////////
// 2004.12.28
// 류지훈 수정
// 버스 정류장 통과 여부 및 에러 로그 파일 생성
/////////////////////////////////////////////////////////////////////////
typedef struct  _GPS_INFO_STATION
{
	char szLineNum[8];		// 버스노선ID
	char szBusID[9];		// 버스차량ID
	char szStartTime[14];
	char szOrder[3];		// 정류장순서
	char szID[7];			// 정류장 ID
	char szSerialNum[4];            // Serial number
	char szLongitude[10];	// 위도
	char szLatitude[9];		// 경도
	char szPassDis[3];		// 통과시 최소거리
	char szPass[2];			// 통과여부 1: 통과
	char szPassTime[14];	// 통과당시시간
	char szSateCnt[2];		// 위성수
	char szIVcnt[10];		// Invalid cnt : 5바이트, Valid cnt : 5바이트, 12월 28일 Jeff 추가 
	char szErrorLog[19];		// 정류장명칭	16바이트: 명칭,  남은 16바이트:운행시작시간,각 데이타 수신시간, 운행종료시간	
	char szTemp[4];


} __attribute__((packed)) GPS_INFO_STATION, *LPGPS_INFO_STATION;
/////////////////////////////////////////////////////////////////////////



typedef struct
{
	unsigned char order;
	unsigned char pass;
}__attribute__((packed)) ACCUM_DIST_DATA;

typedef struct
{
	byte station_id[7];
	byte station_nm[16];
}__attribute__((packed)) FOUND_STA_INFO;

// X좌표에 해당하는 정류장 별 기점 기준 가까운 순으로 정렬때 사용하기 위함.
typedef struct
{
	double 	dbX;
	int		nOrder;
}__attribute__((packed)) SORT_X;

// Y좌표에 해당하는 정류장 별 기점 기준 가까운 순으로 정렬때 사용하기 위함.
typedef struct
{
	double 	dbY;
	int		nOrder;
}__attribute__((packed)) SORT_Y;


typedef struct
{
	byte abStationID[7];				// 버스정류장 ID
	byte bCityInOutClassCode;			// 시계내외구분 코드
	byte bStationOrder[3];				// 정류장순서
	byte abStationName[16];				// 버스정류장명
	double dLongitude;			// 버스정류장 경도
	double dLatitude;			// 버스정류장 위도
	word wOffset;						// offset
	dword dwDist;		// 첫정류장에서의 거리
	word wAngle;			// 정류장 진입각
} __attribute__((packed)) COMM_STA_INFO;

typedef struct
{
	int nValidCnt;		// 정상 수치 카운트
	int nInvalidCnt;	// 감도 저하 수치 카운트
	int nTimeoutCnt;	// 타임아웃 카운트
} __attribute__((packed)) GPS_STATUS_CNT;

int GpsPortOpen(char* dev, int baudrate, int lineread);
void DataStatusSet(int param);
void GpsDataLogRead(void);
void GpsDataLogWrite(void);
void GlobalParameterInit(void);

#endif