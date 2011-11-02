#	Client USB Logger
#

PROJECT=cul

PROJECTDIR = .

CFLAGS =
CFLAGS += -I /usr/include
CFLAGS += -Wall 
CFLAGS += -g

LDFLAGS = 
LDLAGS += -Wall 
LDLAGS += -g
LDFLAGS += -ludev

compile: ClientUSBLogger.c SocketUDP.c
	gcc -c -g -Wall ClientUSBLogger.c SocketUDP.c
	
link: ClientUSBLogger.o SocketUDP.o
	gcc -Wall -ludev ClientUSBLogger.o SocketUDP.o -o $(PROJECT)
	
build: clean all

all: compile link
	@echo "\n"
	@echo "---------------------------------"
	@echo "---   Compiled Successfully   ---"
	@echo "---------------------------------"
	
clean:
	@echo "---   Clean   ---"
	rm -f *.o
	rm -f $(PROJECT)



