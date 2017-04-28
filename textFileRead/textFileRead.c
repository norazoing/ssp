/*
 * textFileRead.c

 *
 *  Created on: 2017. 4. 28.
 *      Author: LGCNS
 */
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
/*void __attribute__((constructor))console_setting_for_eclipse_debugging(void)
{
	setvbuf(stdout,NULL, _IONBF,0);
	setvbuf(stderr,NULL, _IONBF,0);
}*/

typedef struct GOODS
{
	char abName[15];
	int  nCount;
	double dPrice;
	double dDisrate;
	time_t tRegistorDate;
}GOODS_INFO;

GOODS_INFO* gGoodsInfo =NULL;
int nGoodsCount=0;
void main()
{
	loadTextFile();
	printGoods();
}
void loadTextFile()
{
	FILE * pFile;
	char abBuf[1024]={0,};

	char* token;
	char* saveptr1;

	pFile = fopen("itemlist.txt","r");

	if( pFile != NULL )
	{
		char *pStr;


		while( !feof( pFile ) )
		{
			memset(abBuf, 0x00, sizeof(abBuf));
			pStr = fgets( abBuf, sizeof(abBuf), pFile );
			if(!pStr)
				break;
			gGoodsInfo = (GOODS_INFO*)realloc(gGoodsInfo,sizeof(GOODS_INFO)*(nGoodsCount+1));
			memset(&gGoodsInfo[nGoodsCount], 0x00, sizeof(GOODS_INFO));
			token = strtok_r(abBuf, "\t\r\n", &saveptr1);

			//만약 동일한 내용 처리하려면 아래 주석 풀고 하시오
			/*while(token)
			{
				printf("[%s]",token);
				token = strtok_r(NULL, "\t\r\n", &saveptr1);

			}*/
			if(!token)
				continue;

			memcpy( gGoodsInfo[nGoodsCount].abName, token, strlen(token));
			token = strtok_r(NULL, "\t\r\n", &saveptr1);

			if(!token)
				continue;

			gGoodsInfo[nGoodsCount].nCount = atoi(token);
			token = strtok_r(NULL, "\t\r\n", &saveptr1);

			if(!token)
				continue;

			gGoodsInfo[nGoodsCount].dPrice = atof(token);
			token = strtok_r(NULL, "\t\r\n", &saveptr1);

			if(!token)
				continue;
			gGoodsInfo[nGoodsCount].dDisrate = atof(token);
			nGoodsCount++;
		}
		fclose( pFile );
	}
	else
	{
		//에러 처리

	}
}
void printGoods()
{
	int i;
	for( i=0 ; i< nGoodsCount; i++ )
	{
		printGoodsRecord(&gGoodsInfo[i]);
		printf("\n");
	}

}
void printGoodsRecord(GOODS_INFO* data)
{
	char abASCTime[15]={0,};
	printf("%.15s\t",data->abName);
	printf("%d\t",data->nCount);
	printf("%.2lf\t",data->dPrice);
	printf("%.2lf\t",data->dDisrate);

}
