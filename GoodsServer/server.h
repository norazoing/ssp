/*
 * server.h
 *
 *  Created on: 2017. 4. 12.
 *      Author: LGCNS
 */

#ifndef SERVER_H_
#define SERVER_H_

#include <stdio.h>
#include <pthread.h>

#define BUFSIZE 1024


void* clnt_connection(void * arg);
void send_message(char * message, int sock, int len);
void error_handling(char * message);



#endif /* SERVER_H_ */
