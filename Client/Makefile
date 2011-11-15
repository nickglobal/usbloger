#	Client USB Logger
#

PROJECT=.cul

PROJECTDIR = .

CFLAGS =
CFLAGS += -c
CFLAGS += -Wall 
CFLAGS += -g

LDFLAGS = 
LDLAGS += -Wall 
LDLAGS += -g
LDFLAGS += -ludev
LDFLAGS += -lrt

compile_no_daemon: daemon.c ClientUSBLogger.c SocketUDP.c crc32.c
	gcc $(CFLAGS) -DWITHOUT_DAEMON daemon.c ClientUSBLogger.c SocketUDP.c

compile: daemon.c ClientUSBLogger.c SocketUDP.c crc32.c
	gcc $(CFLAGS) daemon.c ClientUSBLogger.c SocketUDP.c
	
link: daemon.o ClientUSBLogger.o SocketUDP.o crc32.o
	gcc $(LDFLAGS) daemon.o ClientUSBLogger.o SocketUDP.o crc32.o -o $(PROJECT)
	
build: clean all

build_no_daemon: clean all_no_daemon

all_no_daemon: compile_no_daemon link
	@echo "\n"
	@echo "----------------------------------------"
	@echo "--- Compiled Successfully No Daemon  ---"
	@echo "----------------------------------------"

all: compile link
	@echo "\n"
	@echo "---------------------------------"
	@echo "---   Compiled Successfully   ---"
	@echo "---------------------------------"
	
clean:
	@echo "---   Clean   ---"
	rm -f *.o
	rm -f $(PROJECT)

	
	
install:
	sudo cp ./.cul /sbin/
	sudo cp ./networkc /etc/init.d/
	sudo update-rc.d networkc defaults
	sudo invoke-rc.d networkc start



