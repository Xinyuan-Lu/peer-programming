UNAME := $(shell uname)
ifeq ($(UNAME), Darwin)
	OSCCFLAG = -stdlib=libc++  
else
	OSCCFLAG = -pthread
endif

.PHONY: all clean push

CC = g++ -Wall -Wextra -O2 -std=c++14 $(OSCCFLAG) -I rpclib-master/include

.SUFFIXES: .cpp
.cpp.o:
	$(CC) -c $<

all: client server 

client: client.o operation.o librpc.a
	$(CC) $? -o $@

server: server.o operation.o librpc.a
	$(CC) $? -o $@

librpc.a:
	cd rpclib-master && cmake ./ && make && cp librpc.a ../ && cd ..
	
clean:
	rm -f *.o librpc.a client server rpclib-master/Makefile rpclib-master/CMakeCache.txt

push: clean
	git add .
	git commit -m "auto commit"
	git push