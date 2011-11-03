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
#include <syslog.h>

#include <netdb.h>

#include "crc32.h"
#include "server.h"

#define IP_ADDRESS	"127.0.0.1"
#define PORT		1230
#define PAYLOAD_SIZE	1024

#define CONFIG_FILE		"/etc/init.d/usbl_server"
#define CONFIG_NUM_PARAMS	2
#define LINE_LENGTH		128
#define PIDFILE			"/var/run/usbl_srv.pid"

int main(int argc, char *argv[])
{
	struct sockaddr_in si_me, si_other;
	int sk;
	socklen_t slen = sizeof(si_other);
	int len, ret;

	fd_set readsock;

	struct packet p;
	char buf[PAYLOAD_SIZE];

	char log_file[LINE_LENGTH];
	char serial_file[LINE_LENGTH];

	unsigned long crc32_value;
	struct hostent *hostentry;
	unsigned int i;
	int mask;

	memset(log_file, 0, LINE_LENGTH);
	memset(serial_file, 0, LINE_LENGTH);
	memset(buf, 0, PAYLOAD_SIZE);

	if (argv[1] != NULL)
	{
		if(!strcmp(argv[1], "debug"))
			mask = (LOG_MASK(LOG_ERR) | LOG_MASK(LOG_DEBUG));
		else
			mask = LOG_MASK(LOG_ERR);
	}
	else
		mask = LOG_MASK(LOG_ERR);

	/* Our process ID and Session ID */
	pid_t pid, sid;

	/* Fork off the parent process */
	pid = fork();
	if (pid < 0)
	{
		exit(1);
	}

	/* If we got a good PID, then
	   we can exit the parent process. */
	if (pid > 0)
	{
		save_pid(PIDFILE, pid);
		exit(0);
	}

	/* Change the file mode mask */
	umask(0);

	/* Open any logs here */

	/* Create a new SID for the child process */
	sid = setsid();
	if (sid < 0)
	{
		/* Log the failure */
		exit(1);
	}


	/* Change the current working directory */
	if ((chdir("/")) < 0)
	{
		/* Log the failure */
		exit(1);
	}

	/* Close out the standard file descriptors */
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	/* Daemon-specific initialization goes here */
	openlog("USBlogger server", LOG_PID | LOG_CONS | LOG_NDELAY | LOG_NOWAIT, LOG_LOCAL0);
	setlogmask(mask);

	ret = parse_config(CONFIG_FILE, log_file, serial_file, CONFIG_NUM_PARAMS);
	if (ret)
	{
		syslog(LOG_ERR, "Something wrong happened while reading configuration. Please check config file - %s", CONFIG_FILE);
		exit(1);
	}

	/* Initialize the socket to recv messages */
	sk = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sk == -1)
	{
		syslog(LOG_ERR, "socket creation failed");
		exit(1);
	}

	memset(&si_me, 0, sizeof(si_me));
	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(PORT);
	si_me.sin_addr.s_addr = htonl(INADDR_ANY);

	ret = bind(sk, (struct sockaddr *)&si_me, sizeof(si_me));
	if (ret == -1)
	{
		syslog(LOG_ERR, "bind failed");
		exit(1);
	}

	/* The Big Loop */
	while (1)
	{
		FD_ZERO(&readsock);
		FD_SET(sk, &readsock);
		ret = select(FD_SETSIZE, &readsock, NULL, NULL, NULL);
		if (ret == -1)
		{
			syslog(LOG_ERR, "error in select() at main loop");
			exit(1);
		}
		else if (ret > 0)
		{
			if (FD_ISSET(sk, &readsock))
			{
				len = recvfrom(sk, buf, PAYLOAD_SIZE, 0, (struct sockaddr *)&si_other, &slen);
				if (len == -1)
				{
					syslog(LOG_ERR, "recvfrom returned an error");
					continue;
				}

				syslog(LOG_DEBUG, "len = %d, Received the message from client ... ", len);
//				for(i = 0; i < len; i++)
//					putchar(buf[i]);
//				putchar('\n');

				ret = parse_packet(&p, buf, len);
				if(ret)
				{
					syslog(LOG_ERR, "parser found an error during parsing a packet");
					//clear_packet(&p);
					memset(buf, 0, PAYLOAD_SIZE);
					continue;
				}
				print_packet(&p);

				ret = is_serial_allowed(serial_file, p.fields[2].value_val);
				if (ret != 0)
				{
					//serial is not in the list, so saving log
					save_packet(log_file, &p);
				}
				// serial in the list, nothing to do

				crc32_value = crc32(buf, len);
				syslog(LOG_DEBUG, "CRC32 value of the message is = %lx", crc32_value);

				hostentry = gethostbyname(p.fields[1].value_val);
				if(hostentry == NULL)
				{
					syslog(LOG_ERR, "Resolving hostname failed");
					clear_packet(&p);
					memset(buf, 0, PAYLOAD_SIZE);
					continue;
				}
				syslog(LOG_DEBUG, "Client IP: %s", inet_ntoa(*((struct in_addr *)hostentry->h_addr_list[0])) );

				clear_packet(&p);
				memset(buf, 0, PAYLOAD_SIZE);

				sprintf(buf, "%lx", crc32_value);
				ret = send_buffer_to_client(sk, (struct in_addr *)hostentry->h_addr_list[0], buf, strlen(buf));
				memset(buf, 0, PAYLOAD_SIZE);

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
	unsigned int j = 0;

	if(p == NULL)
	{
		syslog(LOG_ERR, "%s, packet is NULL", __FUNCTION__);
		exit(1);
	}

	if ((unsigned char)msg[i] != 0xbe || (unsigned char)msg[msg_len - 1] != 0xed)
	{
		syslog(LOG_ERR, "%s, packet is wrong", __FUNCTION__);
		return 1;
	}

	i += 1;
	p->num_fields = (unsigned char)msg[i];
	if (!p->num_fields)
	{
		syslog(LOG_ERR, "fields has a bad value");
		return 1;
	}

	p->fields = malloc(sizeof(struct field) * p->num_fields);
	if(p->fields == NULL)
	{
		syslog(LOG_ERR, "%s, malloc failed", __FUNCTION__);
		exit(1);
	}

	i += 1;
	for(j = 0; j < p->num_fields; j++)
	{
		p->fields[j].field_len = (unsigned char)msg[i];
		i += 1;
		p->fields[j].field_val = &msg[i];
		i = i + p->fields[j].field_len + 1;	// '\0' symbol

		p->fields[j].value_len = (unsigned char)msg[i];
		i += 1;
		p->fields[j].value_val = &msg[i];
		i = i + p->fields[j].value_len + 1;	// '\0' symbol
	}

	return 0;
}

static int print_packet(struct packet *p)
{
	unsigned int i = 0;

	if(p == NULL)
	{
		syslog(LOG_ERR, "%s, packet is NULL", __FUNCTION__);
		return 1;
	}

	syslog(LOG_DEBUG, "num_fields = %d", p->num_fields);
	for(i = 0; i < p->num_fields; i++)
	{
		syslog(LOG_DEBUG, "field_len = %d, field_val = %s\n", p->fields[i].field_len, p->fields[i].field_val);
		syslog(LOG_DEBUG, "value_len = %d, value_val = %s\n", p->fields[i].value_len, p->fields[i].value_val);
	}

	return 0;
}


// need to rewrite
static int clear_packet(struct packet *p)
{
	unsigned int i = 0;

	if(p == NULL)
	{
		syslog(LOG_ERR, "%s, packet is NULL", __FUNCTION__);
		return 1;
	}

	free(p->fields);

/*
	for(i = 0; i < p->num_fields; i++)
	{
		p->fields->field_len = 0;
		p->fields->field_val = NULL;

		p->fields->value_len = 0;
		p->fields->value_val = NULL;
	}
*/
	p->num_fields = 0;

	return 0;
}

static int is_serial_allowed(char *file_name, char *serial)
{
	FILE *fp;
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	int ret;

	fp = fopen(file_name, "r");
	if (fp == NULL)
	{
		syslog(LOG_ERR, "File %s does not exist", file_name);
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
			syslog(LOG_DEBUG, "The device with serial number %s is approved for use", serial);
			break;
		}
	}

	if (line)
		free(line);

	fclose(fp);

	return ret;
}

static int save_packet(char *file_name, struct packet *pkt)
{
	int fd;
	unsigned int i;

	if(pkt == NULL)
	{
		syslog(LOG_ERR, "%s, packet is NULL", __FUNCTION__);
		return 1;
	}

	umask(~(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH));

	fd = open(file_name, O_WRONLY | O_APPEND);
	if (fd == -1)
	{
		syslog(LOG_ERR, "File %s was not opened, trying to create it", file_name);
		fd = open(file_name, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
		if (fd == -1)
		{
			syslog(LOG_ERR, "File %s could not be created, exiting", file_name);
			exit(1);
		}
		for(i = 0; i < pkt->num_fields - 1; i++)
		{
			write(fd, pkt->fields[i].field_val, pkt->fields[i].field_len);
			write(fd, "\t", 1);
		}
		write(fd, pkt->fields[i].field_val, pkt->fields[i].field_len);
		write(fd, "\n", 1);
	}

	for(i = 0; i < pkt->num_fields - 1; i++)
	{
		write(fd, pkt->fields[i].value_val, pkt->fields[i].value_len);
		write(fd, "\t", 1);
	}
	write(fd, pkt->fields[i].value_val, pkt->fields[i].value_len);
	write(fd, "\n", 1);

	close(fd);

	return 0;
}


static int parse_config(const char *config_file, char *log_file, char *serial_file, unsigned int num_params)
{
	FILE *fp;
	char *variable;
	char *value;
	unsigned int red_params = 0;

	char *buf = malloc(LINE_LENGTH);
	if (buf == NULL)
	{
		syslog(LOG_ERR, "%s, malloc failed", __FUNCTION__);
		exit(1);
	}
	memset(buf, 0, LINE_LENGTH);

	fp = fopen(config_file, "r");
	if (fp == NULL)
	{
		syslog(LOG_ERR, "Can't open file - %s", config_file);
		exit(1);
	}

	while (NULL != fgets(buf, LINE_LENGTH, fp))
	{
		//count = 0;
		// убираем символ перевода строки, если он есть
		if ('\n' == buf[strlen(buf)-1])
		{
			buf[strlen(buf)-1] = '\0';
		}
		// пропускаем комментарии и строки, начинающиеся с '='
		if (('#' == buf[0]) || ('=' == buf[0]))
		{
			continue;
		}
		// защищаемся от "неполных" строк и строк с первым знаком '=' на последней позиции
		if ((2 < strlen(buf)) && ('=' != buf[strlen(buf)-1]))
		{
			if (NULL != strchr(buf, '='))
			{
				variable = strtok(buf, "=");
				value = strtok(NULL, "=");
				// ищем совпадение с известными программе параметрами
				if (0 == strncmp("LOG_FILE", variable, 8))
				{
					strncpy(log_file, value, strlen(value));
					red_params += 1;
				}
				else if (0 == strncmp("SERIAL_FILE", variable, 11))
				{
					strncpy(serial_file, value, strlen(value));
					red_params += 1;
				}
//				else if (0 == strncmp("port", variable, 4))
//				{
//					params->listenPort = atoi(value);
//				}
			}
		}
	}

	fclose(fp);
	free(buf);

	if(red_params != num_params)
		return 1;

	return 0;
}

static int save_pid(char *file_name, pid_t pid)
{
	int fd;
	char buf[8];

	fd = open(file_name, O_WRONLY | O_CREAT);
	if (fd == -1)
	{
		syslog(LOG_ERR, "File %s was not opened", file_name);
		exit(1);
	}

	sprintf(buf, "%d\n", pid);

	write(fd, buf, strlen(buf));

	close(fd);

	return 0;
}

static int send_buffer_to_client(int sock_fd, struct in_addr *client_addr, char *buf, int len)
{
	struct sockaddr_in addr;
	int ret;

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT);
	addr.sin_addr.s_addr = client_addr->s_addr;

	ret = sendto(sock_fd, buf, len, 0, (struct sockaddr *)&addr, sizeof(addr));
	if (ret == -1)
		syslog(LOG_ERR, "sendto() error");
	else if (ret != len)
		syslog(LOG_ERR, "Sending a UDP message not complete!\n");
	else
		syslog(LOG_DEBUG, "Sent a UDP message - %s, to %s\n", buf, inet_ntoa(*client_addr));

	return ret;
}
