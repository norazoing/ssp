/*
 * client.c
 *
 *  Created on: 2017. 4. 12.
 *      Author: LGCNS
 */


#include<stdio.h>
#include <stdlib.h>

#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include "./client.h"
char name[NAMESIZE] = "[Default]";
int sock;
int connectServer(char* clientName)
{
	printf("connectServer\n");
	 struct sockaddr_in serv_addr;
	 pthread_t rcv_thread;
	 void* thread_result;

	 sprintf(name, "[%s]", clientName);

	 sock = socket(PF_INET, SOCK_STREAM, 0);
	 if(sock == -1)
	  error_handling("socket() error");

	 memset(&serv_addr, 0, sizeof(serv_addr));
	 serv_addr.sin_family = AF_INET;
	 serv_addr.sin_addr.s_addr=inet_addr("127.0.0.1");
	 serv_addr.sin_port=htons(1111);

	 if(connect(sock,(struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
	  error_handling("connect() error!");

	 //pthread_create(&snd_thread, NULL, send_message, (void*)sock);
	 pthread_create(&rcv_thread, NULL, recv_message, (void*)sock);

	 //pthread_join(snd_thread, &thread_result);
	 pthread_join(rcv_thread, &thread_result);

	 close(sock);
	 return 0;
}

void send_message(char* message)
{
	printf("send message\n");
	char name_message[NAMESIZE+BUFSIZE];
	sprintf(name_message, "%s %s", message, name);
	write(sock, name_message, strlen(name_message));

}
void* recv_message(void* arg)
{
 int sock = (int) arg;
 char name_message[NAMESIZE+BUFSIZE];
 int str_len;
 while(1)
 {
  str_len = read(sock, name_message, NAMESIZE+BUFSIZE-1);
  if(str_len == -1) return (void*)1;
  name_message[str_len]=0;

  printf("recv message : \n");

  for(int i =0 ;i< str_len; i++)
  {
	  printf("%c",name_message[i]);
  }
  //fputs(name_message,stdout);
 }
}

void error_handling(char * message)
{
 fputs(message ,stderr);
 fputc('\n', stderr);
}




