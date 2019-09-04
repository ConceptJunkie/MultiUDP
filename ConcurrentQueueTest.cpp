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
        explicit TemplateTypeBase( int nID = 0 ) :
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

constexpr int BIGGER_TYPE_SIZE = 100;

class BiggerType : public TemplateTypeBase {
	public:
        explicit BiggerType( int nID = 0 ) : TemplateTypeBase( nID ) {
            for ( int i = 0; i < BIGGER_TYPE_SIZE; i++ ) {
				m_buffer[ i ] = i * i + getID( );
			}
		}

		virtual bool isValid( ) const {
            for ( int i = 0; i < BIGGER_TYPE_SIZE; i++ ) {
				if ( m_buffer[ i ] != i * i + getID( ) ) {
					return false;
				}
			}

			return true;
		}

	private:
		int m_nID;
        int m_buffer[ BIGGER_TYPE_SIZE ];
};


//******************************************************************************
//
//  insertData
//
//******************************************************************************

template< typename T >
void insertData( int nPrefix, int nItemsPerThread, concurrentQueue< T > & testQueue ) {
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

    concurrentQueue< T > testQueue;

    std::vector< std::thread > threads( nThreads );

    for ( int i = 0; i < nThreads; i++ ) {
        threads[ i ] = std::thread( insertData< T >, i, nItemsPerThread, std::ref( testQueue ) );
        threads[ i ].join( );
    }

	std::vector< T > items;

    T item;

    while ( testQueue.tryPop( item ) ) {
        items.push_back( item );
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
void popData( concurrentQueue< T > & testQueue, std::vector< T > & items ) {
    T item;

    while ( testQueue.tryPop( item ) ) {
        items.push_back( item );
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

    concurrentQueue< T > testQueue;

    for ( int i = 0; i < nItems; i++ ) {
        testQueue.push( T( i ) );
    }

    std::vector< std::thread > threads( nThreads );
	std::vector< std::vector< T > > itemVectors( nThreads );

    for ( int i = 0; i < nThreads; i++ ) {
        threads[ i ] = std::thread( popData< T >, std::ref( testQueue ), std::ref( itemVectors[ i ] ) );
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
    int testIndex = 1;

    std::cout << "Test " << testIndex++ << " " << ( testInserting< TemplateTypeBase >( 1, 100 ) ? "is successful" : "failed" ) << std::endl;
    std::cout << "Test " << testIndex++ << " " << ( testInserting< TemplateTypeBase >( 2, 10000 ) ? "is successful" : "failed" ) << std::endl;
    std::cout << "Test " << testIndex++ << " " << ( testInserting< TemplateTypeBase >( 10, 10000 ) ? "is successful" : "failed" ) << std::endl;
    std::cout << "Test " << testIndex++ << " " << ( testInserting< TemplateTypeBase >( 100, 10000 ) ? "is successful" : "failed" ) << std::endl;
    std::cout << "Test " << testIndex++ << " " << ( testInserting< TemplateTypeBase >( 1000, 10000 ) ? "is successful" : "failed" ) << std::endl;
    std::cout << "Test " << testIndex++ << " " << ( testInserting< TemplateTypeBase >( 10, 1000000 ) ? "is successful" : "failed" ) << std::endl;

    std::cout << "Test " << testIndex++ << " " << ( testInserting< BiggerType >( 1, 100 ) ? "is successful" : "failed" ) << std::endl;
    std::cout << "Test " << testIndex++ << " " << ( testInserting< BiggerType >( 2, 10000 ) ? "is successful" : "failed" ) << std::endl;
    std::cout << "Test " << testIndex++ << " " << ( testInserting< BiggerType >( 10, 10000 ) ? "is successful" : "failed" ) << std::endl;
    std::cout << "Test " << testIndex++ << " " << ( testInserting< BiggerType >( 100, 10000 ) ? "is successful" : "failed" ) << std::endl;

	// step 1, test popping
    std::cout << "Test " << testIndex++ << " " << ( testPopping< TemplateTypeBase >( 1, 100 ) ? "is successful" : "failed" ) << std::endl;
    std::cout << "Test " << testIndex++ << " " << ( testPopping< TemplateTypeBase >( 2, 1000 ) ? "is successful" : "failed" ) << std::endl;
    std::cout << "Test " << testIndex++ << " " << ( testPopping< TemplateTypeBase >( 10, 1000 ) ? "is successful" : "failed" ) << std::endl;
    std::cout << "Test " << testIndex++ << " " << ( testPopping< TemplateTypeBase >( 100, 100000 ) ? "is successful" : "failed" ) << std::endl;
    std::cout << "Test " << testIndex++ << " " << ( testPopping< TemplateTypeBase >( 1000, 1000000 ) ? "is successful" : "failed" ) << std::endl;
    std::cout << "Test " << testIndex++ << " " << ( testPopping< TemplateTypeBase >( 10000, 10000000 ) ? "is successful" : "failed" ) << std::endl;

    std::cout << "Test " << testIndex++ << " " << ( testPopping< BiggerType >( 1, 100 ) ? "is successful" : "failed" ) << std::endl;
    std::cout << "Test " << testIndex++ << " " << ( testPopping< BiggerType >( 2, 1000 ) ? "is successful" : "failed" ) << std::endl;
    std::cout << "Test " << testIndex++ << " " << ( testPopping< BiggerType >( 10, 10000 ) ? "is successful" : "failed" ) << std::endl;
    std::cout << "Test " << testIndex++ << " " << ( testPopping< BiggerType >( 100, 10000 ) ? "is successful" : "failed" ) << std::endl;

	return 0;
}

