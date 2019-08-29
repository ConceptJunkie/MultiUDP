all : sender listener server ConcurrentQueueTest

ConcurrentQueueTest : ConcurrentQueueTest.o
	g++ ConcurrentQueueTest.o -o ConcurrentQueueTest -lpthread

sender : sender.o utils.o
	g++ sender.o utils.o -o sender

listener : listener.o utils.o
	g++ listener.o utils.o -o listener

server : server.o utils.o
	g++ server.o utils.o -o server -lpthread

sender.o: sender.cpp PacketData.h utils.h
	g++ -c sender.cpp

listener.o: listener.cpp PacketData.h utils.h
	g++ -c listener.cpp

ConcurrentQueueTest.o : ConcurrentQueueTest.cpp ConcurrentQueue.h
	g++ -c ConcurrentQueueTest.cpp

server.o : server.cpp ConcurrentQueue.h PacketData.h
	g++ -c server.cpp

utils.o : utils.cpp
	g++ -c utils.cpp

clean :
	rm sender listener server *.o

