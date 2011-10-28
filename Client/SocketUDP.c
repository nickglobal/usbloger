/*
 * SocketUDP.c
 *
 *  Created on: Oct 18, 2011
 *      Author: Igor
 */

#include "common.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int send_buffer_to_server(char *server_addr, int port, char *buf, int len)
{
    struct sockaddr_in addr;
    int sock_fd, ret;

    sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock_fd == -1)
    {
        perror("socket creation failed");
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(server_addr);

    ret = sendto(sock_fd, buf, len, 0, (struct sockaddr *)&addr, sizeof(addr));
    if (ret == -1)
        perror("sendto() error");
    else if (ret != len)
        perror("Send UDP message not complete!");
    else
        DPRINT("Sent a UDP message to %s, was sent Nbytes = %d \n", server_addr, ret);

    close(sock_fd);

    return 0;
}
