/*
 * time.c
 *
 *  Created on: 2017. 4. 16.
 *      Author: ¿Ãº“¡§
 */
#include<stdio.h>
#include<time.h>

int main()
{
	time_t tNow, tTomorrow;
	struct tm timeInfo;

	time(&tNow);
	localtime_r(&tNow,&timeInfo);

	printf("%d %d %d %d \n",timeInfo.tm_year+1900,timeInfo.tm_mon+1,timeInfo.tm_mday,timeInfo.tm_hour);
	printf("%s\n",asctime(&timeInfo));


	timeInfo.tm_mday+=1;

	tTomorrow= mktime(&timeInfo);
	localtime_r(&tTomorrow,&timeInfo);

	printf("%d %d %d %d \n",timeInfo.tm_year+1900,timeInfo.tm_mon+1,timeInfo.tm_mday,timeInfo.tm_hour);
	printf("%s\n",asctime(&timeInfo));
	return 0;
}
