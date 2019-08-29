//******************************************************************************
//
//  ConcurrentQueue unit tests
//
//******************************************************************************

#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include <thread>

#include "ConcurrentQueue.h"
#include "utils.h"


//******************************************************************************
//
//  processData
//
//******************************************************************************

void insertData( int nPrefix, int nItemsPerThread, concurrentQueue< int > & testQueue ) {
	for ( int i = 0; i < nItemsPerThread; i++ ) {
        testQueue.push( nPrefix * nItemsPerThread + 1 );
    }
}


//******************************************************************************
//
//  testInserting
//
//******************************************************************************

bool testInserting( int nThreads, int nItemsPerThread ) {
	bool bSuccess = true;

    concurrentQueue< int > testQueue;

    std::vector< std::thread > threads( nThreads );

    for ( int i = 0; i < nThreads; i++ ) {
        threads[ i ] = std::thread( insertData, i, nItemsPerThread, std::ref( testQueue ) );
        threads[ i ].join( );
    }

	std::vector< int > items;

	while ( true ) {
		int nItem;

		if ( !testQueue.tryPop( nItem ) ) {
			break;
		}

		items.push_back( nItem );
	}

	std::sort( items.begin( ), items.end( ) );

	if ( items.size( ) != nThreads * nItemsPerThread ) {
		std::cout << "Found " << items.size( ) << " items, but expected " <<
		              nThreads * nItemsPerThread << std::endl;
		bSuccess = false;
	}

	return bSuccess;
}


int main( int argc, char * argv[ ] ) {
	// step 1, test inserting
	testInserting( 1, 100 );
	testInserting( 2, 10000 );
	testInserting( 10, 10000 );
	testInserting( 100, 10000 );
	testInserting( 1000, 10000 );
	testInserting( 1000, 100000 );
	return 0;
}

