/*
 * SocketUDP.h
 *
 *  Created on: Oct 18, 2011
 *      Author: Igor
 */

#ifndef SOCKETUDP_H_
#define SOCKETUDP_H_

#define PORT        1230
#define PORT_ACK    1231

int open_socket_to_rcv(int port);
int send_buffer_to_server(char *server_addr, int port,unsigned char *buf, int len);
int get_ip_by_hostname(const char *pHostname, char *pIp);
void get_ack_from_server(void);

#endif /* SOCKETUDP_H_ */
