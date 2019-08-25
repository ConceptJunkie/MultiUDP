//******************************************************************************
//
//  MultiUDP sender
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
//  main
//
//******************************************************************************

int main( int argc, char * argv[ ] ) {
    if ( argc < 4 ) {
        fprintf( stderr, "usage %s hostname port filename\n", argv[ 0 ] );
        exit( 0 );
    }

    std::string strHostName = argv[ 1 ];    // hostname
    int port = std::stoi( argv[ 2 ] );      // port number
    std::string strFileName = argv[ 3 ];    // filename

    struct sockaddr_in si_other;

    int sockfd = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );

    if ( sockfd == -1 ) {
        die( "socket" );
    }

    bzero( &si_other, sizeof( si_other ) );

    si_other.sin_family = AF_INET;
    si_other.sin_port = htons( port );

    if ( inet_aton( strHostName.c_str( ), &si_other.sin_addr ) == 0 ) {
        die( "inet_aton( ) failed\n" );
    }

    // open the file to read from
    std::ifstream fileInput( strFileName, std::ios::binary );

    if ( !fileInput.is_open( ) ) {
        die( "ERROR opening input file" );
    }

    char buffer[ PACKET_DATA_SIZE ];

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
        if ( sendto( sockfd, buffer, readSize, 0,
                     ( struct sockaddr * ) &si_other, sizeof( si_other ) ) == -1 ) {
            die( "sendto( )" );
        }

        // grab another chunk of data from the file
        fileInput.read( buffer, PACKET_DATA_SIZE );
    }

    std::cout << "finished!" << std::endl;

    fileInput.close( );

    return 0;
}

