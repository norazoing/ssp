#ifndef _GPS_UTIL_
#define _GPS_UTIL_

double AtoDouble(char* str, int len);

#ifndef	_GPS_27_
	int atointeger (byte *s, byte length);
	void EndStationAdviceMsg( void );
#endif

void BubbleSortStaInfo(void);
double GetDis(double x1, double y1, double x2, double y2);
long getFileSize(char* f);
int gpstime_check(void);
void gpstime_convert (char *gps_time);

int GpsTimeCheck(void);
int HeadingErrorABS_D(int Ang, int RefAng);
void LatLonHgtToXYZ_D(double llh[3],double  xyz[3]);
int XyztoENU_D(double xyz[3], double orgxyz[3],double LLA[3],double ENU[3]);
size_t UsrFwrite(const void *data, size_t size, size_t count, FILE *stream);
int IsOverDistance(int n, float dis);
int GetMaxTimeforNextStaSearch(void);
double GetGpsActStatus(void);
int GpsDataProcessing(void);
void GetStationInfoForNewLogic(void);
int GetConstantlyNumber(int nFoundSta[], int cnt);
void DoubleToASC(double src, byte* desc);

#endif