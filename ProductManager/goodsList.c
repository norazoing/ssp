/*
 * goodsList.c
 *
 *  Created on: 2017. 4. 13.
 *      Author: LGCNS
 */
#include<stdio.h>
#include<stdlib.h>
#include<glib.h>
#include<math.h>
#include "./goodsList.h"
#include<string.h>
GList *list =NULL;

static void _free_fun(gpointer data)
{
	if(data)
		free(data);
}
gint sorter(gconstpointer a, gconstpointer b, gpointer data)
{
	return strcasecmp(((GOODS_INFO*)a)->abName, ((GOODS_INFO*)b)->abName);
}
void addGoods(GOODS_INFO inputGoods)
{
	GOODS_INFO *newGoods;
	newGoods =(GOODS_INFO*)malloc(sizeof(GOODS_INFO));
	memcpy(newGoods, &inputGoods, sizeof(GOODS_INFO));
	list = g_list_append( list, newGoods);

	FILE * file;

	file = fopen("goodslist.txt","ab+");

	if(file)
	{
		fwrite(newGoods,sizeof(GOODS_INFO),1, file);
		fclose(file);
	}

}
void deleteGoods(char* abName)
{
	GList* curr;
	for(curr = list; curr!= NULL; curr= curr->next)
	{
		if(memcmp(((GOODS_INFO*)curr->data)->abName, abName, sizeof(((GOODS_INFO*)curr->data)->abName))==0)
		{
			list= g_list_delete_link( list, curr );
			break;
		}

	}

}
void searchGoods(char* abName)
{
	GList* curr;
	for(curr = list; curr!= NULL; curr= curr->next)
	{
		if(memcmp(((GOODS_INFO*)curr->data)->abName, abName, sizeof(((GOODS_INFO*)curr->data)->abName))==0)
		{
			printGoodsRecord((GOODS_INFO*)curr->data);
			break;
		}

	}

}
void sortGoodsbyName()
{
	list =g_list_sort(list, (GCompareDataFunc)sorter);

}

void updateGoods(char* abName,char* abName2)
{
	GList* curr;
	for(curr = list; curr!= NULL; curr= curr->next)
	{
		if(memcmp(((GOODS_INFO*)curr->data)->abName, abName,sizeof(((GOODS_INFO*)curr->data)->abName))==0)
		{
			memset(((GOODS_INFO*)curr->data)->abName,0x00,sizeof(((GOODS_INFO*)curr->data)->abName));
			memcpy(((GOODS_INFO*)curr->data)->abName,abName2,strlen(abName2));
			break;
		}

	}

}
void loadGoods()
{
	FILE * file;
	int nResult=0;
	file = fopen("goodslist.txt","rb");

	GOODS_INFO *newGoods;

	if(file)
	{
		do
		{
			newGoods =(GOODS_INFO*)malloc(sizeof(GOODS_INFO));
			nResult =fread(newGoods,sizeof(GOODS_INFO),1, file);
			if(nResult <= 0)
				break;

			list = g_list_append( list, newGoods);

		}
		while(nResult);

	}

	if(file)
		fclose(file);
}
void printGoods()
{
	GList* curr;
	time_t tNow;
	struct tm timeInfo;

	int nResult=0;

	for(curr = list; curr!= NULL; curr= curr->next)
	{
		printGoodsRecord((GOODS_INFO*)curr->data);
	}

}
void printGoodsRecord(GOODS_INFO* data)
{
	char abASCTime[15]={0,};
	printf("%.15s\t",data->abName);
	printf("%d\t",data->nCount);
	printf("%.2lf\t",data->dPrice);
	printf("%.2lf\n",data->dDisrate);
	Timet2ASC( data->tRegistorDate,abASCTime, 0, 14);
	printf("%s\n",abASCTime);
}
void destroyGoods()
{
	g_list_free_full(list,_free_fun);

}
int getGoods(GOODS_INFO** goodsList)
{

	int nCount=0;
	int nIndex =0;
	GList* curr;
	for(curr = list; curr!= NULL; curr= curr->next)
	{
		nCount++;
	}
	*goodsList = (GOODS_INFO*)malloc(sizeof(GOODS_INFO)*nCount);

	for(curr = list; curr!= NULL; curr= curr->next)
	{
		memcpy(&(*goodsList)[nIndex++],(GOODS_INFO*)curr->data,sizeof(GOODS_INFO));

	}
	//for(int i = 0; i<sizeof(GOODS_INFO);i++)
	//{

	//}
	return nCount;
}
