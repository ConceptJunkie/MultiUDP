//******************************************************************************
//
//  MultiUDP server
//
//******************************************************************************

#include <cctype>
#include <chrono>
#include <ctime>
#include <iostream>
#include <map>
#include <random>
#include <sstream>
#include <string>
#include <thread>

#include <unistd.h>

#include <arpa/inet.h>

#include <sys/socket.h>

#include "ConcurrentQueue.h"
#include "PacketData.h"
#include "utils.h"

#define THREADS     8


//******************************************************************************
//
//  processData
//
//******************************************************************************

void processData( concurrentQueue< PacketData > & inputQueue,
                  concurrentPriorityQueue< PacketData, PacketDataIndexLessThan > & outputQueue ) {
    PacketData data;

    while ( true ) {
        inputQueue.waitAndPop( data );

        PacketData newData( data.index );
        transformPacket( data, newData );
        outputQueue.push( newData );

        std::cout << "received packet " << newData.index << std::endl;
    }
}


//******************************************************************************
//
//  outputData
//
//******************************************************************************

void outputData( concurrentPriorityQueue< PacketData, PacketDataIndexLessThan > & outputQueue,
                 int sockfd, const struct sockaddr_in & si_client ) {
    PacketData data;

    int nPacketIndex = 0;

    while ( true ) {
        outputQueue.waitForIndexAndPop( nPacketIndex, data );

        nPacketIndex++;

        // now reply the client with the same data
        if ( sendto( sockfd, data.buffer, data.used, 0, ( struct sockaddr * )
                     &si_client, sizeof( si_client ) ) == -1 ) {
            die( "sendto( )" );
        }
    }
}


//******************************************************************************
//
//  main
//
//******************************************************************************

int main( int argc, char * argv[ ] ) {
    if ( argc < 2 ) {
        std::cerr << "usage:  " << argv[ 0 ] << " port" << std::endl;
        exit( 1 );
    }

    int nPort = std::stoi( argv[ 1 ] );

    struct sockaddr_in si_server,
                       si_client;

	socklen_t si_size = sizeof( si_client );

    PacketData data;

    // create a UDP socket
    int sockfd = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );

	if ( sockfd == -1 ) {
		die( "failed to create socket" );
	}

    bzero( &si_server, sizeof( si_server ) );

	si_server.sin_family = AF_INET;
	si_server.sin_port = htons( nPort );
	si_server.sin_addr.s_addr = htonl( INADDR_ANY );

	// bind socket to port
	if ( bind( sockfd, ( struct sockaddr * ) &si_server, sizeof( si_server ) ) == -1 ) {
		die( "bind" );
	}

    socklen_t si_len = sizeof( si_client );

    // set up our data structures and worker threads
    printTime( "creating worker threads" );

    concurrentQueue< PacketData > inputQueue;
    concurrentPriorityQueue< PacketData, PacketDataIndexLessThan > outputQueue;

    int nPacketIndex = 0;

    std::vector< std::thread > threads( THREADS - 2 );

    for ( int i = 0; i < THREADS - 2; i++ ) {
        threads[ i ] = std::thread( processData, std::ref( inputQueue ), std::ref( outputQueue ) );
        threads[ i ].detach( );
    }

    std::thread outputThread( outputData, std::ref( outputQueue ), sockfd, std::cref( si_client ) );
    outputThread.detach( );

    // keep listening for data
    printTime( "start waiting for data" );

	while ( true ) {
		fflush( stdout );

        PacketData data( nPacketIndex++ );

        // try to receive some data, this is a blocking call
        int nReceived = recvfrom( sockfd, data.buffer, PACKET_DATA_SIZE, 0,
                                  ( struct sockaddr * ) &si_client, &si_len );

        if ( nReceived == -1 ) {
            die( "recvfrom( )" );
		}

        // print details of the client/peer and the data received
        std::cout << "Received packet from " << inet_ntoa( si_client.sin_addr ) << ":" <<
                     ntohs( si_client.sin_port ) << std::endl;

        data.used = nReceived;

        // push the received packet into the queue to be processed
        printTime( "pushing packet " + std::to_string( nPacketIndex - 1 ) + " to input queue" );
        inputQueue.push( data );
	}

	close( sockfd );
	return 0;
}

