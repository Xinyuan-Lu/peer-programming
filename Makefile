CC = g++ -Wall -Wextra -O2

all: client.o server.o
	$(CC) client.o librpc.a -o client
	$(CC) server.o librpc.a -o server

server.o: server.cpp
	$(CC) -c server.cpp -o server.o

client.o: client.cpp
	$(CC) -c client.cpp -o client.o
	
clean:
	rm -f *.o client server