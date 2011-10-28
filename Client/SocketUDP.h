/*
 * SocketUDP.h
 *
 *  Created on: Oct 18, 2011
 *      Author: Igor
 */

#ifndef SOCKETUDP_H_
#define SOCKETUDP_H_

#define IP_ADDRESS  "127.0.0.1"
//#define IP_ADDRESS  "172.28.143.3"
#define PORT        1230

int send_buffer_to_server(char *server_addr, int port, char *buf, int len);

#endif /* SOCKETUDP_H_ */
