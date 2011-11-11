/*
 * SocketUDP.c
 *
 *  Created on: Oct 18, 2011
 *      Author: TI x0169005
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
#include <stdlib.h>
#include <syslog.h>
#include <errno.h>

int sk_rcv;

int open_socket_to_rcv(int port)
{
    struct sockaddr_in adr_rcv;
    int ret, status = EXIT_SUCCESS;

    /* Initialize the socket to recv messages */
    sk_rcv = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sk_rcv == -1)
    {
        syslog(LOG_ERR | LOG_USER, "socket creation failed\n");
        status = EXIT_FAILURE;
    }

    memset(&adr_rcv, 0, sizeof(adr_rcv));
    adr_rcv.sin_family = AF_INET;
    adr_rcv.sin_port = htons(port);
    adr_rcv.sin_addr.s_addr = htonl(INADDR_ANY);

    ret = bind(sk_rcv, (struct sockaddr *)&adr_rcv, sizeof(adr_rcv));
    if (EXIT_SUCCESS != ret)
    {
        syslog(LOG_ERR | LOG_USER, "bind failed, status=%d, errno=%d\n", ret, errno);
        status = EXIT_FAILURE;
    }

    return status;
}

void get_ack_from_server(void)
{
    struct sockaddr_in adr_rcv;
    socklen_t adr_len = sizeof(adr_rcv);
    int status, len;
    fd_set readsock;
    char buf_rcv[LEN_BUF];
    unsigned int *crc32_rcv;
    unsigned int crc32_val = 0;
    struct timeval timeout;

    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    memset(buf_rcv, 0, sizeof(buf_rcv));

    FD_ZERO(&readsock);
    FD_SET(sk_rcv, &readsock);

    status = select(FD_SETSIZE, &readsock, NULL, NULL, &timeout);
    if (status == -1)
    {
        syslog(LOG_ERR | LOG_USER, "error in select() at main loop\n");
        exit(1);
    }
    else if (status > 0)
    {
        if (FD_ISSET(sk_rcv, &readsock))
        {
            len = recvfrom(sk_rcv, buf_rcv, LEN_BUF, 0, (struct sockaddr *)&adr_rcv, &adr_len);
            if (len == -1)
            {
                syslog(LOG_ERR | LOG_USER, "recvfrom returned an error\n");
            }
            else
            {
                crc32_rcv = (unsigned int *)buf_rcv;

                if (*crc32_rcv != get_msg_crc())
                {
                    syslog(LOG_ERR | LOG_USER, "len = %d, Received the acknowledge message from server.", len);
                    syslog(LOG_ERR | LOG_USER, "CRC32 dosen't match. Expected CRC32 =%x, Got CRC32 =%x\n",
                            crc32_val, *crc32_rcv);
                }
                else
                {
                    delete_buf_msg();
                }
            }//else
        }//if (FD_ISSET(sk_rcv, &readsock))
    }//else if (status > 0)
}

int get_ip_by_hostname(const char *pHostname, char *pIp)
{
    int status = EXIT_FAILURE;
    struct in_addr sin_addr;
    struct hostent *pHost;

    pHost = gethostbyname(pHostname);

    if(pHost == NULL)
    {
        syslog(LOG_ERR | LOG_USER, "Error get host by name.\n");
        status = EXIT_FAILURE;
    }
    else
    {

    memset(&sin_addr, 0, sizeof(sin_addr));

    memcpy(&sin_addr,
            pHost->h_addr,
            pHost->h_length);

    strncpy(pIp, inet_ntoa(sin_addr), LEN_IP);
    status = EXIT_SUCCESS;
    }

    return status;
}

int send_buffer_to_server(const char *server_addr, int port,unsigned char *pBuf, int len)
{
    struct sockaddr_in addr;
    int sock_fd, ret;
    int status = EXIT_FAILURE;

    sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock_fd == -1)

    {
        syslog(LOG_ERR | LOG_USER, "socket creation failed");
        status = EXIT_FAILURE;
    }
    else
    {
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = inet_addr(server_addr);

        ret = sendto(sock_fd, pBuf, len, 0, (struct sockaddr *)&addr, sizeof(addr));
        if (ret == -1)
            syslog(LOG_ERR | LOG_USER, "sendto() error");
        else if (ret != len)
            syslog(LOG_ERR | LOG_USER, "Send UDP message not complete!");
        else
            DPRINT("Sent a UDP message to IP:%s, was sent Nbytes = %d \n", server_addr, ret);

        close(sock_fd);
        status = EXIT_SUCCESS;
    }

    return status;
}
