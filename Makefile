UNAME := $(shell uname)
ifeq ($(UNAME), Darwin)
	OSCCFLAG = -stdlib=libc++ -std=c++14 
else
	OSCCFLAG = -std=c++14
endif

CC = g++ -Wall -Wextra -O2 $(OSCCFLAG) -I rpclib-master/include

all: client server 

client: client.o librpc.a
	$(CC) client.o librpc.a -o client

server: server.o librpc.a
	$(CC) server.o librpc.a -o server

server.o: server.cpp
	$(CC) -c server.cpp -o server.o

client.o: client.cpp
	$(CC) -c client.cpp -o client.o

librpc.a:
	cd rpclib-master && cmake ./ && make && cp librpc.a ../ && cd ..
	
clean:
	rm -f *.o librpc.a client server rpclib-master/Makefile rpclib-master/CMakeCache.txt

push: clean
	git add .
	git commit -m "auto commit"
	git push