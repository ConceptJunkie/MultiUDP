//******************************************************************************
//
//  MultiUDP utils
//
//******************************************************************************

#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <thread>

#include <arpa/inet.h>

#include <sys/socket.h>

#include "PacketData.h"


//******************************************************************************
//
//  globals
//
//******************************************************************************

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

//          if ( std::islower( c ) ) {
//              newPacket.buffer[ i ] = static_cast< char >( std::toupper( c ) );
//          } else {
            newPacket.buffer[ i ] = c;
//          }
    }

    //std::default_random_engine generator;
    //std::uniform_int_distribution< int > distribution( 100, 2000 );
    //int nMS = distribution( generator );
    int nMS = 100 + rand( ) % 1000;

    std::cout << "sleep " << nMS << " ms (thread: " << getThreadID( ) << ")" << std::endl;

    //std::this_thread::sleep_for( std::chrono::milliseconds( distribution( generator ) ) );
    std::this_thread::sleep_for( std::chrono::milliseconds( nMS ) );

    //printf( "here: (size: %d) '%s'\n", newPacket.used, newPacket.buffer );
}


//******************************************************************************
//
//  createUDPSocket
//
//******************************************************************************

int createUDPSocket( int nPort, struct sockaddr_in & si_client ) {
    struct sockaddr_in si_server;

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


    return sockfd;
}

