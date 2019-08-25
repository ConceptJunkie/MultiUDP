//******************************************************************************
//
//  MultiUDP client
//
//******************************************************************************

#include <fstream>
#include <iostream>
#include <string>
#include <thread>

#include <unistd.h>

#include <arpa/inet.h>

#include <sys/socket.h>

#include "PacketData.h"
#include "utils.h"


//******************************************************************************
//
//  globals
//
//******************************************************************************

std::ofstream fileOutput;
int sockfd;


//******************************************************************************
//
//  handleSigInt
//
//  listener uses ctrl-c as a signal to clean up and shut down
//
//******************************************************************************

void handleSigInt( int signal ) {
    std::cout << std::endl << "Shutting down..." << std::endl;

    fileOutput.close( );
    close( sockfd );

    exit( 0 );
}


//******************************************************************************
//
//  outputData
//
//******************************************************************************

void outputData( int sockfd, const struct sockaddr_in & si_other, std::ofstream & fileOutput ) {
    PacketData data;

    int nPacketIndex = 0;

    char buffer[ PACKET_DATA_SIZE ];

    std::streamsize readSize;

    socklen_t si_len = sizeof( si_other );

    while ( true ) {
        // clear the buffer by filling null, it might have previously received data
        bzero( buffer, PACKET_DATA_SIZE );

        std::cout << "receiving..." << std::endl;

        // try to receive some data, this is a blocking call
        if ( recvfrom( sockfd, buffer, readSize, 0, ( struct sockaddr * ) &si_other, &si_len ) == -1 ) {
            die( "recvfrom( )" );
        }

        std::cout << "got it! (" << readSize << ")" << std::endl;

        fileOutput.write( buffer, readSize );
    }
}


//******************************************************************************
//
//  main
//
//******************************************************************************

int main( int argc, char * argv[ ] ) {
    if ( argc < 4 ) {
        fprintf( stderr, "usage %s hostname port filename\n", argv[ 0 ] );
        exit( 0 );
    }

    std::string strHostName = argv[ 1 ];    // hostname
    int nPort = std::stoi( argv[ 2 ] );     // port number
    std::string strFileName = argv[ 3 ];    // filename

    struct sockaddr_in si_other;
    socklen_t si_len = sizeof( si_other );

    char buffer[ PACKET_DATA_SIZE ];
    char message[ PACKET_DATA_SIZE ];

    sockfd = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );

    if ( sockfd == -1 ) {
        die( "socket" );
    }

    bzero( &si_other, sizeof( si_other ) );

    si_other.sin_family = AF_INET;
    si_other.sin_port = htons( nPort );

    if ( inet_aton( strHostName.c_str( ), &si_other.sin_addr ) == 0 ) {
        die( "inet_aton( ) failed\n" );
    }

    std::ifstream fileInput( strFileName, std::ios::binary );
    std::ofstream fileOutput( strFileName + ".out", std::ios::binary );

    if ( !fileInput.is_open( ) ) {
        die( "ERROR opening input file" );
    }

    if ( !fileOutput.is_open( ) ) {
        die( "ERROR opening output file" );
    }

    std::thread outputThread( outputData, sockfd, std::cref( si_other ), std::ref( fileOutput ) );
    outputThread.detach( );

    fileInput.read( buffer, PACKET_DATA_SIZE );

    while( fileInput || fileInput.gcount( ) ) {
        std::streamsize readSize;

        if ( fileInput ) {
            readSize = PACKET_DATA_SIZE;
        } else {
            readSize = fileInput.gcount( );
        }

        std::cout << "sending..." << std::endl;

        // send the message
        if ( sendto( sockfd, buffer, readSize, 0, ( struct sockaddr * ) &si_other, si_len ) == -1 ) {
            die( "sendto( )" );
        }

        // grab another chunk of data from the file
        fileInput.read( buffer, PACKET_DATA_SIZE );
    }

    std::cout << "finished!" << std::endl;

    fileInput.close( );

    // let the listener thread finish, use ctrl-c to stop
    while ( true ) {
    }

    return 0;
}

