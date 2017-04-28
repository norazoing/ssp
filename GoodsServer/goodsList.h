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
	char abCount[3];
	char abPrice[7];
	char abDate[8];
}GOODS_INFO;

void addGoods();
void printGoods();
void loadGoods();
int getGoods(GOODS_INFO** goodsList);
void destroyGoods();
#endif /* GOODSLIST_H_ */
