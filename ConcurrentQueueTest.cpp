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
//  TemplateTypeBase
//
//******************************************************************************

class TemplateTypeBase {
	public:
		explicit TemplateTypeBase( int nID ) :
			m_nID( nID ) {
		}

		int getID( ) const {
			return m_nID;
		}

		bool operator <( const TemplateTypeBase & other ) const {
			return getID( ) < other.getID( );
		}

		bool operator >( const TemplateTypeBase & other ) const {
			return getID( ) > other.getID( );
		}

		virtual bool isValid( ) const {
			return true;
		}

	private:
		int m_nID;
};


//******************************************************************************
//
//  BiggerType
//
//******************************************************************************

constexpr int BiggerTypeSize = 1000;

class BiggerType : public TemplateTypeBase {
	public:
		explicit BiggerType( int nID ) : TemplateTypeBase( nID ) {
			for ( int i = 0; i < BiggerTypeSize; i++ ) {
				m_buffer[ i ] = i * i + getID( );
			}
		}

		virtual bool isValid( ) const {
			for ( int i = 0; i < BiggerTypeSize; i++ ) {
				if ( m_buffer[ i ] != i * i + getID( ) ) {
					return false;
				}
			}

			return true;
		}

	private:
		int m_nID;
		int m_buffer[ BiggerTypeSize ];
};


//******************************************************************************
//
//  insertData
//
//******************************************************************************

template< typename T >
void insertData( int nPrefix, int nItemsPerThread, concurrentQueue< int > & testQueue ) {
	for ( int i = 0; i < nItemsPerThread; i++ ) {
        testQueue.push( T( nPrefix * nItemsPerThread + i ) );
    }
}


//******************************************************************************
//
//  testInserting
//
//******************************************************************************

template< typename T >
bool testInserting( int nThreads, int nItemsPerThread ) {
	bool bSuccess = true;

    concurrentQueue< int > testQueue;

    std::vector< std::thread > threads( nThreads );

    for ( int i = 0; i < nThreads; i++ ) {
        threads[ i ] = std::thread( insertData< int >, i, nItemsPerThread, std::ref( testQueue ) );
        threads[ i ].join( );
    }

	std::vector< T > items;

	int nItem;

	while ( testQueue.tryPop( nItem ) ) {
		items.push_back( T( nItem ) );
	}

	std::sort( items.begin( ), items.end( ) );

	if ( items.size( ) != nThreads * nItemsPerThread ) {
		std::cout << "test1:  Found " << items.size( ) << " items, but expected " <<
		              nThreads * nItemsPerThread << std::endl;
		bSuccess = false;
	}

    for ( int i = 0; i < nThreads * nItemsPerThread; i++ ) {
		if ( items[ i ].getID( ) != i ) {
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

template< typename T >
void popData( concurrentQueue< int > & testQueue, std::vector< T > & items ) {
	int nItem;

	while ( testQueue.tryPop( nItem ) ) {
		items.push_back( T( nItem ) );
    }
}


//******************************************************************************
//
//  testPopping
//
//******************************************************************************

template< typename T >
bool testPopping( int nThreads, int nItems ) {
	bool bSuccess = true;

    concurrentQueue< int > testQueue;

    for ( int i = 0; i < nItems; i++ ) {
        testQueue.push( i );
    }

    std::vector< std::thread > threads( nThreads );
	std::vector< std::vector< T > > itemVectors( nThreads );

    for ( int i = 0; i < nThreads; i++ ) {
        threads[ i ] = std::thread( popData< TemplateTypeBase >, std::ref( testQueue ), std::ref( itemVectors[ i ] ) );
        threads[ i ].join( );
    }

	std::vector< T > items;

    for ( int i = 0; i < nThreads; i++ ) {
		items.insert( std::end( items ), std::begin( itemVectors[ i ] ), std::end( itemVectors[ i ] ) );
	}

	std::sort( items.begin( ), items.end( ) );

	if ( items.size( ) != nItems ) {
		std::cout << "test2:  Found " << items.size( ) << " items, but expected " << nItems << std::endl;
		return false;
	}

    for ( int i = 0; i < nItems; i++ ) {
		if ( items[ i ].getID( ) != i ) {
			std::cout << "test2:  Somehow, we don't have the items we inserted!" << std::endl;
			bSuccess = false;
			break;
		}
	}

	return bSuccess;
}


//******************************************************************************
//
//  main
//
//******************************************************************************

int main( int argc, char * argv[ ] ) {
	// step 1, test inserting
	std::cout << "Test 1 " << ( testInserting< TemplateTypeBase >( 1, 100 ) ? "is successful" : "failed" ) << std::endl;
	std::cout << "Test 2 " << ( testInserting< TemplateTypeBase >( 2, 10000 ) ? "is successful" : "failed" ) << std::endl;
	std::cout << "Test 3 " << ( testInserting< TemplateTypeBase >( 10, 10000 ) ? "is successful" : "failed" ) << std::endl;
	std::cout << "Test 4 " << ( testInserting< TemplateTypeBase >( 100, 10000 ) ? "is successful" : "failed" ) << std::endl;
	std::cout << "Test 5 " << ( testInserting< TemplateTypeBase >( 1000, 10000 ) ? "is successful" : "failed" ) << std::endl;
	std::cout << "Test 6 " << ( testInserting< TemplateTypeBase >( 10, 1000000 ) ? "is successful" : "failed" ) << std::endl;

	// step 1, test popping
	std::cout << "Test 7 " << ( testPopping< TemplateTypeBase >( 1, 100 ) ? "is successful" : "failed" ) << std::endl;
	std::cout << "Test 8 " << ( testPopping< TemplateTypeBase >( 2, 1000 ) ? "is successful" : "failed" ) << std::endl;
	std::cout << "Test 9 " << ( testPopping< TemplateTypeBase >( 10, 10000 ) ? "is successful" : "failed" ) << std::endl;
	std::cout << "Test 10 " << ( testPopping< TemplateTypeBase >( 100, 100000 ) ? "is successful" : "failed" ) << std::endl;
	std::cout << "Test 11 " << ( testPopping< TemplateTypeBase >( 1000, 1000000 ) ? "is successful" : "failed" ) << std::endl;
	std::cout << "Test 12 " << ( testPopping< TemplateTypeBase >( 10000, 10000000 ) ? "is successful" : "failed" ) << std::endl;

	return 0;
}

