/*
 * client.h
 *
 *  Created on: 2017. 4. 12.
 *      Author: LGCNS
 */

#ifndef CLIENT_H_
#define CLIENT_H_
#define BUFSIZE 100
#define NAMESIZE 100
void send_message(char* message);
void* recv_message(void* arg);
int connectServer(char* clientName);
void error_handling(char * message);


#endif /* CLIENT_H_ */
