/*
 * Simple udp client
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fstream>
#include <string>

#define SERVER "127.0.0.1"
#define BUFLEN 512	//Max length of buffer
#define PORT 8888	//The port on which to send data

void die( const char * s ) {
    perror(s);
    exit(1);
}

int main( int argc, char * argv[ ] ) {
    if ( argc < 4 ) {
        fprintf( stderr, "usage %s hostname port filename\n", argv[ 0 ] );
        exit( 0 );
    }  
    
    struct sockaddr_in si_other;
    int s, i;
    socklen_t slen=sizeof(si_other);
    char buf[BUFLEN];
    char message[BUFLEN];
    
    if ( (s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }
    
    memset((char *) &si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(PORT);
    
    if (inet_aton(SERVER , &si_other.sin_addr) == 0) 
    {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }
       
    std::ifstream fin( argv[ 3 ] );
    
    if ( !fin.is_open( ) ) {
        die( "ERROR opening file" );
    }
    
    std::string line;
    
    while( getline( fin, line ) ) {
        // send the message
        if ( sendto( s, line.c_str( ), line.length( ) , 0 , ( struct sockaddr * ) &si_other, slen)==-1) {
            die( "sendto()" );
        }
        
        // receive a reply and print it
        // clear the buffer by filling null, it might have previously received data
        memset( buf,'\0', BUFLEN);
        
        //try to receive some data, this is a blocking call
        if (recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen) == -1)
        {
            die("recvfrom()");
        }
        
        printf( "message:  '%s'\n", buf );
    }
    
    close(s);
    return 0;
}


#if 0

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

#endif
