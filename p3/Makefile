CFLAGS=-Wall -std=c++11
CC=g++

all: proxy
proxy: proxy.o
	$(CC) $(CFLAGS) $< -lboost_system -o $@
proxy.o: proxy.cpp proxy.hpp
	$(CC) $(CFLAGS) -c $<  
clear: 
	rm -f *.o proxy
