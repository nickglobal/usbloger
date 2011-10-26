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
static int is_serail_exist_in_list(char *serial);
static int write_packet_to_file(struct packet *p);


#endif
