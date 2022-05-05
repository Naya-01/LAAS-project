CC=gcc
CCFLAGS=-D_DEFAULT_SOURCE -D_XOPEN_SOURCE -D_BSD_SOURCE -std=c11 -pedantic -Wvla -Wall -Werror

all: maint pdr client

maint : maint.o 
	$(CC) $(CCFLAGS) -o maint maint.o utils_v1.o

client : client.o 
	$(CC) $(CCFLAGS) -o client client.o utils_v1.o

client.o: client.c utils_v1.o
	$(CC) $(CCFLAGS) -c client.c 

pdr : pdr.o 
	$(CC) $(CCFLAGS) -o pdr pdr.o utils_v1.o

maint.o: maint.c utils_v1.o
	$(CC) $(CCFLAGS) -c maint.c 

pdr.o: pdr.c utils_v1.o
	$(CC) $(CCFLAGS) -c pdr.c

utils_v1.o: utils_v1.c utils_v1.h
	$(CC) $(CCFLAGS) -c utils_v1.c 


clean : 
	rm -f pdr maint *.o
	clear