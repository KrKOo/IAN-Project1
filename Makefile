CC=gcc
LDFLAGS=-lelf

all: ian-proj1

ian-proj1: ian-proj1.o
	$(CC) -o $@ $^ $(LDFLAGS)

.PHONY : clean
clean :
	-rm -rf *.o ian-proj1  