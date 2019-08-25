//******************************************************************************
//
//  PacketData.h
//
//******************************************************************************

#include <string.h>

constexpr int PACKET_DATA_SIZE = 4096;    // Max length of buffer


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


struct PacketDataIndexLessThan {
    bool operator( ) ( const PacketData & lhs, const PacketData & rhs ) const {
        return lhs.index < rhs.index;
    }
};

