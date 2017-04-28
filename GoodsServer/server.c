/*
 * server.c
 *
 *  Created on: 2017. 4. 12.
 *      Author: LGCNS
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include "./server.h"
#include "./goodsList.h"
int clnt_number=0;
int clnt_socks[10];
pthread_mutex_t mutx;
void connectServer()
{
	 int serv_sock;
	 int clnt_sock;
	 struct sockaddr_in serv_addr;
	 struct sockaddr_in clnt_addr;
	 int clnt_addr_size;
	 pthread_t thread;

	 if(pthread_mutex_init(&mutx,NULL))
	  error_handling("mutex init error");
	 serv_sock = socket(PF_INET, SOCK_STREAM, 0);
	 if(serv_sock == -1)
	  error_handling("socket() error");
	 memset(&serv_addr, 0, sizeof(serv_addr));
	 serv_addr.sin_family = AF_INET;
	 serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	 serv_addr.sin_port = htons(1111);
	 if(bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
	  error_handling("bind() error");
	 if(listen(serv_sock, 5) == -1)
	  error_handling("listen() error");
	 while(1) {
	  clnt_addr_size = sizeof(clnt_addr);
	  clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_addr, &clnt_addr_size);
	  pthread_mutex_lock(&mutx);
	  clnt_socks[clnt_number++]=clnt_sock;
	  pthread_mutex_unlock(&mutx);
	  pthread_create(&thread, NULL, clnt_connection, (void*) clnt_sock);
	  printf(" IP : %s \n", inet_ntoa(clnt_addr.sin_addr));
	 }
}
void *clnt_connection(void *arg)
{
 int clnt_sock = (int) arg;
 int str_len=0;
 char message[BUFSIZE]={0,};
 GOODS_INFO* goodsList;
 int i;
 int nGoodsCount;
 int nIndex =0;
 while((str_len=read(clnt_sock, message, sizeof(message))) != 0 )
 {
	 printf("message:%s\n",message);


	 if(memcmp(message,"REQ",strlen("REQ"))==0)
	 {
		 memset( message, 0x00, sizeof(message));
		 nIndex =0;
		 nGoodsCount =getGoods(&goodsList);
		 printf("nGoodsCount:%d\n",nGoodsCount);
		 for(i =0; i <nGoodsCount; i++)
		 {
			 memcpy(&message[nIndex],(char*)&goodsList[i],sizeof(GOODS_INFO));
			 nIndex+=sizeof(GOODS_INFO);
			 message[nIndex++]=0x0D;
			 message[nIndex++]=0x0A;
		 }
		 send_message(message,clnt_sock, nIndex);
	 }
 }

 pthread_mutex_lock(&mutx);
 for(i=0;i<clnt_number;i++)
 {
  if(clnt_sock == clnt_socks[i])
  {
   for(;i<clnt_number-1;i++) clnt_socks[i] = clnt_socks[i+1]; break;
  }
 }
 clnt_number--;
 pthread_mutex_unlock(&mutx);
 close(clnt_sock);
 return 0;
}
void send_message(char * message, int sock, int len)
{
 int i;
 pthread_mutex_lock(&mutx);

  write(sock, message, len);
 pthread_mutex_unlock(&mutx);
}
void error_handling(char * message)
{
 fputs(message, stderr);
 fputc('\n',stderr);
 exit(1);
}

