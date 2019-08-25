//******************************************************************************
//
//  ConcurrentQueue.h
//
//  adapted from https://www.justsoftwaresolutions.co.uk/threading/implementing-a-thread-safe-queue-using-condition-variables.html
//
//******************************************************************************

#ifndef __ConcurrentQueue_h__
#define __ConcurrentQueue_h__

#include <stdio.h>

#include <condition_variable>
#include <mutex>
#include <queue>


//******************************************************************************
//
//  class concurrentQueue
//
//******************************************************************************

template< typename Data >
class concurrentQueue {
    private:
        std::queue< Data > m_queue;
        mutable std::mutex m_mutex;
        std::condition_variable_any m_condition;

    public:
        concurrentQueue & operator =( const concurrentQueue & ) = delete;

        void push( const Data & data ) {
            std::unique_lock< std::mutex > lock( m_mutex );
            m_queue.push( data );
            lock.unlock( );

            m_condition.notify_one( );
        }

        bool tryPop( Data & value ) {
            std::unique_lock< std::mutex > lock( m_mutex );

            if ( m_queue.empty( ) ) {
                return false;
            }

            value = m_queue.front( );
            m_queue.pop( );

            return true;
        }

        void waitAndPop( Data & value ) {
            std::unique_lock< std::mutex > lock( m_mutex );

            while ( m_queue.empty( ) ) {
                m_condition.wait( lock );
            }

            value = m_queue.front( );
            m_queue.pop( );
        }

};


//******************************************************************************
//
//  class concurrentPriorityQueue
//
//******************************************************************************

template< typename Data, typename Compare >
class concurrentPriorityQueue {
    private:
        std::priority_queue< Data, std::vector< Data >, Compare > m_queue;
        mutable std::mutex m_mutex;
        std::condition_variable_any m_condition;

    public:
        concurrentPriorityQueue & operator =( const concurrentPriorityQueue & ) = delete;

        void push( const Data & data ) {
            std::unique_lock< std::mutex > lock( m_mutex );
            m_queue.push( data );
            lock.unlock( );

            m_condition.notify_one( );
        }

        bool tryPop( Data & value ) {
            std::unique_lock< std::mutex > lock( m_mutex );

            if ( m_queue.empty( ) ) {
                return false;
            }

            value = m_queue.front( );
            m_queue.pop( );

            return true;
        }

        // We are waiting for a specific index to show up, because we want to
        // output the packets in order, so we watch the index of the top item
        // in the priority queue.
        const Data & waitForIndexAndPop( int nIndex, Data & value ) {
            std::unique_lock< std::mutex > lock( m_mutex );

            while ( m_queue.empty( ) || ( m_queue.top( ).index != nIndex ) ) {
                //if ( m_queue.empty( ) ) {
                //    printf( "---> waiting for %d, but queue is empty\n", nIndex );
                //} else {
                //    printf( "---> waiting for %d, but top index is %d\n", nIndex, m_queue.top( ).index );
                //}
                m_condition.wait( lock );
            }

            value = m_queue.top( );
            m_queue.pop( );
        }
};

#endif

