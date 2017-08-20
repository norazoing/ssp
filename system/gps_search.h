#ifndef _GPS_SEARCH_
#define _GPS_SEARCH_

#include "simx.h"

int BusStationDetecting(FOUND_STA_INFO *stFoundStaInfo, double x, double y, 
                                                   double utctime, double speed);
int DecideStation(int nRecogOrder, double dbNextStaOrderDis);
void GetShortestStaOrder(double curX, double curY, int *nMinOrder, double *dbDis);
int GetStartOrder(int nMinOrder, double dbDis);
int RecognizeStation(double dbCurStaOrderDis, double dbNextStaOrderDis, 
					 double dbShortDis, int nMinOrder, int nCurStaOrder, 
					 int nNextStaOrder);
void SaveCurrentStation(int nDecidedOrder, int nContinueFlag);
int SetInitStaOrder(int nMinOrder, double dbDis);
int VirturalDetectProcess(int nDetectStaOrder);
void WriteLog(void);
int	GetStation(void);
void CheckingCurrStation(void);

#endif