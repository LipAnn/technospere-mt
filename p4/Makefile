CFLAGS=-Wall -Wextra -g -std=c++11
CC=g++

all: reverse_index
reverse_index: reverse_index.o filereader.o filesorter.o
	$(CC) $(CFLAGS) $^ -o $@
reverse_index.o: reverse_index.cpp
	$(CC) $(CFLAGS) -c $< 
filereader.o: filereader.cpp filereader.hpp
	$(CC) $(CFLAGS) -c $<
filesorter.o: filesorter.cpp filesorter.hpp
	$(CC) $(CFLAGS) -c $<

clean: 
	rm -f *.o reverse_index
