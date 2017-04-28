/*
 * goodsList.h
 *
 *  Created on: 2017. 4. 13.
 *      Author: LGCNS
 */

#ifndef GOODSLIST_H_
#define GOODSLIST_H_

typedef struct GOODS
{
	char abName[15];
	int  nCount;
	double dPrice;
	double dDisrate;
    int ValidDuration;
    char Availiable;
	time_t tRegistorDate;
}GOODS_INFO;

void addGoods();
void deleteGoods(char* abName);
void searchGoods(char* abName);
void updateGoods(char*abBuf,char*abBuf2);
void printGoods();
void CheckValidGoods();

void loadGoods();
int getGoods(GOODS_INFO** goodsList);
void destroyGoods();
#endif /* GOODSLIST_H_ */
