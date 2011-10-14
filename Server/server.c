#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/select.h>
#include <string.h>
#include <errno.h>

#include "server.h"

#define IP_ADDRESS	"127.0.0.1"
#define PORT		1230
#define PAYLOAD_SIZE	1024
#define LOG_FILE	"./report.txt"
#define SERIAL_FILE	"./serial.txt"

typedef struct bus_msg
{
	char msgtype;
	char busname[4];
} BUS_MSG;

typedef struct device_table
{
	u_int8_t portnum;
	char busname[4];
} DEVICE_TABLE;
DEVICE_TABLE local_table[8];

static void init_table()
{
	int i;
	for (i = 0; i < 8; i++)
		local_table[i].portnum = 9;
}

static int add_to_table(BUS_MSG *bus_msg)
{
	int i;
	for (i = 0; i < 8; i++)
	{
		if (local_table[i].portnum == 9)
		{
			local_table[i].portnum = i;
			strcpy(local_table[i].busname, bus_msg->busname);
			return 0;
		}
	}
	return -1;
}

static int del_from_table(BUS_MSG *bus_msg)
{
	int i;
	for (i = 0; i < 8; i++)
	{
		if (strcmp(local_table[i].busname, bus_msg->busname) == 0)
		{
			local_table[i].portnum = 9;
			strcpy(local_table[i].busname, "");
			return i;
		}
	}
	return -1;
}

static int get_port_from_table(BUS_MSG *bus_msg)
{
	int i;
	for (i = 0; i < 8; i++)
	{
		if (strcmp(local_table[i].busname, bus_msg->busname) == 0)
		{
			return (int)local_table[i].portnum;
		}
	}
	return -1;
}

static void print_table()
{
	int i;
	printf("******************** Local Table ***************************\n");
	for (i = 0; i < 8; i++)
		printf("%d\t%s\n", local_table[i].portnum, local_table[i].busname);
	printf("************************************************************\n");
}

int parse_packet(struct packet *p, char *msg, size_t msg_len)
{
	unsigned int i = 0;

	if ((unsigned char)msg[i] != 0xbe && (unsigned char)msg[msg_len - 1] != 0xed)
	{
		perror("packet is wrong\n");
		return 1;
	}

	i += 1;
	p->time_len = (unsigned char)msg[i];
	p->time = malloc(p->time_len);
	i += 1;
	strncpy(p->time, &msg[i], p->time_len);
//	printf("time len = %d, time = %s\n", p->time_len, p->time);

	i += p->time_len;
	p->hostname_len = (unsigned char)msg[i];
	p->hostname = malloc(p->hostname_len);
	i += 1;
	strncpy(p->hostname, &msg[i], p->hostname_len);
//	printf("hostname len = %d, hostname = %s\n", p->hostname_len, p->hostname);

	i += p->hostname_len;
	p->serial_len = (unsigned char)msg[i];
	p->serial = malloc(p->serial_len);
	i += 1;
	strncpy(p->serial, &msg[i], p->serial_len);
//	printf("serial_len = %d, serial = %s\n", p->serial_len, p->serial);

	i += p->serial_len;
	p->vendor_id_len = (unsigned char)msg[i];
	p->vendor_id = malloc(p->vendor_id_len);
	i += 1;
	strncpy(p->vendor_id, &msg[i], p->vendor_id_len);
//	printf("vendor_id_len = %d, vendor_id = %s\n", p->vendor_id_len, p->vendor_id);

	i += p->vendor_id_len;
	p->product_id_len = (unsigned char)msg[i];
	p->product_id = malloc(p->product_id_len);
	i += 1;
	strncpy(p->product_id, &msg[i], p->product_id_len);
//	printf("product_id_len = %d, product_id = %s\n", p->product_id_len, p->product_id);

	return 0;
}

int free_packet()
{
	
	return 0;
}

static int is_serail_exist_in_list(char *serial)
{
	FILE *fp;
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	int ret;

	fp = fopen(SERIAL_FILE, "r");
	if (fp == NULL)
	{
		perror("fopen filed");
		exit(1);
	}

	while ((read = getline(&line, &len, fp)) != -1)
	{
		// getline includes symbol \n at the end of string.
		// we should change it on \0 before comparing.
		line[read - 1] = '\0';
		ret = strcmp(serial, line);
		if (ret == 0)
		{
			// strings equal
			printf("The device with serial number %s is approved for use\n", serial);
			//printf("Red line = %s of length = %zu, ret = \n", line, read, ret);
			break;
		}
		printf("Red line = %s of length = %zu, ret = %d\n", line, read, ret);
	}

	if (line)
		free(line);

	return ret;
}


int write_packet_to_file(struct packet *p)
{
	int fd;
	fd = open(LOG_FILE, O_WRONLY | O_CREAT | O_APPEND);
	if (fd == -1)
		printf("File %s was not opened\n", LOG_FILE);

	write(fd, p->time, p->time_len);
	write(fd, "\t", 1);
	write(fd, p->hostname, p->hostname_len);
	write(fd, "\t", 1);
	write(fd, p->serial, p->serial_len);
	write(fd, "\t", 1);
	write(fd, p->vendor_id, p->vendor_id_len);
	write(fd, "\t", 1);
	write(fd, p->product_id, p->product_id_len);
	write(fd, "\n", 1);
	close(fd);
}

int main(int argc, char *argv[])
{
	struct sockaddr_in si_me, si_other;
	int s;
	socklen_t slen = sizeof(si_other), ret;
	char buf[PAYLOAD_SIZE];
	char cmd[40];
	int tableret;
	fd_set readsock;
	BUS_MSG *bus_msg;

	struct packet p;

//	init_table();
//	print_table();

	/* Initialize the socket to recv messages */
	if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
		perror ("socket");

	memset(&si_me, 0, sizeof(si_me));
	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(PORT);
	si_me.sin_addr.s_addr = htonl(INADDR_ANY);

	ret = bind(s, (struct sockaddr *)&si_me, sizeof(si_me));
	if (ret == -1)
		perror("bind");

	while (1)
	{
		memset(buf, 0, sizeof(buf));
		FD_ZERO(&readsock);
		FD_SET(s, &readsock);
		ret = select(FD_SETSIZE, &readsock, NULL, NULL, NULL);
		if (ret == -1)
			perror("error in select() at main loop");
		if (ret > 0)
		{
			if (FD_ISSET(s, &readsock))
			{
				ret = recvfrom(s, buf, PAYLOAD_SIZE, 0, (struct sockaddr *)&si_other, &slen);
				if (ret == -1)
					perror("recvfrom()");
				//bus_msg = (BUS_MSG*)buf;
				printf("Received the message from client - %s\n", buf);
				parse_packet(&p, buf, strlen(buf));
				
				is_serail_exist_in_list(p.serial);
				//is_serail_exist_in_list("aaaa\n");
				
				//write_packet_to_file(&p);

/*
				// Any device plugged into server
				if (bus_msg->msgtype == '1')
				{
					tableret = add_to_table(bus_msg);
					if (tableret != 0)
					{
						printf("No hub any more.\n");
						return -1;
					}
					print_table();
					printf("Received a bus (%s) from %s\n", bus_msg->busname, inet_ntoa(si_other.sin_addr));
					sprintf(cmd, "usbip_attach ");
					strcat(cmd, inet_ntoa(si_other.sin_addr));
					strcat(cmd, " ");
					strcat(cmd, bus_msg->busname);
					system(cmd);
				}

				// Any device unplugged from server
				if (bus_msg->msgtype == '0')
				{
					char str[2];
					tableret = del_from_table(bus_msg);
					if (tableret == -1)
					{
						printf("Device not found.\n");
						return -1;
					}
					print_table();
					printf("Received a bus (%s) from %s\n", bus_msg->busname, inet_ntoa(si_other.sin_addr));
					sprintf(cmd, "usbip_detach ");
					sprintf(str, "%d", tableret);
					strcat(cmd, str);
					system(cmd);
				}
*/
			}
		}
	}
	close(s);
	return 0;
}
