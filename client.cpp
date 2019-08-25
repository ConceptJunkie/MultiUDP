#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string>
#include <fstream>


void error( const char * msg ) {
    perror( msg );
    exit( 0 );
}


int main( int argc, char * argv[ ] ) {
    struct sockaddr_in serv_addr;

    char buffer[ 256 ];
    
    if ( argc < 4 ) {
       fprintf( stderr, "usage %s hostname port filename\n", argv[ 0 ] );
       exit( 0 );
    }
    
    int nPort = atoi( argv[ 2 ] );
    int sockfd = socket( AF_INET, SOCK_STREAM, 0 );
    
    if ( sockfd < 0 ) {
        error( "ERROR opening socket" );
    }
    
    struct hostent * pServer = gethostbyname( argv[ 1 ] );
    
    if ( !pServer ) {
        fprintf( stderr,"ERROR, no such host\n" );
        exit( 0 );
    } 
    
    bzero( ( char * ) &serv_addr, sizeof( serv_addr ) ); 
    serv_addr.sin_family = AF_INET;
    
    bcopy( ( char * ) pServer->h_addr, ( char * ) &serv_addr.sin_addr.s_addr, pServer->h_length );
    serv_addr.sin_port = htons( nPort );
    
    if ( connect( sockfd, ( struct sockaddr * ) &serv_addr, sizeof( serv_addr ) ) < 0 ) {
        error( "ERROR connecting" );
    }
    
    std::ifstream fin( argv[ 3 ] );

    if ( !fin.is_open( ) ) {
        error( "ERROR opening file" );
    }

    std::string line;

    while( getline( fin, line ) ) {  
        int nBytesWritten = write( sockfd, line.c_str( ), line.length( ) );
        
        if ( nBytesWritten < 0 ) {
            error( "ERROR writing bytes" );
        }
    }
    
    close( sockfd );
    return 0;
}
