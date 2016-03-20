CFLAGS=-Wall -std=c++11
CC=g++

all: chatsrv client
chatsrv: server.o
	$(CC) $(CFLAGS) $< -o $@
server.o: server.cpp server.hpp
	$(CC) $(CFLAGS) -c $< 

client: client.o
	$(CC) $(CFLAGS) $< -o $@
client.o: client.cpp server.hpp
	$(CC) $(CFLAGS) -c $<
clear: 
	rm -f *.o chatsrv client
