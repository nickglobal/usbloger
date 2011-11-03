#ifndef __USB_SERVER_H
#define __USB_SERVER_H

struct field
{
	unsigned char field_len;
	char *field_val;

	unsigned char value_len;
	char *value_val;
};

struct packet
{
	unsigned char num_fields;
	struct field *fields;
};

static int parse_packet(struct packet *p, char *msg, size_t msg_len);
static int print_packet(struct packet *p);
static int clear_packet(struct packet *p);
static int is_serial_allowed(char *file_name, char *serial);
static int save_packet(char *file_name, struct packet *pkt);

static int parse_config(const char *config_file, char *log_file, char *serial_file, unsigned int num_params);
static int save_pid(char *file_name, pid_t pid);

static int send_buffer_to_client(int sock_fd, struct in_addr *client_addr, char *buf, int len);

#endif
