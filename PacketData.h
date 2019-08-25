//******************************************************************************
//
//  PacketData.h
//
//******************************************************************************

#ifndef __PacketData_h__
#define __PacketData_h__

#include <string.h>

constexpr int PACKET_DATA_SIZE = 4096;


//******************************************************************************
//
//  struct PacketData
//
//******************************************************************************

struct PacketData {
    char buffer[ PACKET_DATA_SIZE ];
    int index;
    int used;

    PacketData( int _index = 0 ) {
        index = _index;
        used = 0;
        bzero( buffer, PACKET_DATA_SIZE );
    }

    constexpr int getSize( ) const {
        return PACKET_DATA_SIZE;
    }
};


struct PacketDataIndexGreaterThan {
    bool operator( ) ( const PacketData & lhs, const PacketData & rhs ) const {
        return lhs.index > rhs.index;
    }
};

#endif

