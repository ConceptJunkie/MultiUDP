all : client server

client : client.o utils.o
	g++ client.o utils.o -o client -lpthread

server : server.o utils.o
	g++ server.o utils.o -o server -lpthread

server.o : server.cpp ConcurrentQueue.h PacketData.h
	g++ -c server.cpp

client.o: client.cpp PacketData.h
	g++ -c client.cpp

utils.o : utils.cpp
	g++ -c utils.cpp

clean :
	rm client server *.o

