/*
 * queue_sample.c
 *
 *  Created on: 2017. 4. 16.
 *      Author: 이소정
 */
#include<stdio.h>
#include<glib.h>
#include<string.h>
#include<stdlib.h>
typedef struct PERSON
{
	char abName[15];
	int nAge;
}PERSON;
static void _free_fun(gpointer data)
{
	if(data)
		free(data);
}

PERSON* makePerson(char* name, int age) {
	PERSON* p = g_new(PERSON, 1);
	strcpy( p->abName , name);
	p->nAge = age;
	return p;
}
void printPerson(gpointer item) {
	printf("%s\t%d\n", ((PERSON*)item)->abName,((PERSON*)item)->nAge);
}
gint sorter(gconstpointer a, gconstpointer b, gpointer data) {
	return ((PERSON*)a)->nAge - ((PERSON*)b)->nAge;
}
int main()
{
	//요걸 해줘야 런타임시 콘솔에서 시스템 출력을 확인할 수 있음.
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);

	//문자열, 숫자, 구조체를 가지고 있는  리스트를 만든다
	 GQueue* q = g_queue_new();

	 g_queue_push_tail(q, makePerson("Misa", 3));
	 g_queue_push_tail(q, makePerson("Binna", 2));
	 g_queue_push_tail(q, makePerson("Jeon", 4));
	 g_queue_push_tail(q, makePerson("Lim", 1));

	 printf("Original queue: \n");
	 g_queue_foreach(q, (GFunc)printPerson, NULL);
	 g_queue_sort(q, (GCompareDataFunc)sorter, NULL);
	 printf("Sorted queue: \n");
	 g_queue_foreach(q, (GFunc)printPerson, NULL);
	 g_queue_free_full(q,_free_fun);


	return 0;
}



