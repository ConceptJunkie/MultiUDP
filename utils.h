//******************************************************************************
//
//  utils.h
//
//******************************************************************************

#ifndef __utils_h__
#define __utils_h__

class PacketData;
struct sockaddr_in;

void die( const char * s );
int getThreadID( );
void printTime( const std::string & strTag );
void transformPacket( const PacketData & oldPacket, PacketData & newPacket );
int createUDPSocket( int nPort, struct sockaddr_in & si_client );

#endif


