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
#include <sys/syscall.h>
#include <unistd.h>

#define gettid() syscall(__NR_gettid)  

namespace thread {


template <class T>
BlockingQueue<T>::BlockingQueue (size_t capacity )
    : _capacity(capacity)
{
    pthread_mutex_init(&_mutex_queue, NULL);
    pthread_cond_init(&_cond_is_empty, NULL);
    pthread_cond_init(&_cond_is_full, NULL);
}


template <class T>
BlockingQueue<T>::BlockingQueue (const BlockingQueue & rhs)
    : _capacity(rhs._capacity), _container(rhs._container)
{
    pthread_mutex_init(&_mutex_queue, NULL);
    pthread_cond_init(&_cond_is_empty, NULL);
    pthread_cond_init(&_cond_is_full, NULL);
}


template <class T>
BlockingQueue<T>::~BlockingQueue ( )
{
    pthread_mutex_destroy(&_mutex_queue);
    pthread_cond_destroy(&_cond_is_empty);
    pthread_cond_destroy(&_cond_is_full);
}


template <class T>
void BlockingQueue<T>::put (const T & val )
{
    pthread_mutex_lock(&_mutex_queue);
    
//    fprintf(stderr, "thread %d before while loop in put(). size %d\n", gettid(), _container.size());
    while (!_is_stopped && (_container.size() >= _capacity)) {
        int rc = pthread_cond_wait(&_cond_is_full, &_mutex_queue);
//        fprintf(stderr, "thread %d waked up in put(). rc %d\n", gettid(), rc);
    }

    if (_is_stopped) {
//        fprintf(stderr, "thread %d exiting from put()...\n", gettid());
        pthread_mutex_unlock(&_mutex_queue);
        pthread_exit(nullptr);
    }

    _container.push_back(val);
    int rc = pthread_cond_signal(&_cond_is_empty);

    pthread_mutex_unlock(&_mutex_queue);
}


template <class T>
T BlockingQueue<T>::take ( )
{
    pthread_mutex_lock(&_mutex_queue);

//    fprintf(stderr, "thread %d before while loop in take(). size %d\n", gettid(), _container.size());
    while (!_is_stopped && _container.empty()) {
        int rc = pthread_cond_wait(&_cond_is_empty, &_mutex_queue);
//        fprintf(stderr, "thread %d waked up in take(). rc %d\n", gettid(), rc);
    }

    if (_is_stopped) {
//        fprintf(stderr, "thread %d exiting from take...\n", gettid());
        pthread_mutex_unlock(&_mutex_queue);
        pthread_exit(nullptr);
    }

    T val = _container.front();
    _container.pop_front();
    int rc = pthread_cond_signal(&_cond_is_full);

    pthread_mutex_unlock(&_mutex_queue);
    return val;
}


template <class T>
size_t BlockingQueue<T>::size ( )
{
    return _container.size();
}


template <class T>
size_t BlockingQueue<T>::capacity ( )
{
    return _capacity;
}


template <class T>
void BlockingQueue<T>::capacity (size_t value )
{
    _capacity = value;
}


template <class T>
bool BlockingQueue<T>::empty ( )
{
    return _container.empty();
}


template <class T>
bool BlockingQueue<T>::full ( )
{
    return (_container.size() >= _capacity);
}


template <class T>
void BlockingQueue<T>::clear ( )
{
    _container.clear();
}


template <class T>
void BlockingQueue<T>::interrupt ( )
{
    pthread_mutex_lock(&_mutex_queue);
    _is_stopped = true;
    pthread_cond_broadcast(&_cond_is_empty);
    pthread_cond_broadcast(&_cond_is_full);
    pthread_mutex_unlock(&_mutex_queue);
}

};

#endif
