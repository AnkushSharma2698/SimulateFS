CC = g++
CFLAGS = -Wall -Werror -std=c++11

fs: compile
	$(CC) $(CFLAGS) -o fs FileSystem.cc

compile:
	$(CC) $(CFLAGS) -c FileSystem.cc
clean:
	rm *.o
	rm ./fs

compress:
	tar -czf filesystem.tar.gz FileSystem.cc FileSystem.h Makefile readme.md