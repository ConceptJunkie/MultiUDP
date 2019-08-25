//******************************************************************************
//
//  MultiUDP client
//
//******************************************************************************

#include <arpa/inet.h>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFFER_LENGTH 512    //Max length of buffer


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

    char buffer[ BUFFER_LENGTH ];
    char message[ BUFFER_LENGTH ];

    int sockfd = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );

    if ( sockfd == -1 ) {
        die( "socket" );
    }

    bzero( &si_other, sizeof( si_other ) );

    si_other.sin_family = AF_INET;
    si_other.sin_port = htons( nPort );

    if ( inet_aton( strHostName.c_str( ), &si_other.sin_addr ) == 0 ) {
        die( "inet_aton( ) failed\n" );
    }

    std::ifstream fileInput( strFileName );

    if ( !fileInput.is_open( ) ) {
        die( "ERROR opening file" );
    }

    std::string line;

    while( getline( fileInput, line ) ) {
        // send the message
        if ( sendto( sockfd, line.c_str( ), line.length( ) , 0 , ( struct sockaddr * ) &si_other, si_len ) == -1 ) {
            die( "sendto( )" );
        }

        // clear the buffer by filling null, it might have previously received data
        bzero( buffer, BUFFER_LENGTH );

        // try to receive some data, this is a blocking call
        if ( recvfrom( sockfd, buffer, BUFFER_LENGTH, 0, ( struct sockaddr * ) &si_other, &si_len ) == -1 ) {
            die( "recvfrom( )" );
        }

        printf( "message:  '%s'\n", buffer );
    }

    close( sockfd );

    return 0;
}


