
struct packet
{
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
};
