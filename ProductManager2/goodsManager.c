/*
 * goodsManager.c
 *
 *  Created on: 2017. 4. 10.
 *      Author: LGCNS
 */

#include<stdio.h>
#include<glib.h>
#include<stdlib.h>
#include<pthread.h>
//#include"./server.h"
#include "./goodsList.h"
void __attribute__((constructor))console_setting_for_eclipse_debugging(void)
{
	setvbuf(stdout,NULL, _IONBF,0);
	setvbuf(stderr,NULL, _IONBF,0);
}
void* checkValidDuration();
void* inputMenuLoop();
void printMenu();


int main()
{
	pthread_t thread;
    pthread_t timer_thread;
	int status;
    int status2;

	loadGoods();


    pthread_create(&timer_thread, NULL, checkValidDuration, NULL);
    pthread_create(&thread, NULL, inputMenuLoop, NULL);
    pthread_join(thread, (void**)&status);
    pthread_join(timer_thread, (void**)&status2);



	return 0;

}
void printMenu()
{
	printf("===============================\n");
	printf("=1. add goods                 =\n");
	printf("=2. display goods             =\n");
	printf("=3. search goods              =\n");
	printf("=4. delete goods              =\n");
	printf("=5. update goods              =\n");
	printf("=6. sort goods                =\n");
    printf("=input 'q' for exit           =\n");
	printf("===============================\n");
    printf ("Enter Menu No.:");
}


void * checkValidDuration()
{
    while(1) {

        CheckValidGoods();
        sleep(3);
    }
}




void* inputMenuLoop()
{
	char choice;
	char abBuf[256]={0,};
	char abBuf2[256]={0,};
	do
	{
		printMenu();
		memset(abBuf, 0x00, sizeof(GOODS_INFO));
		memset(abBuf2, 0x00, sizeof(GOODS_INFO));
        scanf(" %c",&choice);

		switch(choice)
		{
		case '1':
			printf(" add goods \n");
			GOODS_INFO newGoods;
			memset(&newGoods, 0x00, sizeof(GOODS_INFO));
			scanf("%s %d %lf %lf",newGoods.abName, &newGoods.nCount, &newGoods.dPrice, &newGoods.dDisrate);
			time(&newGoods.tRegistorDate);
			addGoods(newGoods);
			break;
		case '2':
			printf(" display goods \n");
			printGoods();
			break;
		case '3':
			printf(" input search goods name\n");

			scanf("%s",abBuf);
			searchGoods(abBuf);
			break;
		case '4':
			printf(" input delete goods name \n");
			scanf("%s",abBuf);
			deleteGoods(abBuf);
			break;
		case '5':
			printf(" input orginal name\n");
			scanf("%s",abBuf);
			printf(" input replace name\n");
			scanf("%s",abBuf2);
			updateGoods(abBuf,abBuf2);
			break;
		case '6':
			sortGoodsbyName();
			break;
		}

	}
    while(  choice != 'q' );

	printf(" quit program\n");
	destroyGoods();
	exit(1);
}

