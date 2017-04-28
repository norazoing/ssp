/*
 * goodsList.c
 *
 *  Created on: 2017. 4. 13.
 *      Author: LGCNS
 */
#include<stdio.h>
#include<stdlib.h>
#include<glib.h>
#include "./goodsList.h"
GList *list =NULL;
void addGoods()
{
	GOODS_INFO *newGoods;
	newGoods =(GOODS_INFO*)malloc(sizeof(GOODS_INFO));
	memset(newGoods, 0x00, sizeof(GOODS_INFO));
	scanf("%s %s %s %s",newGoods->abName, newGoods->abCount, newGoods->abPrice, newGoods->abDate);

	list = g_list_append( list, newGoods);

	FILE * file;

	file = fopen("goodslist.txt","ab+");

	if(file)
	{
		fwrite(newGoods,sizeof(GOODS_INFO),1, file);
		fclose(file);
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

	int nResult=0;

	for(curr = list; curr!= NULL; curr= curr->next)
	{
		printf("%.15s\t",((GOODS_INFO*)curr->data)->abName);
		printf("%.3s\t",((GOODS_INFO*)curr->data)->abCount);
		printf("%.7s\t",((GOODS_INFO*)curr->data)->abPrice);
		printf("%.8s\n",((GOODS_INFO*)curr->data)->abDate);

	}

}
void destroyGoods()
{
	g_list_free(list);
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
