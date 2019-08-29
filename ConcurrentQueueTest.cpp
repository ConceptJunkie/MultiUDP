//******************************************************************************
//
//  ConcurrentQueue unit tests
//
//  TODO:  use gtest...
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
//  insertData
//
//******************************************************************************

void insertData( int nPrefix, int nItemsPerThread, concurrentQueue< int > & testQueue ) {
	for ( int i = 0; i < nItemsPerThread; i++ ) {
        testQueue.push( nPrefix * nItemsPerThread + i );
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

	int nItem;

	while ( testQueue.tryPop( nItem ) ) {
		items.push_back( nItem );
	}

	std::sort( items.begin( ), items.end( ) );

	if ( items.size( ) != nThreads * nItemsPerThread ) {
		std::cout << "test1:  Found " << items.size( ) << " items, but expected " <<
		              nThreads * nItemsPerThread << std::endl;
		bSuccess = false;
	}

    for ( int i = 0; i < nThreads * nItemsPerThread; i++ ) {
		if ( items[ i ] != i ) {
			std::cout << "test2:  Somehow, we don't have the items we inserted!" << std::endl;
			bSuccess = false;
			break;
		}
	}

	return bSuccess;
}


//******************************************************************************
//
//  popData
//
//******************************************************************************

void popData( concurrentQueue< int > & testQueue, std::vector< int > & items ) {
	int nItem;

	while ( testQueue.tryPop( nItem ) ) {
		items.push_back( nItem );
    }
}


//******************************************************************************
//
//  testPopping
//
//******************************************************************************

bool testPopping( int nThreads, int nItems ) {
	bool bSuccess = true;

    concurrentQueue< int > testQueue;

    for ( int i = 0; i < nItems; i++ ) {
        testQueue.push( i );
    }

    std::vector< std::thread > threads( nThreads );
	std::vector< std::vector< int > > itemVectors( nThreads );

    for ( int i = 0; i < nThreads; i++ ) {
        threads[ i ] = std::thread( popData, std::ref( testQueue ), std::ref( itemVectors[ i ] ) );
        threads[ i ].join( );
    }

	std::vector< int > items;

    for ( int i = 0; i < nThreads; i++ ) {
		items.insert( std::end( items ), std::begin( itemVectors[ i ] ), std::end( itemVectors[ i ] ) );
	}

	std::sort( items.begin( ), items.end( ) );

	if ( items.size( ) != nItems ) {
		std::cout << "test2:  Found " << items.size( ) << " items, but expected " << nItems << std::endl;
		return false;
	}

    for ( int i = 0; i < nItems; i++ ) {
		if ( items[ i ] != i ) {
			std::cout << "test2:  Somehow, we don't have the items we inserted!" << std::endl;
			bSuccess = false;
			break;
		}
	}

	return bSuccess;
}


int main( int argc, char * argv[ ] ) {
	// step 1, test inserting
	std::cout << "Test 1 " << ( testInserting( 1, 100 ) ? "is successful" : "failed" ) << std::endl;
	std::cout << "Test 2 " << ( testInserting( 2, 10000 ) ? "is successful" : "failed" ) << std::endl;
	std::cout << "Test 3 " << ( testInserting( 10, 10000 ) ? "is successful" : "failed" ) << std::endl;
	std::cout << "Test 4 " << ( testInserting( 100, 10000 ) ? "is successful" : "failed" ) << std::endl;
	std::cout << "Test 5 " << ( testInserting( 1000, 10000 ) ? "is successful" : "failed" ) << std::endl;
	std::cout << "Test 6 " << ( testInserting( 10, 1000000 ) ? "is successful" : "failed" ) << std::endl;

	std::cout << "Test 7 " << ( testPopping( 1, 100 ) ? "is successful" : "failed" ) << std::endl;
	std::cout << "Test 8 " << ( testPopping( 2, 1000 ) ? "is successful" : "failed" ) << std::endl;
	std::cout << "Test 10 " << ( testPopping( 10, 10000 ) ? "is successful" : "failed" ) << std::endl;
	std::cout << "Test 11 " << ( testPopping( 100, 100000 ) ? "is successful" : "failed" ) << std::endl;
	std::cout << "Test 12 " << ( testPopping( 1000, 1000000 ) ? "is successful" : "failed" ) << std::endl;
	std::cout << "Test 13 " << ( testPopping( 10000, 10000000 ) ? "is successful" : "failed" ) << std::endl;

	return 0;
}

