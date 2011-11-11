#ifndef __USB_SERVER_H
#define __USB_SERVER_H


struct packet
{
	unsigned char fields;

	unsigned char time_len;
	char *time;

	unsigned char hostname_len;
	char *hostname;

	unsigned char serial_len;
	char *serial;

	unsigned char vendor_id_len;
	char *vendor_id;

	unsigned char product_id_len;
	char *product_id;

	unsigned char action_len;
	char *action;
};

static int parse_packet(struct packet *p, char *msg, size_t msg_len);
static int print_packet(struct packet *p);
static int clear_packet(struct packet *p, char *buf);
static int is_serial_allowed(char *file_name, char *serial);
static int save_packet(char *file, struct packet *p);

static int parse_config(const char *config_file, char *log_file, char *serial_file, unsigned int num_params);

#endif
