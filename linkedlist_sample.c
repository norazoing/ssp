/*
 * linkedlist_sample.c
 *
 *  Created on: 2017. 4. 16.
 *      Author: ¿Ãº“¡§
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



int main()
{
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);
	GList *list= NULL;
	GList *listNumber= NULL;
	GList *listStruct = NULL;
	GList *curr;
	int i=0;

	list = g_list_append(list,"abcd");
	list = g_list_append(list,"efgh");
	list = g_list_append(list,"ijkl");
	list = g_list_append(list,"mnop");

	for(curr= list; curr!= NULL; curr= curr->next)
	{
		printf("%s\n",curr->data);
	}
	g_list_free(list);
	list = NULL;

	listNumber = g_list_append(listNumber,GINT_TO_POINTER(1));
	listNumber = g_list_append(listNumber,GINT_TO_POINTER(2));
	listNumber = g_list_append(listNumber,GINT_TO_POINTER(3));
	listNumber = g_list_append(listNumber,GINT_TO_POINTER(4));

	for(curr= listNumber; curr!= NULL; curr= curr->next)
	{
		printf("%d\n",GPOINTER_TO_INT(curr->data));
	}
	g_list_free(listNumber);
	listNumber = NULL;

	for(i =0 ;i< 4; i++)
	{
		PERSON* newPerson =(PERSON*) malloc(sizeof(PERSON));
		sprintf(newPerson->abName,"person%d",i+1);
		newPerson->nAge = i+1;
		listStruct = g_list_append(listStruct,newPerson);

	}
	for(curr= listStruct; curr!= NULL; curr= curr->next)
	{
		printf("%s\t %d\n",((PERSON*)curr->data)->abName, ((PERSON*)curr->data)->nAge);
	}
	g_list_free_full(listStruct,_free_fun);


	return 0;
}

