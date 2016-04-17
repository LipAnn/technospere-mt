CFLAGS=-Wall -Wextra -g -std=c++11
CC=g++

all: runshell
runshell: runshell.o parser.o
	$(CC) $(CFLAGS) $^ -o $@
runshell.o: runshell.cpp runshell.hpp
	$(CC) $(CFLAGS) -c $< 
parser.o: parser.cpp parser.hpp
	$(CC) $(CFLAGS) -c $<

clean: 
	rm -f *.o runshell
