/**
* Copyright (c) 2013-2014
* @author Li Yu
* @email churiver86 at gmail.com
* @create-date 04/15/2014
* @description blocking queue
*/

#ifndef BLOCKINGQUEUE_CC
#define BLOCKINGQUEUE_CC

#include "BlockingQueue.h"

#include <cstdio>

namespace thread {


template <class T>
BlockingQueue<T>::BlockingQueue (size_t capacity )
    : _capacity(capacity)
{
    pthread_mutex_init(&mutex_queue, NULL);
    pthread_cond_init(&cond_is_empty, NULL);
    pthread_cond_init(&cond_is_full, NULL);
}


template <class T>
BlockingQueue<T>::~BlockingQueue ( )
{
    pthread_mutex_destroy(&mutex_queue);
    pthread_cond_destroy(&cond_is_empty);
    pthread_cond_destroy(&cond_is_full);
}


template <class T>
void BlockingQueue<T>::put (const T & val )
{
    pthread_mutex_lock(&mutex_queue);
    
//    fprintf(stderr, "  BlockingQ put. 1. %d wait\n", pthread_self());
    while (container.size() == _capacity) {
        int rc = pthread_cond_wait(&cond_is_full, &mutex_queue);
//        fprintf(stderr, "  BlockingQ. put: 2. cond_wait return %d\n", rc);
    }

    container.push_back(val);
    fprintf(stderr, "BlockingQ put. %d waken up and put. queue size %d\n", 
            pthread_self(), container.size());
    
//    fprintf(stderr, "  BlockingQ. put: 4. signal cond_is_empty. contanier size %d\n", container.size());
    int rc = pthread_cond_signal(&cond_is_empty);
//    fprintf(stderr, "  BlockingQ. put: 5. cond_signal return %d\n", rc);

    pthread_mutex_unlock(&mutex_queue);
}


template <class T>
T BlockingQueue<T>::take ( )
{
    pthread_mutex_lock(&mutex_queue);

//    fprintf(stderr, "  BlockingQ. take: 1. wait. container size %d\n", container.size());
    while (container.size() == 0) {
        int rc = pthread_cond_wait(&cond_is_empty, &mutex_queue);
//        fprintf(stderr, "  BlockingQ. take: 2. cond_wait return %d\n", rc);
    }

    T val = container.front();
    container.pop_front();
//    fprintf(stderr, "BlockingQ take. %d waken up\n", pthread_self());

//    fprintf(stderr, "  BlockingQ. take: 4. signal cond_is_full\n");
    int rc = pthread_cond_signal(&cond_is_full);
//    fprintf(stderr, "  BlockingQ. take: 5. cond_signal return %d\n", rc);

    pthread_mutex_unlock(&mutex_queue);
    return val;
}


template <class T>
size_t BlockingQueue<T>::size ( )
{
    return container.size();
}


template <class T>
size_t BlockingQueue<T>::capacity ( )
{
    return _capacity;
}


template <class T>
bool BlockingQueue<T>::empty ( )
{
    return container.empty();
}

};

#endif
