
#ifndef _SIMX_
#define _SIMX_

//#include "gps_env.h"


//////////////////////////////////////////////
//GPS Station Detection Status
//
#define INIT_STATION_SEARCH 1  //초기 정류장 탐색 중
#define INIT_STATION_SEARCH_COMPLETE1 2  //초기 정류장 탐색 완료(운행중)
#define INIT_STATION_SEARCH_COMPLETE2 3  //초기 정류장 탐색 완료(1번정류장 검색)
#define NEXT_STATION_SEARCH 4  //다음 정류장 검색중
#define STATION_VERIFICATION 5  //현재 정류장 검증 상태
#define NEXT_STATION_UPDATE_COMPLETE1 6  //다음 정류장으로 
#define NEXT_STATION_UPDATE_COMPLETE2 7 //
#define NEXT_STATION_UPDATE_COMPLETE3 8
#define STATION_DETECTION_FAIL 9

#define GPS_INVALID 10
#define GPS_DOP_ERROR 11
#define GPS_UN_USEDATA 12
#define GPS_DB_HEADING_ERROR 13

#define GPS_INVALID_TIMEOUT 14
#define INIT_DB_ERROR 15
//////////////////////////////////////////////

//Threshold value
#define GPS_TIMEOUT 15  //GPS timeout value threshold
#define DOP_THRESHOLD 10  //dop error threshold
//////////////////////////////////////////////

// global parameter
#define D2R 0.01745329251994   
#define R2D 57.29577951308232


#define DIFF_DIST_P_FIRST	40  //초기 db 데이터 누적 거리
#define DIFF_DIST_P			0.7  //db percentage


/////////////////////////////////////////////////////////////////////////
//rmc data
typedef struct {

	double	UTCTime;
	char	PosStatus;
	double	Lat;
	char 	LatDir;
	double	Lon;
	char 	LonDir;
	double	SpeedKn;
	double	TrackTrue;
	double	date;
} __attribute__((packed)) RMC_DATA;

typedef struct
{
       byte start_st;
	byte gps_st[2];
	byte data_kind[3];
	byte utc_world_time[11];
	byte use_kind[2];
	byte lattitude[10];
	byte north_south[2];
	byte longitude[11];
	byte east_west[2];
	byte knot_speed[7];
	byte heading[7];	
	byte date[7];
	byte adjust_dec[5];
	byte west_dod[2];
	byte schecksum[2];
	byte dchecksum[3];	
	byte crlf[3];
} __attribute__((packed)) GPS_DATA;

//버스 정류장 구조체 
typedef struct {
	
	unsigned int StationID; //ID
	unsigned int StationOrder; //Station order
	unsigned int DBLen;  //Station number
	double LonDeg;  //Station longitude
	double LatDeg;  //Station latitude
	double Angle;  //Station Angle
	double AccumDist; //Station Accumulated Distance
	double DiffDist;  //70% Distance between current station and previous station 
	double DiffDist2; // Distance between current station and previous station 
	double ENU[3];  //ENU value
} __attribute__((packed)) BUS_STATION;  


typedef struct{
	
	double		AllStationMinDist[256];	//전체 Station에 대한 통과시 최소 거리 저장 
	unsigned int	DBLen;  				//DB의 총 개수 
	unsigned int	CurrentStationID;                       //현재 정류장 ID
	unsigned int	CurrentStationOrder;                    //현재 정류장 순서
	unsigned int	NextStationID;                          //다음 정류장 ID
	unsigned int	NextStationOrder;                       //다은 정류장 순서
	unsigned int	Status;  				//상태 코드 
	double		CurrentStationDist; 			//현재 Station과의  거리 
	double		CurrentVelms;                           //현재 버스 속도(m/s)
	double		CurrentPosLat;                          //현재 버스 위도
	double		CurrentPosLon;                          //현재 버스 경도
	double		CurrentPosENU[3];                       //현재 버스 ENU 값 
	double		CurrentHeading;                         //현재 버스 헤딩
	double		CurrentHeadingError;                    //현재 정류장과 버스 헤딩과의 차이값
	unsigned int	GPSValidFlag;				// GPS Valid Flag
	double		MinStationUTCTime;			//최소 거리에서의 UTC time 


} __attribute__((packed)) BUSINFO;

//cordinate value
typedef struct {
	
	double OrgLLH[3];
	double OrgXYZ[3];  
} __attribute__((packed)) COORDINATES;



//gga data
typedef struct {
	double UTCTime;			//UTC of Position
	double Lat;	   		//Latitude  (DDmm.mm)
	char   LatDir;			//N or S
	double Lon;			//Longitude (DDmm.mm)
	char   LonDir;			//E or W
	int    GPSQual;			//GPS quality indicator (0=invalid; 1=GPS fix; 2=Diff. GPS fix)
	int    SatNum;			//Number of satellites in use [not those in view]
	double HDOP;			//Horizontal dilution of position
	double Altitude;		//Antenna altitude above/below mean sea level (geoid)
} __attribute__((packed)) GGA_DATA;



typedef struct				
{
        byte  appltn_dtime[14];          // 적용일시(BCD)
        byte  appltn_seq[2];               // 적용일련번호(BCD)
        byte  bus_route_id[8];          // 버스노선 ID
        byte  transp_method_cd[16];     // 버스노선명
        byte  bus_sta_cnt[3];           // 버스정류장갯수
        byte  bus_sta_id[7];            // 버스정류장 ID
        byte  city_bnd_in_out_class_cd; // 시계내외구분 코드
        byte  bus_sta_order[3];            // 정류장순서
        byte  bus_sta_pos_nm[16];       // 버스정류장 명
        byte  bus_sta_pos_x[10];        // 버스정류장좌표 X
        byte  bus_sta_pos_y[9];        // 버스정류장좌표 Y
        byte  dist_6[3];                   // Off Set
        byte  bus_sta_cum_dist[6];      // 첫정류장에서의 거리(BCD)
        byte  heading[3];
} __attribute__((packed)) STA_INFO_LOAD;              // 정류장정보 파일 로딩

extern unsigned int InitStationIDFixFlag;  // 초기 정류장을 잡고 나면 1로 셋팅

/*************************************************************************************
 *		
 *	function: PreProcess
 *	Param: sBus_station : 버스 정류장 정보를 저장할 구조체 
 *		nSTA_IF_LOAD: 버스 정류장 정보 저장 구조체(아스키 형태),
 *		coordinates : 좌표값
 *      Return: zero
 *	discription: 버스정류장 인식 알고리즘을 수행하기전에 전역 변수 초기화와 DB데이터를 읽어옴  
 *	note: 
 **************************************************************************************/
extern unsigned int PreProcess(BUS_STATION *sBus_station, STA_INFO_LOAD* nSTA_IF_LOAD, COORDINATES* coordinates);



/*************************************************************************************
 *		
 *	function: DBManage
 *	Param: sBus_station : 버스 정류장 정보를 저장할 구조체 
 *      Return: zero
 *	discription: 인식 알고리즘에서 사용할 DB데이터를 가공 한다.  
 *	note: 
 **************************************************************************************/
extern unsigned int DBManage(BUS_STATION *sBus_station);




/*************************************************************************************
 *		
 *	function: GPSCorr
 *	Param: sBus_station : gps : gps data(아스키) , rmc : rmc데이타 
 *      Return: zero
 *	discription: 인식 알고리즘에서 사용할 GPS데이터를 보정 한다. 
 *      어있는 gga도 사용한다. 
 *	note: 
 **************************************************************************************/
extern unsigned int GPSCorr(GPS_DATA *gps, RMC_DATA *rmc,GGA_DATA* gga);




/*************************************************************************************
 *		
 *	function: BusStationDetect
 *	Param: sBus_station : 버스 정류장 정보를 저장할 구조체
 *              spBusInfo : 버스 상태및 정류장 번호 저장 구조체 
 *              rmc : rmc 데이타 
 *              gga : gga 데이타 
 *              coordinates : 좌표값 
 *              InitStationOrder : 초기 정류장 셋팅값 
 *      Return: zero
 *	discription: 버스 정류장 인식 알고리즘 
 *	note: 
 **************************************************************************************/
extern unsigned int BusStationDetect(BUS_STATION* sBus_station, BUSINFO *spBusInfo, RMC_DATA* rmc, GGA_DATA* gga, COORDINATES* coordinates, unsigned int InitStationOrder);



/*************************************************************************************
 *		
 *	function: WriteLogSimx
 *	Param: rmc : rmc 데이터
 *             spBusInfo : 버스 데이터 구조체 포이터  
 *      Return: zero
 *	discription: 수신기에서 들어오는 Raw데이터를 쓴다. 이 함수의 목적은 나중에
 *                   주행 괘적을 그리거나 알고리즘 테스트용도이기 때문에 가공되지 않는 
 *                   데이터를 쓴다.   
 *	note: 
 **************************************************************************************/
extern unsigned int WriteLogSimx(RMC_DATA* rmc,BUSINFO *spBusInfo);  //500k simxlog를 쓰는 함수 



extern GPS_DATA GPS;

#endif
