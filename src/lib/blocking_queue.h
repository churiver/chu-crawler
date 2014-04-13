#ifndef BLOCKING_QUEUE_
#define BLOCKING_QUEUE_

#include <deque>
#include <pthread.h>

#define DEFAULT_CAPACITY 5

template <class T>
class blocking_queue
{
public:
    blocking_queue (size_t capacity = DEFAULT_CAPACITY);
    
    ~blocking_queue ( );

    void put (const T & val );

    T take ( ); 

    size_t size ( );

    bool empty ( );

private:
    std::deque<T> container;

    size_t _capacity;

    pthread_mutex_t mutex_queue;
    pthread_cond_t cond_is_empty;
    pthread_cond_t cond_is_full;
};

#include "blocking_queue.cc"

#endif
