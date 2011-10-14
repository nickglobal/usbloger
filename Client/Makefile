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

compile: ClientUSBLogger.c
	gcc -c -g -Wall ClientUSBLogger.c
	
link: ClientUSBLogger.o
	gcc -Wall -ludev ClientUSBLogger.o -o cUSBLog
	
build: clean all

all: compile link
	@echo "---   Compile Complete   ---"
	
clean:
	@echo "---   Clean   ---"
	rm -f *.o



