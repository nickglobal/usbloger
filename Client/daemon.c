/*
 * daemon.c
 *
 *  Created on: Nov 3, 2011
 *      Author: TI x0169005
 */


#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include "ClientUSBLogger.h"
#include "SocketUDP.h"
#include "common.h"

#define PIDFILE         "/var/run/cul.pid"
//#define PATH2DEMO "/root/cul/.culdemo"
//#define WITHOUT_DAEMON


static int save_pid(char *file_name, pid_t pid);

int main (int argc, char *argv[])
{
    char hostName[LEN_STR];
    char hostIP[LEN_IP];
    int status = EXIT_FAILURE;
    /* Our process ID and Session ID */
    pid_t pid, sid;
    //FILE *pFileBuf;

    /* Open Log */
    openlog("CUL", LOG_PID|LOG_CONS|LOG_NDELAY|LOG_NOWAIT, LOG_LOCAL0);
      setlogmask(LOG_MASK(LOG_ERR));

    if (argc != 3) {
        syslog(LOG_ERR | LOG_USER, "usage: cul [OPTION] [host/file name]");
        DPRINT("usage: cul [OPTION] [host/file name]\n");
        exit(EXIT_FAILURE);
    }

    if( 0 == strcmp("-h", argv[1]) )
    {
        strncpy(hostName, argv[2], LEN_STR);
    }

    if (EXIT_FAILURE == get_ip_by_hostname(hostName, hostIP))
    {
        syslog(LOG_ERR | LOG_USER, "Failure to get server IP by hostName:%s", hostName);
        DPRINT("Failure to get server IP by hostName\n");
        exit(EXIT_FAILURE);
    }

    if (EXIT_FAILURE == open_socket_to_rcv(PORT_ACK))
    {
        syslog(LOG_ERR | LOG_USER, "Failure open socket to listen Ip:%s Port:%d", hostIP, PORT);
        DPRINT("Failure open socket to listen\n");
        exit(EXIT_FAILURE);
    }

//    pFileBuf = fopen (PATH2DEMO,"a+");
//    if(NULL == pFileBuf)
//    {
//        printf("Fail to open(a+) Buff file\n");
//    }
//    else printf("Success\n");
//    fflush(stdout);
//    while(1);

#ifndef WITHOUT_DAEMON
    /* Fork off the parent process */
    pid = fork();
    if (pid < 0)
    {
        syslog(LOG_ERR | LOG_USER, "Failure to fork parent process");
        exit(EXIT_FAILURE);
    }
    /* If we got a good PID, then
         we can exit the parent process. */
    if (pid > 0)
    {
        save_pid(PIDFILE, pid);
        exit(EXIT_SUCCESS);
    }

    /* Change the file mode mask */
    umask(0);

    /* Create a new SID for the child process */
    sid = setsid();
    if (sid < 0) {
    /* Log the failure */
    syslog(LOG_ERR | LOG_USER, "Failure to get SID of child process. CID=%d", sid);
    exit(EXIT_FAILURE);
    }

    /* Change the current working directory */
    status = chdir("/");
    if (status < 0) {
    /* Log the failure */
    syslog(LOG_ERR | LOG_USER, "Failure to change working directory. status=%d", status);
    exit(EXIT_FAILURE);
    }

    /* Close out the standard file descriptors */
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    /* Daemon-specific initialization goes here */
#endif

    /* The Big Loop */
    while(1)
    {
        LogUdevEvents();
        send_msg_to_server(hostIP);
        usleep(250*1000);
        get_ack_from_server();
    }

    exit(EXIT_SUCCESS);
}

static int save_pid(char *file_name, pid_t pid)
{
    int fd;
    char buf[8];

    fd = open(file_name, O_WRONLY | O_CREAT);
    if (fd == -1)
    {
        syslog(LOG_ERR | LOG_USER, "File %s was not opened\n", file_name);
        exit(1);
    }

    sprintf(buf, "%d\n", pid);

    write(fd, buf, strlen(buf));

    close(fd);

    return 0;
}

