CC=gcc
APP_NAME=usbl_srv

all:
	$(CC) -o $(APP_NAME) server.c crc32.c

clean:
	rm -f $(APP_NAME)
	rm -f *.o

install:
	sudo cp ./usbl_srv /sbin/
	sudo cp ./usbl_server /etc/init.d/
	sudo mkdir /var/log/usbl_server
	sudo chmod 777 /var/log/usbl_server
	sudo update-rc.d usbl_server defaults
	sudo invoke-rc.d usbl_server start

uninstall:
	sudo invoke-rc.d usbl_server stop
	sudo update-rc.d -f usbl_server remove
	sudo rm /sbin/usbl_srv
	sudo rm /etc/init.d/usbl_server
	sudo rm -r /var/log/usbl_server
