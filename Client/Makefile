#	Client USB Logger
#

PROJECT=ClientUSBLogger

PROJECTDIR = .

CFLAGS =
CFLAGS += -I /usr/include
CFLAGS += -Wall 
CFLAGS += -g
CFLAGS += -ludev

LDFLAGS = 
LDLAGS += -Wall 
LDLAGS += -g
LDFLAGS += -ludev

compile: ClientUSBLogger.c SocketUDP.c ListDev.c
	gcc -c -g -Wall ClientUSBLogger.c SocketUDP.c ListDev.c
	
link: ClientUSBLogger.o
	gcc -Wall -ludev ClientUSBLogger.o SocketUDP.o ListDev.o -o client
	
build: clean all

all: compile link
	@echo "---   Compile Complete   ---"
	
clean:
	@echo "---   Clean   ---"
	rm -f *.o



