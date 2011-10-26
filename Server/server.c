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

#include <netdb.h>

#include "crc32.h"
#include "server.h"

#define IP_ADDRESS	"127.0.0.1"
#define PORT		1230
#define PAYLOAD_SIZE	1024
#define LOG_FILE	"./report.txt"
#define SERIAL_FILE	"./serial.txt"


int main(int argc, char *argv[])
{
	struct sockaddr_in si_me, si_other;
	int sk;
	socklen_t slen = sizeof(si_other), ret;

	fd_set readsock;

	struct packet p;
	char buf[PAYLOAD_SIZE];

	unsigned long crc32_value;
	struct hostent *hostentry;
	char *ipbuf; //need to delete
	unsigned int i;

	clear_packet(&p, buf);
	/* Initialize the socket to recv messages */
	sk = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sk == -1)
	{
		perror("socket creation failed");
		exit(1);
	}

	memset(&si_me, 0, sizeof(si_me));
	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(PORT);
	si_me.sin_addr.s_addr = htonl(INADDR_ANY);

	ret = bind(sk, (struct sockaddr *)&si_me, sizeof(si_me));
	if (ret == -1)
	{
		perror("bind failed");
		exit(1);
	}

	while (1)
	{
		FD_ZERO(&readsock);
		FD_SET(sk, &readsock);
		ret = select(FD_SETSIZE, &readsock, NULL, NULL, NULL);
		if (ret == -1)
		{
			perror("error in select() at main loop");
			exit(1);
		}
		else if (ret > 0)
		{
			if (FD_ISSET(sk, &readsock))
			{
				ret = recvfrom(sk, buf, PAYLOAD_SIZE, 0, (struct sockaddr *)&si_other, &slen);
				if (ret == -1)
				{
					perror("recvfrom returned an error");
					// ???
				}

				printf("Received the message from client - ");
				for(i = 0; i < ret; i++)
					putchar(buf[i]);
				putchar('\n');

				crc32_value = crc32(buf, ret);
				printf("CRC32 value of the message is = %x\n", crc32_value);

				ret = parse_packet(&p, buf, ret);
				if(ret)
				{
					//parser found an error during parsing
					clear_packet(&p, buf);
					continue;
				}
				print_packet(&p);

				hostentry = gethostbyname(p.hostname);
				if(hostentry == NULL)
				{
					perror("Resolving hostname failed");
					exit(1);
				}

				ipbuf = inet_ntoa(*((struct in_addr *)hostentry->h_addr_list[0]));
				printf("Host IP: %s\n", ipbuf);

				ret = is_serail_exist_in_list(p.serial);
				if (ret != 0)
				{
					//serial is not in the list, so saving log
					write_packet_to_file(&p);
				}

				// serial in the list, nothing to do
				clear_packet(&p, buf);

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
	close(sk);
	return 0;
}


static int parse_packet(struct packet *p, char *msg, size_t msg_len)
{
	unsigned int i = 0;

	if(p == NULL)
	{
		printf("%s, packet is NULL\n", __FUNCTION__);
		exit(1);
	}

	if ((unsigned char)msg[i] != 0xbe || (unsigned char)msg[msg_len - 1] != 0xed)
	{
		printf("%s, packet is wrong\n", __FUNCTION__);
		return 1;
	}

	i += 1;
	p->fields = (unsigned char)msg[i];
	if (!p->fields)
	{
		perror("fields has a bad value");
		return 1;
	}

	i += 1;
	p->time_len = (unsigned char)msg[i];
	i += 1;
	p->time = &msg[i];

	i = i + p->time_len + 1;	// '\0' symbol
	p->hostname_len = (unsigned char)msg[i];
	i += 1;
	p->hostname = &msg[i];

	i = i + p->hostname_len + 1;	// '\0' symbol
	p->serial_len = (unsigned char)msg[i];
	i += 1;
	p->serial = &msg[i];

	i = i + p->serial_len + 1;	// '\0' symbol
	p->vendor_id_len = (unsigned char)msg[i];
	i += 1;
	p->vendor_id = &msg[i];

	i = i + p->vendor_id_len + 1;	// '\0' symbol
	p->product_id_len = (unsigned char)msg[i];
	i += 1;
	p->product_id = &msg[i];

	i = i + p->product_id_len + 1;	// '\0' symbol
	p->action_len = (unsigned char)msg[i];
	i += 1;
	p->action = &msg[i];

	return 0;
}

static int print_packet(struct packet *p)
{
	if(p == NULL)
	{
		printf("%s, packet is NULL\n", __FUNCTION__);
		return 1;
	}

	printf("fields = %d\n", p->fields);
	printf("time len = %d, time = %s\n", p->time_len, p->time);
	printf("hostname len = %d, hostname = %s\n", p->hostname_len, p->hostname);
	printf("serial_len = %d, serial = %s\n", p->serial_len, p->serial);
	printf("vendor_id_len = %d, vendor_id = %s\n", p->vendor_id_len, p->vendor_id);
	printf("product_id_len = %d, product_id = %s\n", p->product_id_len, p->product_id);
	printf("action_len = %d, action = %s\n", p->action_len, p->action);

	return 0;
}

static int clear_packet(struct packet *p, char *buf)
{
	if(p == NULL)
	{
		printf("%s, packet is NULL\n", __FUNCTION__);
		return 1;
	}

	p->time_len = 0;
	p->time = NULL;

	p->hostname_len = 0;
	p->hostname = NULL;

	p->serial_len = 0;
	p->serial = NULL;

	p->vendor_id_len = 0;
	p->vendor_id = NULL;

	p->product_id_len = 0;
	p->product_id = NULL;

	p->action_len = 0;
	p->action = NULL;

	memset(buf, 0, PAYLOAD_SIZE);

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
		printf("fopen filed on %s\n", SERIAL_FILE);
		return 1;
	}

	while ((read = getline(&line, &len, fp)) != -1)
	{
		// getline includes symbol \n at the end of string.
		// we should change it on \0 before comparing.
		line[read - 1] = '\0';
		ret = strcmp(serial, line);
		if (ret == 0)
		{
			// strings are equal
			printf("The device with serial number %s is approved for use\n", serial);
			break;
		}
		//printf("serial = %s, Red line = %s of length = %zu, ret = %d\n", serial, line, read, ret);
	}

	if (line)
		free(line);

	fclose(fp);

	return ret;
}


static int write_packet_to_file(struct packet *p)
{
	int fd;

	if(p == NULL)
	{
		printf("%s, packet is NULL\n", __FUNCTION__);
		return 1;
	}

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
	write(fd, "\t", 1);
	write(fd, p->action, p->action_len);
	write(fd, "\n", 1);
	close(fd);

	return 0;
}
