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
                  concurrentPriorityQueue< PacketData, PacketDataIndexGreaterThan > & outputQueue ) {
    PacketData data;

    while ( true ) {
        inputQueue.waitAndPop( data );

        std::stringstream ss;
        ss << "process thread received packet " << data.index;
        printTime( ss.str( ) );

        std::cout << "process thread received packet " << data.index << std::endl;

        PacketData newData( data.index );
        transformPacket( data, newData );
        outputQueue.push( newData );

        std::stringstream ss2;
        ss2 << "process thread pushing packet " << data.index << " on the output queue";
        printTime( ss2.str( ) );
    }
}


//******************************************************************************
//
//  outputData
//
//******************************************************************************

void outputData( concurrentPriorityQueue< PacketData, PacketDataIndexGreaterThan > & outputQueue,
                 int port ) {
    struct sockaddr_in si_other;

    int sockfd = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );

    if ( sockfd == -1 ) {
        die( "socket" );
    }

    printf( "\n------------------> port %d\n", port );

    bzero( &si_other, sizeof( si_other ) );

    si_other.sin_family = AF_INET;
    si_other.sin_port = htons( port );

    std::string strHostName = "127.0.0.1";

    if ( inet_aton( strHostName.c_str( ), &si_other.sin_addr ) == 0 ) {
        die( "inet_aton( ) failed\n" );
    }

    PacketData data;

    int nPacketIndex = 0;

    char buffer[ PACKET_DATA_SIZE ];

    while ( true ) {
        printTime( "output thread is waiting for a packet" );

        outputQueue.waitForIndexAndPop( nPacketIndex, data );
        printTime( "output thread got a packet off the queue" );

        nPacketIndex++;

        // now send the data to the listener
        if ( sendto( sockfd, data.buffer, data.used, 0,
                     ( struct sockaddr * ) &si_other, sizeof( si_other ) ) == -1 ) {
            die( "sendto( )" );
        }

        printTime( "output thread sent a packet to the listener" );
    }
}


//******************************************************************************
//
//  main
//
//******************************************************************************

int main( int argc, char * argv[ ] ) {
    if ( argc < 3 ) {
        std::cerr << "usage:  " << argv[ 0 ] << " port_in port_out" << std::endl;
        exit( 1 );
    }

    int portIn = std::stoi( argv[ 1 ] );
    int portOut = std::stoi( argv[ 2 ] );

    struct sockaddr_in si_server,
                       si_client;

    PacketData data;

    // create a UDP socket
    int sockfd = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );

	if ( sockfd == -1 ) {
		die( "failed to create socket" );
	}

    bzero( &si_server, sizeof( si_server ) );

	si_server.sin_family = AF_INET;
    si_server.sin_port = htons( portIn );
	si_server.sin_addr.s_addr = htonl( INADDR_ANY );

	// bind socket to port
	if ( bind( sockfd, ( struct sockaddr * ) &si_server, sizeof( si_server ) ) == -1 ) {
		die( "bind" );
	}

    socklen_t si_len = sizeof( si_client );

    // set up our data structures and worker threads
    printTime( "creating worker threads" );

    concurrentQueue< PacketData > inputQueue;
    concurrentPriorityQueue< PacketData, PacketDataIndexGreaterThan > outputQueue;

    int nPacketIndex = 0;

    std::vector< std::thread > threads( THREADS - 2 );

    for ( int i = 0; i < THREADS - 2; i++ ) {
        threads[ i ] = std::thread( processData, std::ref( inputQueue ), std::ref( outputQueue ) );
        threads[ i ].detach( );
    }

    std::thread outputThread( outputData, std::ref( outputQueue ), portOut );
    outputThread.detach( );

    // keep listening for data
    printTime( "start waiting for data" );

	while ( true ) {
        PacketData data( nPacketIndex++ );

        // try to receive some data, this is a blocking call
        int nReceived = recvfrom( sockfd, data.buffer, PACKET_DATA_SIZE, 0,
                                  ( struct sockaddr * ) &si_client, &si_len );

        if ( nReceived == -1 ) {
            die( "recvfrom( )" );
		}

        // print details of the client/peer and the data received
        std::cout << "main thread received packet from " << inet_ntoa( si_client.sin_addr ) << ":" <<
                     ntohs( si_client.sin_port ) << std::endl;

        data.used = nReceived;

        // push the received packet into the queue to be processed
        printTime( "main thread pushing packet " + std::to_string( nPacketIndex - 1 ) + " to input queue" );
        inputQueue.push( data );
	}

    // we won't really get here
	close( sockfd );
	return 0;
}

