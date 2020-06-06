UNAME := $(shell uname)
ifeq ($(UNAME), Darwin)
	OSCCFLAG = -stdlib=libc++  
else
	OSCCFLAG = -pthread
endif

.PHONY: all clean push

CC = g++ -Wall -Wextra -O2 -std=c++14 $(OSCCFLAG)

.SUFFIXES: .cpp
.cpp.o:
	$(CC) -c $<

all: test

test: client1 server


operation: operation.o testOperation.o
	$(CC) $? -o $@

util: util.o client.o operation.o
	$(CC) $? -o $@ 

client1: client.o operation.o testClient1.o
	$(CC) $? -o $@ 

# client2: client.o operation.o testClient2.o
# 	$(CC) $? -o $@

server: server.o operation.o
	$(CC) $? -o $@


clean:
	rm -f *.o client1 client2 server
	# rm -f *.o librpc.a client server rpclib-master/Makefile rpclib-master/CMakeCache.txt

push: clean
	git add .
	git commit -m "auto commit"
	git push

