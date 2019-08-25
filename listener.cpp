//******************************************************************************
//
//  MultiUDP listener
//
//******************************************************************************

#include <fstream>
#include <iostream>
#include <string>

#include <signal.h>
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

int sockfd = -1;
std::ofstream fileOutput;


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
//  main
//
//******************************************************************************

int main( int argc, char * argv[ ] ) {
    if ( argc < 2 ) {
        fprintf( stderr, "usage %s port filename\n", argv[ 0 ] );
        exit( 0 );
    }

    int port = std::stoi( argv[ 1 ] );      // port
    std::string strFileName = argv[ 2 ];    // output file name

    struct sockaddr_in si_other;

    // create a UDP socket
    sockfd = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );

    if ( sockfd == -1 ) {
        die( "failed to create socket" );
    }

    bzero( &si_other, sizeof( si_other ) );

    si_other.sin_family = AF_INET;
    si_other.sin_port = htons( port );
    si_other.sin_addr.s_addr = htonl( INADDR_ANY );

    // bind socket to port
    if ( bind( sockfd, ( struct sockaddr * ) &si_other, sizeof( si_other ) ) == -1 ) {
        die( "bind" );
    }

    socklen_t si_len = sizeof( si_other );

    // open the output file to save what we receive
    fileOutput.open( strFileName, std::ios::binary );

    if ( !fileOutput.is_open( ) ) {
        die( "ERROR opening file" );
    }

    // set up the sigint handler so we can cleanly stop the listener when we are finished
    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = handleSigInt;
    sigemptyset( &sigIntHandler.sa_mask );
    sigIntHandler.sa_flags = 0;

    sigaction( SIGINT, &sigIntHandler, NULL );

    char buffer[ PACKET_DATA_SIZE ];

    // keep waiting to receive data
    while ( true ) {
        bzero( buffer, PACKET_DATA_SIZE );

        // this is a blocking call
        ssize_t received =
            recvfrom( sockfd, buffer, PACKET_DATA_SIZE, MSG_WAITALL,
                      ( struct sockaddr * ) &si_other, &si_len );

        if ( received == -1 ) {
            die( "recvfrom( )" );
        }

        fileOutput.write( buffer, received );
        std::cout << "Writing " << received << " bytes to file." << std::endl;
    }

    return 0;
}


