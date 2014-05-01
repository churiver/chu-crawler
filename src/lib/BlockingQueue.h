/**
* Copyright (c) 2013-2014
* @author Li Yu
* @email churiver86 at gmail.com
* @create-date 04/15/2014
* @description blocking queue header
*/

#ifndef BLOCKING_QUEUE_
#define BLOCKING_QUEUE_

#include <deque>
#include <pthread.h>

namespace thread {


// By default the capacity is unlimited (max int value)
#define DEFAULT_QUEUE_CAPACITY -1

template <class T>
class BlockingQueue
{
public:
    BlockingQueue (size_t capacity = DEFAULT_QUEUE_CAPACITY);
    
    ~BlockingQueue ( );

    void put (const T & val );

    T take ( ); 

    size_t size ( );

    size_t capacity ( );

    bool empty ( );

private:
    std::deque<T> container;

    size_t _capacity;

    pthread_mutex_t mutex_queue;
    pthread_cond_t cond_is_empty;
    pthread_cond_t cond_is_full;
};

};

#include "BlockingQueue.cc"

#endif
