/**
* Copyright (c) 2013-2014
* @author Li Yu
* @email churiver86 at gmail.com
* @create-date 04/15/2014
* @description task and thread pool header
*/

#ifndef THREAD_POOL_
#define THREAD_POOL_

#include "BlockingQueue.h"

#include <vector>

namespace thread {


class Task
{
public:
    Task (void (* func_ptr)(void *), void * func_arg );
    
    ~Task ( );

    void operator() ( );

    void run ( );

private:
    void (* _func_ptr)(void *);
    void * _func_arg;
};


#define DEFAULT_THREAD_POOL_SIZE 5

class ThreadPool
{
public:
    ThreadPool (size_t pool_size = DEFAULT_THREAD_POOL_SIZE, 
                size_t que_capacity = DEFAULT_QUEUE_CAPACITY);

    ~ThreadPool ( );

    int destroy ( );

    bool addTask (Task * );

private:
    static void * executeThread (void * );
//    static void stopThread (void * );

    BlockingQueue<Task *> _task_queue;
    std::vector<pthread_t> _thread_list;
    size_t _poolid;
};

};
#endif
