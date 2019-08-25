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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <thread>
#include <unistd.h>

#include <arpa/inet.h>

#include <sys/socket.h>

#include "ConcurrentQueue.h"
#include "PacketData.h"

#define THREADS     8

long long sinceEpoch = std::chrono::duration_cast< std::chrono::milliseconds >( std::chrono::steady_clock::now( ).time_since_epoch( ) ).count( );


//******************************************************************************
//
//  die
//
//******************************************************************************

void die( const char * s ) {
    perror( s );
    exit( 1 );
}


//******************************************************************************
//
//  getThreadID
//
//******************************************************************************

int getThreadID( ) {
    static int threads = 0;
    static std::map< std::thread::id, int > threadMap;

    std::thread::id threadID = std::this_thread::get_id( );

    auto it = threadMap.find( threadID );

    if ( it == threadMap.end( ) ) {
        threadMap.emplace( threadID, threads++ );
        return threads - 1;
    } else {
        return it->second;
    }
}


//******************************************************************************
//
//  printTime
//
//******************************************************************************

void printTime( const std::string & strTag ) {
    std::stringstream ss;
    ss << " (thread: " << getThreadID( ) << ")";

    std::chrono::system_clock::time_point now = std::chrono::system_clock::now( );
    int ms = std::chrono::duration_cast< std::chrono::milliseconds >( std::chrono::steady_clock::now( ).time_since_epoch( ) ).count( ) - sinceEpoch;
    std::cout << strTag << ss.str( ) << ": " << ms << " ms" << std::endl;
}


//******************************************************************************
//
//  transformPacket
//
//******************************************************************************

void transformPacket( const PacketData & oldPacket, PacketData & newPacket ) {
    newPacket.used = oldPacket.used;

    for ( int i = 0; i < oldPacket.used; i++ ) {
        unsigned char c = static_cast< unsigned char >( oldPacket.buffer[ i ] );

        if ( std::islower( c ) ) {
            newPacket.buffer[ i ] = static_cast< char >( std::toupper( c ) );
        } else {
            newPacket.buffer[ i ] = c;
        }
    }

    //std::default_random_engine generator;
    //std::uniform_int_distribution< int > distribution( 100, 2000 );
    //int nMS = distribution( generator );
    int nMS = 1000 + rand( ) % 500;

    std::cout << "sleep " << nMS << " ms (thread: " << getThreadID( ) << ")" << std::endl;

    //std::this_thread::sleep_for( std::chrono::milliseconds( distribution( generator ) ) );
    std::this_thread::sleep_for( std::chrono::milliseconds( nMS ) );

    //printf( "here: (size: %d) '%s'\n", newPacket.used, newPacket.buffer );
}


//******************************************************************************
//
//  processData
//
//******************************************************************************

void processData( concurrentQueue< PacketData > & inputQueue,
                  concurrentPriorityQueue< PacketData, PacketDataIndexLessThan > & outputQueue ) {
    PacketData data;

    while ( true ) {

        printTime( "pre-wait 1" );
        inputQueue.waitAndPop( data );
        printTime( "post-wait 1" );

        PacketData newData( data.index );

        transformPacket( data, newData );

        outputQueue.push( newData );
        //printf( "Received packet %d\n", newData.index );
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
        printf( "waiting for packet %d...\n", nPacketIndex );
        printTime( "pre-wait 2" );
        outputQueue.waitForIndexAndPop( nPacketIndex, data );
        printTime( "post-wait 2" );

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
        fprintf( stderr, "ERROR, no port provided\n" );
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

	// zero out the structure
    bzero( &si_server, sizeof( si_server ) );

	si_server.sin_family = AF_INET;
	si_server.sin_port = htons( nPort );
	si_server.sin_addr.s_addr = htonl( INADDR_ANY );

	// bind socket to port
	if ( bind( sockfd, ( struct sockaddr * ) &si_server, sizeof( si_server ) ) == -1 ) {
		die( "bind" );
	}

    socklen_t si_len = sizeof( si_client );

    printTime( "creating worker threads" );

    // set up our data structures and worker threads
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
        printf( "Received packet from %s:%d\n", inet_ntoa( si_client.sin_addr ),
                ntohs( si_client.sin_port ) );

        data.used = nReceived;

        // push the received packet into the queue to be processed
        printTime( "pushing packet " + std::to_string( nPacketIndex - 1 ) + " to input queue" );
        inputQueue.push( data );
	}

	close( sockfd );
	return 0;
}


