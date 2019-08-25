/*
	Simple udp server
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFLEN 512	// Max length of buffer

void die( const char * s ) {
	perror( s );
	exit( 1 );
}

int main( int argc, char * argv[ ] ) {
    if ( argc < 2 ) {
        fprintf( stderr, "ERROR, no port provided\n" );
        exit( 1 );
    }

    int nPort = atoi( argv[ 1 ] );    
    
	struct sockaddr_in si_server, si_client;
	
	socklen_t si_size = sizeof( si_client );
    
	char buf[ BUFLEN ];

    // create a UDP socket
    int sockfd = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
    
	if ( sockfd == -1 ) {
		die( "failed to create socket" );
	}
	
	// zero out the structure
	memset( ( char * ) &si_server, 0, sizeof( si_server ) );
	
	si_server.sin_family = AF_INET;
	si_server.sin_port = htons( nPort );
	si_server.sin_addr.s_addr = htonl( INADDR_ANY );
	
	// bind socket to port
	if ( bind( sockfd, ( struct sockaddr * ) &si_server, sizeof( si_server ) ) == -1 ) {
		die( "bind" );
	}
	
	socklen_t si_len;
	
	// keep listening for data
	while ( true ) {
		printf( "Waiting for data..." );
		fflush( stdout );
		
        int recv_len = recvfrom( sockfd, buf, BUFLEN, 0, ( struct sockaddr * ) &si_client, &si_len );
        
		//try to receive some data, this is a blocking call
		if ( recv_len == -1 ) {
			die( "recvfrom()" );
		}
		
		// print details of the client/peer and the data received
		printf( "Received packet from %s:%d\n", inet_ntoa( si_client.sin_addr ), ntohs( si_client.sin_port ) );
		//printf( "Data: %s\n" , buf );
		
		//now reply the client with the same data
		if ( sendto( sockfd, buf, recv_len, 0, ( struct sockaddr * ) &si_client, si_len ) == -1 ) {
			die( "sendto()" );
		}
	}

	close( sockfd );
	return 0;
}


#if 0

/* The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main( int argc, char * argv[ ] ) {
     char buffer[ 256 ];
     struct sockaddr_in serv_addr, cli_addr;
     int n;

     if ( argc < 2 ) {
         fprintf( stderr, "ERROR, no port provided\n" );
         exit( 1 );
     }

     // create a socket
     // socket(int domain, int type, int protocol)
     int sockfd = socket( AF_INET, SOCK_STREAM, 0 );

     if ( sockfd < 0 ) {
        error( "ERROR opening socket" );
     }

     // clear address structure
     bzero( ( char * ) &serv_addr, sizeof( serv_addr ) );

     int nPort = atoi( argv[ 1 ] );

     // setup the host_addr structure for use in bind call
     // server byte order
     serv_addr.sin_family = AF_INET;

     // automatically be filled with current host's IP address
     serv_addr.sin_addr.s_addr = INADDR_ANY;

     // convert short integer value for port must be converted into network byte order
     serv_addr.sin_port = htons( nPort );

     // bind(int fd, struct sockaddr *local_addr, socklen_t addr_length)
     // bind() passes file descriptor, the address structure,
     // and the length of the address structure
     // This bind() call will bind  the socket to the current IP address on port, portno
     if ( bind( sockfd, ( struct sockaddr * ) &serv_addr, sizeof( serv_addr ) ) < 0 ) {
         error( "ERROR on binding" );
     }

     printf( "Hello1\n" );
     
     // This listen() call tells the socket to listen to the incoming connections.
     // The listen() function places all incoming connection into a backlog queue
     // until accept() call accepts the connection.
     // Here, we set the maximum size for the backlog queue to 5.
     listen( sockfd, 5 );
     
     printf( "Hello2\n" );     

     // The accept() call actually accepts an incoming connection
     socklen_t uClientLength = sizeof( cli_addr );

     // This accept() function will write the connecting client's address info
     // into the the address structure and the size of that structure is clilen.
     // The accept() returns a new socket file descriptor for the accepted connection.
     // So, the original socket file descriptor can continue to be used
     // for accepting new connections while the new socker file descriptor is used for
     // communicating with the connected client.
     int newsockfd = accept( sockfd, ( struct sockaddr * ) &cli_addr, &uClientLength );

     printf( "Hello3\n" );
     
     if ( newsockfd < 0 ) {
          error( "ERROR on accept" ); 
    }

    printf( "server: got connection from %s port %d\n",
            inet_ntoa( cli_addr.sin_addr ), ntohs( cli_addr.sin_port ) );

    // This send() function sends the 13 bytes of the string to the new socket
    send( newsockfd, "Hello, world!\n", 13, 0 );

    while ( true ) {
        bzero( buffer, 256 );
        
        int nBytesRead = read( newsockfd, buffer, 255 );

        if ( nBytesRead < 0 ) {
            error( "ERROR reading from socket" );
        }

        printf( "message: %s\n", buffer );
    }

    close( newsockfd );
    close( sockfd );

    return 0; 
}

#endif
