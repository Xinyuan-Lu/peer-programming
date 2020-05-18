CC = g++ -Wall -Wextra -O2 -stdlib=libc++ -std=c++14 -I rpclib-master/include

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
	cd rpclib-master
	cmake ./
	make
	cp librpc.a ../
	cd ..
clean:
	rm -f *.o client server