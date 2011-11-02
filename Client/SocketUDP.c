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
#include <netdb.h>
#include "ClientUSBLogger.h"

int send_buffer_to_server(const char *pHostname, int port, char *pBuf, int len)
{
    struct sockaddr_in addr;
    int sock_fd, ret;
    struct hostent *pHost;
    char *pIp_addr;

    sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock_fd == -1)

    {
        perror("socket creation failed");
        return -1;
    }

    pHost = gethostbyname(pHostname);

    if(pHost == NULL)
    {
        perror("Error get host by name.\n");
        return -1;
    }

    memset(&addr, 0, sizeof(addr));

    memcpy(&addr.sin_addr,
            pHost->h_addr,
            pHost->h_length);

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    pIp_addr = inet_ntoa(addr.sin_addr);

    ret = sendto(sock_fd, pBuf, len, 0, (struct sockaddr *)&addr, sizeof(addr));
    if (ret == -1)
        perror("sendto() error");
    else if (ret != len)
        perror("Send UDP message not complete!");
    else
        DPRINT("Sent a UDP message to Host:%s IP:%s, was sent Nbytes = %d \n", pHostname, pIp_addr, ret);

    close(sock_fd);

    return 0;
}
